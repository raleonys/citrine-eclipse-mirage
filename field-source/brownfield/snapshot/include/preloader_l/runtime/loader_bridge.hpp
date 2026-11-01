#pragma once
#include <stdint.h>

namespace preloader_l { namespace runtime {

constexpr uint32_t loader_process_detach = 0u;
constexpr uint32_t loader_process_attach = 1u;
constexpr uint32_t loader_thread_attach = 2u;
constexpr uint32_t loader_thread_detach = 3u;

// Loader-lock-safe bridge used by DllMain. It only updates process-local state.
void handle_loader_event(void* module, uint32_t reason, void* reserved = nullptr) noexcept;
void handle_surge_entry() noexcept;

} }
