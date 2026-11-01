#pragma once
#include <cstddef>

namespace preloader_l { namespace runtime {

struct AuthCodeValueView {
    const char* data;
    size_t size;
};

// Transactional lease used by serializers that must not commit session state
// during size measurement or a failed render. begin_* holds the session lock
// until finish_* is called. The staged value is always 8 alphanumeric bytes.
struct AuthCodeSessionLease {
    char value[9];
    bool lock_held;
    bool commit_pending;
};

// Session lifecycle reset. The shipping GetAuthCode producer is the local
// transactional Auth8 lease below; no secondary callback producer is retained.
void clear_auth_code_service() noexcept;

// Minimal local Auth8 adaptation. The value is exactly 8 alphanumeric ASCII
// characters [0-9A-Za-z], is generated once per live release-service session,
// stays stable for that session, and is reset on service shutdown. It is
// independent of request ClientId, is not loaded from Config2, and does not
// participate in Online policy. Session token access is lease-only so no
// caller can retain a view into mutable global storage across reset.

// Side-effect-free-until-commit serialization boundary. A successful begin
// acquires the session lock and stages either the existing token or a fresh
// candidate in lease->value. finish(..., true) commits a fresh candidate;
// finish(..., false) discards it. Every successful begin must be finished.
bool begin_session_auth_code_lease(AuthCodeSessionLease* lease) noexcept;
AuthCodeValueView session_auth_code_lease_value(const AuthCodeSessionLease* lease) noexcept;
void finish_session_auth_code_lease(AuthCodeSessionLease* lease, bool commit) noexcept;

// Read-only lifecycle probe used by deterministic regression tests.
bool session_auth_code_ready() noexcept;

} }
