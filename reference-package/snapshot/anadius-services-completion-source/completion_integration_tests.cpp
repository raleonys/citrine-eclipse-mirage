#include "completion_test_fixture.hpp"
#include <set>

namespace {
using namespace b10_test;
struct Queue {
    std::shared_ptr<sc::ExitWaitRegistry> registry;
    std::array<std::shared_ptr<sc::WaitTarget>,3> targets;
    std::array<std::atomic<DWORD>,3> ids{};
    std::set<std::uintptr_t> resumed;
    unsigned creates=0,closes=0;
    bool timeout_once=true;
    std::atomic<unsigned> registrations=0,payload_deletes=0;
    std::atomic<bool> cleanup_before_delete=true;
};
Queue* queue=nullptr;
struct Launch {LPTHREAD_START_ROUTINE start;void* argument;unsigned index;};
DWORD WINAPI registered_start(void* parameter){
    const auto saved=*static_cast<Launch*>(parameter);
    delete static_cast<Launch*>(parameter); // Source entry may ExitThread.
    try{
        auto* target=queue->targets[saved.index].get();target->lock();
        queue->registry->register_current_thread(queue->targets[saved.index]);
        queue->ids[saved.index]=::GetCurrentThreadId();++queue->registrations;
    }catch(...){std::_Exit(96);}
    return saved.start(saved.argument);
}
HANDLE WINAPI queued_create(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE start,LPVOID argument,DWORD flags,LPDWORD id){
    require(!security&&!stack&&start&&argument&&!flags&&id,"actual Bootstrap passes observed CreateThread operands");
    const auto index=queue->creates++;if(index>=3)std::terminate();
    auto* launch=new Launch{start,argument,index};
    const auto handle=::CreateThread(security,stack,registered_start,launch,CREATE_SUSPENDED,id);
    const auto error=::GetLastError();if(!handle)delete launch;::SetLastError(error);return handle;
}
DWORD WINAPI resume_wait(HANDLE handle,DWORD timeout){
    require(timeout==10000,"B3 keeps configured finite worker join timeout");
    if(queue->timeout_once){queue->timeout_once=false;return WAIT_TIMEOUT;}
    const bool first=queue->resumed.insert(reinterpret_cast<std::uintptr_t>(handle)).second;
    const auto old=first?::ResumeThread(handle):1;
    const auto result=::WaitForSingleObject(handle,timeout);
    require(old==1&&result==WAIT_OBJECT_0,"actual CRT worker and FLS cleanup complete before join succeeds");
    return result;
}
BOOL WINAPI join_close(HANDLE handle){++queue->closes;return ::CloseHandle(handle);}
BOOL WINAPI observed_free(HANDLE heap,DWORD flags,LPVOID pointer){
    const auto error=::GetLastError();SIZE_T size=0;
    {std::lock_guard lock(b7_test::memory_sdk->gate);const auto it=b7_test::memory_sdk->blocks.find(pointer);if(it!=b7_test::memory_sdk->blocks.end())size=it->second;}
    if(queue&&(size==8||size==16)){
        const auto tid=::GetCurrentThreadId();bool found=false;
        for(unsigned i=0;i<3;++i)if(queue->ids[i].load()==tid){
            const auto& target=queue->targets[i];
            found=target->ready_value_quiescent()==1&&!target->is_locked()&&!target->registered();break;
        }
        if(!found)queue->cleanup_before_delete=false;
        ++queue->payload_deletes;
    }
    ::SetLastError(error);return b7_test::heap_free(heap,flags,pointer);
}
void full_runtime(){
    b7_test::MemorySdk observation;b7_test::memory_sdk=&observation;
    auto api=b7_test::domain_api();api.heap.heap_free=observed_free;
    auto domain=thread_domain::DomainStorage::create(b7_test::domain_seed(),api);
    require(domain->initialize(),"full Services graph owns actual B6 initial-locale domain");
    auto memory=domain->make_memory();auto exits=registry();auto sink=std::make_shared<Sink>();auto log=logger(sink);
    Queue pending;pending.registry=exits;for(auto& target:pending.targets)target=std::make_shared<sc::WaitTarget>(1,0);queue=&pending;
    {
        b4_test::Fixture fixture;auto resources=fixture.resources();
        resources.remaining=std::make_unique<sc::CompletionServices>(log,exits,
            std::make_unique<bootstrap_policies::SourceServices>(memory,std::move(resources.remaining)));
        auto threads=b7_test::thread_api();threads.create=queued_create;resources.threads=domain->make_threads(threads);
        resources.join_api=bootstrap_backend::native_join_api();resources.join_api.wait=resume_wait;resources.join_api.close=join_close;resources.join_timeout_ms=10000;
        fixture.construct(std::move(resources));fixture.runtime->publish();
        require(fixture.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"B4 Runtime attaches through concrete B10 Services and actual recovered threads");
        require(sink->buffer.str()=="LSX emu version 2.0.1.0\n"&&sink->buffer.syncs==1,"actual source Bootstrap logs exact retained startup line through owned ostream");
        require(pending.creates==3&&memory->outstanding_allocations()==3&&observation.size()==7,"three suspended workers retain real source Contexts and same-domain payloads");
        require(exits->active()==0&&pending.registrations==0,"unstarted workers have no fabricated exit-wait entries");
        require(!fixture.runtime->close()&&fixture.runtime->phase()==bootstrap_owner::Phase::Stopping,"initial join timeout retains application owner and selected policies");
        require(memory->outstanding_allocations()==3&&fixture.cleanup->uninitializations==0,"timeout cannot release payloads or run post-join source cleanup");
        require(fixture.runtime->close()&&pending.closes==3,"retry joins every actual CRT worker before releasing native Binding");
        require(pending.registrations==3&&pending.payload_deletes==3&&pending.cleanup_before_delete,"each source worker exit cleanup finishes before Bootstrap payload deletion");
        for(const auto& target:pending.targets)require(target->ready_value_quiescent()==1&&!target->is_locked()&&!target->registered(),"joined source workers leave consumed owned exit targets");
        require(exits->active()==0&&memory->outstanding_allocations()==0&&observation.size()==1&&observation.valid,"registry, source payloads and worker FLS PTDs finish before controller teardown");
        require(b4_test::absent(fixture.track.events,"bootstrap-log-part")&&b4_test::absent(fixture.track.events,"bootstrap-log-finish")&&b4_test::absent(fixture.track.events,"cleanup-thread"),"five selected B10 routes bypass coarse lower log/exit-cleanup policies");
        require(fixture.runtime->configuration().username=="network-user"&&fixture.runtime->configuration().server_port==0x1234,"actual source workers still share one application state");
        require(fixture.library_os->unloaded==std::vector<std::uintptr_t>({0x777,0x501,0x502})&&fixture.cleanup->uninitializations==1,"source library/online/hook cleanup runs once after actual joins");
        require(!domain->close()&&domain->provider_leases()==2,"closed Runtime and explicit allocator retain domain leases until providers die");
        fixture.runtime.reset();require(domain->provider_leases()==1,"Runtime destruction releases thread provider before domain close");
    }
    require(exits->close(),"explicit exit registry closes after all worker joins");
    memory.reset();require(domain->close()&&observation.size()==0&&observation.valid,"application allocator and actual main FLS/PTD close after selected Services destruction");
    require(domain->locale_references()==1&&domain->multibyte_references()==0,"source initial-data counters return to baseline");
    domain.reset();queue=nullptr;b7_test::memory_sdk=nullptr;
}
}
int main(){try{
    full_runtime();
    std::cout<<"PASS: "<<b10_test::checks<<" services-completion integration checks; actual CRT worker cleanup before payload deletion, owned logging, B3/B4 join retry and final FLS teardown\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
