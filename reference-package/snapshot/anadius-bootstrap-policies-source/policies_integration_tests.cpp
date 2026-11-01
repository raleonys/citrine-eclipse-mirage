#include "policies_test_fixture.hpp"

namespace {
using namespace b7_test;namespace t=threads_recovered;
struct MemoryWorker {
    d::ApplicationMemory* memory=nullptr;bootstrap_providers::NativeThreads* threads=nullptr;
    DWORD index=FLS_OUT_OF_INDEXES;unsigned number=0;
    HANDLE release=nullptr,all_ready=nullptr;std::atomic<unsigned>* ready=nullptr;
    std::atomic<crt_ptd_recovered::Ptd*> ptd=nullptr;std::atomic<bool> valid=false;
};
unsigned __stdcall own_memory_worker(void* argument){
    auto& plan=*static_cast<MemoryWorker*>(argument);void* value=nullptr;bool valid=false;
    try{
        auto* ptd=static_cast<crt_ptd_recovered::Ptd*>(::FlsGetValue(plan.index));plan.ptd=ptd;
        value=plan.memory->allocate(20+plan.number);
        std::memset(value,static_cast<int>(0x30+plan.number),20+plan.number);
        ::SetLastError(0x98760000+plan.number);plan.threads->write_error_slot_05d724(200+plan.number);
        valid=ptd&&::GetLastError()==0x98760000+plan.number&&crt_ptd_data_recovered::View(ptd).load<std::uint32_t>(0x20)==200+plan.number;
    }catch(...){valid=false;}
    if(++*plan.ready==2)::SetEvent(plan.all_ready);
    if(::WaitForSingleObject(plan.release,10000)!=WAIT_OBJECT_0)valid=false;
    if(value){
        const auto* bytes=static_cast<const unsigned char*>(value);
        for(unsigned i=0;i<20+plan.number;++i)valid=valid&&bytes[i]==0x30+plan.number;
        try{plan.memory->deallocate(value,999);}catch(...){valid=false;}
    }
    plan.valid=valid;return 0;
}
void concurrent_application_memory(){
    DomainFixture f;auto memory=f.domain->make_memory();auto threads=f.domain->make_threads(thread_api());
    require(f.domain->provider_leases()==2,"application-memory and recovered-thread providers retain independent leases of one domain");
    auto* main=f.domain->current_ptd();auto main_error=f.domain->errno_cell();main_error.store(444);
    HANDLE release=::CreateEventW(nullptr,TRUE,FALSE,nullptr),ready=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
    require(release&&ready,"own simultaneous memory-worker coordination events created");
    std::atomic<unsigned> arrived=0;std::array<MemoryWorker,2> plans;std::array<t::Handle,2> handles{};
    for(unsigned i=0;i<plans.size();++i){
        auto& plan=plans[i];plan.memory=memory.get();plan.threads=threads.get();plan.index=f.domain->native_fls_index();plan.number=i;plan.release=release;plan.all_ready=ready;plan.ready=&arrived;
        std::uint32_t id=0;handles[i]=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&own_memory_worker),reinterpret_cast<std::uintptr_t>(&plan),0,&id,*threads);
        require(handles[i]&&id,"existing begin-thread source creates own ApplicationMemory worker through actual CRT entry");
    }
    require(::WaitForSingleObject(ready,10000)==WAIT_OBJECT_0&&arrived==2,"both actual source workers concurrently retain application allocations");
    require(plans[0].ptd.load()&&plans[1].ptd.load()&&plans[0].ptd.load()!=plans[1].ptd.load()&&plans[0].ptd.load()!=main&&plans[1].ptd.load()!=main,"two source workers own distinct PTDs within the same FLS domain");
    require(memory->outstanding_allocations()==2&&f.sdk.size()==7&&f.sdk.valid,"shared allocator tickets coexist with main/worker PTDs and source Contexts on the same heap");
    require(main_error.load()==444&&f.domain->locale_references()==4&&f.domain->multibyte_references()==3,"concurrent application allocation/error routes preserve main errno and exact initial-data references");
    ::SetEvent(release);
    for(auto handle:handles){
        require(::WaitForSingleObject(reinterpret_cast<HANDLE>(handle),10000)==WAIT_OBJECT_0,"own memory worker joined through native kernel handle");
        require(threads->close_handle(handle),"own joined memory-worker handle closed");
    }
    require(plans[0].valid&&plans[1].valid&&memory->outstanding_allocations()==0,"concurrent workers verify payload bytes and release each owned ticket exactly once");
    require(f.sdk.valid&&f.sdk.size()==1&&f.domain->locale_references()==2&&f.domain->multibyte_references()==1,"worker Contexts/PTDs/FLS and application allocations all finish before provider destruction");
    require(!f.domain->close()&&f.domain->provider_leases()==2,"joined but live providers still block premature root/FLS release");
    threads.reset();memory.reset();require(f.domain->provider_leases()==0&&f.domain->close()&&f.sdk.size()==0,"all providers release before root closes actual main PTD");
    ::CloseHandle(ready);::CloseHandle(release);
}
struct Queue {
    std::set<std::uintptr_t> resumed;unsigned creates=0,waits=0,closes=0;bool timeout_once=true;
};
Queue* queue=nullptr;
HANDLE WINAPI suspended_create(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE start,LPVOID argument,DWORD flags,LPDWORD id){
    require(!security&&!stack&&start&&argument&&!flags&&id,"actual Bootstrap create operands reach B7-owned domain");++queue->creates;
    return ::CreateThread(security,stack,start,argument,flags|CREATE_SUSPENDED,id);
}
DWORD WINAPI resume_wait(HANDLE handle,DWORD timeout){
    require(timeout==10000,"configured finite B3 join timeout retained");++queue->waits;
    if(queue->timeout_once){queue->timeout_once=false;return WAIT_TIMEOUT;}
    const auto key=reinterpret_cast<std::uintptr_t>(handle);const bool fresh=queue->resumed.insert(key).second;
    const auto previous=fresh?::ResumeThread(handle):1;
    const auto result=::WaitForSingleObject(handle,timeout);
    require(previous==1&&result==WAIT_OBJECT_0,"actual CRT worker and FLS cleanup finish before tracker reports join success");return result;
}
BOOL WINAPI close_join(HANDLE handle){++queue->closes;return ::CloseHandle(handle);}
void full_runtime(){
    DomainFixture domain;auto memory=domain.domain->make_memory();PathSdk sdk;paths=&sdk;
    b5_test::PlatformState platform;b5_test::platform=&platform;Queue pending;queue=&pending;
    {
        b4_test::Fixture fixture;auto resources=fixture.resources();
        auto lower=std::make_unique<b5_test::Policies>(fixture.track.events);auto* config=lower.get();resources.configuration=std::move(lower);
        fixture.files->over=R"("Config2" {"User" {"Username" "override-user" "Avatar" "assets/caf)" "\xc3\xa9" R"(.png"} "Emulator" {"LoadExtraDLLsFromMain" "main.dll" "LoadExtraDLLs" "worker.dll"}})";
        resources=p::bind_configuration_platform(std::move(resources),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),path_api());
        resources=bootstrap_providers::bind_configuration_and_console(std::move(resources),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),b5_test::config_api(),b5_test::console_api(),b5_test::streams());
        resources.remaining=std::make_unique<p::SourceServices>(memory,std::move(resources.remaining));
        auto api=thread_api();api.create=suspended_create;resources.threads=domain.domain->make_threads(api);
        resources.join_api=bootstrap_backend::native_join_api();resources.join_api.wait=resume_wait;resources.join_api.close=close_join;resources.join_timeout_ms=10000;
        fixture.construct(std::move(resources));fixture.runtime->publish();
        require(fixture.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"full B4 Runtime attaches through actual B5/B6/B7 configuration, memory and thread graph");
        require(sdk.module_capacities==std::vector<DWORD>{260}&&sdk.names==std::vector<std::wstring>({L"ProgramData",L"ProgramData",L"LocalAppData"}),"actual source configuration calls concrete process/environment routes in retained pass order");
        require(fixture.files->opened==std::vector<configuration::Path>({u"C:\\fixture\\anadius.cfg",u"C:\\fixture\\anadius_override.cfg"})&&config->logs==2,"source FilePass/parser uses derived paths with memory-only CFG fixtures");
        const auto& state=fixture.runtime->configuration();
        require(state.avatar==u"C:\\fixture\\assets/caf\u00e9.png"&&state.extra_dlls_from_main==std::vector<configuration::Path>{u"C:\\fixture\\main.dll"}&&state.extra_dlls==std::vector<configuration::Path>{u"C:\\fixture\\worker.dll"},"actual UTF8 conversion and relative-resolution routes update same owned configuration");
        require(state.license_directory==u"PROGRAMDATA\\Electronic Arts\\EA Services\\License\\base-content"&&state.user_directory==u"LOCAL\\anadius\\LSX emu","source orchestration combines both concrete environment paths with existing joins");
        require(config->pngs==1&&config->persistence==1&&b4_test::pos(fixture.track.events,"avatar-remove")<b4_test::pos(fixture.track.events,"persistence"),"unselected PNG/persistence remain explicit policies after source avatar enumeration filter");
        require(pending.creates==3&&memory->outstanding_allocations()==3&&domain.sdk.size()==7,"three actual suspended Bootstrap jobs retain three same-domain payloads and three Contexts");
        require(fixture.remaining->allocated.empty()&&fixture.remaining->deleted.empty()&&b4_test::absent(fixture.track.events,"guard-return"),"actual Bootstrap bypasses old allocator/delete and final guard policies");
        require(!fixture.runtime->close()&&fixture.runtime->phase()==bootstrap_owner::Phase::Stopping&&bootstrap_native_recovered::is_bound(),"first B3 join timeout retains callback/runtime ownership");
        require(memory->outstanding_allocations()==3&&domain.sdk.size()==7&&fixture.cleanup->uninitializations==0&&fixture.library_os->unloaded.empty(),"timeout preserves all payloads and defers application cleanup");
        require(fixture.runtime->close()&&pending.closes==3&&!bootstrap_native_recovered::is_bound(),"retry joins every actual own CRT worker before native Binding release");
        require(memory->outstanding_allocations()==0&&domain.sdk.valid&&domain.sdk.size()==1,"worker Bootstrap delete routes free all three payloads through matching domain heap");
        require(fixture.remaining->allocated.empty()&&fixture.remaining->deleted.empty()&&std::count(fixture.track.events.begin(),fixture.track.events.end(),"cleanup-thread")==3,"existing worker cleanup policy runs before actual same-domain payload deletion");
        require(state.username=="network-user"&&state.server_port==0x1234,"all actual source workers share the one configuration state");
        require(fixture.library_os->unloaded==std::vector<std::uintptr_t>({0x777,0x501,0x502})&&fixture.cleanup->uninitializations==1,"existing library/online/hook cleanup still runs exactly once after joins");
        const auto before=fixture.track.events.size();fixture.host.callbacks.back()();require(fixture.track.events.size()==before,"late image callback stays inert after controlled shutdown");
        require(!domain.domain->close()&&domain.domain->provider_leases()==2,"closed Runtime and retained allocator still hold two explicit domain leases");
        fixture.runtime.reset();require(domain.domain->provider_leases()==1,"Runtime destruction releases thread lease while explicit allocator owner remains");
    }
    memory.reset();require(domain.domain->provider_leases()==0&&domain.domain->close()&&domain.sdk.size()==0&&domain.sdk.valid,"full graph closes actual FLS/PTD only after Runtime and allocator destruction");
    paths=nullptr;b5_test::platform=nullptr;queue=nullptr;
}
void failed_configuration_prefix(){
    b2_test::Events events;PathSdk sdk;paths=&sdk;b5_test::PlatformState platform;b5_test::platform=&platform;
    b2_test::Files files(events);files.base=b2_test::base_text();files.over=R"("Config2" {"User" {"Username" "override-user"}})";
    configuration::State state{};state.sdk="1.2.3";
    auto cfg=std::make_unique<b5_test::Policies>(events);cfg->fail_mkdir=true;auto* config=cfg.get();
    auto platform_config=std::make_unique<p::ConfigurationPlatform>(std::move(cfg),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),path_api());
    bootstrap_providers::Configuration selected(std::move(platform_config),std::make_unique<encoding_recovered::NativeCalls>(),std::make_unique<encoding_recovered::PortableErrors>(),b5_test::config_api());
    config_startup::FilePass pass(files);
    rejects<std::runtime_error>([&]{configuration::initialize_configuration(state,selected,pass);},"actual source directory failure propagates after both parse passes");
    require(state.user_directory==u"LOCAL\\anadius\\LSX emu"&&state.cookie_path.empty()&&files.opened.size()==2&&config->logs==2,"failure retains observed configuration prefix after both source file/parser passes");
    require(config->pngs==0&&config->persistence==0&&platform.environment.empty(),"failed prefix does not invent later storage/environment success");
    paths=nullptr;b5_test::platform=nullptr;
}
}
int main(){try{
    concurrent_application_memory();full_runtime();failed_configuration_prefix();
    std::cout<<"PASS: "<<b7_test::count()<<" bootstrap-policy integration checks; simultaneous same-domain application-memory workers, actual B4 Runtime/source parser/config routes, worker payload deletion and join-timeout retry; only own test workers executed\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
