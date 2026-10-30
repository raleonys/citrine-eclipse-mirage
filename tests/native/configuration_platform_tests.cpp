#include "../../src/runtime/application_internal.hpp"
#include "preloader_l/config/application.hpp"
#include "preloader_l/core/paths.hpp"
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <iterator>
#include <thread>

using namespace preloader_l;
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
    file_recovered::NativeFactory files;
    xml_reader_bridge::Backend xml;
    achievement_storage::Paths paths;
    achievement_runtime::HostClock clock;
    achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
};
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
        const auto relative=path_recovered::is_absolute_0195b0(expected)?expected:configuration::Path(u"C:\\game\\")+expected;
        require(f.env.resolve_path_0192d0(input,exe,true)==relative,"only relative paths receive executable parent");
    }
    require(f.env.resolve_path_0192d0("",exe,false).empty(),"empty nonrelative path");
    const std::string nul("\xc3\xa9\0x",4);
    require(f.env.resolve_path_0192d0(nul,exe,false)==configuration::Path(u"\u00e9\0x",3),"counted conversion preserves embedded NUL");
    rejects<std::system_error>([&]{f.env.resolve_path_0192d0("\xc3\x28",exe,false);},"malformed UTF8 reports conversion failure");
    Directory dir;f.state.executable_path=(dir.path/"host.exe").u16string();
    const auto main=dir.path/"surge.cfg",over=dir.path/"surge_override.cfg";
    const std::string text="\"Config2\" { \"User\" { \"Avatar\" \"caf\xc3\xa9.png\" } }";
    require(file_recovered::write_binary_03c1d0(main.u16string(),text,f.files),"real UTF8 config file write");
    config_application::load_and_apply(f.state,false,f.env,f.files);
    require(f.state.avatar==(dir.path/fs::path(u"caf\u00e9.png")).u16string(),"active file/parser/hash path uses corrected UTF8 resolver");
    require(file_recovered::write_binary_03c1d0(over.u16string(),"\"Config2\" { \"User\" { \"Avatar\" \"\xe4\xb8\xad.png\" } }",f.files),"override UTF8 fixture");
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
    require(!file_recovered::write_binary_03c1d0(missing.u16string(),"fixture",f.files),"actual initial open failure returns false");
    f.env.write_embedded_png_03c020(missing.u16string(),Image::default_user_1800809d0);
    f.env.write_embedded_png_03c020(missing.u16string(),Image::default_avatar_180082710);
    require(!fs::exists(missing)&&!fs::exists(missing.parent_path()),"ignored initial open failure invents no file or directory");
    const auto valid=dir.path/"valid.png";require(file_recovered::write_binary_03c1d0(valid.u16string(),"sentinel",f.files),"sentinel fixture");
    rejects<std::invalid_argument>([&]{f.env.write_embedded_png_03c020(valid.u16string(),static_cast<Image>(-1));},"unknown image identity rejected");
    require(read(valid)=="sentinel","invalid image identity cannot truncate existing file");
    f.env.write_embedded_png_03c020(valid.u16string(),Image::default_avatar_180082710);
    require(fs::file_size(valid)==3882,"known fallback data remains selected");
    // Both configuration entry paths continue after an initial avatar open failure.
    const auto data=dir.path/"data";fs::create_directory(data);fs::create_directory(data/"SURGE.png");fs::create_directory(data/"default.png");
    f.options.executable_path=(dir.path/"host.exe").u16string();f.options.data_root=data.u16string();
    f.env.configure();require(f.state.default_avatar==(data/"default.png").u16string(),"runtime configure continues past initial avatar open failure");
}
void directory(){
    Fixture f;Directory dir;
    for(const auto* name:{"a_AVATAR_x.png","b_AVATAR_y","lower_avatar_z.png","keep.txt"})
        require(file_recovered::write_binary_03c1d0((dir.path/name).u16string(),"own fixture",f.files),"owned directory fixture");
    fs::create_directory(dir.path/"empty_AVATAR_dir");
    configuration::cleanup_cached_avatars(dir.path.u16string(),f.env);
    require(!fs::exists(dir.path/"a_AVATAR_x.png")&&!fs::exists(dir.path/"b_AVATAR_y")&&!fs::exists(dir.path/"empty_AVATAR_dir"),"real streaming cleanup removes matching files and empty directory");
    require(fs::exists(dir.path/"keep.txt")&&fs::exists(dir.path/"lower_avatar_z.png"),"cleanup preserves nonmatches and exact case");
}
void directory_failure(){
    Fixture f;Directory dir;
    for(int i=0;i<3;++i)file_recovered::write_binary_03c1d0((dir.path/std::to_string(i)).u16string(),"own",f.files);
    unsigned calls=0;
    rejects<std::runtime_error>([&]{f.env.visit_directory_entries(dir.path.u16string(),{&calls,[](void* value,const configuration::Path&){++*static_cast<unsigned*>(value);throw std::runtime_error("callback failure");}});},"real visitor exception propagates");
    require(calls==1,"failed callback cannot visit a later entry");
    rejects<std::invalid_argument>([&]{f.env.visit_directory_entries((dir.path/"absent").u16string(),{});},"null visitor rejected before opening nonexistent directory");
    require(f.env.directory_entries(dir.path.u16string()).size()==3,"compatibility collection still available after callback exception");
    const auto inner=dir.path/"inner";fs::create_directory(inner);file_recovered::write_binary_03c1d0((inner/"child").u16string(),"own",f.files);
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
    execution_recovered::WindowsPlatform platform{application::detail::thread_error};
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
    struct Storage final:event_queue_recovered::Storage {
        event_queue_recovered::OwnedStorage owned;bool first=true;
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
    event_queue_recovered::Context context{owner.guards,owner,owner.mutex,storage};
    rejects<std::bad_alloc>([&]{event_queue_recovered::queue_02c400(context);},"queue allocation failure propagates");
    require(owner.guards.queue_0a39a0==0,"failed owned initializer resets its in-progress guard");
    require(&event_queue_recovered::queue_02c400(context)==&storage&&storage.nonempty_locked(),"same owner retries failed initialization");
    // Another thread must not remain blocked by the failed first initialization.
    std::thread peer([&]{event_queue_recovered::queue_02c400(context);});peer.join();
    require(storage.copy_front_locked()==event_queue_recovered::initial_down_event,"retry seeds exactly one initial event");
}
}
int main(int argc,char** argv)try{
    const std::string_view group=argc>1?argv[1]:"all";
    require(group=="all"||group=="paths"||group=="avatar"||group=="directory"||group=="thread-errors","known test group");
    if(group=="all"||group=="paths")paths();
    if(group=="all"||group=="avatar")avatar();
    if(group=="all"||group=="directory"){directory();directory_failure();}
    if(group=="all"||group=="thread-errors"){thread_errors();event_initializer_retry();}
    std::cout<<"PASS: "<<checks<<" configuration platform checks\n";return 0;
}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
