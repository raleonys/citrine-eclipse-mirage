#pragma once
#include "preloader_l/net/transport_operations.hpp"

namespace preloader_l::socket_recovered {
inline constexpr Socket invalid_socket=~Socket{0};
struct AcceptorService {std::uintptr_t completion_port=0;};
// Logical selected fields, not an overlay of the native ASIO object.
struct AcceptorState {
    Socket socket=invalid_socket;
    std::uint8_t socket_state=0;
    std::int32_t protocol_40=2;
    std::uint8_t byte_44=0;
    std::uint64_t slot_48=2,slot_50=0,slot_58=0;
    std::uint32_t slot_60=0;
    AcceptorService* service=nullptr; // caller-owned service/implementation projection
};
class AcceptorApi {
public:
    virtual ~AcceptorApi()=default;
    virtual Socket create_socket(std::int32_t family)=0;
    virtual std::uintptr_t attach_completion_port(Socket,std::uintptr_t port)=0;
    virtual std::uint32_t get_win32_error()=0;
    virtual std::int32_t set_option(Socket,std::int32_t level,std::int32_t option,std::int32_t value)=0;
    virtual std::int32_t bind_socket(Socket,std::span<const std::byte> address,std::int32_t size)=0;
    virtual std::int32_t listen_socket(Socket,std::int32_t backlog)=0;
    virtual std::int32_t close_socket(Socket)=0;
    virtual std::int32_t set_nonblocking(Socket,std::uint32_t command,std::uint32_t& value)=0;
};
class AcceptorRuntime {
public:
    virtual ~AcceptorRuntime()=default;
    // Required constructor/service/intrusive-list and shared owner operations.
    // These internals are not newly reconstructed here.
    virtual void initialize_0211b0(AcceptorState&)=0;
    virtual void reset_cancellation_0237f0(AcceptorState&)=0;
    virtual Category miscellaneous_category_0285b0()=0;
    [[noreturn]] virtual void raise_028240(Error,std::string_view context)=0;
};
// Caches the input socket, does not invalidate the caller's slot, ignores errors.
void close_025310(const Socket&,Api& errors,AcceptorApi&);
// Endpoint holds initialized SDK sockaddr bytes. Family is read after initialize
// and again immediately before bind. Short spans are outside native valid-memory
// domain and raise UnsupportedInput, not a modeled WSA/native exception.
AcceptorState& acceptor_020e20(AcceptorState&,std::span<const std::byte> endpoint,
    bool reuse_address,Api& errors,AcceptorApi&,AcceptorRuntime&);
// Partial existing ServerBackend bridge; remaining context/accept/lifetime/run
// methods stay abstract. No real API is invoked merely by construction.
class AcceptorBackend:public AddressBackend {
public:
    AcceptorBackend(AcceptorState& state,Api& errors,AcceptorApi& api,AcceptorRuntime& runtime)
        :AddressBackend(errors),state_(state),errors_(errors),api_(api),runtime_(runtime){}
    void construct_acceptor_020e20(const server_recovered::Endpoint&,bool,int) final;
private:
    AcceptorState& state_;Api& errors_;AcceptorApi& api_;AcceptorRuntime& runtime_;
};
} // namespace preloader_l::socket_recovered
