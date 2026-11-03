// SPDX-License-Identifier: GPL-3.0-only

#include "src/runtime/application_internal.hpp"
#include <windows.h>
#include <cstdlib>
#include <iostream>
#include <new>
#include <future>
#include <thread>
#include <stdexcept>

namespace fault {thread_local bool next=false;unsigned loads=0,frees=0,fail_frees=0;void(*on_load)()=nullptr;void(*on_mode)(UINT)=nullptr;}
void* operator new(std::size_t n){if(fault::next){fault::next=false;throw std::bad_alloc();}if(auto p=std::malloc(n?n:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p)noexcept{std::free(p);}
void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}
void operator delete[](void* p,std::size_t)noexcept{std::free(p);}
HMODULE WINAPI TestLoadLibraryW(LPCWSTR path){++fault::loads;if(fault::on_load)fault::on_load();return ::LoadLibraryW(path);}
BOOL WINAPI TestFreeLibrary(HMODULE module){++fault::frees;if(fault::fail_frees){--fault::fail_frees;SetLastError(ERROR_BUSY);return FALSE;}return ::FreeLibrary(module);}
UINT WINAPI TestSetErrorMode(UINT value){if(fault::on_mode)fault::on_mode(value);return ::SetErrorMode(value);}
#define SetErrorMode TestSetErrorMode
#define LoadLibraryW TestLoadLibraryW
#define FreeLibrary TestFreeLibrary
#include "src/platform/library_system.cpp"
#include "src/runtime/application.cpp"
#undef FreeLibrary
#undef LoadLibraryW
#undef SetErrorMode

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
struct ModeInterleave {
    std::promise<void> enter,entered,release,finished;
    std::future<void> entered_result=entered.get_future(),finished_result=finished.get_future();
    bool provider_first,provider_finishes_first,fail_extra;
    DWORD observed=0;
    static inline ModeInterleave* active=nullptr;
    static void await(std::future<void>& future) {
        if(future.wait_for(std::chrono::seconds(5))!=std::future_status::ready)
            throw std::runtime_error("error-mode interleave timed out");
        future.get();
    }
    void begin_provider(){enter.set_value();await(entered_result);}
    void end_provider(){release.set_value();await(finished_result);}
    static void at_load() {
        auto& self=*active;
        if(!self.provider_first)self.begin_provider();
        if(self.provider_finishes_first)self.end_provider();
        self.observed=GetErrorMode();
        if(self.fail_extra)throw std::runtime_error("controlled extra load failure");
    }
};
void overlapping_error_modes(const wchar_t* path) {
    const auto original=GetErrorMode();
    struct Restore {DWORD mode;~Restore(){fault::on_load=nullptr;ModeInterleave::active=nullptr;SetErrorMode(mode);}} restore{original};
    for(bool provider_first:{false,true})for(bool provider_finishes_first:{false,true})
    for(bool fail_extra:{false,true})for(DWORD incoming:{DWORD{SEM_NOOPENFILEERRORBOX},
            DWORD{SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS}}) {
        Fixture f;f.state.extra_dlls_from_main.push_back(application::detail::utf16(path));f.extras.configure(f.state);
        ModeInterleave interleave;interleave.provider_first=provider_first;
        interleave.provider_finishes_first=provider_finishes_first;interleave.fail_extra=fail_extra;
        SetErrorMode(incoming);const auto expected=GetErrorMode();
        auto enter=interleave.enter.get_future(),release=interleave.release.get_future();
        // Synchronize at the adapter boundary BEFORE invoking real LoadLibraryW;
        // never wait inside a native loader callback or while holding its lock.
        std::jthread worker([&] {
            std::optional<std::uint32_t> saved;bool announced=false;std::exception_ptr failure;
            try {
                ModeInterleave::await(enter);saved=f.provider.suppress_errors_0450a0();
                interleave.entered.set_value();announced=true;ModeInterleave::await(release);
            }catch(...){failure=std::current_exception();}
            if(saved)f.provider.restore_error_mode(*saved);
            if(failure) {
                if(!announced)interleave.entered.set_exception(failure);
                interleave.finished.set_exception(failure);
            }else interleave.finished.set_value();
        });
        ModeInterleave::active=&interleave;fault::on_load=&ModeInterleave::at_load;
        if(provider_first)interleave.begin_provider();
        bool failed=false;std::exception_ptr unexpected;
        try{f.extras.load(true);}catch(const std::runtime_error& error) {
            if(std::string_view(error.what())=="controlled extra load failure")failed=true;
            else unexpected=std::current_exception();
        }catch(...){unexpected=std::current_exception();}
        fault::on_load=nullptr;ModeInterleave::active=nullptr;
        const auto after_extra=GetErrorMode();
        if(!provider_finishes_first)interleave.end_provider();
        worker.join();const auto final_mode=GetErrorMode();
        f.extras.close();
        if(unexpected)std::rethrow_exception(unexpected);
        require(failed==fail_extra,"controlled extra load exception outcome changed");
        require(interleave.observed==(expected|SEM_FAILCRITICALERRORS),
            "overlapping load lost suppression before its final lease finished");
        require(after_extra==(provider_finishes_first?expected:expected|SEM_FAILCRITICALERRORS),
            "extra completion restored process mode while provider still held suppression");
        require(final_mode==expected,"overlapping loaders left a stale process error mode");
    }
}
namespace interval {
std::uint8_t* ready=nullptr;unsigned changes=0;bool early=true;
void observe(UINT){++changes;early=early&&ready&&!*ready;}
}
void error_mode_finish_interval(const wchar_t*) {
    Fixture f;f.a=f.b=1;
    const auto original=GetErrorMode();
    struct Restore {DWORD mode;~Restore(){fault::on_mode=nullptr;interval::ready=nullptr;SetErrorMode(mode);}} restore{original};
    SetErrorMode(SEM_NOOPENFILEERRORBOX);const auto expected=GetErrorMode();
    interval::ready=&f.c;interval::changes=0;interval::early=true;fault::on_mode=&interval::observe;
    f.extras.load(false);fault::on_mode=nullptr;
    require(interval::changes==2&&interval::early&&f.c==1&&GetErrorMode()==expected,
        "suppression must begin/end before worker finish_flag publishes readiness");
}
void acquisition_failure(const wchar_t* path) {
    Fixture f;bool threw=false;fault::next=true;fault::fail_frees=1;
    try{(void)f.extras.load_library(path);}catch(const std::bad_alloc&){threw=true;}
    fault::next=false;fault::fail_frees=0;
    require(threw&&fault::loads==0&&fault::frees==0,"ownership allocation failed after acquiring a native module");f.extras.close();
}
void load_error_mode_unwind(const wchar_t* path) {
    const auto original=GetErrorMode();
    struct Restore {DWORD mode;~Restore(){SetErrorMode(mode);}} restore{original};
    for(bool acquired_prefix:{false,true})for(bool main:{true,false})for(DWORD incoming:{DWORD{SEM_NOOPENFILEERRORBOX},
            DWORD{SEM_NOOPENFILEERRORBOX|SEM_FAILCRITICALERRORS}}) {
        Fixture f;
        // An isolated high surrogate fails the actual Win32 UTF-8 conversion
        // before LoadLibraryW. Both source startup prerequisites are ready.
        const configuration::Path invalid(1,char16_t{0xd800});
        auto& paths=main?f.state.extra_dlls_from_main:f.state.extra_dlls;
        require(GetModuleHandleW(path)==nullptr,"error mode fixture begins without a module lease");
        if(acquired_prefix)paths.push_back(application::detail::utf16(path));
        paths.push_back(invalid);
        f.extras.configure(f.state);f.a=f.b=1;
        SetErrorMode(incoming);
        const auto expected=GetErrorMode();
        bool failed=false;
        try{f.extras.load(main);}
        catch(const std::system_error& error) {
            failed=error.code().value()==ERROR_NO_UNICODE_TRANSLATION;
        }
        const auto after=GetErrorMode();
        require(failed,"extra library load must preserve the conversion failure");
        require(after==expected,"failed extra library phase changed the host error mode");
        const unsigned expected_loads=acquired_prefix?1u:0u;
        require(fault::loads==expected_loads&&fault::frees==0&&f.c==0,
            "failed conversion must preserve earlier leases without publishing readiness");
        require((GetModuleHandleW(path)!=nullptr)==acquired_prefix,
            "an earlier acquired image must remain owned after a later conversion failure");
        f.extras.close();f.extras.close();
        require(fault::frees==expected_loads&&GetModuleHandleW(path)==nullptr,
            "cleanup must release exactly the retained prefix once");
        Fixture success;success.a=success.b=1;
        success.extras.load(main);
        require(GetErrorMode()==expected&&success.c==(main?0:1),
            "successful empty phase must retain error mode and readiness semantics");
    }
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
                      {L"load-error-mode",load_error_mode_unwind},{L"overlap-error-mode",overlapping_error_modes},{L"mode-finish-interval",error_mode_finish_interval}})
        if(selected==L"all"||selected==name){fn(argv[1]);ran=true;}
    require(ran,"unknown case");std::cout<<"PASS: native-module acquisition/retained-release/retry controls\n";return 0;
}catch(const std::exception& e){fault::next=false;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
