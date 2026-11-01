// SPDX-License-Identifier: GPL-3.0-only

#include "src/runtime/application_internal.hpp"
#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>

namespace fault {thread_local bool next=false;unsigned loads=0,frees=0,fail_frees=0;}
void* operator new(std::size_t n){if(fault::next){fault::next=false;throw std::bad_alloc();}if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
HMODULE WINAPI TestLoadLibraryW(LPCWSTR path){++fault::loads;return ::LoadLibraryW(path);}
BOOL WINAPI TestFreeLibrary(HMODULE module){++fault::frees;if(fault::fail_frees){--fault::fail_frees;SetLastError(ERROR_BUSY);return FALSE;}return ::FreeLibrary(module);}
#define LoadLibraryW TestLoadLibraryW
#define FreeLibrary TestFreeLibrary
#include "src/platform/library_system.cpp"
#include "src/runtime/application.cpp"
#undef FreeLibrary
#undef LoadLibraryW

namespace {
using namespace preloader_l;
void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
struct Fixture {
    configuration::State state=application::initial_state();application::Options options;
    file::NativeFactory files;xml_reader_bridge::Backend xml;
    achievement_storage::Paths paths;achievement_runtime::HostClock clock;achievement_runtime::HostFormatter formatter;
    application::detail::Environment env{state,options,files,xml,paths,clock,formatter};
    execution::MutexState mutex{};std::uint8_t a=0,b=0,c=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("mutex error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{a,b,c},frequency,platform};
    application::ExtraLibraries extras{env,execution,c};
    application::Provider provider{state,env,files,execution,b};
    Fixture(){execution::initialize_mutex_04a698(mutex,2);fault::loads=fault::frees=fault::fail_frees=0;}
};
void acquisition_failure(const wchar_t* path) {
    Fixture f;bool threw=false;fault::next=true;fault::fail_frees=1;
    try{(void)f.extras.load_library(path);}catch(const std::bad_alloc&){threw=true;}
    fault::next=false;fault::fail_frees=0;
    require(threw&&fault::loads==0&&fault::frees==0,"ownership allocation failed after acquiring a native module");f.extras.close();
}
void mixed_release(const wchar_t* path) {
    Fixture f;require(f.extras.load_library(path)!=0&&f.extras.load_library(path)!=0,"fixture DLL load");
    fault::fail_frees=1;bool threw=false;
    try{f.extras.close();}catch(const std::exception&){threw=true;}
    require(threw&&fault::frees==2,"failed release was not retained or other leases were skipped");
    f.extras.close();f.extras.close();require(fault::frees==3,"retry lost the failed lease or freed a successful lease twice");
}
void repeated_failure(const wchar_t* path) {
    Fixture f;require(f.extras.load_library(path)!=0,"fixture DLL load");fault::fail_frees=2;
    for(unsigned i=0;i<2;++i){bool threw=false;try{f.extras.close();}catch(const std::exception&){threw=true;}require(threw,"cleanup failure reported success");}
    f.extras.close();f.extras.close();require(fault::frees==3,"third cleanup did not release exactly the retained module");
}
void provider_retry(const wchar_t* path) {
    Fixture f;f.provider.module=reinterpret_cast<std::uintptr_t>(TestLoadLibraryW(path));
    const auto module=f.provider.module;require(module!=0,"provider fixture DLL load");
    f.provider.online=1;fault::fail_frees=2;
    for(unsigned i=0;i<2;++i) {
        bool threw=false;try{f.provider.close();}catch(const std::exception&){threw=true;}
        require(threw&&f.provider.module==module&&f.provider.online==1&&fault::frees==i+1,
            "provider cleanup lost its failed module lease or reported success");
    }
    f.provider.close();f.provider.close();
    require(f.provider.module==0&&f.provider.online==0&&fault::frees==3,
        "provider cleanup retry did not release exactly the retained lease");
}
// B54 source comparison: distinguish a retained numeric module record from
// a real executable image lease during failed FreeLibrary cleanup.
void provider_physical_lease(const wchar_t* path) {
    Fixture f;
    require(::GetModuleHandleW(path)==nullptr,"provider test DLL was already loaded");
    f.provider.module=reinterpret_cast<std::uintptr_t>(TestLoadLibraryW(path));
    auto* const module=reinterpret_cast<HMODULE>(f.provider.module);
    require(module!=nullptr&&::GetModuleHandleW(path)==module,
        "provider acquisition must hold the actual Windows image");
    const auto reset=reinterpret_cast<void(*)(int)>(GetProcAddress(module,"FixtureMode"));
    const auto create=reinterpret_cast<std::uintptr_t(*)(const wchar_t*)>(
        GetProcAddress(module,"create_online"));
    const auto destroyed=reinterpret_cast<int(*)()>(GetProcAddress(module,"FixtureDestroyed"));
    const auto version=reinterpret_cast<int(*)()>(GetProcAddress(module,"version_check"));
    require(reset&&create&&destroyed&&version,"provider test DLL has required native exports");
    reset(0);
    f.provider.object=create(L"NUL");
    require(f.provider.object!=0&&destroyed()==0&&version()==2,
        "live provider methods are executable before shutdown");
    fault::fail_frees=1;
    bool retained=false;
    try{f.provider.close();}catch(const std::runtime_error&){retained=true;}
    require(retained&&f.provider.object==0&&
            f.provider.module==reinterpret_cast<std::uintptr_t>(module)&&fault::frees==1,
        "failed FreeLibrary retains the exact module after one object destruction");
    require(::GetModuleHandleW(path)==module&&destroyed()==1&&version()==2,
        "failed release preserves executable provider code and one-time destruction");
    f.provider.close();
    require(f.provider.module==0&&f.provider.object==0&&fault::frees==2&&
            ::GetModuleHandleW(path)==nullptr,
        "successful retry physically unloads the same module exactly once");
    f.provider.close();
    require(fault::frees==2,"completed provider cleanup must not double-unload");
}
void provider_object_once(const wchar_t* path) {
    Fixture f;
    struct Observer {
        HMODULE module;
        ~Observer(){if(module)::FreeLibrary(module);}
    } observer{::LoadLibraryW(path)};
    require(observer.module!=nullptr,"provider observer lease");
    const auto destroyed=reinterpret_cast<int(*)()>(GetProcAddress(observer.module,"FixtureDestroyed"));
    const auto create=reinterpret_cast<std::uintptr_t(*)(const wchar_t*)>(GetProcAddress(observer.module,"create_online"));
    require(destroyed&&create,"provider fixture methods");
    f.provider.module=reinterpret_cast<std::uintptr_t>(TestLoadLibraryW(path));
    require(f.provider.module!=0,"provider owned lease");
    f.provider.object=create(L"NUL");require(f.provider.object!=0&&destroyed()==0,"provider object creation");
    fault::fail_frees=1;bool threw=false;
    try{f.provider.close();}catch(const std::exception&){threw=true;}
    require(threw&&f.provider.object==0&&f.provider.module!=0&&destroyed()==1,
        "provider object/module ownership was lost after partial cleanup");
    f.provider.close();f.provider.close();
    require(f.provider.module==0&&destroyed()==1&&fault::frees==2,
        "provider cleanup destroyed the object or released the module twice");
}
}
int wmain(int argc,wchar_t** argv)try {
    require(argc==2||argc==3,"expected fixture DLL path and optional case");
    const std::wstring_view selected=argc==3?argv[2]:L"all";bool ran=false;
    for(auto [name,fn]:{std::pair{L"acquire",acquisition_failure},{L"mixed",mixed_release},{L"retry",repeated_failure},
                      {L"provider-retry",provider_retry},{L"provider-object",provider_object_once},
                      {L"provider-physical-lease",provider_physical_lease}})
        if(selected==L"all"||selected==name){fn(argv[1]);ran=true;}
    require(ran,"unknown case");std::cout<<"PASS: native-module acquisition/retained-release/retry controls\n";return 0;
}catch(const std::exception& e){fault::next=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
