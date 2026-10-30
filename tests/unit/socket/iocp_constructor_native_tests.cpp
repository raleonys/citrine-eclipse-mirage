#include "preloader_l/net/transport_service_constructor_native.hpp"
#include "preloader_l/net/transport_iocp_constructor_native.hpp"
#include "preloader_l/net/transport_recycle_storage.hpp"
#include <functional>
#include <cstring>
#include <iostream>
#include <limits>
#include <vector>

namespace s=preloader_l::socket_recovered;
namespace srv=preloader_l::server_recovered;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected controlled registry integration dependency");}
struct Gate final:s::SocketGate {
    int depth=0;unsigned enters=0,leaves=0;std::function<void(bool)> hook;
    void enter()override{check(!depth,"each integration gate acquired from unlocked state");++depth;++enters;if(hook)hook(true);}
    void leave()override{check(depth==1,"integration gate held on release");if(hook)hook(false);--depth;++leaves;}
};
struct Entry final:s::SocketServiceEntry {using SocketServiceEntry::SocketServiceEntry;unsigned deletes=0;void scalar_delete(std::uint32_t flags)override{check(flags==1,"registry race delete flag1");++deletes;}};
struct IocpFixture {CRITICAL_SECTION* section=nullptr;BOOL verified=1,closed=1;DWORD error=0,threads=0;HANDLE file=nullptr,existing=nullptr;ULONG_PTR key=0;ULONGLONG mask=0xfedcba9876543210ULL;unsigned masks=0,verifies=0,creates=0,gets=0,closes=0;HANDLE port=reinterpret_cast<HANDLE>(0x8877665544332211ULL),handle=nullptr;};
IocpFixture* active_iocp=nullptr;
ULONGLONG WINAPI fake_mask(ULONGLONG mask,DWORD type,BYTE condition){check(active_iocp && !mask && type==2 && condition==3,"SDK VerSet exact uint64/DWORD/BYTE");++active_iocp->masks;return active_iocp->mask;}
BOOL WINAPI fake_verify(LPOSVERSIONINFOEXA version,DWORD type,DWORDLONG mask){check(active_iocp && version && type==2 && mask==active_iocp->mask,"SDK Verify exact request pointer/type/64-bit mask");OSVERSIONINFOEXA expected{};expected.dwOSVersionInfoSize=0x9c;expected.dwMajorVersion=6;check(!std::memcmp(version,&expected,sizeof(expected)),"SDK Verify reads fully initialized156-byte request");++active_iocp->verifies;return active_iocp->verified;}
HANDLE WINAPI fake_port(HANDLE file,HANDLE existing,ULONG_PTR key,DWORD threads){check(active_iocp!=nullptr,"SDK port fixture bound");auto& f=*active_iocp;f.file=file;f.existing=existing;f.key=key;f.threads=threads;++f.creates;return f.port;}
DWORD WINAPI fake_port_error(){check(active_iocp!=nullptr,"SDK Win32 error fixture bound");++active_iocp->gets;return active_iocp->error;}
BOOL WINAPI fake_close(HANDLE handle){check(active_iocp!=nullptr,"SDK CloseHandle fixture bound");active_iocp->handle=handle;++active_iocp->closes;return active_iocp->closed;}
s::IocpConstructorImports iocp_imports(){s::IocpConstructorImports i;i.set_version_mask=fake_mask;i.verify_version=fake_verify;i.create_completion_port=fake_port;i.get_last_error=fake_port_error;i.close_handle=fake_close;return i;}
void native_iocp(){
    {s::NativeIocpConstructorApi defaults;check(!active_iocp,"default adapter construction invokes no imports");}
    IocpFixture f;active_iocp=&f;s::NativeIocpConstructorApi api(iocp_imports());check(!f.masks && !f.verifies && !f.creates && !f.gets && !f.closes,"injected adapter construction invokes no imports");
    for(auto mask:{0ULL,1ULL,0x8000000000000000ULL,0xffffffffffffffffULL}){f.mask=mask;check(api.set_version_mask(0,2,3)==mask,"SDK64bit mask return exact");for(BOOL result:{BOOL{0},BOOL{1},BOOL{-1},std::numeric_limits<BOOL>::min()}){f.verified=result;OSVERSIONINFOEXA version{};version.dwOSVersionInfoSize=0x9c;version.dwMajorVersion=6;check(api.verify_version(version,2,mask)==result,"SDK BOOL raw zero/positive/negative preserved");}}
    for(auto file:{std::uintptr_t{0},std::uintptr_t{0xffffffffffffffffULL}})for(auto existing:{std::uintptr_t{0},std::uintptr_t{0x876543219abcdef0ULL}})for(auto key:{std::uintptr_t{0},std::uintptr_t{0xfedcba9876543210ULL}})for(auto threads:{0U,1U,0x80000000U,0xffffffffU})for(auto port:{std::uintptr_t{0},std::uintptr_t{0x8877665544332211ULL}}){f.port=reinterpret_cast<HANDLE>(port);check(api.create_completion_port(file,existing,key,threads)==port,"SDK HANDLE return preserves null/fullwidth bits");check(reinterpret_cast<std::uintptr_t>(f.file)==file && reinterpret_cast<std::uintptr_t>(f.existing)==existing && f.key==key && f.threads==threads,"SDK all four IOCP arguments preserve widths");}
    for(auto error:{0U,1U,0x80000000U,0xffffffffU}){f.error=error;check(api.get_last_error()==error,"SDK DWORD GetLastError exact including zero/highbit");}
    for(auto handle:{std::uintptr_t{0},std::uintptr_t{0xffffffffffffffffULL},std::uintptr_t{0xfedcba9876543210ULL}})for(BOOL result:{BOOL{0},BOOL{1},BOOL{-1}}){f.closed=result;check(api.close_handle(handle)==result && reinterpret_cast<std::uintptr_t>(f.handle)==handle,"SDK CloseHandle preserves HANDLE and raw BOOL");}
    check(f.masks==4 && f.verifies==16 && f.creates==64 && f.gets==4 && f.closes==9,"exact injected-five-import call counts");active_iocp=nullptr;
}
struct GateFixture {CRITICAL_SECTION* expected=nullptr;BOOL result=1;DWORD error=0;unsigned initializes=0,gets=0;std::vector<std::string> trace;};
GateFixture* active=nullptr;
BOOL WINAPI fake_initialize(LPCRITICAL_SECTION section,DWORD spin){check(active && (section==active->expected || (active_iocp && section==active_iocp->section)) && spin==0x80000000UL,"SDK exact section pointer and unsigned high spin bit");++active->initializes;active->trace.push_back("initialize");return active->result;}
DWORD WINAPI fake_get(){check(active!=nullptr,"GetLastError has controlled fixture");++active->gets;active->trace.push_back("GetLastError");return active->error;}
s::ServiceGateImports imports(){s::ServiceGateImports value;value.initialize=fake_initialize;value.get_last_error=fake_get;return value;}
struct SchedulerEntry final:s::IocpSchedulerEntry {using IocpSchedulerEntry::IocpSchedulerEntry;unsigned deletes=0;void scalar_delete(std::uint32_t flags)override{check(flags==1,"scheduler scalar-delete flag1");++deletes;}};
struct Workers final:s::IocpConstructionEffects {
    s::IocpWorker worker{0x1234,0x2222,0x3333};s::IocpWorkerFunction function{0x4444,0x5555,0x6666,nullptr};unsigned starts=0;
    s::IocpWorker* allocate_worker_04b0bc(std::uint32_t bytes)override{check(bytes==24,"integration actual024990 allocates native24 worker projection");return &worker;}
    s::IocpWorkerFunction* allocate_function_04b0bc(std::uint32_t bytes)override{check(bytes==32 && worker.word_00==0x1234 && !worker.handle_08 && !worker.event_10,"integration function allocation after only worker fields8/10 zero");return &function;}
    void start_worker_02a000(s::IocpWorker& w,s::IocpWorkerFunction* f,std::uint32_t stack)override{check(&w==&worker && f==&function && !stack && f->table_identity==0x180084710ULL && f->scheduler_18 && f->event_08==0x5555 && f->event_10==0x6666,"actual ctor delegates worker start without real event/thread");check(f->scheduler_18->work_count_30==1 && f->scheduler_18->completion.port==0x8877665544332211ULL,"actual IOCP scheduler count/port initialized before required start");++starts;}
    void delete_worker_04b0f8(s::IocpWorker&,std::uint32_t)override{unused();}
    [[noreturn]]void raise_028240(s::Error,std::uintptr_t)override{unused();}
};
struct SchedulerEffects final:s::IocpSchedulerRegistryEffects {
    Gate& gate;s::ServiceRegistry& registry;SchedulerEntry& candidate;SchedulerEntry& winner;bool race=false;unsigned allocations=0,comparisons=0;
    SchedulerEffects(Gate& gate,s::ServiceRegistry& registry,SchedulerEntry& candidate,SchedulerEntry& winner,s::IocpConstructorApi& api,s::ServiceGateApi& gates,s::Api& errors,Workers& workers):IocpSchedulerRegistryEffects(api,gates,errors,workers),gate(gate),registry(registry),candidate(candidate),winner(winner){}
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t left,std::uintptr_t right)override{check(gate.depth==1 && right==0x1800a2548ULL,"actual nested resolver compares under shared registry gate");++comparisons;return left==right?0U:1U;}
    s::ServiceEntry* allocate_04b0bc(std::uint32_t bytes)override{check(!gate.depth && bytes==0xb0,"actual nested resolver requests176 outside registry gate");++allocations;return &candidate;}
};
struct Construction final:s::SchedulerConstructionEffects {
    Gate& registry_gate;Entry& entry;unsigned allocations=0;
    Construction(Gate& g,Entry& e,std::span<s::SchedulerRegistryBinding> registries,SchedulerEffects& effects,std::span<s::SchedulerServiceBinding> services):SchedulerConstructionEffects(registries,effects,services),registry_gate(g),entry(e){}
    s::SocketServiceEntry* allocate_04b0bc(std::uint32_t bytes)override{check(bytes==0x80 && !registry_gate.depth,"actual socket constructor allocation128 outside gate");++allocations;return &entry;}
    [[noreturn]]void raise_028240(s::Error,std::string_view)override{unused();}
};
struct RegistryEffects final:s::ConstructorRegistryEffects {
    using ConstructorRegistryEffects::ConstructorRegistryEffects;unsigned comparisons=0;
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t left,std::uintptr_t right)override{check(right==0x1800a2628ULL,"socket RTTI exact descriptorplus8 in mixed registry");++comparisons;return left==right?0U:1U;}
};
struct Errors final:s::Api {
    unsigned sends=0,categories=0,sets=0;std::int32_t error=0;s::Socket wake=0xfedcba9876543210ULL;
    void set_last_error(std::int32_t value)override{error=value;++sets;}
    s::Category system_category_028620()override{++categories;return {0x9922,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{return error;}
    s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
    s::SendCall wsa_send(s::Socket socket,std::span<s::Buffer> buffer,std::uint32_t flags)override{check(socket==wake && buffer.size()==1 && buffer[0].len==1 && buffer[0].buf[0]==0 && !flags,"actual queue wake uses injected one-byte NUL transfer only");++sends;return {0,1};}
    std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Acceptor final:s::AcceptorApi {
    unsigned creates=0,closes=0;
    s::Socket create_socket(std::int32_t family)override{check(family==2,"actual acceptor selects IPv4 family");++creates;return 13;}
    std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{check(socket==13 && port==99,"actual registry-resolved service supplies selected IOCP token");return 99;}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t name,std::int32_t value)override{check(socket==13 && level==65535 && name==128 && value==0,"actual low-level close requests observed linger option");return 0;}
    std::int32_t bind_socket(s::Socket socket,std::span<const std::byte>,std::int32_t size)override{check(socket==13 && size==16,"actual acceptor binds IPv4 endpoint length");return 0;}
    std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{check(socket==13 && backlog==2147483647,"actual acceptor listens with observed backlog");return 0;}
    std::int32_t close_socket(s::Socket socket)override{check(socket==13,"actual close retains selected socket identity");++closes;return 0;}
    std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Executor final:s::RegistryExecutorEffects {
    unsigned calls=0;s::SocketObject* expected=nullptr;std::uintptr_t token=0;std::function<void()> hook;
    void executor_cleanup(s::SocketObject& object,std::uintptr_t table)override{check(&object==expected && table==token,"bridge forwards exact borrowed object and full-width table token");++calls;if(hook)hook();}
};
struct Failure final:s::RecycleFailureEffects {[[noreturn]]void length_error_047fc4(std::uintptr_t)override{unused();}};
struct Post final:s::CompletionApi {
    bool fail=false;std::vector<s::CancelOperation*> posted;
    std::int32_t post(std::uintptr_t port,s::CancelOperation& operation)override{check(port==0x8877665544332211ULL && operation.queued==1 && !operation.next,"actual queue posts detached marked operations using selected scheduler token");posted.push_back(&operation);return fail?0:1;}
};
struct Owner final:s::Control {unsigned destroys=0,deletes=0;void destroy_resource()override{++destroys;}void delete_control()override{++deletes;}};
struct Factory final:s::ControlFactory {Owner owner;s::Control* allocate_04b0bc(std::uint32_t bytes)override{check(bytes==24,"actual lifecycle requests observed shared-owner allocation size");return &owner;}};
struct Runtime final:s::LifecycleRuntime {using LifecycleRuntime::LifecycleRuntime;s::Category miscellaneous_category_0285b0()override{unused();}[[noreturn]]void raise_028240(s::Error,std::string_view)override{unused();}};
void native_gate(){
    {s::NativeServiceGateApi default_adapter;check(active==nullptr,"constructing default SDK adapter invokes no imports");}
    CRITICAL_SECTION section{};GateFixture fixture;fixture.expected=&section;active=&fixture;s::NativeServiceGateApi api(imports());
    check(fixture.trace.empty(),"constructing injected adapter invokes no imports");
    for(BOOL result:{BOOL{1},BOOL{2},BOOL{-1},std::numeric_limits<BOOL>::min()}){
        fixture.result=result;fixture.error=0xffffffffUL;fixture.trace.clear();const auto gets=fixture.gets;
        check(s::initialize_gate_027be0(section,api)==0 && fixture.gets==gets && fixture.trace==std::vector<std::string>{"initialize"},"every nonzero SDK BOOL succeeds without GetLastError");
    }
    for(DWORD error:{DWORD{0},DWORD{1},DWORD{0x7fffffffUL},DWORD{0x80000000UL},DWORD{0xffffffffUL}}){
        fixture.result=0;fixture.error=error;fixture.trace.clear();
        check(s::initialize_gate_027be0(section,api)==error && fixture.trace==std::vector<std::string>{"initialize","GetLastError"},"SDK zero BOOL calls exact GetLastError and preserves DWORD including zero/high bit");
    }
    check(fixture.initializes==9 && fixture.gets==5,"fake SDK invocation counts exact; no WSA error getter involved");active=nullptr;
}
void full_chain(){
    for(bool failed_post:{false,true})for(bool preexisting:{false,true})for(bool cached_scheduler:{false})for(bool scheduler_race:{false}){
        Gate registry_gate,service_gate,engine_gate,fallback_gate;s::ServiceRegistry registry(registry_gate);CRITICAL_SECTION section{};Entry entry(service_gate,section);Errors errors;GateFixture sdk;sdk.expected=&section;active=&sdk;s::NativeServiceGateApi gate_api(imports());s::SchedulerRegistry scheduler_registry(registry);scheduler_registry.owner_context=0xdeadbeef99887766ULL;CRITICAL_SECTION scheduler_section{};SchedulerEntry scheduler_entry(fallback_gate,scheduler_section),scheduler_winner(fallback_gate,scheduler_section);IocpFixture iocp;iocp.section=&scheduler_section;active_iocp=&iocp;s::NativeIocpConstructorApi iocp_api(iocp_imports());Workers workers;SchedulerEffects scheduler_effects(registry_gate,registry,scheduler_entry,scheduler_winner,iocp_api,gate_api,errors,workers);scheduler_effects.race=scheduler_race;std::array<s::SchedulerRegistryBinding,1> scheduler_registries{{{0x123456789abcdef0ULL,&scheduler_registry}}};std::array<s::SchedulerServiceBinding,2> scheduler_services{{{&scheduler_entry,0x9988776655443322ULL},{&scheduler_winner,0x9988776655443322ULL}}};Construction construction(registry_gate,entry,scheduler_registries,scheduler_effects,scheduler_services);s::SocketConstructionContext construction_context{{42},0x123456789abcdef0ULL};RegistryEffects required(construction_context,construction,gate_api,errors);
        if(cached_scheduler){scheduler_entry.key={0x1800a2540ULL,0};registry.head=&scheduler_entry;}if(preexisting){entry.key={0x1800a2620ULL,0};entry.next=registry.head;registry.head=&entry;}
        auto& service=entry.service;service.completion_port=99;s::CancellationEngine engine(engine_gate);s::CloseContext context;context.engine=&engine;
        Acceptor api;Post completion;completion.fail=failed_post;auto& scheduler=scheduler_entry.completion;scheduler.port=0x8877665544332211ULL; /* selected cached projection only; actual024990 rewrites this for new entries */ s::CancellationState queue;queue.scheduler=&scheduler;queue.wake.socket=errors.wake;
        s::OwnedRecycleMap storage(3);std::array<s::CancelOperation,3> operations;
        for(auto& op:operations){op.overlapped.Internal=0x7788;op.queued=99;storage.append(13,op);}auto* original=&storage.insert(13);queue.maps[0]=storage.map();
        std::array<s::RecycleMap*,1> metadata{&storage.metadata()};Failure failure;s::RecoveredCancellationRecycler recycler(metadata,failure);
        s::QueueCancellationEffects cancellation(engine,queue,recycler,completion,errors);std::array<s::RegistryServiceBinding,1> bindings{{{&entry,&service,&context}}};Executor executor;
        s::RegistryLifecycleEffects bridge(registry,{42},required,bindings,errors,api,cancellation,executor);s::AcceptorState state;s::SocketObject object(state);Factory factory;Runtime runtime(object,{42},bridge,factory);
        executor.expected=&object;executor.token=0x180084928ULL;
        executor.hook=[&]{check(!service.head && !object.next && !object.previous && state.socket==s::invalid_socket && !object.cancellation.control,"actual destroy unlinks/resets/releases before bridge executor callback");check(!engine_gate.depth && !service_gate.depth && !fallback_gate.depth && !registry_gate.depth,"all gates released before final executor callback");};
        engine_gate.hook=[&](bool enter){if(!enter)check(storage.metadata().free_size==1 && storage.metadata().active_size==0,"real recycler transfers node while cancellation engine gate held");};
        alignas(8)std::array<std::byte,128> endpoint{};endpoint[0]=std::byte{2};s::acceptor_020e20(state,endpoint,false,errors,api,runtime);
        check(service.head==&object && object.service==&service && state.service==&service && state.socket==13 && object.cancellation.control==&factory.owner,"actual acceptor resolves real registry and initializes selected lifecycle service");
        check(construction.allocations==static_cast<unsigned>(!preexisting) && registry.head==&entry,"mixed registry preserves outer socket service publication");if(!preexisting){auto* scheduler_expected=(!cached_scheduler && scheduler_race)?&scheduler_winner:&scheduler_entry;check(entry.next==scheduler_expected && scheduler_expected->key.type==0x1800a2540ULL && !scheduler_expected->next,"nested scheduler publication survives outer publication");check(scheduler_effects.allocations==static_cast<unsigned>(!cached_scheduler) && workers.starts==static_cast<unsigned>(!cached_scheduler) && scheduler_entry.deletes==static_cast<unsigned>(!cached_scheduler && scheduler_race),"nested scheduler cached/new/duplicate paths retain exact allocation and deletion counts");}else check(!scheduler_effects.allocations && !workers.starts && !scheduler_effects.comparisons,"cached outer socket bypasses nested scheduler entirely");
        if(!preexisting)check(entry.table_identity==0x1800845b0ULL && entry.owner_context.value==42 && entry.implementation_context.value==42 && entry.scheduler_identity==0x9988776655443322ULL && !entry.word_38 && !entry.word_40 && !entry.word_48 && service.completion_port==99,"new full-chain service retains exact finalized constructor fields and caller completion port");
        check(sdk.initializes==2*static_cast<unsigned>(!preexisting) && !sdk.gets && workers.starts==static_cast<unsigned>(!preexisting && !cached_scheduler),"actual constructor uses injected SDK gate once and caches registry service");
        const auto previous_compares=required.comparisons;check(&bridge.resolve_service_0239c0({42})==&service && required.comparisons==previous_compares+1,"second outer resolution compares cached socket without reconstructing scheduler");
        state.socket_state=0x1b;runtime.destroy_socket_024db0();
        check(api.creates==1 && api.closes==1 && executor.calls==1 && factory.owner.destroys==1 && factory.owner.deletes==1,"actual full chain creates/closes socket token and releases owner exactly once");
        check(!storage.metadata().map_size && !storage.metadata().active_size && storage.metadata().free_size==1 && storage.free_sentinel().next==original && storage.active_sentinel().next==&storage.active_sentinel(),"actual cancellation uses real0283c0 and preserves host storage rings");
        check(completion.posted.size()==(failed_post?1:3) && errors.sends==1,"full chain posts operations or switches after first failure and wakes once");
        check(scheduler.head==(failed_post?&operations[0]:nullptr) && scheduler.tail==(failed_post?&operations[2]:nullptr) && scheduler.dispatch_required==(failed_post?1:0),"fallback chain preserves all three operation addresses");
        for(unsigned i=0;i<operations.size();++i){const auto& op=operations[i];check(op.error.value==995 && op.error.category.name_identity==0x9922 && op.error.category.logical==srv::ErrorCategory::system && op.overlapped.Internal==0x7788,"actual outer close supplies exact cancellation reason while preserving borrowed payload");check(op.queued==(!failed_post || !i?1U:99U) && op.next==(failed_post && i+1<operations.size()?&operations[i+1]:nullptr),"post success and fallback retain exact operation marker/link state");}
        check(!registry_gate.depth && !engine_gate.depth && !service_gate.depth && !fallback_gate.depth && registry_gate.enters==registry_gate.leaves && engine_gate.enters==1 && engine_gate.leaves==1,"actual acceptor through real registry and recycler balances independent gates");
        check(iocp.creates==static_cast<unsigned>(!preexisting) && workers.starts==static_cast<unsigned>(!preexisting),"actual IOCP constructor runs once through nested resolver");active_iocp=nullptr;
        check(registry.head==&entry && !entry.deletes,"socket destruction leaves borrowed cached service entry alive");
        check(&storage.insert(0x10000000dULL)==original && storage.owned_nodes()==1,"real full-chain pooled node can be reused at exact address with full-width socket key");
    }
}
}
int main(){try{native_iocp();native_gate();full_chain();std::cout<<"PASS: "<<checks<<" socket IOCP constructor native checks; five SDK-typed injected imports and actual024990/023b30/0228e0/acceptor/cancellation/recycler source integration only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}





