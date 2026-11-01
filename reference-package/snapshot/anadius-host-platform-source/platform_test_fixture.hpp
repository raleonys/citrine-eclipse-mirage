#pragma once
#include "platform.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include <cstring>
#include <iomanip>

namespace b14_test {
using namespace anadius;namespace hp=host_platform;namespace hi=host_init_recovered;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct MaskCall {ULONGLONG mask;DWORD type;BYTE condition;};
struct Sdk {
    std::vector<MaskCall> masks;std::vector<std::wstring> modules;
    std::vector<std::pair<std::uintptr_t,std::string>> procedures;
    OSVERSIONINFOEXW version{};DWORD types=0;ULONGLONG mask=0;unsigned verifies=0;
    BOOL result=0x100;std::wstring missing_module;std::string missing_proc;
    std::uintptr_t next_target=0x1000;
};
inline thread_local Sdk* sdk=nullptr;
inline ULONGLONG WINAPI condition(ULONGLONG mask,DWORD type,BYTE value){
    sdk->masks.push_back({mask,type,value});return ::VerSetConditionMask(mask,type,value);
}
inline BOOL WINAPI verify(LPOSVERSIONINFOEXW request,DWORD types,DWORDLONG mask){
    std::memcpy(&sdk->version,request,sizeof(*request));sdk->types=types;sdk->mask=mask;++sdk->verifies;return sdk->result;
}
inline HMODULE WINAPI module(LPCWSTR name){
    if(!name)throw std::logic_error("host module lookup must not substitute current executable");
    sdk->modules.emplace_back(name);return reinterpret_cast<HMODULE>(sdk->missing_module==name?0:0x8000);
}
inline FARPROC WINAPI procedure(HMODULE module,LPCSTR name){
    sdk->procedures.emplace_back(reinterpret_cast<std::uintptr_t>(module),name);
    return reinterpret_cast<FARPROC>(sdk->missing_proc==name?0:(sdk->next_target+=0x100));
}
inline hp::HostApi api(){return {condition,verify,module,procedure};}
struct Engine final:recovered_hooks::Api {
    std::vector<std::string> events;std::vector<std::uintptr_t> targets,detours,enabled,disabled;
    std::int32_t init=0,uninit=0;std::map<std::size_t,int> create_results,enable_results,disable_results;
    unsigned init_calls=0,uninit_calls=0;bool throw_create=false,throw_enable=false,throw_disable=false,throw_uninit=false;
    int initialize()override{events.emplace_back("init");++init_calls;return init;}
    int create(void* target,void* detour,void** original)override{
        const auto index=targets.size();targets.push_back(reinterpret_cast<std::uintptr_t>(target));detours.push_back(reinterpret_cast<std::uintptr_t>(detour));events.emplace_back("create");
        *original=reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(target)+0x100000);
        if(throw_create)throw std::runtime_error("controlled create outcome");
        const auto found=create_results.find(index);return found==create_results.end()?0:found->second;
    }
    int enable(void* target)override{
        const auto index=enabled.size();enabled.push_back(reinterpret_cast<std::uintptr_t>(target));events.emplace_back("enable");
        if(throw_enable)throw std::runtime_error("controlled enable outcome");
        const auto found=enable_results.find(index);return found==enable_results.end()?0:found->second;
    }
    int disable(void* target)override{
        const auto index=disabled.size();disabled.push_back(reinterpret_cast<std::uintptr_t>(target));events.emplace_back("disable");
        if(throw_disable)throw std::runtime_error("controlled disable outcome");
        const auto found=disable_results.find(index);return found==disable_results.end()?0:found->second;
    }
    int uninitialize()override{events.emplace_back("uninit");++uninit_calls;if(throw_uninit)throw std::runtime_error("controlled uninit outcome");return uninit;}
};
struct Resolver final:recovered_hooks::CallbackResolver {
    std::vector<hi::Callback> seen;
    std::uintptr_t rebuilt_address(hi::Callback value)override{seen.push_back(value);return 0x90000+hi::slot(value)*0x100;}
};
struct Graph {
    Sdk plan;
    std::shared_ptr<b11_test::OwnedStream> output=std::make_shared<b11_test::OwnedStream>();
    std::shared_ptr<Engine> engine=std::make_shared<Engine>();
    std::shared_ptr<Resolver> resolver=std::make_shared<Resolver>();
    std::shared_ptr<int> callback_lifetime=std::make_shared<int>(17);
    std::shared_ptr<b13_test::EncodingState> encoding=std::make_shared<b13_test::EncodingState>();
    std::shared_ptr<hp::Domain> domain;
    std::unique_ptr<hp::HostPlatform> host;std::unique_ptr<hp::HookCleanup> cleanup;
    hp::Resources resources(){hp::Resources r;r.api=api();r.encoding=std::make_unique<b13_test::Encoding>(encoding);return r;}
    Graph(){sdk=&plan;domain=std::make_shared<hp::Domain>(output,engine,resolver,callback_lifetime,resources());host=std::make_unique<hp::HostPlatform>(domain);cleanup=std::make_unique<hp::HookCleanup>(domain);}
    ~Graph(){host.reset();cleanup.reset();domain.reset();sdk=nullptr;}
};
struct State {
    configuration::State configuration{};hi::Values values{{"old",u"erase"}};
    std::vector<std::uintptr_t> targets{0x111};std::array<std::uintptr_t,10> originals{};
    State(){configuration.executable_path=u"C:\\own\\host.exe";originals.fill(0xabc);}
    hi::StateRefs refs(){return {configuration,values,targets,originals};}
};
} // namespace b14_test
