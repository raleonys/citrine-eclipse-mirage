#include "domain.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <map>
#include <mutex>
#include <array>
#include <atomic>
#include <set>

namespace {
using namespace anadius;namespace d=thread_domain;namespace t=threads_recovered;
using b2_test::require;using b2_test::rejects;
struct HeapObservation {
    std::mutex gate;std::map<void*,std::size_t> blocks;std::atomic<bool> valid=true;
    std::size_t size(){std::lock_guard lock(gate);return blocks.size();}
};
HeapObservation* heap_observation=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T size){
    auto* value=::HeapAlloc(heap,flags,size);const auto error=::GetLastError();
    if(value){std::lock_guard lock(heap_observation->gate);if(!heap_observation->blocks.emplace(value,size).second)heap_observation->valid=false;}
    ::SetLastError(error);return value;
}
BOOL WINAPI free_heap(HANDLE heap,DWORD flags,LPVOID value){
    // Keep removal atomic with the real free: another allocating worker can
    // receive the same address immediately after HeapFree returns.
    BOOL result;DWORD error;
    {const std::lock_guard lock(heap_observation->gate);
        result=::HeapFree(heap,flags,value);error=::GetLastError();
        if(result&&heap_observation->blocks.erase(value)!=1)heap_observation->valid=false;
    }
    ::SetLastError(error);return result;
}
d::Apis observed_api(){d::Apis api;api.heap.heap_alloc=allocate;api.heap.heap_free=free_heap;return api;}
d::Seed seed(){
    auto identity=std::make_shared<std::array<std::byte,8>>();
    return {0x31c872450ab966d1ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity};
}
// Our callbacks live in this own EXE, whose lifetime is process-wide. The
// module pin boundary is deliberately controlled; all thread/FLS/heap calls
// below and the entire recovered ThreadEffects graph are real source paths.
BOOL WINAPI no_module(DWORD flags,LPCWSTR address,HMODULE* result){
    require(flags==4&&address&&result,"source module lookup operands retained");*result=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;
}
thread_native::Api thread_api(){auto api=thread_native::native_api();api.get_module=no_module;return api;}
struct WorkerPlan {
    DWORD index;crt_ptd_recovered::Ptd* main_ptd;bootstrap_providers::NativeThreads* effects;
    HANDLE release,all_ready;std::atomic<unsigned>* ready;unsigned total;
    std::uint32_t error;std::uintptr_t exception,locale,mb;
    std::atomic<crt_ptd_recovered::Ptd*> ptd=nullptr;std::atomic<bool> valid=false;
};
unsigned __stdcall own_worker(void* argument){
    auto& plan=*static_cast<WorkerPlan*>(argument);
    try{
        auto* ptd=static_cast<crt_ptd_recovered::Ptd*>(::FlsGetValue(plan.index));
        bool valid=ptd&&ptd!=plan.main_ptd;
        if(ptd){
            const crt_ptd_data_recovered::View view(ptd);
            valid=valid&&view.load<std::uintptr_t>(0)==plan.exception&&view.load<std::uintptr_t>(0x88)==plan.mb&&view.load<std::uintptr_t>(0x90)==plan.locale;
            valid=valid&&ptd->thread_context&&view.load<std::uint32_t>(0x20)==0;
            ::SetLastError(0x778899aa);plan.effects->write_error_slot_05d724(plan.error);
            valid=valid&&::GetLastError()==0x778899aa&&view.load<std::uint32_t>(0x20)==plan.error;
        }
        plan.ptd=ptd;plan.valid=valid;
    }catch(...){plan.valid=false;}
    if(++*plan.ready==plan.total)::SetEvent(plan.all_ready);
    const auto released=::WaitForSingleObject(plan.release,10000);
    if(released!=WAIT_OBJECT_0)plan.valid=false;
    return 63;
}
void real_domain_workers(){
    HeapObservation observation;heap_observation=&observation;const auto initial=seed();
    auto domain=d::DomainStorage::create(initial,observed_api());
    require(domain->initialize()&&domain->phase()==d::Phase::Ready,"owned DomainStorage initializes actual heap/FLS/error/locale graph");
    auto* main_ptd=domain->current_ptd();const auto index=domain->native_fls_index();auto main_error=domain->errno_cell();main_error.store(401);
    require(main_ptd&&::FlsGetValue(index)==main_ptd&&observation.size()==1,"main PTD is owned by the exact native FLS slot");
    require(domain->locale_references()==2&&domain->multibyte_references()==1,"main PTD retains actual owned initial locale and multibyte counters");
    HANDLE release=::CreateEventW(nullptr,TRUE,FALSE,nullptr),ready=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
    require(release&&ready,"own worker coordination events created");
    std::atomic<unsigned> arrived=0;std::array<WorkerPlan,3> plans;
    auto effects=domain->make_threads(thread_api());require(domain->provider_leases()==1,"actual make_recovered_threads provider retains one domain lease");
    {
        bootstrap_backend::TrackingThreads tracked(*effects,bootstrap_backend::native_join_api());
        for(unsigned i=0;i<plans.size();++i){
            auto& plan=plans[i];plan.index=index;plan.main_ptd=main_ptd;plan.effects=effects.get();plan.release=release;plan.all_ready=ready;plan.ready=&arrived;plan.total=plans.size();plan.error=100+i;
            plan.exception=initial.exception_table_identity;plan.locale=domain->locale().ids().initial_locale;plan.mb=domain->locale().ids().multibyte_info;
            std::uint32_t id=0;
            const auto handle=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&own_worker),reinterpret_cast<std::uintptr_t>(&plan),0,&id,tracked);
            require(handle&&id,"recovered begin_thread creates own real CRT wrapper worker");
            require(tracked.close_handle(handle),"source detached original handle closes while tracker owns duplicate");
        }
        require(::WaitForSingleObject(ready,10000)==WAIT_OBJECT_0&&arrived==3,"all three own workers simultaneously retain distinct native PTDs");
        std::set<crt_ptd_recovered::Ptd*> identities;
        for(auto& plan:plans){require(plan.valid&&plan.ptd.load(),"worker PTD/error/source-context and LastError route verified");identities.insert(plan.ptd.load());}
        require(identities.size()==3&&!identities.contains(main_ptd)&&main_error.load()==401,"worker PTDs and errno cells are isolated from each other and control thread");
        require(domain->locale_references()==5&&domain->multibyte_references()==4&&observation.size()==7,"live main plus three workers retain exact shared counters and heap allocations");
        require(!domain->close()&&domain->phase()==d::Phase::Stopping&&domain->source_fls_index()==index&&domain->native_fls_index()==index,"retained provider lease prevents premature FLS/domain teardown");
        rejects<std::logic_error>([&]{(void)domain->make_threads();},"stopping domain cannot admit another provider");
        ::SetEvent(release);require(tracked.stop_and_join(10000)&&tracked.tracked_jobs()==0,"B3 duplicate-handle tracker joins every actual CRT wrapper and FLS exit callback");
        require(observation.valid&&observation.size()==1&&domain->locale_references()==2&&domain->multibyte_references()==1,"actual FLS callbacks release all worker PTDs and Contexts with balanced real counters");
        require(main_error.load()==401,"worker FLS teardown leaves main errno unchanged");
    }
    require(!domain->close()&&domain->provider_leases()==1,"joined but retained provider still keeps domain graph alive");
    effects.reset();require(domain->provider_leases()==0,"destroying recovered provider releases final graph lease");
    require(domain->close()&&domain->phase()==d::Phase::Closed,"domain close releases actual main FLS/PTD after provider destruction");
    require(domain->native_fls_index()==crt_ptd_recovered::invalid_fls_index&&domain->source_fls_index()==crt_ptd_recovered::invalid_fls_index,"source/native FLS ownership both cleared");
    require(observation.valid&&observation.size()==0&&domain->locale_references()==1&&domain->multibyte_references()==0,"all domain heap allocations freed and initial data counter baselines restored");
    require(domain->close(),"closed domain teardown idempotent");
    ::CloseHandle(release);::CloseHandle(ready);domain.reset();heap_observation=nullptr;
}
struct QueueState {std::set<std::uintptr_t> resumed;unsigned creates=0,waits=0,closes=0;bool timeout_once=true;};
QueueState* queue_state=nullptr;
HANDLE WINAPI suspended_create(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE start,LPVOID argument,DWORD flags,LPDWORD id){
    require(!security&&!stack&&start&&argument&&!flags&&id,"Bootstrap source operands reach actual owned ThreadEffects creation");++queue_state->creates;
    return ::CreateThread(security,stack,start,argument,flags|CREATE_SUSPENDED,id);
}
DWORD WINAPI resume_and_wait(HANDLE handle,DWORD timeout){
    require(timeout==10000,"configured finite B3 join timeout preserved");++queue_state->waits;
    if(queue_state->timeout_once){queue_state->timeout_once=false;return WAIT_TIMEOUT;}
    const bool fresh=queue_state->resumed.insert(reinterpret_cast<std::uintptr_t>(handle)).second;
    const DWORD resumed=fresh?::ResumeThread(handle):1;
    const auto result=::WaitForSingleObject(handle,timeout);
    require(resumed==1&&result==WAIT_OBJECT_0,"queued actual source worker finishes before B3 reports successful join");return result;
}
BOOL WINAPI join_close(HANDLE handle){++queue_state->closes;return ::CloseHandle(handle);}
void full_bootstrap_runtime(){
    HeapObservation observation;heap_observation=&observation;QueueState state;queue_state=&state;
    auto domain=d::DomainStorage::create(seed(),observed_api());require(domain->initialize(),"full runtime owns initialized real shared ThreadDomain");
    {
        b4_test::Fixture fixture;auto resources=fixture.resources();auto api=thread_api();api.create=suspended_create;
        resources.threads=domain->make_threads(api);resources.join_api=bootstrap_backend::native_join_api();resources.join_api.wait=resume_and_wait;resources.join_api.close=join_close;resources.join_timeout_ms=10000;
        fixture.construct(std::move(resources));fixture.runtime->publish();
        require(fixture.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"B4 Runtime attaches through actual DomainStorage/make_recovered_threads graph");
        require(state.creates==3&&observation.size()==4&&domain->provider_leases()==1,"three actual suspended workers retain domain through native Context ownership");
        require(!fixture.runtime->close()&&fixture.runtime->phase()==bootstrap_owner::Phase::Stopping&&bootstrap_native_recovered::is_bound(),"first join timeout preserves B4 runtime and real domain graph");
        require(observation.size()==4&&fixture.library_os->unloaded.empty()&&fixture.cleanup->targets.empty(),"suspended jobs neither release domain nor trigger application cleanup early");
        require(fixture.runtime->close()&&!bootstrap_native_recovered::is_bound()&&state.closes==3,"retry completes actual CRT workers then B4 native Binding closes");
        require(observation.valid&&observation.size()==1&&domain->locale_references()==2&&domain->multibyte_references()==1,"all actual worker FLS callbacks finish before application owner teardown");
        require(fixture.runtime->configuration().username=="network-user"&&fixture.runtime->configuration().server_port==0x1234,"actual source workers update the same B4 owned configuration");
        require(fixture.library_os->unloaded==std::vector<std::uintptr_t>{0x777,0x501,0x502}&&fixture.cleanup->uninitializations==1,"existing source exit cleanup runs once after all real-domain workers join");
        const auto before=fixture.track.events.size();fixture.host.callbacks.back()();require(fixture.track.events.size()==before,"late exit callback remains inert after controlled runtime close");
        require(!domain->close()&&domain->provider_leases()==1,"closed Runtime still retains thread provider until object destruction");
        fixture.runtime.reset();require(domain->provider_leases()==0,"Runtime destruction releases recovered provider and DomainStorage lease");
    }
    require(domain->close()&&observation.valid&&observation.size()==0,"whole graph closes main/domain FLS only after runtime/provider destruction");
    require(domain->locale_references()==1&&domain->multibyte_references()==0,"full bootstrap graph returns owned initial-data counters to baseline");
    domain.reset();queue_state=nullptr;heap_observation=nullptr;
}
}
int main(){try{real_domain_workers();full_bootstrap_runtime();std::cout<<"PASS: "<<b2_test::checks+b3_tracking_test::checks<<" owned ThreadDomain integration checks; actual make_recovered_threads/ThreadEffects, native heap/FLS/locks, three concurrent own PTDs and full B3/B4 bootstrap join/retry/cleanup graph; original targets never executed\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
