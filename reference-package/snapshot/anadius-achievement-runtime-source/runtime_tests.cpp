#include "runtime.hpp"
#include "../anadius-timestamp-source/timestamp.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <stdexcept>

namespace ar=anadius::achievement_runtime;
namespace content=anadius::content_handlers_recovered;
namespace cfg=anadius::configuration;
namespace io=anadius::file_recovered;
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
    void push_locked_034660(std::string value)override{require(mutex.held && achievement.held && initialized,"push event under both locks");entries.push_back(std::move(value));}
};
struct Clock final:ar::Clock {
    Mutex& mutex;unsigned calls=0;bool absent=false;
    ar::BrokenDown fields{0,0,0,1,0,100,6,0,0};
    explicit Clock(Mutex& m):mutex(m){}
    std::int64_t time_05eeb8()override{require(mutex.held,"clock invoked under achievement lock");++calls;return 123;}
    std::optional<ar::BrokenDown> gmtime_05fffc(std::int64_t raw)override{require(raw==123,"clock forwarded to gmtime");if(absent)return std::nullopt;return fields;}
};
struct Directories final:anadius::achievement_writer::Effects {
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
    cfg::State state{};anadius::achievement_storage::Paths paths;
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
    require(f.trace==std::vector<std::string>{"open:xml","write:xml","close:xml","mkdir","open:ini","write:ini","close:ini"},"XML write precedes INI parent/write under achievement lock");
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
    {Fixture f(root/"false-xml");f.files.native_io=false;f.dirs.native=false;f.files.xml_false=true;(void)f.grant("a");require(f.trace==std::vector<std::string>{"open:xml","mkdir","open:ini","write:ini","close:ini"},"false XML initial-state BOOL ignored and INI continues");require(!f.mutex.held,"false XML return unlocks outer mutex");}
    {Fixture f(root/"throw-xml");f.files.native_io=false;f.dirs.native=false;f.files.xml_throw=true;rejects<std::runtime_error>([&]{(void)f.grant("a");},"XML exception propagates");require(f.trace==std::vector<std::string>{"open:xml","write:xml"} && f.dirs.paths.empty() && !f.mutex.held,"XML exception skips INI and unlocks outer mutex");require(f.state.achievements.at("a").field08==1 && f.state.achievements.at("a").field70==946684800,"write exception does not roll back achievement changes");}
    {Fixture f(root/"empty-paths");f.paths.xml_path.clear();f.paths.ini_path.clear();(void)f.grant("a");require(f.trace.empty() && f.clock.calls==1,"empty persistence paths skip both files without skipping grant");}
}
struct StorageEffects final:anadius::timestamp_recovered::StorageEffects {
    explicit StorageEffects(anadius::timestamp_recovered::Parser& parser):anadius::timestamp_recovered::StorageEffects(parser){}
    cfg::Path filesystem_widen_006d70(std::string_view value)override{return {value.begin(),value.end()};}
    anadius::status_recovered::Status status_01ea20(const cfg::Path&,std::uint32_t flags)override{require(flags==3,"roundtrip storage uses flags3");return {2,0,0};}
    [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const cfg::Path&)override{unused();}
    void create_directories_018e00(const cfg::Path&)override{unused();}
};
void roundtrip(const std::filesystem::path& root){
    Fixture f(root/"roundtrip");f.state.achievements.erase("negative");f.state.achievements.erase("skip");(void)f.grant("a");
    cfg::State restored{};restored.user_directory=f.state.user_directory;restored.steam_app_id="42";
    restored.achievements.emplace("a",cfg::Achievement{2,0,0,"old","","100",999});
    anadius::achievement_storage::Paths paths{f.paths.xml_path,{}};anadius::xml_reader_bridge::Backend xml;anadius::achievement_storage::HostCrtNumbers numbers;
    anadius::timestamp_recovered::ClassicIso parser;StorageEffects effects(parser);io::NativeFactory files;
    anadius::achievement_storage::Storage storage(paths,xml,numbers,effects,files);storage.update_01eb00(restored);
    const auto& a=restored.achievements.at("a");require(a.field04==0 && a.field08==1 && a.field70==946684800 && a.timestamp=="2000-01-01T00:00:00" && a.display_name=="A&\"<","runtime write to actual XML reader/storage/parser/calendar roundtrip");
    require(a.field00==2 && a.steam_id=="100","roundtrip import preserves configured total and Steam ID");
}
}
int main(int argc,char** argv){try{if(argc!=2)throw std::invalid_argument("owned fixture directory required");const std::filesystem::path root(argv[1]);bridge_and_partial(root);completion_and_edges(root);failures_and_empty_paths(root);roundtrip(root);std::cout<<"PASS: "<<checks<<" achievement runtime integration checks; own fixtures and controlled clock/locks/events only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
