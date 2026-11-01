// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
namespace preloader_l {
void set_shutdown_caller(std::uint32_t) noexcept;
bool external_threads_outside_module() noexcept;
}
