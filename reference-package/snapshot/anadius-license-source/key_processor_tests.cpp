#include "key_processor.hpp"
#include "../anadius-token-source/constants.hpp"
#include "../anadius-bootstrap-policies-source/policies_test_fixture.hpp"
#include "../anadius-config-storage-source/storage_test_fixture.hpp"
#include "../anadius-configuration-factory-source/factory.hpp"
#include "../anadius-config-application-source/application.hpp"
#include <array>
#include <algorithm>
#include <filesystem>
#include <functional>
#include <iostream>
#include <sstream>

// Only owned synthetic strings, injected SDK outcomes and memory files. No
// original image, user configuration/token/license file or network is used.
namespace {
using namespace anadius;using Path=configuration::Path;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Fixture;
Fixture* current=nullptr;
BOOL WINAPI create_directory(LPCWSTR,LPSECURITY_ATTRIBUTES);
BOOL WINAPI attributes(LPCWSTR,GET_FILEEX_INFO_LEVELS,LPVOID);
struct Stats final:status_recovered::Stats {
    status_recovered::StatsResult result{ERROR_FILE_NOT_FOUND,0,{}};
    std::vector<Path> paths;std::vector<std::string>& timeline;std::function<void()> on_query;
    explicit Stats(std::vector<std::string>& events):timeline(events){}
    status_recovered::StatsResult query(const Path& path,std::uint32_t flags)override{
        require(flags==3,"key processor uses bounded source status flags3");
        timeline.emplace_back("status");paths.push_back(path);if(on_query)on_query();return result;
    }
};
struct Files final:file_recovered::Factory {
    b8_test::MemoryFiles inner;std::vector<std::string>& timeline;
    Files(std::shared_ptr<b8_test::MemoryFilesState> state,std::vector<std::string>& events):inner(std::move(state)),timeline(events){}
    std::unique_ptr<file_recovered::Input> open_input_039f70(const Path& path,std::uint32_t mode,std::uint32_t share)override{return inner.open_input_039f70(path,mode,share);}
    std::unique_ptr<file_recovered::Output> open_output_03a1a0(const Path& path,std::uint32_t mode,std::uint32_t share)override{
        require(mode==0x20&&share==0x40,"key output keeps binary mode20/share40");timeline.emplace_back("open");return inner.open_output_03a1a0(path,mode,share);
    }
};
struct Fixture {
    b8_test::FsSdk sdk;std::vector<std::string> timeline;
    b7_test::Encoding encoding;encoding_recovered::PortableErrors errors;Stats stats{timeline};
    std::shared_ptr<b8_test::MemoryFilesState> memory=std::make_shared<b8_test::MemoryFilesState>();
    Files files{memory,timeline};config_storage::Filesystem filesystem;
    Path base=u"MEM\\license";std::string content="CONTENT",user="USER";
    static config_storage::FilesystemApi api(){auto a=b8_test::fs_api();a.create_directory=create_directory;a.attributes_ex=attributes;return a;}
    Fixture():filesystem(api()){current=this;b8_test::fs_sdk=&sdk;}
    ~Fixture(){b8_test::fs_sdk=nullptr;current=nullptr;}
    license_recovered::KeyState state(){return {base,content,user};}
    license_recovered::KeyDependencies deps(){return {encoding,errors,stats,filesystem,files};}
    void apply(std::string key){license_recovered::apply_license_key_02a830(std::move(key),state(),deps());}
};
BOOL WINAPI create_directory(LPCWSTR path,LPSECURITY_ATTRIBUTES security){current->timeline.emplace_back("mkdir");return b8_test::fs_create(path,security);}
BOOL WINAPI attributes(LPCWSTR path,GET_FILEEX_INFO_LEVELS level,LPVOID data){current->timeline.emplace_back("attributes");return b8_test::fs_attributes(path,level,data);}
std::string expected_xml(std::string_view key,std::string_view content,std::string_view user){
    std::string text="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><License xmlns=\"http://ea.com/license\"><CipherKey>";
    text.append(key);text+="</CipherKey><MachineHash>1</MachineHash><ContentId>";text.append(content);
    text+="</ContentId><UserId>";text.append(user);
    text+="</UserId><GrantTime>2024-01-01T00:00:00Z</GrantTime><StartTime>2024-01-01T00:00:00Z</StartTime></License>";return text;
}
std::string decrypt_payload(const std::string& bytes){
    constexpr std::array<unsigned char,8> prefix{0x30,0x06,0x02,0x01,0x00,0x02,0x01,0x00};
    std::string header(65,'\0');std::copy(prefix.begin(),prefix.end(),header.begin());
    require(bytes.size()>65&&bytes.substr(0,65)==header,"output begins with exact eight header bytes followed by57 zeros");
    require((bytes.size()-65)%16==0,"ciphertext after offset41 has whole AES blocks");
    auto decrypted=bytes;crypto_recovered::Cipher cipher;
    cipher.set_key_and_iv_042760(token_recovered::constants::key_084a50,token_recovered::constants::iv_084a60);
    require(cipher.decrypt_with_offset(decrypted,0x41),"actual recovered AES decrypts emitted CBC/padding at offset41");
    require(decrypted.substr(0,65)==header,"AES offset leaves all65 header bytes untouched");return decrypted.substr(65);
}
void regular_skip_and_status_prefix(){
    {Fixture f;f.stats.result={0,FILE_ATTRIBUTE_NORMAL,{}};f.stats.on_query=[&]{f.content="mutated-not-written";f.user="mutated-not-written";};const std::string key("K\0<&",4);f.apply(key);
        require(f.stats.paths==std::vector<Path>{u"MEM\\license.dlf"},"regular-file check targets existing02af30 base plus filesystem suffix");
        require(f.timeline==std::vector<std::string>{"status"}&&f.sdk.creates.empty()&&f.memory->writes.empty(),"regular file returns before document/directory/output effects");
        require(f.encoding.narrow_calls.size()==2&&f.encoding.narrow_calls[0].cp==65001&&f.encoding.narrow_calls[0].flags==8&&f.encoding.cp_calls==1,"skip path performs only filesystem-codepage suffix conversion before status");
        require(key==std::string("K\0<&",4),"owned COPY does not clear or modify caller key");}
    {Fixture f;f.stats.result={ERROR_ACCESS_DENIED,0,{}};bool matched=false;
        try{f.apply("synthetic");}catch(const std::filesystem::filesystem_error& e){matched=e.code().value()==ERROR_ACCESS_DENIED&&e.code().category()==std::system_category()&&e.path1()==std::filesystem::path(u"MEM\\license.dlf")&&std::string(e.what()).find("status")!=std::string::npos;}
        require(matched&&f.timeline==std::vector<std::string>{"status"}&&f.memory->writes.empty(),"unsupported status error preserves code/path/status prefix without creation");}
}
void encrypted_counted_xml_and_live_state(){
    Fixture f;const std::string key("K\0<&",4);const std::string content("C\0>&",4),user("U\0\"<",4);
    f.stats.on_query=[&]{f.content=content;f.user=user;f.base=u"CHANGED\\base";};f.apply(key);
    require(f.timeline==std::vector<std::string>({"status","mkdir","open"})&&f.sdk.creates==std::vector<Path>{u"MEM"},"source creates actual parent before opening captured original target");
    require(f.memory->writes==std::vector<Path>{u"MEM\\license.dlf"}&&f.memory->events==std::vector<std::string>({"open","write","close"}),"actual binary writer writes/closes cached pre-status path once");
    const auto& bytes=f.memory->outputs.at(u"MEM\\license.dlf");const auto xml=decrypt_payload(bytes);
    require(xml==expected_xml(key,content,user),"decrypted fixed XML preserves counted key/liveContentId/liveUserId bytes including NUL");
    require(xml.find("&lt;")==std::string::npos&&xml.find("&amp;")==std::string::npos,"source concatenation does not introduce XML escaping");
    require(f.content==content&&f.user==user&&key==std::string("K\0<&",4),"state and caller key are borrowed/read or copied, never consumed in place");
}
void directory_and_write_outcomes(){
    {Fixture f;f.stats.result={0,FILE_ATTRIBUTE_DIRECTORY,{}};f.apply("directory-kind");require(f.memory->outputs.contains(u"MEM\\license.dlf"),"nonregular directory-kind target does not take regular-file skip");}
    {Fixture f;f.sdk.create_errors.push_back(ERROR_ALREADY_EXISTS);f.apply("existing-parent");require(f.sdk.attribute_calls==1&&f.timeline==std::vector<std::string>({"status","mkdir","attributes","open"}),"real createDirectories validates already-existing parent through GetFileAttributes spy");}
    {Fixture f;f.memory->bad_output=true;f.apply("initial-output-failure");require(f.timeline==std::vector<std::string>({"status","mkdir","open"})&&f.memory->events==std::vector<std::string>{"open"}&&f.memory->outputs.empty(),"native void route ignores actual write_binary FALSE without write or close");}
    {Fixture f;f.sdk.create_errors.push_back(ERROR_ACCESS_DENIED);bool matched=false;try{f.apply("mkdir-failure");}catch(const std::filesystem::filesystem_error& e){matched=e.code().value()==ERROR_ACCESS_DENIED&&e.code().category()==std::system_category()&&e.path1()==std::filesystem::path(u"MEM")&&std::string(e.what()).find("create_directories")!=std::string::npos;}
        require(matched&&f.timeline==std::vector<std::string>({"status","mkdir"})&&f.memory->writes.empty(),"directory failure propagates exact prefix before file acquisition");}
    {Fixture f;f.memory->throw_write=true;rejects<std::runtime_error>([&]{f.apply("write-failure");},"known write exception propagates");require(f.memory->events==std::vector<std::string>({"open","write"}),"write exception skips source explicit close instead of fabricating cleanup");}
}
// Test-only call-point bridge; production factory binding is not implemented
// or claimed by this test. All other routes use the existing concrete factory.
class AtConfigurationCall final:public configuration_factory::LicenseEffects {
    configuration::State& state_;license_recovered::KeyDependencies dependencies_;
public:
    AtConfigurationCall(configuration::State& s,license_recovered::KeyDependencies d):state_(s),dependencies_(d){}
    void apply(std::string_view key)override{license_recovered::apply_license_key_02a830(std::string(key),{state_.license_directory,state_.content_id,state_.user_id},dependencies_);}
};
class UnusedLegacy final:public configuration_factory::LegacyDocumentReader {
public:configuration::Node read(const Path&)override{throw std::logic_error("actual Config2 application must not enter legacy document route");}
};
void actual_configuration_call_order(){
    Fixture f;configuration::State state{};state.executable_path=u"C:\\own\\host.exe";state.content_id="OLD-CONTENT";state.user_id="OLD-USER";
    config_hash::Node selected;selected.lookup("Game").lookup("ContentId").value="NEW-CONTENT";selected.lookup("Game").lookup("KeyForLicense").value="SYNTHETIC-KEY";selected.lookup("User").lookup("UserId").value="NEW-USER";
    b7_test::PathSdk sdk;sdk.program=L"MEM";b7_test::paths=&sdk;
    configuration_factory::Inputs in;in.stream=std::make_shared<std::ostringstream>();in.filesystem=std::shared_ptr<config_storage::Filesystem>(&f.filesystem,[](auto*){});in.encoding=std::make_unique<b7_test::Encoding>();in.platform=b7_test::path_api();
    configuration_factory::Configuration dependencies(std::move(in),{},std::make_unique<AtConfigurationCall>(state,f.deps()),std::make_unique<UnusedLegacy>());
    config_application::apply_config2(state,selected,false,dependencies);
    require(state.content_id=="NEW-CONTENT"&&state.user_id=="NEW-USER","actual existing Config2 assignment completes after key call");
    const auto path=u"MEM\\Electronic Arts\\EA Services\\License\\NEW-CONTENT.dlf";
    require(f.memory->outputs.contains(path),"actual call point uses newly assigned Game ContentId in derived license path");
    require(decrypt_payload(f.memory->outputs.at(path))==expected_xml("SYNTHETIC-KEY","NEW-CONTENT","OLD-USER"),"actual call point reads prior UserId because current User assignment occurs later");
    b7_test::paths=nullptr;
}
}
int main(){try{regular_skip_and_status_prefix();encrypted_counted_xml_and_live_state();directory_and_write_outcomes();actual_configuration_call_order();std::cout<<"PASS: "<<checks<<" key-processor checks; synthetic status/SDK/memory files, actual AES header/XML and existing Config2 call order\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
