// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_response_hex.hpp"

namespace preloader_l::socket {
void encode_response_hex_03d190(ReceiveBufferString& value,ReceiveBufferContext& context){
    const auto count=value.size_10;if(!count)return;
    const auto doubled=count+count;
    resize_receive_buffer_010040(value,doubled,0,context);
    auto destination=doubled-std::uint64_t{2};
    for(auto remaining=count;remaining;--remaining,destination-=std::uint64_t{2}){
        const auto byte=context.memory.view(receive_buffer_address(value)+static_cast<std::uintptr_t>(remaining-1),1)[0];
        const auto low=static_cast<std::uint8_t>(byte&15),high=static_cast<std::uint8_t>(byte>>4);
        context.memory.view(receive_buffer_address(value)+static_cast<std::uintptr_t>(destination+1),1)[0]=static_cast<std::uint8_t>(low+(low<10?0x30:0x57));
        context.memory.view(receive_buffer_address(value)+static_cast<std::uintptr_t>(destination),1)[0]=static_cast<std::uint8_t>(high+(high<10?0x30:0x57));
    }
}
} // namespace preloader_l::socket
