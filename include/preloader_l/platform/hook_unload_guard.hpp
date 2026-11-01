// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
namespace preloader_l {
#ifdef PRELOADER_L_SHUTDOWN_TRACE
// Opt-in CI diagnostics; no allocation, CRT streams, or changes to acceptance.
void trace_shutdown_failure(const char*) noexcept;
#else
inline void trace_shutdown_failure(const char*) noexcept {}
#endif
void set_shutdown_caller(std::uint32_t) noexcept;
// False also retains incomplete OS cleanup. Retry before unloading the image;
// successful resume is never repeated merely because handle close failed.
bool external_threads_outside_module() noexcept;
}
