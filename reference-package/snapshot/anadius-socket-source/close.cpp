#include "close.hpp"

namespace anadius::socket_recovered {
namespace {
Error capture(Api& errors){
    const auto category=errors.system_category_028620();
    return {errors.get_last_error(),category};
}
}
std::int32_t close_026e50(Socket socket,std::uint8_t& state,bool destroying,
    Error& error,Api& errors,AcceptorApi& api){
    if(socket==invalid_socket)return 0;
    if(destroying && (state&8)!=0){
        state=static_cast<std::uint8_t>(state|8);
        if(api.set_option(socket,65535,128,0)!=0){
            (void)errors.system_category_028620();
            (void)errors.get_last_error();
        }
    }
    auto result=api.close_socket(socket);
    if(result==0){error.value=0;return result;}
    error=capture(errors);
    const auto first=errors.system_category_028620();
    if(!(error.category.name_identity==first.name_identity && error.value==10035)){
        const auto second=errors.system_category_028620();
        if(error.category.name_identity!=second.name_identity || error.value!=1237)return result;
    }
    std::uint32_t nonblocking=0;
    (void)api.set_nonblocking(socket,0x8004667eU,nonblocking);
    state=static_cast<std::uint8_t>(state&0xfc);
    result=api.close_socket(socket);
    if(result==0)error.value=0;
    else error=capture(errors);
    return result;
}
} // namespace anadius::socket_recovered
