// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

// Behavioral-visible UTC formatter used by GetAllGameInfo and achievement grant
// state. Output is exactly YYYY-MM-DDTHH:MM:SS plus NUL (20 bytes total).
bool current_utc_timestamp(char* output, size_t capacity, size_t* written) noexcept;
uint64_t current_unix_seconds() noexcept;

} }
