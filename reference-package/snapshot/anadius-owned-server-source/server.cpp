#include "server.hpp"
#include "../anadius-socket-source/native.hpp"
#include <bit>
#include <cstring>
#include <exception>
#include <limits>
#include <mutex>
#include <system_error>

namespace anadius::owned_server {
namespace {
const std::error_category& contextual_error_category(s::Error error,std::uintptr_t address){
    // Only the measured024990/023f30 system-error contexts select the
    // native asio.system provider; unrelated HOST projections keep theirs.
    if(error.category.logical==server_recovered::ErrorCategory::system&&
       (address==0x180084428ULL||address==0x180084528ULL||address==0x180084500ULL))return s::system_error_category_028620();
    return error.category.logical==server_recovered::ErrorCategory::asio_misc?s::misc_error_category():std::system_category();
}
[[noreturn]]void fail(int value,std::string_view context){throw std::system_error(value,std::system_category(),std::string(context));}
[[noreturn]]void fail(s::Error error,std::string_view context,std::uintptr_t address=0){throw SourceError(error,context,address);}
struct PreserveError {DWORD saved=::GetLastError();~PreserveError(){::SetLastError(saved);}};
WinsockImports native_winsock_imports()noexcept{return {};}
bool context_profile_valid(const Dependencies& d)noexcept{
    if(d.context_profile==ContextProfile::Host)return d.winsock_module==nullptr;
    return d.context_profile==ContextProfile::Original&&d.winsock_module&&
        d.winsock_module->matches({d.kernel.startup,d.kernel.cleanup});
}
bool operation_policy_valid(s::OperationPolicy policy)noexcept{
    return policy==s::OperationPolicy::Serialized||policy==s::OperationPolicy::OriginalConcurrentActors;
}
}
WinsockModuleState& native_winsock_module()noexcept{
    PreserveError error;
    // Imported function addresses are resolved in runtime context. Keep this
    // passive initialization separate from, and complete before, acquire().
    static WinsockModuleState state{native_winsock_imports()};
    return state;
}
void WinsockModuleState::acquire(bool& held){
    {PreserveError error;if(held||!imports_.startup||!imports_.cleanup)throw std::invalid_argument("Fresh Winsock owner and complete immutable imports required");}
    const auto previous=count_.fetch_add(1,std::memory_order_seq_cst);
    held=true; // retain the increment even if startup throws or caches failure
    if(previous==0){WSADATA data{};const auto result=imports_.startup(WORD{2},&data);status_.exchange(result,std::memory_order_seq_cst);}
}
std::int32_t WinsockModuleState::cached_status()noexcept{return status_.fetch_add(0,std::memory_order_seq_cst);}
void WinsockModuleState::release(bool& held){
    {PreserveError error;if(!held||!imports_.startup||!imports_.cleanup)throw std::invalid_argument("Live Winsock owner and complete immutable imports required");}
    const auto previous=count_.fetch_sub(1,std::memory_order_seq_cst);
    held=false; // the decrement cannot be replayed if cleanup throws
    if(previous==1)(void)imports_.cleanup();
}
SourceError::SourceError(s::Error error)
    :std::system_error(error.value,error.category.logical==server_recovered::ErrorCategory::asio_misc?s::misc_error_category():std::system_category()),
     source_(error),context_address_(0){}
SourceError::SourceError(s::Error error,std::string_view operation,std::uintptr_t address)
    :SourceError(error,operation,contextual_error_category(error,address),address){}
SourceError::SourceError(s::Error error,std::string_view operation,const std::error_category& category,std::uintptr_t address)
    :std::system_error(error.value,category,std::string(operation)),
     source_(error),operation_(operation),context_address_(address){}
struct AdmittedDispatcherApi::Impl {
    const std::shared_ptr<s::SessionMemoryDomain> memory;const std::shared_ptr<s::DispatcherApi> lower;
    std::shared_ptr<void> lease;std::unique_ptr<s::SessionMemoryDomain::Operation> operation;
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bool bad=false,saved=false,maintaining=false;std::uint32_t error=0;WaitMaintenance maintenance{};
    Impl(std::shared_ptr<s::SessionMemoryDomain> m,std::shared_ptr<s::DispatcherApi> l):memory(std::move(m)),lower(std::move(l)){
        lease=memory->acquire_provider_lease();enter();
    }
    void enter(){operation.reset(new s::SessionMemoryDomain::Operation(memory->enter()));}
    void require()const{
        PreserveError sdk_error; // metadata/PTD checks must not alter the SDK error queried next
        if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber()||!operation||bad)throw std::logic_error("Dispatcher admission is not held on its worker thread/fiber");
        (void)memory->buffer_context(); // sticky parent phase/PTD checkpoint BEFORE SDK effects
    }
};
AdmittedDispatcherApi::AdmittedDispatcherApi(std::shared_ptr<s::SessionMemoryDomain> memory,std::shared_ptr<s::DispatcherApi> lower){
    if(!memory||!lower)throw std::invalid_argument("Dispatcher admission requires owned parent and lower API");
    impl_=std::make_unique<Impl>(std::move(memory),std::move(lower));
}
AdmittedDispatcherApi::~AdmittedDispatcherApi()noexcept{
    auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber())std::terminate();
    if(v.maintenance.call||v.maintaining)std::terminate();
    v.operation.reset();v.lease.reset();
}
void AdmittedDispatcherApi::set_last_error(std::uint32_t n){auto& v=*impl_;v.require();v.saved=false;try{v.lower->set_last_error(n);}catch(...){v.bad=true;throw;}}
std::uint32_t AdmittedDispatcherApi::get_last_error(){auto& v=*impl_;v.require();if(v.saved){v.saved=false;return v.error;}try{return v.lower->get_last_error();}catch(...){v.bad=true;throw;}}
std::int32_t AdmittedDispatcherApi::get(std::uintptr_t port,std::uint32_t& bytes,std::uintptr_t& key,OVERLAPPED*& raw,std::uint32_t timeout){
    auto& v=*impl_;if(v.maintaining){v.bad=true;throw std::logic_error("Wait maintenance cannot reenter dispatcher");}v.require();if(v.saved)throw std::logic_error("Previous physical wait error was not consumed");
    {PreserveError error;v.operation.reset();} // no ROOT admission during physical wait
    std::int32_t result=0;std::uint32_t status=0;
    try{result=v.lower->get(port,bytes,key,raw,timeout);status=v.lower->get_last_error();}
    catch(...){v.bad=true;throw;}
    try{if(v.maintenance.call){v.maintaining=true;{PreserveError error;v.maintenance.call(v.maintenance.context);}v.maintaining=false;if(v.bad)throw std::logic_error("Wait maintenance was poisoned");}}
    catch(...){v.maintaining=false;v.bad=true;throw;}
    try{v.enter();}catch(...){v.bad=true;throw;} // lookup/handshake cannot precede admission
    v.error=status;v.saved=true;return result;
}
std::int32_t AdmittedDispatcherApi::post(std::uintptr_t p,std::uint32_t n,std::uintptr_t k,OVERLAPPED* o){auto& v=*impl_;v.require();try{return v.lower->post(p,n,k,o);}catch(...){v.bad=true;throw;}}
std::int32_t AdmittedDispatcherApi::set_timer(std::uintptr_t t,LARGE_INTEGER& d,std::int32_t p,PTIMERAPCROUTINE cb,void* a,std::int32_t r){auto& v=*impl_;v.require();try{return v.lower->set_timer(t,d,p,cb,a,r);}catch(...){v.bad=true;throw;}}
bool AdmittedDispatcherApi::poisoned()const noexcept{return impl_->bad;}
void AdmittedDispatcherApi::bind_wait_maintenance(WaitMaintenance hook){auto& v=*impl_;v.require();if(v.maintenance.call||v.maintaining||!hook.context||!hook.call)throw std::logic_error("Complete idle wait maintenance required");v.maintenance=hook;}
void AdmittedDispatcherApi::clear_wait_maintenance()noexcept{auto& v=*impl_;if(v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber()||v.maintaining)std::terminate();v.maintenance={};}

struct OwnedServerBackend::Impl {
    struct Core;
    struct Entry final:s::IocpSchedulerEntry {using IocpSchedulerEntry::IocpSchedulerEntry;void scalar_delete(std::uint32_t)override{throw std::logic_error("Owned scheduler is embedded, not a native heap entry");}};
    struct Service final:s::SocketServiceEntry {using SocketServiceEntry::SocketServiceEntry;void scalar_delete(std::uint32_t)override{throw std::logic_error("Owned socket service is embedded, not a native heap entry");}};
    struct Life final:s::ClosingLifecycleEffects {
        Core& c;explicit Life(Core&);
        s::SocketService& resolve_service_0239c0(s::ContextIdentity)override;
        void executor_cleanup(s::SocketObject&,std::uintptr_t)override;
    };
    struct AcceptRuntime final:s::LifecycleRuntime {
        Core& c;explicit AcceptRuntime(Core&);
        s::Category miscellaneous_category_0285b0()override;
        [[noreturn]]void raise_028240(s::Error,std::string_view)override;
    };
    struct Gates final:s::ServiceGateApi {
        Core& c;explicit Gates(Core& c):c(c){}
        std::int32_t initialize(CRITICAL_SECTION&,std::uint32_t)override;
        std::uint32_t get_last_error()override;
    };
    struct Construction final:s::IocpConstructionEffects,s::ServiceConstructionEffects {
        Core& c;explicit Construction(Core& c):c(c){}
        s::IocpWorker* allocate_worker_04b0bc(std::uint32_t)override{throw std::logic_error("Selected context has no hidden scheduler worker");}
        s::IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t)override{throw std::logic_error("Selected context has no hidden worker function");}
        void start_worker_02a000(s::IocpWorker&,s::IocpWorkerFunction*,std::uint32_t)override{throw std::logic_error("Selected context has no hidden worker start");}
        void delete_worker_04b0f8(s::IocpWorker&,std::uint32_t)override{throw std::logic_error("Selected context has no hidden worker allocation");}
        s::SocketServiceEntry* allocate_04b0bc(std::uint32_t)override;
        std::uintptr_t resolve_scheduler_023b30(std::uintptr_t)override;
        [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override;
        [[noreturn]]void raise_028240(s::Error,std::string_view)override;
    };
    struct Core {
        const Dependencies deps;CRITICAL_SECTION service_section{},scheduler_section{},registry_section{};
        s::NativeSocketGate service_gate{service_section},scheduler_gate{scheduler_section};
        Entry scheduler{scheduler_gate,scheduler_section};Service service{service_gate,service_section};
        s::CloseContext close;Life life{*this};s::OwnedNoopControlFactory control;
        s::AcceptorState accept_state,pending_state;s::SocketObject acceptor{accept_state},pending{pending_state};
        AcceptRuntime accept_runtime{*this};Gates gates{*this};Construction construction{*this};
        bool service_gate_live=false,scheduler_gate_live=false,registry_gate_live=false,wsa=false;
        std::atomic<bool> context_ready{false};
        explicit Core(Dependencies d):deps(std::move(d)){}
        ~Core()noexcept{if(service_gate_live||scheduler_gate_live||registry_gate_live||wsa||context_ready.load()||scheduler.completion.port||service.service.head)std::terminate();}
        s::ContextIdentity identity(){return {reinterpret_cast<std::uintptr_t>(this)};}
        // No native resources in the C++ constructor; teardown is explicit after
        // external drain. Memory's lifecycle alias owns this Core, never Impl.
    };
    OwnedServerBackend& owner;const std::shared_ptr<thread_domain::DomainStorage> backing;
    const std::shared_ptr<Core> core;
    std::shared_ptr<s::SessionMemoryDomain> memory;std::unique_ptr<s::OwnedIoMemory> io;
    mutable std::mutex gate;Phase phase=Phase::Constructed;std::size_t active=0;
    bool accept_live=false,pending_live=false,attested=false;
    Impl(OwnedServerBackend& o,std::shared_ptr<thread_domain::DomainStorage> b,Dependencies d,std::size_t capacity)
        :owner(o),backing(std::move(b)),core(std::make_shared<Core>(std::move(d))){
        memory=std::make_shared<s::SessionMemoryDomain>(backing->make_memory(),std::shared_ptr<s::LifecycleEffects>(core,&core->life),core->deps.operation_policy);
        try{io=std::make_unique<s::OwnedIoMemory>(memory,capacity);}catch(...){if(!memory->close())std::terminate();throw;}
    }
    Impl(OwnedServerBackend& o,std::shared_ptr<thread_domain::DomainStorage> b,worker_handoff::PreparedMemory::Borrow&& borrow,Dependencies d,std::size_t capacity)
        :owner(o),backing(std::move(b)),core(std::make_shared<Core>(std::move(d))){
        memory=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::shared_ptr<s::LifecycleEffects>(core,&core->life),core->deps.operation_policy);
        try{io=std::make_unique<s::OwnedIoMemory>(memory,capacity);}catch(...){if(!memory->close())std::terminate();throw;}
    }
    ServerView view(){return {owner,core->identity(),core->service.service,core->acceptor,core->pending,core->scheduler,*memory,*io};}
    struct Call {
        Impl& v;
        Call(Impl& v,std::initializer_list<Phase> allowed,bool exclusive=false):v(v){
            std::lock_guard lock(v.gate);bool found=false;for(auto p:allowed)if(v.phase==p)found=true;
            if(!found||(exclusive&&v.active)||(!exclusive&&v.phase==Phase::Quiescing&&v.attested))throw std::logic_error("Server lifecycle/admission precondition failed");++v.active;
        }
        ~Call(){const auto parent=v.memory->snapshot();std::lock_guard lock(v.gate);if(parent.phase==s::SessionMemoryPhase::Poisoned)v.phase=Phase::Poisoned;--v.active;}
    };
    void poison()noexcept{std::lock_guard lock(gate);phase=Phase::Poisoned;}
    void expect(Phase expected){std::lock_guard lock(gate);if(phase!=expected)throw std::logic_error("Server admission was poisoned or changed during a lower effect");}
    void usable(){std::lock_guard lock(gate);if(phase==Phase::Poisoned||phase==Phase::Closed||phase==Phase::Retiring)throw std::logic_error("Server source effect observed sticky poisoned/closed admission");}
    void quiesce(){
        {std::lock_guard lock(gate);if(attested)return;if(active!=1)throw std::logic_error("Source callbacks are still active");phase=Phase::Quiescing;}
        auto borrowed=view();bool drained=false;
        try{drained=core->deps.runtime->quiescent(borrowed);}catch(...){poison();throw;}
        expect(Phase::Quiescing);
        if(!drained)throw QuiescenceDeferred();
        auto operation=memory->enter();const auto pools=io->snapshot();const auto domain=memory->snapshot();
        if(pools.phase!=s::IoMemoryPhase::Open||pools.allocations||pools.pending||pools.active_calls||domain.allocations||domain.partial_constructions||domain.buffer_borrows||domain.provider_leases!=1||
           core->scheduler.work_count_30.load()||core->scheduler.completion.head||core->scheduler.completion.tail)
            throw std::logic_error("Source sessions, operations, queues or terminal caches are not drained");
        core->service.service.gate.enter();
        auto* node=core->service.service.head;unsigned seen=0;bool valid=true;
        while(node&&seen<3){
            const unsigned bit=node==&core->acceptor?1U:(node==&core->pending?2U:0U);
            if(!bit||(seen&bit)){valid=false;break;}seen|=bit;node=node->next;
        }
        core->service.service.gate.leave();
        if(!valid||node||seen!=3)throw std::logic_error("Owned socket service has foreign, duplicated or missing nodes");
        std::lock_guard lock(gate);if(phase!=Phase::Quiescing||active!=1)throw std::logic_error("Cleanup admission is active or was poisoned during attestation");attested=true;
    }
};
OwnedServerBackend::Impl::Life::Life(Core& c):ClosingLifecycleEffects(c.service.service,c.close,*c.deps.errors,*c.deps.sockets,c.deps.cancellation.get()),c(c){}
s::SocketService& OwnedServerBackend::Impl::Life::resolve_service_0239c0(s::ContextIdentity id){if(!c.context_ready.load()||id.value!=c.identity().value)throw std::logic_error("Socket requires this live owned context");return c.service.service;}
void OwnedServerBackend::Impl::Life::executor_cleanup(s::SocketObject& o,std::uintptr_t t){c.deps.executor->executor_cleanup(o,t);}
OwnedServerBackend::Impl::AcceptRuntime::AcceptRuntime(Core& c):LifecycleRuntime(c.acceptor,c.identity(),c.life,c.control),c(c){}
s::Category OwnedServerBackend::Impl::AcceptRuntime::miscellaneous_category_0285b0(){return c.deps.misc_category;}
void OwnedServerBackend::Impl::AcceptRuntime::raise_028240(s::Error e,std::string_view text){
    if(e.value!=0&&e.category.logical==server_recovered::ErrorCategory::system&&
       (text=="open"||text=="set_option"||text=="bind"||text=="listen"))
        throw SourceError(e,text,s::system_error_category_028620());
    fail(e,text);
}
std::int32_t OwnedServerBackend::Impl::Gates::initialize(CRITICAL_SECTION& section,std::uint32_t count){
    const auto result=c.deps.gates->initialize(section,count);
    if(result){if(&section==&c.service_section)c.service_gate_live=true;else if(&section==&c.scheduler_section)c.scheduler_gate_live=true;else if(&section==&c.registry_section)c.registry_gate_live=true;else throw std::logic_error("Foreign context section");}return result;
}
std::uint32_t OwnedServerBackend::Impl::Gates::get_last_error(){return c.deps.gates->get_last_error();}
s::SocketServiceEntry* OwnedServerBackend::Impl::Construction::allocate_04b0bc(std::uint32_t bytes){if(bytes!=0x80)throw std::logic_error("Unexpected socket-service native request");return &c.service;}
std::uintptr_t OwnedServerBackend::Impl::Construction::resolve_scheduler_023b30(std::uintptr_t registry){if(registry!=c.identity().value)throw std::logic_error("Foreign scheduler registry identity");return reinterpret_cast<std::uintptr_t>(&c.scheduler);}
void OwnedServerBackend::Impl::Construction::raise_028240(s::Error e,std::uintptr_t address){
    const auto* context=address==0x180084428ULL?"mutex":address==0x180084528ULL?"iocp":"IOCP constructor";
    fail(e,context,address);
}
void OwnedServerBackend::Impl::Construction::raise_028240(s::Error e,std::string_view text){
    if(e.value!=0&&e.category.logical==server_recovered::ErrorCategory::system&&text=="mutex")
        throw SourceError(e,text,s::system_error_category_028620());
    fail(e,text);
}
OwnedServerBackend::OwnedServerBackend(std::shared_ptr<thread_domain::DomainStorage> backing,Dependencies d,std::size_t capacity){
    const auto& k=d.kernel;
    if(!backing||!d.errors||!d.sockets||!d.iocp||!d.gates||!d.dispatcher||!d.executor||!d.runtime||!capacity||capacity>65536||
       !k.startup||!k.cleanup||!k.local_name||!k.environment||!k.delete_gate||!context_profile_valid(d)||!operation_policy_valid(d.operation_policy))throw std::invalid_argument("Complete owning server providers, matching context profile, Parent policy and bounded IO capacity required");
    impl_=std::make_unique<Impl>(*this,std::move(backing),std::move(d),capacity);
}
OwnedServerBackend::OwnedServerBackend(std::shared_ptr<thread_domain::DomainStorage> backing,worker_handoff::PreparedMemory::Borrow&& borrow,Dependencies d,std::size_t capacity){
    const auto& k=d.kernel;
    if(!backing||borrow.domain_identity()!=backing.get()||!d.errors||!d.sockets||!d.iocp||!d.gates||!d.dispatcher||!d.executor||!d.runtime||!capacity||capacity>65536||
       !k.startup||!k.cleanup||!k.local_name||!k.environment||!k.delete_gate||!context_profile_valid(d)||!operation_policy_valid(d.operation_policy))throw std::invalid_argument("Matching dedicated worker memory, context profile, Parent policy and complete owning server providers required");
    (void)borrow.memory(); // reject foreign/moved/retired capability before adopting providers
    impl_=std::make_unique<Impl>(*this,std::move(backing),std::move(borrow),std::move(d),capacity);
}
OwnedServerBackend::~OwnedServerBackend()noexcept{auto& v=*impl_;if(v.phase!=Phase::Closed||v.active)std::terminate();}
void OwnedServerBackend::initialize_context_0241b0(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Constructed},true);{std::lock_guard lock(v.gate);v.phase=Phase::Initializing;}
    try{auto operation=v.memory->enter();auto& c=*v.core;
        if(c.deps.context_profile==ContextProfile::Original){
            const auto gate_status=s::initialize_gate_027be0(c.registry_section,c.gates);
            const auto gate_category=c.deps.errors->system_category_028620();
            if(gate_status)c.construction.raise_028240({std::bit_cast<std::int32_t>(gate_status),gate_category},0x180084428ULL);
            v.expect(Phase::Initializing);
            c.deps.winsock_module->acquire(c.wsa);
            v.expect(Phase::Initializing);
            const auto status=c.deps.winsock_module->cached_status();
            if(status){const auto category=c.deps.errors->system_category_028620();fail(s::Error{status,category},"winsock",0x180084500ULL);}
        }else{WSADATA data{};const auto status=c.deps.kernel.startup(MAKEWORD(2,2),&data);if(status)fail(status,"WSAStartup");c.wsa=true;}
        s::construct_iocp_024990(c.scheduler,c.identity().value,0xffffffffU,0,*c.deps.iocp,c.gates,*c.deps.errors,c.construction);
        s::SocketConstructionContext context{c.identity(),c.identity().value};
        if(s::construct_service_0228e0(context,c.construction,c.gates,*c.deps.errors)!=&c.service||!c.service_gate_live||!c.scheduler_gate_live||!c.scheduler.completion.port)throw std::logic_error("Selected context requires initialized gates and nonnull owned IOCP");
        std::lock_guard lock(v.gate);if(v.phase!=Phase::Initializing)throw std::logic_error("Context construction was poisoned by a lower effect");
        c.service.service.completion_port=c.scheduler.completion.port;c.context_ready=true;v.phase=Phase::Ready;
    }catch(...){v.poison();throw;}
}
server_recovered::ParsedAddress OwnedServerBackend::parse_address_028690(std::uint16_t family,std::string_view text){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready});auto operation=v.memory->enter();server_recovered::ParsedAddress parsed;s::Error error;
    parsed.conversion_result=s::address_028690(family,text,parsed.bytes,&parsed.scope_id,error,*v.core->deps.errors);parsed.error={error.value,error.category.logical};v.usable();return parsed;
}
void OwnedServerBackend::construct_acceptor_020e20(const server_recovered::Endpoint& endpoint,bool reuse,int backlog){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready},true);{std::lock_guard lock(v.gate);if(v.accept_live)throw std::logic_error("Acceptor already initialized");}
    if(backlog!=std::numeric_limits<std::int32_t>::max())throw s::UnsupportedInput("Native acceptor backlog is INT_MAX");
    try{auto operation=v.memory->enter();std::array<std::byte,28> bytes{};const auto family=endpoint.family;std::memcpy(bytes.data(),&family,2);
        bytes[2]=static_cast<std::byte>(endpoint.port_host_order>>8);bytes[3]=static_cast<std::byte>(endpoint.port_host_order);
        if(family!=2){std::memcpy(bytes.data()+8,endpoint.address.data(),16);std::memcpy(bytes.data()+24,&endpoint.scope_id,4);}else std::memcpy(bytes.data()+4,endpoint.address.data(),4);
        s::acceptor_020e20(v.core->accept_state,bytes,reuse,*v.core->deps.errors,*v.core->deps.sockets,v.core->accept_runtime);
        std::lock_guard lock(v.gate);if(v.phase!=Phase::Ready)throw std::logic_error("Acceptor construction was poisoned by a lower effect");v.accept_live=true;
    }catch(...){v.poison();throw;}
}
void OwnedServerBackend::initialize_pending_socket_0211b0(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready},true);{std::lock_guard lock(v.gate);if(!v.accept_live||v.pending_live)throw std::logic_error("Pending socket initialization order mismatch");}
    try{auto operation=v.memory->enter();s::initialize_0211b0(v.core->pending,v.core->identity(),v.core->life);std::lock_guard lock(v.gate);if(v.phase!=Phase::Ready)throw std::logic_error("Pending construction was poisoned");v.pending_live=true;}catch(...){v.poison();throw;}
}
server_recovered::LocalEndpoint OwnedServerBackend::local_endpoint(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready});auto operation=v.memory->enter();sockaddr_storage address{};int bytes=sizeof(address);
    const auto result=v.core->deps.kernel.local_name(v.core->accept_state.socket,reinterpret_cast<sockaddr*>(&address),&bytes);
    server_recovered::LocalEndpoint local;local.returned_size=static_cast<std::uint32_t>(bytes);
    if(result){const auto error=v.core->deps.errors->get_last_error();local.error={error,v.core->deps.errors->system_category_028620().logical};}
    if(bytes>=4)std::memcpy(&local.port_network_order,reinterpret_cast<const std::byte*>(&address)+2,2);v.usable();return local;
}
bool OwnedServerBackend::set_environment_variable(std::string_view key,std::string_view value){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready});if(key.find('\0')!=key.npos||value.find('\0')!=value.npos)throw std::invalid_argument("Environment strings must be terminated independently");
    const auto result=v.core->deps.kernel.environment(std::string(key).c_str(),std::string(value).c_str());v.usable();return result!=0;
}
void OwnedServerBackend::async_accept_021b00(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready,Phase::Running});
    {std::lock_guard lock(v.gate);if(!v.accept_live||!v.pending_live)throw std::logic_error("Accept requires initialized acceptor and pending socket");}
    try{auto operation=v.memory->enter();auto view=v.view();v.core->deps.runtime->submit_accept(view);v.usable();}catch(...){v.poison();throw;}
}
void OwnedServerBackend::rearm_from_operation(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready,Phase::Running});(void)v.memory->buffer_context();auto view=v.view();
    try{v.core->deps.runtime->submit_accept(view);v.usable();}catch(...){v.poison();throw;}
}
void ServerView::rearm_accept(){backend.rearm_from_operation();}
server_recovered::Error OwnedServerBackend::run_context_0298b0(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready},true);{std::lock_guard lock(v.gate);if(!v.accept_live||!v.pending_live)throw std::logic_error("Run requires constructed socket state");v.phase=Phase::Running;}
    try{server_recovered::Error result;
        {AdmittedDispatcherApi admitted(v.memory,v.core->deps.dispatcher);auto view=v.view();result=v.core->deps.runtime->run(view,admitted);if(admitted.poisoned())throw std::logic_error("Physical wait outcome poisoned admission");}
        std::lock_guard lock(v.gate);if(v.phase==Phase::Poisoned)throw std::logic_error("Nested server source call poisoned run");v.phase=Phase::Returned;return result;
    }catch(...){v.poison();throw;}
}
void OwnedServerBackend::destroy_pending_socket_024db0(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Returned,Phase::Quiescing},true);v.quiesce();
    {std::lock_guard lock(v.gate);if(!v.pending_live)throw std::logic_error("Pending socket already destroyed");}
    try{v.expect(Phase::Quiescing);auto operation=v.memory->enter();s::destroy_024db0(v.core->pending,v.core->life);std::lock_guard lock(v.gate);if(v.phase!=Phase::Quiescing)throw std::logic_error("Pending teardown was poisoned");v.pending_live=false;}catch(...){v.poison();throw;}
}
void OwnedServerBackend::destroy_acceptor_024db0(){
    auto& v=*impl_;Impl::Call call(v,{Phase::Quiescing},true);{std::lock_guard lock(v.gate);if(!v.attested||v.pending_live||!v.accept_live)throw std::logic_error("Acceptor teardown order mismatch");}
    try{v.expect(Phase::Quiescing);auto operation=v.memory->enter();s::destroy_024db0(v.core->acceptor,v.core->life);std::lock_guard lock(v.gate);if(v.phase!=Phase::Quiescing)throw std::logic_error("Acceptor teardown was poisoned");v.accept_live=false;}catch(...){v.poison();throw;}
}
void OwnedServerBackend::destroy_context(){
    auto& v=*impl_;
    {std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return;}
    Impl::Call call(v,{Phase::Constructed,Phase::Quiescing},true);
    {std::lock_guard lock(v.gate);if(v.accept_live||v.pending_live||(v.phase!=Phase::Constructed&&!v.attested))throw std::logic_error("Context still owns live socket/admission state");v.phase=Phase::Retiring;}
    try{
        if(!v.io->close()||!v.memory->close())throw std::logic_error("Session/IO domains remain leased or undrained");
        v.expect(Phase::Retiring);
        auto& c=*v.core;c.context_ready=false;if(c.service.service.head)throw std::logic_error("Owned service still has source socket nodes");
        if(c.deps.context_profile==ContextProfile::Original){
            if(c.wsa){c.deps.winsock_module->release(c.wsa);v.expect(Phase::Retiring);}
            // Fixed socket-service then scheduler shutdown pass. Selected
            // own_thread0/no timer/zero-work state needs no hidden drain path.
            if(c.service_gate_live){c.service.service.gate.enter();c.service.service.gate.leave();v.expect(Phase::Retiring);}
            // Native029bb6 exchanges only +3c. Under exclusive Retiring this
            // typed value projection replaces high32 exactly, retaining wake;
            // it does not claim the native dword atomic/layout ABI.
            if(c.scheduler_gate_live)c.scheduler.word_38=(c.scheduler.word_38&0xffffffffULL)|0x100000000ULL;
            if(c.service_gate_live){c.deps.kernel.delete_gate(&c.service_section);v.expect(Phase::Retiring);c.service_gate_live=false;}
            if(c.scheduler_gate_live){c.deps.kernel.delete_gate(&c.scheduler_section);v.expect(Phase::Retiring);c.scheduler_gate_live=false;}
            if(c.scheduler.completion.port){(void)c.deps.iocp->close_handle(c.scheduler.completion.port);v.expect(Phase::Retiring);c.scheduler.completion.port=0;c.service.service.completion_port=0;}
        }else{
            if(c.scheduler.completion.port){if(!c.deps.iocp->close_handle(c.scheduler.completion.port))fail(static_cast<int>(c.deps.iocp->get_last_error()),"CloseHandle IOCP");v.expect(Phase::Retiring);c.scheduler.completion.port=0;c.service.service.completion_port=0;}
            if(c.service_gate_live){c.deps.kernel.delete_gate(&c.service_section);v.expect(Phase::Retiring);c.service_gate_live=false;}
            if(c.scheduler_gate_live){c.deps.kernel.delete_gate(&c.scheduler_section);v.expect(Phase::Retiring);c.scheduler_gate_live=false;}
            if(c.wsa){if(c.deps.kernel.cleanup()!=0)fail(c.deps.errors->get_last_error(),"WSACleanup");v.expect(Phase::Retiring);c.wsa=false;}
        }
        if(c.registry_gate_live){c.deps.kernel.delete_gate(&c.registry_section);v.expect(Phase::Retiring);c.registry_gate_live=false;}
        std::lock_guard lock(v.gate);if(v.phase!=Phase::Retiring)throw std::logic_error("Context retirement was poisoned");v.phase=Phase::Closed;
    }catch(...){v.poison();throw;}
}
void OwnedServerBackend::raise_error(server_recovered::Error error,std::string_view text){
    impl_->poison();const s::Error source{error.value,{0,error.category}};
    if(text.empty())throw SourceError(source); //028210: avoid even an empty context constructor
    fail(source,text); //028240
}
void OwnedServerBackend::with_operation(const std::function<void(ServerView&)>& fn){
    auto& v=*impl_;Impl::Call call(v,{Phase::Ready,Phase::Running,Phase::Returned,Phase::Quiescing});if(!fn)throw std::invalid_argument("Source operation callback required");
    try{auto operation=v.memory->enter();auto view=v.view();fn(view);v.usable();}catch(...){v.poison();throw;}
}
Snapshot OwnedServerBackend::snapshot()const{auto& v=*impl_;std::lock_guard lock(v.gate);return {v.phase,v.active,v.accept_live,v.pending_live,v.attested};}
} // namespace anadius::owned_server
