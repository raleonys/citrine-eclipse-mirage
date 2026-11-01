// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_native.hpp"
#include <cstring>
#include <limits>
#include <memory>

namespace preloader_l::socket {
static_assert(sizeof(Socket)==8 && sizeof(SOCKET)==8);
static_assert(sizeof(wchar_t)==2 && sizeof(DWORD)==4);
static_assert(AF_INET==2 && AF_INET6==23 && sizeof(sockaddr_storage)==128);
static_assert(sizeof(sockaddr_in)==16 && offsetof(sockaddr_in,sin_addr)==4);
static_assert(sizeof(sockaddr_in6)==28 && offsetof(sockaddr_in6,sin6_addr)==8 &&
              offsetof(sockaddr_in6,sin6_scope_id)==24);
void NativeApi::set_last_error(std::int32_t value){imports_.set_last_error(value);}
Category NativeApi::system_category_028620(){return categories_.get_028620();}
std::int32_t NativeApi::get_last_error(){return imports_.get_last_error();}
Conversion NativeApi::convert_acp(const char* text,std::int32_t capacity) {
    if(capacity<=0)throw UnsupportedInput("Wide address capacity must be positive");
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[static_cast<std::size_t>(capacity)]);
    const auto result=imports_.convert(0,0,text,-1,buffer.get(),capacity);
    Conversion converted;converted.result=result;
    if(result>0 && result<=capacity && buffer[result-1]==L'\0')
        converted.text.emplace(buffer.get(),static_cast<std::size_t>(result-1));
    return converted;
}
AddressCall NativeApi::string_to_address(Conversion& wide,std::int32_t family,std::int32_t initial_size) {
    if(!wide.text)throw UnsupportedInput("Original address conversion would read undefined wide storage");
    if(initial_size!=128 || (family!=AF_INET && family!=AF_INET6))
        throw UnsupportedInput("Typed address call requires modeled family and128byte storage");
    sockaddr_storage storage{};int size=initial_size;
    AddressCall call;call.result=imports_.parse(wide.text->data(),family,nullptr,
        reinterpret_cast<sockaddr*>(&storage),&size);
    if(call.result!=-1){
        const auto needed=family==AF_INET?sizeof(sockaddr_in):sizeof(sockaddr_in6);
        if(size<static_cast<int>(needed) || size>128)
            throw UnsupportedInput("Successful address API output outside declared initialized range");
        // Copy actual SDK structures from storage; no incompatible C++ alias.
        if(family==AF_INET){sockaddr_in value;std::memcpy(&value,&storage,sizeof(value));
            std::memcpy(call.bytes.data(),&value.sin_addr,4);
        }else{sockaddr_in6 value;std::memcpy(&value,&storage,sizeof(value));
            std::memcpy(call.bytes.data(),&value.sin6_addr,16);call.scope=value.sin6_scope_id;
        }
    }
    return call;
}
SendCall NativeApi::wsa_send(Socket socket,std::span<Buffer> buffers,std::uint32_t flags) {
    if(buffers.size()>std::numeric_limits<DWORD>::max())
        throw UnsupportedInput("WSABUF count exceeds modeled DWORD domain");
    DWORD bytes=0;
    const auto result=imports_.send(static_cast<SOCKET>(socket),buffers.data(),
        static_cast<DWORD>(buffers.size()),&bytes,flags,nullptr,nullptr);
    return {result,bytes};
}
std::int32_t NativeApi::select_write(Socket socket,std::int32_t nfds) {
    fd_set write{};write.fd_count=1;write.fd_array[0]=static_cast<SOCKET>(socket);
    return imports_.select_write(nfds,nullptr,&write,nullptr,nullptr);
}
} // namespace preloader_l::socket
