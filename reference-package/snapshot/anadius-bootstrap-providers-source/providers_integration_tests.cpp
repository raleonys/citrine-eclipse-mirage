#include "providers_test_fixture.hpp"
#include <map>

namespace {
using namespace b5_test;
struct Job {LPTHREAD_START_ROUTINE start;LPVOID argument;};
inline std::map<std::uintptr_t,Job>* pending=nullptr;
inline std::uintptr_t next_job=0x7000;
HANDLE WINAPI queue(LPSECURITY_ATTRIBUTES security,SIZE_T stack,LPTHREAD_START_ROUTINE start,LPVOID argument,DWORD flags,LPDWORD id){
    require(!security&&!stack&&!flags&&id,"Bootstrap source operands reach concrete thread SDK wrapper");
    const auto handle=next_job++;pending->emplace(handle,Job{start,argument});*id=static_cast<DWORD>(handle);return reinterpret_cast<HANDLE>(handle);
}
void full_runtime(){
    b4_test::Fixture fixture;PlatformState platform_state;platform=&platform_state;std::map<std::uintptr_t,Job> jobs,joins;pending=&jobs;
    auto r=fixture.resources();auto policies=std::make_unique<Policies>(fixture.track.events);auto* config=policies.get();r.configuration=std::move(policies);r.console.reset();
    auto lower=std::make_unique<ThreadLower>();auto* lower_raw=lower.get();auto api=thread_native::native_api();api.create=queue;r.threads=std::make_unique<p::NativeThreads>(std::move(lower),api);
    r= p::bind_configuration_and_console(std::move(r),std::make_unique<Encoding>(),std::make_unique<encoding_recovered::PortableErrors>(),config_api(),console_api(),streams());
    fixture.api_state.duplicate_hook=[&](HANDLE source,LPHANDLE result){const auto h=fixture.api_state.next_join++;*result=reinterpret_cast<HANDLE>(h);joins[h]=jobs.at(reinterpret_cast<std::uintptr_t>(source));return TRUE;};
    fixture.api_state.wait_hook=[&](HANDLE handle,DWORD timeout){
        require(timeout==7,"B3 configured join timeout retained");const auto h=reinterpret_cast<std::uintptr_t>(handle);const auto job=joins.at(h);
        HANDLE worker=::CreateThread(nullptr,0,job.start,job.argument,CREATE_SUSPENDED,nullptr);require(worker!=nullptr,"own queued CRT wrapper worker created");
        const auto resumed=::ResumeThread(worker);const auto waited=::WaitForSingleObject(worker,10000);
        require(resumed==1&&waited==WAIT_OBJECT_0,"own source worker joined before reporting synthetic tracker completion");DWORD code=99;
        require(::GetExitCodeThread(worker,&code)&&code==0,"actual Bootstrap/CRT thread source worker returns expected code");::CloseHandle(worker);joins.erase(h);return DWORD{WAIT_OBJECT_0};
    };
    fixture.construct(std::move(r));fixture.runtime->publish();require(fixture.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"B4 Runtime attaches with all three B5 concrete provider routes");
    require(config->logs==2&&config->pngs==2&&config->persistence==1,"actual file/config source path reaches explicit remaining policies exactly once");
    require(fixture.files->opened==std::vector<configuration::Path>{u"C:\\fixture\\anadius.cfg",u"C:\\fixture\\anadius_override.cfg"},"actual parser inputs use recovered parent/join paths with memory-only files");
    require(platform_state.opens==std::vector<std::string>({"conin$:r","conout$:w","conout$:w"}),"actual Bootstrap console branch reaches compiler CRT bridge with no GUI/device calls");
    require(jobs.size()==3&&joins.size()==3&&lower_raw->invocations==0,"all CRT wrapper jobs remain pending until controlled join");
    require(fixture.runtime->close()&&joins.empty()&&!bootstrap_native_recovered::is_bound(),"B3 joins all actual own wrapper workers then B4 closes native binding");
    require(lower_raw->invocations==3&&lower_raw->contexts.empty(),"three workers use recovered CRT entry and finish with no synthetic Context leak");
    require(fixture.library_os->unloaded==std::vector<std::uintptr_t>{0x777,0x501,0x502}&&fixture.cleanup->uninitializations==1,"existing post-join application cleanup still runs exactly once");
    require(fixture.runtime->configuration().username=="network-user"&&fixture.runtime->configuration().server_port==0x1234,"all provider routes share one live runtime configuration");
    const auto n=fixture.track.events.size();fixture.host.callbacks.back()();require(fixture.track.events.size()==n,"late CRT callback remains inert after full provider graph closes");
    pending=nullptr;
}
void assembly_limits(){
    PlatformState state;platform=&state;b2_test::Events events;bootstrap_backend::Resources r{};r.configuration=std::make_unique<Policies>(events);
    r.console=std::make_unique<p::CompilerConsole>(console_api(),streams());auto* selected=r.console.get();
    r=p::bind_configuration_and_console(std::move(r),std::make_unique<Encoding>(),std::make_unique<encoding_recovered::PortableErrors>(),config_api(),console_api(),streams());
    require(r.console.get()==selected&&dynamic_cast<p::Configuration*>(r.configuration.get()),"assembly preserves explicitly selected console while wrapping config source routes");
    require(state.attaches==0&&state.environment.empty(),"provider assembly performs no console/environment actions before application entry");
    rejects<std::invalid_argument>([&]{p::NativeThreads invalid({});},"absent lower thread domain cannot be silently fabricated");
    auto api=thread_native::native_api();api.create=nullptr;
    rejects<std::invalid_argument>([&]{p::NativeThreads invalid(std::make_unique<ThreadLower>(),api);},"incomplete thread SDK API rejected before publication");
}
}
int main(){try{full_runtime();assembly_limits();std::cout<<"PASS: "<<b2_test::checks+b3_tracking_test::checks<<" startup-provider integration checks; actual file/config/bootstrap/B3/B4 graph plus three own recovered-CRT wrapper workers, controlled console/encoding effects and post-join cleanup; native domain/foreign ABI/PE entry remain explicit\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
