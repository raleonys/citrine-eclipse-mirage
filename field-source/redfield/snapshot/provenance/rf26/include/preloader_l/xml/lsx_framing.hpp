// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace xml {

enum class FrameStatus : uint8_t {
    frame_ready = 0,
    need_more,
    invalid_prefix,
    invalid_argument,
};

struct FrameScanResult {
    FrameStatus status;
    size_t frame_length;
};

// Scan an accumulated plaintext receive buffer. A valid frame must begin at
// byte zero with "<LSX>" and ends at the first "</LSX>" outside a quoted tag
// value. Bytes after frame_length are intentionally left for the caller.
FrameScanResult scan_lsx_frame(const char* input, size_t length) noexcept;

} } // namespace preloader_l::xml
