#pragma once
#include "../anadius-io-memory-source/io_memory.hpp"
#include "../anadius-socket-source/acceptor_native.hpp"
#include "../anadius-socket-source/iocp_constructor_native.hpp"
#include "../anadius-socket-source/lifecycle_native.hpp"
#include "../anadius-socket-source/service_constructor_native.hpp"
#include "../anadius-socket-source/completion_dispatcher_native.hpp"
#include <functional>

namespace anadius::owned_server {
namespace s=socket_recovered;
// One worker thread/fiber only. Start before the WHOLE canonical worker/dispatch
// chain, not merely its complete callback. All packet lookup/queued/callback/
// cache access then occurs under parent Operation. Physical get alone releases
// it, captures the immediate SDK error and reacquires before returning a packet.
class AdmittedDispatcherApi final:public s::DispatcherApi {
public:
    AdmittedDispatcherApi(std::shared_ptr<s::SessionMemoryDomain>,std::shared_ptr<s::DispatcherApi>);
    ~AdmittedDispatcherApi()noexcept;
    AdmittedDispatcherApi(const AdmittedDispatcherApi&)=delete;
    AdmittedDispatcherApi& operator=(const AdmittedDispatcherApi&)=delete;
    void set_last_error(std::uint32_t)override;
    std::uint32_t get_last_error()override;
    std::int32_t get(std::uintptr_t,std::uint32_t&,std::uintptr_t&,OVERLAPPED*&,std::uint32_t)override;
    std::int32_t post(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override;
    std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override;
    bool poisoned()const noexcept;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
class OwnedServerBackend;
struct ServerView {
    OwnedServerBackend& backend;
    s::ContextIdentity identity;s::SocketService& service;
    s::SocketObject& acceptor;s::SocketObject& pending;s::IocpSchedulerEntry& scheduler;
    s::SessionMemoryDomain& memory;s::OwnedIoMemory& io;
    // Existing parent Operation required; actual023000, not coarse Session.
    s::SessionPair construct_accepted(s::ExecutorMove move){return memory.construct_session(pending,move);}
    void rearm_accept(); // already inside admitted worker/source Operation
};
class Runtime {
public:
    virtual ~Runtime()=default;
    // MANDATORY missing021b00/0271a0 implementation. It owns distinct accept
    // operation/endpoint buffers and kernel borrower lifetimes. Never alias a
    // pending accept to B34 receive or fabricate a SessionControl before accept.
    virtual void submit_accept(ServerView&)=0;
    // Use admitted API for every physical wait in actual0298b0/027c10; this
    // method begins with parent Operation held. Do not open a nested Operation.
    virtual server_recovered::Error run(ServerView&,AdmittedDispatcherApi&)=0;
    // External stop + final accept/IO packets + raw jobs + all workers/caches
    // drained; true is a trusted lower attestation, NOT inferred from counts.
    // Called outside parent Operation, after new backend admission is stopped.
    virtual bool quiescent(ServerView&)=0;
};
struct KernelImports {
    decltype(&::WSAStartup) startup=&::WSAStartup;
    decltype(&::WSACleanup) cleanup=&::WSACleanup;
    decltype(&::getsockname) local_name=&::getsockname;
    decltype(&::SetEnvironmentVariableA) environment=&::SetEnvironmentVariableA;
    decltype(&::DeleteCriticalSection) delete_gate=&::DeleteCriticalSection;
};
struct Dependencies {
    std::shared_ptr<void> state_lifetime; // retained after dependent providers
    std::shared_ptr<s::Api> errors;std::shared_ptr<s::AcceptorApi> sockets;
    std::shared_ptr<s::IocpConstructorApi> iocp;std::shared_ptr<s::ServiceGateApi> gates;
    std::shared_ptr<s::DispatcherApi> dispatcher;
    std::shared_ptr<s::CancellationEffects> cancellation;
    std::shared_ptr<s::RegistryExecutorEffects> executor;
    std::shared_ptr<Runtime> runtime;KernelImports kernel;
    s::Category misc_category{0,server_recovered::ErrorCategory::asio_misc};
};
enum class Phase {Constructed,Initializing,Ready,Running,Returned,Quiescing,Retiring,Closed,Poisoned};
struct Snapshot {Phase phase;std::size_t active_calls;bool acceptor_initialized,pending_initialized,quiescence_attested;};
// Selected compiler-host IOCP/service context policy; original0241b0 and its
// native class/registry/EH layout are NOT recovered by this composition.
// Owns stable context/service/acceptor/pending storage and shared session+IO
// domains. Fits bootstrap_backend::Resources.server directly. No PE entry.
class OwnedServerBackend final:public server_recovered::ServerBackend {
public:
    OwnedServerBackend(std::shared_ptr<thread_domain::DomainStorage>,Dependencies,std::size_t io_capacity=64);
    OwnedServerBackend(std::shared_ptr<thread_domain::DomainStorage>,worker_handoff::PreparedMemory::Borrow&&,Dependencies,std::size_t io_capacity=64);
    ~OwnedServerBackend()noexcept;
    OwnedServerBackend(const OwnedServerBackend&)=delete;OwnedServerBackend& operator=(const OwnedServerBackend&)=delete;
    void initialize_context_0241b0()override;
    server_recovered::ParsedAddress parse_address_028690(std::uint16_t,std::string_view)override;
    void construct_acceptor_020e20(const server_recovered::Endpoint&,bool,int)override;
    void initialize_pending_socket_0211b0()override;
    server_recovered::LocalEndpoint local_endpoint()override;
    bool set_environment_variable(std::string_view,std::string_view)override;
    void async_accept_021b00()override;
    server_recovered::Error run_context_0298b0()override;
    void destroy_pending_socket_024db0()override;
    void destroy_acceptor_024db0()override;
    void destroy_context()override;
    [[noreturn]]void raise_error(server_recovered::Error,std::string_view)override;
    // For existing source callbacks/releases outside the worker chain. View is
    // borrowed only for this call; no reentrant/nested Operation. After Quiescing
    // this admits cleanup only, not new work (caller contract).
    void with_operation(const std::function<void(ServerView&)>&);
    Snapshot snapshot()const;
private:
    friend struct ServerView;void rearm_from_operation();
    struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::owned_server
