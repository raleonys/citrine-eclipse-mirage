#include "flow.hpp"
#include <algorithm>
#include <array>
#include <bit>

namespace anadius::socket_recovered {
namespace {
FlowServer& server(std::uintptr_t id,FlowContext& c){
    FlowServer* result=nullptr;bool found=false;
    for(const auto& b:c.servers)if(b.identity==id){if(found)throw UnsupportedInput("Ambiguous live flow server");found=true;result=b.server;}
    if(!result)throw UnsupportedInput("Flow server has no live binding");return *result;
}
WriteFinishCategory& category(const Category& value,FlowContext& c){
    WriteFinishCategory* result=nullptr;bool found=false;
    for(const auto& b:c.categories)if(b.key.name_identity==value.name_identity&&b.key.logical==value.logical){
        if(found)throw UnsupportedInput("Ambiguous live flow error category");found=true;result=b.category;
    }
    if(!result)throw UnsupportedInput("Flow error category has no live projection");return *result;
}
std::uintptr_t data(const ReceiveBufferString& value){return receive_buffer_address(value);}
void validate_data(const ReceiveBufferString& value,std::uintptr_t address,std::uint64_t bytes,FlowContext& c){
    if(value.capacity_18<=15){if(bytes>15)throw UnsupportedInput("Local flow string leaves its inline prefix");}
    else if(bytes)(void)c.strings.memory.view(address,bytes);
}
void destroy_string(ReceiveBufferString& value,FlowContext& c){
    const auto capacity=value.capacity_18;if(capacity<=15)return;
    auto bytes=capacity+std::uint64_t{1};auto pointer=data(value);
    if(bytes>=4096){
        bytes+=0x27;const auto header=c.strings.memory.view(pointer-std::uintptr_t{8},8);std::uintptr_t base=0;
        for(unsigned i=0;i<8;++i)base|=std::uintptr_t{header[i]}<<(8*i);
        if(pointer-base-std::uintptr_t{8}>31)c.logging.invalid_parameter_05c974();pointer=base;
    }
    c.logging.free_sized_04b0f8(pointer,bytes);
}
void release_local(IoReference* control){
    if(control&&control->strong_08.fetch_sub(1)==1){control->dispose_slot_00();if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();}
}
}
void copy_session_cipher_defined_fields(SessionCipher& destination,const SessionCipher& source){
    destination.schedule_=source.schedule_; // value copy; no vptr memcpy
    destination.native_.mode_00=source.native_.mode_00;
    destination.native_.iv_b4=source.native_.iv_b4; // destination.blocks stays bound to OWN schedule_
    destination.encrypted_c4=source.encrypted_c4;destination.unknown_c5=source.unknown_c5;
}
void rearm_accept_0270e0(FlowServer& value,FlowContext& c){
    const auto local_server=value.identity;
    c.api.start_accept(value.service,value.listener,value.pending,nullptr,local_server,value.identity+std::uintptr_t{0x68});
}
void accept_session_025720(AcceptHandler& original,Error& incoming,FlowContext& c){
    const auto code=incoming.value;
    if(code==0){
        SessionPair local;auto& source=server(original.server_this,c);
        auto* const object=c.api.construct(local,source.pending.object).object;
        if(!object)throw UnsupportedInput("Accepted factory returned no live session object");
        SessionCipher temporary;reset_session_cipher_02bc70(temporary);
        copy_session_cipher_defined_fields(object->cipher,temporary);
        ReceiveBufferString greeting;auto& returned=c.api.greeting(*object,greeting);
        const auto total=returned.size_10;const auto pointer=data(returned);validate_data(returned,pointer,total,c);
        Error error{0,c.success_category_a10f8};std::uint64_t sent=0;
        if(total)do{
            if(error.value)c.api.raise_write(error,0x180084720ULL);
            const auto offset=std::min(sent,total);const auto length=std::min<std::uint64_t>(total-offset,0x10000);
            std::array<Buffer,1> buffers{{{static_cast<std::uint32_t>(length),reinterpret_cast<char*>(pointer+offset)}}};
            const auto count=send_02a200(object->socket_state.socket,object->socket_state.socket_state,buffers,0,length==0,error,c.sockets);
            sent+=static_cast<std::uint64_t>(count);
        }while(sent<total);
        if(error.value)c.api.raise_write(error,0x180084720ULL);
        destroy_string(greeting,c); // LOCAL output, live after all send callbacks
        start_frame_read_0280d0(object->identity(),c.frame);
        auto* const control=local.control; // native load AFTER initial read
        release_local(control); // callback may end object/control lifetime
    }else{
        auto& first=c.logging.begin_log_006880(0x1800a4130ULL,0x1800845f8ULL);
        ReceiveBufferString message;
        category(incoming.category,c).message_slot_10(message,code); // live category AFTER begin
        const auto address=data(message);const auto length=message.size_10;validate_data(message,address,length,c);
        auto& stream=c.logging.append_007990(first,address,length);
        const auto adjusted=stream.identity+static_cast<std::uintptr_t>(static_cast<std::intptr_t>(stream.virtual_base_offset_04));
        const auto character=c.logging.widen_01e510(adjusted,10);
        c.logging.put_00fe90(stream,static_cast<std::uint8_t>(character));c.logging.flush_00e1e0(stream);
        destroy_string(message,c);
    }
    rearm_accept_0270e0(server(original.server_this,c),c); // LIVE original handler after callbacks
}
void invoke_accept_handler_021a10(AcceptHandler& original,FlowContext& c){
    auto local=original.error;accept_session_025720(original,local,c);
}
} // namespace anadius::socket_recovered
