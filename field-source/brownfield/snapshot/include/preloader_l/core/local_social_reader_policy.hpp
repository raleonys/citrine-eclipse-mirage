#pragma once
#include "preloader_l/router/route_inventory.hpp"
#include "preloader_l/runtime/local_social_host.hpp"

namespace preloader_l { namespace core {
// A per-socket, immutable committed view is mandatory before local list
// responses. Zero entries retain the verified Donor1 empty response bytes.
// Populated list projection is explicitly local-only and must pass a native
// Consumer acceptance gate before shipping the final DLL.
enum class LocalSocialReaderDecision : uint8_t {
    use_existing_empty_response, render_populated_local_view, deny_untrusted_or_stale
};
// An unconfigured socket is not a trusted social session. Preserve its
// historical empty-list response; a BOUND but expired/forged session MUST
// still reach the snapshot check and fail closed (never downgrade to legacy).
inline bool requires_committed_social_read(
    const runtime::LocalSocialSession* session, router::RouteId route) noexcept {
    return session && session->bound &&
        (route==router::RouteId::query_friends || route==router::RouteId::get_block_list);
}
inline LocalSocialReaderDecision local_social_reader_decision(
    router::RouteId route, runtime::LocalSocialResult result,
    const runtime::LocalSocialSnapshot& snapshot) noexcept {
    if(result!=runtime::LocalSocialResult::ok)return LocalSocialReaderDecision::deny_untrusted_or_stale;
    if(route==router::RouteId::query_friends)
        return snapshot.friends_n==0 ? LocalSocialReaderDecision::use_existing_empty_response
            : LocalSocialReaderDecision::render_populated_local_view;
    if(route==router::RouteId::get_block_list)
        return snapshot.blocked_n==0 ? LocalSocialReaderDecision::use_existing_empty_response
            : LocalSocialReaderDecision::render_populated_local_view;
    return LocalSocialReaderDecision::deny_untrusted_or_stale;
}
} }
