// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/achievement_session.hpp"
#include <mutex>
#include <stdint.h>
#include <utility>
namespace redfield {
namespace {
bool within_budget(const ConfigSnapshot& s) noexcept {
    if(s.achievements.size()>AchievementSession::max_records)return false;
    std::size_t left=AchievementSession::max_text_bytes;
    for(const auto& a:s.achievements){
        const std::string* fields[]={&a.id,&a.name,&a.description,&a.how_to,&a.image_id,&a.grant_date,&a.expiration};
        for(const auto* v:fields){if(v->size()>left)return false;left-=v->size();}
    }
    return true;
}
// HF19 arithmetic; timestamp assignment adapts borrowed view to owned string.
// This runs only against an unpublished candidate; exceptions discard it.
void project(AchievementRecord& v,int32_t increment,std::string_view timestamp,
             bool* mutated_now,bool* completed_now) {
    const bool was_complete=v.count!=0;
    if(v.progress<v.total){
        if(mutated_now)*mutated_now=true;
        const uint32_t sum=static_cast<uint32_t>(v.progress)+static_cast<uint32_t>(increment);
        v.progress=static_cast<int32_t>(sum);
        v.grant_date.assign(timestamp.data()?timestamp.data():"",timestamp.size());
        if(v.progress>=v.total){v.count=1;v.progress=v.total;}
    }
    if(completed_now)*completed_now=!was_complete&&v.count!=0;
}
}
struct AchievementSession::Control {
    mutable std::mutex mutex;
    std::shared_ptr<const ConfigSnapshot> current;
    std::weak_ptr<Pending> pending;
    bool initialized=false,closed=false;
};
struct AchievementSession::Pending {
    std::shared_ptr<Control> owner;
    std::shared_ptr<const ConfigSnapshot> before,candidate;
    std::size_t index=0;
    bool mutated=false,completed=false,inserted=false;
};
AchievementSession::AchievementSession():control_(std::make_shared<Control>()){}
AchievementSession::~AchievementSession(){close();}
AchievementSession::InitializeResult AchievementSession::initialize(std::shared_ptr<const ConfigSnapshot> initial) {
    std::lock_guard lock(control_->mutex);
    if(control_->closed)return InitializeResult::closed;
    if(control_->initialized)return InitializeResult::already_initialized;
    if(!initial)return InitializeResult::invalid;
    if(!within_budget(*initial))return InitializeResult::limit;
    control_->current=std::move(initial);control_->initialized=true;
    return InitializeResult::initialized;
}
std::shared_ptr<const ConfigSnapshot> AchievementSession::snapshot() const {
    std::lock_guard lock(control_->mutex);
    return control_->closed?nullptr:control_->current;
}
void AchievementSession::close() {
    std::lock_guard lock(control_->mutex);
    control_->closed=true;control_->pending.reset();control_->current.reset();
}
AchievementSession::PrepareResult AchievementSession::prepare(std::string_view id,int32_t increment,std::string_view timestamp,PreparedGrant& output) noexcept {
    if(output.pending_)return PrepareResult::invalid;
    try {
        std::lock_guard lock(control_->mutex);
        if(control_->closed)return PrepareResult::closed;
        if(!control_->initialized)return PrepareResult::not_initialized;
        if(!control_->pending.expired())return PrepareResult::busy;
        if(id.empty() || !id.data() || (!timestamp.data() && timestamp.size()))return PrepareResult::invalid;
        if(id.size()>max_text_bytes || timestamp.size()>max_text_bytes)return PrepareResult::limit;
        const auto before=control_->current;
        std::size_t index=0;while(index<before->achievements.size() && before->achievements[index].id!=id)++index;
        const bool inserted=index==before->achievements.size();
        if(inserted && index==max_records)return PrepareResult::limit;
        auto candidate=std::make_shared<ConfigSnapshot>(*before);
        if(inserted)candidate->achievements.push_back({std::string(id),"","","","","0000-00-00T00:00:00","0000-00-00T00:00:00",increment,0,0});
        bool mutated=false,completed=false;
        project(candidate->achievements[index],increment,timestamp,&mutated,&completed);
        if(!within_budget(*candidate))return PrepareResult::limit;
        auto pending=std::make_shared<Pending>();
        pending->owner=control_;pending->before=before;pending->candidate=std::move(candidate);
        pending->index=index;pending->mutated=mutated;pending->completed=completed;pending->inserted=inserted;
        control_->pending=pending;output.pending_=std::move(pending);
        return PrepareResult::prepared;
    }catch(...){return PrepareResult::resource;}
}
AchievementSession::PreparedGrant::~PreparedGrant(){rollback();}
AchievementSession::PreparedGrant::PreparedGrant(PreparedGrant&& other) noexcept:pending_(std::move(other.pending_)){}
AchievementSession::PreparedGrant& AchievementSession::PreparedGrant::operator=(PreparedGrant&& other) noexcept {
    if(this!=&other){rollback();pending_=std::move(other.pending_);}return *this;
}
bool AchievementSession::PreparedGrant::active() const noexcept {return bool(pending_);}
bool AchievementSession::PreparedGrant::mutated() const noexcept {return pending_ && pending_->mutated;}
bool AchievementSession::PreparedGrant::completed() const noexcept {return pending_ && pending_->completed;}
bool AchievementSession::PreparedGrant::inserted() const noexcept {return pending_ && pending_->inserted;}
std::size_t AchievementSession::PreparedGrant::index() const noexcept {return pending_?pending_->index:0;}
std::shared_ptr<const ConfigSnapshot> AchievementSession::PreparedGrant::projected() const noexcept {return pending_?pending_->candidate:nullptr;}
bool AchievementSession::PreparedGrant::commit() noexcept {
    auto p=std::move(pending_);if(!p)return false;
    try {
        std::lock_guard lock(p->owner->mutex);
        if(p->owner->closed || p->owner->pending.lock()!=p || p->owner->current!=p->before)return false;
        if(p->mutated || p->inserted)p->owner->current=p->candidate;
        p->owner->pending.reset();return true;
    }catch(...){return false;}
}
void AchievementSession::PreparedGrant::rollback() noexcept {
    auto p=std::move(pending_);if(!p)return;
    try {std::lock_guard lock(p->owner->mutex);if(p->owner->pending.lock()==p)p->owner->pending.reset();}catch(...){}
}
}
