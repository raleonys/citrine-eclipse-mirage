#include "preloader_l/runtime/local_social_host.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include <cassert>
#include <cstring>
using namespace preloader_l;
namespace preloader_l { namespace config {
static uint64_t g_generation=5;
static bool g_loaded=false;
static const char* g_uid="10";
bool begin_runtime_configuration_read(RuntimeConfigurationReadLease* x) noexcept {
    if(!x)return false;
    *x={g_generation,true};
    return true;
}
void end_runtime_configuration_read(RuntimeConfigurationReadLease* x) noexcept {
    if(x)x->active=false;
}
uint64_t runtime_configuration_generation() noexcept {return g_generation;}
bool runtime_configuration_loaded() noexcept {return g_loaded;}
ValueView runtime_configuration_setting(const char* key) noexcept {
    static constexpr char configured[]="20,21";
    if(std::strcmp(key,"LocalSocialRecipients")==0)
        return ValueView{configured,sizeof(configured)-1,true};
    return ValueView{nullptr,0,false};
}
ProfilePolicyView runtime_profile_policy() noexcept {
    const ValueView v{g_uid,std::strlen(g_uid),true};
    return ProfilePolicyView{v,v,v,v,v,v,v,v,v};
}
} }
int main(){
    using namespace runtime;
    LocalSocialSession session{999,888,777,true};
    assert(!bind_host_social_profile(&session) && !session.bound);
    config::g_loaded=true;
    assert(bind_host_social_profile(&session));
    assert(session.actor==10 && session.profile_generation==5 && local_social_session_current(session));
    LocalSocialPending pending{};
    assert(prepare_local_social_mutation(session,99,LocalSocialAction::block_user,20,&pending)==LocalSocialResult::no_principal);
    assert(!pending.active);
    LocalSocialSnapshot before{};
    assert(snapshot_local_social(session,&before)==LocalSocialResult::ok);
    const auto generation=before.generation;
    assert(prepare_local_social_mutation(session,10,LocalSocialAction::block_user,20,&pending)==LocalSocialResult::ok);
    assert(pending.active && !pending.new_local_target);
    assert(snapshot_local_social(session,&before)==LocalSocialResult::ok && before.generation==generation);
    rollback_local_social_mutation(&pending);
    assert(snapshot_local_social(session,&before)==LocalSocialResult::ok && before.generation==generation);
    assert(prepare_local_social_mutation(session,10,LocalSocialAction::block_user,20,&pending)==LocalSocialResult::ok);
    assert(commit_local_social_mutation(session,&pending)==LocalSocialResult::ok);
    assert(snapshot_local_social(session,&before)==LocalSocialResult::ok);
    assert(before.blocked_n==1 && before.blocked[0]==20);
    assert(prepare_local_social_mutation(session,10,LocalSocialAction::unblock_user,20,&pending)==LocalSocialResult::ok);
    assert(commit_local_social_mutation(session,&pending)==LocalSocialResult::ok);
    // Host-configured 20/21 are local recipients; arbitrary 30 is not.
    assert(prepare_local_social_mutation(session,10,LocalSocialAction::request_friend,30,&pending)==LocalSocialResult::unknown_recipient);
    assert(!pending.active);
    assert(snapshot_local_social(session,&before)==LocalSocialResult::ok && before.pending_n==0);
    config::g_generation=6;
    assert(!local_social_session_current(session));
    assert(snapshot_local_social(session,&before)==LocalSocialResult::no_principal);
    assert(prepare_local_social_mutation(session,10,LocalSocialAction::block_user,22,&pending)==LocalSocialResult::no_principal);
    config::g_uid="22";
    LocalSocialSession fresh{};
    assert(bind_host_social_profile(&fresh) && fresh.actor==22);
    assert(snapshot_local_social(fresh,&before)==LocalSocialResult::ok);
    assert(before.actor==22 && before.blocked_n==0 && before.pending_n==0);
    config::g_uid="0";config::g_generation=7;
    assert(!bind_host_social_profile(&session));
    config::g_uid="18446744073709551616";
    assert(!bind_host_social_profile(&session));
    return 0;
}
