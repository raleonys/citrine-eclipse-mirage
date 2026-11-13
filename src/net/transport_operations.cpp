// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_operations.hpp"
#include <algorithm>
#include <bit>
#include <limits>

namespace citrine_eclipse_mirage::socket {
std::int32_t address_028690(std::int32_t family,std::string_view raw,
    std::array<std::uint8_t,16>& output,std::uint32_t* scope,Error& error,Api& api) {
    api.set_last_error(0);
    if(family!=2 && family!=23){error={10047,api.system_category_028620()};return -1;}
    const auto prefix=raw.substr(0,raw.find('\0'));
    if(prefix.size()>=static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
        throw UnsupportedInput("Address C string length exceeds modeled signed capacity");
    const std::string text(prefix);
    auto wide=api.convert_acp(text.c_str(),static_cast<std::int32_t>(prefix.size()+1));
    // The original ignores MultiByteToWideChar's return and still calls WSA.
    // A native adapter may reject an undefined/uninitialized wide input domain.
    const auto parsed=api.string_to_address(wide,family,128);
    const auto category=api.system_category_028620();
    error={api.get_last_error(),category};
    if(parsed.result!=-1){
        std::copy_n(parsed.bytes.begin(),family==2?4:16,output.begin());
        if(family==23 && scope)*scope=parsed.scope;
        error.value=0;
        return 1;
    }
    if(family==2 && text=="255.255.255.255"){
        std::fill_n(output.begin(),4,std::uint8_t{0xff});
        error.value=0;
        // Native0287d5 jumps into the10022 branch even after writing broadcast.
        error={10022,api.system_category_028620()};
    }else if(error.value==0)error={10022,api.system_category_028620()};
    return -1;
}
std::int64_t send_02a200(Socket socket,std::uint8_t state,std::span<Buffer> buffers,
    std::uint32_t flags,bool all_empty,Error& error,Api& api) {
    if(socket==std::numeric_limits<Socket>::max()){
        error={10009,api.system_category_028620()};return 0;
    }
    if(all_empty && (state&0x10)!=0){error.value=0;return 0;}
    for(;;){
        const auto sent=api.wsa_send(socket,buffers,flags);
        const auto category=api.system_category_028620();
        error={api.get_last_error(),category};
        if(error.value==64)error={10054,api.system_category_028620()};
        else if(error.value==1234)error={10061,api.system_category_028620()};
        if(sent.result==0){
            error.value=0;
            const auto bytes=std::bit_cast<std::int32_t>(sent.transferred);
            if(bytes>=0)return bytes;
        }
        if((state&1)!=0)return 0;
        const auto first=api.system_category_028620();
        if(!(error.category.name_identity==first.name_identity && error.value==10035)){
            const auto second=api.system_category_028620();
            if(error.category.name_identity!=second.name_identity || error.value!=1237)return 0;
        }
        const auto nfds=std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(socket)+1U);
        if(api.select_write(socket,nfds)<0){
            const auto selected_category=api.system_category_028620();
            error={api.get_last_error(),selected_category};return 0;
        }
        // select0 also retries. Native has no invented timeout/retry limit.
        error.value=0;
    }
}
server::ParsedAddress AddressBackend::parse_address_028690(std::uint16_t family,std::string_view raw) {
    server::ParsedAddress result;Error error;
    result.conversion_result=address_028690(family,raw,result.bytes,&result.scope_id,error,api_);
    result.error={error.value,error.category.logical};return result;
}
server::WriteResult SynchronousTransport::synchronous_send(std::span<const char> bytes) {
    if(bytes.size()>std::numeric_limits<ULONG>::max())
        throw UnsupportedInput("Synchronous buffer exceeds DWORD byte length");
    Buffer buffer{static_cast<ULONG>(bytes.size()),const_cast<char*>(bytes.data())};
    Error error;
    const auto count=send_02a200(socket_,state_,std::span<Buffer>(&buffer,1),0,bytes.empty(),error,api_);
    return {{error.value,error.category.logical},static_cast<std::size_t>(count)};
}
} // namespace citrine_eclipse_mirage::socket
