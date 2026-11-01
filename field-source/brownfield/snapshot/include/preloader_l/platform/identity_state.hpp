#pragma once
#include <stdint.h>

namespace preloader_l { namespace platform {

struct ReconstructedIdentityState {
    uint32_t schema_version;
    uint32_t initialized;
    uint32_t branch_hits;
    const char* target_module_ascii;
    const char* companion_module_ascii;
    const char* identity_ascii;
};

// Clean-room model of the identity/selection state recovered from the compact
// reconstructed oracle. This state is diagnostic/integration metadata only;
// it never redirects certificate/trust verification and never fabricates a
// trust result.
void initialize_reconstructed_identity_state() noexcept;
void note_reconstructed_identity_branch() noexcept;
ReconstructedIdentityState reconstructed_identity_state() noexcept;
void reset_reconstructed_identity_state_for_test() noexcept;

} }
