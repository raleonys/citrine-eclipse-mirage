#include "preloader_l/runtime/local_social_state.hpp"
#include <cassert>
#include <cstring>
#include <cstdint>
using namespace preloader_l::runtime;
static bool contains(const uint64_t* ids,size_t n,uint64_t id){
    for(size_t i=0;i<n;++i)if(ids[i]==id)return true;
    return false;
}
int main(){
    LocalSocialStore s;
    assert(s.register_local_actor(10));assert(s.register_local_actor(20));
    assert(s.register_local_actor(30));
    auto a=s.trusted_lease_for_test_only(10);
    auto b=s.trusted_lease_for_test_only(20);
    LocalSocialSnapshot first{};
    first.actor=0xA5; first.epoch=0xB6; first.generation=0xC7;
    LocalSocialSnapshot unchanged=first;
    assert(s.read_snapshot(a,&first)==LocalSocialResult::disabled);
    assert(first.actor==unchanged.actor && first.epoch==unchanged.epoch && first.generation==unchanged.generation);
    s.set_enabled_for_test_only(true);
    assert(s.read_snapshot(a,nullptr)==LocalSocialResult::invalid_output);
    assert(s.read_snapshot({10,a.epoch,false},&first)==LocalSocialResult::no_principal);
    assert(first.actor==unchanged.actor && first.epoch==unchanged.epoch && first.generation==unchanged.generation);
    assert(s.read_snapshot({30,a.epoch,true},&first)==LocalSocialResult::ok);
    assert(first.actor==30 && first.blocked_n==0);
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.actor==10 && first.epoch==a.epoch && first.generation==s.generation());
    LocalSocialSnapshot before=first;
    LocalSocialPending p{};
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::ok);
    // A prepared but not committed mutation is not observable by readers.
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.generation==before.generation && first.blocked_n==0);
    s.rollback(&p);
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.generation==before.generation && first.blocked_n==0);
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.generation>before.generation && first.blocked_n==1 && first.blocked[0]==20);
    assert(before.blocked_n==0); // previously returned copy is immutable
    LocalSocialSnapshot bob{};
    assert(s.read_snapshot(b,&bob)==LocalSocialResult::ok);
    assert(bob.actor==20 && bob.blocked_n==0); // no cross-principal leaks
    assert(s.prepare(a,LocalSocialAction::unblock_user,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(s.seed_mutual_friendship_for_test_only(10,20));
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(s.read_snapshot(b,&bob)==LocalSocialResult::ok);
    assert(first.friends_n==1 && first.friends[0]==20);
    assert(bob.friends_n==1 && bob.friends[0]==10);
    assert(s.prepare(a,LocalSocialAction::remove_friend,20,&p)==LocalSocialResult::ok);
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.friends_n==1); // still old until commit
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(s.read_snapshot(b,&bob)==LocalSocialResult::ok);
    assert(first.friends_n==0 && bob.friends_n==0);
    assert(s.prepare(a,LocalSocialAction::request_friend,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.pending_n==1 && contains(first.pending,first.pending_n,20) && first.friends_n==0);
    assert(s.seed_mutual_friendship_for_test_only(10,20));
    assert(s.read_snapshot(a,&first)==LocalSocialResult::ok);
    assert(first.pending_n==0 && first.friends_n==1);
    assert(s.prepare(a,LocalSocialAction::send_invite,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(s.read_snapshot(b,&bob)==LocalSocialResult::ok);
    assert(bob.inbox_n==1 && bob.inbox[0]==10);
    s.reset_epoch();
    LocalSocialSnapshot old_copy=bob;
    assert(s.read_snapshot(b,&bob)==LocalSocialResult::disabled);
    assert(bob.actor==old_copy.actor && bob.epoch==old_copy.epoch && bob.generation==old_copy.generation && bob.inbox_n==old_copy.inbox_n);
    s.set_enabled_for_test_only(true);
    assert(s.read_snapshot(b,&bob)==LocalSocialResult::stale_lease);
    assert(bob.actor==old_copy.actor && bob.epoch==old_copy.epoch && bob.generation==old_copy.generation && bob.inbox_n==old_copy.inbox_n);
    assert(s.register_local_actor(10));assert(s.register_local_actor(20));
    auto fresh=s.trusted_lease_for_test_only(20);
    assert(s.read_snapshot(fresh,&bob)==LocalSocialResult::ok);
    assert(bob.epoch==fresh.epoch && bob.inbox_n==0 && bob.friends_n==0);
    return 0;
}
