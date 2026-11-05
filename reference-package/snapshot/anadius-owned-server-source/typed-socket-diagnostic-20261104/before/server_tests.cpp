#include "server.hpp"
#include "../anadius-socket-source/native.hpp"
#include "../anadius-socket-source/async_receive_test_fixture.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <iostream>
#include <thread>
#include <limits>
#include <array>

namespace {
namespace own=anadius::owned_server;namespace s=anadius::socket_recovered;namespace td=anadius::thread_domain;namespace b=b6_core_test;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<"\n";throw std::runtime_error(text);}}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
struct Unknown{};
void typed_source_error_values(){
    const s::Category misc{0x3333,anadius::server_recovered::ErrorCategory::asio_misc};
    std::string operation="open";own::SourceError error({1,misc},operation);operation="changed";
    require(error.code().value()==1&&&error.code().category()==&s::misc_error_category()&&error.code().message()=="Already open","Typed owned-server error uses actual shared misc message/category rather than Windows system code1");
    require(error.source_error().value==1&&error.source_error().category.name_identity==misc.name_identity&&error.source_error().category.logical==misc.logical&&error.operation()=="open"&&!error.context_address(),"Typed source error retains complete source category/code and owns operation text");
    bool caught=false;try{throw error;}catch(const std::system_error& base){
        const auto* typed=dynamic_cast<const own::SourceError*>(&base);caught=typed&&typed->source_error().category.name_identity==0x3333&&std::string_view(base.what()).find("open")!=std::string_view::npos&&std::string_view(base.what()).find("Already open")!=std::string_view::npos;
    }require(caught,"SourceError remains catchable as std::system_error after copy/throw while retaining its typed metadata");
    own::SourceError bare({1,misc});
    const std::system_error compiler_bare(1,s::misc_error_category());
    require(bare.operation().empty()&&!bare.context_address()&&bare.source_error().category.name_identity==misc.name_identity&&bare.code()==error.code(),"028210 projection retains typed error without an operation or context address");
    require(std::string_view(bare.what())==compiler_bare.what(),"Context-free source error uses compiler's no-context system_error overload");
    const s::Category system{0x2222,anadius::server_recovered::ErrorCategory::system};
    const auto& native=s::system_error_category_028620();
    struct Case {std::int32_t value;std::uintptr_t address;const char* context;};
    for(const auto& item:std::array<Case,2>{{{std::numeric_limits<std::int32_t>::min(),0x180084428ULL,"mutex"},{5,0x180084528ULL,"iocp"}}}){
        // A separate SDK oracle is computed once; the constructor's own SDK
        // calls are not intercepted or claimed to have been counted here.
        const auto expected=std::string(item.context)+": "+native.message(item.value);std::string operation=item.context;
        own::SourceError selected({item.value,system},operation,item.address);operation="changed";
        require(selected.code().value()==item.value&&&selected.code().category()==&native&&std::string_view(native.name())=="asio.system"&&selected.source_error().value==item.value&&selected.source_error().category.name_identity==system.name_identity&&selected.source_error().category.logical==system.logical,"Known constructor context preserves signed raw value and opaque source token while selecting native system category");
        require(selected.operation()==item.context&&selected.context_address()==item.address&&std::string(selected.what())==expected,"Known mutex/iocp context stores owned operation and address with complete native SDK diagnostic");
        const own::SourceError copy=selected;const std::system_error base=selected;const std::runtime_error sliced=selected;
        require(copy.operation()==item.context&&copy.context_address()==item.address&&copy.source_error().category.name_identity==system.name_identity&&std::string(copy.what())==expected&&std::string(base.what())==expected&&std::string(sliced.what())==expected&&&base.code().category()==&native,"Typed copy, system_error base copy and runtime_error value slice own the same native diagnostic without re-querying message text");
        bool caught=false;try{throw copy;}catch(const std::system_error& caught_base){const auto* typed=dynamic_cast<const own::SourceError*>(&caught_base);caught=typed&&typed->context_address()==item.address&&caught_base.code().value()==item.value&&&caught_base.code().category()==&native&&std::string(caught_base.what())==expected;}require(caught,"Native constructor diagnostic retains typed metadata through ordinary system_error catch");
    }
    own::SourceError caller_context({5,system},"caller-owned",0x180084528ULL);require(caller_context.operation()=="caller-owned"&&std::string_view(caller_context.what()).starts_with("caller-owned: ")&&&caller_context.code().category()==&native,"SourceError owns its caller operation rather than overriding text from a numeric address");
    const std::system_error compiler_system(5,std::system_category(),"host");
    own::SourceError unknown({5,system},"host",0x180099999ULL),no_address({5,system},"host"),context_free({5,system});const std::system_error compiler_context_free(5,std::system_category());
    require(&unknown.code().category()==&std::system_category()&&&no_address.code().category()==&std::system_category()&&unknown.context_address()==0x180099999ULL&&std::string(unknown.what())==compiler_system.what()&&std::string(no_address.what())==compiler_system.what(),"Unknown and address-free HOST operations retain compiler system category and original operation formatting");
    require(&context_free.code().category()==&std::system_category()&&context_free.operation().empty()&&!context_free.context_address()&&std::string(context_free.what())==compiler_context_free.what(),"Context-free logical system projection preserves its original compiler category/no-context overload");
    const s::Category other{0x7777,anadius::server_recovered::ErrorCategory::other};own::SourceError non_system({5,other},"host",0x180084428ULL),known_misc({1,misc},"host",0x180084528ULL);
    require(&non_system.code().category()==&std::system_category()&&std::string(non_system.what())==compiler_system.what()&&non_system.source_error().category.name_identity==other.name_identity&&&known_misc.code().category()==&s::misc_error_category()&&std::string(known_misc.what())=="host: Already open","Known numeric context alone does not reinterpret other/misc category inputs as native system errors");
}
struct Faults {
    bool fail_alloc=false,throw_alloc=false,throw_after_free=false;
    void* fail_free=nullptr;unsigned free_calls=0;
    std::vector<std::pair<void*,std::size_t>> allocations;
    std::function<void()> allocation_hook;
    std::function<void()> free_hook;
};
Faults* faults=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T bytes){
    if(heap!=::GetProcessHeap()||(flags!=0&&flags!=HEAP_ZERO_MEMORY))throw std::logic_error("unexpected heap allocation contract");
    ++b::observed->allocations;
    if(faults->fail_alloc){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;}
    auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();
    if(raw){b::observed->blocks.emplace(raw,bytes);faults->allocations.emplace_back(raw,bytes);}
    if(faults->allocation_hook)faults->allocation_hook();
    if(faults->throw_alloc)throw Unknown{};
    ::SetLastError(error);return raw;
}
BOOL WINAPI release(HANDLE heap,DWORD flags,void* raw){
    ++faults->free_calls;
    if(heap!=::GetProcessHeap()||flags||!b::observed->blocks.contains(raw))throw std::logic_error("unknown/double raw heap free");
    if(faults->fail_free==raw){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    const auto result=::HeapFree(heap,flags,raw);const auto error=::GetLastError();
    if(result){b::observed->blocks.erase(raw);++b::observed->frees;}
    if(faults->free_hook)faults->free_hook();
    if(faults->throw_after_free)throw Unknown{};
    ::SetLastError(error);return result;
}
struct Heap {
    b::Observation observed;b::RuntimeState runtime;Faults state;std::shared_ptr<td::DomainStorage> root;
    Heap(){
        if(b::observed||b::runtime_state||faults)throw std::logic_error("overlapping heap fixture");
        b::observed=&observed;b::runtime_state=&runtime;faults=&state;
        auto api=b::apis();api.heap.heap_alloc=allocate;api.heap.heap_free=release;
        root=td::DomainStorage::create(b::seed(),api);
        if(!root->initialize())throw std::runtime_error("own FLS/heap initialization failed");
    }
    ~Heap(){
        state.fail_alloc=false;state.fail_free=nullptr;state.throw_after_free=false;state.throw_alloc=false;state.allocation_hook={};
        if(!root->close())std::terminate();root.reset();
        if(!observed.blocks.empty())std::terminate();faults=nullptr;b::observed=nullptr;b::runtime_state=nullptr;
    }
};
struct Gate:s::SocketGate {void enter()override{}void leave()override{}};
struct Life:s::LifecycleEffects {
    Gate gate;s::SocketService service{gate};unsigned closes=0;
    std::function<void()> retiring;
    ~Life(){if(retiring)retiring();}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService&,s::SocketObject& socket)override{if(socket.state.socket!=s::invalid_socket)++closes;socket.state.socket=s::invalid_socket;}
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}
};
void executor(void*,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t){to.context=from.context;to.zero_74=from.zero_74;to.self_reference_80=true;}
void weak_release(s::IoReference* value){if(value&&value->weak_0c.fetch_sub(1)==1)value->delete_slot_08();}
void strong_release(s::IoReference* value){if(value&&value->strong_08.fetch_sub(1)==1){value->dispose_slot_00();weak_release(value);}}
struct Pending {
    std::shared_ptr<Life> life;s::AcceptorState state;s::SocketObject socket{state};
    explicit Pending(std::shared_ptr<Life> l):life(std::move(l)){s::initialize_0211b0(socket,{77},*life);state.socket=77;}
    void destroy(){s::destroy_024db0(socket,*life);}
};
[[noreturn]]void unused(){throw std::logic_error("Unselected owned IO dependency");}
struct Table:s::IoHandlerVtable {
    Table():IoHandlerVtable(0x777){}unsigned moves=0,destroys=0;
    void destroy_slot_00(s::IoHandlerEnvironment&)override{++destroys;}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{++moves;to.payload_00=from.payload_00;to.state_18=from.state_18;}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{unused();}
};
struct Effects:s::IoCompletionEffects {
    unsigned receives=0,sends=0;std::function<void()> receive,send;
    s::Category misc_category_0285b0()override{return {0x3333};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{++receives;if(receive)receive();}
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{++sends;if(send)send();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unused();}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unused();}
};
struct Send:s::AsyncSendApi {
    unsigned calls=0;s::AsyncSendResult answer{-1,997,0};std::function<void()> hook;
    s::AsyncSendResult send(s::Socket socket,s::Buffer& buffer,std::uint32_t flags,OVERLAPPED& ov)override{
        require(socket==77&&buffer.len==3&&!flags&&!ov.Internal&&!ov.InternalHigh&&!ov.Pointer&&!ov.hEvent,"selected SDK send gets live three-byte buffer and true zeroed OVERLAPPED");
        ++calls;if(hook)hook();return answer;
    }
};
struct Functor:s::PostedFunctor {unsigned calls=0;void call_slot_00(std::uint8_t flag)override{require(flag==1,"posted completion invokes moved functor");++calls;}};
struct Fixture {
    Heap heap;std::shared_ptr<Life> life=std::make_shared<Life>();Pending pending{life};
    std::shared_ptr<s::SessionMemoryDomain> root=std::make_shared<s::SessionMemoryDomain>(heap.root->make_memory(),life);
    s::OwnedIoMemory memory;s::SessionPair pair;std::uint32_t index=17;
    receive_test::Worker worker;receive_test::Errors errors;Effects effects;Table table;
    s::IoHandlerExecutor executor;s::WorkerThreadState thread{};s::WorkerTlsContext tls{nullptr,&thread,nullptr};
    explicit Fixture(std::size_t capacity=8):memory(root,capacity){auto operation=root->enter();pair=root->construct_session(pending.socket,{nullptr,::executor});}
    s::IoCompletionEnvironment environment(){return {{index,memory.storage_bindings()},{&table,0x846e8,&executor,0x777}};}
    s::IoHandlerEnvironment handler_environment(){return {{1,2},&table,0,0x846e8,&executor};}
    s::Buffer buffer(){s::resize_receive_buffer_010040(pair.object->input,3,'i',root->buffer_context());s::Buffer value;value.len=3;value.buf=reinterpret_cast<char*>(s::receive_buffer_address(pair.object->input));return value;}
    void receive_owner(s::AsyncReceiveOperation& op){pair.control->strong_08.fetch_add(1);pair.control->weak_0c.fetch_add(1);op.io.handler.control_a8=pair.control;op.io.handler.object_a0=reinterpret_cast<std::uintptr_t>(pair.object);op.io.weak_48=pair.control;op.io.environment_b0=handler_environment();}
    void send_owner(s::AsyncSendOperation& op){pair.control->strong_08.fetch_add(1);pair.control->weak_0c.fetch_add(1);op.io.handler.control_98=pair.control;op.io.handler.object_90=reinterpret_cast<std::uintptr_t>(pair.object);op.io.weak_40=pair.control;op.io.environment_a0=handler_environment();}
    void drain_terminal_cache(){for(const auto identity:thread.cache_00)if(identity)memory.free_cached_identity(identity);}
    void finish(){
        {auto operation=root->enter();require(memory.snapshot().allocations==0,"all IO allocations drained before session disposal");strong_release(pair.control);pair={};pending.destroy();require(root->snapshot().allocations==0,"actual B32 disposal drains controls/proxy/input after IO");}
        require(!root->close()&&root->snapshot().provider_leases==1,"idle concrete IO provider keeps parent open");
        require(memory.close()&&memory.close()&&root->snapshot().provider_leases==0,"IO close retires provider lease idempotently");
        require(root->close()&&heap.root->provider_leases()==0,"session close drains dedicated real application-memory lease");
    }
};
struct Physical:s::DispatcherApi {
    unsigned gets=0,errors=0,posts=0;std::uint32_t status=0;std::function<void()> waiting,error_read,posting;
    void set_last_error(std::uint32_t n)override{status=n;}
    std::uint32_t get_last_error()override{++errors;if(error_read)error_read();return status;}
    std::int32_t get(std::uintptr_t p,std::uint32_t& n,std::uintptr_t& key,OVERLAPPED*& raw,std::uint32_t timeout)override{
        require(p==99&&timeout==17,"admitted physical get preserves caller port/timeout");++gets;if(waiting)waiting();n=7;key=2;raw=nullptr;status=0x80000005U;return 0;
    }
    std::int32_t post(std::uintptr_t p,std::uint32_t n,std::uintptr_t key,OVERLAPPED* raw)override{require(p==99&&n==1&&key==2&&!raw,"admitted nonblocking post preserves arguments");++posts;if(posting)posting();return 1;}
    std::int32_t set_timer(std::uintptr_t,LARGE_INTEGER&,std::int32_t,PTIMERAPCROUTINE,void*,std::int32_t)override{unused();}
};
void wait_boundary_and_error(){
    Fixture f;auto physical=std::make_shared<Physical>();
    rejects<std::invalid_argument>([&]{own::AdmittedDispatcherApi bad(f.root,nullptr);},"null physical dispatcher rejected without taking root lease");
    {own::AdmittedDispatcherApi admitted(f.root,physical);
     require(f.root->snapshot().provider_leases==2,"worker admission holds independent root lease beside concrete IO provider");
     (void)f.root->buffer_context();rejects<std::logic_error>([&]{auto nested=f.root->enter();},"whole source chain starts with parent Operation held");
     physical->waiting=[&]{
        auto independent=f.root->enter();auto& context=f.root->buffer_context();auto p=context.api.allocate_04b0bc(24);context.api.fill_bytes(p,0x69,24);f.root->free_sized_04b0f8(p,24);
        rejects<std::logic_error>([&]{admitted.post(99,1,2,nullptr);},"admitted wrapper cannot be reentered while physical wait owns no Operation");
     };
     physical->error_read=[&]{auto independent=f.root->enter();require(!f.root->close(),"physical last-error read also occurs before packet admission reacquires root");};
     std::uint32_t n=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;admitted.set_last_error(0);
     require(!admitted.get(99,n,key,raw,17)&&n==7&&key==2&&!raw,"physical wait BOOL and packet fields are returned unchanged");
     (void)f.root->buffer_context();require(physical->gets==1&&physical->errors==1,"exactly one immediate lower last-error read per physical wait");
     rejects<std::logic_error>([&]{admitted.get(99,n,key,raw,17);},"unconsumed captured error prevents a second physical wait");
     physical->status=19;require(admitted.get_last_error()==0x80000005U&&physical->errors==1,"full-width SDK status is cached across metadata/reacquire and later lower mutation");
     require(admitted.post(99,1,2,nullptr)==1&&physical->posts==1,"nonblocking source publication keeps Operation held");
     bool denied=false;std::thread wrong([&]{try{admitted.post(99,1,2,nullptr);}catch(const std::logic_error&){denied=true;}});wrong.join();
     require(denied&&physical->posts==1,"foreign worker cannot use another worker's admitted dispatcher");
     require(!admitted.poisoned(),"rejected invalid admission calls do not poison a known clean physical wait");
    }
    require(f.root->snapshot().provider_leases==1,"worker scope releases Operation and metadata lease after source loop exits");
    {auto native=std::make_shared<s::NativeDispatcherApi>();own::AdmittedDispatcherApi admitted(f.root,native);
     require(!admitted.post(~std::uintptr_t{0},0,0,nullptr)&&admitted.get_last_error()==ERROR_INVALID_HANDLE,"real failed PostQueuedCompletionStatus error survives parent metadata/PTD checkpoint");}
    f.finish();
}
void wait_maintenance_boundary(){
    Fixture f;auto physical=std::make_shared<Physical>();unsigned calls=0;
    {own::AdmittedDispatcherApi admitted(f.root,physical);
     struct Probe {Fixture& f;Physical& physical;unsigned& calls;static void check(void* raw){auto& p=*static_cast<Probe*>(raw);++p.calls;require(p.physical.errors==1,"wait maintenance occurs after immediate SDK error capture");bool outside=false;try{(void)p.f.root->buffer_context();}catch(const std::logic_error&){outside=true;}require(outside,"maintenance has no Parent Operation held");{auto independent=p.f.root->enter();(void)p.f.root->buffer_context();}p.physical.status=17;::SetLastError(0x1234);}} probe{f,*physical,calls};
     admitted.bind_wait_maintenance({&probe,Probe::check});
     rejects<std::logic_error>([&]{admitted.bind_wait_maintenance({&probe,Probe::check});},"live wait maintenance cannot be replaced");
     std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;admitted.set_last_error(0);
     require(!admitted.get(99,bytes,key,raw,17)&&calls==1&&bytes==7&&key==2&&!raw,"maintenance preserves returned physical packet fields");
     require(admitted.get_last_error()==0x80000005U&&physical->errors==1,"maintenance cannot overwrite captured physical wait status");
     (void)f.root->buffer_context();admitted.clear_wait_maintenance();
     admitted.set_last_error(0);require(!admitted.get(99,bytes,key,raw,17)&&calls==1,"cleared POD context is not retained or invoked again");(void)admitted.get_last_error();
    }f.finish();
}
void real_wait_concurrent_source(){
    Fixture f;auto callbacks=f.heap.root->make_callback_threads();auto native=std::make_shared<s::NativeDispatcherApi>();
    HANDLE port=::CreateIoCompletionPort(INVALID_HANDLE_VALUE,nullptr,0,0);require(port!=nullptr,"own real Win32 IOCP acquired without creating a network socket");
    OVERLAPPED packet{};bool worked=false,attached=false;std::exception_ptr producer_error;
    {own::AdmittedDispatcherApi admitted(f.root,native);
     std::thread producer([&]{try{
        callbacks->enter();attached=true;
        {auto source=f.root->enter();auto& context=f.root->buffer_context();auto p=context.api.allocate_04b0bc(32);context.api.fill_bytes(p,0x42,32);worked=context.memory.view(p,32)[31]==0x42;f.root->free_sized_04b0f8(p,32);}
        if(!::PostQueuedCompletionStatus(port,17,2,&packet))throw std::runtime_error("own IOCP producer post failed");
        if(!callbacks->leave())throw std::runtime_error("foreign source PTD scope did not leave");attached=false;
     }catch(...){producer_error=std::current_exception();::PostQueuedCompletionStatus(port,0,0,nullptr);}});
     std::uint32_t bytes=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;admitted.set_last_error(0);
     const auto received=admitted.get(reinterpret_cast<std::uintptr_t>(port),bytes,key,raw,10000);const auto status=admitted.get_last_error();producer.join();
     if(producer_error)std::rethrow_exception(producer_error);
     require(received&&status==0&&bytes==17&&key==2&&raw==&packet,"actual GQCS returns producer packet with immediate preserved status");
     require(worked&&!attached,"same-domain callback thread allocates/frees while worker physically waits; no global ROOT deadlock");
     (void)f.root->buffer_context();rejects<std::logic_error>([&]{auto nested=f.root->enter();},"actual wait returns only after reacquiring source admission");
    }
    callbacks.reset();require(::CloseHandle(port)!=FALSE,"own real IOCP handle closed after producer and admitted worker drain");f.finish();
}
[[noreturn]]void unknown_wait_child(bool post){
    Fixture f;auto physical=std::make_shared<Physical>();own::AdmittedDispatcherApi admitted(f.root,physical);
    auto buffer=f.buffer();auto& storage=s::allocate_io_storage_0269b0(0xe0,f.environment().storage,f.worker,f.memory);auto& op=f.memory.project_async_receive(storage);f.receive_owner(op);
    receive_test::Receive lower;s::PinnedAsyncReceiveApi sdk(f.memory,lower);sdk.receive(77,buffer,0,op.io.operation.overlapped);
    physical->waiting=[](){throw Unknown{};};physical->posting=physical->waiting;std::uint32_t n=0;std::uintptr_t key=0;OVERLAPPED* raw=nullptr;
    rejects<Unknown>([&]{if(post)admitted.post(99,1,2,nullptr);else admitted.get(99,n,key,raw,17);},"unknown physical wait or publication propagates without guessing packet ownership");
    require(admitted.poisoned()&&f.memory.snapshot().pending==1&&f.root->snapshot().buffer_borrows==1,"unknown wait keeps existing SDK raw projection, buffer loan and extra owner strong");
    rejects<std::logic_error>([&]{admitted.post(99,1,2,nullptr);},"poisoned wait cannot resume source dispatch");
    require(!f.memory.close()&&!f.root->close(),"unknown wait cannot pretend to close pending IO or root");
    std::cout<<"retained unknown wait verified\n"<<std::flush;std::_Exit(0);
}
void child_check(const wchar_t* mode){
    std::array<wchar_t,32768> executable{};if(!::GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size())))unused();
    std::wstring command=L"\""+std::wstring(executable.data())+L"\" "+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
    if(!::CreateProcessW(executable.data(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process))unused();
    auto waited=::WaitForSingleObject(process.hProcess,30000);DWORD code=99;if(waited!=WAIT_OBJECT_0)::TerminateProcess(process.hProcess,98);else ::GetExitCodeProcess(process.hProcess,&code);
    ::CloseHandle(process.hThread);::CloseHandle(process.hProcess);require(waited==WAIT_OBJECT_0&&code==0,"bounded own child passed source concurrency or retained unknown wait scenario");
}
} // namespace
int main(int argc,char** argv){try{
    if(argc==2){if(std::string_view(argv[1])=="--unknown-wait")unknown_wait_child(false);if(std::string_view(argv[1])=="--unknown-post")unknown_wait_child(true);real_wait_concurrent_source();return 0;}
    typed_source_error_values();wait_boundary_and_error();wait_maintenance_boundary();child_check(L"--concurrent-source");child_check(L"--unknown-wait");child_check(L"--unknown-post");
    std::cout<<"PASS: "<<checks<<" owned server worker-admission controls\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected server admission failure\n";return 1;}}
