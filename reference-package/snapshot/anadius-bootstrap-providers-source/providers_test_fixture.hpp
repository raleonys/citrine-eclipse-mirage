#pragma once
#include "providers.hpp"
#include "native_threads.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <atomic>
#include <system_error>

namespace b5_test {
using namespace anadius;namespace p=bootstrap_providers;namespace t=threads_recovered;
using b2_test::require;using b2_test::rejects;
struct Policies final:configuration::Dependencies {
    b2_test::Events& events;unsigned logs=0,pngs=0,persistence=0;bool fail_mkdir=false;
    explicit Policies(b2_test::Events& e):events(e){}
    configuration::Path executable_path_03c6d0()override{return u"C:\\fixture\\host.exe";}
    configuration::Path parent_path_019820(const configuration::Path&)override{throw std::logic_error("source parent route required");}
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&)override{throw std::logic_error("source join route required");}
    configuration::Path widen_0145d0(std::string_view s)override{return b2_test::wide(s);}
    configuration::Path filesystem_widen_010540(std::string_view)override{throw std::logic_error("source encoding route required");}
    std::string narrow_path(const configuration::Path& s)override{return b2_test::ascii(s);}
    configuration::Path resolve_path_0192d0(std::string_view s,const configuration::Path& exe,bool relative)override{require(exe==u"C:\\fixture\\host.exe"&&relative,"config resolution preserves caller policy operands");return b2_test::wide(s);}
    configuration::Path program_data_03c920()override{return u"PROGRAMDATA";}
    configuration::Path local_app_data_03c6f0()override{return u"LOCAL";}
    configuration::Node load_document(const configuration::Path&)override{throw std::logic_error("actual FilePass required");}
    void log_config_path(const configuration::Path& s)override{++logs;events.emplace_back(s.ends_with(u"anadius_override.cfg")?"log-override":"log-base");}
    bool crt_is_space(unsigned char)override{throw std::logic_error("selected compiler CRT route required");}
    int crt_tolower(int)override{throw std::logic_error("selected compiler CRT route required");}
    void apply_license_key_02a830(std::string_view)override{throw std::logic_error("no license fixture");}
    void create_directories_018e00(const configuration::Path& s)override{require(s==u"LOCAL\\anadius\\LSX emu","recovered join feeds derived directory to explicit policy");events.emplace_back("mkdir");if(fail_mkdir)throw std::runtime_error("mkdir-prefix");}
    void write_embedded_png_03c020(const configuration::Path& s,configuration::EmbeddedPng)override{require(s.starts_with(u"LOCAL\\anadius\\LSX emu\\"),"PNG path uses same derived source state");++pngs;}
    std::vector<configuration::Path> directory_entries(const configuration::Path&)override{return {u"C:\\fixture\\plain",u"C:\\fixture\\name_AVATAR_x.png"};}
    configuration::Path filename(const configuration::Path&)override{throw std::logic_error("source filename route required");}
    void remove_04948c(const configuration::Path& s)override{require(s==u"C:\\fixture\\name_AVATAR_x.png","source filename filter selects exact case-sensitive avatar");events.emplace_back("avatar-remove");}
    void update_achievement_storage_01eb00(configuration::State& s)override{require(s.sdk=="1,2,3","source config normalization precedes persistence");++persistence;events.emplace_back("persistence");}
    bool set_environment_variable(std::string_view,std::string_view)override{throw std::logic_error("selected SDK environment route required");}
};
struct Encoding final:encoding_recovered::Calls {
    unsigned conversions=0;std::uint32_t cp=65001,error=0;bool fail=false;
    std::uint32_t locale_codepage_0629ec()override{return cp;}
    bool are_file_apis_ansi()override{return true;}
    int multi_byte_to_wide(std::uint32_t code,std::uint32_t flags,const char* s,int n,char16_t* out,int cap)override{
        require(code==cp&&flags==8,"existing encoding algorithm selects exact codepage and flags");++conversions;
        if(fail){error=1113;return 0;}if(out){require(cap==n,"existing two-pass encoding preserves requested capacity");for(int i=0;i<n;++i)out[i]=static_cast<unsigned char>(s[i]);}return n;
    }
    int wide_to_multi_byte(std::uint32_t,std::uint32_t,const char16_t*,int,char*,int,bool*)override{throw std::logic_error("no inferred narrow-path binding");}
    std::uint32_t last_error()override{return error;}
};
struct PlatformState {
    bool attach_ok=false,allocate_ok=false;int reopen_result=0;unsigned attaches=0,allocations=0,moves=0;
    std::vector<std::string> opens;std::vector<FILE*> original_streams;
    std::vector<std::pair<std::string,std::string>> environment;int lower_operand=0,space_operand=0;
};
inline PlatformState* platform=nullptr;
inline int space(int v){platform->space_operand=v;return v==' '?8:0;}
inline int lower(int v){platform->lower_operand=v;return v>='A'&&v<='Z'?v+32:v;}
inline BOOL WINAPI environment(LPCSTR k,LPCSTR v){platform->environment.emplace_back(k,v);return FALSE;}
inline BOOL WINAPI attach(DWORD v){require(v==0xffffffffu,"console uses exact parent-process identifier");++platform->attaches;return platform->attach_ok;}
inline BOOL WINAPI allocate(){++platform->allocations;return platform->allocate_ok;}
inline HWND WINAPI window(){return reinterpret_cast<HWND>(0x55);}
inline BOOL WINAPI move(HWND h,int x,int y,int w,int ht,BOOL repaint){require(h==reinterpret_cast<HWND>(0x55)&&x==100&&y==100&&w==800&&ht==500&&repaint,"console move preserves observed operands");++platform->moves;return FALSE;}
inline errno_t reopen(FILE** out,const char* name,const char* mode,FILE* input){
    platform->opens.push_back(std::string(name)+":"+mode);platform->original_streams.push_back(input);
    require(out&&*out==input,"compiler reopen has a live FILE* output cell separate from FILE record");
    *out=platform->reopen_result?nullptr:reinterpret_cast<FILE*>(reinterpret_cast<std::uintptr_t>(input)+1);
    return platform->reopen_result;
}
inline p::ConfigApi config_api(){return {space,lower,environment};}
inline p::ConsoleApi console_api(){return {attach,allocate,window,move,reopen};}
inline std::array<FILE*,3> streams(){return {reinterpret_cast<FILE*>(0x100),reinterpret_cast<FILE*>(0x200),reinterpret_cast<FILE*>(0x300)};}
inline thread_local t::Context* thread_slot=nullptr;
struct ThreadLower final:b3_tracking_test::Lower {
    bool throw_entry=false;std::atomic<unsigned> invocations=0;
    bool get_module_handle_ex_w(std::uint32_t f,std::uintptr_t a,t::Handle* result)override{require(f==4&&a!=0,"recovered begin receives live rebuilt application entry");*result=0;return false;}
    std::uint32_t last_error()override{return ::GetLastError();}
    t::Context*& required_thread_slot_064b90()override{return thread_slot;}
    t::Context* optional_thread_context_064d08()override{return thread_slot;}
    int runtime_policy_06c0d0()override{return 0;}
    std::uint32_t invoke_entry(std::uintptr_t entry,std::uintptr_t argument)override{++invocations;if(throw_entry)throw std::runtime_error("worker-escape");return reinterpret_cast<unsigned(__stdcall*)(void*)>(entry)(reinterpret_cast<void*>(argument));}
    [[noreturn]] void exit_thread(std::uint32_t code)override{
        auto* p=thread_slot;thread_slot=nullptr;if(p)release(p);::ExitThread(code);
    }
    [[noreturn]] void free_library_and_exit_thread(t::Handle,std::uint32_t code)override{exit_thread(code);}
};
} // namespace b5_test
