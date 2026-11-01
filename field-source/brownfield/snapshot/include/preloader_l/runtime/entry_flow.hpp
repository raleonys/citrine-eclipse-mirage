#pragma once
#include <stdint.h>
namespace preloader_l { namespace runtime {
using EntryCallback = int (*)(void* module, uint32_t reason, void* reserved, void* context) noexcept;
// Clean source form of curio RVA 0x4EB900..0x4EB94C ordering.
int run_curio_entry_flow(void* module, uint32_t reason, void* reserved,
                             EntryCallback active, EntryCallback secondary,
                             void* context) noexcept;
} }
