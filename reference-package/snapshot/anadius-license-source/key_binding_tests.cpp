#include "../anadius-configuration-factory-source/factory.hpp"
#include "../anadius-config-source/state_binding.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include "../anadius-bootstrap-policies-source/policies_test_fixture.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-token-source/constants.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <sstream>

// Actual Runtime/SourceBackend/native Configuration binding and application
// startup. B3/B4 recorders supply only the unselected service/thread/server/
// online/host boundaries. No native OS thread, DLL, network or user file runs;
// controlled joins invoke only this EXE's reconstructed queued source entries.
namespace {
using namespace anadius;namespace cf=configuration_factory;namespace br=bootstrap_runtime;
using Path=configuration::Path;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Harness;
Harness* current=nullptr;
int space(int v){return v==' '||v=='\t';}
int lower(int v){return v>='A'&&v<='Z'?v+32:v;}
BOOL WINAPI set_environment(LPCSTR,LPCSTR);
struct Stats final:status_recovered::Stats {
    Harness& harness;std::vector<Path> paths;
    explicit Stats(Harness& h):harness(h){}
    status_recovered::StatsResult query(const Path&,std::uint32_t)override;
};
struct Legacy final:cf::LegacyDocumentReader {
    unsigned* calls;explicit Legacy(unsigned& count):calls(&count){}
    configuration::Node read(const Path&)override{++*calls;throw std::logic_error("native FilePass must bypass legacy document reader");}
};
struct Harness {
    b2_test::Track track;b3_tracking_test::ApiState joins;b4_test::HostState host;
    b7_test::PathSdk paths;b8_test::FsSdk sdk;
    std::shared_ptr<b8_test::MemoryFilesState> memory=std::make_shared<b8_test::MemoryFilesState>();
    std::shared_ptr<config_storage::Filesystem> filesystem;
    std::shared_ptr<std::ostringstream> output=std::make_shared<std::ostringstream>();
    std::vector<std::pair<std::string,std::string>> environment;
    std::map<std::uintptr_t,threads_recovered::Context*> jobs,duplicates;
    b3_tracking_test::Lower* threads=nullptr;cf::Configuration* configuration=nullptr;Stats* stats=nullptr;
    std::unique_ptr<br::Runtime> runtime;unsigned legacy_calls=0;bool regular=false,observed_live_prefix=false;
    Harness(){current=this;b3_tracking_test::current=&joins;b4_test::host_state=&host;b7_test::paths=&paths;b8_test::fs_sdk=&sdk;paths.program=L"MEM";paths.local=L"LOCAL";sdk.names.clear();filesystem=std::make_shared<config_storage::Filesystem>(b8_test::fs_api());}
    ~Harness(){if(runtime){try{if(runtime->phase()!=bootstrap_owner::Phase::Closed&&!runtime->close())std::terminate();}catch(...){std::terminate();}runtime.reset();}b3_tracking_test::current=nullptr;b4_test::host_state=nullptr;b7_test::paths=nullptr;b8_test::fs_sdk=nullptr;current=nullptr;}
    cf::Inputs inputs(){
        cf::Inputs in;in.stream=output;in.filesystem=filesystem;in.encoding=std::make_unique<b7_test::Encoding>();in.platform=b7_test::path_api();in.crt={space,lower,set_environment};
        in.persistence.files=std::make_unique<b8_test::MemoryFiles>(memory);auto probe=std::make_unique<Stats>(*this);stats=probe.get();in.persistence.stats=std::move(probe);return in;
    }
    bootstrap_backend::Resources resources(){
        bootstrap_backend::Resources r{};
        cf::install_configuration(r,inputs(),{},std::make_unique<Legacy>(legacy_calls));
        configuration=dynamic_cast<cf::Configuration*>(r.configuration.get());
        r.files=std::make_unique<b8_test::MemoryFiles>(memory);
        r.input=std::make_unique<b2_test::InputPlatform>(track.events);r.console=std::make_unique<b2_test::Console>(track.events);
        auto worker=std::make_unique<b3_tracking_test::Lower>();threads=worker.get();
        worker->create_hook=[&](threads_recovered::Context* context,std::uint32_t* id){const auto handle=0x1100+threads->creations;jobs[handle]=context;if(id)*id=threads->creations+10;return handle;};r.threads=std::move(worker);
        r.remaining=std::make_unique<b2_test::Backend>(track);r.execution=std::make_unique<b4_test::Execution>(track.events);r.server=std::make_unique<b4_test::Server>(track.events);
        auto online=std::make_unique<b4_test::Online>(track.events);online->found=false;r.online=std::move(online);
        r.library_os=std::make_unique<b4_test::LibraryOS>(track.events);r.library_effects=std::make_unique<b4_test::LibraryEffects>();r.host=std::make_unique<b4_test::Host>(track.events);r.hook_cleanup=std::make_unique<b4_test::Cleanup>(track.events);
        r.stop_sources=[]{return true;};r.join_api=b3_tracking_test::api();r.join_timeout_ms=7;
        joins.duplicate_hook=[&](HANDLE source,LPHANDLE out){const auto handle=joins.next_join++;*out=b3_tracking_test::handle(handle);duplicates[handle]=jobs.at(b3_tracking_test::number(source));return TRUE;};
        joins.wait_hook=[&](HANDLE handle,DWORD timeout){require(timeout==7,"controlled source join preserves finite timeout");const auto found=duplicates.find(b3_tracking_test::number(handle));require(found!=duplicates.end(),"join uses recorded duplicate owner");auto* context=found->second;const auto entry=reinterpret_cast<bootstrap_native_recovered::ThreadStart>(context->entry);require(entry(reinterpret_cast<void*>(context->argument))==0,"controlled join executes actual queued source trampoline");threads->release(context);duplicates.erase(found);return DWORD{WAIT_OBJECT_0};};
        return r;
    }
};
BOOL WINAPI set_environment(LPCSTR key,LPCSTR value){current->environment.emplace_back(key,value);return FALSE;}
status_recovered::StatsResult Stats::query(const Path& path,std::uint32_t flags){
    require(flags==3,"bound native key route preserves source status flags3");paths.push_back(path);
    if(harness.runtime){const auto& state=harness.runtime->configuration();harness.observed_live_prefix=state.content_id=="NEW-CONTENT"&&state.user_id=="OLD-USER";}
    return harness.regular?status_recovered::StatsResult{0,FILE_ATTRIBUTE_NORMAL,{}}:
        status_recovered::StatsResult{ERROR_FILE_NOT_FOUND,0,{}};
}
std::string decrypt(const std::string& bytes){
    constexpr std::array<unsigned char,8> prefix{0x30,0x06,0x02,0x01,0x00,0x02,0x01,0x00};std::string header(65,'\0');std::copy(prefix.begin(),prefix.end(),header.begin());
    require(bytes.size()>65&&bytes.substr(0,65)==header&&(bytes.size()-65)%16==0,"actual bound route emits native65byte header and whole ciphertext blocks");
    auto decoded=bytes;crypto_recovered::Cipher cipher;cipher.set_key_and_iv_042760(token_recovered::constants::key_084a50,token_recovered::constants::iv_084a60);require(cipher.decrypt_with_offset(decoded,0x41),"actual AES decrypts bound route output at offset41");return decoded.substr(65);
}
std::string expected_xml(){return "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><License xmlns=\"http://ea.com/license\"><CipherKey>SYNTHETIC-KEY</CipherKey><MachineHash>1</MachineHash><ContentId>NEW-CONTENT</ContentId><UserId>OLD-USER</UserId><GrantTime>2024-01-01T00:00:00Z</GrantTime><StartTime>2024-01-01T00:00:00Z</StartTime></License>";}
void runtime_binds_actual_moved_state(bool regular){
    Harness h;h.regular=regular;
    h.memory->inputs[u"C:\\fixture\\anadius.cfg"]=R"("Config2" {"Game" {"Name" "synthetic" "Version" "1" "ContentId" "NEW-CONTENT" "KeyForLicense" "SYNTHETIC-KEY"} "Emulator" {"SDK" "1.2.3" "ServerPort" "4660"} "User" {"UserId" "NEW-USER"}})";
    auto resources=h.resources();require(h.configuration&&h.stats&&h.paths.module_capacities.empty()&&h.memory->reads.empty()&&h.memory->writes.empty()&&h.sdk.creates.empty(),"native Configuration install/constructor is passive without a test LicenseEffects bridge");
    bootstrap_owner::InitialState initial{};initial.configuration.content_id="SEED-CONTENT";initial.configuration.user_id="OLD-USER";auto* moved_initial=&initial.configuration;
    bootstrap_backend::RuntimeSeed seed{};execution_recovered::initialize_mutex_04a698(seed.execution_mutex,2);seed.frequency_cache=10000000;
    h.runtime=br::make_runtime(std::move(initial),std::move(seed),resources,b2_test::permanent_errors,b4_test::host_api());
    require(&h.runtime->configuration()!=moved_initial&&h.runtime->configuration().user_id=="OLD-USER","make_runtime owns a distinct stable moved configuration object");
    require(h.memory->reads.empty()&&h.memory->writes.empty()&&h.stats->paths.empty()&&h.paths.module_capacities.empty(),"automatic SourceBackend state binding remains passive before startup");
    initial.configuration.content_id="MOVED-FROM-WRONG-CONTENT";initial.configuration.user_id="MOVED-FROM-WRONG-USER";
    configuration::State other{};auto* binding=dynamic_cast<configuration::StateBinding*>(h.configuration);require(binding!=nullptr,"native Configuration exposes optional separate StateBinding interface");rejects<std::logic_error>([&]{binding->bind_configuration_state(other);},"Runtime construction already bound provider before startup");
    h.runtime->publish();require(h.runtime->application_entry()(nullptr,1,nullptr)==TRUE,"actual Bootstrap application entry loads memory configuration with native key processing");
    const Path target=u"MEM\\Electronic Arts\\EA Services\\License\\NEW-CONTENT.dlf";
    require(h.stats->paths==std::vector<Path>{target}&&h.observed_live_prefix,"native key route reads actual Runtime Game state before its later User assignment");
    require(h.runtime->configuration().content_id=="NEW-CONTENT"&&h.runtime->configuration().user_id=="NEW-USER"&&initial.configuration.user_id=="MOVED-FROM-WRONG-USER","later User assignment updates Runtime state without touching caller moved-from object");
    const auto writes=std::count(h.memory->writes.begin(),h.memory->writes.end(),target);
    if(regular)require(!writes&&!h.memory->outputs.contains(target),"regular-file status skips native key overwrite through actual runtime binding");
    else require(writes==1&&decrypt(h.memory->outputs.at(target))==expected_xml(),"production native binding writes actual encrypted NEW-content/OLD-user document exactly once");
    require(h.legacy_calls==0&&h.threads->creations==3,"real FilePass bypasses legacy reader while only controlled fixture threads are queued");
    require(h.runtime->close()&&h.runtime->phase()==bootstrap_owner::Phase::Closed&&h.duplicates.empty(),"actual Runtime close joins queued source entries then drains source exits");h.runtime.reset();
}
struct LegacyLicense final:cf::LicenseEffects {
    std::vector<std::string>& calls;explicit LegacyLicense(std::vector<std::string>& values):calls(values){}
    void apply(std::string_view key)override{calls.emplace_back(key);}
};
void unbound_rebind_and_legacy_guards(){
    Harness h;configuration::State first{},second{};unsigned docs=0;
    auto native=cf::make_configuration(h.inputs(),{},std::make_unique<Legacy>(docs));auto* binding=dynamic_cast<configuration::StateBinding*>(native.get());require(binding!=nullptr,"three-argument native make overload exposes StateBinding");
    rejects<std::logic_error>([&]{native->apply_license_key_02a830("UNBOUND");},"native route rejects unbound state before reading fields or SDK");require(h.stats->paths.empty()&&h.memory->writes.empty(),"unbound failure has no status/output prefix");
    binding->bind_configuration_state(first);rejects<std::logic_error>([&]{binding->bind_configuration_state(first);},"same-state repeat binding is rejected");rejects<std::logic_error>([&]{binding->bind_configuration_state(second);},"foreign state rebinding is rejected");native.reset();
    std::vector<std::string> calls;auto legacy=cf::make_configuration(h.inputs(),{},std::make_unique<LegacyLicense>(calls),std::make_unique<Legacy>(docs));const std::string key("LEG\0ACY",7);legacy->apply_license_key_02a830(key);
    require(calls==std::vector<std::string>{key}&&h.stats->paths.empty()&&h.memory->writes.empty(),"existing four-argument constructor still delegates counted key without requiring native state binding");legacy.reset();
}
}
int main(){try{unbound_rebind_and_legacy_guards();runtime_binds_actual_moved_state(false);runtime_binds_actual_moved_state(true);std::cout<<"PASS: "<<checks<<" key-binding checks; actual Runtime state binding/native configuration and encrypted key output, controlled external worker boundaries\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
