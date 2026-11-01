#include "preloader_l/platform/identity_state.hpp"
#include "preloader_l/platform/crypt_path_restore.hpp"
#include "identity_constants.hpp"

namespace preloader_l { namespace platform {
namespace {
uint32_t g_initialized = 0;
uint32_t g_branch_hits = 0;
}

void initialize_reconstructed_identity_state() noexcept {
    g_initialized = 1;
}

void note_reconstructed_identity_branch() noexcept {
    if (!g_initialized) g_initialized = 1;
    ++g_branch_hits;
}

ReconstructedIdentityState reconstructed_identity_state() noexcept {
    return ReconstructedIdentityState{
        1u,
        g_initialized,
        g_branch_hits,
        crypt_query_restore_module_ascii,
        reconstructed_alt_module_ascii,
        reconstructed_identity_ascii,
    };
}

void reset_reconstructed_identity_state_for_test() noexcept {
    g_initialized = 0;
    g_branch_hits = 0;
}

} }
