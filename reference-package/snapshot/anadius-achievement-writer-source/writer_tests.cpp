#include "writer.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <stdexcept>

using namespace anadius;
namespace {
unsigned checks=0;
void require(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
using Events=std::vector<std::string>;
struct Output final:file_recovered::Output {
    Events& events;std::string& captured;std::uint32_t state;std::string fail;
    Output(Events& e,std::string& c,std::uint32_t s,std::string f):events(e),captured(c),state(s),fail(std::move(f)){}
    ~Output()override{events.emplace_back("destroy");}
    std::uint32_t initial_state()const override{events.emplace_back("state");return state;}
    void write_03be80(std::string_view bytes)override{events.emplace_back("write");if(fail=="write")throw std::runtime_error("synthetic write failure");captured.assign(bytes);}
    void close_03ae70()override{events.emplace_back("close");if(fail=="close")throw std::runtime_error("synthetic close failure");}
};
struct Files final:file_recovered::Factory {
    Events& events;std::string captured,fail;configuration::Path opened;std::uint32_t state=0;bool null_stream=false;
    explicit Files(Events& e):events(e){}
    std::unique_ptr<file_recovered::Input> open_input_039f70(const configuration::Path&,std::uint32_t,std::uint32_t)override{throw std::logic_error("writer may not read");}
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const configuration::Path& p,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20 && share==0x40,"existing binary output open flags");events.emplace_back("open");opened=p;
        if(null_stream)return {};
        return std::make_unique<Output>(events,captured,state,fail);
    }
};
struct Effects final:achievement_writer::Effects {
    Events& events;configuration::Path parent;bool fail=false;configuration::State* mutate=nullptr;
    explicit Effects(Events& e):events(e){}
    void create_directories_018e00(const configuration::Path& p)override{
        events.emplace_back("mkdir");parent=p;if(fail)throw std::runtime_error("synthetic directory failure");
        if(mutate)mutate->achievements.at("a").field04=7;
    }
};
configuration::Achievement record(std::int32_t total,std::int32_t count,std::int32_t progress,
                                  std::string steam_id,std::uint64_t unlock) {
    return {total,count,progress,"unused timestamp","unused name",std::move(steam_id),unlock};
}
void exact_bytes() {
    configuration::State state{};
    require(achievement_writer::serialize_ini(state).empty(),"empty map serializes empty bytes");
    state.achievements["z"]=record(1,1,777,"first-section",99);
    state.achievements["a"]=record(20,-3,-7,"last-section",1000);
    require(achievement_writer::serialize_ini(state)==
        "[last-section]\r\nAchieved=-3\r\nCurProgress=-7\r\nMaxProgress=20\r\nUnlockTime=1000\r\n\r\n"
        "[first-section]\r\nAchieved=1\r\nUnlockTime=99\r\n\r\n",
        "map-key order, steam section name, CRLF and conditional progress golden bytes");
    state.achievements.clear();state.achievements["id"]=record(std::numeric_limits<std::int32_t>::max(),
        std::numeric_limits<std::int32_t>::min(),std::numeric_limits<std::int32_t>::min(),"",
        std::numeric_limits<std::uint64_t>::max());
    require(achievement_writer::serialize_ini(state)==
        "[]\r\nAchieved=-2147483648\r\nCurProgress=-2147483648\r\nMaxProgress=2147483647\r\nUnlockTime=18446744073709551615\r\n\r\n",
        "signed32 minimum and unsigned64 maximum exact formatting");
    for(const auto total:{std::numeric_limits<std::int32_t>::min(),-1,0,1}) {
        state.achievements["id"]=record(total,0,-999,"id",0);
        require(achievement_writer::serialize_ini(state)=="[id]\r\nAchieved=0\r\nUnlockTime=0\r\n\r\n","signed total <=1 suppresses both progress keys");
    }
    state.achievements["id"]=record(2,2147483647,2147483647,"id",0);
    require(achievement_writer::serialize_ini(state)=="[id]\r\nAchieved=2147483647\r\nCurProgress=2147483647\r\nMaxProgress=2\r\nUnlockTime=0\r\n\r\n","total2 enables progress; count is numeric not boolean");
    const std::string section("x\0]\r\ny",6);
    state.achievements["id"]=record(1,-1,0,section,1);
    std::string expected="[";expected+=section;expected+="]\r\nAchieved=-1\r\nUnlockTime=1\r\n\r\n";
    require(achievement_writer::serialize_ini(state)==expected,"section explicit bytes including NUL/INI delimiters are unescaped");
    state.achievements.clear();state.achievements["map-a"]=record(0,0,0,"same",0);state.achievements["map-b"]=record(0,0,0,"same",0);
    require(achievement_writer::serialize_ini(state)=="[same]\r\nAchieved=0\r\nUnlockTime=0\r\n\r\n[same]\r\nAchieved=0\r\nUnlockTime=0\r\n\r\n","duplicate steam sections not deduplicated");
}
void writer_flow() {
    Events events;Files files(events);Effects effects(events);configuration::State state{};
    state.achievements["a"]=record(1,0,0,"id",0);
    achievement_writer::write_ini_01f9a0({},state,effects,files);
    require(events.empty(),"empty path skips mkdir and write regardless of records");
    achievement_writer::write_ini_01f9a0(u"C:/fixture/achievement_watcher/123/achievements.ini",state,effects,files);
    require(effects.parent==u"C:/fixture/achievement_watcher/123","actual recovered parent path result");
    require(files.opened==u"C:/fixture/achievement_watcher/123/achievements.ini","unchanged output path");
    require(files.captured=="[id]\r\nAchieved=0\r\nUnlockTime=0\r\n\r\n","actual existing file writer receives exact bytes");
    require(events==Events{"mkdir","open","state","write","close","destroy"},"directory before stream open/write/close");
    events.clear();effects.mutate=&state;
    achievement_writer::write_ini_01f9a0(u"fixture.ini",state,effects,files);effects.mutate=nullptr;
    require(effects.parent.empty() && files.captured=="[id]\r\nAchieved=7\r\nUnlockTime=0\r\n\r\n","serialization observes state after directory callback");
    events.clear();files.state=1;files.captured="untouched";
    achievement_writer::write_ini_01f9a0(u"fixture.ini",state,effects,files);
    require(events==Events{"mkdir","open","state","destroy"} && files.captured=="untouched","initial stream failure boolean ignored with no write or explicit close");
    files.state=0;events.clear();effects.fail=true;
    rejects<std::runtime_error>([&]{achievement_writer::write_ini_01f9a0(u"fixture.ini",state,effects,files);},"directory error propagates");
    require(events==Events{"mkdir"},"directory failure prevents file open");effects.fail=false;
    for(const auto* failure:{"write","close"}) {
        events.clear();files.fail=failure;
        rejects<std::runtime_error>([&]{achievement_writer::write_ini_01f9a0(u"fixture.ini",state,effects,files);},"stream exception propagates");
        require(events.back()=="destroy","stream destroyed during exception");
        require(std::count(events.begin(),events.end(),"close")==static_cast<int>(files.fail=="close"),"no synthetic close after failed write");
    }
    files.fail.clear();files.null_stream=true;
    rejects<std::invalid_argument>([&]{achievement_writer::write_ini_01f9a0(u"fixture.ini",state,effects,files);},"null output provider contract rejected");
    files.null_stream=false;state.achievements.clear();events.clear();
    achievement_writer::write_ini_01f9a0(u"fixture.ini",state,effects,files);
    require(files.captured.empty() && events==Events{"mkdir","open","state","write","close","destroy"},"nonempty path empty map still writes empty file");
}
void native_fixture(const std::filesystem::path& directory) {
    std::filesystem::create_directories(directory);
    const auto target=directory/"achievement-writer-generated.ini";
    Events events;Effects effects(events);file_recovered::NativeFactory files;configuration::State state{};
    state.achievements["owned-fixture"]=record(2,-1,-2,"own-fixture-id",123);
    achievement_writer::write_ini_01f9a0(target.u16string(),state,effects,files);
    std::ifstream input(target,std::ios::binary);
    const std::string actual((std::istreambuf_iterator<char>(input)),{});
    require(actual=="[own-fixture-id]\r\nAchieved=-1\r\nCurProgress=-2\r\nMaxProgress=2\r\nUnlockTime=123\r\n\r\n","own native fixture exact raw bytes");
    require(events==Events{"mkdir"},"only native file IO executed; directory effect controlled");
}
}
int main(int argc,char** argv) {
    try{if(argc!=2)throw std::invalid_argument("requires own fixture directory");exact_bytes();writer_flow();native_fixture(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" achievement INI writer checks; own fixture only\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
