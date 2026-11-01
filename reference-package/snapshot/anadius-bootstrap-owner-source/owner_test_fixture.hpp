#pragma once
#include "owner.hpp"
#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <process.h>
namespace b2_test {
using namespace anadius;
namespace o=bootstrap_owner;namespace n=bootstrap_native_recovered;namespace t=threads_recovered;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,label);}
using Events=std::vector<std::string>;
inline configuration::Path wide(std::string_view s){return {s.begin(),s.end()};}
inline std::string ascii(const configuration::Path& s){return {s.begin(),s.end()};}
inline std::size_t position(const Events& e,std::string_view s){auto it=std::find(e.begin(),e.end(),s);require(it!=e.end(),"expected event present");return static_cast<std::size_t>(it-e.begin());}
struct FileInput final:file_recovered::Input {
    std::string bytes;Events& events;
    FileInput(std::string b,Events& e):bytes(std::move(b)),events(e){}
    std::uint32_t initial_state()const override{return 0;}
    void seek_03b610(std::int64_t offset,file_recovered::Seek)override{require(offset==0,"binary stream seeks from zero");}
    std::uint64_t position_sum_03ba50()override{return bytes.size();}
    std::int32_t peek_043790()override{throw std::logic_error("unexpected peek");}
    std::size_t read_03b2a0(std::span<std::uint8_t> out)override{std::copy(bytes.begin(),bytes.end(),out.begin());return bytes.size();}
    void close_03ae70()override{events.emplace_back("file-close");}
};
struct Files final:file_recovered::Factory {
    Events& events;std::string base,over;
    std::vector<configuration::Path> opened;
    explicit Files(Events& e):events(e){}
    std::unique_ptr<file_recovered::Input> open_input_039f70(const configuration::Path& path,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20 && share==0x40,"original input mode/share");opened.push_back(path);
        const bool second=path.ends_with(u"anadius_override.cfg");events.emplace_back(second?"read-override":"read-base");
        return std::make_unique<FileInput>(second?over:base,events);
    }
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const configuration::Path&,std::uint32_t,std::uint32_t)override{throw std::logic_error("configuration must not open output stream here");}
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
    void log_config_path(const configuration::Path& path)override{logs.push_back(path);event(path.ends_with(u"anadius_override.cfg")?"log-override":"log-base");}
    bool crt_is_space(unsigned char c)override{return c==' ' || (c>='\t' && c<='\r');}
    int crt_tolower(int c)override{return c>='A' && c<='Z'?c+32:c;}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("license effect not part of fixtures");}
    void create_directories_018e00(const configuration::Path& path)override{require(path==u"SYNTHETIC_LOCALAPPDATA/anadius/LSX emu","derived user directory");event("mkdir");}
    void write_embedded_png_03c020(const configuration::Path& path,configuration::EmbeddedPng image)override{writes.push_back(path);images.push_back(image);event(images.size()==1?"png-first":"png-second");}
    std::vector<configuration::Path> directory_entries(const configuration::Path& path)override{require(path==u"SYNTHETIC_LOCALAPPDATA/anadius/LSX emu","enumeration scope");event("enumerate");return {u"fixture/plain.txt",u"fixture/a_AVATAR_x.png",u"fixture/lower_avatar_x.png",u"fixture/_AVATAR_dir"};}
    configuration::Path filename(const configuration::Path& path)override{const auto slash=path.find_last_of(u"/\\");return slash==configuration::Path::npos?path:path.substr(slash+1);}
    void remove_04948c(const configuration::Path& path)override{removed.push_back(path);event(removed.size()==1?"remove-first":"remove-second");}
    void update_achievement_storage_01eb00(configuration::State& state)override{
        require(state.sdk=="1,2,3","SDK normalization before persistence");
        require(state.game_attributes.at("AVAILABLE_VERSION")=="base-version","available version assigned before persistence");
        language_at_persistence=state.game_attributes["INSTALLED_LANGUAGE"];event("persistence");
        if(mutate_persistence){state.content_id="persistence-content";state.game_attributes["INSTALLED_LANGUAGE"]="persistence-language";}
    }
    bool set_environment_variable(std::string_view name,std::string_view value)override{environment.emplace_back(name,value);event("env:"+std::string(name));return false;}
};
struct InputPlatform final:startup_recovered::InputPlatform {
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
struct Console final:startup_recovered::ConsolePlatform {
    Events& events;unsigned attaches=0,allocations=0,reopens=0;
    explicit Console(Events& e):events(e){}
    bool attach_console(std::uint32_t id)override{require(id==0xffffffffu,"attach parent process");++attaches;events.emplace_back("console-attach");return false;}
    bool allocate_console()override{++allocations;events.emplace_back("console-allocate");return false;}
    startup_recovered::ConsoleHandle console_window()override{return 1;}
    bool move_window(startup_recovered::ConsoleHandle h,int x,int y,int w,int height,bool repaint)override{require(h==1 && x==100 && y==100 && w==800 && height==500 && repaint,"console move arguments");return false;}
    std::uintptr_t stream_052238(unsigned index)override{return 0x100+index;}
    int reopen_05f024(std::uintptr_t destination,const char*,const char*,std::uintptr_t stream)override{require(destination==stream,"both opaque stream pointers preserved");++reopens;return -1;}
};


struct Fault{n::LifecycleFault reason;};
struct PermanentErrors final:n::Errors {
    [[noreturn]] void lifecycle(n::LifecycleFault f)override{throw Fault{f};}
    [[noreturn]] void callback_exception(n::Entry,std::exception_ptr)noexcept override{std::_Exit(86);}
};
inline PermanentErrors permanent_errors;
struct Track {Events events;unsigned provider_queries=0,binds=0,stops=0,destroyed=0;std::string destroyed_state;};
struct Threads final:t::Effects {
    Events& events;std::vector<std::unique_ptr<t::Context>> contexts;std::vector<t::Context*> pending;
    std::vector<t::Handle> closed;unsigned null_frees=0,creations=0;bool fail_create=false;
    explicit Threads(Events& e):events(e){}
    t::Context* calloc_context_066848()override{events.emplace_back("calloc-context");contexts.push_back(std::make_unique<t::Context>());return contexts.back().get();}
    void free_0668c0(t::Context* p)override{if(!p){++null_frees;events.emplace_back("free-null-context");return;}events.emplace_back("free-context");}
    bool get_module_handle_ex_w(std::uint32_t f,std::uintptr_t code,t::Handle* out)override{require(f==4&&code&&!*out,"source begin requests module by rebuilt entry");return false;}
    t::Handle create_thread(void* security,std::uint64_t stack,t::Context* c,std::uint32_t flags,std::uint32_t* id)override{
        require(!security&&!stack&&!flags,"existing begin wrapper exact create operands");++creations;events.emplace_back("thread-"+std::to_string(creations));
        if(fail_create)return 0;pending.push_back(c);*id=creations+10;return 0x300+creations;
    }
    std::uint32_t last_error()override{return 5;}
    void write_error_slot_05d724(std::uint32_t)override{throw std::logic_error("unexpected errno");}
    void invalid_parameter_05c954()override{throw std::logic_error("unexpected invalid parameter");}
    void map_os_error_05d698(std::uint32_t v)override{require(v==5,"thread failure mapped exact OS error");events.emplace_back("map-error");}
    bool close_handle(t::Handle h)override{closed.push_back(h);events.emplace_back("close-thread");return true;}
    bool free_library(t::Handle)override{throw std::logic_error("unused FreeLibrary");}
    t::Context*& required_thread_slot_064b90()override{throw std::logic_error("runtime PTD remains supplied");}
    t::Context* optional_thread_context_064d08()override{throw std::logic_error("runtime PTD remains supplied");}
    int runtime_policy_06c0d0()override{throw std::logic_error("runtime policy not requested");}
    int initialize_runtime_067320(int)override{throw std::logic_error("runtime init not requested");}
    void uninitialize_runtime_06736c()override{throw std::logic_error("runtime teardown not requested");}
    std::uint32_t invoke_entry(std::uintptr_t,std::uintptr_t)override{throw std::logic_error("whole CRT trampoline remains open");}
    [[noreturn]]void free_library_and_exit_thread(t::Handle,std::uint32_t)override{throw std::logic_error("not native worker exit");}
    [[noreturn]]void exit_thread(std::uint32_t)override{throw std::logic_error("not native worker exit");}
};
inline std::string base_text(){return R"("Config2" {"Game" {"Name" "base-game" "Version" "base-version" "ContentId" "base-content" "Language" "base-language" "LanguageRegistrySpoof" "false"} "Emulator" {"SDK" "1.2.3"} "User" {"Username" "base-user"}})";}
struct Backend final:o::ProviderOwner,o::Services {
    Track& track;Files files;Effects config;InputPlatform input;Console console;Threads threads;
    configuration::State* state=nullptr;std::optional<config_startup::ConsoleFlags> flags;
    bool bind_failure=false,stop_success=true,stop_failure=false,drain=true;
    n::ExitCallback exit_callback=nullptr;std::terminate_handler terminate_callback=nullptr;
    std::vector<void*> payloads;std::vector<std::size_t> allocated,deleted;
    std::function<void()> guard_hook,stop_hook;std::string service_failure;
    static inline Backend* active=nullptr;
    explicit Backend(Track& tr):track(tr),files(tr.events),config(tr.events),input(tr.events),console(tr.events),threads(tr.events){
        files.base=base_text();files.over=R"("Config2" {"User" {"Username" "override-user"}})";
    }
    ~Backend()override{++track.destroyed;if(state)track.destroyed_state=state->username;for(auto* p:payloads)::operator delete(p);if(active==this)active=nullptr;}
    o::Providers providers()override{++track.provider_queries;return {config,files,input,console,threads,*this};}
    void bind_state(configuration::State& s,config_startup::ConsoleFlags f)override{++track.binds;state=&s;flags.emplace(f);if(bind_failure)throw std::runtime_error("bind-failure");}
    bool stop_and_join()override{
        ++track.stops;track.events.emplace_back("stop-join");if(stop_hook)stop_hook();if(stop_failure)throw std::runtime_error("stop-failure");if(!stop_success)return false;
        if(drain){active=this;for(auto* c:threads.pending){const auto call=reinterpret_cast<n::ThreadStart>(c->entry);require(call==n::Resolver{}.thread_entry(recovered::ThreadEntry::no_argument_0388a0)||call==n::Resolver{}.thread_entry(recovered::ThreadEntry::boolean_0388d0),"queued entry is rebuilt native source");require(call(reinterpret_cast<void*>(c->argument))==0,"source native callback completes");}threads.pending.clear();}return true;
    }
    void event(const char* s){track.events.emplace_back(s);if(service_failure==s)throw std::runtime_error(s);}
    int register_exit(n::ExitCallback f)override{exit_callback=f;event("register-exit");return -1;}
    void install_terminate(std::terminate_handler f)override{terminate_callback=f;event("install-terminate");}
    void* log_insert(void* stream,std::uintptr_t)override{event("bootstrap-log-part");return stream;}
    void log_finish(void*)override{event("bootstrap-log-finish");}
    void* allocate(std::size_t size)override{event("allocate");allocated.push_back(size);auto* p=::operator new(size);payloads.push_back(p);return p;}
    void deallocate(void* p,std::size_t size)override{deleted.push_back(size);event("delete-payload");auto it=std::find(payloads.begin(),payloads.end(),p);require(it!=payloads.end(),"payload belongs to owner backend");payloads.erase(it);::operator delete(p);}
    static void first(){require(active&&active->state&&active->state->username=="override-user","first worker sees shared configured owner state");active->event("worker-first");}
    static void second(){require(active&&active->state&&active->state->sdk=="1,2,3","second worker sees actual normalized SDK");active->event("worker-second");}
    static void boolean(bool value){require(active&&active->state,"boolean worker has bound state");active->event(value?"sync-boolean":"worker-boolean");}
    recovered::NoArgumentFunction worker(recovered::Worker which)override{active=this;return which==recovered::Worker::first_029ea0?&first:&second;}
    recovered::BooleanFunction boolean_worker()override{active=this;return &boolean;}
    [[noreturn]]void thread_error(int index)override{require(index==1||index==6,"source thread error index");throw std::runtime_error("thread-error");}
    void between_workers()override{event("between-workers");}
    void guard_return()override{event("guard-return");if(guard_hook)guard_hook();}
    void cleanup_threads()override{event("cleanup-thread");}
    void cleanup_first()override{event("cleanup-first");}
    void cleanup_second()override{event("cleanup-second");}
    void cleanup_network()override{event("cleanup-network");}
    void terminate_log()override{event("terminate-log");}
    [[noreturn]]void unexpected_terminate_return()override{throw std::runtime_error("terminate-trap");}
};
inline o::InitialState seed(){configuration::State s{};s.username="seed-user";s.sdk="seed-sdk";return {std::move(s),{0,0,0}};}
struct Owned {
    Track track;Backend* backend;std::unique_ptr<o::RuntimeOwner> owner;
    Owned(){auto p=std::make_unique<Backend>(track);backend=p.get();owner=std::make_unique<o::RuntimeOwner>(seed(),std::move(p),permanent_errors);}
    ~Owned(){if(owner&&(owner->phase()==o::Phase::Bound||owner->phase()==o::Phase::Stopping)){backend->stop_success=true;backend->stop_failure=false;backend->drain=true;backend->guard_hook={};try{owner->close();}catch(...){std::terminate();}}}
};
} // namespace b2_test
