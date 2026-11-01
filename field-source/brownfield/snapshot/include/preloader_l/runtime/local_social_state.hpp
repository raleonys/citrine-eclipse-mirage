#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {
// W03 bounded local-only model; deliberately not wired to unauthenticated TCP.
// All operations are allocation-free. A trusted-principal issuer and a
// concurrent source-of-truth owner are still required before live activation.
enum class LocalSocialAction : uint8_t { block_user, unblock_user, request_friend, remove_friend, send_invite };
enum class LocalSocialResult : uint8_t {
    ok, disabled, no_principal, stale_lease, unknown_recipient, self_target,
    already_exists, not_found, blocked_pair, not_mutual, capacity, reserved, stale_reservation, invalid_output
};
struct LocalPrincipalLease { uint64_t actor; uint64_t epoch; bool trusted; };
// Snapshot is a copy, not a pointer into mutable backing storage. Creation
// requires a trusted actor/epoch lease. This bounded W04 model assumes
// SERIALIZED access from one external owner; it is NOT a concurrent socket
// reader implementation or a replacement for the QueryFriends/GetBlockList
// wire-response contract.
struct LocalSocialSnapshot {
    uint64_t actor{0};
    uint64_t epoch{0};
    uint64_t generation{0};
    uint64_t blocked[12]{}; size_t blocked_n{0};
    uint64_t friends[12]{}; size_t friends_n{0};
    uint64_t pending[12]{}; size_t pending_n{0};
    uint64_t inbox[12]{}; size_t inbox_n{0};
};
struct LocalSocialPending {
    LocalSocialAction action{}; uint64_t actor{}; uint64_t target{};
    uint64_t epoch{}; uint64_t generation{}; uint64_t reservation_serial{};
    bool active{false}; bool new_local_target{false};
    uint64_t invitees[12]{}; size_t invitee_n{0};
    char invitation[1025]{}; size_t invitation_n{0};
};
class LocalSocialStore final {
public:
    static constexpr size_t actor_limit=16;
    static constexpr size_t edge_limit=12;
    struct Actor {
        uint64_t id;
        bool used;
        bool trusted_local_principal; // verified Config2 registration, not a fabricated contact
        uint64_t blocked[edge_limit]; size_t blocked_n;
        uint64_t friends[edge_limit]; size_t friends_n;
        uint64_t pending[edge_limit]; size_t pending_n;
        uint64_t inbox[edge_limit]; size_t inbox_n;
        char invitations[edge_limit][1025]{};
        size_t invitation_lengths[edge_limit]{};
    };
    LocalSocialStore() noexcept;
    LocalSocialStore(const LocalSocialStore&)=delete;
    LocalSocialStore& operator=(const LocalSocialStore&)=delete;
    // Only trusted host integration may invoke registration/lease creation.
    // These model methods alone do NOT authenticate a TCP peer.
    bool register_local_actor(uint64_t id) noexcept;
    LocalPrincipalLease trusted_lease_for_test_only(uint64_t id) const noexcept;
    void set_enabled_for_test_only(bool enabled) noexcept;
    void enable_for_bound_local_profile() noexcept;
    void reset_epoch() noexcept;
    // Failure-atomic bounded local read model. Caller supplies the lease;
    // request.UserId and TCP peer address cannot create an authorized lease.
    LocalSocialResult read_snapshot(LocalPrincipalLease lease, LocalSocialSnapshot* out) const noexcept;
    // Fixture-only setup to test RemoveFriend's existing-mutual-edge branch;
    // never callable from shipping request ingress.
    bool seed_mutual_friendship_for_test_only(uint64_t actor,uint64_t target) noexcept;
    LocalSocialResult prepare(LocalPrincipalLease lease,LocalSocialAction action,
                              uint64_t target,LocalSocialPending* out) noexcept;
    // Exactly one reservation covers every named recipient. No partial inbox
    // commit. Host-controlled recipients are registered separately.
    LocalSocialResult prepare_invites(LocalPrincipalLease,const uint64_t* targets,
       size_t count,const char* invitation,size_t invitation_n,LocalSocialPending*) noexcept;
    LocalSocialResult commit(LocalSocialPending* pending) noexcept;
    // Read-only in-process inbox detail; not an application-level ACK.
    bool inbox_message(uint64_t recipient,uint64_t sender,const char** text,size_t* len) const noexcept;
    void rollback(LocalSocialPending* pending) noexcept;
    bool blocked(uint64_t actor,uint64_t target) const noexcept;
    bool friends(uint64_t actor,uint64_t target) const noexcept;
    bool pending(uint64_t actor,uint64_t target) const noexcept;
    bool inbox(uint64_t actor,uint64_t target) const noexcept;
    uint64_t generation() const noexcept { return generation_; }
    uint64_t epoch() const noexcept { return epoch_; }
private:
    Actor actors_[actor_limit]{};
    uint64_t epoch_{1};
    uint64_t generation_{1};
    bool enabled_{false};
    bool reserved_{false};
    uint64_t reservation_serial_{0};
    Actor* find(uint64_t id) noexcept;
    const Actor* find(uint64_t id) const noexcept;
};
} }
