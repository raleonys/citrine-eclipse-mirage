#pragma once
#include "preloader_l/net/transport_acceptor.hpp"

namespace preloader_l::socket_recovered {
// Raw closesocket result, not a synthesized success/failure boolean. INVALID
// socket returns0 without reading/writing state or error. Success clears only
// error.value; category identity remains whatever the caller/native path held.
std::int32_t close_026e50(Socket,std::uint8_t& state,bool destroying,
    Error&,Api& errors,AcceptorApi&);
} // namespace preloader_l::socket_recovered
