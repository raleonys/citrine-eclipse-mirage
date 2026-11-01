// SPDX-License-Identifier: GPL-3.0-only
#include "src/files/license_files.hpp"
#include "src/files/atomic_io.hpp"
#include "src/application/environment.hpp"
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#if defined(__linux__)
#include <cerrno>
#include <cstdarg>
#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
enum class Fault {none,open,write,flush,commit,allocation,recovery,collision};
static Fault fault=Fault::none;
static int staged_fd=-1;
static unsigned open_attempts=0;
// Test-only syscall interposition. Only the process-owned staging filename
// branch is controlled; no production fault selector or ABI change is added.
extern "C" int open(const char* p,int flags,...){
    mode_t mode=0;if(flags&O_CREAT){va_list ap;va_start(ap,flags);mode=static_cast<mode_t>(va_arg(ap,int));va_end(ap);}
    const bool stage=(flags&O_EXCL)&&(flags&O_CREAT)&&std::strstr(p,".preloader_l-");
    if(stage){
        ++open_attempts;
        if(fault==Fault::open){errno=EACCES;return -1;}
        if(fault==Fault::allocation)throw std::bad_alloc{};
        if(fault==Fault::recovery)throw preloader_l::PersistenceRecoveryRequired("test recovery sentinel");
        if(fault==Fault::collision){errno=EEXIST;return -1;}
    }
    const int fd=static_cast<int>(::syscall(SYS_openat,AT_FDCWD,p,flags,mode));if(stage)staged_fd=fd;return fd;
}
extern "C" ssize_t write(int fd,const void* p,size_t size){if(fd==staged_fd&&fault==Fault::write){errno=ENOSPC;return -1;}return static_cast<ssize_t>(::syscall(SYS_write,fd,p,size));}
extern "C" int fsync(int fd){if(fd==staged_fd&&fault==Fault::flush){errno=EIO;return -1;}return static_cast<int>(::syscall(SYS_fsync,fd));}
extern "C" int rename(const char* from,const char* to)noexcept{if(fault==Fault::commit&&std::strstr(from,".preloader_l-")){errno=EIO;return -1;}return static_cast<int>(::syscall(SYS_renameat,AT_FDCWD,from,AT_FDCWD,to));}
struct FaultScope {explicit FaultScope(Fault value){fault=value;open_attempts=0;staged_fd=-1;}~FaultScope(){fault=Fault::none;staged_fd=-1;}};
#endif
namespace fs=std::filesystem;
namespace f=preloader_l::files;
namespace l=preloader_l::licensing;
using preloader_l::Path;
static unsigned checks=0;
static void check(bool ok,const char* label){++checks;if(!ok)throw std::runtime_error(label);}
static std::string get(const fs::path& p){std::ifstream in(p,std::ios::binary);return {std::istreambuf_iterator<char>(in),std::istreambuf_iterator<char>()};}
static void put(const fs::path& p,std::string_view bytes){fs::create_directories(p.parent_path());std::ofstream out(p,std::ios::binary);out.write(bytes.data(),static_cast<std::streamsize>(bytes.size()));check(bool(out),"fixture write");}
static bool no_stages(const fs::path& p){if(!fs::exists(p))return true;for(auto& e:fs::recursive_directory_iterator(p))if(e.path().filename().string().starts_with(".preloader_l-"))return false;return true;}
struct Node final:l::Node {
    std::string_view name()const override{return "RequestLicense";}
    const l::Node* first_element()const override{return nullptr;}
    const l::Node* next_element()const override{return nullptr;}
    std::optional<std::string_view> attribute(std::string_view key)const override{if(key=="RequestTicket")return "synthetic-ticket";if(key=="TicketEngine")return "synthetic-engine";return {};}
};
struct Xml final:l::Xml {
    const char* attribute_002cc0(preloader_l::handlers::Request,const char*)override{throw std::logic_error("unexpected XML lookup");}
    void open_004080(const char*,bool)override{throw std::logic_error("unexpected success response");}
    void attribute_005560(const char*,const char*)override{throw std::logic_error("unexpected response attribute");}
    void close_virtual_48(bool)override{throw std::logic_error("unexpected response close");}
};
struct Exit {int code;};
struct Effects final:l::Effects {
    std::vector<std::string> dialogs;
    std::vector<std::string> trace;
    bool first_wine=false,second_wine=false;unsigned wine_calls=0;
    const char* online_auth_virtual_30(const char*)override{throw std::logic_error("unexpected auth");}
    l::FileStatus status_01ea20(const Path&,std::uint32_t)override{return {2,0};}
    [[noreturn]]void filesystem_error_018130(std::string_view,std::int32_t,const Path&)override{throw std::logic_error("unexpected status error");}
    bool file_hash_matches_042d20(const Path&,std::string)override{throw std::logic_error("LicenseFiles hash bypassed");}
    std::string load_token_02afa0(const Path&)override{throw std::logic_error("LicenseFiles token bypassed");}
    Path filesystem_widen_006d70(std::string_view s)override{return preloader_l::filesystem_path(s);}
    Path filename_00e120(const Path& p)override{return fs::path(p).filename().u16string();}
    Path parent_path_019820(const Path& p)override{return fs::path(p).parent_path().u16string();}
    Path join_016b00(const Path& a,const Path& b)override{return (fs::path(a)/fs::path(b)).u16string();}
    std::string path_utf8_01e3f0(const Path& p)override{return preloader_l::path_utf8(p);}
    std::string current_time_041b60_null_true()override{return "fixed";}
    bool wine_detected_045010()override{trace.push_back("wine");return wine_calls++==0?first_wine:second_wine;}
    void log_literal_006880(std::string_view)override{trace.push_back("literal");}
    void log_string_006860(std::string_view)override{trace.push_back("ticket");}
    bool write_ticket_03c1d0(const Path&,std::string_view)override{throw std::logic_error("Inactive remaining ticket method was called");}
    void dialog_038bc0(std::string_view title,std::string_view,std::uint32_t)override{dialogs.emplace_back(title);trace.push_back("dialog:"+std::string(title));}
    [[noreturn]]void exit_05cdc0(std::int32_t code)override{trace.push_back("exit:"+std::to_string(code));throw Exit{code};}
    [[noreturn]]void no_response_02bcd0_04d228()override{throw std::logic_error("unexpected no response");}
};
struct Fixture {
    preloader_l::RuntimePaths paths;
    std::string auth,license,dll_hash="f8e35c47f44d379d14be1c8c98c77ad79198347f",exe_hash="eedf922ce5c596b88935b313af80472b698dc5da",content="fixture-content";
    std::uint8_t online=0,fake=0,blocked=0;std::int32_t attempts=0;
    Path dll,exe,base;
    preloader_l::handlers::Attributes game{{"DISPLAY_NAME","Fixture Game"},{"INSTALLED_VERSION","1"}};
    Effects effects;f::NativeFactory files;f::NativeTextEnvironment text;f::LicenseFiles bound;l::Handlers handlers;Node request;Xml xml;
    explicit Fixture(const fs::path& root):paths(preloader_l::RuntimePaths::from_system(root/"module/preloader_l.dll",root/"game/game.exe",root/"local",root/"program")),dll((root/"game/dbdata.dll").u16string()),exe(paths.executable().u16string()),base((paths.data()/"license").u16string()),files(paths),bound(effects,files,text),handlers({auth,online,fake,license,dll_hash,exe_hash,content,dll,exe,base,game,attempts,blocked},bound){put(paths.executable(),"fixture-exe");put(fs::path(dll),"fixture-dll");}
    fs::path ticket()const{return paths.executable().parent_path()/"Denuvo_ticket_fixed.txt";}
    void run(){(void)handlers.request_license_0320c0(request,xml);}
};
static void stream_basics(const fs::path& root){
    Fixture v(root);const auto target=v.paths.data()/"output";put(target,"OLD");
    {auto output=v.files.open_output_03a1a0(target.u16string(),0x20,0x40);check(output->initial_state()==0,"open success");output->write_03be80("NEW");check(get(target)=="OLD","target visible before commit");}
    check(get(target)=="OLD"&&no_stages(root),"abandoned stage preservation/cleanup");
    check(f::write_binary_03c1d0(target.u16string(),"COMMITTED",v.files),"normal binary output");check(get(target)=="COMMITTED"&&no_stages(root),"commit bytes/cleanup");
    auto blocker=v.paths.data()/"blocked";put(blocker,"KEEP");
    check(!f::write_binary_03c1d0((blocker/"ticket").u16string(),"NO",v.files),"initial filesystem setup error returns false");
    check(get(blocker)=="KEEP"&&no_stages(root),"failed open leaves files intact");
    bool invalid=false;try{(void)v.files.open_output_03a1a0((root/"unowned").u16string(),0x20,0x40);}catch(const std::invalid_argument&){invalid=true;}
    check(invalid,"ownership validation must propagate");
    try{v.run();check(false,"ticket branch must not return");}catch(const Exit& e){check(e.code==-1,"success source exit code");}
    check(v.effects.dialogs==std::vector<std::string>{"Ticket saved"},"success source dialog");check(v.blocked==1&&v.attempts==2,"source state changes");check(get(v.ticket()).find("synthetic-ticket")!=std::string::npos,"actual ticket bytes");
}
#if defined(__linux__)
static void controlled_failures(const fs::path& root){
    for(auto opening_fault:{Fault::open,Fault::collision})for(bool first:{false,true})for(bool second:{false,true}){
        Fixture v(root/(std::string(first?"w":"n")+(second?"w":"n")));v.effects.first_wine=first;v.effects.second_wine=second;put(v.ticket(),"OLD-TICKET");
        {FaultScope inject(opening_fault);try{v.run();check(false,"open failure must end via source exit");}catch(const Exit& e){check(e.code==-1,"failure source exit code");}check(open_attempts==(opening_fault==Fault::collision?100u:1u),"open-error branch and retained retry count were controlled");}
        const std::vector<std::string> expected=first?std::vector<std::string>{"Wine detected",second?"Wine detected":"Token required"}:std::vector<std::string>{second?"Wine detected":"Token required"};
        check(v.effects.dialogs==expected,"source false-result fallback dialog");check(v.effects.wine_calls==2,"source false-result wine predicate order");check(v.effects.trace.back()=="exit:-1","fallback must precede exit");check(get(v.ticket())=="OLD-TICKET"&&no_stages(v.ticket().parent_path()),"open failure preserves ticket");
    }
    for(auto fault_kind:{Fault::write,Fault::flush,Fault::commit}){
        Fixture v(root/"post_open");put(v.ticket(),"OLD");bool failed=false;
        {FaultScope inject(fault_kind);try{v.run();}catch(const std::system_error&){failed=true;}}
        check(failed,"post-open IO must propagate");check(v.effects.dialogs.empty(),"post-open failure must not become initial false fallback");check(get(v.ticket())=="OLD"&&no_stages(v.ticket().parent_path()),"post-open failure preserves target and cleans stage");
    }
    for(auto fault_kind:{Fault::allocation,Fault::recovery}){
        Fixture v(root/"non_io");bool propagated=false;
        {FaultScope inject(fault_kind);try{(void)v.bound.write_ticket_03c1d0(v.ticket().u16string(),"payload");}catch(const std::bad_alloc&){propagated=fault_kind==Fault::allocation;}catch(const preloader_l::PersistenceRecoveryRequired&){propagated=fault_kind==Fault::recovery;}}
        check(propagated,"non-system open error masked");check(no_stages(v.ticket().parent_path()),"non-system failure stage leak");
    }
    Fixture v(root/"poison");put(v.ticket(),"OLD");std::unique_ptr<f::Output> output;
    {FaultScope inject(Fault::write);output=v.files.open_output_03a1a0(v.ticket().u16string(),0x20,0x40);
        bool failed=false;try{output->write_03be80("partial");}catch(const std::system_error&){failed=true;}check(failed,"controlled poisoned write");}
    bool rejected=false;try{output->close_03ae70();}catch(const std::logic_error&){rejected=true;}check(rejected,"failed stage must not commit after caller catches write error");output.reset();check(get(v.ticket())=="OLD"&&no_stages(v.ticket().parent_path()),"poisoned stage preservation/cleanup");
}
#endif
int main(){try{
    static_assert(!std::is_base_of_v<std::system_error,preloader_l::PersistenceRecoveryRequired>);
    auto root=fs::temp_directory_path()/("preloader_l_ticket_"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));struct Cleanup{fs::path p;~Cleanup(){std::error_code ec;fs::remove_all(p,ec);}}cleanup{root};
#if defined(__linux__)
    controlled_failures(root/"controlled");
#else
    std::cout<<"Controlled syscall fault cases NOT_RUN on this platform\n";
#endif
    stream_basics(root/"basic");
    std::cout<<"PASS ticket output checks="<<checks<<"; native Windows fault cases NOT_RUN\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL unexpected exception\n";return 1;}}
