#include "preloader_l/runtime/local_social_state.hpp"
namespace preloader_l { namespace runtime {
namespace {
using Actor=LocalSocialStore::Actor;
bool contains(const uint64_t* values,size_t count,uint64_t v) noexcept {
    for(size_t i=0;i<count;++i)if(values[i]==v)return true;
    return false;
}
bool add(uint64_t* values,size_t* count,uint64_t v) noexcept {
    if(contains(values,*count,v))return false;
    if(*count>=LocalSocialStore::edge_limit)return false;
    values[(*count)++]=v;return true;
}
void erase(uint64_t* values,size_t* count,uint64_t v) noexcept {
    for(size_t i=0;i<*count;++i)if(values[i]==v){
        for(size_t j=i+1;j<*count;++j)values[j-1]=values[j];
        values[--(*count)]=0;return;
    }
}
}
LocalSocialStore::LocalSocialStore() noexcept = default;
LocalSocialStore::Actor* LocalSocialStore::find(uint64_t id) noexcept {
    for(auto& a:actors_)if(a.used&&a.id==id)return &a;
    return nullptr;
}
const LocalSocialStore::Actor* LocalSocialStore::find(uint64_t id) const noexcept {
    for(const auto& a:actors_)if(a.used&&a.id==id)return &a;
    return nullptr;
}
bool LocalSocialStore::register_local_actor(uint64_t id) noexcept {
    if(!id)return false;
    if(reserved_)return false;
    if(Actor* existing=find(id)){
        // A synthetic contact can be independently promoted by a verified
        // local profile binding, but never by a request target or peer IP.
        if(!existing->trusted_local_principal){existing->trusted_local_principal=true;++generation_;}
        return true;
    }
    for(auto& a:actors_)if(!a.used){a=Actor{};a.id=id;a.used=true;a.trusted_local_principal=true;++generation_;return true;}
    return false;
}
LocalPrincipalLease LocalSocialStore::trusted_lease_for_test_only(uint64_t id) const noexcept {
    const Actor* actor=find(id);
    return {id,epoch_,actor&&actor->trusted_local_principal};
}
void LocalSocialStore::set_enabled_for_test_only(bool enabled) noexcept {if(!reserved_)enabled_=enabled;}
void LocalSocialStore::enable_for_bound_local_profile() noexcept {if(!reserved_)enabled_=true;}
void LocalSocialStore::reset_epoch() noexcept {
    // Clear even pending transactions: stale tokens must not commit after reset.
    ++epoch_;++generation_;reserved_=false;enabled_=false;
    for(auto& a:actors_)a=Actor{};
}
LocalSocialResult LocalSocialStore::read_snapshot(LocalPrincipalLease lease,
                                                  LocalSocialSnapshot* out) const noexcept {
    if(!out)return LocalSocialResult::invalid_output;
    if(!enabled_)return LocalSocialResult::disabled;
    if(!lease.trusted||lease.actor==0)return LocalSocialResult::no_principal;
    if(lease.epoch!=epoch_)return LocalSocialResult::stale_lease;
    const Actor* actor=find(lease.actor);
    if(!actor||!actor->trusted_local_principal)return LocalSocialResult::no_principal;
    LocalSocialSnapshot candidate{};
    candidate.actor=actor->id;
    candidate.epoch=epoch_;
    candidate.generation=generation_;
    candidate.blocked_n=actor->blocked_n;
    candidate.friends_n=actor->friends_n;
    candidate.pending_n=actor->pending_n;
    candidate.inbox_n=actor->inbox_n;
    for(size_t i=0;i<actor->blocked_n;++i)candidate.blocked[i]=actor->blocked[i];
    for(size_t i=0;i<actor->friends_n;++i)candidate.friends[i]=actor->friends[i];
    for(size_t i=0;i<actor->pending_n;++i)candidate.pending[i]=actor->pending[i];
    for(size_t i=0;i<actor->inbox_n;++i)candidate.inbox[i]=actor->inbox[i];
    *out=candidate;
    return LocalSocialResult::ok;
}
bool LocalSocialStore::seed_mutual_friendship_for_test_only(uint64_t actor,
                                                          uint64_t target) noexcept {
    if(!enabled_||reserved_||actor==0||target==0||actor==target)return false;
    Actor* a=find(actor); Actor* b=find(target);
    if(!a||!b)return false;
    if(contains(a->blocked,a->blocked_n,target)||contains(b->blocked,b->blocked_n,actor))return false;
    const bool ab=contains(a->friends,a->friends_n,target);
    const bool ba=contains(b->friends,b->friends_n,actor);
    if(ab!=ba)return false;
    if(ab)return true;
    if(a->friends_n>=edge_limit||b->friends_n>=edge_limit)return false;
    if(!add(a->friends,&a->friends_n,target))return false;
    // Both capacities were checked with no concurrent readers/writers.
    if(!add(b->friends,&b->friends_n,actor))return false;
    erase(a->pending,&a->pending_n,target);
    erase(b->pending,&b->pending_n,actor);
    ++generation_;
    return true;
}
LocalSocialResult LocalSocialStore::prepare(LocalPrincipalLease lease,LocalSocialAction action,
                                             uint64_t target,LocalSocialPending* out) noexcept {
    if(out)*out=LocalSocialPending{action,0,0,0,0,0,false};
    if(!out)return LocalSocialResult::reserved;
    if(!enabled_)return LocalSocialResult::disabled;
    if(!lease.trusted||lease.actor==0)return LocalSocialResult::no_principal;
    if(lease.epoch!=epoch_)return LocalSocialResult::stale_lease;
    if(reserved_)return LocalSocialResult::reserved;
    const Actor* a=find(lease.actor);const Actor* b=find(target);
    if(!a||!a->trusted_local_principal)return LocalSocialResult::no_principal;
    // LOCAL_ONLY: an unknown numeric recipient may become a local-only contact
    // if and only if the response completes and the mutation commits.
    // No remote account or delivery is implied. Never publish a phantom actor
    // during prepare/rollback/partial socket send.
    if((action==LocalSocialAction::request_friend || action==LocalSocialAction::send_invite) &&
       (!b || !b->trusted_local_principal))
        return LocalSocialResult::unknown_recipient;
    if(!b && (action==LocalSocialAction::unblock_user || action==LocalSocialAction::remove_friend))
        return LocalSocialResult::not_found;
    if(!b){
        size_t available=0;
        for(const auto& slot:actors_)if(!slot.used)++available;
        if(!available)return LocalSocialResult::capacity;
    }
    if(lease.actor==target)return LocalSocialResult::self_target;
    const bool direct_block=contains(a->blocked,a->blocked_n,target);
    const bool reverse_block=b&&contains(b->blocked,b->blocked_n,lease.actor);
    switch(action){
        case LocalSocialAction::block_user:
            if(direct_block)return LocalSocialResult::already_exists;
            if(a->blocked_n==edge_limit)return LocalSocialResult::capacity;
            break;
        case LocalSocialAction::unblock_user:
            if(!direct_block)return LocalSocialResult::not_found;
            break;
        case LocalSocialAction::request_friend:
            if(direct_block||reverse_block)return LocalSocialResult::blocked_pair;
            if(contains(a->pending,a->pending_n,target)||contains(a->friends,a->friends_n,target))
                return LocalSocialResult::already_exists;
            if(a->pending_n==edge_limit)return LocalSocialResult::capacity;
            break;
        case LocalSocialAction::remove_friend:
            if(!b||!contains(a->friends,a->friends_n,target)||!contains(b->friends,b->friends_n,lease.actor))
                return LocalSocialResult::not_mutual;
            break;
        case LocalSocialAction::send_invite:
            if(direct_block||reverse_block)return LocalSocialResult::blocked_pair;
            if(b&&contains(b->inbox,b->inbox_n,lease.actor))return LocalSocialResult::already_exists;
            if(b->inbox_n==edge_limit)return LocalSocialResult::capacity;
            break;
    }
    reserved_=true;
    ++reservation_serial_;
    *out={action,lease.actor,target,epoch_,generation_,reservation_serial_,true,b==nullptr};
    return LocalSocialResult::ok;
}
LocalSocialResult LocalSocialStore::prepare_invites(LocalPrincipalLease lease,
    const uint64_t* targets,size_t count,const char* invitation,size_t invitation_n,
    LocalSocialPending* out) noexcept {
    if(out)*out=LocalSocialPending{};
    if(!out||!targets||!invitation||count==0||count>edge_limit||
       invitation_n==0||invitation_n>1024)return LocalSocialResult::invalid_output;
    LocalSocialResult first=prepare(lease,LocalSocialAction::send_invite,targets[0],out);
    if(first!=LocalSocialResult::ok)return first;
    for(size_t i=0;i<count;++i){
        const uint64_t target=targets[i];
        if(target==0||target==lease.actor){rollback(out);return LocalSocialResult::self_target;}
        for(size_t j=0;j<i;++j)if(targets[j]==target){rollback(out);return LocalSocialResult::already_exists;}
        const Actor* a=find(lease.actor);const Actor* b=find(target);
        if(!a||!b||!b->trusted_local_principal){rollback(out);return LocalSocialResult::unknown_recipient;}
        if(contains(a->blocked,a->blocked_n,target)||contains(b->blocked,b->blocked_n,lease.actor)){
            rollback(out);return LocalSocialResult::blocked_pair;
        }
        if(contains(b->inbox,b->inbox_n,lease.actor)){rollback(out);return LocalSocialResult::already_exists;}
        if(b->inbox_n>=edge_limit){rollback(out);return LocalSocialResult::capacity;}
        out->invitees[i]=target;
    }
    out->invitee_n=count;
    out->invitation_n=invitation_n;
    for(size_t i=0;i<invitation_n;++i)out->invitation[i]=invitation[i];
    out->invitation[invitation_n]='\0';
    return LocalSocialResult::ok;
}
LocalSocialResult LocalSocialStore::commit(LocalSocialPending* pending) noexcept {
    if(!pending||!pending->active||!reserved_||pending->reservation_serial!=reservation_serial_)
        return LocalSocialResult::stale_reservation;
    if(pending->epoch!=epoch_||pending->generation!=generation_) {
        reserved_=false;pending->active=false;return LocalSocialResult::stale_reservation;
    }
    Actor* a=find(pending->actor);Actor* b=find(pending->target);
    if(!a || (!b && !pending->new_local_target)){
        reserved_=false;pending->active=false;return LocalSocialResult::stale_reservation;
    }
    if(pending->new_local_target){
        // No other writer can have changed actor capacity while reserved_.
        for(auto& slot:actors_)if(!slot.used){
            slot=Actor{};slot.used=true;slot.id=pending->target;b=&slot;break;
        }
        if(!b){reserved_=false;pending->active=false;return LocalSocialResult::stale_reservation;}
    }
    // Prepare already proved capacity and all preconditions. Commit cannot allocate/fail.
    switch(pending->action){
    case LocalSocialAction::block_user:
        add(a->blocked,&a->blocked_n,b->id);
        erase(a->friends,&a->friends_n,b->id);erase(b->friends,&b->friends_n,a->id);
        erase(a->pending,&a->pending_n,b->id);erase(b->pending,&b->pending_n,a->id);
        break;
    case LocalSocialAction::unblock_user:
        erase(a->blocked,&a->blocked_n,b->id);break;
    case LocalSocialAction::request_friend:
        add(a->pending,&a->pending_n,b->id);break;
    case LocalSocialAction::remove_friend:
        erase(a->friends,&a->friends_n,b->id);erase(b->friends,&b->friends_n,a->id);break;
    case LocalSocialAction::send_invite:
        if(pending->invitee_n){
            // All targets and capacities were checked atomically at prepare.
            for(size_t k=0;k<pending->invitee_n;++k){
                Actor* recipient=find(pending->invitees[k]);
                const size_t index=recipient->inbox_n;
                recipient->inbox[index]=a->id;
                recipient->invitation_lengths[index]=pending->invitation_n;
                for(size_t m=0;m<pending->invitation_n;++m)
                    recipient->invitations[index][m]=pending->invitation[m];
                recipient->invitations[index][pending->invitation_n]='\0';
                ++recipient->inbox_n;
            }
        }else{
            const size_t index=b->inbox_n;
            b->inbox[index]=a->id;b->invitation_lengths[index]=0;
            b->invitations[index][0]='\0';++b->inbox_n;
        }
        break;
    }
    ++generation_;reserved_=false;pending->active=false;return LocalSocialResult::ok;
}
void LocalSocialStore::rollback(LocalSocialPending* pending) noexcept {
    if(!pending||!pending->active)return;
    if(pending->epoch==epoch_&&reserved_&&pending->reservation_serial==reservation_serial_)reserved_=false;
    pending->active=false;
}
bool LocalSocialStore::inbox_message(uint64_t recipient,uint64_t sender,
    const char** text,size_t* len) const noexcept {
    if(!text||!len)return false;
    const Actor* a=find(recipient);
    if(!a||!a->trusted_local_principal)return false;
    for(size_t i=0;i<a->inbox_n;++i)if(a->inbox[i]==sender){
        *text=a->invitations[i];*len=a->invitation_lengths[i];return true;
    }
    return false;
}
bool LocalSocialStore::blocked(uint64_t a,uint64_t b) const noexcept {const Actor* p=find(a);return p&&contains(p->blocked,p->blocked_n,b);}
bool LocalSocialStore::friends(uint64_t a,uint64_t b) const noexcept {const Actor* p=find(a);return p&&contains(p->friends,p->friends_n,b);}
bool LocalSocialStore::pending(uint64_t a,uint64_t b) const noexcept {const Actor* p=find(a);return p&&contains(p->pending,p->pending_n,b);}
bool LocalSocialStore::inbox(uint64_t a,uint64_t b) const noexcept {const Actor* p=find(a);return p&&contains(p->inbox,p->inbox_n,b);}
} }
