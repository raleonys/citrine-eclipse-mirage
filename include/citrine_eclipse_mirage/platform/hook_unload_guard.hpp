// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
namespace citrine_eclipse_mirage {
void set_shutdown_caller(std::uint32_t) noexcept;
// False also retains incomplete OS cleanup. Retry before unloading the image;
// successful resume is never repeated merely because handle close failed.
bool external_threads_outside_module() noexcept;
}
