#include "composed_write_native.hpp"
namespace anadius::socket_recovered {
AsyncSendResult NativeAsyncSendApi::send(Socket socket,Buffer& buffer,std::uint32_t flags,OVERLAPPED& overlapped){
    if(!imports_.send || !imports_.get_error)throw UnsupportedInput("Async send imports require both SDK functions");
    DWORD transferred=0;
    const auto result=imports_.send(static_cast<SOCKET>(socket),&buffer,1,&transferred,flags,&overlapped,nullptr);
    const auto error=imports_.get_error(); // unconditional, immediately after WSASend
    return {result,static_cast<std::uint32_t>(error),static_cast<std::uint32_t>(transferred)};
}
} // namespace anadius::socket_recovered
