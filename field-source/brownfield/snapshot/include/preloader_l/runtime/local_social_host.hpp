#pragma once
#include <stdint.h>
#include <stddef.h>
#include "preloader_l/runtime/local_social_state.hpp"
namespace preloader_l { namespace runtime {
// Local-only, Config2-backed profile binding; not remote authentication.
// Never derive the actor from LSX Request/@UserId or TCP peer address.
struct LocalSocialSession {
    uint64_t actor;
    uint64_t profile_generation;
    uint64_t store_epoch;
    bool bound;
};
// Bind only after a successful local LSX handshake. Requires an explicitly
// loaded host Config2 UserId, not the constructor fallback profile.
bool bind_host_social_profile(LocalSocialSession* out) noexcept;
bool local_social_session_current(const LocalSocialSession& session) noexcept;
LocalSocialResult prepare_local_social_mutation(const LocalSocialSession& session,
    uint64_t claimed_actor, LocalSocialAction action, uint64_t target,
    LocalSocialPending* out) noexcept;
LocalSocialResult commit_local_social_mutation(const LocalSocialSession& session,
    LocalSocialPending* pending) noexcept;
LocalSocialResult prepare_local_social_invites(const LocalSocialSession&,uint64_t claimed_actor,
    const uint64_t* targets,size_t count,const char* invitation,size_t invitation_n,
    LocalSocialPending*) noexcept;
void rollback_local_social_mutation(LocalSocialPending* pending) noexcept;
LocalSocialResult snapshot_local_social(const LocalSocialSession& session,
    LocalSocialSnapshot* snapshot) noexcept;
} }
