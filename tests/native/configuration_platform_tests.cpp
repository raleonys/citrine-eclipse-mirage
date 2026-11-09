// SPDX-License-Identifier: GPL-3.0-only

#include "../../src/runtime/application_internal.hpp"
#include "citrine_eclipse_mirage/config/application.hpp"
#include "citrine_eclipse_mirage/core/paths.hpp"
#include "citrine_eclipse_mirage/codec/crypto.hpp"
#include "citrine_eclipse_mirage/core/token_constants.hpp"
#include <map>
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <locale.h>
#include <thread>

namespace controlled_encoding {
enum class Mode { complete, short_fill, query_error, fill_error };
bool armed=false;
Mode mode=Mode::complete;
unsigned calls=0;
const std::string bytes("\xc3\xa9\0Z",4);
const auto real_convert=&::MultiByteToWideChar;
const auto real_codepage=&___lc_codepage_func;
const auto real_ansi=&::AreFileApisANSI;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Scope {
    explicit Scope(Mode selected){mode=selected;calls=0;armed=true;}
    ~Scope(){armed=false;}
};
unsigned int codepage(){
    require(!armed,"UTF8 Environment route must not query the filesystem locale");
    return real_codepage();
}
BOOL WINAPI ansi(){
    require(!armed,"UTF8 Environment route must not query file API encoding");
    return real_ansi();
}
int WINAPI convert(UINT cp,DWORD flags,LPCCH input,int size,LPWSTR output,int capacity){
    if(!armed)return real_convert(cp,flags,input,size,output,capacity);
    require(cp==65001&&flags==8,"Environment uses strict UTF8 for both conversion passes");
    require(input&&size==4&&std::string_view(input,4)==bytes,"conversion retains counted embedded NUL input");
    ++calls;require(calls<=2,"Environment query/fill conversion makes bounded progress");
    if(calls==1){
        require(!output&&capacity==0,"first conversion pass queries required UTF16 length");
        if(mode==Mode::query_error){SetLastError(ERROR_NO_UNICODE_TRANSLATION);return 0;}
        SetLastError(ERROR_ACCESS_DENIED);return 3;
    }
    require(output&&capacity==3,"fill receives exactly the queried UTF16 capacity");
    require(output[0]==0&&output[1]==0&&output[2]==0,"fill owns fresh initialized storage");
    if(mode==Mode::fill_error){output[0]=L'?';SetLastError(ERROR_INVALID_DATA);return 0;}
    output[0]=L'\u00e9';
    // Short fill is a controlled response, not a claim about usual Win32 output.
    if(mode==Mode::short_fill){SetLastError(ERROR_ACCESS_DENIED);return 1;}
    output[1]=0;output[2]=L'Z';SetLastError(ERROR_ACCESS_DENIED);return 3;
}
}

#define MultiByteToWideChar controlled_encoding::convert
#define ___lc_codepage_func controlled_encoding::codepage
#define AreFileApisANSI controlled_encoding::ansi
#include "../../src/platform/encoding_windows.cpp"
#undef AreFileApisANSI
#undef ___lc_codepage_func
#undef MultiByteToWideChar
#include "../../src/runtime/application_environment.cpp"

using namespace citrine_eclipse_mirage;
namespace fs=std::filesystem;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class E,class F>void rejects(F&& action,const char* why){
    bool rejected=false;try{action();}catch(const E&){rejected=true;}require(rejected,why);
}
struct Directory {
    fs::path path=fs::temp_directory_path()/("surge-config-contract-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));
    Directory(){require(fs::create_directory(path),"fresh owned fixture directory");}
    ~Directory(){std::error_code e;fs::remove_all(path,e);}
};
struct Fixture {
    configuration::State state{};
    application::Options options;
    file::NativeFactory files;
    xml_reader_bridge::Backend xml;
    achievement_storage::Paths paths;
    achievement_runtime::HostClock clock;
    achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
};
void encoding_failure_boundaries(){
    using controlled_encoding::Scope;using controlled_encoding::Mode;
    using controlled_encoding::calls;using controlled_encoding::bytes;
    Fixture f;
    {
        Scope probe(Mode::complete);
        require(f.env.widen_0145d0({}).empty()&&calls==0,"empty UTF8 path skips native conversion");
        require(f.env.resolve_path_0192d0(bytes,u"C:\\game\\host.exe",false)==configuration::Path(u"\u00e9\0Z",3)&&calls==2,
            "actual resolver retains exact counted UTF16 and ignores stale success error");
    }
    {
        Scope probe(Mode::short_fill);
        require(f.env.widen_0145d0(bytes)==configuration::Path(u"\u00e9\0\0",3)&&calls==2,
            "controlled short fill retains the first-pass length and initialized suffix");
    }
    for(const auto selected:{Mode::query_error,Mode::fill_error}){
        f.state.registry_spoof=0;f.state.language_registry_key=u"retained-key";
        {
            Scope probe(selected);bool caught=false;
            try{configuration::apply_registry_path(f.state,bytes,f.env);}
            catch(const std::system_error& error){
                caught=error.code().category()==std::system_category()&&
                    error.code().value()==(selected==Mode::query_error?ERROR_NO_UNICODE_TRANSLATION:ERROR_INVALID_DATA);
            }
            require(caught&&calls==(selected==Mode::query_error?1u:2u),"query/fill failures report their own native error and stop");
            require(f.state.registry_spoof==1&&f.state.language_registry_key==u"retained-key",
                "actual registry path failure retains the old key after publishing its flag");
        }
        {
            Scope retry(Mode::complete);
            configuration::apply_registry_path(f.state,bytes,f.env);
            require(calls==2&&f.state.language_registry_key==configuration::Path(u"\u00e9\0Z",3),
                "conversion retry starts with fresh storage and publishes only complete counted output");
        }
    }
    require(!controlled_encoding::armed,"controlled responses are disarmed before real filesystem/path checks");
    std::cout<<"PASS: actual Environment UTF8 query/fill failures, counted results, registry partial state and retry\n";
}
std::string read(const fs::path& path){std::ifstream f(path,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void paths(){
    Fixture f;const configuration::Path exe=u"C:\\game\\host.exe";
    const std::vector<std::pair<std::string,configuration::Path>> cases{
        {"plain.dat",u"plain.dat"},{"\xc3\xa9.dat",u"\u00e9.dat"},
        {"\xc4\xb1\xc5\x9f-\xe4\xb8\xad.dat",u"\u0131\u015f-\u4e2d.dat"},
        {"\xf0\x9f\x9a\x80.dat",u"\U0001f680.dat"},
        {"D:\\caf\xc3\xa9.dat",u"D:\\caf\u00e9.dat"},
        {"\\\\server\\share\\\xc3\xa9.dat",u"\\\\server\\share\\\u00e9.dat"}
    };
    for(const auto& [input,expected]:cases){
        require(f.env.resolve_path_0192d0(input,exe,false)==expected,"UTF8 counted path conversion independent of file API codepage");
        const auto relative=path::is_absolute_0195b0(expected)?expected:configuration::Path(u"C:\\game\\")+expected;
        require(f.env.resolve_path_0192d0(input,exe,true)==relative,"only relative paths receive executable parent");
    }
    require(f.env.resolve_path_0192d0("",exe,false).empty(),"empty nonrelative path");
    const std::string nul("\xc3\xa9\0x",4);
    require(f.env.resolve_path_0192d0(nul,exe,false)==configuration::Path(u"\u00e9\0x",3),"counted conversion preserves embedded NUL");
    rejects<std::system_error>([&]{f.env.resolve_path_0192d0("\xc3\x28",exe,false);},"malformed UTF8 reports conversion failure");
    Directory dir;f.state.executable_path=(dir.path/"host.exe").u16string();
    const auto main=dir.path/"surge.cfg",over=dir.path/"surge_override.cfg";
    const std::string text="\"Config2\" { \"User\" { \"Avatar\" \"caf\xc3\xa9.png\" } }";
    require(file::write_binary_03c1d0(main.u16string(),text,f.files),"real UTF8 config file write");
    config_application::load_and_apply(f.state,false,f.env,f.files);
    require(f.state.avatar==(dir.path/fs::path(u"caf\u00e9.png")).u16string(),"active file/parser/hash path uses corrected UTF8 resolver");
    require(file::write_binary_03c1d0(over.u16string(),"\"Config2\" { \"User\" { \"Avatar\" \"\xe4\xb8\xad.png\" } }",f.files),"override UTF8 fixture");
    config_application::load_and_apply(f.state,true,f.env,f.files);
    require(f.state.avatar==(dir.path/fs::path(u"\u4e2d.png")).u16string(),"override path uses same UTF8 boundary");
    auto projected=configuration::State{};projected.executable_path=f.state.executable_path;
    configuration::load_and_apply(projected,false,f.env);
    require(projected.avatar==(dir.path/fs::path(u"caf\u00e9.png")).u16string(),"document projection shares corrected resolver");
    configuration::load_and_apply(projected,true,f.env);
    require(projected.avatar==f.state.avatar,"both configuration adapters apply the same override path");
    const auto root=f.env.load_document(main.u16string());
    require(root.child("Config2").child("User").text("Avatar")=="caf\xc3\xa9.png","document boundary returns Config2 under its owned root");
}
void avatar(){
    Fixture f;Directory dir;using Image=configuration::EmbeddedPng;
    const auto missing=dir.path/"absent"/"image.png";
    require(!file::write_binary_03c1d0(missing.u16string(),"fixture",f.files),"actual initial open failure returns false");
    f.env.write_embedded_png_03c020(missing.u16string(),Image::default_user_1800809d0);
    f.env.write_embedded_png_03c020(missing.u16string(),Image::default_avatar_180082710);
    require(!fs::exists(missing)&&!fs::exists(missing.parent_path()),"ignored initial open failure invents no file or directory");
    const auto valid=dir.path/"valid.png";require(file::write_binary_03c1d0(valid.u16string(),"sentinel",f.files),"sentinel fixture");
    rejects<std::invalid_argument>([&]{f.env.write_embedded_png_03c020(valid.u16string(),static_cast<Image>(-1));},"unknown image identity rejected");
    require(read(valid)=="sentinel","invalid image identity cannot truncate existing file");
    f.env.write_embedded_png_03c020(valid.u16string(),Image::default_avatar_180082710);
    require(fs::file_size(valid)==3882,"known fallback data remains selected");
    // Both configuration entry paths continue after an initial avatar open failure.
    const auto data=dir.path/"data";fs::create_directory(data);fs::create_directory(data/"SURGE.png");fs::create_directory(data/"default.png");
    f.options.executable_path=(dir.path/"host.exe").u16string();f.options.data_root=data.u16string();
    f.env.configure();require(f.state.default_avatar==(data/"default.png").u16string(),"runtime configure continues past initial avatar open failure");
}
void license_xml_text_escaping() {
    // Use the actual product Environment, filesystem, AES and XML reader.
    // Markup in local Config2 text must remain text, never inject new nodes.
    Directory dir;Fixture f;
    f.state.license_directory=(dir.path/"markup-license").u16string();
    f.state.content_id="game&<item>";
    f.state.user_id="user<&>\"'";
    const std::string key="secret&<Injected/>\"'";
    const auto path=dir.path/"markup-license.dlf";
    f.env.apply_license_key_02a830(key);
    auto encrypted=read(path);
    require(encrypted.size()>65&&(encrypted.size()-65)%16==0,
        "license has the same 65-byte AES envelope");
    crypto::Cipher cipher;
    cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    require(cipher.decrypt_with_offset(encrypted,65),
        "real license file decrypts with product key and IV");
    const auto xml=encrypted.substr(65);
    require(xml.find("<CipherKey>secret&amp;&lt;Injected/&gt;\"'</CipherKey>")!=std::string::npos,
        "CipherKey escapes markup and entities before encryption");
    require(xml.find("<ContentId>game&amp;&lt;item&gt;</ContentId>")!=std::string::npos,
        "ContentId cannot add markup or unresolved entities");
    require(xml.find("<UserId>user&lt;&amp;&gt;\"'</UserId>")!=std::string::npos,
        "UserId is escaped while ordinary quotes remain unchanged");
    auto parsed=f.xml.parse_c_string(xml);
    require(parsed.document!=nullptr&&!parsed.has_error,
        "actual XML backend accepts decrypted license");
    const auto* root=parsed.document->first_element();
    require(root&&root->name()=="License","license keeps its single XML root");
    std::map<std::string,std::string> fields;
    for(auto* child=root->first_element();child;child=child->next_element()) {
        const char* value=f.xml.text_003860(*child);
        require(value!=nullptr,"all license elements keep text values");
        const auto [it,inserted]=fields.emplace(std::string(child->name()),value);
        (void)it;require(inserted,"no injected or repeated XML elements");
    }
    require(fields.size()==6&&fields.at("CipherKey")==key&&
            fields.at("ContentId")==f.state.content_id&&
            fields.at("UserId")==f.state.user_id&&fields.at("MachineHash")=="1",
        "six children round-trip all raw local identifiers exactly");
    const auto preserved=read(path);
    f.env.apply_license_key_02a830("different");
    require(read(path)==preserved,"existing license is never overwritten");

    f.state.license_directory=(dir.path/"plain-license").u16string();
    f.state.content_id="42";f.state.user_id="user123";
    f.env.apply_license_key_02a830("plain-KEY");
    auto ordinary=read(dir.path/"plain-license.dlf");
    cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    require(cipher.decrypt_with_offset(ordinary,65),"normal file decrypts identically");
    require(ordinary.substr(65).find(
            "<CipherKey>plain-KEY</CipherKey><MachineHash>1</MachineHash>"
            "<ContentId>42</ContentId><UserId>user123</UserId>")!=std::string::npos,
        "ordinary values preserve the old XML byte format and field order");
}
void directory(){
    Fixture f;Directory dir;
    for(const auto* name:{"a_AVATAR_x.png","b_AVATAR_y","lower_avatar_z.png","keep.txt"})
        require(file::write_binary_03c1d0((dir.path/name).u16string(),"own fixture",f.files),"owned directory fixture");
    fs::create_directory(dir.path/"empty_AVATAR_dir");
    configuration::cleanup_cached_avatars(dir.path.u16string(),f.env);
    require(!fs::exists(dir.path/"a_AVATAR_x.png")&&!fs::exists(dir.path/"b_AVATAR_y")&&!fs::exists(dir.path/"empty_AVATAR_dir"),"real streaming cleanup removes matching files and empty directory");
    require(fs::exists(dir.path/"keep.txt")&&fs::exists(dir.path/"lower_avatar_z.png"),"cleanup preserves nonmatches and exact case");
}
void directory_nonempty(){
    Fixture f;Directory dir;
    const auto matched=dir.path/"owned_AVATAR_directory",child=matched/"child.txt",keep=dir.path/"keep.txt";
    require(fs::create_directory(matched),"own a nonempty matching directory fixture");
    require(file::write_binary_03c1d0(child.u16string(),"owned child",f.files)&&
            file::write_binary_03c1d0(keep.u16string(),"preserved nonmatch",f.files),"seed owned child and unrelated file");
    bool caught=false;
    try{configuration::cleanup_cached_avatars(dir.path.u16string(),f.env);}
    catch(const fs::filesystem_error& error){caught=error.code().value()!=0&&error.path1()==matched;}
    require(caught,"actual nonempty-directory removal propagates filesystem error and exact path");
    require(fs::is_directory(matched)&&read(child)=="owned child"&&read(keep)=="preserved nonmatch",
            "failed cleanup never recursively deletes children or changes unrelated files");
    require(fs::remove(child),"repair only the owned directory's blocking child");
    configuration::cleanup_cached_avatars(dir.path.u16string(),f.env);
    require(!fs::exists(matched)&&read(keep)=="preserved nonmatch",
            "same Environment retries removal after repair while preserving nonmatches");
    std::cout<<"PASS: actual Environment nonempty avatar-directory refusal, child preservation and repaired retry\n";
}
void directory_failure(){
    Fixture f;Directory dir;
    for(int i=0;i<3;++i)file::write_binary_03c1d0((dir.path/std::to_string(i)).u16string(),"own",f.files);
    unsigned calls=0;
    rejects<std::runtime_error>([&]{f.env.visit_directory_entries(dir.path.u16string(),{&calls,[](void* value,const configuration::Path&){++*static_cast<unsigned*>(value);throw std::runtime_error("callback failure");}});},"real visitor exception propagates");
    require(calls==1,"failed callback cannot visit a later entry");
    rejects<std::invalid_argument>([&]{f.env.visit_directory_entries((dir.path/"absent").u16string(),{});},"null visitor rejected before opening nonexistent directory");
    require(f.env.directory_entries(dir.path.u16string()).size()==3,"compatibility collection still available after callback exception");
    const auto inner=dir.path/"inner";fs::create_directory(inner);file::write_binary_03c1d0((inner/"child").u16string(),"own",f.files);
    struct Nested {application::detail::Environment& env;configuration::Path inner;unsigned outer=0,child=0;};Nested n{f.env,inner.u16string()};
    f.env.visit_directory_entries(dir.path.u16string(),{&n,[](void* p,const configuration::Path&){
        auto& n=*static_cast<Nested*>(p);++n.outer;
        n.env.visit_directory_entries(n.inner,{&n,[](void* p,const configuration::Path&){++static_cast<Nested*>(p)->child;}});
    }});
    require(n.outer==4&&n.child==4,"nested real visitors own independent iterator state");
}
void thread_errors(){
    // Binary table at087df0: values are an independent fixed compatibility oracle.
    constexpr std::array<int,7> expected{16,22,3,12,1,36,11};
    execution::WindowsPlatform platform{application::detail::thread_error};
    for(int i=0;i<7;++i){
        bool caught=false;try{platform.raise_thread_error_04a648(i);}catch(const std::system_error& e){
            caught=e.code().value()==expected[i]&&e.code().category()==std::generic_category();
        }require(caught,"actual Windows Execution boundary retains thread error category/code");
    }
    for(const auto i:{-1,7,2147483647})rejects<std::out_of_range>([&]{application::detail::thread_error(i);},"unsupported error index rejected without table read");
    application::detail::Mutex mutex;
    rejects<std::system_error>([&]{mutex.thread_error_04a648(5);},"event mutex shares structured error policy");
}
void event_initializer_retry(){
    struct Storage final:event_queue::Storage {
        event_queue::OwnedStorage owned;bool first=true;
        void construct_seed_copy(std::string_view seed)override {
            if(first){first=false;throw std::bad_alloc();}
            owned.construct_seed_copy(seed);
        }
        bool nonempty_relaxed()const override{return owned.nonempty_relaxed();}
        bool nonempty_locked()const override{return owned.nonempty_locked();}
        std::string copy_front_locked()const override{return owned.copy_front_locked();}
        void push_move_034660(std::string& value)override{owned.push_move_034660(value);}
        void pop_front_locked()override{owned.pop_front_locked();}
    } storage;
    application::detail::Events owner;
    event_queue::Context context{owner.guards,owner,owner.mutex,storage};
    rejects<std::bad_alloc>([&]{event_queue::queue_02c400(context);},"queue allocation failure propagates");
    require(owner.guards.queue_0a39a0==0,"failed owned initializer resets its in-progress guard");
    require(&event_queue::queue_02c400(context)==&storage&&storage.nonempty_locked(),"same owner retries failed initialization");
    // Another thread must not remain blocked by the failed first initialization.
    std::thread peer([&]{event_queue::queue_02c400(context);});peer.join();
    require(storage.copy_front_locked()==event_queue::initial_down_event,"retry seeds exactly one initial event");
}
}
int main(int argc,char** argv)try{
    const std::string_view group=argc>1?argv[1]:"all";
    require(group=="all"||group=="paths"||group=="avatar"||group=="directory"||group=="thread-errors"||group=="license-xml","known test group");
    if(group=="all"||group=="paths"){encoding_failure_boundaries();paths();}
    if(group=="all"||group=="avatar")avatar();
    if(group=="all"||group=="license-xml")license_xml_text_escaping();
    if(group=="all"||group=="directory"){directory();directory_nonempty();directory_failure();}
    if(group=="all"||group=="thread-errors"){thread_errors();event_initializer_retry();}
    std::cout<<"PASS: "<<checks<<" configuration platform checks\n";return 0;
}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
