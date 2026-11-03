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
struct ForeignDestroyFailure {unsigned marker=0x7351;};
struct ForeignObject {
    std::uintptr_t table;
    unsigned calls=0,effects=0;
    bool before_effect=false,poison=false;
};
void foreign_destroy(void* raw) {
    auto& object=*static_cast<ForeignObject*>(raw);++object.calls;
    if(object.before_effect)throw ForeignDestroyFailure{};
    ++object.effects;
    if(object.poison)object.table=1; // A retry must reject before reading this address.
    throw ForeignDestroyFailure{};
}
void foreign_destroy_ok(void* raw){++static_cast<ForeignObject*>(raw)->calls;}
void provider_unknown_destruction(const wchar_t* path) {
    for(const bool before:{false,true}) {
        // Quarantined owners, object storage, table and real module leases
        // stay allocated until process exit. This case runs last in "all".
        auto& f=*new Fixture;
        f.provider.module=reinterpret_cast<std::uintptr_t>(TestLoadLibraryW(path));
        const auto module=f.provider.module;require(module!=0,"quarantine provider module lease");
        auto& table=*new std::array<std::uintptr_t,7>{};table[0]=reinterpret_cast<std::uintptr_t>(&foreign_destroy);
        auto& object=*new ForeignObject{reinterpret_cast<std::uintptr_t>(table.data()),0,0,before,!before};
        f.provider.object=reinterpret_cast<std::uintptr_t>(&object);f.provider.online=1;
        f.env.provider=f.provider.object;
        bool original=false;std::exception_ptr first_failure;
        try{f.provider.close();}catch(const ForeignDestroyFailure& error){original=error.marker==0x7351;first_failure=std::current_exception();}
        require(original&&object.calls==1&&object.effects==(before?0u:1u),
            "foreign destructor failure was replaced or its effects changed");
        const auto retained=f.provider.object;
        auto rejected=[&](auto operation){bool blocked=false;try{operation();}catch(const ForeignDestroyFailure& error){blocked=error.marker==0x7351&&std::current_exception()==first_failure;}require(blocked,"unknown destruction allowed another foreign operation");};
        rejected([&]{f.provider.close();});
        rejected([&]{f.provider.destroy_virtual_00(retained);});
        rejected([&]{(void)f.provider.status_virtual_08(retained);});
        rejected([&]{(void)f.provider.username_virtual_10(retained);});
        rejected([&]{(void)f.provider.persona_virtual_18(retained);});
        rejected([&]{(void)f.provider.user_id_virtual_20(retained);});
        rejected([&]{(void)f.provider.checked_method<const char*(*)(void*,const char*)>(retained,6);});
        rejected([&]{(void)f.provider.free_library(module);});
        rejected([&]{f.provider.load();});
        require(object.calls==1&&fault::frees==0&&f.provider.object==retained&&
                f.provider.module==module&&f.provider.online==1&&f.env.provider==0,
            "unknown destruction lost retained ownership or repeated destruction/release");
        require(::GetModuleHandleW(path)==reinterpret_cast<HMODULE>(module),
            "quarantined provider module was unloaded");
    }
}
void provider_resolution_retry(const wchar_t* path) {
    Fixture f;f.provider.module=reinterpret_cast<std::uintptr_t>(TestLoadLibraryW(path));
    require(f.provider.module!=0,"resolution retry module lease");
    std::array<std::uintptr_t,7> table{};
    ForeignObject object{0};f.provider.object=reinterpret_cast<std::uintptr_t>(&object);
    for(unsigned attempt=0;attempt<2;++attempt) {
        bool rejected=false;try{f.provider.close();}catch(const std::logic_error&){rejected=true;}
        require(rejected&&object.calls==0&&fault::frees==0,"slot resolution consumed ownership");
        object.table=reinterpret_cast<std::uintptr_t>(table.data());
    }
    table[0]=reinterpret_cast<std::uintptr_t>(&foreign_destroy_ok);
    fault::fail_frees=1;bool failed=false;
    try{f.provider.close();}catch(const std::runtime_error&){failed=true;}
    require(failed&&object.calls==1&&f.provider.object==0&&f.provider.module!=0&&fault::frees==1,
        "normal destruction did not preserve the failed module-only lease");
    f.provider.close();f.provider.close();
    require(object.calls==1&&f.provider.module==0&&fault::frees==2,
        "pre-call retry or module-only retry repeated successful destruction");
}
void provider_empty_slot(const wchar_t*) {
    std::array<std::uintptr_t,7> table{};
    const std::uintptr_t object=reinterpret_cast<std::uintptr_t>(table.data());
    bool rejected=false;
    try{(void)application::Provider::method<int(*)(void*)>(reinterpret_cast<std::uintptr_t>(&object),1);}
    catch(const std::logic_error&){rejected=true;}
    require(rejected,"empty provider vtable slot must be rejected before an indirect call");
}
void provider_byte_layout(const wchar_t*) {
    using Method=int(*)(void*);
    const auto expected=+[](void*){return 73;};
    // Deliberately unaligned readable storage: no pointer object is constructed
    // in either buffer. Resolve/call the slot through the production adapter.
    alignas(std::uintptr_t) std::array<std::byte,1+7*sizeof(std::uintptr_t)> table{};
    alignas(std::uintptr_t) std::array<std::byte,1+sizeof(std::uintptr_t)> object{};
    const auto table_address=reinterpret_cast<std::uintptr_t>(table.data()+1);
    const auto function_address=reinterpret_cast<std::uintptr_t>(expected);
    std::memcpy(object.data()+1,&table_address,sizeof(table_address));
    std::memcpy(table.data()+1+sizeof(std::uintptr_t),&function_address,sizeof(function_address));
    const auto object_address=reinterpret_cast<std::uintptr_t>(object.data()+1);
    const auto function=application::Provider::method<Method>(object_address,1);
    require(function==expected&&function(reinterpret_cast<void*>(object_address))==73,
        "provider byte-backed vtable resolution changed the slot or object");
    const std::uintptr_t null_table=0;
    for(const auto address:{std::uintptr_t{0},reinterpret_cast<std::uintptr_t>(&null_table)}) {
        bool rejected=false;
        try{(void)application::Provider::method<Method>(address,1);}
        catch(const std::logic_error&){rejected=true;}
        require(rejected,"null provider object/table must fail before the foreign call");
    }
}
}
int wmain(int argc,wchar_t** argv)try {
    require(argc==2||argc==3,"expected fixture DLL path and optional case");
    const std::wstring_view selected=argc==3?argv[2]:L"all";bool ran=false;
    for(auto [name,fn]:{std::pair{L"acquire",acquisition_failure},{L"mixed",mixed_release},{L"retry",repeated_failure},
                      {L"provider-retry",provider_retry},{L"provider-object",provider_object_once},
                      {L"provider-slot",provider_empty_slot},{L"provider-layout",provider_byte_layout},
                      {L"provider-physical-lease",provider_physical_lease},
                      {L"provider-resolution-retry",provider_resolution_retry},
                      {L"provider-unknown-destruction",provider_unknown_destruction}})
        if(selected==L"all"||selected==name){fn(argv[1]);ran=true;}
    require(ran,"unknown case");std::cout<<"PASS: native-module acquisition/retained-release/retry controls\n";return 0;
}catch(const std::exception& e){fault::next=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
