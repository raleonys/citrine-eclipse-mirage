// SPDX-License-Identifier: GPL-3.0-only

#include "application_internal.hpp"
#include "preloader_l/codec/crypto.hpp"
#include "preloader_l/core/protocol_identity.hpp"
#include "preloader_l/net/asio_service.hpp"
#include "preloader_l/net/session_handler.hpp"
#include "preloader_l/net/session_routing.hpp"
#include "preloader_l/runtime/startup.hpp"
#include "preloader_l/runtime/libraries.hpp"
#include "preloader_l/platform/hook_unload_guard.hpp"
#include "preloader_l/platform/callback_lock.hpp"
#include "preloader_l/platform/hook_transaction.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <future>
#include <mutex>
#include <optional>
#include <thread>

namespace preloader_l::application {
namespace {
using namespace detail;
// Provider and main-thread extra loads intentionally overlap. Their process-wide
// suppression is one shared lease, not independent save/restore transactions.
// Hold the mutex only while updating the mode/count, never across loader calls
// or foreign callbacks. Unrelated host SetErrorMode calls remain outside this
// module's coordination contract.
class ErrorModeLease final {
    struct Shared {std::mutex mutex;std::size_t users=0;DWORD previous=0;};
    static Shared& shared(){static Shared value;return value;}
    DWORD previous_;
public:
    ErrorModeLease() {
        auto& state=shared();std::lock_guard lock(state.mutex);
        if(!state.users){state.previous=GetErrorMode();SetErrorMode(state.previous|SEM_FAILCRITICALERRORS);}
        previous_=state.previous;++state.users;
    }
    ~ErrorModeLease() {
        auto& state=shared();std::lock_guard lock(state.mutex);
        if(!--state.users)SetErrorMode(state.previous);
    }
    ErrorModeLease(const ErrorModeLease&)=delete;
    ErrorModeLease& operator=(const ErrorModeLease&)=delete;
    DWORD previous()const noexcept{return previous_;}
};
class Console final:public startup::NativeConsolePlatform {
    std::uintptr_t stream_052238(unsigned n) override {
        if(n==0)return reinterpret_cast<std::uintptr_t>(stdin);
        if(n==1)return reinterpret_cast<std::uintptr_t>(stdout);
        if(n==2)return reinterpret_cast<std::uintptr_t>(stderr);
        throw std::out_of_range("CRT stream index");
    }
    int reopen_05f024(std::uintptr_t,const char* path,const char* mode,std::uintptr_t stream) override {
        return std::freopen(path,mode,reinterpret_cast<FILE*>(stream))?0:errno;
    }
};
class Provider final:public online::Effects {
public:
    configuration::State& state;
    Environment& env;
    file::Factory& files;
    execution::Execution& execution;
    std::uint8_t& ready;
    std::uintptr_t module=0,object=0;
    std::uint8_t online=0;
    std::optional<ErrorModeLease> error_mode;
    Provider(configuration::State& s,Environment& e,file::Factory& f,
        execution::Execution& x,std::uint8_t& r):state(s),env(e),files(f),execution(x),ready(r){}
    online::StateRefs refs(){return {state.executable_path,state.cookie_path,state.last_profile_path,
        state.last_avatar_path,state.avatar,module,object,state.username,state.persona_id,state.user_id,
        state.use_last_online_profile,online};}
    template<class Fn> static Fn method(std::uintptr_t o,std::size_t slot) {
        if(!o)throw std::logic_error("Provider object is unavailable");
        // The foreign object/table are readable byte layouts, not C++ objects
        // created with this module's pointer types or alignment.
        std::uintptr_t table=0,address=0;
        std::memcpy(&table,reinterpret_cast<const void*>(o),sizeof(table));
        if(!table)throw std::logic_error("Provider vtable is unavailable");
        std::memcpy(&address,reinterpret_cast<const std::byte*>(table)+slot*sizeof(address),sizeof(address));
        if(!address)throw std::logic_error("Provider method is unavailable");
        return reinterpret_cast<Fn>(address);
    }
    std::uint32_t suppress_errors_0450a0() override {
        if(error_mode)throw std::logic_error("Provider error suppression already active");
        error_mode.emplace();return error_mode->previous();
    }
    void restore_error_mode(std::uint32_t) override {error_mode.reset();}
    Path filesystem_widen_006d70(std::string_view s) override {
        return env.filesystem_widen_006d70(s);
    }
    Path parent_path_019820(const Path& p) override{return env.parent_path_019820(p);}
    Path join_016b00(const Path& a,const Path& b) override{return env.join_016b00(a,b);}
    std::uintptr_t load_copy_0450d0(const Path& p) override{return reinterpret_cast<std::uintptr_t>(LoadLibraryW(wide(p).c_str()));}
    std::uintptr_t get_proc_address(std::uintptr_t m,std::string_view n) override{return reinterpret_cast<std::uintptr_t>(GetProcAddress(reinterpret_cast<HMODULE>(m),std::string(n).c_str()));}
    std::int32_t call_version_check(std::uintptr_t fn) override{return reinterpret_cast<int(*)()>(fn)();}
    std::uintptr_t call_create_online(std::uintptr_t fn,const char16_t* p) override {
        const auto value=wide(p);return reinterpret_cast<std::uintptr_t>(reinterpret_cast<void*(*)(const wchar_t*)>(fn)(value.c_str()));
    }
    std::int32_t status_virtual_08(std::uintptr_t o) override{return method<int(*)(void*)>(o,1)(reinterpret_cast<void*>(o));}
    const char* username_virtual_10(std::uintptr_t o) override{return method<const char*(*)(void*)>(o,2)(reinterpret_cast<void*>(o));}
    const char* persona_virtual_18(std::uintptr_t o) override{return method<const char*(*)(void*)>(o,3)(reinterpret_cast<void*>(o));}
    const char* user_id_virtual_20(std::uintptr_t o) override{return method<const char*(*)(void*)>(o,4)(reinterpret_cast<void*>(o));}
    void destroy_virtual_00(std::uintptr_t o) override{method<void(*)(void*)>(o,0)(reinterpret_cast<void*>(o));}
    bool free_library(std::uintptr_t m) override {
        if(!FreeLibrary(reinterpret_cast<HMODULE>(m)))
            throw std::runtime_error("Online DLL cleanup incomplete");
        return true;
    }
    status::Status status_01ea20(const Path& p,std::uint32_t f) override{return env.status_01ea20(p,f);}
    [[noreturn]] void filesystem_error_018130(std::string_view op,std::uint32_t e,const Path& p) override{env.filesystem_error_018130(op,e,p);}
    std::string read_binary_03b3f0(const Path& p) override{return file::read_binary_03b3f0(p,files);}
    bool write_binary_03c1d0(const Path& p,std::string_view s) override{return file::write_binary_03c1d0(p,s,files);}
    void log_literal_006880(std::string_view text) override{env.debug_log({"provider status: ",text});}
    void log_finish_0088e0() override{env.debug_log("provider phase complete");}
    void dialog_038bc0(std::string_view,std::string_view text,std::uint32_t flags) override{MessageBoxW(nullptr,wide(env.widen_0145d0(text)).c_str(),L"SURGE",flags);}
    void wait_steps_036880(std::int32_t n) override{execution.wait_steps_036880(n);}
    void finish_flag_036820() override{execution.finish_flag_036820(ready);}
    void load(){online::worker_035b60(refs(),*this);env.provider=object;}
    void close(){env.provider=0;online::cleanup_0366f0(refs(),*this);online=0;}
};
class ExtraLibraries final:public system_services::libraries::SystemCalls,
                           public system_services::libraries::Effects {
    Environment& env_;
    execution::Execution& execution_;
    std::uint8_t& ready_;
    system_services::libraries::NativeSystemCalls native_;
    system_services::libraries::State state_;
    std::vector<std::uintptr_t> leases_;
    std::optional<ErrorModeLease> error_mode_;
public:
    ExtraLibraries(Environment& e,execution::Execution& x,std::uint8_t& r):env_(e),execution_(x),ready_(r){}
    void configure(const configuration::State& s) {
        for(const auto& p:s.extra_dlls_from_main)state_.from_main.push_back(wide(p));
        for(const auto& p:s.extra_dlls)state_.from_worker.push_back(wide(p));
    }
    void load(bool main) {
        // The normal helper exit releases before publication/finish_flag. On
        // unwind, release the remaining lease while preserving the exception.
        struct Cleanup {std::optional<ErrorModeLease>& mode;~Cleanup(){mode.reset();}} cleanup{error_mode_};
        system_services::libraries::load_phase_035850(*this,*this,state_,main);
    }
    void close() {
        bool pending=false;
        for(auto& module:leases_)if(module) {
            if(native_.free_library(module))module=0;
            else pending=true;
        }
        // Keep the exact failed leases for the next stop request. Successful
        // slots are already zero and must never be freed a second time.
        if(pending)throw std::runtime_error("Extra DLL cleanup incomplete");
        leases_.clear();state_.loaded.clear();
    }
    std::uint32_t get_error_mode() override{return native_.get_error_mode();}
    std::uint32_t set_error_mode(std::uint32_t m) override{return native_.set_error_mode(m);}
    std::uint32_t begin_error_suppression() override {
        if(error_mode_)throw std::logic_error("Extra-library error suppression already active");
        error_mode_.emplace();return error_mode_->previous();
    }
    void end_error_suppression(std::uint32_t) override {error_mode_.reset();}
    std::uintptr_t load_library(const wchar_t* p) override {
        // Reserve ownership before native acquisition; a vector allocation
        // failure must not leave an untracked module and fallible rollback.
        const auto index=leases_.size();leases_.push_back(0);
        const auto module=native_.load_library(p);leases_[index]=module;
        if(!module&&index+1==leases_.size())leases_.pop_back();
        return module; // Track even if the source list later throws on allocation.
    }
    bool free_library(std::uintptr_t m) override{return native_.free_library(m);}
    int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* input,int count,char* output,int capacity,bool* used) override{return native_.wide_to_narrow(cp,flags,input,count,output,capacity,used);}
    std::uint32_t last_error() override{return native_.last_error();}
    void begin_load_log_0863e0() override{env_.debug_log("extra library load");}
    void append_log_path_and_flush(std::string_view path) override{env_.debug_log({"extra library load: ",path});}
    void message_038bc0(std::string,std::string text,std::uint32_t flags) override{MessageBoxW(nullptr,wide(env_.widen_0145d0(text)).c_str(),L"SURGE",flags);}
    [[noreturn]] void throw_runtime_00cd70(int n) override{throw std::system_error(n,std::generic_category());}
    [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t n) override{throw std::system_error(n,std::system_category());}
    void wait_steps_036880(int n) override{execution_.wait_steps_036880(n);}
    void finish_flag_036820() override{execution_.finish_flag_036820(ready_);}
};
class LicenseEffects final:public license::Effects {
    Environment& env_;
    Provider& provider_;
    file::Factory& files_;
public:
    LicenseEffects(Environment& e,Provider& p,file::Factory& f):env_(e),provider_(p),files_(f){}
    const char* online_auth_virtual_30(const char* id) override {
        return Provider::method<const char*(*)(void*,const char*)>(provider_.object,6)(reinterpret_cast<void*>(provider_.object),id);
    }
    license::FileStatus status_01ea20(const Path& p,std::uint32_t f) override {auto s=env_.status_01ea20(p,f);return {s.kind,static_cast<std::int32_t>(s.error)};}
    [[noreturn]] void filesystem_error_018130(std::string_view op,std::int32_t e,const Path& p) override{env_.filesystem_error_018130(op,e,p);}
    bool file_hash_matches_042d20(const Path&,std::string) override{throw std::logic_error("LicenseFiles owns hashing");}
    std::string load_token_02afa0(const Path&) override{throw std::logic_error("LicenseFiles owns token loading");}
    Path filesystem_widen_006d70(std::string_view v) override{return env_.filesystem_widen_006d70(v);}
    Path filename_00e120(const Path& p) override{return env_.filename(p);}
    Path parent_path_019820(const Path& p) override{return env_.parent_path_019820(p);}
    Path join_016b00(const Path& a,const Path& b) override{return env_.join_016b00(a,b);}
    std::string path_utf8_01e3f0(const Path& p) override{return env_.path_to_utf8_031ca0(p);}
    std::string current_time_041b60_null_true() override{return env_.ticket_time_041b60();}
    bool wine_detected_045010() override{return GetProcAddress(GetModuleHandleW(L"ntdll.dll"),"wine_get_version")!=nullptr;}
    void log_literal_006880(std::string_view text) override{env_.debug_log({"licensing status: ",text});}
    void log_string_006860(std::string_view text) override{env_.debug_log({"licensing diagnostic: ",text});}
    bool write_ticket_03c1d0(const Path& p,std::string_view v) override{return file::write_binary_checked(p,v,files_);}
    void dialog_038bc0(std::string_view,std::string_view body,std::uint32_t flags) override{MessageBoxW(nullptr,wide(env_.widen_0145d0(body)).c_str(),L"SURGE",flags);}
    [[noreturn]] void exit_05cdc0(std::int32_t code) override{ExitProcess(code);}
    [[noreturn]] void no_response_02bcd0_04d228() override{throw NoResponse();}
};
class HookErrors final:public native::Errors {
    Environment& env_;
public:
    explicit HookErrors(Environment& e):env_(e){}
    [[noreturn]] void lifecycle(native::LifecycleFault) override{throw std::runtime_error("Native hook binding lifecycle violation");}
    [[noreturn]] void callback_exception(host_init::Callback,std::exception_ptr) noexcept override{
        try{env_.debug_log("failure");}catch(...){}
        RaiseFailFastException(nullptr,nullptr,0);std::abort();
    }
};
class Hooks final:public native::WideOperations,
                  public native::OriginalEffects,
                  public host_init::Effects,
                  public platform::hook_engine::Quiescence {
    configuration::State& state_;
    Environment& env_;
    execution::Execution& execution_;
    platform::hook_engine::NativeApi engine_;
    native::Resolver resolver_;
    platform::WindowsCryptQueryHost crypt_host_;
    std::array<std::uintptr_t,11> originals_{};
    host_init::Values values_;
    std::vector<std::uintptr_t> targets_;
    registry::Handles client_,language_,fabricated_;
    std::int32_t ipv4_=16,ipv6_=28;
    std::uint32_t attempts_=0;
    HookErrors errors_;
    native::Context context_;
    std::unique_ptr<native::Binding> binding_;
    platform::hook_engine::Transaction transaction_{engine_,*this};
    bool failed_=false;
public:
    Hooks(configuration::State& s,Environment& e,execution::Execution& x)
        :OriginalEffects(originals_,*this,crypt_host_),state_(s),env_(e),execution_(x),errors_(e),
         context_{{values_,client_,language_,fabricated_,s.registry_spoof},{ipv4_,ipv6_,s.server_port,attempts_},*this,*this,*this,*this}{}
    void start() {
        binding_=std::make_unique<native::Binding>(context_,errors_);
        try {
        {std::lock_guard lock(context_.registry_gate);
         host_init::initialize_host_00e310({state_,values_,targets_,originals_},*this);}
        if(!transaction_.owns_engine()||failed_)throw std::runtime_error("Selected hook installation failed; owner must abort startup");
        const std::size_t expected=7+(state_.block_reruns?2:0)+(!state_.documents.empty()?2:0);
        if(targets_.size()!=expected)throw std::runtime_error("Required native hook staging incomplete");
        if(transaction_.activate()!=platform::hook_engine::status_code::ok)throw std::runtime_error("Native hook batch apply failed");
        }catch(...){
            // Keep this Hooks owner/context alive if rollback is incomplete.
            // Application abort releases startup gates before retrying teardown.
            (void)transaction_.rollback(std::chrono::milliseconds(0));throw;
        }
    }
    bool drain(std::chrono::milliseconds timeout) override {
        const auto until=std::chrono::steady_clock::now()+timeout;
        while(native::active_calls()!=0){if(std::chrono::steady_clock::now()>=until)return false;std::this_thread::sleep_for(std::chrono::milliseconds(1));}
        return external_threads_outside_module();
    }
    bool quiesce(std::chrono::milliseconds timeout) {
        if(!binding_)return true;
        if(!transaction_.finish(timeout))return false;
        targets_.clear();
        if(binding_){binding_->close();binding_.reset();}
        for(auto p:fabricated_)::operator delete(reinterpret_cast<void*>(p));fabricated_.clear();
        return true;
    }
    void crypt_observed(hooks::Selection) noexcept override {
#ifndef NDEBUG
        thread_local bool logging=false;if(logging)return;logging=true;
        // Diagnostics cannot escape this noexcept observer or prevent the
        // original API result from reaching the host. Always reset reentry.
        try{env_.debug_log("phase");}catch(...){}
        logging=false;
#endif
    }
    int compare_utf16(const void* a,const void* b) override{return _wcsicmp(static_cast<const wchar_t*>(a),static_cast<const wchar_t*>(b));}
    int copy_utf16(void* to,std::size_t n,const void* from) override{return wcscpy_s(static_cast<wchar_t*>(to),n,static_cast<const wchar_t*>(from));}
    std::string filesystem_narrow(std::u16string_view p) override{return env_.narrow_path(Path(p));}
    int crt_stricmp(const char* a,const char* b) override{return _stricmp(a,b);}
    int crt_strcpy_s(char* to,std::size_t n,const char* from) override{return strcpy_s(to,n,from);}
    std::uint64_t allocate_4_04b0bc() override{return reinterpret_cast<std::uintptr_t>(::operator new(4));}
    void free_04b0f8(std::uint64_t p) override{::operator delete(reinterpret_cast<void*>(p));}
    void wait_036880(std::int32_t n) override{
        const auto incoming=GetLastError();
        {callback_lock::Pause pause;execution_.wait_steps_036880(n);}
        SetLastError(incoming);
    }
    void wait_steps_036880(std::int32_t n) override{execution_.wait_steps_036880(n);}
    void log_literal_006880(std::uint64_t id) override{env_.debug_log_number("registry callback literal: ",static_cast<std::int64_t>(id));}
    void log_literal_006880(std::string_view text) override{env_.debug_log({"network callback: ",text});}
    void log_text_007990(std::string_view text) override{env_.debug_log({"registry value: ",text});}
    void finish_log_0088e0() override{env_.debug_log("registry callback complete");}
    void log_finish_0088e0() override{env_.debug_log("network callback complete");}
    void log_port_00ad70(std::uint16_t port) override{env_.debug_log_number("connect port: ",port);}
    void dialog_038bc0(std::string_view,std::string_view text,std::uint32_t flags) override{MessageBoxW(nullptr,wide(env_.widen_0145d0(text)).c_str(),L"SURGE",flags);}
    [[noreturn]] void exit_05cdc0(std::uint32_t n) override{ExitProcess(n);}
    Path filesystem_widen_006d70(std::string_view s) override{return env_.filesystem_widen_006d70(s);}
    std::uint64_t version_condition_mask(std::uint64_t m,std::uint32_t t,std::uint8_t c) override{return VerSetConditionMask(m,t,c);}
    bool verify_version(const host_init::VersionRequest& r,std::uint32_t types,std::uint64_t mask) override {
        OSVERSIONINFOEXW v{};v.dwOSVersionInfoSize=sizeof(v);v.dwMajorVersion=r.major;v.dwMinorVersion=r.minor;
        v.dwBuildNumber=r.build;v.dwPlatformId=r.platform;std::copy(r.service_pack_text.begin(),r.service_pack_text.end(),v.szCSDVersion);
        v.wServicePackMajor=r.service_pack_major;v.wServicePackMinor=r.service_pack_minor;v.wSuiteMask=r.suite;v.wProductType=r.product_type;
        return VerifyVersionInfoW(&v,types,mask)!=FALSE;
    }
    std::int32_t initialize_engine_046a20() override{return transaction_.begin();}
    std::uintptr_t get_module_handle(std::u16string_view s) override{return reinterpret_cast<std::uintptr_t>(GetModuleHandleW(wide(s).c_str()));}
    std::uintptr_t get_proc_address(std::uintptr_t m,std::string_view s) override {
        // Required hook11: a real SDK dllimport reference keeps CRYPT32 in the
        // loader dependency graph, as in the supplied Infrastructure/Hook reference path.
        // MinHook still writes the original trampoline into the owned slot.
        if(s=="CryptQueryObject"&&reinterpret_cast<HMODULE>(m)==GetModuleHandleW(L"crypt32.dll"))
            return reinterpret_cast<std::uintptr_t>(&::CryptQueryObject);
        return reinterpret_cast<std::uintptr_t>(GetProcAddress(reinterpret_cast<HMODULE>(m),std::string(s).c_str()));
    }
    std::int32_t create_hook_0466e0(std::uintptr_t target,host_init::Callback c,std::uintptr_t& original) override {
        void* result=nullptr;auto status=transaction_.create(reinterpret_cast<void*>(target),reinterpret_cast<void*>(resolver_.rebuilt_address(c)),&result);
        original=reinterpret_cast<std::uintptr_t>(result);if(status!=platform::hook_engine::status_code::ok)failed_=true;return status;
    }
    std::int32_t enable_hook_046a10(std::uintptr_t p) override{auto s=transaction_.queue(reinterpret_cast<void*>(p));if(s!=platform::hook_engine::status_code::ok)failed_=true;return s;}
    void log_text_006880(std::string_view s) override {if(s=="Hooking function "||s=="Enabling hook for ")failed_=true;env_.debug_log({"hook installation status: ",s});}
    void log_integer_00af90(std::int32_t code) override{env_.debug_log_number("hook status code: ",code);}
};
}

struct Application::Impl final:net::HandlerFactory,misc_handlers::NoResponse {
    Options options;
    configuration::State state=initial_state();
    const std::string sdk_version{protocol_identity::sdk_version};
    file::NativeFactory files;
    xml_reader_bridge::Backend xml;
    achievement_storage::Paths achievement_paths;
    achievement_runtime::HostClock clock;
    achievement_runtime::HostFormatter formatter;
    Environment environment{state,options,files,xml,achievement_paths,clock,formatter};
    execution::MutexState mutex{};
    std::uint8_t server_ready=0,provider_ready=0,extra_ready=0,extra_loaded=0,show_lsx=0,show_console=0,no_console=0;
    std::int64_t frequency=0;
    execution::WindowsPlatform platform{detail::thread_error};
    execution::Execution execution{mutex,{server_ready,provider_ready,extra_ready},frequency,platform};
    Provider provider{state,environment,files,execution,provider_ready};
    ExtraLibraries extras{environment,execution,extra_loaded};
    Events events;
    Mutex achievements_mutex;
    handlers::Profile profile;
    std::int32_t saved_avatar_size=0,attempted_files=0;
    std::uint8_t blocked=0;
    LicenseEffects license_base{environment,provider,files};
    file::NativeTextEnvironment hash_text;
    file::LicenseFiles license_files{license_base,files,hash_text};
    license::Handlers license{{environment.auth_code,provider.online,state.fake_auth,state.denuvo_token,
        state.dll_hash,state.exe_hash,state.content_id,state.denuvo_dll,state.executable_path,state.license_directory,
        state.game_attributes,attempted_files,blocked},license_files};
    std::mutex handler_state_mutex;
    std::unique_ptr<Hooks> hooks;
    std::unique_ptr<net::AsioService> service;
    std::atomic<bool> accepting{false};
    bool started=false,used=false;
    std::thread provider_worker,extra_worker;
    std::exception_ptr extra_failure;
    explicit Impl(Options o):options(std::move(o)) {
        execution::initialize_mutex_04a698(mutex,2);
        // Publish an immutable QPC frequency before callbacks can concurrently
        // enter the source polling code's otherwise unsynchronized cold cache.
        frequency=platform.query_performance_frequency();
        if(frequency<=0)throw std::runtime_error("Invalid Windows QPC frequency");
    }
    ~Impl(){try{stop();}catch(...){std::terminate();}}
    [[noreturn]] void throw_no_response_02bcd0_04d228() override{throw detail::NoResponse();}
    void error(std::string_view,std::exception_ptr) noexcept override{try{environment.debug_log("session operation failed");}catch(...){}}
    struct SessionHandler final:server::Handler,messages::Effects {
        Impl& app;
        std::uint8_t encrypted=0,unknown=0;
        crypto::Cipher cipher;
        crypto::CryptoFactory crypto;
        crypto::WindowsTimeSource time;
        crypto::Random random{time};
        dispatch::Router router;
        session_bridge::MessageHandler handler;
        SessionHandler(Impl& a):app(a),
            router({a.state.game_attributes,a.state.emulator_attributes,a.profile,a.provider.online,a.state.pretend_connected},
                achievement_runtime::bind_state(a.state,a.achievement_paths,a.provider.online,a.saved_avatar_size,a.achievements_mutex,a.events.content),
                {a.xml,a.environment,a.events.igo,a,a.environment,a.license}),
            handler({encrypted,unknown,a.show_lsx,a.sdk_version,cipher,a.events.messages},a.xml,crypto,random,*this){}
        void reset_02bc70() override{handler.reset_02bc70();}
        std::string initial_message_02d540() override{return handler.initial_message_02d540();}
        std::string process_message_033d70(std::string_view s) override{return handler.process_message_033d70(s);}
        server::PreparedResponse prepare_response(std::string_view s) override{return handler.prepare_response(s);}
        bool registered_handler(std::uint32_t rva,const messages::XmlNode& n,messages::XmlPrinter& out) override {
            // Protect the shared mutable source maps/counters only. XML parsing,
            // cipher work and socket completion remain per-session/concurrent.
            std::lock_guard lock(app.handler_state_mutex);return router.registered_handler(rva,n,out);
        }
        int atoi_05ee08(std::string_view s) override{return app.environment.atoi_05ee08(s);}
        [[noreturn]] void invalid_message_02bd30_04d228() override{throw std::invalid_argument("RE invalid LSX message");}
        [[noreturn]] void thread_error_04a648(int c) override{thread_error(c);}
        void log_received(std::string_view input) override{app.environment.debug_log({"message received: ",input});}
        void log_response(std::string_view response) override{app.environment.debug_log({"message response: ",response});}
        void log_event(std::string_view event) override{app.environment.debug_log({"message event: ",event});}
        void log_bad_xml(std::string_view error,std::string_view input) override{app.environment.debug_log({"invalid XML: ",error,"; input: ",input});}
        void log_decryption_failure(std::string_view input) override{app.environment.debug_log({"decryption failure: ",input});}
    };
    std::unique_ptr<server::Handler> create() override {
        if(!accepting.load(std::memory_order_acquire))throw std::runtime_error("Application is initializing or stopping");
        return std::make_unique<SessionHandler>(*this);
    }
    void start() {
        if(used)throw std::logic_error("Application start is single-use");used=true;
        try {
            startup::NativeInputPlatform input;Console console;
            startup::StateReferences refs{state.language_from_arguments,state.registry_spoof,show_lsx,show_console,no_console,state.game_attributes};
            startup::setup_environment_019910(refs,input);
            startup::setup_console_038760(refs,console);
            environment.configure();
            extras.configure(state);
            // Initialize shared source queue before publishing sessions; its DOWN
            // seed is the same one the first source message getter would create.
            (void)event_queue::mutex_02c620(events.context);
            (void)event_queue::queue_02c400(events.context);
            service=std::make_unique<net::AsioService>(*this,execution,state.server_port,server_ready,
                net::SessionPolicy{},true); // Bind first; leave accept callbacks inactive until ready.
            service->start();
            if(options.install_hooks){hooks=std::make_unique<Hooks>(state,environment,execution);hooks->start();}
            std::promise<std::exception_ptr> completed;
            auto provider_result=completed.get_future().share();
            // Preserve the source worker/main-extra overlap and dependency
            // order using standard-CRT threads owned until stop/join.
            provider_worker=std::thread([this,done=std::move(completed)]() mutable {
                try{provider.load();done.set_value(nullptr);}catch(...){done.set_value(std::current_exception());}
            });
            extras.load(true);
            extra_worker=std::thread([this,provider_result] {
                if(provider_result.get())return; // Failed predecessor must not leave an infinite wait_steps loop.
                try{extras.load(false);}
                catch(...){extra_failure=std::current_exception();}
            });
            provider_worker.join();extra_worker.join();
            if(auto failure=provider_result.get())std::rethrow_exception(failure);
            if(extra_failure)std::rethrow_exception(extra_failure);
            profile={state.username,state.persona_id,state.user_id,state.country,state.currency,state.subscriber_level,
                state.is_subscriber,state.is_under_age,state.is_steam_subscriber};
            if(!extra_loaded)throw std::runtime_error("Extra library worker did not complete");
            accepting.store(true,std::memory_order_release);
            // A connect callback waiting for all three source phases must not
            // escape while the handler factory still rejects new sessions.
            execution.finish_flag_036820(extra_ready);
            service->resume_accepts(); // IO executor arms accepts only after profile publication.
            started=true;environment.debug_log("application ready");
        }catch(...){stop();throw;}
    }
    void stop() {
        accepting.store(false,std::memory_order_release);
        if(used&&!started) {
            // Abort releases callbacks waiting for a later source phase before
            // hooks/workers are drained. These private gates do not publish a
            // successful startup: start throws and the DLL API reports failure.
            execution.finish_flag_036820(server_ready);
            execution.finish_flag_036820(provider_ready);
            execution.finish_flag_036820(extra_ready);
        }
        if(provider_worker.joinable())provider_worker.join();
        if(extra_worker.joinable())extra_worker.join();
        if(service){service->stop();service.reset();}
        files.recover_atomic(); // Retain Application ownership if rollback remains incomplete.
        if(hooks){
            // The DLL worker retains this owner and binding on failure. Let
            // the next shutdown request retry instead of spinning forever.
            if(!hooks->quiesce(std::chrono::milliseconds(100)))
                throw std::runtime_error("Native hook cleanup incomplete");
            hooks.reset();
        }
        extras.close();
        provider.close();started=false;
    }
};
Application::Application(Options o):impl_(std::make_unique<Impl>(std::move(o))){}
Application::~Application()=default;
void Application::start(){impl_->start();}
void Application::stop(){impl_->stop();}
const configuration::State& Application::configuration() const{return impl_->state;}
}
