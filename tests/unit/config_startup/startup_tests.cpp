// SPDX-License-Identifier: GPL-3.0-only

#include "../../support/fixture_ascii.hpp"
#include "laureondo/config/startup.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <new>
#include <stdexcept>

using namespace laureondo;
namespace {
unsigned checks=0;
void require(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,label);}
using Events=std::vector<std::string>;
configuration::Path wide(std::string_view value){return {value.begin(),value.end()};}
std::string ascii(const configuration::Path& value){return laureondo::test_support::fixture_ascii(value);}
std::size_t position(const Events& events,std::string_view value){const auto found=std::find(events.begin(),events.end(),value);require(found!=events.end(),"expected event present");return static_cast<std::size_t>(found-events.begin());}

struct FileInput final:file::Input {
    std::string bytes;Events& events;
    FileInput(std::string b,Events& e):bytes(std::move(b)),events(e){}
    std::uint32_t initial_state()const override{return 0;}
    void seek_03b610(std::int64_t offset,file::Seek)override{require(offset==0,"binary stream seeks from zero");}
    std::uint64_t position_sum_03ba50()override{return bytes.size();}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{std::copy(bytes.begin(),bytes.end(),out.begin());return bytes.size();}
    void close_03ae70()override{events.emplace_back("file-close");}
};
struct Files final:file::Factory {
    Events& events;std::string base,over;
    std::vector<configuration::Path> opened;
    explicit Files(Events& e):events(e){}
    std::unique_ptr<file::Input> open_input_039f70(const configuration::Path& path,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20 && share==0x40,"original input mode/share");opened.push_back(path);
        const bool second=path.ends_with(u"surge_override.cfg");events.emplace_back(second?"read-override":"read-base");
        return std::make_unique<FileInput>(second?over:base,events);
    }
    std::unique_ptr<file::Output> open_output_03a1a0(const configuration::Path&,std::uint32_t,std::uint32_t)override{throw std::logic_error("configuration must not open output stream here");}
};
struct Effects final:configuration::Dependencies {
    Events& events;std::string fail;
    configuration::Path executable=u"C:/fixture/host.exe";
    std::vector<configuration::Path> logs,writes,removed;
    std::vector<configuration::EmbeddedPng> images;
    std::vector<std::pair<std::string,std::string>> environment;
    bool mutate_persistence=true;
    std::string language_at_persistence;
    explicit Effects(Events& e):events(e){}
    void event(std::string name){events.push_back(name);if(fail==name)throw std::runtime_error("synthetic failure: "+name);}
    configuration::Path executable_path_03c6d0()override{event("executable");return executable;}
    configuration::Path parent_path_019820(const configuration::Path& path)override{require(path==executable,"new executable state used");return std::filesystem::path(path).parent_path().u16string();}
    configuration::Path join_016b00(const configuration::Path& a,const configuration::Path& b)override{return a+u"/"+b;}
    configuration::Path widen_0145d0(std::string_view s)override{return wide(s);}
    configuration::Path filesystem_widen_010540(std::string_view s)override{return wide(s);}
    std::string narrow_path(const configuration::Path& path)override{return ascii(path);}
    configuration::Path resolve_path_0192d0(std::string_view s,const configuration::Path& path,bool relative)override{require(path==executable && relative,"resolution receives same state path");return wide(s);}
    configuration::Path program_data_03c920()override{return u"SYNTHETIC_PROGRAMDATA";}
    configuration::Path local_app_data_03c6f0()override{event("local-app-data");return u"SYNTHETIC_LOCALAPPDATA";}
    configuration::Node load_document(const configuration::Path&)override{throw std::logic_error("old document adapter must be bypassed");}
    void log_config_path(const configuration::Path& path)override{logs.push_back(path);event(path.ends_with(u"surge_override.cfg")?"log-override":"log-base");}
    bool crt_is_space(unsigned char c)override{return c==' ' || (c>='\t' && c<='\r');}
    int crt_tolower(int c)override{return c>='A' && c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("license effect not part of fixtures");}
    void create_directories_018e00(const configuration::Path& path)override{require(path==u"SYNTHETIC_LOCALAPPDATA/SURGE","derived user directory");event("mkdir");}
    void write_embedded_png_03c020(const configuration::Path& path,configuration::EmbeddedPng image)override{writes.push_back(path);images.push_back(image);event(images.size()==1?"png-first":"png-second");}
    std::vector<configuration::Path> directory_entries(const configuration::Path& path)override{require(path==u"SYNTHETIC_LOCALAPPDATA/SURGE","enumeration scope");event("enumerate");return {u"fixture/plain.txt",u"fixture/a_AVATAR_x.png",u"fixture/lower_avatar_x.png",u"fixture/_AVATAR_dir"};}
    configuration::Path filename(const configuration::Path& path)override{const auto slash=path.find_last_of(u"/\\");return slash==configuration::Path::npos?path:path.substr(slash+1);}
    void remove_04948c(const configuration::Path& path)override{removed.push_back(path);event(removed.size()==1?"remove-first":"remove-second");}
    void update_achievement_storage_01eb00(configuration::State& state)override{
        require(state.game_attributes.at("AVAILABLE_VERSION")=="base-version","available version assigned before persistence");
        language_at_persistence=state.game_attributes["INSTALLED_LANGUAGE"];event("persistence");
        if(mutate_persistence){state.content_id="persistence-content";state.game_attributes["INSTALLED_LANGUAGE"]="persistence-language";}
    }
    bool set_environment_variable(std::string_view name,std::string_view value)override{environment.emplace_back(name,value);event("env:"+std::string(name));return false;}
};
struct InputPlatform final:startup::InputPlatform {
    Events& events;std::string arguments="-emuGameLang cli-language -showLSX -showConsole";
    std::string retained;
    explicit InputPlatform(Events& e):events(e){}
    const char* crt_getenv_05f318(const char* name)override{require(std::string_view(name)=="OriginEmuCmdArgs","CRT getenv input");events.emplace_back("arguments");return arguments.c_str();}
    const wchar_t* get_command_line_w()override{throw std::logic_error("synthetic environment should avoid real command line");}
    wchar_t** command_line_to_argv_w(const wchar_t*,int*)override{throw std::logic_error("not invoked");}
    void local_free(void*)override{throw std::logic_error("not invoked");}
    int utf8_no_flags(const wchar_t*,int,char*,int)override{throw std::logic_error("not invoked");}
    bool set_environment_variable_a(const char* name,const char* value)override{require(std::string_view(name)=="OriginEmuCmdArgs","argument output name");retained=value;events.emplace_back("arguments-publish");return false;}
};
struct Console final:startup::ConsolePlatform {
    Events& events;unsigned attaches=0,allocations=0,reopens=0;
    explicit Console(Events& e):events(e){}
    bool attach_console(std::uint32_t id)override{require(id==0xffffffffu,"attach parent process");++attaches;events.emplace_back("console-attach");return false;}
    bool allocate_console()override{++allocations;events.emplace_back("console-allocate");return false;}
    startup::ConsoleHandle console_window()override{return 1;}
    bool move_window(startup::ConsoleHandle h,int x,int y,int w,int height,bool repaint)override{require(h==1 && x==100 && y==100 && w==800 && height==500 && repaint,"console move arguments");return false;}
    std::uintptr_t stream_052238(unsigned index)override{return 0x100+index;}
    int reopen_05f024(std::uintptr_t destination,const char*,const char*,std::uintptr_t stream)override{require(destination==stream,"both opaque stream pointers preserved");++reopens;return -1;}
};

struct BootstrapEffects final:config_startup::BootstrapConfiguration {
    configuration::State& state;Events& events;unsigned started=0,closed=0,allocations=0;
    std::vector<void*> owned;
    static inline BootstrapEffects* current=nullptr;
    BootstrapEffects(configuration::State& s,Effects& deps,Files& files,config_startup::ConsoleFlags flags,InputPlatform& input,Console& console,Events& e)
        :BootstrapConfiguration(s,deps,files,flags,input,console),state(s),events(e){current=this;}
    ~BootstrapEffects()override{for(void* block:owned)::operator delete(block);current=nullptr;}
    static void worker_fixture(){}
    static void boolean_fixture(bool value){require(current && value,"only synchronous true branch invoked");current->events.emplace_back("sync-boolean");}
    int register_exit(system_services::RegisteredCallback id)override{require(id==system_services::RegisteredCallback::exit_038950,"exit callback identity");events.emplace_back("register-exit");return -1;}
    void install_terminate(system_services::RegisteredCallback id)override{require(id==system_services::RegisteredCallback::terminate_038890,"terminate callback identity");events.emplace_back("install-terminate");}
    void* log_insert_006880(void* stream,std::uintptr_t)override{events.emplace_back("bootstrap-log-part");return stream;}
    void log_finish_0088e0(void*)override{events.emplace_back("bootstrap-log-finish");}
    void* allocate_04b0bc(std::size_t bytes)override{++allocations;events.emplace_back("allocate");auto* p=::operator new(bytes);owned.push_back(p);return p;}
    void delete_04b0f8(void*,std::size_t)override{throw std::logic_error("test-owned payload handled by fake thread boundary");}
    system_services::NoArgumentFunction worker(system_services::Worker)override{return &worker_fixture;}
    system_services::BooleanFunction boolean_worker_035850()override{return &boolean_fixture;}
    system_services::Handle begin_thread_05e8e0(system_services::ThreadEntry entry,void* payload,std::uint32_t* id)override{
        require(state.username=="override-user","worker sees completed configuration with SDK override ignored");
        require(state.game_attributes.at("AVAILABLE_VERSION")=="base-version","worker sees post-configuration state");
        if(entry==system_services::ThreadEntry::no_argument_0388a0)require(static_cast<system_services::NoArgumentPayload*>(payload)->function==&worker_fixture,"noargument payload identity");
        else require(!static_cast<system_services::BooleanPayload*>(payload)->value && static_cast<system_services::BooleanPayload*>(payload)->function==&boolean_fixture,"boolean false payload identity");
        ++started;*id=started;events.emplace_back("thread-"+std::to_string(started));
        const auto found=std::find(owned.begin(),owned.end(),payload);require(found!=owned.end(),"owned test payload");owned.erase(found);::operator delete(payload);
        return 0x200+started;
    }
    bool close_handle(system_services::Handle h)override{require(h==0x201+closed,"close matches synthetic thread");++closed;events.emplace_back("close-thread");return true;}
    [[noreturn]]void throw_thread_error_04a648(int)override{throw std::runtime_error("unexpected thread error");}
    void between_workers_00e310()override{events.emplace_back("between-workers");}
    void guard_return_017a30()override{events.emplace_back("guard-return");}
    void cleanup_threads_04a47c()override{throw std::logic_error("workers not executed");}
    void cleanup_035ab0()override{throw std::logic_error("not invoked");}
    void cleanup_0366f0()override{throw std::logic_error("not invoked");}
    void cleanup_0102b0()override{throw std::logic_error("not invoked");}
    void terminate_log_038920()override{throw std::logic_error("not invoked");}
    [[noreturn]]void unexpected_terminate_return_trap()override{throw std::logic_error("not invoked");}
};
std::string base_text(){
    std::string content;for(unsigned i=0;i<12;++i)content+="\"id"+std::to_string(i)+"\" {} ";
    return "\"Config2\" {\"Game\" {\"Name\" \"base-game\" \"Version\" \"base-version\" \"ContentId\" \"base-content\" \"Language\" \"base-language\" \"LanguageRegistrySpoof\" \"false\"} \"Emulator\" {\"SDK\" \"1.2.3\"} \"User\" {\"Username\" \"base-user\"} \"Content\" {"+content+"}}";
}
struct Fixture {
    Events events;configuration::State state{};Files files{events};Effects deps{events};InputPlatform input{events};Console console{events};
    std::uint8_t show_lsx=0,show_console=0,no_console=0;
    BootstrapEffects bridge{state,deps,files,{show_lsx,show_console,no_console},input,console,events};
    system_services::Bootstrap bootstrap{bridge};
    Fixture(){state.executable_path=u"old-path";files.base=base_text();files.over=R"("Config2" {"Game" {"Name" "ignored-name" "Version" "ignored-version" "ContentId" "ignored-content" "Language" "override-language" "LanguageRegistrySpoof" "false"} "User" {"Username" "override-user"} "Content" {"ignored" {}}})";}
};
void whole_startup() {
    Fixture f;f.bootstrap.initialize();
    require(f.state.executable_path==u"C:/fixture/host.exe","module path initialized before reading");
    require(f.files.opened==std::vector<configuration::Path>{u"C:/fixture/surge.cfg",u"C:/fixture/surge_override.cfg"} && f.deps.logs==f.files.opened,"base/override raw file reads and logs");
    require(f.state.content.size()==12,"grown base collection survives override skip");
    require(f.state.game_attributes.at("DISPLAY_NAME")=="base-game" && f.state.game_attributes.at("INSTALLED_VERSION")=="base-version","override base-only metadata skip");
    require(f.state.username=="override-user","override ordinary setting");
    require(f.state.language_from_arguments==1 && f.state.registry_spoof==1 && f.deps.language_at_persistence=="cli-language","same state CLI flags lock both configuration passes");
    require(f.show_lsx==1 && f.show_console==1 && f.no_console==0,"shared startup flags");
    require(f.input.retained=="-emuGameLang cli-language -showLSX -showConsole","actual argument filtering output");
    require(f.console.attaches==1 && f.console.allocations==1 && f.console.reopens==3,"actual console orchestration despite controlled failures");
    require(f.state.cookie_path==u"SYNTHETIC_LOCALAPPDATA/SURGE/remid_cookie" && f.state.last_profile_path==u"SYNTHETIC_LOCALAPPDATA/SURGE/last_profile" && f.state.last_avatar_path==u"SYNTHETIC_LOCALAPPDATA/SURGE/last_avatar","per-user paths after both passes");
    require(f.deps.images==std::vector<configuration::EmbeddedPng>{configuration::EmbeddedPng::default_user_1800809d0,configuration::EmbeddedPng::default_avatar_180082710},"default embedded asset identity/order");
    require(f.deps.writes==std::vector<configuration::Path>{u"SYNTHETIC_LOCALAPPDATA/SURGE/SURGE.png",u"SYNTHETIC_LOCALAPPDATA/SURGE/default.png"},"default avatar paths");
    require(f.deps.removed==std::vector<configuration::Path>{u"fixture/a_AVATAR_x.png",u"fixture/_AVATAR_dir"},"case-sensitive avatar cleanup without extension filter");
    require(f.deps.environment==std::vector<std::pair<std::string,std::string>>{{"ContentId","persistence-content"},{"EAGameLocale","persistence-language"}},"persistence mutation observed by later environment output");
    require(f.bridge.started==3 && f.bridge.closed==3 && f.bridge.owned.empty(),"all test-owned payloads consumed without running real workers");
    require(position(f.events,"arguments-publish")<position(f.events,"console-attach") && position(f.events,"console-attach")<position(f.events,"bootstrap-log-finish") && position(f.events,"bootstrap-log-finish")<position(f.events,"executable"),"bootstrap environment/console/log/config order");
    require(position(f.events,"log-base")<position(f.events,"read-override") && position(f.events,"log-override")<position(f.events,"mkdir"),"two passes before post-configuration effects");
    require(position(f.events,"png-second")<position(f.events,"enumerate") && position(f.events,"remove-second")<position(f.events,"persistence") && position(f.events,"persistence")<position(f.events,"env:ContentId") && position(f.events,"env:EAGameLocale")<position(f.events,"thread-1"),"post-config order and environment completion before worker");
    require(position(f.events,"thread-1")<position(f.events,"between-workers") && position(f.events,"between-workers")<position(f.events,"thread-2") && position(f.events,"thread-2")<position(f.events,"sync-boolean") && position(f.events,"sync-boolean")<position(f.events,"thread-3") && f.events.back()=="guard-return","later bootstrap order retained");
}
void custom_avatar_and_no_console() {
    Fixture f;f.input.arguments="-noConsole -emuGameLang locked";f.state.avatar=u"synthetic-custom-avatar";f.deps.mutate_persistence=false;
    f.bootstrap.initialize();
    require(f.no_console==1 && f.console.attaches==0 && f.console.allocations==0 && f.console.reopens==0,"noConsole is the same flag consumed by console setup");
    require(f.deps.images==std::vector<configuration::EmbeddedPng>{configuration::EmbeddedPng::default_avatar_180082710} && f.state.avatar==u"synthetic-custom-avatar","custom avatar suppresses only user-default asset");
    require(f.deps.environment[1].second=="locked","CLI language survives config with no persistence change");
}
void failure_boundaries() {
    {Fixture f;f.files.base="\"Config2\" {";rejects<parser::ParseError>([&]{f.bootstrap.initialize();},"base parser failure propagates");require(f.files.opened.size()==1 && f.deps.logs.empty() && f.bridge.allocations==0 && f.deps.writes.empty(),"base error stops override/post/worker");}
    {Fixture f;f.files.over="\"Config2\" {";rejects<parser::ParseError>([&]{f.bootstrap.initialize();},"override parser failure propagates");require(f.files.opened.size()==2 && f.deps.logs.size()==1 && f.state.username=="base-user" && f.state.content.size()==12 && f.bridge.allocations==0 && f.deps.writes.empty(),"override failure retains base, stops later steps");}
    for(const auto* failure:{"mkdir","png-first","enumerate","remove-first","persistence","env:ContentId","env:EAGameLocale"}) {
        Fixture f;f.deps.fail=failure;
        rejects<std::runtime_error>([&]{f.bootstrap.initialize();},"controlled post-configuration failure propagates");
        require(f.files.opened.size()==2 && f.deps.logs.size()==2 && f.state.username=="override-user" && f.bridge.allocations==0,"post error happens after both passes, before workers");
        require(f.events.back()==failure,"no subsequent step runs after failure");
        if(std::string_view(failure)=="mkdir")require(f.deps.writes.empty() && f.deps.environment.empty(),"directory failure stops PNG/environment");
        if(std::string_view(failure)=="png-first")require(f.deps.images.size()==1 && f.deps.removed.empty(),"first PNG failure stops second and cleanup");
        if(std::string_view(failure)=="persistence")require(f.deps.environment.empty() && f.state.content_id=="base-content","persistence exception stops mutation/environment");
        if(std::string_view(failure)=="env:ContentId")require(f.deps.environment.size()==1,"first environment error stops second");
    }
}
void native_generated_files(const std::filesystem::path& directory) {
    std::filesystem::create_directories(directory);
    const auto base=directory/"surge.cfg",over=directory/"surge_override.cfg";
    {std::ofstream file(base,std::ios::binary|std::ios::trunc);file<<base_text();require(static_cast<bool>(file),"own native base fixture written");}
    {std::ofstream file(over,std::ios::binary|std::ios::trunc);file<<R"("Config2" {"User" {"Username" "native-override-user"} "Content" {"ignored" {}}})";require(static_cast<bool>(file),"own native override fixture written");}
    Events events;Effects deps(events);configuration::State state{};deps.executable=(directory/"synthetic-host.exe").u16string();deps.mutate_persistence=false;
    file::NativeFactory files;
    config_startup::initialize_configuration(state,deps,files);
    require(state.username=="native-override-user" && state.content.size()==12,"real own files reach base collection and override application");
    require(deps.logs.size()==2 && std::filesystem::path(deps.logs[0]).lexically_normal()==base.lexically_normal() && std::filesystem::path(deps.logs[1]).lexically_normal()==over.lexically_normal(),"own native base/override paths logged in order");
    require(deps.environment.size()==2 && deps.environment[1].second=="base-language","native read path reaches controlled post-configuration work");
}
}
int main(int argc,char** argv) {
    try{if(argc!=2)throw std::invalid_argument("requires owned fixture directory");whole_startup();custom_avatar_and_no_console();failure_boundaries();native_generated_files(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" Config2 startup checks; own generated files, all other OS/worker effects controlled\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
