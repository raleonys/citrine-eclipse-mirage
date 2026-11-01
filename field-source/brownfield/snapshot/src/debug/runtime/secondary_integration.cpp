#include "preloader_l/runtime/secondary_integration.hpp"

namespace preloader_l { namespace runtime {
namespace {
uint32_t g_attach_calls = 0;
uint32_t g_crypt_install_status = 0;
uint32_t g_bootstrap_scheduled = 0;
}

void reset_secondary_integration_for_test() noexcept {
    g_attach_calls = 0;
    g_crypt_install_status = 0;
    g_bootstrap_scheduled = 0;
}

SecondaryIntegrationStatus secondary_integration_status() noexcept {
    return SecondaryIntegrationStatus{g_attach_calls, g_crypt_install_status, g_bootstrap_scheduled};
}

// Internal setters are intentionally translation-unit local to the Windows
// bridge through these narrow functions.
void secondary_note_attach(uint32_t crypt_status, bool bootstrap) noexcept {
    ++g_attach_calls;
    g_crypt_install_status = crypt_status;
    g_bootstrap_scheduled = bootstrap ? 1u : 0u;
}

} }
