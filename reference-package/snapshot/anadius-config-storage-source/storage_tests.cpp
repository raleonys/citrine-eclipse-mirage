#include "storage_test_fixture.hpp"
#include <csignal>
#include <cstdlib>
#include <process.h>
#include <array>
#include <cstring>

namespace {
using namespace b8_test;
std::uint32_t big_endian(std::span<const std::uint8_t> bytes,std::size_t offset){
    return (std::uint32_t(bytes[offset])<<24)|(std::uint32_t(bytes[offset+1])<<16)|(std::uint32_t(bytes[offset+2])<<8)|bytes[offset+3];
}
std::uint32_t crc(std::span<const std::uint8_t> bytes){
    std::uint32_t value=0xffffffff;for(auto byte:bytes){value^=byte;for(unsigned bit=0;bit<8;++bit)value=(value>>1)^((value&1)?0xedb88320:0);}return value^0xffffffff;
}
void embedded_png_integrity(){
    using E=configuration::EmbeddedPng;
    for(const auto [identity,size]:std::array<std::pair<E,std::size_t>,2>{{{E::default_user_1800809d0,7488},{E::default_avatar_180082710,3882}}}){
        const auto bytes=cs::embedded_png(identity);constexpr std::array<std::uint8_t,8> signature{137,80,78,71,13,10,26,10};
        require(bytes.size()==size&&std::equal(signature.begin(),signature.end(),bytes.begin()),"retained PNG extent and signature match independent evidence");
        std::size_t cursor=8;std::vector<std::string> chunks;
        while(cursor+12<=bytes.size()){
            const auto length=big_endian(bytes,cursor);require(length<=bytes.size()-cursor-12,"PNG chunk payload remains inside exact retained extent");
            chunks.emplace_back(reinterpret_cast<const char*>(bytes.data()+cursor+4),4);
            require(crc(bytes.subspan(cursor+4,length+4))==big_endian(bytes,cursor+8+length),"retained PNG chunk CRC validates immutable bytes");
            cursor+=12+length;
        }
        require(cursor==bytes.size()&&chunks==std::vector<std::string>({"IHDR","IDAT","IEND"}),"PNG consists of complete IHDR IDAT IEND without invented trailing bytes");
        require(big_endian(bytes,16)!=0&&big_endian(bytes,20)!=0,"retained PNG has nonzero dimensions");
    }
    rejects<std::invalid_argument>([]{cs::embedded_png(static_cast<E>(99));},"unknown embedded asset identity is rejected");
}
void png_source_io(){
    PersistenceFixture f;require(f.files->reads.empty()&&f.files->writes.empty()&&f.sdk.creates.empty(),"persistence construction performs no file or directory work");
    const Path user=u"own-user.png",avatar=u"own-avatar.png";
    f.policy->write_embedded_png_03c020(user,configuration::EmbeddedPng::default_user_1800809d0);
    f.policy->write_embedded_png_03c020(avatar,configuration::EmbeddedPng::default_avatar_180082710);
    const auto a=cs::embedded_png(configuration::EmbeddedPng::default_user_1800809d0),b=cs::embedded_png(configuration::EmbeddedPng::default_avatar_180082710);
    require(f.files->outputs.at(user)==std::string(reinterpret_cast<const char*>(a.data()),a.size())&&f.files->outputs.at(avatar)==std::string(reinterpret_cast<const char*>(b.data()),b.size()),"both PNG routes write every exact retained byte through source file body");
    require(f.files->events==std::vector<std::string>({"open","write","close","open","write","close"})&&f.lower->pngs==0,"embedded output preserves explicit open write close and bypasses lower PNG policy");
    f.files->events.clear();f.files->outputs.clear();f.files->bad_output=true;
    f.policy->write_embedded_png_03c020(user,configuration::EmbeddedPng::default_user_1800809d0);
    require(f.files->outputs.empty()&&f.files->events==std::vector<std::string>{"open"},"initial output failure remains source false branch with ignored caller result");
    f.files->bad_output=false;f.files->throw_write=true;rejects<std::runtime_error>([&]{f.policy->write_embedded_png_03c020(user,configuration::EmbeddedPng::default_user_1800809d0);},"write exception propagates from selected file provider");
    f.files->throw_write=false;f.files->throw_close=true;rejects<std::runtime_error>([&]{f.policy->write_embedded_png_03c020(user,configuration::EmbeddedPng::default_user_1800809d0);},"close exception propagates after exact PNG bytes were written");
    f.files->throw_close=false;f.files->null_output=true;rejects<std::invalid_argument>([&]{f.policy->write_embedded_png_03c020(user,configuration::EmbeddedPng::default_user_1800809d0);},"null factory result stays an integration failure");
    require(f.sdk.creates.empty(),"PNG method does not invent directory creation before original file route");
}
void achievement_routes(){
    PersistenceFixture f({u"retained.xml",u"retained.ini"});configuration::State state{};state.user_directory=u"OWN";
    f.files->inputs[u"retained.xml"]=R"(<r><a Id="known" Count="3" Progress="2" Grant="1970-01-01T00:00:00" Name="file-name"/><b Id="new" Count="4"/></r>)";
    state.achievements["known"]={5,8,9,"old-time","configured-name","steam-known",77};
    f.policy->update_achievement_storage_01eb00(state);
    require(f.files->reads==std::vector<Path>{u"retained.xml"}&&f.policy->paths().xml_path==u"retained.xml"&&f.policy->paths().ini_path==u"retained.ini","empty set and app preserve explicitly supplied XML and INI path cells");
    require(state.achievements.at("known").field04==3&&state.achievements.at("known").field08==2&&state.achievements.at("known").display_name=="configured-name"&&state.achievements.at("new").field04==4,"actual XML backend and Storage update existing and new achievement rows");
    require(f.parser->seen.empty()&&f.stats->queried.empty()&&f.files->writes.empty(),"empty Steam app does not invent timestamp status or INI work");
    state.steam_app_id="42";state.achievements_set="fixture";
    state.achievements.erase("new");
    f.files->inputs[u"OWN\\achievement-fixture.xml"]=R"(<r><a Id="known" Count="3" Progress="2" Grant="1970-01-01T00:00:00"/></r>)";
    f.policy->update_achievement_storage_01eb00(state);
    const Path ini=u"OWN\\achievement_watcher\\42\\achievements.ini";
    require(f.policy->paths().xml_path==u"OWN\\achievement-fixture.xml"&&f.policy->paths().ini_path==ini,"actual Storage computes both path cells from selected source conversions");
    require(state.achievements.at("known").field70==0&&f.parser->seen==std::vector<std::string>{"1970-01-01T00:00:00"},"owned classic parser and recovered calendar convert epoch without machine timezone");
    require(f.files->outputs.at(ini)=="[steam-known]\r\nAchieved=3\r\nCurProgress=2\r\nMaxProgress=5\r\nUnlockTime=0\r\n\r\n","actual Storage writer preserves exact CRLF INI representation");
    require(!f.sdk.creates.empty()&&f.lower->creates==0&&f.lower->persistence==0,"INI parent directory uses selected filesystem and bypasses unresolved lower methods");
    f.stats->value={0,FILE_ATTRIBUTE_NORMAL,{}};const auto writes=f.files->writes.size();f.policy->update_achievement_storage_01eb00(state);
    require(f.files->writes.size()==writes,"existing regular INI kind2 bypasses rewrite as recovered source requires");
    f.stats->value={ERROR_ACCESS_DENIED,0,{}};bool denied=false;try{f.policy->update_achievement_storage_01eb00(state);}catch(const std::filesystem::filesystem_error& e){denied=e.code().value()==ERROR_ACCESS_DENIED&&e.path1()==std::filesystem::path(ini);}
    require(denied&&f.files->writes.size()==writes,"selected status error carries native code and computed path before any write");
    f.stats->value={0,FILE_ATTRIBUTE_REPARSE_POINT,{}};rejects<status_recovered::OutOfEvidenceStatus>([&]{f.policy->update_achievement_storage_01eb00(state);},"missing reparse provenance remains unsupported rather than guessed tag");
}
void persistence_failures_and_lifetimes(){
    PersistenceFixture f;configuration::State state{};state.user_directory=u"OWN";state.steam_app_id="42";
    state.achievements["a"]={1,1,0,"1970-01-01T00:00:01",{},"a",88};state.achievements["b"]={1,0,0,"FAIL",{},"b",99};f.parser->fail="FAIL";
    rejects<std::runtime_error>([&]{f.policy->update_achievement_storage_01eb00(state);},"timestamp policy failure propagates from same owned Storage");
    require(state.achievements.at("a").field70==1&&state.achievements.at("b").field70==99&&!f.policy->paths().ini_path.empty()&&f.stats->queried.empty(),"timestamp failure retains earlier source updates and path prefix without later status work");
    auto lower=std::make_shared<LowerState>();auto fs=std::make_shared<cs::Filesystem>(fs_api());
    rejects<std::invalid_argument>([&]{cs::ConfigurationPersistence p({}, {},std::make_unique<Lower>(lower));},"missing shared filesystem owner rejected");
    rejects<std::invalid_argument>([&]{cs::ConfigurationPersistence p(fs,{},{});},"missing lower policy owner rejected");
    for(unsigned missing=0;missing<7;++missing){
        cs::PersistenceResources resources;switch(missing){case 0:resources.files.reset();break;case 1:resources.xml.reset();break;case 2:resources.numbers.reset();break;case 3:resources.parser.reset();break;case 4:resources.stats.reset();break;case 5:resources.encoding.reset();break;default:resources.errors.reset();}
        rejects<std::invalid_argument>([&]{cs::ConfigurationPersistence p(fs,{},std::make_unique<Lower>(lower),std::move(resources));},"incomplete owned persistence resource graph rejected before I/O");
    }
    f.policy->log_config_path(u"anadius.cfg");require(f.lower->events==std::vector<std::string>{"log-base"},"unselected configuration log policy remains explicit");
    rejects<std::logic_error>([&]{f.policy->apply_license_key_02a830("fixture");},"unselected license subsystem is never fabricated");
    require(!f.policy->set_environment_variable("fixture","value")&&f.lower->environment.size()==1,"unselected environment result and operands forward unchanged");
}
void filesystem_enumeration(){
    FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());
    const auto entries=fs.directory_entries(u"C:\\own");
    require(entries==std::vector<Path>({u"C:\\own\\z",u"C:\\own\\a"})&&sdk.enum_operands,"directory enumeration skips only dot entries and keeps SDK traversal order");
    require(sdk.enumerations==std::vector<Path>{u"C:\\own\\*"}&&sdk.nexts==4&&sdk.find_closes==1,"enumeration wildcard source join and owned handle lifetime are exact");
    const auto calls=sdk.enumerations.size();rejects<std::system_error>([&]{fs.directory_entries({});},"empty enumeration path rejects before SDK");
    rejects<std::system_error>([&]{fs.directory_entries(Path(u"C:\\x\0other",10));},"embedded NUL enumeration path rejects before SDK");
    require(sdk.enumerations.size()==calls,"invalid enumeration paths never select a different SDK path by truncation");
    sdk.enum_error=ERROR_NO_MORE_FILES;require(fs.directory_entries(u"C:\\empty").empty()&&sdk.find_closes==1,"initial end-of-enumeration returns empty without closing invalid handle");
    sdk.enum_error=ERROR_ACCESS_DENIED;rejects<std::system_error>([&]{fs.directory_entries(u"C:\\denied");},"initial enumeration failure retains actual failure boundary");
    sdk.enum_error=0;sdk.next_error=ERROR_ACCESS_DENIED;rejects<std::system_error>([&]{fs.directory_entries(u"C:\\partial");},"later enumeration failure does not return partial success");
    require(sdk.find_closes==2,"later enumeration failure closes acquired find handle exactly once");fs_sdk=nullptr;
}
void filesystem_create_policies(){
    FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());
    fs.create_directories(u"C:\\base\\one\\two");
    require(sdk.creates==std::vector<Path>({u"C:\\base",u"C:\\base\\one",u"C:\\base\\one\\two"}),"recursive directory creation preserves retained prefix order");
    sdk.creates.clear();fs.create_directories(u"C:\\");require(sdk.creates.empty(),"existing drive root requires no invented create operation");
    rejects<std::system_error>([&]{fs.create_directories({});},"empty create path remains original path-not-found failure");
    sdk.create_errors={ERROR_ACCESS_DENIED,ERROR_PATH_NOT_FOUND,0};fs.create_directories(u"C:\\base\\one\\two");
    require(sdk.create_errors.empty(),"successful final prefix preserves original suppression of earlier create error");
    sdk.create_errors={ERROR_ACCESS_DENIED,ERROR_PATH_NOT_FOUND,ERROR_PATH_NOT_FOUND};bool denied=false;try{fs.create_directories(u"C:\\base\\one\\two");}catch(const std::system_error& e){denied=e.code().value()==ERROR_ACCESS_DENIED;}
    require(denied,"failed final prefix reports earlier nonignorable access-denied rather than generic missing parent");
    sdk.create_errors={ERROR_ALREADY_EXISTS};sdk.attrs=FILE_ATTRIBUTE_DIRECTORY;fs.create_directories(u"C:\\base");
    require(sdk.attribute_calls==1,"already-exists directory is validated by existing source status policy");
    sdk.create_errors={ERROR_ALREADY_EXISTS};sdk.attrs=FILE_ATTRIBUTE_NORMAL;denied=false;try{fs.create_directories(u"C:\\base");}catch(const std::system_error& e){denied=e.code().value()==ERROR_ALREADY_EXISTS;}
    require(denied,"already-exists regular file is not accepted as a directory");
    sdk.create_errors={ERROR_ALREADY_EXISTS};sdk.attrs=FILE_ATTRIBUTE_DIRECTORY;sdk.attribute_error=ERROR_SHARING_VIOLATION;
    fs.create_directories(u"C:\\base");require(sdk.firsts==std::vector<Path>{u"C:\\base"}&&sdk.find_closes==1,"already-exists sharing-violation status uses recovered find fallback and consumes owned handle");
    auto incomplete=fs_api();incomplete.find_next=nullptr;rejects<std::invalid_argument>([&]{cs::Filesystem invalid(incomplete);},"partial filesystem SDK table is rejected before side effects");fs_sdk=nullptr;
}
DWORD disposition_value(const FsSdk& sdk,std::size_t index){DWORD value=0;std::memcpy(&value,sdk.set_values[index].data(),sizeof(value));return value;}
void filesystem_remove_policies(){
    {
        FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());fs.remove(u"C:\\own\\file");
        require(sdk.accesses==std::vector<DWORD>{0x10180}&&sdk.shares==std::vector<DWORD>{7}&&sdk.dispositions==std::vector<DWORD>{OPEN_EXISTING}&&sdk.open_flags==std::vector<DWORD>{0x02200000},"remove opens exact delete/read/write attributes with all sharing and reparse-point boundary");
        require(sdk.set_classes==std::vector<FILE_INFO_BY_HANDLE_CLASS>{static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x15)}&&sdk.set_sizes==std::vector<DWORD>{4}&&disposition_value(sdk,0)==0x13&&sdk.handle_closes==1,"normal remove applies exact extended disposition flags and closes once");
    }
    {
        FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());sdk.open_errors={ERROR_FILE_NOT_FOUND};fs.remove(u"C:\\own\\missing");
        require(sdk.handle_closes==0&&sdk.set_classes.empty(),"first-open missing file is original normal no-op without invalid handle close");
        sdk.open_errors={ERROR_ACCESS_DENIED,ERROR_FILE_NOT_FOUND};bool missing=false;try{fs.remove(u"C:\\own\\retry-missing");}catch(const std::system_error& e){missing=e.code().value()==ERROR_FILE_NOT_FOUND;}
        require(missing&&sdk.accesses==std::vector<DWORD>({0x10180,0x10180,0x10000})&&sdk.handle_closes==0,"access-denied open retries reduced rights but does not normalize second missing failure");
    }
    {
        FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());sdk.set_errors={ERROR_INVALID_PARAMETER,ERROR_NOT_SUPPORTED,0};fs.remove(u"C:\\own\\legacy");
        require(sdk.set_classes==std::vector<FILE_INFO_BY_HANDLE_CLASS>({static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x15),static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x15),FileDispositionInfo})&&sdk.set_sizes==std::vector<DWORD>({4,4,1}),"unsupported extended deletion follows flags3 then exact one-byte legacy disposition");
        require(disposition_value(sdk,1)==3&&sdk.set_values[2][0]==1&&sdk.handle_closes==1,"legacy deletion consumes one SDK handle after exact operands");
    }
    {
        FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());sdk.attrs=FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_ARCHIVE;
        sdk.set_errors={ERROR_INVALID_PARAMETER,ERROR_ACCESS_DENIED,0,ERROR_ACCESS_DENIED,0};bool denied=false;try{fs.remove(u"C:\\own\\readonly");}catch(const std::system_error& e){denied=e.code().value()==ERROR_ACCESS_DENIED;}
        require(denied&&sdk.get_classes==std::vector<FILE_INFO_BY_HANDLE_CLASS>{FileBasicInfo}&&sdk.set_classes.size()==5&&sdk.handle_closes==1,"repeated readonly deletion denial queries metadata restores attributes and preserves failure");
        FILE_BASIC_INFO clear{},restore{};std::memcpy(&clear,sdk.set_values[2].data(),sizeof(clear));std::memcpy(&restore,sdk.set_values[4].data(),sizeof(restore));
        require(clear.FileAttributes==FILE_ATTRIBUTE_ARCHIVE&&restore.FileAttributes==(FILE_ATTRIBUTE_ARCHIVE|FILE_ATTRIBUTE_READONLY),"readonly fallback clears only readonly then restores exact attributes on repeated access-denied");
    }
    {
        FsSdk sdk;fs_sdk=&sdk;cs::Filesystem fs(fs_api());sdk.set_errors={ERROR_ACCESS_DENIED};rejects<std::system_error>([&]{fs.remove(u"C:\\own\\denied");},"non-fallback disposition failure propagates actual error");
        require(sdk.set_classes.size()==1&&sdk.handle_closes==1,"non-fallback removal error still closes acquired handle exactly once");
        auto state=std::make_shared<LowerState>();auto shared=std::make_shared<cs::Filesystem>(fs_api());cs::ConfigurationFilesystem policy(shared,std::make_unique<Lower>(state));
        sdk.set_errors.clear();policy.create_directories_018e00(u"C:\\own");policy.directory_entries(u"C:\\own");policy.remove_04948c(u"C:\\own\\file");
        require(state->creates==0&&state->enumerations==0&&state->removes==0,"three configuration filesystem routes bypass unselected lower filesystem methods");
        policy.write_embedded_png_03c020(u"forward",configuration::EmbeddedPng::default_user_1800809d0);configuration::State cfg{};policy.update_achievement_storage_01eb00(cfg);
        require(state->pngs==1&&state->persistence==1,"filesystem wrapper leaves two persistence methods to their explicit owner");
        rejects<std::invalid_argument>([&]{cs::ConfigurationFilesystem invalid({},std::make_unique<Lower>(state));},"filesystem configuration requires shared native policy lifetime");
        rejects<std::invalid_argument>([&]{cs::ConfigurationFilesystem invalid(shared,{});},"filesystem configuration requires remaining lower owner");
    }
    fs_sdk=nullptr;
}
}
namespace {
int fatal_exit=80;
void expected_abort(int){std::_Exit(fatal_exit);}
HANDLE WINAPI unknown_acquisition(LPCWSTR,FINDEX_INFO_LEVELS,LPVOID,FINDEX_SEARCH_OPS,LPVOID,DWORD){
    throw std::runtime_error("unknown synthetic handle acquisition");
}
int terminal_case(std::string_view mode){
    using namespace b8_test;
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::signal(SIGABRT,expected_abort);FsSdk sdk;fs_sdk=&sdk;auto api=fs_api();
    if(mode=="--find-close-failure"){fatal_exit=81;sdk.find_close_ok=false;cs::Filesystem fs(api);fs.directory_entries(u"C:\\own");}
    else if(mode=="--status-close-failure"){fatal_exit=82;sdk.find_close_ok=false;sdk.create_errors={ERROR_ALREADY_EXISTS};sdk.attribute_error=ERROR_SHARING_VIOLATION;cs::Filesystem fs(api);fs.create_directories(u"C:\\own");}
    else if(mode=="--file-close-failure"){fatal_exit=83;sdk.close_ok=false;cs::Filesystem fs(api);fs.remove(u"C:\\own\\file");}
    else if(mode=="--unknown-acquisition"){fatal_exit=84;api.find_first_ex=unknown_acquisition;cs::Filesystem fs(api);fs.directory_entries(u"C:\\own");}
    return 79;
}
}
int main(int argc,char** argv){if(argc==2&&std::string_view(argv[1]).starts_with("--"))return terminal_case(argv[1]);try{
    embedded_png_integrity();png_source_io();achievement_routes();persistence_failures_and_lifetimes();filesystem_enumeration();filesystem_create_policies();filesystem_remove_policies();
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--find-close-failure",static_cast<const char*>(nullptr))==81,"own child makes failed enumeration handle close terminal");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--status-close-failure",static_cast<const char*>(nullptr))==82,"own child verifies stronger selected status fallback close policy");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--file-close-failure",static_cast<const char*>(nullptr))==83,"own child makes failed owned delete handle close terminal");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--unknown-acquisition",static_cast<const char*>(nullptr))==84,"own child makes unknown acquisition outcome terminal");
    std::cout<<"PASS: "<<b8_test::checks<<" configuration-storage core checks; exact PNG assets, owned persistence graph, source XML/INI/error prefixes and injected filesystem boundaries; no original execution\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
