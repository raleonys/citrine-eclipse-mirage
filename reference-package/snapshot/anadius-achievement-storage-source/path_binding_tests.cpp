#include "path_binding.hpp"
#include "../anadius-configuration-factory-source/factory.hpp"
#include "../anadius-application-worker-source/worker.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/observed_runtime.hpp"
#include "../anadius-bootstrap-policies-source/policies_test_fixture.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <thread>

// Runtime tests execute reconstructed entries through controlled B3/B4 joins;
// these are external service boundaries, not native worker/kernel-drain proof.
// Real Worker tests below stop in Prepared: no Graph, socket or DLL is started.
namespace {
using namespace anadius;namespace cf=configuration_factory;namespace br=bootstrap_runtime;
namespace aw=application_worker;namespace as=achievement_storage;using Path=configuration::Path;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
const Path xml=u"LOCAL\\anadius\\LSX emu\\achievement-set.xml";
const Path ini=u"LOCAL\\anadius\\LSX emu\\achievement_watcher\\42\\achievements.ini";
int space(int v){return v==' '||v=='\t';}
int lower(int v){return v>='A'&&v<='Z'?v+32:v;}
BOOL WINAPI set_environment(LPCSTR,LPCSTR){return FALSE;}
struct Stats final:status_recovered::Stats {
    std::vector<Path> queried;
    status_recovered::StatsResult query(const Path& path,std::uint32_t flags)override{require(flags==3,"storage requests source status flags3");queried.push_back(path);return {ERROR_FILE_NOT_FOUND,0,{}};}
};
struct Legacy final:cf::LegacyDocumentReader {
    configuration::Node read(const Path&)override{throw std::logic_error("native FilePass must bypass legacy document reader");}
};
struct WorkerProbe {
    execution_recovered::MutexState* mutex=nullptr;execution_recovered::Platform* mutex_platform=nullptr;
    const license_recovered::AuthenticationCode* authentication=nullptr;
    as::Paths* paths=nullptr;execution_recovered::Execution* execution=nullptr;
    unsigned binds=0,runs=0,stops=0;bool run_valid=false,retired=false,retired_valid=false;
};
struct RecordingWorker final:bootstrap_backend::ApplicationWorker,as::PathBinding,license_recovered::AuthenticationBinding,achievement_runtime::MutexBinding {
    WorkerProbe& p;explicit RecordingWorker(WorkerProbe& v):p(v){}
    ~RecordingWorker()override{p.retired=true;p.retired_valid=p.paths&&p.paths->xml_path==xml&&p.paths->ini_path==ini&&p.authentication&&p.authentication->value()=="SGV5LCB5b3UsIHN0b3AgcmlnaHQgdGhlcmUhIDsp"&&p.mutex&&p.mutex_platform&&p.mutex->type==2&&p.mutex->owner_48==0xffffffffU&&!p.mutex->count_4c;}
    void bind_achievement_mutex(execution_recovered::MutexState& state,execution_recovered::Platform& platform)override{require(!p.mutex,"Runtime binds achievement mutex once");require(state.type==2&&state.owner_48==0xffffffffU&&!state.count_4c&&!state.storage_08&&!state.srw_10,"Actual Runtime owns initialized idle native achievement mutex before publication");p.mutex=&state;p.mutex_platform=&platform;}
    void bind_authentication_code(const license_recovered::AuthenticationCode& code)override{require(!p.authentication,"Runtime binds owned authentication cell once");p.authentication=&code;require(code.value()=="SGV5LCB5b3UsIHN0b3AgcmlnaHQgdGhlcmUhIDsp","actual Runtime owns exact native auth field before worker publication");}
    void bind_achievement_paths(as::Paths& paths)override{require(!p.paths,"recording worker binds only once");p.paths=&paths;++p.binds;}
    void run_server(std::uint16_t&,std::uint8_t& ready,execution_recovered::Execution& execution)noexcept override{++p.runs;p.run_valid=p.paths&&p.paths->xml_path==xml&&p.paths->ini_path==ini&&p.execution==&execution;ready=1;}
    void startup_complete(execution_recovered::ReadyStage,const execution_recovered::Execution*)noexcept override{}
    void startup_failed(const execution_recovered::Execution*)noexcept override{}
    void request_stop()override{++p.stops;}
    bool finish_after_join()override{return true;}
    bool cancel_unpublished()override{return true;}
};
struct Harness {
    b2_test::Track track;b3_tracking_test::ApiState joins;b4_test::HostState host;
    b7_test::PathSdk paths;b8_test::FsSdk sdk;
    std::shared_ptr<b8_test::MemoryFilesState> files=std::make_shared<b8_test::MemoryFilesState>();
    std::shared_ptr<config_storage::Filesystem> filesystem;
    std::shared_ptr<std::ostringstream> output=std::make_shared<std::ostringstream>();
    std::map<std::uintptr_t,threads_recovered::Context*> jobs,duplicates;
    WorkerProbe probe;b3_tracking_test::Lower* threads=nullptr;cf::Configuration* config=nullptr;
    std::unique_ptr<br::Runtime> runtime;
    Harness(){b3_tracking_test::current=&joins;b4_test::host_state=&host;b7_test::paths=&paths;b8_test::fs_sdk=&sdk;paths.local=L"LOCAL";sdk.names.clear();filesystem=std::make_shared<config_storage::Filesystem>(b8_test::fs_api());}
    ~Harness(){if(runtime){try{if(runtime->phase()!=bootstrap_owner::Phase::Closed&&!runtime->close())std::terminate();}catch(...){std::terminate();}runtime.reset();}b3_tracking_test::current=nullptr;b4_test::host_state=nullptr;b7_test::paths=nullptr;b8_test::fs_sdk=nullptr;}
    config_storage::PersistenceResources persistence(){config_storage::PersistenceResources r;r.files=std::make_unique<b8_test::MemoryFiles>(files);r.stats=std::make_unique<Stats>();r.encoding=std::make_unique<b7_test::Encoding>();return r;}
    cf::Inputs inputs(){cf::Inputs in;in.stream=output;in.filesystem=filesystem;in.encoding=std::make_unique<b7_test::Encoding>();in.platform=b7_test::path_api();in.crt={space,lower,set_environment};in.persistence=persistence();return in;}
    bootstrap_backend::Resources resources(){
        bootstrap_backend::Resources r{};cf::install_configuration(r,inputs(),{},std::make_unique<Legacy>());config=dynamic_cast<cf::Configuration*>(r.configuration.get());
        r.files=std::make_unique<b8_test::MemoryFiles>(files);r.input=std::make_unique<b2_test::InputPlatform>(track.events);r.console=std::make_unique<b2_test::Console>(track.events);
        auto lower_threads=std::make_unique<b3_tracking_test::Lower>();threads=lower_threads.get();
        lower_threads->create_hook=[&](threads_recovered::Context* context,std::uint32_t* id){const auto handle=0x1100+threads->creations;jobs[handle]=context;if(id)*id=threads->creations+10;return handle;};r.threads=std::move(lower_threads);
        r.remaining=std::make_unique<b2_test::Backend>(track);r.execution=std::make_unique<b4_test::Execution>(track.events);
        r.application_worker=[&](bootstrap_backend::CallbackStateView view){probe.execution=&view.execution;return std::make_unique<RecordingWorker>(probe);};
        auto online=std::make_unique<b4_test::Online>(track.events);online->found=false;r.online=std::move(online);
        r.library_os=std::make_unique<b4_test::LibraryOS>(track.events);r.library_effects=std::make_unique<b4_test::LibraryEffects>();r.host=std::make_unique<b4_test::Host>(track.events);r.hook_cleanup=std::make_unique<b4_test::Cleanup>(track.events);
        r.stop_sources=[]{return true;};r.join_api=b3_tracking_test::api();r.join_timeout_ms=7;
        joins.duplicate_hook=[&](HANDLE source,LPHANDLE out){const auto handle=joins.next_join++;*out=b3_tracking_test::handle(handle);duplicates[handle]=jobs.at(b3_tracking_test::number(source));return TRUE;};
        joins.wait_hook=[&](HANDLE handle,DWORD timeout){require(timeout==7,"controlled join preserves finite timeout");const auto found=duplicates.find(b3_tracking_test::number(handle));require(found!=duplicates.end(),"controlled join owns duplicate context");auto* context=found->second;const auto entry=reinterpret_cast<bootstrap_native_recovered::ThreadStart>(context->entry);require(entry(reinterpret_cast<void*>(context->argument))==0,"actual reconstructed queued thread entry returns normally");threads->release(context);duplicates.erase(found);return DWORD{WAIT_OBJECT_0};};return r;
    }
};
void observed_constructor_paths(){
    for(const auto& initial:{as::Paths{u"preseed.xml",{}},as::Paths{{},u"preseed.ini"}}){
        Harness h;auto resources=h.resources();
        resources.configuration=cf::make_configuration(h.inputs(),initial,std::make_unique<Legacy>());
        auto* provider=resources.configuration.get();
        rejects<std::invalid_argument>([&]{auto runtime=br::make_observed_runtime_036d20(
            br::OutsideConstructorState{10000000,{}, {},{}},resources,b2_test::permanent_errors,b4_test::host_api());},
            "Observed constructor rejects native XML or INI path overrides before consuming resources");
        require(resources.configuration.get()==provider&&resources.application_worker&&h.files->reads.empty()&&h.files->writes.empty()&&!h.host.registrations&&!h.probe.paths,
            "Rejected observed path preflight neither consumes Resources nor binds/publishes/executes IO");
    }
    {
        Harness h;auto resources=h.resources();
        require(h.config->accepts_observed_empty_paths(),"Fresh empty native Configuration passes passive path preflight");
        h.runtime=br::make_observed_runtime_036d20(br::OutsideConstructorState{10000000,{}, {},{}},
            resources,b2_test::permanent_errors,b4_test::host_api());
        require(h.probe.paths&&h.probe.paths->xml_path.empty()&&h.probe.paths->ini_path.empty()&&h.runtime->configuration().username=="anadius",
            "Observed constructor binds native Configuration and actual Runtime root cells with empty XML/INI and native profile defaults");
        require(h.files->reads.empty()&&h.files->writes.empty()&&!h.host.registrations,
            "Successful observed preflight leaves binding possible and performs no file IO or publication");
        require(h.runtime->close(),"Never-published observed Runtime closes through existing constructed route");
    }
}
void runtime_shares_live_cells(){
    Harness h;h.files->inputs[u"C:\\fixture\\anadius.cfg"]=R"("Config2" {"Achievements" {"AchievementsSet" "set" "SteamAppId" "42"}})";h.files->inputs[xml]="<Achievements/>";
    auto resources=h.resources();require(h.config&&h.files->reads.empty()&&h.files->writes.empty()&&h.paths.module_capacities.empty(),"native configuration installation performs no file/path IO");
    bootstrap_owner::InitialState initial{};bootstrap_backend::RuntimeSeed seed{};execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);seed.frequency_cache=10000000;
    h.runtime=br::make_runtime(std::move(initial),std::move(seed),resources,b2_test::permanent_errors,b4_test::host_api());
    require(h.probe.binds==1&&h.probe.paths&&(&h.config->paths()==h.probe.paths),"actual SourceBackend binds configuration and worker to identical owned path cells");
    require(h.probe.paths->xml_path.empty()&&h.probe.paths->ini_path.empty()&&h.files->reads.empty()&&h.files->writes.empty()&&h.sdk.creates.empty(),"shared-cell binding is passive and precedes startup");
    h.runtime->publish();require(h.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"actual application startup loads synthetic configuration");
    require(h.runtime->configuration().achievements_set=="set"&&h.runtime->configuration().steam_app_id=="42","native FilePass updates actual Runtime achievement identifiers");
    require(&h.config->paths()==h.probe.paths&&h.probe.paths->xml_path==xml&&h.probe.paths->ini_path==ini,"actual Storage derives XML and INI in the SAME previously bound cells");
    require(std::count(h.files->reads.begin(),h.files->reads.end(),xml)==1&&h.files->outputs.contains(ini)&&h.files->outputs.at(ini).empty(),"actual Storage reads synthetic XML and writes empty achievement INI through memory files");
    require(h.threads->creations==3&&h.probe.runs==0,"only controlled external thread entries are queued before close");
    require(h.runtime->close()&&h.probe.runs==1&&h.probe.run_valid&&h.probe.stops==1&&h.duplicates.empty(),"queued source worker observes updated shared cells and exact Execution on controlled join");
    h.runtime.reset();require(h.probe.retired&&h.probe.retired_valid,"shared root cells outlive application-worker destructor during backend retirement");
}
void persistence_guards(){
    Harness h;as::Paths root;
    config_storage::Persistence p(h.filesystem,{u"seed.xml",u"seed.ini"},h.persistence());p.bind_achievement_paths(root);
    require(&p.paths()==&root&&root.xml_path==u"seed.xml"&&root.ini_path==u"seed.ini"&&h.files->reads.empty()&&h.files->writes.empty(),"passive binding transfers initial legacy paths into exact shared cells");
    rejects<std::logic_error>([&]{p.bind_achievement_paths(root);},"same-root repeat binding rejects");
    config_storage::Persistence borrowed(h.filesystem,{},h.persistence());auto& self=const_cast<as::Paths&>(borrowed.paths());
    rejects<std::logic_error>([&]{borrowed.bind_achievement_paths(self);},"borrowed/self path binding rejects");
    config_storage::Persistence occupied(h.filesystem,{},h.persistence());as::Paths nonempty{u"occupied",{}};
    rejects<std::invalid_argument>([&]{occupied.bind_achievement_paths(nonempty);},"nonempty destination rejects without overwriting its values");require(nonempty.xml_path==u"occupied","rejected destination remains intact");
    as::Paths empty;config_storage::Persistence updated(h.filesystem,{},h.persistence());configuration::State state{};updated.update_achievement_storage_01eb00(state);
    rejects<std::logic_error>([&]{updated.bind_achievement_paths(empty);},"even empty Storage update seals subsequent binding");
    config_storage::Persistence written(h.filesystem,{},h.persistence());written.write_embedded_png_03c020(u"memory.png",configuration::EmbeddedPng::default_user_1800809d0);
    rejects<std::logic_error>([&]{written.bind_achievement_paths(empty);},"PNG output also seals subsequent binding");
    config_storage::Persistence legacy(h.filesystem,{u"legacy.xml",u"legacy.ini"},h.persistence());require(legacy.paths().xml_path==u"legacy.xml"&&legacy.paths().ini_path==u"legacy.ini","unbound legacy engine preserves its own explicit paths");
}
struct FiberCall {
    void* controller;aw::Worker& worker;as::Paths& paths;bool rejected=false;
    static VOID WINAPI run(void* raw){auto& v=*static_cast<FiberCall*>(raw);try{v.worker.bind_achievement_paths(v.paths);}catch(const std::logic_error&){v.rejected=true;}::SwitchToFiber(v.controller);std::terminate();}
};
void prepared_worker_guards(){
    const bool converted=!::IsThreadAFiber();void* controller=converted ? ::ConvertThreadToFiber(nullptr) : ::GetCurrentFiber();require(controller!=nullptr,"own controller fiber is available");
    {
        b7_test::DomainFixture domain;configuration::State config{};bootstrap_backend::RuntimeSeed seed{};b2_test::Events events;b4_test::Execution platform(events);
        execution_recovered::Execution execution(seed.execution_mutex,{seed.server_ready,seed.online_ready,seed.libraries_ready},seed.frequency_cache,platform);execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);
        unsigned configured=0;auto make=[&](Path initial){aw::Settings settings;settings.lifetime=std::make_shared<int>(7);settings.configure=[&](aw::Inputs&){++configured;throw std::logic_error("Prepared test must not configure Graph");};settings.state=aw::Settings::StateSeed{0,std::move(initial)};settings.startup_timeout_ms=100;return aw::Worker::create(domain.domain,{config,seed,execution},std::move(settings));};
        as::Paths root{u"shared.xml",u"shared.ini"};auto worker=make({});
        bool foreign_rejected=false;std::thread foreign([&]{try{worker->bind_achievement_paths(root);}catch(const std::logic_error&){foreign_rejected=true;}});foreign.join();require(foreign_rejected&&worker->snapshot().phase==aw::Phase::Prepared,"foreign controller thread rejects before changing Prepared state");
        FiberCall call{controller,*worker,root};auto* fiber=::CreateFiber(0,FiberCall::run,&call);require(fiber!=nullptr,"own secondary fiber is created");::SwitchToFiber(fiber);::DeleteFiber(fiber);
        require(call.rejected&&worker->snapshot().phase==aw::Phase::Prepared,"same-thread foreign fiber rejects without consuming binding");
        worker->bind_achievement_paths(root);rejects<std::logic_error>([&]{worker->bind_achievement_paths(root);},"actual Worker binds only once even to same cells");
        require(worker->cancel_unpublished()&&worker->snapshot().phase==aw::Phase::Closed,"actual never-published Worker cancels prepared memory and event");worker.reset();
        auto conflicting=make(u"different.xml");rejects<std::invalid_argument>([&]{conflicting->bind_achievement_paths(root);},"conflicting nonempty legacy seed rejects shared path binding");
        require(conflicting->snapshot().phase==aw::Phase::Prepared,"seed conflict leaves worker Prepared for known cancellation");require(conflicting->cancel_unpublished(),"conflicting worker remains safely cancellable");conflicting.reset();
        auto matching=make(root.xml_path);matching->bind_achievement_paths(root);require(matching->cancel_unpublished(),"matching nonempty initial seed accepts shared cells");matching.reset();
        auto legacy=make(u"legacy-only.xml");require(legacy->cancel_unpublished(),"unbound legacy Worker seed remains supported");legacy.reset();
        require(configured==0&&domain.domain->provider_leases()==0,"all Prepared-only paths avoid Graph configuration and release actual Domain provider leases");
    }
    if(converted)require(::ConvertFiberToThread()!=0,"own converted controller returns to ordinary thread after Domain retirement");
}
}
int main(){try{persistence_guards();runtime_shares_live_cells();prepared_worker_guards();observed_constructor_paths();std::cout<<"PASS: "<<checks<<" shared achievement-path checks; actual Runtime/Storage aliasing and Prepared Worker guards, controlled external worker boundaries\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
