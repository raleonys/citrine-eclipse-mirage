// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_prefix_cipher.hpp"
#include <algorithm>

namespace preloader_l::socket {
void PrefixCipherBlocks::decrypt_ecb_045620(PrefixCipherState& state,std::uintptr_t address){
    state.schedule.decrypt_block_0458d0(std::span<std::uint8_t,16>(memory_.view(address,16).data(),16));
}
void PrefixCipherBlocks::encrypt_ecb_045630(PrefixCipherState& state,std::uintptr_t address){
    state.schedule.encrypt_block_045670(std::span<std::uint8_t,16>(memory_.view(address,16).data(),16));
}
void PrefixCipherBlocks::decrypt_cbc_0454e0(PrefixCipherState& state,std::uintptr_t address,std::uint64_t count){
    if(!count)return;auto remaining=((count-1)>>4)+1;
    do{
        const auto bytes=memory_.view(address,16);crypto::Key original;std::copy(bytes.begin(),bytes.end(),original.begin());
        state.schedule.decrypt_block_0458d0(std::span<std::uint8_t,16>(bytes.data(),16));
        for(unsigned i=0;i<16;++i)bytes[i]^=state.iv_b4[i];
        state.iv_b4=original;address+=16;
    }while(--remaining);
}
void PrefixCipherBlocks::encrypt_cbc_045580(PrefixCipherState& state,std::uintptr_t address,std::uint64_t count){
    auto previous=state.iv_b4;
    if(count){auto remaining=((count-1)>>4)+1;do{
        const auto bytes=memory_.view(address,16);
        for(unsigned i=0;i<16;++i)bytes[i]^=previous[i];
        state.schedule.encrypt_block_045670(std::span<std::uint8_t,16>(bytes.data(),16));
        std::copy(bytes.begin(),bytes.end(),previous.begin());address+=16;
    }while(--remaining);}
    state.iv_b4=previous; // one final IV store after whole batch, including count0 self-copy
}
bool decrypt_prefix_042480(PrefixCipherState& state,ReceiveBufferString& value,std::uint64_t offset,PrefixCipherContext& context){
    const auto mode=state.mode_00;if(!mode)return false;
    auto size=value.size_10;if(((size-offset)&15)!=0)return false;
    if(mode==2){
        while(offset<size){context.blocks.decrypt_ecb_045620(state,receive_buffer_address(value)+offset);size=value.size_10;offset+=16;}
    }else if(mode==1){context.blocks.decrypt_cbc_0454e0(state,receive_buffer_address(value)+offset,size-offset);size=value.size_10;}
    else return false;
    const auto padding=context.strings.memory.view(receive_buffer_address(value)+size-1,1)[0];
    if(padding<1||padding>16)return false;
    auto cursor=receive_buffer_address(value)+size-padding;
    const auto end=receive_buffer_address(value)+size;
    for(;cursor!=end;++cursor)if(context.strings.memory.view(cursor,1)[0]!=padding)return false;
    // Native inlined resize has no effects between cached-cap load and this
    // branch. Reuse010040/008350 normal source; no native CALL010040 claimed.
    resize_receive_buffer_010040(value,value.size_10-padding,0,context.strings);return true;
}
bool encrypt_prefix_042610(PrefixCipherState& state,ReceiveBufferString& value,std::uint64_t offset,PrefixCipherContext& context){
    if(!state.mode_00)return false;
    const auto padding=std::uint64_t{16}-((value.size_10-offset)&15);
    resize_receive_buffer_010040(value,value.size_10+padding,static_cast<std::uint8_t>(padding),context.strings);
    const auto mode=state.mode_00; // live after fill/growth provider effects
    if(mode==2){while(offset<value.size_10){context.blocks.encrypt_ecb_045630(state,receive_buffer_address(value)+offset);offset+=16;}return true;}
    if(mode!=1)return false;
    context.blocks.encrypt_cbc_045580(state,receive_buffer_address(value)+offset,value.size_10-offset);return true;
}
namespace {
PrefixCipherState& bound(std::uintptr_t id,std::span<PrefixCipherBinding> bindings){
    PrefixCipherState* found=nullptr;bool matched=false;
    for(const auto& binding:bindings)if(binding.handler_f0==id){if(matched)throw UnsupportedInput("Cipher handler identity is ambiguous");matched=true;found=binding.state;}
    if(!id||!found)throw UnsupportedInput("Cipher handler state is unbound");return *found;
}
}
bool MessageCipherApi::decrypt_042480(std::uintptr_t id,ReceiveBufferString& value,std::uint32_t offset){return decrypt_prefix_042480(bound(id,bindings_),value,offset,context_);}
bool MessageCipherApi::encrypt_042610(std::uintptr_t id,ReceiveBufferString& value,std::uint32_t offset){return encrypt_prefix_042610(bound(id,bindings_),value,offset,context_);}
} // namespace preloader_l::socket
