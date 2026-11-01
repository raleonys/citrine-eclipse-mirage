#pragma once
#include "config_platform.hpp"
#include "services.hpp"
#include "../anadius-bootstrap-providers-source/providers_test_fixture.hpp"
#include "../anadius-thread-domain-source/domain.hpp"
#include <array>
#include <atomic>
#include <cstring>
#include <map>
#include <mutex>
#include <set>
#include <system_error>

namespace b7_test {
using namespace anadius;
namespace p=bootstrap_policies;namespace d=thread_domain;
using b2_test::require;using b2_test::rejects;
inline unsigned count(){return b2_test::checks+b3_tracking_test::checks;}
struct PathSdk {
    std::wstring executable=L"C:\\fixture\\host.exe",program=L"PROGRAMDATA",local=L"LOCAL";
    bool no_module=false,no_environment=false;
    std::vector<DWORD> module_capacities,environment_capacities,incoming_errors;
    std::vector<std::wstring> names;
};
inline PathSdk* paths=nullptr;
inline DWORD WINAPI module_filename(HMODULE module,LPWSTR buffer,DWORD capacity){
    auto& s=*paths;require(module==nullptr,"configuration executable lookup selects current process module");
    s.module_capacities.push_back(capacity);s.incoming_errors.push_back(::GetLastError());
    ::SetLastError(0x701+static_cast<DWORD>(s.module_capacities.size()));
    if(s.no_module)return 0;
    const auto copied=std::min<std::size_t>(s.executable.size(),capacity);
    std::copy_n(s.executable.data(),copied,buffer);
    if(copied<capacity)buffer[copied]=0;
    return static_cast<DWORD>(copied);
}
inline DWORD WINAPI environment(LPCWSTR name,LPWSTR buffer,DWORD capacity){
    auto& s=*paths;s.names.emplace_back(name);s.environment_capacities.push_back(capacity);s.incoming_errors.push_back(::GetLastError());
    ::SetLastError(0x721+static_cast<DWORD>(s.environment_capacities.size()));
    if(s.no_environment)return 0;
    const auto& text=std::wstring_view(name)==L"ProgramData"?s.program:s.local;
    if(text.size()>=capacity)return static_cast<DWORD>(text.size()+1);
    std::copy(text.begin(),text.end(),buffer);buffer[text.size()]=0;return static_cast<DWORD>(text.size());
}
inline p::ConfigPlatformApi path_api(){return {module_filename,environment,::GetLastError,::SetLastError};}
struct Encoding final:encoding_recovered::Calls {
    std::uint32_t cp=65001,error=1113;bool ansi=true,used_default=false,fallback_once=false,fail=false,fail_fill=false,short_fill=false;
    unsigned cp_calls=0,ansi_calls=0;
    struct Call {std::uint32_t cp,flags;int size,capacity;bool fill,has_default;};
    std::vector<Call> narrow_calls,wide_calls;
    std::uint32_t locale_codepage_0629ec()override{++cp_calls;return cp;}
    bool are_file_apis_ansi()override{++ansi_calls;return ansi;}
    int multi_byte_to_wide(std::uint32_t code,std::uint32_t flags,const char* s,int n,char16_t* output,int cap)override{
        narrow_calls.push_back({code,flags,n,cap,output!=nullptr,false});
        if(fail||(fail_fill&&output))return 0;
        const auto result=output&&short_fill?n-1:n;
        if(output){require(cap==n,"two-pass UTF16 conversion retains first-pass capacity");for(int i=0;i<result;++i)output[i]=static_cast<unsigned char>(s[i]);}return result;
    }
    int wide_to_multi_byte(std::uint32_t code,std::uint32_t flags,const char16_t* s,int n,char* output,int cap,bool* used)override{
        wide_calls.push_back({code,flags,n,cap,output!=nullptr,used!=nullptr});
        if(used)*used=used_default;
        if(fallback_once&&flags){fallback_once=false;error=0x3ec;return 0;}
        if(fail)return 0;
        if(output){require(cap==n,"two-pass narrow conversion retains first-pass capacity");for(int i=0;i<n;++i)output[i]=static_cast<char>(s[i]);}return n;
    }
    std::uint32_t last_error()override{return error;}
};
struct ConfigFixture {
    b2_test::Events events;PathSdk sdk;std::unique_ptr<p::ConfigurationPlatform> config;Encoding* encoding=nullptr;b5_test::Policies* lower=nullptr;
    ConfigFixture(){paths=&sdk;auto policy=std::make_unique<b5_test::Policies>(events);lower=policy.get();auto calls=std::make_unique<Encoding>();encoding=calls.get();config=std::make_unique<p::ConfigurationPlatform>(std::move(policy),std::move(calls),std::make_unique<encoding_recovered::PortableErrors>(),path_api());}
    ~ConfigFixture(){config.reset();paths=nullptr;}
};
struct MemorySdk {
    std::mutex gate;std::map<void*,SIZE_T> blocks;
    bool deny_alloc=false,deny_free=false,throw_after_alloc=false,throw_after_free=false;std::atomic<unsigned> allocations=0,frees=0;
    std::atomic<bool> valid=true;unsigned handlers=0;std::uint64_t handler_bytes=0;
    std::size_t size(){const std::lock_guard lock(gate);return blocks.size();}
};
inline MemorySdk* memory_sdk=nullptr;
inline LPVOID WINAPI heap_alloc(HANDLE heap,DWORD flags,SIZE_T bytes){
    auto& s=*memory_sdk;++s.allocations;
    if(heap!=::GetProcessHeap())s.valid=false;
    if(s.deny_alloc){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;}
    auto* value=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();
    if(value){const std::lock_guard lock(s.gate);if(!s.blocks.emplace(value,bytes).second)s.valid=false;}
    ::SetLastError(error);if(s.throw_after_alloc)throw std::runtime_error("unknown allocation outcome");return value;
}
inline BOOL WINAPI heap_free(HANDLE heap,DWORD flags,LPVOID block){
    auto& s=*memory_sdk;
    if(s.deny_free){::SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    const std::lock_guard lock(s.gate);
    if(heap!=::GetProcessHeap()||flags||!s.blocks.contains(block))s.valid=false;
    const auto result=::HeapFree(heap,flags,block);const auto error=::GetLastError();
    if(result){if(s.blocks.erase(block)!=1)s.valid=false;++s.frees;}::SetLastError(error);if(s.throw_after_free)throw std::runtime_error("unknown release outcome");return result;
}
inline std::int32_t new_handler(std::uint64_t bytes){auto& s=*memory_sdk;++s.handlers;s.handler_bytes=bytes;s.deny_alloc=false;return 1;}
inline d::Seed domain_seed(bool handler=false){
    auto identity=std::make_shared<std::array<std::byte,8>>();
    return {0x4b1c258aff390261ULL,0,handler?new_handler:nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity};
}
inline d::Apis domain_api(){d::Apis api;api.heap.heap_alloc=heap_alloc;api.heap.heap_free=heap_free;return api;}
struct DomainFixture {
    MemorySdk sdk;std::shared_ptr<d::DomainStorage> domain;
    explicit DomainFixture(bool handler=false){memory_sdk=&sdk;domain=d::DomainStorage::create(domain_seed(handler),domain_api());require(domain->initialize(),"test owns initialized real DomainStorage");}
    ~DomainFixture(){sdk.deny_alloc=false;sdk.deny_free=false;if(domain){try{if(!domain->close())std::terminate();}catch(...){std::terminate();}domain.reset();}memory_sdk=nullptr;}
};
inline BOOL WINAPI no_module(DWORD flags,LPCWSTR address,HMODULE* output){
    if(flags!=4||!address||!output)std::terminate();*output=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;
}
inline thread_native::Api thread_api(){auto api=thread_native::native_api();api.get_module=no_module;return api;}
} // namespace b7_test
