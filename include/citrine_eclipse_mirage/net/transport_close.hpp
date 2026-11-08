// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_acceptor.hpp"

namespace citrine_eclipse_mirage::socket {
// Raw closesocket result, not a synthesized success/failure boolean. INVALID
// socket returns0 without reading/writing state or error. Success clears only
// error.value; category identity remains whatever the caller/native path held.
std::int32_t close_026e50(Socket,std::uint8_t& state,bool destroying,
    Error&,Api& errors,AcceptorApi&);
} // namespace citrine_eclipse_mirage::socket
