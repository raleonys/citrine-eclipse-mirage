#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

struct SecondaryIntegrationStatus {
    uint32_t attach_calls;
    uint32_t crypt_install_status;
    uint32_t bootstrap_scheduled;
};

void reset_secondary_integration_for_test() noexcept;
SecondaryIntegrationStatus secondary_integration_status() noexcept;

#if defined(_WIN32)
// Clean-room equivalent of the model binary's secondary 0x4E8800 entry.
// On PROCESS_ATTACH this performs only lightweight integration setup and then
// schedules the heavy runtime worker outside loader lock.
bool secondary_integration_entry(void* module, uint32_t reason, void* reserved) noexcept;
#endif

} }
