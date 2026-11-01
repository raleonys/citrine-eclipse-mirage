// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_receive_buffer.hpp"

namespace preloader_l::socket {
class RequestMutationApi {
public:
    virtual ~RequestMutationApi()=default;
    [[noreturn]]virtual void out_of_range_00cec0()=0;
    // Mandatory lower growth/reallocation body. Native03489f forwards these
    // exact defined arguments; return identity may differ from the input.
    virtual ReceiveBufferString& grow_replace_02b940(ReceiveBufferString&,
        std::uint64_t extra,std::uint64_t unused_hint,std::uint64_t offset,
        std::uint64_t erase_count,std::uintptr_t source,std::uint64_t insert_count)=0;
};
struct RequestMutationContext {ReceiveBufferContext& buffer;RequestMutationApi& api;};
void decode_request_hex_03ce30(ReceiveBufferString&,ReceiveBufferContext&);
ReceiveBufferString& replace_request_034730(ReceiveBufferString&,std::uint64_t offset,
    std::uint64_t erase_count,std::uintptr_t source,std::uint64_t insert_count,RequestMutationContext&);
} // namespace preloader_l::socket
