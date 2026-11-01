#pragma once
#include "../anadius-accept-source/owned.hpp"

namespace anadius::socket_recovered {
// Logical server fields, not a native object overlay. identity is the live
// server token; native fields are service00/listener08/pending98/allocator68.
struct FlowServer {
    std::uintptr_t identity=0;AsyncAcceptService& service;
    AcceptorState& listener;AcceptPending& pending;
};
struct FlowServerBinding {std::uintptr_t identity=0;FlowServer* server=nullptr;};
class FlowApi {
public:
    virtual ~FlowApi()=default;
    virtual SessionPair& construct(SessionPair& output,SocketObject& pending)=0;
    // Native captures data/size from RETURNED prefix, but frees LOCAL output.
    virtual ReceiveBufferString& greeting(SessionOwner&,ReceiveBufferString& output)=0;
    virtual void start_accept(AsyncAcceptService&,AcceptorState&,AcceptPending&,
        AcceptEndpoint*,const std::uintptr_t& handler,std::uintptr_t allocator)=0;
    [[noreturn]]virtual void raise_write(Error&,std::uintptr_t context_identity)=0;
};
struct FlowContext {
    FlowApi& api;ReceiveBufferContext& strings;FrameReadContext& frame;
    Api& sockets;WriteFinishApi& logging;
    std::span<FlowServerBinding> servers;
    std::span<WriteFinishCategoryBinding> categories;
    Category success_category_a10f8;
};
// Copies the 198 defined native cipher fields while preserving destination's
// OWN schedule reference. The two native padding bytes and C++ vptr/layout are
// not projected. This selected typed transfer is not another native body.
void copy_session_cipher_defined_fields(SessionCipher&,const SessionCipher&);
void rearm_accept_0270e0(FlowServer&,FlowContext&);
void accept_session_025720(AcceptHandler& original_handler,Error& private_error,FlowContext&);
void invoke_accept_handler_021a10(AcceptHandler&,FlowContext&);

// Application message construction/error rendering remain explicit providers.
// Implementations may call the existing actual MessageHandler sources with
// synthetic/supplied state; all references outlive pending work and callbacks.
class FlowApplication {
public:
    virtual ~FlowApplication()=default;
    virtual ReceiveBufferString& greeting(SessionOwner&,ReceiveBufferString&)=0;
    [[noreturn]]virtual void raise_write(Error&,std::uintptr_t)=0;
};
class DomainFlowApi final:public FlowApi {
public:
    DomainFlowApi(SessionMemoryDomain& m,OwnedAcceptMemory& a,ExecutorMove e,FlowApplication& app)
        :memory_(m),accept_(a),executor_(e),application_(app){}
    SessionPair& construct(SessionPair&,SocketObject&)override;
    ReceiveBufferString& greeting(SessionOwner&,ReceiveBufferString&)override;
    void start_accept(AsyncAcceptService&,AcceptorState&,AcceptPending&,AcceptEndpoint*,const std::uintptr_t&,std::uintptr_t)override;
    [[noreturn]]void raise_write(Error&,std::uintptr_t)override;
private:SessionMemoryDomain& memory_;OwnedAcceptMemory& accept_;ExecutorMove executor_;FlowApplication& application_;
};
} // namespace anadius::socket_recovered
