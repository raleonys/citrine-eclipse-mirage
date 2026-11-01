#include "preloader_l/runtime/local_social_state.hpp"
#include <cassert>
using namespace preloader_l::runtime;
int main(){
    LocalSocialStore s;
    assert(s.register_local_actor(10));assert(s.register_local_actor(20));
    auto a=s.trusted_lease_for_test_only(10);
    LocalSocialPending p{};
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::disabled);
    s.set_enabled_for_test_only(true);
    assert(s.prepare({10,s.epoch(),false},LocalSocialAction::block_user,20,&p)==LocalSocialResult::no_principal);
    assert(s.prepare(a,LocalSocialAction::block_user,10,&p)==LocalSocialResult::self_target);
    // A new LOCAL-ONLY numeric contact is created only on commit; no phantom
    // recipient survives prepare + rollback or a failed socket send.
    const uint64_t before=s.generation();
    assert(s.prepare(a,LocalSocialAction::block_user,30,&p)==LocalSocialResult::ok);
    assert(p.new_local_target && !s.trusted_lease_for_test_only(30).trusted);
    s.rollback(&p);
    assert(s.generation()==before && !s.trusted_lease_for_test_only(30).trusted);
    assert(s.prepare(a,LocalSocialAction::request_friend,30,&p)==LocalSocialResult::unknown_recipient);
    assert(s.prepare(a,LocalSocialAction::send_invite,30,&p)==LocalSocialResult::unknown_recipient);
    assert(s.prepare(a,LocalSocialAction::block_user,30,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(!s.trusted_lease_for_test_only(30).trusted); // synthetic contact != bound user
    // A manually forged test lease cannot promote a local contact into a principal.
    const LocalPrincipalLease forged{30,s.epoch(),true};
    LocalSocialSnapshot probe{};
    probe.generation=UINT64_C(0xDEADBEEF);
    assert(s.read_snapshot(forged,&probe)==LocalSocialResult::no_principal);
    assert(probe.generation==UINT64_C(0xDEADBEEF));
    assert(s.prepare(forged,LocalSocialAction::block_user,10,&p)==LocalSocialResult::no_principal);
    assert(!p.active);
    assert(s.prepare(a,LocalSocialAction::request_friend,30,&p)==LocalSocialResult::unknown_recipient);
    assert(s.prepare(a,LocalSocialAction::send_invite,30,&p)==LocalSocialResult::unknown_recipient);
    assert(s.register_local_actor(30)); // controlled independently verified registration
    assert(s.trusted_lease_for_test_only(30).trusted);
    assert(s.prepare(a,LocalSocialAction::send_invite,30,&p)==LocalSocialResult::blocked_pair);
    assert(s.prepare(a,LocalSocialAction::unblock_user,30,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);
    assert(s.prepare(a,LocalSocialAction::send_invite,30,&p)==LocalSocialResult::ok);
    s.rollback(&p);
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::ok);
    assert(!s.blocked(10,20));
    LocalSocialPending q{};
    assert(s.prepare(a,LocalSocialAction::request_friend,20,&q)==LocalSocialResult::reserved);
    auto stale_copy=p;
    s.rollback(&p);assert(!s.blocked(10,20));
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&stale_copy)==LocalSocialResult::stale_reservation);
    assert(!s.blocked(10,20));
    s.rollback(&p);
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);assert(s.blocked(10,20));
    assert(s.prepare(a,LocalSocialAction::request_friend,20,&p)==LocalSocialResult::blocked_pair);
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::already_exists);
    assert(s.prepare(a,LocalSocialAction::unblock_user,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);assert(!s.blocked(10,20));
    assert(s.prepare(a,LocalSocialAction::request_friend,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);assert(s.pending(10,20)&&!s.friends(10,20));
    assert(s.prepare(a,LocalSocialAction::remove_friend,20,&p)==LocalSocialResult::not_mutual);
    assert(s.prepare(a,LocalSocialAction::send_invite,20,&p)==LocalSocialResult::ok);
    assert(!s.inbox(20,10));s.rollback(&p);assert(!s.inbox(20,10));
    assert(s.prepare(a,LocalSocialAction::send_invite,20,&p)==LocalSocialResult::ok);
    assert(s.commit(&p)==LocalSocialResult::ok);assert(s.inbox(20,10));
    assert(s.prepare(a,LocalSocialAction::send_invite,20,&p)==LocalSocialResult::already_exists);
    s.reset_epoch();
    s.set_enabled_for_test_only(true);
    assert(s.prepare(a,LocalSocialAction::block_user,20,&p)==LocalSocialResult::stale_lease);
    assert(!s.blocked(10,20)&&!s.pending(10,20)&&!s.inbox(20,10));
    assert(s.register_local_actor(10));assert(s.register_local_actor(20));
    auto b=s.trusted_lease_for_test_only(10);
    assert(s.prepare(b,LocalSocialAction::block_user,20,&p)==LocalSocialResult::ok);
    s.reset_epoch();assert(s.commit(&p)==LocalSocialResult::stale_reservation);
    return 0;
}
