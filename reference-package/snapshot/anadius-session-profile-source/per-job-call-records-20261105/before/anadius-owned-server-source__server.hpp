#pragma once
#include "../anadius-io-memory-source/io_memory.hpp"
#include "../anadius-socket-source/acceptor_native.hpp"
#include "../anadius-socket-source/iocp_constructor_native.hpp"
#include "../anadius-socket-source/lifecycle_native.hpp"
#include "../anadius-socket-source/service_constructor_native.hpp"
#include "../anadius-socket-source/completion_dispatcher_native.hpp"
#include <functional>
#include <atomic>
#include <stdexcept>
#include <system_error>
#include <string>

namespace anadius::owned_server {
// Compiler-host exception projection retaining the original typed source error
// and optional operation. A numeric native context is stored, never dereferenced/called.
// This is not an original MSVC exception class, RTTI or unwind reconstruction.
class SourceError final:public std::system_error {
public:
    explicit SourceError(socket_recovered::Error); //028210: no operation prefix
    SourceError(socket_recovered::Error,std::string_view,std::uintptr_t context_address=0);
    // Explicit typed provider binding at a measured caller. The category must
    // outlive this exception and every copied/extracted error_code reference.
    SourceError(socket_recovered::Error,std::string_view,const std::error_category&,std::uintptr_t context_address=0);
    socket_recovered::Error source_error()const noexcept{return source_;}
    const std::string& operation()const noexcept{return operation_;}
    std::uintptr_t context_address()const noexcept{return context_address_;}
private:
    socket_recovered::Error source_;std::string operation_;std::uintptr_t context_address_;
};
namespace s=socket_recovered;
// Selected HOST lifecycle signal: the lower returned a known false drain.
// No teardown was attempted. Only retry destroy_pending_socket; never restart
// server_start/run or treat arbitrary lower exceptions as safe deferrals.
class QuiescenceDeferred final:public std::logic_error {
public:
    QuiescenceDeferred():std::logic_error("Lower accept/kernel/raw-job quiescence has not been attested"){}
};
// One worker thread/fiber only. Start before the WHOLE canonical worker/dispatch
// chain, not merely its complete callback. All packet lookup/queued/callback/
// cache access then occurs under parent Operation. Physical get alone releases
// it, captures the immediate SDK error and reacquires before returning a packet.
class AdmittedDispatcherApi final:public s::DispatcherApi {
public:
    struct WaitMaintenance {void* context=nullptr;void (*call)(void*)=nullptr;};
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
    // Borrowed synchronous POD hook; Runtime pins its context until clear.
    // Runs after physical get/error capture, before Parent is reacquired.
    void bind_wait_maintenance(WaitMaintenance);
    void clear_wait_maintenance()noexcept;
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
struct WinsockImports {
    decltype(&::WSAStartup) startup=&::WSAStartup;
    decltype(&::WSACleanup) cleanup=&::WSACleanup;
};
// Typed projection of the module count/cache at A3900/A3904. Construction is
// passive; imports never change. Each held flag belongs to one serialized
// owner. A failed/unknown SDK prefix is retained, never implicitly rolled back.
class WinsockModuleState final {
public:
    constexpr explicit WinsockModuleState(WinsockImports imports={})noexcept:imports_(imports){}
    void acquire(bool& held); //001310/023f30 increment; no cached-status check
    std::int32_t cached_status()noexcept; //023f30 atomic XADD0 observation
    void release(bool& held); //029ea0/07df30; consumes before last-only cleanup
    bool matches(WinsockImports imports)const noexcept{return imports_.startup==imports.startup&&imports_.cleanup==imports.cleanup;}
private:
    const WinsockImports imports_;
    std::atomic<std::uint32_t> count_{0};std::atomic<std::int32_t> status_{0};
};
// Passive module-lifetime default. Merely linking the common server TU does
// not acquire Winsock; the actual image first owner lives in DLL-only entry.cpp.
WinsockModuleState& native_winsock_module()noexcept;
enum class ContextProfile {Host,Original};
struct Dependencies {
    std::shared_ptr<void> state_lifetime; // retained after dependent providers
    std::shared_ptr<s::Api> errors;std::shared_ptr<s::AcceptorApi> sockets;
    std::shared_ptr<s::IocpConstructorApi> iocp;std::shared_ptr<s::ServiceGateApi> gates;
    std::shared_ptr<s::DispatcherApi> dispatcher;
    // This selected Core's private CloseContext starts with a null engine and
    // exposes no engine-install route. Provider may be absent for that branch.
    // A future nonnull engine still requires a real provider before any close;
    // ClosingLifecycleEffects rejects that missing-provider case, never skips it.
    std::shared_ptr<s::CancellationEffects> cancellation;
    std::shared_ptr<s::RegistryExecutorEffects> executor;
    std::shared_ptr<Runtime> runtime;KernelImports kernel;
    s::Category misc_category{0,server_recovered::ErrorCategory::asio_misc};
    // Original requires an explicit borrowed state with the same immutable
    // startup/cleanup pair as kernel. Retain its owner in state_lifetime or use
    // module-lifetime storage; it must outlive every live/failed Core claim.
    // Host requires nullptr and retains its per-context WORD0x0202 policy.
    ContextProfile context_profile=ContextProfile::Host;
    WinsockModuleState* winsock_module=nullptr;
};
enum class Phase {Constructed,Initializing,Ready,Running,Returned,Quiescing,Retiring,Closed,Poisoned};
struct Snapshot {Phase phase;std::size_t active_calls;bool acceptor_initialized,pending_initialized,quiescence_attested;};
// Selected compiler-host IOCP/service context composition. Original selects
// the measured0241b0 registry-gate/shared-Winsock order; native class/registry
// layout, the remaining internal service order and EH ABI remain unproved.
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
