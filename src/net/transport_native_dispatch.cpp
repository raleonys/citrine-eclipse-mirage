// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_native_dispatch.hpp"

namespace preloader_l::socket {
namespace {
std::string_view c_string(std::string_view value){const auto at=value.find('\0');return at==std::string_view::npos?value:value.substr(0,at);}
void free_name(ReceiveBufferString& value,ReceiveBufferContext& context){
    const auto capacity=value.capacity_18;if(capacity<=15)return;
    auto data=receive_buffer_address(value),count=capacity+std::uint64_t{1};
    if(count>=4096){std::uintptr_t raw=0;const auto header=context.memory.view(data-8,8);for(unsigned i=0;i<8;++i)raw|=std::uintptr_t{header[i]}<<(i*8);count+=39;if(data-raw-8>31)context.api.invalid_parameter_05c974();data=raw;}
    context.api.free_sized_04b0f8(data,count); // native leaves dead prefix metadata intact
}
}
ReceiveBufferString& dispatch_native_032d00(ReceiveBufferString& output,std::uintptr_t handler,
    NativeMessageDocument& document,NativeDispatchContext& context){
    auto& effects=context.effects;auto& scratch=context.locals.scratch;
    if(&output==&scratch)throw UnsupportedInput("Dispatch output aliases automatic scratch projection");
    effects.prepare_registry(messages::registrations_032d00());
    const auto& parsed=effects.document(document);bool invalid=true,constructed_name=false;
    const messages::XmlNode* command=nullptr;NativeDispatchEntry* selected=nullptr;
    std::optional<std::string_view> id,recipient;
    const auto* root=parsed.first_element();
    if(root&&c_string(root->name())=="LSX"){
        const auto* request=root->first_element();
        if(request&&c_string(request->name())=="Request"&&!request->next_element()){
            id=request->attribute("id");
            if(id){recipient=request->attribute("recipient");if(recipient){
                command=request->first_element();
                if(command){
                    const auto command_name=command->name();
                    scratch.storage_00.fill(0);scratch.size_10=0;scratch.capacity_18=0;
                    effects.construct_name_006b30(scratch,c_string(command_name));constructed_name=true;
                    selected=effects.find_exact(scratch);
                    invalid=selected==effects.registry_end()&&c_string(command->name())!="ChallengeResponse";
                }
            }}
        }
    }
    if(constructed_name)free_name(scratch,context.strings);
    if(invalid)effects.invalid_message_02bd30_04d228();
    auto& printer=effects.construct_printer_0019b0(0,0,0);
    printer.open("LSX");printer.open("Response");printer.attribute("id",c_string(*id));printer.attribute("sender",c_string(*recipient));
    // Native compares against the live sentinel after printer callbacks and
    // loads the selected function at+40 only when invoking it.
    bool accepted=false;
    if(selected==effects.registry_end())accepted=effects.challenge_02d7e0(handler,*command,printer);
    else{if(!selected)throw UnsupportedInput("Dispatch selected handler is unbound");accepted=effects.registered_handler(selected->handler_rva,handler,*command,printer);}
    if(!accepted)effects.invalid_message_02bd30_04d228();
    printer.close();printer.close();
    scratch.storage_00.fill(0);scratch.size_10=0;scratch.capacity_18=15;
    const auto count=static_cast<std::uint64_t>(static_cast<std::int64_t>(effects.printer_size(printer)));
    effects.reserve_02b300(scratch,count*std::uint64_t{2}+32);
    const auto append_count=static_cast<std::uint64_t>(static_cast<std::int64_t>(effects.printer_size(printer)))-1;
    effects.append_018b30(scratch,effects.printer_data(printer),append_count);
    (void)effects.construct_output_009fd0(output,effects.printer_data(printer));
    const auto capacity=scratch.capacity_18;
    if(capacity>15)effects.free_scratch_00b520(scratch,receive_buffer_address(scratch),capacity);
    effects.destroy_printer_001f60(printer);return output;
}
ReceiveBufferString& MessageDispatchApi::dispatch_032d00(ReceiveBufferString& output,std::uintptr_t handler,NativeMessageDocument& document){
    return dispatch_native_032d00(output,handler,document,dispatch_);
}
} // namespace preloader_l::socket
