#include "memory.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include <iostream>
#include <thread>

namespace {
namespace s=anadius::socket_recovered;namespace td=anadius::thread_domain;namespace b=b6_core_test;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
struct Unknown{};
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
void validation_and_scope(){
    Heap h;auto life=std::make_shared<Life>();auto memory=h.root->make_memory();auto* observer=memory.get();auto alias=memory;
    rejects<std::invalid_argument>([&]{s::SessionMemoryDomain bad(std::move(memory),life);},"shared backing provider rejected");
    require(memory&&memory.use_count()==2,"rejected constructor does not consume caller ownership");alias.reset();
    auto* raw=memory->allocate(17);
    rejects<std::invalid_argument>([&]{s::SessionMemoryDomain bad(std::move(memory),life);},"nonempty backing provider rejected");
    require(memory&&memory->allocation_state(raw)==td::ApplicationMemory::AllocationState::Live,"nonempty rejected provider still owns its allocation");
    ::SetLastError(0x712);require(!memory->tracking_poisoned()&&memory->allocation_state(raw)==td::ApplicationMemory::AllocationState::Live&&::GetLastError()==0x712,"read-only backing state observations preserve LastError");
    memory->deallocate(raw,17);require(memory->allocation_state(raw)==td::ApplicationMemory::AllocationState::Missing,"released pointer no longer owned");
    s::SessionMemoryDomain domain(std::move(memory),life);
    require(!memory&&domain.memory_domain_identity()==h.root.get()&&h.root->provider_leases()==1,"session domain owns dedicated backing lease");
    rejects<std::logic_error>([&]{domain.allocate_proxy_04b0bc(16);},"allocation outside whole-call Operation rejected");
    {
        auto operation=domain.enter();rejects<std::logic_error>([&]{auto second=domain.enter();},"same-thread nested Operation rejected without deadlock");
        require(!domain.close()&&domain.snapshot().phase==s::SessionMemoryPhase::Open,"active scope prevents close without stopping admission");
        rejects<std::invalid_argument>([&]{domain.allocate_proxy_04b0bc(17);},"wrong native proxy request rejected");
        rejects<std::invalid_argument>([&]{domain.allocate_control_04b0bc(32);},"wrong native control request rejected");
        rejects<std::length_error>([&]{domain.allocate_output_map(3);},"map count must be power of two");
        rejects<std::length_error>([&]{domain.allocate_output_map(0);},"production map requires positive extent");
        auto& ctx=domain.buffer_context();auto address=ctx.api.allocate_04b0bc(32);ctx.api.fill_bytes(address,0x41,32);
        require(ctx.memory.view(address,32)[31]==0x41&&observer->allocation_state(reinterpret_cast<void*>(address))==td::ApplicationMemory::AllocationState::Live,"bounded bytes are backed by real same-domain allocation");
        auto before=h.state.free_calls;
        rejects<std::invalid_argument>([&]{domain.free_sized_04b0f8(address,31);},"wrong sized free rejected before heap");
        rejects<std::invalid_argument>([&]{domain.free_sized_04b0f8(0x1234,32);},"foreign pointer rejected before heap");
        require(h.state.free_calls==before,"invalid frees never reach native SDK");domain.free_sized_04b0f8(address,32);
        rejects<std::invalid_argument>([&]{domain.free_sized_04b0f8(address,32);},"double free rejected before heap");
        domain.free_sized_04b0f8(0,16);require(domain.snapshot().allocations==0,"null native free is harmless and raw ticket drained");
    }
    require(domain.close()&&domain.close()&&h.root->provider_leases()==0,"empty domain closes idempotently and drops backing lease");
    rejects<std::logic_error>([&]{auto operation=domain.enter();},"closed domain rejects Operation");
}
void complete_owned_graph(){
    Heap h;auto life=std::make_shared<Life>();Pending pending(life);s::SessionMemoryDomain domain(h.root->make_memory(),life);s::SessionPair pair;
    {
        auto operation=domain.enter();const auto before=h.state.allocations.size();pair=domain.construct_session(pending.socket,{nullptr,executor});
        require(pair.object&&pair.control&&pair.control->strong_08==1&&pair.control->weak_0c==2,"actual023000 factory returns strong1 weak2");
        require(h.state.allocations[before].second==sizeof(s::RecoveredSessionControl)&&h.state.allocations[before+1].second==16,"typed control extent differs from native1c8 request; proxy uses16");
        require(domain.snapshot().controls==1&&domain.snapshot().proxies==1&&!domain.snapshot().partial_constructions,"owned constructor scratch discarded only after normal completion");
        auto& ctx=domain.buffer_context();s::resize_receive_buffer_010040(pair.object->input,6000,'i',ctx);
        const auto data=s::receive_buffer_address(pair.object->input);
        require((data&31)==0&&ctx.memory.view(data,6001)[5999]=='i'&&ctx.memory.view(data+6000,1)[0]==0,"real grow008350 creates aligned owned input and NUL");
        auto map=domain.allocate_output_map(512);
        require((reinterpret_cast<std::uintptr_t>(map.data())&31)==0&&map.size()==512&&map[0]==nullptr&&map[511]==nullptr,"large map pointer array has initialized typed slots and aligned storage");
        map[12]=domain.allocate_output_block();map[510]=domain.allocate_output_block();map[511]=domain.allocate_output_block();
        s::resize_receive_buffer_010040(*map[510],5000,'q',ctx);s::resize_receive_buffer_010040(*map[511],4,'r',ctx);
        pair.object->output.fields={map,512,510,2};pair.control->weak_0c.fetch_add(1);
        rejects<std::logic_error>([&]{domain.free_sized_04b0f8(reinterpret_cast<std::uintptr_t>(pair.control),0x1c8);},"live strong/weak prevent premature control free");
        strong_release(pair.control);auto state=domain.snapshot();
        require(state.controls==1&&state.allocations==1&&!state.proxies&&!state.blocks&&!state.maps&&!state.buffers,"actual B32 disposal releases all resources before observer control");
        require(state.controls_destroyed==0&&pair.control->weak_0c==1&&life->closes==1,"observer weak retains live control/context after socket close");
    }
    require(!domain.close()&&domain.snapshot().phase==s::SessionMemoryPhase::Open,"observer weak blocks close while retaining admission");
    {
        auto operation=domain.enter();weak_release(pair.control);pair={};pending.destroy();auto state=domain.snapshot();
        require(state.allocations==0&&state.controls_destroyed==1&&state.successful_allocations==state.successful_frees,"finalweak actual017420/026290 destroys control once and drains raw ledger");
        require(life->service.head==nullptr,"actual socket moves/destructors leave no list nodes");
    }
    require(domain.close()&&h.root->provider_leases()==0,"drained graph releases real memory lease");
}
void known_failures_and_reentry(){
    Heap h;auto life=std::make_shared<Life>();Pending pending(life);auto memory=h.root->make_memory();auto* backing=memory.get();s::SessionMemoryDomain domain(std::move(memory),life);
    {
        auto operation=domain.enter();h.state.fail_alloc=true;
        rejects<std::bad_alloc>([&]{domain.construct_session(pending.socket,{nullptr,executor});},"known first allocation failure propagates without guessed rollback");
        h.state.fail_alloc=false;
        require(domain.snapshot().allocations==0&&!domain.snapshot().partial_constructions&&!backing->tracking_poisoned(),"first allocation failure retains no scratch because no move ran");
        bool rejected=false,reserved=false;
        h.state.allocation_hook=[&]{reserved=domain.snapshot().reserved==1;try{domain.buffer_context();}catch(const std::logic_error&){rejected=true;}};
        auto proxy=domain.allocate_proxy_04b0bc(16);h.state.allocation_hook={};
        require(rejected&&reserved,"backing effect sees reserved ledger and cannot reenter allocation API");domain.free_sized_04b0f8(reinterpret_cast<std::uintptr_t>(proxy),16);
        const auto before=h.state.allocations.size();auto pair=domain.construct_session(pending.socket,{nullptr,executor});auto* control_raw=h.state.allocations[before].first;
        pair.control->weak_0c.fetch_add(1);strong_release(pair.control);h.state.fail_free=control_raw;
        rejects<std::system_error>([&]{weak_release(pair.control);},"known HeapFreeFALSE propagates at final control free");
        auto state=domain.snapshot();require(state.pending_frees==1&&state.controls==0&&state.controls_destroyed==1&&state.phase==s::SessionMemoryPhase::Open,"failed free retains retired raw block after exactly one typed destructor");
        require(backing->allocation_state(control_raw)==td::ApplicationMemory::AllocationState::Live,"known failure retains a live quiescent backing ticket");
        require(!domain.retry_pending_frees()&&domain.snapshot().controls_destroyed==1,"failed raw retry does not repeat C++ destructor");
        h.state.fail_free=nullptr;require(domain.retry_pending_frees()&&!domain.snapshot().allocations&&domain.snapshot().controls_destroyed==1,"explicit retry releases retained raw storage once");
        pending.destroy();
    }
    require(domain.close(),"known failures leave closable owned graph after explicit raw retry");
}
void provider_retirement(){
    Heap h;auto life=std::make_shared<Life>();s::SessionMemoryDomain* borrowed=nullptr;bool rejected=false,nested_closed=true,saw_retiring=false;
    life->retiring=[&]{saw_retiring=borrowed->snapshot().phase==s::SessionMemoryPhase::Retiring;nested_closed=borrowed->close();try{auto op=borrowed->enter();}catch(const std::logic_error&){rejected=true;}};
    std::weak_ptr<Life> observed=life;s::SessionMemoryDomain domain(h.root->make_memory(),std::move(life));borrowed=&domain;
    require(!observed.expired(),"domain retains lifecycle/service owner");
    require(domain.close()&&observed.expired()&&rejected&&!nested_closed&&saw_retiring,"provider retirement closes admission before reentrant destructor and avoids premature nested-close success");
}
void foreign_thread_scope(){
    Heap h;auto life=std::make_shared<Life>();s::SessionMemoryDomain domain(h.root->make_memory(),life);auto callbacks=h.root->make_callback_threads();bool denied=false,worked=false;
    std::thread thread([&]{
        try{auto operation=domain.enter();}catch(const std::logic_error&){denied=true;}
        callbacks->enter();{
            auto operation=domain.enter();auto& ctx=domain.buffer_context();auto p=ctx.api.allocate_04b0bc(24);ctx.api.fill_bytes(p,0x73,24);
            worked=ctx.memory.view(p,24)[23]==0x73;domain.free_sized_04b0f8(p,24);
        }callbacks->leave();
    });thread.join();callbacks.reset();
    require(denied&&worked,"foreign thread denied until backing-domain callback PTD explicitly installed; same-domain scope then works");
    require(domain.close(),"joined callback scope leaves no session memory ticket");
}
[[noreturn]]void retained_child(std::string_view mode){
    Heap h;auto life=std::make_shared<Life>();Pending pending(life);auto memory=h.root->make_memory();auto* backing=memory.get();s::SessionMemoryDomain domain(std::move(memory),life);
    if(mode=="--partial"){
        struct Hook{unsigned moves=0;} hook;
        auto move=[](void* p,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t table){executor(nullptr,to,from,table);if(++static_cast<Hook*>(p)->moves==2)faults->fail_alloc=true;};
        {auto op=domain.enter();rejects<std::bad_alloc>([&]{domain.construct_session(pending.socket,{&hook,move});},"proxy failure retains completed two-move prefix");h.state.fail_alloc=false;
         const auto state=domain.snapshot();require(state.controls==1&&state.partial_constructions==1&&state.proxies==0&&state.phase==s::SessionMemoryPhase::Open,"actual scratch and partial control retained");
         require(life->service.head&&life->service.head->next&&life->service.head->next->next==&pending.socket,"retained temporary remains a live service-list node after construct_session returned by throw");}
    }else if(mode=="--ptd"||mode=="--ptd-post"||mode=="--ptd-exit"||mode=="--ptd-free"||mode=="--ptd-restore"||mode=="--ptd-restore-free"){
        auto index=h.root->native_fls_index();auto* prior=::FlsGetValue(index);
        if(mode=="--ptd-exit"){
            {auto op=domain.enter();require(::FlsSetValue(index,anadius::crt_ptd_recovered::construction_sentinel())!=FALSE,"synthetic PTD slot changed before scope exit");}
            ::FlsSetValue(index,prior);
        }else{
            auto op=domain.enter();
            if(mode=="--ptd-restore"||mode=="--ptd-restore-free"){
                bool rejected=false;
                auto mutation=[&]{::FlsSetValue(index,anadius::crt_ptd_recovered::construction_sentinel());try{domain.buffer_context();}catch(const std::logic_error&){rejected=true;}::FlsSetValue(index,prior);};
                if(mode=="--ptd-restore"){
                    h.state.allocation_hook=mutation;
                    rejects<std::logic_error>([&]{domain.allocate_proxy_04b0bc(16);},"caught/restored PTD violation still blocks outer typed publication");h.state.allocation_hook={};
                    require(rejected&&domain.snapshot().reserved==1&&domain.snapshot().proxies==0,"sticky poison retains raw allocation even when PTD identity was restored");
                }else{
                    auto p=domain.buffer_context().api.allocate_04b0bc(32);h.state.free_hook=mutation;
                    rejects<std::logic_error>([&]{domain.free_sized_04b0f8(p,32);},"caught/restored PTD violation still blocks post-free ledger commit");h.state.free_hook={};
                    require(rejected&&domain.snapshot().pending_frees==1&&backing->allocation_state(reinterpret_cast<void*>(p))==td::ApplicationMemory::AllocationState::Missing,"sticky poison never retries physical free after callback restored PTD");
                }
            }else if(mode=="--ptd-post"){
                h.state.allocation_hook=[&]{::FlsSetValue(index,anadius::crt_ptd_recovered::construction_sentinel());};
                rejects<std::logic_error>([&]{domain.allocate_proxy_04b0bc(16);},"post-allocation PTD mutation stops typed object publication");h.state.allocation_hook={};
                require(domain.snapshot().reserved==1&&!domain.snapshot().proxies,"known raw allocation retained before typed commit after PTD mutation");
            }else if(mode=="--ptd-free"){
                auto p=domain.buffer_context().api.allocate_04b0bc(32);
                h.state.free_hook=[&]{::FlsSetValue(index,anadius::crt_ptd_recovered::construction_sentinel());};
                rejects<std::logic_error>([&]{domain.free_sized_04b0f8(p,32);},"post-free PTD mutation is retained as poisoned context");h.state.free_hook={};
                require(domain.snapshot().pending_frees==1&&backing->allocation_state(reinterpret_cast<void*>(p))==td::ApplicationMemory::AllocationState::Missing,"physically freed block is not eligible for retry after PTD mismatch");
            }else{
                ::FlsSetValue(index,anadius::crt_ptd_recovered::construction_sentinel());rejects<std::logic_error>([&]{domain.allocate_proxy_04b0bc(16);},"PTD mutation rejected before allocation");
            }
            ::FlsSetValue(index,prior);
        }
        require(domain.snapshot().phase==s::SessionMemoryPhase::Poisoned,"observed PTD identity mismatch remains poisoned even after caller restores slot");
    }else{
        {auto op=domain.enter();auto& ctx=domain.buffer_context();
         if(mode=="--unknown-alloc"){
             h.state.throw_alloc=true;rejects<Unknown>([&]{ctx.api.allocate_04b0bc(32);},"unknown native allocation outcome propagated");h.state.throw_alloc=false;
             require(domain.snapshot().reserved==1&&backing->tracking_poisoned(),"unknown allocation retains reservation and backing poison");
         }else{
             auto p=ctx.api.allocate_04b0bc(32);h.state.throw_after_free=true;
             rejects<Unknown>([&]{domain.free_sized_04b0f8(p,32);},"physical free then throw yields unknown outcome");h.state.throw_after_free=false;
             require(backing->allocation_state(reinterpret_cast<void*>(p))==td::ApplicationMemory::AllocationState::Unknown,"backing reports Unknown rather than safe retry after physical free");
         }
         const auto before=h.state.free_calls;rejects<std::logic_error>([&]{domain.retry_pending_frees();},"poison forbids raw retry");require(before==h.state.free_calls,"unknown outcome never physically freed again");}
        require(domain.snapshot().phase==s::SessionMemoryPhase::Poisoned,"unknown outcome poisons session domain");
    }
    require(!domain.close(),"partial/unknown state cannot pretend to close");
    // Deliberate retention of own child-only failed state; do not run unsafe
    // C++ teardown or invent native EH recovery. OS reclaims this own process.
    std::cout<<"retained child verified "<<mode<<'\n'<<std::flush;std::_Exit(0);
}
void child_checks(){
    std::array<wchar_t,32768> executable{};if(!::GetModuleFileNameW(nullptr,executable.data(),static_cast<DWORD>(executable.size())))throw std::runtime_error("own executable path unavailable");
    for(const auto* mode:{L"--partial",L"--unknown-alloc",L"--unknown-free",L"--ptd",L"--ptd-post",L"--ptd-exit",L"--ptd-free",L"--ptd-restore",L"--ptd-restore-free"}){
        std::wstring command=L"\""+std::wstring(executable.data())+L"\" "+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};
        if(!::CreateProcessW(executable.data(),command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process))throw std::runtime_error("own child launch failed");
        const auto waited=::WaitForSingleObject(process.hProcess,30000);DWORD code=99;
        if(waited!=WAIT_OBJECT_0)::TerminateProcess(process.hProcess,98);else ::GetExitCodeProcess(process.hProcess,&code);
        ::CloseHandle(process.hThread);::CloseHandle(process.hProcess);require(waited==WAIT_OBJECT_0&&code==0,"own retained-prefix child passed without loading any DLL target");
    }
}
}
int main(int argc,char** argv){try{
    if(argc==2)retained_child(argv[1]);
    validation_and_scope();complete_owned_graph();known_failures_and_reentry();provider_retirement();foreign_thread_scope();child_checks();
    std::cout<<"PASS: "<<checks<<" owned session-memory controls\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"unexpected failure\n";return 1;}}
