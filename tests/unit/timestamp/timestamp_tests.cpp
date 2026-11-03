// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/timestamp.hpp"
#include "preloader_l/xml/reader.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

using namespace preloader_l;
namespace {
unsigned checks=0;
void require(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
template<class E,class F>void rejects(F f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
bool zeroed(const calendar::BrokenDown& t){return t.second==0 && t.minute==0 && t.hour==0 && t.day==0 && t.month==0 && t.year==0 && t.weekday==0 && t.yearday==0 && t.daylight==0;}
struct Spy final:timestamp::Parser {
    std::uint32_t state=0;bool fail=false;unsigned calls=0;std::string expected;
    std::uint32_t extract_03d310(std::string_view raw,std::string_view pattern,calendar::BrokenDown& fields)override{
        ++calls;require(zeroed(fields),"wrapper initializes all nine tm fields");
        require(pattern=="%Y-%m-%dT%H:%M:%S","exact original time format");
        require(raw==expected,"input bytes and explicit length unchanged");
        if(fail)throw std::runtime_error("synthetic parser failure");
        fields={0,0,0,1,0,100,99,-77,1};return state;
    }
};
void wrapper_contract() {
    for(const auto state:{0U,1U,2U,4U,3U,5U,6U,7U,16U,0xffffffffU}){
        Spy parser;parser.state=state;parser.expected=std::string("  raw\0tail  ",12);
        const auto result=timestamp::timestamp_to_epoch_041e10(parser.expected,parser);
        require(result==((state&6U)?0:946684800),"native bad/fail mask6, EOF and unrelated bits allowed");
        require(parser.calls==1,"one parser extraction per input");
    }
    Spy parser;parser.fail=true;parser.expected="throw";
    rejects<std::runtime_error>([&]{(void)timestamp::timestamp_to_epoch_041e10("throw",parser);},"backend exception is not changed into zero");
}
void selected_host_backend() {
    timestamp::HostGetTime host;
    for(const auto& [value,expected]:std::vector<std::pair<std::string,std::int64_t>>{
        {"1970-01-01T00:00:00",0},{"1970-01-01T00:00:01",1},
        {"1969-12-31T23:59:59",-1},{"1900-01-01T00:00:00",-2208988800LL},
        {"2000-01-01T00:00:00",946684800},{"2000-02-29T00:00:00",951782400},
        {"2000-03-01T00:00:00",951868800},{"2000-01-01T12:34:56",946730096},
        {"  2000-01-01T00:00:00",946684800},
        {"2000-01-01T00:00:00Z",946684800},
        {"2000-01-01T00:00:00+12:30ignored",946684800}})
        require(timestamp::timestamp_to_epoch_041e10(value,host)==expected,"chosen host parser normal fixture plus recovered calendar");
    for(const auto* value:{"","not a timestamp","0000-00-00T00:00:00","2000-13-01T00:00:00","2000-01-01T25:00:00","2000-01-01T00:"})
        require(timestamp::timestamp_to_epoch_041e10(value,host)==0,"chosen host invalid fixture returns zero through fail mask");
    const std::string after_date("2000-01-01T00:00:00\0unparsed",27);
    require(timestamp::timestamp_to_epoch_041e10(after_date,host)==946684800,"format completion does not require full input consumption");
    const std::string inside_date("2000-01-\0T00:00:00",18);
    require(timestamp::timestamp_to_epoch_041e10(inside_date,host)==0,"embedded NUL in required field is not silently removed");
    for(const auto* value:{"2000-01-01T00:00:00","2000-01-01T00:00:00tail","invalid",""}) {
        calendar::BrokenDown fields{};
        const auto native_bits=host.extract_03d310(value,timestamp::format,fields);
        std::istringstream independent(value);std::tm tm{};independent>>std::get_time(&tm,"%Y-%m-%dT%H:%M:%S");
        require(((native_bits&1U)!=0)==((independent.rdstate()&std::ios::eofbit)!=0),"host EOF translated into native bit1");
        require(((native_bits&2U)!=0)==((independent.rdstate()&std::ios::failbit)!=0),"host fail translated into native bit2");
        require(((native_bits&4U)!=0)==((independent.rdstate()&std::ios::badbit)!=0),"host bad translated into native bit4");
    }
    // These tests verify this selected standard-library backend. They do not
    // claim equivalent malformed input/locale/time_get behavior for MSVC.
}
bool same_fields(const calendar::BrokenDown& a,const calendar::BrokenDown& b){
    return a.second==b.second && a.minute==b.minute && a.hour==b.hour &&
           a.day==b.day && a.month==b.month && a.year==b.year &&
           a.weekday==b.weekday && a.yearday==b.yearday && a.daylight==b.daylight;
}
void classic_native_branch() {
    timestamp::ClassicIso parser;
    struct Case {std::string input;std::uint32_t state;calendar::BrokenDown fields;};
    // Expected fields/states come from the retained numeric/runner branches,
    // not a host parser, and include each earlier-field EOF acceptance.
    for(const auto& c:std::vector<Case>{
        {"",3,{}},{" \t\r\n\v\f",3,{}},{"x",2,{}},{"+",3,{}},
        {"2024",1,{0,0,0,0,0,124,0,0,0}},
        {"2024-",3,{0,0,0,0,0,124,0,0,0}},
        {"2024-02",1,{0,0,0,0,1,124,0,0,0}},
        {"2024-02-",3,{0,0,0,0,1,124,0,0,0}},
        {"2024-02-03",1,{0,0,0,3,1,124,0,0,0}},
        {"2024-02-03T",3,{0,0,0,3,1,124,0,0,0}},
        {"2024-02-03T04",1,{0,0,4,3,1,124,0,0,0}},
        {"2024-02-03T04:",3,{0,0,4,3,1,124,0,0,0}},
        {"2024-02-03T04:05",1,{0,5,4,3,1,124,0,0,0}},
        {"2024-02-03T04:05:",3,{0,5,4,3,1,124,0,0,0}},
        {"2024-02-03T04:05:06",1,{6,5,4,3,1,124,0,0,0}},
        {"2024-02-31t04:05:60tail",0,{60,5,4,31,1,124,0,0,0}},
        {"+2024-2-3t4:5:6",1,{6,5,4,3,1,124,0,0,0}},
        {"\t\n\v\f\r 2024-+2-+3t+4:+5:+6",1,{6,5,4,3,1,124,0,0,0}},
        {"2024-02-03T  :05:06",1,{6,5,0,3,1,124,0,0,0}},
        {"2024-02-03T +4:05:06",1,{6,5,4,3,1,124,0,0,0}},
        {"2024-02-03T04:  :  ",1,{0,0,4,3,1,124,0,0,0}},
        {"2024-02-03T04:05: + ",0,{0,5,4,3,1,124,0,0,0}},
        {"2024- 1- 2T-0:-0:-0",1,{0,0,0,2,0,124,0,0,0}},
        {"-0-1-1T0:0:0",1,{0,0,0,1,0,-1900,0,0,0}},
        {"9999-12-31T23:59:60",1,{60,59,23,31,11,8099,0,0,0}},
        {"2024-13-01T00:00:00",2,{0,0,0,0,0,124,0,0,0}},
        {"2024-00",3,{0,0,0,0,0,124,0,0,0}},
        {"2024-  1-01T00:00:00",2,{0,0,0,0,0,124,0,0,0}},
        {"20240-02-03T04:05:06",2,{0,0,0,0,0,124,0,0,0}},
        {"2024-02-03T +04:05:06",2,{0,0,0,3,1,124,0,0,0}},
        {"2024-02-03T04:05:06123",0,{6,5,4,3,1,124,0,0,0}},
        {std::string("2024-02-03T04:05:06\0tail",24),0,{6,5,4,3,1,124,0,0,0}},
        {std::string("2024\0-02-03T04:05:06",20),2,{0,0,0,0,0,124,0,0,0}},
        {"2024/02-03T04:05:06",2,{0,0,0,0,0,124,0,0,0}}
    }){
        calendar::BrokenDown fields{};
        const auto state=parser.extract_03d310(c.input,timestamp::format,fields);
        require(state==c.state,"classic branch native state fixture");
        require(same_fields(fields,c.fields),"classic branch native partial tm fixture");
    }
    for(const auto* text:{"-1","2024-0","2024-13","2024-1-0","2024-1-32",
                         "2024-1-1T24","2024-1-1T-1","2024-1-1T0:60",
                         "2024-1-1T0:-1","2024-1-1T0:0:61","2024-1-1T0:0:-1"}){
        calendar::BrokenDown fields{77,77,77,77,77,77,77,77,77};
        require((parser.extract_03d310(text,timestamp::format,fields)&2)!=0,"each numeric range rejects boundary outside domain");
        require(fields.weekday==77 && fields.yearday==77 && fields.daylight==77,"ISO directives leave unrelated fields untouched");
    }
    calendar::BrokenDown fields{77,77,77,77,77,77,77,77,77};
    require(parser.extract_03d310("2024-13",timestamp::format,fields)==3 && fields.month==77 && fields.year==124,"failed numeric destination unchanged after earlier successful year");
    rejects<std::invalid_argument>([&]{(void)parser.extract_03d310("2000","%Y",fields);},"unrecovered format explicit domain rejection");
    rejects<std::invalid_argument>([&]{(void)timestamp::timestamp_to_epoch_041e10(std::string("2000-01-01T00:00:00")+char(0x80),parser);},"nonASCII including trailing bytes explicit domain rejection");
    for(const auto& [text,epoch]:std::vector<std::pair<std::string,std::int64_t>>{
        {"1970-01-01T00:00:00",0},{"1969-12-31T23:59:59",-1},
        {"2000-02-29t0:0:0",951782400},{"2000-02-31T0:0:0",951955200},
        {"2000-01-01T00:00:60",946684860},{"2000-01-01T00:00:00Z",946684800}})
        require(timestamp::timestamp_to_epoch_041e10(text,parser)==epoch,"classic parser to real calendar golden epoch");
}
struct StorageEffects final:timestamp::StorageEffects {
    std::vector<configuration::Path> created,status_paths;
    explicit StorageEffects(timestamp::Parser& parser):timestamp::StorageEffects(parser){}
    configuration::Path filesystem_widen_006d70(std::string_view value)override{return {value.begin(),value.end()};}
    status::Status status_01ea20(const configuration::Path& path,std::uint32_t flags)override{
        require(flags==3,"status flags3 after actual timestamp conversions");status_paths.push_back(path);return {1,0,2};
    }
    [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const configuration::Path&)override{throw std::runtime_error("unexpected filesystem error");}
    void create_directories_018e00(const configuration::Path& path)override{created.push_back(path);}
};
void storage_native_fixture(const std::filesystem::path& directory) {
    std::filesystem::create_directories(directory);
    const auto xml=directory/"achievement-timestamp-fixture.xml";
    {
        std::ofstream out(xml,std::ios::binary|std::ios::trunc);
        out<<R"(<root><entry Id="d" Count="4" Grant="invalid"/><entry Id="b" Count="2" Grant="2000-01-01T00:00:00"/><entry Id="a" Count="1" Grant="1970-01-01T00:00:00"/><entry Id="c" Count="3" Grant="1969-12-31T23:59:59"/></root>)";
        require(static_cast<bool>(out),"own timestamp XML fixture written");
    }
    const auto ini_directory=directory/"achievement_watcher"/"42";
    std::filesystem::create_directories(ini_directory);
    configuration::State state{};state.user_directory=directory.u16string();state.achievements_set="timestamp-fixture";state.steam_app_id="42";
    achievement_storage::Paths paths;xml_reader_bridge::Backend reader;achievement_storage::HostCrtNumbers numbers;
    timestamp::ClassicIso parser;StorageEffects effects(parser);file::NativeFactory files;
    achievement_storage::Storage storage(paths,reader,numbers,effects,files);storage.update_01eb00(state);
    require(state.achievements.size()==4,"actual XML/storage imports four timestamp records");
    require(state.achievements.at("a").field70==0 && state.achievements.at("b").field70==946684800,"actual epoch helper reaches achievement field70");
    require(state.achievements.at("c").field70==0xffffffffffffffffULL && state.achievements.at("d").field70==0,"negative epoch low64 and failed parse zero stored");
    require(state.achievements.at("a").steam_id.empty(),"new XML IDs keep original empty Steam ID");
    require(effects.status_paths.size()==1 && effects.created.size()==1,"status and actual writer reached after conversions");
    std::ifstream input(ini_directory/"achievements.ini",std::ios::binary);
    const std::string bytes{std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
    require(bytes=="[]\r\nAchieved=1\r\nUnlockTime=0\r\n\r\n"
                   "[]\r\nAchieved=2\r\nUnlockTime=946684800\r\n\r\n"
                   "[]\r\nAchieved=3\r\nUnlockTime=18446744073709551615\r\n\r\n"
                   "[]\r\nAchieved=4\r\nUnlockTime=0\r\n\r\n","actual recovered timestamp/storage/writer path exact INI bytes in map order");
}
}
int main(int argc,char** argv) {
    try{if(argc!=2)throw std::invalid_argument("requires owned fixture directory");wrapper_contract();selected_host_backend();classic_native_branch();storage_native_fixture(std::filesystem::path(argv[1]));std::cout<<"PASS: "<<checks<<" timestamp checks; classic ASCII native branch plus explicit host compatibility; own fixtures only\n";}
    catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
}
