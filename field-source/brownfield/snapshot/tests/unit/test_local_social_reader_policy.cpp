#include "preloader_l/core/local_social_reader_policy.hpp"
#include <cassert>
using namespace preloader_l;
int main() {
    runtime::LocalSocialStore store{};
    assert(store.register_local_actor(101));
    store.set_enabled_for_test_only(true);
    const auto lease=store.trusted_lease_for_test_only(101);
    runtime::LocalSocialSnapshot snapshot{};
    // An unconfigured socket must not spuriously fail native GetBlockList.
    runtime::LocalSocialSession unbound{};
    assert(!core::requires_committed_social_read(&unbound,router::RouteId::get_block_list));
    assert(!core::requires_committed_social_read(&unbound,router::RouteId::query_friends));
    assert(!core::requires_committed_social_read(nullptr,router::RouteId::get_block_list));
    runtime::LocalSocialSession bound{101,1,1,true};
    assert(core::requires_committed_social_read(&bound,router::RouteId::get_block_list));
    assert(core::requires_committed_social_read(&bound,router::RouteId::query_friends));
    assert(!core::requires_committed_social_read(&bound,router::RouteId::get_config));
    assert(store.read_snapshot(lease,&snapshot)==runtime::LocalSocialResult::ok);
    using core::LocalSocialReaderDecision;
    assert(core::local_social_reader_decision(router::RouteId::query_friends,runtime::LocalSocialResult::ok,snapshot)==LocalSocialReaderDecision::use_existing_empty_response);
    assert(core::local_social_reader_decision(router::RouteId::get_block_list,runtime::LocalSocialResult::ok,snapshot)==LocalSocialReaderDecision::use_existing_empty_response);
    runtime::LocalSocialPending p{};
    assert(store.prepare(lease,runtime::LocalSocialAction::block_user,202,&p)==runtime::LocalSocialResult::ok);
    // A staged mutation must remain invisible to readers.
    assert(store.read_snapshot(lease,&snapshot)==runtime::LocalSocialResult::ok&&snapshot.blocked_n==0);
    assert(store.commit(&p)==runtime::LocalSocialResult::ok);
    assert(store.read_snapshot(lease,&snapshot)==runtime::LocalSocialResult::ok&&snapshot.blocked_n==1);
    assert(core::local_social_reader_decision(router::RouteId::get_block_list,runtime::LocalSocialResult::ok,snapshot)==LocalSocialReaderDecision::render_populated_local_view);
    assert(core::local_social_reader_decision(router::RouteId::query_friends,runtime::LocalSocialResult::ok,snapshot)==LocalSocialReaderDecision::use_existing_empty_response);
    assert(core::local_social_reader_decision(router::RouteId::get_block_list,runtime::LocalSocialResult::stale_lease,snapshot)==LocalSocialReaderDecision::deny_untrusted_or_stale);
    assert(core::local_social_reader_decision(router::RouteId::query_friends,runtime::LocalSocialResult::no_principal,snapshot)==LocalSocialReaderDecision::deny_untrusted_or_stale);
    // A forged lease cannot authenticate a synthetic contact; failed reads
    // must not modify the caller's last committed snapshot.
    const auto keep=snapshot.generation;
    const runtime::LocalPrincipalLease forged{202,store.epoch(),true};
    assert(store.read_snapshot(forged,&snapshot)==runtime::LocalSocialResult::no_principal);
    assert(snapshot.generation==keep&&snapshot.blocked_n==1);
    store.rollback(&p);
    return 0;
}
