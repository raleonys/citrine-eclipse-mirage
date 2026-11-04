// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/achievement_runtime.hpp"
#include "preloader_l/runtime/timestamp.hpp"
#include "preloader_l/xml/reader.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <windows.h>

namespace ar=preloader_l::achievement_runtime;
namespace content=preloader_l::content_handlers;
namespace cfg=preloader_l::configuration;
namespace io=preloader_l::file;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class E,class F>void rejects(F f,const char* why){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected provider in achievement-only test");}
struct Mutex final:content::Mutex {
    bool held=false;std::int32_t count=1;unsigned locks=0,unlocks=0;
    int lock_04a6ac()override{require(!held,"synthetic mutex not recursively entered");held=true;++locks;return 0;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override{require(held,"unlock held mutex");held=false;++unlocks;}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
};
struct Queue final:content::EventQueue {
    Mutex mutex;Mutex& achievement;bool initialized=false;std::vector<std::string> entries;
    explicit Queue(Mutex& outer):achievement(outer){}
    content::Mutex& mutex_02c620()override{return mutex;}
    bool initialized_locked()const override{require(mutex.held && achievement.held,"queue access under both locks");return initialized;}
    void initialize_locked(std::string value)override{require(mutex.held && achievement.held && !initialized,"lazy initial event under locks");initialized=true;entries.push_back(std::move(value));}
    void push_persisted_locked(std::string value,const std::function<void()>& persist)override {
        push_locked_034660(std::move(value));try{persist();}catch(...){entries.pop_back();throw;}
    }
    void push_locked_034660(std::string value)override{require(mutex.held && achievement.held && initialized,"push event under both locks");entries.push_back(std::move(value));}
};
struct Clock final:ar::Clock {
    Mutex& mutex;unsigned calls=0;bool absent=false;
    ar::BrokenDown fields{0,0,0,1,0,100,6,0,0};
    explicit Clock(Mutex& m):mutex(m){}
    std::int64_t time_05eeb8()override{require(mutex.held,"clock invoked under achievement lock");++calls;return 123;}
    std::optional<ar::BrokenDown> gmtime_05fffc(std::int64_t raw)override{require(raw==123,"clock forwarded to gmtime");if(absent)return std::nullopt;return fields;}
};
struct Directories final:preloader_l::achievement_writer::Effects {
    Mutex& mutex;std::vector<std::string>& trace;std::vector<cfg::Path> paths;bool native=true;
    Directories(Mutex& m,std::vector<std::string>& t):mutex(m),trace(t){}
    void create_directories_018e00(const cfg::Path& path)override{require(mutex.held,"INI parent creation under lock");trace.push_back("mkdir");paths.push_back(path);if(native)std::filesystem::create_directories(std::filesystem::path(path));}
};
struct Factory final:io::Factory {
    struct Proxy final:io::Output {
        Factory& owner;std::string kind;std::unique_ptr<io::Output> inner;bool initial_failure,write_exception;
        Proxy(Factory& o,std::string k,std::unique_ptr<io::Output> i,bool f,bool e):owner(o),kind(std::move(k)),inner(std::move(i)),initial_failure(f),write_exception(e){}
        std::uint32_t initial_state()const override{return initial_failure?2:inner?inner->initial_state():0;}
        void write_03be80(std::string_view bytes)override{require(owner.mutex.held,"actual output write under achievement lock");owner.trace.push_back("write:"+kind);if(write_exception)throw std::runtime_error("synthetic write exception");owner.writes.emplace_back(kind,bytes);if(inner)inner->write_03be80(bytes);}
        void close_03ae70()override{owner.trace.push_back("close:"+kind);if(inner)inner->close_03ae70();}
    };
    Mutex& mutex;std::vector<std::string>& trace;const cfg::Path& xml_path;
    io::NativeFactory native;bool native_io=true,xml_false=false,xml_throw=false;
    std::vector<std::pair<std::string,std::string>> writes;std::vector<cfg::Path> opened;
    Factory(Mutex& m,std::vector<std::string>& t,const cfg::Path& x):mutex(m),trace(t),xml_path(x){}
    void replace_atomic(std::span<const io::FileWrite> batch)override {
        require(mutex.held,"atomic group under achievement lock");trace.push_back("atomic");
        if(xml_false||xml_throw)throw std::runtime_error("injected atomic staging failure");
        if(native_io)native.replace_atomic(batch);
        for(const auto& w:batch)writes.emplace_back(w.path==xml_path?"xml":"ini",w.bytes);
    }
    std::unique_ptr<io::Input> open_input_039f70(const cfg::Path&,std::uint32_t,std::uint32_t)override{unused();}
    std::unique_ptr<io::Output> open_output_03a1a0(const cfg::Path& path,std::uint32_t mode,std::uint32_t share)override{
        require(mutex.held && mode==0x20 && share==0x40,"recovered file body forwards binary/share modes under lock");
        const auto kind=path==xml_path?"xml":"ini";trace.push_back(std::string("open:")+kind);opened.push_back(path);
        const bool fail=kind==std::string("xml") && xml_false,throws=kind==std::string("xml") && xml_throw;
        std::unique_ptr<io::Output> output;if(native_io && !fail && !throws)output=native.open_output_03a1a0(path,mode,share);
        return std::make_unique<Proxy>(*this,kind,std::move(output),fail,throws);
    }
};
struct Effects final:ar::PersistenceEffects {
    std::int32_t atoi_value=0;unsigned atoi_calls=0;
    using PersistenceEffects::PersistenceEffects;
    std::int32_t atoi_05ee08(std::string_view)override{++atoi_calls;return atoi_value;}
    void unknown_entitlement_field(std::string_view)override{unused();}
    cfg::Path filesystem_widen_006d70(std::string_view)override{unused();}
    cfg::Path path_append_016b50(const cfg::Path&,const cfg::Path&)override{unused();}
    bool online_avatar_virtual_28(const cfg::Path&)override{unused();}
    bool copy_overwrite_048d20(const cfg::Path&,const cfg::Path&)override{unused();}
    std::string path_to_utf8_031ca0(const cfg::Path&)override{unused();}
    void log_saved_size_085530(std::int32_t)override{unused();}
};
struct Fixture {
    cfg::State state{};preloader_l::achievement_storage::Paths paths;
    Mutex mutex;Queue events{mutex};Clock clock{mutex};ar::HostFormatter formatter;
    std::vector<std::string> trace;Directories dirs{mutex,trace};Factory files{mutex,trace,paths.xml_path};
    Effects effects{state,paths,clock,formatter,dirs,files};std::uint8_t online=0;std::int32_t saved=0;
    explicit Fixture(const std::filesystem::path& directory){
        std::filesystem::create_directories(directory);
        paths.xml_path=(directory/"runtime.xml").u16string();paths.ini_path=(directory/"watcher"/"achievements.ini").u16string();
        state.user_directory=directory.u16string();state.achievements_set="set";state.content_id="content";state.user_id="user";
        state.game_attributes["DISPLAY_NAME"]="G&\"<";
        state.achievements.emplace("a",cfg::Achievement{2,0,0,"0000-00-00T00:00:00","A&\"<","100",0});
        state.achievements.emplace("negative",cfg::Achievement{3,0,-1,"old","negative","200",5});
        state.achievements.emplace("skip",cfg::Achievement{1,0,0,"old","skip","300",7});
    }
    content::StateReferences refs(){return ar::bind_state(state,paths,online,saved,mutex,events);}
    std::string grant(const char* id,const char* progress=nullptr){
        content::RequestTree request;if(id)request.attributes.emplace_back("AchievementId",id);if(progress)request.attributes.emplace_back("Progress",progress);
        auto response=effects.printer_0019b0();const bool result=content::grant_achievement_030010(request,response->xml(),refs(),effects);
        require(result==(id!=nullptr),"grant handler result reflects required ID");return response->c_string_without_nul();
    }
};
std::string read(const cfg::Path& path){std::ifstream input(std::filesystem::path(path),std::ios::binary);require(static_cast<bool>(input),"own generated file exists");return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};}
void bridge_and_partial(const std::filesystem::path& root){
    Fixture f(root/"partial");const auto refs=f.refs();
    require(&refs.game_0a3d88==&f.state.game_attributes && &refs.achievements_0a3e58==&f.state.achievements,"bind_state shares actual game and achievement maps");
    require(&refs.achievement_file_0a3de8==&f.paths.xml_path && &refs.online_0a3c3b==&f.online && &refs.saved_avatar_size_0a1130==&f.saved,"bind_state shares live path and external cells");
    require(&refs.avatar_0a3a90==&f.state.avatar && &refs.last_avatar_0a3a60==&f.state.last_avatar_path && &refs.events==&f.events && &refs.achievements_mutex_0a3e08==&f.mutex,"bind_state shares avatar/lock/event identities");
    f.state.content.push_back({"content","game","1","ready"});f.state.entitlements.push_back({"offer","group","1","type","tag"});
    const auto populated=f.refs();require(populated.content_0a3d98.data()==f.state.content.data() && populated.entitlements_0a3db0.data()==f.state.entitlements.data(),"bind_state borrows vector spans");
    f.paths.xml_path=(root/"partial"/"live-path.xml").u16string();
    require(refs.achievement_file_0a3de8==f.paths.xml_path,"previous binding observes later path replacement");
    const auto response=f.grant("a");const auto& a=f.state.achievements.at("a");
    require(a.field04==0 && a.field08==1 && a.field70==946684800 && a.timestamp=="2000-01-01T00:00:00","partial grant uses actual UTC helper and recomputed epoch");
    require(f.events.entries.empty() && f.clock.calls==1 && !f.mutex.held && f.mutex.locks==f.mutex.unlocks,"partial grant persists without completion event and unlocks");
    const std::string expected="<Achievements GameName=\"G&amp;&quot;&lt;\" ContentId=\"content\">\n"
        "    <Achievement Id=\"a\" Count=\"0\" Progress=\"1\" Grant=\"2000-01-01T00:00:00\" Name=\"A&amp;&quot;&lt;\"/>\n"
        "    <Achievement Id=\"negative\" Count=\"0\" Progress=\"-1\" Grant=\"old\" Name=\"negative\"/>\n</Achievements>\n";
    require(read(f.paths.xml_path)==expected,"actual XML bytes escape attributes, persist signed nonzero progress and omit zero progress");
    require(read(f.paths.ini_path)=="[100]\r\nAchieved=0\r\nCurProgress=1\r\nMaxProgress=2\r\nUnlockTime=946684800\r\n\r\n[200]\r\nAchieved=0\r\nCurProgress=-1\r\nMaxProgress=3\r\nUnlockTime=5\r\n\r\n[300]\r\nAchieved=0\r\nUnlockTime=7\r\n\r\n","actual INI bytes include all map entries and unsigned epochs");
    require(f.trace==std::vector<std::string>{"mkdir","atomic"},"one atomic XML/INI group after parent creation");
    require(response.find("Name=\"A&amp;&quot;&lt;\"")!=std::string::npos && response.find('\0')==std::string::npos,"actual response printer escapes C strings and excludes NUL");
    auto document=f.effects.printer_0019b0();rejects<std::logic_error>([&]{(void)document->xml().attribute_002cc0(0,"Id");},"output-only printer rejects request reads");
    content::AchievementMap alien;const auto before=f.trace.size();rejects<std::invalid_argument>([&]{f.effects.save_steam_01f9a0(alien);},"alien map identity rejected explicitly");require(f.trace.size()==before,"alien map rejected before directory/file effects");
}
void completion_and_edges(const std::filesystem::path& root){
    Fixture f(root/"completion");f.state.achievements.erase("negative");f.state.achievements.erase("skip");
    (void)f.grant("a");(void)f.grant("a");
    require(f.state.achievements.at("a").field04==1 && f.state.achievements.at("a").field08==2,"completion sets count1 and clamps progress");
    require(f.events.entries.size()==2 && f.events.entries[0]=="<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>","completion lazily queues exact initial IGO event");
    const std::string event="<LSX>\n    <Event sender=\"EbisuSDK\">\n        <AchievementSets>\n            <AchievementSet GameName=\"G&amp;&quot;&lt;\" Name=\"set\">\n                <Achievement Id=\"a\" Total=\"2\" Count=\"1\" Progress=\"2\" GrantDate=\"2000-01-01T00:00:00\" Expiration=\"0000-00-00T00:00:00\" ImageId=\"\" Name=\"A&amp;&quot;&lt;\" Description=\"\" HowTo=\"\"/>\n            </AchievementSet>\n        </AchievementSets>\n    </Event>\n</LSX>\n";
    require(f.events.entries[1]==event,"completion actual XML event exact bytes");
    const auto writes=f.files.writes.size();const auto calls=f.clock.calls;(void)f.grant("a");
    require(f.files.writes.size()==writes && f.clock.calls==calls && f.events.entries.size()==2,"already complete achievement skips clock/persistence/event");
    const auto locks=f.mutex.locks;require(f.grant(nullptr).empty() && f.mutex.locks==locks,"missing ID returns before lock and effects");
    f.effects.atoi_value=5;(void)f.grant("unknown","5");
    require(f.state.achievements.at("unknown").field00==5 && f.state.achievements.at("unknown").field08==5 && f.state.achievements.at("unknown").field04==1,"unknown positive grant seeds total from increment and completes");
    require(f.events.entries.size()==3,"event queue initialized only once");
    f.effects.atoi_value=0;(void)f.grant("zero","0");require(f.state.achievements.at("zero").field00==0 && f.state.achievements.at("zero").field08==0 && f.clock.calls==calls+1,"unknown zero total bypasses progress branch");
    f.state.achievements.emplace("pending",cfg::Achievement{4,0,0,"old","","",99});
    const auto prior=f.clock.calls;(void)f.grant("pending","0");require(f.clock.calls==prior+1 && f.state.achievements.at("pending").field08==0 && f.state.achievements.at("pending").field70==946684800,"zero increment on existing pending entry still timestamps and persists");
    f.effects.atoi_value=std::numeric_limits<std::int32_t>::max();f.state.achievements.at("pending").field08=1;
    (void)f.grant("pending","overflow");require(f.state.achievements.at("pending").field08==std::numeric_limits<std::int32_t>::min() && f.state.achievements.at("pending").field04==0,"progress increment wraps32 before signed completion test");
    f.effects.atoi_value=-1;f.state.achievements.at("pending").field08=0;(void)f.grant("pending","-1");require(f.state.achievements.at("pending").field08==-1,"negative increment persists negative progress");
}
void failures_and_empty_paths(const std::filesystem::path& root){
    {Fixture f(root/"gmtime-failure");f.clock.absent=true;(void)f.grant("a");require(f.state.achievements.at("a").timestamp.empty() && f.state.achievements.at("a").field70==0,"gmtime failure persists empty Grant and epoch0");}
    for(bool failure:{false,true}) {
        Fixture f(root/(failure?"throw-xml":"false-xml"));f.files.native_io=false;f.dirs.native=false;
        f.files.xml_false=!failure;f.files.xml_throw=failure;
        rejects<std::runtime_error>([&]{(void)f.grant("a");},"atomic persistence failure propagates");
        require(f.state.achievements.at("a").field08==0&&f.state.achievements.at("a").field70==0&&!f.mutex.held,"failure restores progress/timestamp and releases lock");
        require(f.files.writes.empty()&&f.events.entries.empty(),"failed group publishes no writes/event");
        rejects<std::runtime_error>([&]{(void)f.grant("unknown");},"unknown grant failed");
        require(!f.state.achievements.contains("unknown"),"failed new grant erases newly inserted entry");
        f.effects.atoi_value=2;
        rejects<std::runtime_error>([&]{(void)f.grant("a","2");},"completion staging failure");
        require(f.events.entries.size()==1&&f.state.achievements.at("a").field04==0,"completion failure retains only initial DOWN seed");
    }
    {Fixture f(root/"empty-paths");f.paths.xml_path.clear();f.paths.ini_path.clear();(void)f.grant("a");require(f.trace.empty() && f.clock.calls==1,"empty persistence paths skip both files without skipping grant");}
}
// A11-09: actual native XML/INI atomic retry at the achievement grant boundary.
// The lower atomic file test already holds INI while replacing two files. This
// test proves the live grant's map/lock rollback and its exact second attempt.
void native_locked_ini_grant_retry(const std::filesystem::path& root){
    const auto directory=root/"locked-ini-grant";
    std::error_code cleanup_error;
    std::filesystem::remove_all(directory,cleanup_error);
    require(!cleanup_error,"remove only the owned locked-INI grant fixture directory");
    Fixture f(directory);
    const std::filesystem::path xml(f.paths.xml_path),ini(f.paths.ini_path);
    std::filesystem::create_directories(ini.parent_path());
    {
        std::ofstream output(xml,std::ios::binary|std::ios::trunc);
        output<<"preexisting-xml";
        require(static_cast<bool>(output),"seed existing XML bytes");
    }
    {
        std::ofstream output(ini,std::ios::binary|std::ios::trunc);
        output<<"preexisting-ini";
        require(static_cast<bool>(output),"seed existing INI bytes");
    }
    struct ExclusiveReplacementBarrier {
        HANDLE handle=INVALID_HANDLE_VALUE;
        explicit ExclusiveReplacementBarrier(const std::filesystem::path& path){
            handle=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,nullptr);
            require(handle!=INVALID_HANDLE_VALUE,"hold real INI destination without FILE_SHARE_DELETE");
        }
        void release(){
            if(handle!=INVALID_HANDLE_VALUE){
                const HANDLE owned=handle;
                handle=INVALID_HANDLE_VALUE;
                require(CloseHandle(owned)!=FALSE,"close exact INI lock before retry");
            }
        }
        ~ExclusiveReplacementBarrier(){if(handle!=INVALID_HANDLE_VALUE)CloseHandle(handle);}
    } held(ini);
    rejects<std::runtime_error>([&]{(void)f.grant("a");},
        "live grant rejects native second-destination replacement while INI is held");
    require(read(f.paths.xml_path)=="preexisting-xml"&&read(f.paths.ini_path)=="preexisting-ini",
        "native failed grant restores XML and retains locked INI bytes");
    const auto& restored=f.state.achievements.at("a");
    require(restored.field04==0&&restored.field08==0&&restored.field70==0&&
        restored.timestamp=="0000-00-00T00:00:00"&&!f.mutex.held&&f.events.entries.empty(),
        "native persistence failure restores in-memory grant and releases mutex without event");
    require(f.files.writes.empty()&&f.trace==std::vector<std::string>{"mkdir","atomic"},
        "failed batch publishes no committed fixture writes");
    held.release();
    (void)f.grant("a");
    require(f.state.achievements.at("a").field08==1&&f.state.achievements.at("a").field04==0&&
        f.state.achievements.at("a").field70==946684800&&!f.mutex.held&&f.events.entries.empty(),
        "unlocked retry commits exactly one partial achievement increment");
    require(read(f.paths.xml_path).find("Progress=\"1\"")!=std::string::npos&&
        read(f.paths.ini_path).find("[100]\r\nAchieved=0\r\nCurProgress=1\r\nMaxProgress=2\r\nUnlockTime=946684800\r\n\r\n")!=std::string::npos,
        "retry publishes actual XML and INI data through the native atomic owner");
    require(f.files.writes.size()==2&&f.trace==std::vector<std::string>{"mkdir","atomic","mkdir","atomic"},
        "only successful native retry records both writes");
    std::size_t xml_entries=0,ini_entries=0;
    for(const auto& entry:std::filesystem::directory_iterator(xml.parent_path())){(void)entry;++xml_entries;}
    for(const auto& entry:std::filesystem::directory_iterator(ini.parent_path())){(void)entry;++ini_entries;}
    require(xml_entries==2&&ini_entries==1,"no staging or backup files remain after a successful retry");
}
struct StorageEffects final:preloader_l::timestamp::StorageEffects {
    explicit StorageEffects(preloader_l::timestamp::Parser& parser):preloader_l::timestamp::StorageEffects(parser){}
    cfg::Path filesystem_widen_006d70(std::string_view value)override{return {value.begin(),value.end()};}
    preloader_l::status::Status status_01ea20(const cfg::Path&,std::uint32_t flags)override{require(flags==3,"roundtrip storage uses flags3");return {2,0,0};}
    [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const cfg::Path&)override{unused();}
    void create_directories_018e00(const cfg::Path&)override{unused();}
};
void roundtrip(const std::filesystem::path& root){
    Fixture f(root/"roundtrip");f.state.achievements.erase("negative");f.state.achievements.erase("skip");(void)f.grant("a");
    cfg::State restored{};restored.user_directory=f.state.user_directory;restored.steam_app_id="42";
    restored.achievements.emplace("a",cfg::Achievement{2,0,0,"old","","100",999});
    preloader_l::achievement_storage::Paths paths{f.paths.xml_path,{}};preloader_l::xml_reader_bridge::Backend xml;preloader_l::achievement_storage::HostCrtNumbers numbers;
    preloader_l::timestamp::ClassicIso parser;StorageEffects effects(parser);io::NativeFactory files;
    preloader_l::achievement_storage::Storage storage(paths,xml,numbers,effects,files);storage.update_01eb00(restored);
    const auto& a=restored.achievements.at("a");require(a.field04==0 && a.field08==1 && a.field70==946684800 && a.timestamp=="2000-01-01T00:00:00" && a.display_name=="A&\"<","runtime write to actual XML reader/storage/parser/calendar roundtrip");
    require(a.field00==2 && a.steam_id=="100","roundtrip import preserves configured total and Steam ID");
}
}
int main(int argc,char** argv){try{if(argc!=2)throw std::invalid_argument("owned fixture directory required");const std::filesystem::path root(argv[1]);bridge_and_partial(root);completion_and_edges(root);failures_and_empty_paths(root);native_locked_ini_grant_retry(root);roundtrip(root);std::cout<<"PASS: "<<checks<<" achievement runtime integration checks; own fixtures and controlled clock/locks/events only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
