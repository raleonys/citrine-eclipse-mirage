#include "message_core_bridge.hpp"
#include <algorithm>

namespace anadius::socket_recovered {
std::string BoundedMessageCoreStrings::read_request(const ReceiveBufferString& value){
    const auto count=value.size_10;
    if(value.capacity_18<=15){
        if(count>value.storage_00.size())throw UnsupportedInput("Message inline request leaves bounded prefix");
        return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(count)};
    }
    const auto bytes=context_.memory.view(receive_buffer_address(value),count);
    if(bytes.empty())return {};
    return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
}
void BoundedMessageCoreStrings::replace_request(ReceiveBufferString& value,std::string_view bytes){
    // Actual existing010040/008350 source supplies shrink/grow/NUL semantics.
    // Context must own the source allocation and map the live inline prefix.
    resize_receive_buffer_010040(value,bytes.size(),0,context_);
    if(bytes.empty())return;
    const auto destination=context_.memory.view(receive_buffer_address(value),bytes.size());
    std::copy(bytes.begin(),bytes.end(),destination.begin());
}
void process_message_core(std::uintptr_t identity,ReceiveBufferString& response,ReceiveBufferString& request,MessageCoreContext& context){
    if(&response==&request)throw UnsupportedInput("Message bridge requires distinct request and response prefixes");
    MessageCoreSession* found=nullptr;bool matched=false;
    for(const auto& item:context.handlers){
        if(item.handler_f0!=identity)continue;
        if(matched)throw UnsupportedInput("Message handler identity is ambiguous");
        matched=true;found=item.session;
    }
    if(!identity || !found)throw UnsupportedInput("Message handler identity/state is unbound");
    const auto session=*found; // reference fields copied before string provider
    const auto original=context.strings.read_request(request);auto working=original;
    const auto result=messages_recovered::process_message_033d70(session.state,working,
        session.xml,session.factory,session.random,session.effects);
    if(working!=original)context.strings.replace_request(request,working);
    context.strings.construct_response(response,result);
}
void CompatibleMessageCoreApi::process_message_033d70(std::uintptr_t identity,ReceiveBufferString& response,ReceiveBufferString& request){
    process_message_core(identity,response,request,core_);
}
} // namespace anadius::socket_recovered
