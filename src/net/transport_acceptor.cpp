#include "preloader_l/net/transport_acceptor.hpp"
#include <bit>
#include <cstring>
#include <limits>
#include <ws2tcpip.h>

namespace preloader_l::socket_recovered {
namespace {
Error captured_wsa(Api& errors){
    const auto category=errors.system_category_028620();
    return {errors.get_last_error(),category};
}
std::uint16_t family(std::span<const std::byte> endpoint){
    if(endpoint.size()<sizeof(std::uint16_t))throw UnsupportedInput("Endpoint family is outside initialized storage");
    std::uint16_t result;std::memcpy(&result,endpoint.data(),sizeof(result));return result;
}
void reset_protocol(AcceptorState& state,std::int32_t selected){
    state.protocol_40=selected;state.byte_44=0;
    state.slot_48=2;state.slot_50=0;state.slot_58=0;state.slot_60=0;
}
Error invalid(Api& errors){return {10009,errors.system_category_028620()};}
}
void close_025310(const Socket& slot,Api& errors,AcceptorApi& api){
    const auto socket=slot;
    if(socket==invalid_socket || api.close_socket(socket)==0)return;
    const auto error=captured_wsa(errors);
    const auto first=errors.system_category_028620();
    if(!(error.category.name_identity==first.name_identity && error.value==10035)){
        const auto second=errors.system_category_028620();
        if(error.category.name_identity!=second.name_identity || error.value!=1237)return;
    }
    std::uint32_t nonblocking=0;
    (void)api.set_nonblocking(socket,0x8004667eU,nonblocking);
    if(api.close_socket(socket)!=0){
        (void)errors.system_category_028620();
        (void)errors.get_last_error();
    }
}
AcceptorState& acceptor_020e20(AcceptorState& state,std::span<const std::byte> endpoint,
    bool reuse,Api& errors,AcceptorApi& api,AcceptorRuntime& runtime){
    runtime.initialize_0211b0(state);
    const auto initial_family=family(endpoint);
    const std::int32_t selected=initial_family==2?2:23;
    auto* const service=state.service; // native caches service identity, not its port value
    if(state.socket!=invalid_socket){
        const auto category=runtime.miscellaneous_category_0285b0();
        runtime.raise_028240({1,category},"open");
    }
    const auto socket=api.create_socket(selected);
    if(socket==invalid_socket){
        const auto error=captured_wsa(errors);
        close_025310(socket,errors,api);
        if(error.value!=0)runtime.raise_028240(error,"open");
        // WSA failure plus error0 does not transfer or reset shared ownership.
        reset_protocol(state,selected);
    }else{
        if(initial_family!=2)(void)api.set_option(socket,41,27,0);
        if(!service)throw UnsupportedInput("Live service projection is required for IOCP attachment");
        if(api.attach_completion_port(socket,service->completion_port)==0){
            const auto value=api.get_win32_error(); // unlike WSA: GetLastError first
            const auto category=errors.system_category_028620();
            const Error error{std::bit_cast<std::int32_t>(value),category};
            if(value!=0){
                close_025310(socket,errors,api);
                runtime.raise_028240(error,"open");
            }
        }
        state.socket=socket;state.socket_state=0x10;
        runtime.reset_cancellation_0237f0(state);
        reset_protocol(state,selected);
    }
    if(reuse){
        if(state.socket==invalid_socket)runtime.raise_028240(invalid(errors),"set_option");
        if(api.set_option(state.socket,65535,4,1)!=0){
            const auto error=captured_wsa(errors);
            if(error.value!=0)runtime.raise_028240(error,"set_option");
        }
    }
    if(state.socket==invalid_socket)runtime.raise_028240(invalid(errors),"bind");
    const std::int32_t size=family(endpoint)==2?16:28;
    if(endpoint.size()<static_cast<std::size_t>(size))
        throw UnsupportedInput("Bind endpoint is outside initialized SDK storage");
    if(api.bind_socket(state.socket,endpoint,size)!=0){
        const auto error=captured_wsa(errors);
        if(error.value!=0)runtime.raise_028240(error,"bind");
    }
    if(state.socket==invalid_socket)runtime.raise_028240(invalid(errors),"listen");
    if(api.listen_socket(state.socket,std::numeric_limits<std::int32_t>::max())!=0){
        const auto error=captured_wsa(errors);
        if(error.value!=0)runtime.raise_028240(error,"listen");
    }
    return state;
}
void AcceptorBackend::construct_acceptor_020e20(const server_recovered::Endpoint& endpoint,bool reuse,int backlog){
    if(backlog!=std::numeric_limits<std::int32_t>::max())
        throw UnsupportedInput("Recovered constructor uses fixed INT_MAX backlog");
    static_assert(sizeof(sockaddr_in)==16 && sizeof(sockaddr_in6)==28);
    sockaddr_storage storage{};
    const auto port=static_cast<std::uint16_t>((endpoint.port_host_order>>8)|(endpoint.port_host_order<<8));
    if(endpoint.family==2){
        sockaddr_in value{};value.sin_family=endpoint.family;value.sin_port=port;
        std::memcpy(&value.sin_addr,endpoint.address.data(),4);
        std::memcpy(&storage,&value,sizeof(value));
    }else{
        sockaddr_in6 value{};value.sin6_family=endpoint.family;value.sin6_port=port;
        value.sin6_scope_id=endpoint.scope_id;
        std::memcpy(&value.sin6_addr,endpoint.address.data(),16);
        std::memcpy(&storage,&value,sizeof(value));
    }
    const auto bytes=std::as_bytes(std::span<sockaddr_storage>(&storage,1));
    (void)acceptor_020e20(state_,bytes,reuse,errors_,api_,runtime_);
}
} // namespace preloader_l::socket_recovered
