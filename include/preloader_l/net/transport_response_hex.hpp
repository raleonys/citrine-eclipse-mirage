// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_receive_buffer.hpp"

namespace preloader_l::socket {
// Normal03d190 typed-prefix body. Native inlined resize semantics are composed
// with existing010040/008350 source; native03d190 does not CALL010040 directly.
// Empty input is an exact no-op. Unsigned doubled-size wrap is not replaced by
// a new guard; bounded memory supplies explicit projection failure boundaries.
void encode_response_hex_03d190(ReceiveBufferString&,ReceiveBufferContext&);
} // namespace preloader_l::socket
