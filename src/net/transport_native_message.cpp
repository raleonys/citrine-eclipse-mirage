// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_native_message.hpp"
#include <limits>
#include <string_view>

namespace preloader_l::socket {
namespace {
void reset(ReceiveBufferString& value){value.size_10=0;value.capacity_18=15;value.storage_00[0]=0;}
void destroy(ReceiveBufferString& value,ReceiveBufferContext& context){
    const auto capacity=value.capacity_18;if(capacity<=15)return;
    auto data=receive_buffer_address(value);auto count=capacity+std::uint64_t{1};
    if(count>=4096){
        std::uintptr_t raw=0;const auto header=context.memory.view(data-8,8);
        for(unsigned i=0;i<8;++i)raw|=std::uintptr_t{header[i]}<<(i*8);
        count+=39;if(data-raw-8>31)context.api.invalid_parameter_05c974();data=raw;
    }
    context.api.free_sized_04b0f8(data,count);
}
std::uint64_t find(const ReceiveBufferString& value,std::string_view pattern,ReceiveBufferMemory& memory){
    const auto count=value.size_10;if(count<pattern.size())return UINT64_MAX;
    const auto base=receive_buffer_address(value);
    for(std::uint64_t i=0;i<=count-pattern.size();++i){
        bool same=true;for(std::size_t j=0;j<pattern.size();++j)
            if(memory.view(base+i+j,1)[0]!=static_cast<std::uint8_t>(pattern[j])){same=false;break;}
        if(same)return i;
    }
    return UINT64_MAX;
}
void append_nul(ReceiveBufferString& value,NativeMessageContext& context){
    const auto count=value.size_10,capacity=value.capacity_18;
    if(count<capacity){
        value.size_10=count+1;
        // Native word store after size update, cached count and capacity.
        const auto bytes=context.strings.memory.view(receive_buffer_address(value)+count,2);bytes[0]=0;bytes[1]=0;
    }else context.api.grow_nul_012350(value,1,0,0);
}
ReceiveBufferString& front(NativeMessageEventQueue& queue){
    const auto slot=(queue.map_size_10-1)&queue.offset_18;
    if(slot>=queue.map_08.size()||!queue.map_08[slot])throw UnsupportedInput("Native message event front is unbound");
    return *queue.map_08[slot];
}
}
ReceiveBufferString& process_native_message_033d70(std::uintptr_t identity,ReceiveBufferString& output,
    ReceiveBufferString& request,NativeMessageContext& context){
    std::uint8_t* encrypted=nullptr;bool matched=false;
    for(const auto& binding:context.handlers)if(binding.handler_f0==identity){
        if(matched)throw UnsupportedInput("Native message handler identity is ambiguous");matched=true;encrypted=binding.encrypted_c4;
    }
    if(!identity||!encrypted)throw UnsupportedInput("Native message handler state is unbound");
    auto& local=context.locals;auto& response=local.response;auto& temporary=local.temporary;auto& document=local.document;
    if(&output==&request||&response==&request||&response==&output||&temporary==&request||&temporary==&output)
        throw UnsupportedInput("Native message input/output/locals must be distinct");
    context.api.construct_document_001830(document,true,0);
    response.storage_00.fill(0);reset(response);bool retried_version=false;
    for(;;){
        if(*encrypted){
            decode_request_hex_03ce30(request,context.strings);
            if(!context.api.decrypt_042480(identity,request,0)){
                context.api.log(NativeMessageLog::decryption_failure,document,request);break;
            }
        }
        const auto parsed=context.api.parse_document_004280(document,receive_buffer_address(request),UINT64_MAX);
        if(!document.error_6c){
            if(context.show_lsx_a3ef0)context.api.log(NativeMessageLog::received,document,request);
            auto& produced=context.api.dispatch_032d00(temporary,identity,document);
            if(&response!=&produced){destroy(response,context.strings);reset(response);response=produced;reset(produced);}
            destroy(temporary,context.strings);
            if(context.show_lsx_a3ef0)context.api.log(NativeMessageLog::response,document,response);
            if(!*encrypted){append_nul(response,context);break;}
            if(!context.api.encrypt_042610(identity,response,0)){
                output.storage_00.fill(0);reset(output);destroy(response,context.strings);
                reset(response);context.api.destroy_document_001e00(document);return output;
            }
            encode_response_hex_03d190(response,context.strings);append_nul(response,context);
            auto& queue=context.api.event_queue_02c400();
            if(queue.count_20){
                auto& mutex=context.api.event_mutex_02c620();
                if(context.api.lock_04a6ac(mutex)!=0)context.api.thread_error_04a648(5);
                if(mutex.lock_count_4c==std::numeric_limits<std::int32_t>::max()){
                    --mutex.lock_count_4c;context.api.thread_error_04a648(6);
                }
                if(queue.count_20){
                    copy_response_string_009eb0(temporary,front(queue),context.strings);
                    if(context.show_lsx_a3ef0)context.api.log(NativeMessageLog::event,document,temporary);
                    if(context.api.encrypt_042610(identity,temporary,0)){
                        encode_response_hex_03d190(temporary,context.strings);append_nul(temporary,context);
                        const auto count=temporary.size_10,source=receive_buffer_address(temporary);
                        const auto old=response.size_10,capacity=response.capacity_18;
                        if(count>capacity-old)context.api.grow_append_007e30(response,count,0,source,count);
                        else{
                            response.size_10=old+count;const auto tail=receive_buffer_address(response)+old;
                            context.strings.api.copy_bytes(tail,source,count);
                            context.strings.memory.view(tail+count,1)[0]=0;
                        }
                    }
                    // Reload live queue slot after callbacks, not the copied source.
                    auto& current=front(queue);destroy(current,context.strings);reset(current);
                    --queue.count_20;if(!queue.count_20)queue.offset_18=0;else ++queue.offset_18;
                    destroy(temporary,context.strings);
                }
                context.api.unlock_04a6b4(mutex);
            }
            break;
        }
        if(parsed==8){
            if(!*encrypted&&find(request,"<LSX>",context.strings.memory)==UINT64_MAX){*encrypted=1;continue;}
        }else if(parsed==7&&!retried_version){
            const auto at=find(request,"version=\"\"",context.strings.memory);
            if(at!=UINT64_MAX)replace_request_034730(request,at,10,0x180080097ULL,0,context.mutation);
            retried_version=true;continue;
        }
        context.api.log(NativeMessageLog::bad_xml,document,request);break;
    }
    output=response;reset(response);context.api.destroy_document_001e00(document);return output;
}
void NativeMessageApi::process_message_033d70(std::uintptr_t identity,ReceiveBufferString& output,ReceiveBufferString& request){
    process_native_message_033d70(identity,output,request,context_);
}
} // namespace preloader_l::socket
