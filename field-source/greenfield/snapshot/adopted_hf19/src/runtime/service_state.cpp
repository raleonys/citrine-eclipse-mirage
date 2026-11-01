#include "service_state_internal.hpp"

namespace preloader_l { namespace runtime {
using namespace behavioral_internal;

bool prepare_behavioral_service_state_cfg(const char* cfg,size_t length,BehavioralServiceStateCandidate** candidate) noexcept {
    if(candidate)*candidate=nullptr;
    if(!candidate)return false;
    auto* c=static_cast<BehavioralServiceStateCandidate*>(alloc(sizeof(BehavioralServiceStateCandidate))); if(!c)return false; c->store=Store{};
    if(!build_cfg_store(cfg,length,&c->store)){dealloc(c,sizeof(*c));return false;}
    *candidate=c; return true;
}
void commit_behavioral_service_state_candidate(BehavioralServiceStateCandidate* candidate) noexcept {
    if(!candidate)return;
    GrantDeliveryGuard delivery_guard;
    Guard g;
    Store old{};
    replace_active_store(&candidate->store,&old);
    clear_store(old);
    dealloc(candidate,sizeof(*candidate));
}
void release_behavioral_service_state_candidate(BehavioralServiceStateCandidate* candidate) noexcept { if(!candidate)return; clear_store(candidate->store); dealloc(candidate,sizeof(*candidate)); }
void clear_behavioral_service_state() noexcept {GrantDeliveryGuard delivery_guard;Guard g;clear_active_store();}

bool register_content_record(const ContentRecordInput& in) noexcept {Guard g;return append_active_content(in);}
bool register_entitlement_record(const EntitlementRecordInput& in) noexcept {Guard g;return append_active_entitlement(in);}

void acquire_behavioral_service_state() noexcept { lock_state(); }
void release_behavioral_service_state() noexcept { unlock_state(); }
void acquire_grant_achievement_delivery() noexcept { lock_grant_delivery(); }
void release_grant_achievement_delivery() noexcept { unlock_grant_delivery(); }
size_t content_record_count_locked() noexcept {return active_content_count();}
bool content_record_at_locked(size_t i,ContentRecordView* out) noexcept {return active_content_at(i,out);}
size_t entitlement_record_count_locked() noexcept {return active_entitlement_count();}
bool entitlement_record_at_locked(size_t i,EntitlementRecordView* out) noexcept {return active_entitlement_at(i,out);}
void clear_behavioral_service_state_for_test() noexcept {clear_behavioral_service_state();}

} }
