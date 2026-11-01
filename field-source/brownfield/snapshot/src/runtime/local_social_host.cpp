#include "preloader_l/runtime/local_social_host.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <atomic>
namespace preloader_l { namespace runtime {
namespace {
// One owner across all localhost sessions. The lock is never held during
// socket send, heap allocation, XML serialization, or configuration reads.
std::atomic_flag g_gate=ATOMIC_FLAG_INIT;
LocalSocialStore g_store{};
uint64_t g_profile_generation=0;
uint64_t g_profile_actor=0;
struct Guard {
    Guard() noexcept { while(g_gate.test_and_set(std::memory_order_acquire)) {} }
    ~Guard() noexcept { g_gate.clear(std::memory_order_release); }
};
bool parse_decimal(config::ValueView v,uint64_t* out) noexcept {
    if(!out||!v.present||!v.data||v.size==0||v.size>20)return false;
    uint64_t n=0;
    for(size_t i=0;i<v.size;++i){
        const char c=v.data[i];if(c<'0'||c>'9')return false;
        const uint64_t d=static_cast<uint64_t>(c-'0');
        if(n>(UINT64_MAX-d)/10u)return false;
        n=n*10u+d;
    }
    if(n==0)return false;
    *out=n;return true;
}
// Optional explicitly host-configured local users; not an LSX claim.
// Grammar: comma-separated strict decimal IDs, 1..15 entries. Missing = none.
bool read_host_local_recipients(config::ValueView v,uint64_t* ids,size_t* count) noexcept {
    if(!ids||!count)return false;
    *count=0;
    if(!v.present)return true;
    if(!v.data||!v.size||v.size>320)return false;
    size_t begin=0;
    while(begin<v.size){
        size_t end=begin;
        while(end<v.size&&v.data[end]!=',')++end;
        if(*count>=LocalSocialStore::actor_limit-1 || end==begin)return false;
        uint64_t id=0;
        if(!parse_decimal({v.data+begin,end-begin,true},&id))return false;
        for(size_t i=0;i<*count;++i)if(ids[i]==id)return false;
        ids[(*count)++]=id;
        if(end==v.size)break;
        begin=end+1;
        if(begin==v.size)return false;
    }
    return true;
}
bool eligible(const LocalSocialSession& session) noexcept {
    if(!session.bound||session.actor==0||session.profile_generation==0)return false;
    return config::runtime_configuration_generation()==session.profile_generation;
}
}
bool bind_host_social_profile(LocalSocialSession* out) noexcept {
    if(!out)return false;
    *out=LocalSocialSession{};
    config::RuntimeConfigurationReadLease lease{};
    if(!config::begin_runtime_configuration_read(&lease))return false;
    uint64_t actor=0;
    uint64_t recipients[LocalSocialStore::actor_limit-1]{};
    size_t recipient_n=0;
    // Config2 is host-controlled policy: missing config is NOT authority.
    const bool valid=config::runtime_configuration_loaded() &&
        parse_decimal(config::runtime_profile_policy().user_id,&actor) &&
        read_host_local_recipients(config::runtime_configuration_setting("LocalSocialRecipients"),recipients,&recipient_n);
    const uint64_t generation=lease.generation;
    config::end_runtime_configuration_read(&lease);
    if(!valid||generation==0)return false;
    Guard guard;
    // Fail closed if Config2 was reloaded after the read lease.
    if(config::runtime_configuration_generation()!=generation)return false;
    if(g_profile_generation!=generation||g_profile_actor!=actor){
        g_store.reset_epoch();
        g_profile_generation=generation;
        g_profile_actor=actor;
    }
    if(!g_store.register_local_actor(actor))return false;
    for(size_t i=0;i<recipient_n;++i){
        if(recipients[i]==actor || !g_store.register_local_actor(recipients[i]))return false;
    }
    g_store.enable_for_bound_local_profile();
    *out=LocalSocialSession{actor,generation,g_store.epoch(),true};
    return true;
}
bool local_social_session_current(const LocalSocialSession& session) noexcept {
    if(!eligible(session))return false;
    Guard guard;
    return g_profile_generation==session.profile_generation &&
        g_profile_actor==session.actor && g_store.epoch()==session.store_epoch;
}
LocalSocialResult prepare_local_social_mutation(const LocalSocialSession& session,
    uint64_t claimed_actor,LocalSocialAction action,uint64_t target,
    LocalSocialPending* out) noexcept {
    if(out)*out=LocalSocialPending{action,0,0,0,0,0,false};
    if(!out||claimed_actor!=session.actor||!eligible(session))
        return LocalSocialResult::no_principal;
    Guard guard;
    if(g_profile_generation!=session.profile_generation||g_profile_actor!=session.actor||
       g_store.epoch()!=session.store_epoch)return LocalSocialResult::stale_lease;
    return g_store.prepare({session.actor,session.store_epoch,true},action,target,out);
}
LocalSocialResult commit_local_social_mutation(const LocalSocialSession& session,
    LocalSocialPending* pending) noexcept {
    if(!pending||!eligible(session))return LocalSocialResult::stale_lease;
    Guard guard;
    if(g_profile_generation!=session.profile_generation||g_profile_actor!=session.actor||
       g_store.epoch()!=session.store_epoch)return LocalSocialResult::stale_lease;
    return g_store.commit(pending);
}
LocalSocialResult prepare_local_social_invites(const LocalSocialSession& session,
    uint64_t claimed_actor,const uint64_t* targets,size_t count,
    const char* invitation,size_t invitation_n,LocalSocialPending* out) noexcept {
    if(out)*out=LocalSocialPending{};
    if(!out||claimed_actor!=session.actor||!eligible(session))return LocalSocialResult::no_principal;
    Guard guard;
    if(g_profile_generation!=session.profile_generation||g_profile_actor!=session.actor||
       g_store.epoch()!=session.store_epoch)return LocalSocialResult::stale_lease;
    return g_store.prepare_invites({session.actor,session.store_epoch,true},targets,count,
        invitation,invitation_n,out);
}
void rollback_local_social_mutation(LocalSocialPending* pending) noexcept {
    if(!pending)return;
    Guard guard;g_store.rollback(pending);
}
LocalSocialResult snapshot_local_social(const LocalSocialSession& session,
    LocalSocialSnapshot* snapshot) noexcept {
    if(!snapshot)return LocalSocialResult::invalid_output;
    if(!eligible(session))return LocalSocialResult::no_principal;
    Guard guard;
    if(g_profile_generation!=session.profile_generation||g_profile_actor!=session.actor||
       g_store.epoch()!=session.store_epoch)return LocalSocialResult::stale_lease;
    return g_store.read_snapshot({session.actor,session.store_epoch,true},snapshot);
}
} }
