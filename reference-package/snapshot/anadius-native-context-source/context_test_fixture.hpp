#pragma once
#include "context.hpp"
#include "../anadius-host-platform-source/platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

namespace b15_test {
using namespace anadius;namespace nc=native_context;namespace n=native_recovered;namespace b=bootstrap_backend;
inline unsigned checks=0;
inline void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
template<class E,class F>void rejects(F&& f,const char* label){bool hit=false;try{f();}catch(const E&){hit=true;}require(hit,label);}
struct Life {n::LifecycleFault fault;};
struct Errors final:n::Errors {
    [[noreturn]] void lifecycle(n::LifecycleFault fault)override{throw Life{fault};}
    [[noreturn]] void callback_exception(n::Callback,std::exception_ptr)noexcept override{std::abort();}
};
inline Errors permanent_errors;
struct Probe {
    configuration::State* config=nullptr;b::RuntimeSeed* runtime=nullptr;
    execution_recovered::Execution* execution=nullptr;nc::CallbackSeed* callbacks=nullptr;
    std::atomic<unsigned> closes=0,connects=0,processes=0,waits=0,destroyed=0;
    std::mutex gate;std::condition_variable cv;bool block=false,entered=false,release=false;
};
inline Probe* active_probe=nullptr;
inline LSTATUS WINAPI original_close(HKEY){
    auto& p=*active_probe;++p.closes;
    {std::unique_lock lock(p.gate);if(p.block){p.entered=true;p.cv.notify_all();p.cv.wait(lock,[&]{return p.release;});}}
    ::SetLastError(0x1234);return 0;
}
inline int WSAAPI original_connect(SOCKET,const sockaddr*,int){++active_probe->connects;return -10035;}
inline BOOL WINAPI original_process(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION out){++active_probe->processes;if(out)out->dwProcessId=0x12345678;return 7;}
inline LSTATUS WINAPI original_open_a(HKEY,LPCSTR,DWORD,REGSAM,PHKEY out){if(out)*out=reinterpret_cast<HKEY>(0x789);return 0;}
inline LSTATUS WINAPI original_open_w(HKEY,LPCWSTR,DWORD,REGSAM,PHKEY out){if(out)*out=reinterpret_cast<HKEY>(0x78a);return 0;}
inline LSTATUS WINAPI original_query_a(HKEY,LPCSTR,LPDWORD,LPDWORD,LPBYTE,LPDWORD){return 0;}
inline LSTATUS WINAPI original_query_w(HKEY,LPCWSTR,LPDWORD,LPDWORD,LPBYTE,LPDWORD){return 0;}
inline BOOL WINAPI original_process_w(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION){return 7;}
inline HRESULT WINAPI original_folder_a(HWND,int,HANDLE,DWORD,LPSTR){return 0;}
inline HRESULT WINAPI original_folder_w(HWND,int,HANDLE,DWORD,LPWSTR){return 0;}
inline std::array<std::uintptr_t,10> originals(){return {
    reinterpret_cast<std::uintptr_t>(&original_connect),reinterpret_cast<std::uintptr_t>(&original_open_a),reinterpret_cast<std::uintptr_t>(&original_open_w),reinterpret_cast<std::uintptr_t>(&original_close),
    reinterpret_cast<std::uintptr_t>(&original_query_a),reinterpret_cast<std::uintptr_t>(&original_query_w),reinterpret_cast<std::uintptr_t>(&original_process),reinterpret_cast<std::uintptr_t>(&original_process_w),
    reinterpret_cast<std::uintptr_t>(&original_folder_a),reinterpret_cast<std::uintptr_t>(&original_folder_w)};}
struct Wide final:n::WideOperations {
    static std::u16string read(const void* p){std::u16string result;for(std::size_t i=0;;++i){char16_t value;std::memcpy(&value,static_cast<const std::byte*>(p)+i*2,2);if(!value)return result;result+=value;}}
    int compare_utf16(const void* a,const void* b)override{auto x=read(a),y=read(b);for(auto& c:x)if(c>=u'A'&&c<=u'Z')c+=32;for(auto& c:y)if(c>=u'A'&&c<=u'Z')c+=32;return x.compare(y);}
    int copy_utf16(void* out,std::size_t count,const void* input)override{const auto text=read(input);if(!out||text.size()+1>count)return 34;std::memcpy(out,text.c_str(),(text.size()+1)*2);return 0;}
};
inline Wide wide;
class Policy final:public n::OriginalEffects {
    nc::PolicyView view_;std::shared_ptr<Probe> probe_;
public:
    Policy(nc::PolicyView view,std::shared_ptr<Probe> probe,const std::array<std::uintptr_t,10>* wrong=nullptr)
        :OriginalEffects(wrong?*wrong:view.backend.runtime.originals,wide),view_(view),probe_(std::move(probe)){
        probe_->config=&view.backend.configuration;probe_->runtime=&view.backend.runtime;probe_->execution=&view.backend.execution;probe_->callbacks=&view.callbacks;
    }
    ~Policy()override{++probe_->destroyed;}
    std::string filesystem_narrow(std::u16string_view value)override{return {value.begin(),value.end()};}
    int crt_stricmp(const char* a,const char* b)override{std::string x(a),y(b);for(auto& c:x)if(c>='A'&&c<='Z')c+=32;for(auto& c:y)if(c>='A'&&c<='Z')c+=32;return x.compare(y);}
    int crt_strcpy_s(char* out,std::size_t count,const char* in)override{const auto size=std::strlen(in)+1;if(!out||size>count)return 34;std::memcpy(out,in,size);return 0;}
    registry_recovered::Handle allocate_4_04b0bc()override{throw std::logic_error("allocation not selected by this fixture");}
    void free_04b0f8(registry_recovered::Handle)override{throw std::logic_error("free not selected by this fixture");}
    void wait_036880(std::int32_t count)override{++probe_->waits;view_.backend.execution.wait_steps_036880(count);}
    void wait_steps_036880(std::int32_t count)override{++probe_->waits;view_.backend.execution.wait_steps_036880(count);}
    void log_literal_006880(std::uint64_t)override{}
    void log_text_007990(std::string_view)override{}
    void finish_log_0088e0()override{}
    void log_literal_006880(std::string_view)override{}
    void log_port_00ad70(std::uint16_t)override{}
    void log_finish_0088e0()override{}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{throw std::logic_error("unexpected fixture UI");}
    [[noreturn]] void exit_05cdc0(std::uint32_t)override{throw std::logic_error("unexpected fixture exit");}
};
inline nc::CallbackSeed seed(){return {{},{},{},16,28,0};}
struct Driver {
    b::RuntimeSeed runtime{};configuration::State configuration{};b2_test::Events events;
    b4_test::Execution platform{events};execution_recovered::Execution execution;
    std::shared_ptr<Probe> probe=std::make_shared<Probe>();
    Driver():execution(runtime.execution_mutex,{runtime.server_ready,runtime.online_ready,runtime.libraries_ready},runtime.frequency_cache,platform){
        execution_recovered::initialize_mutex_04a698(runtime.execution_mutex,2);runtime.server_ready=runtime.online_ready=runtime.libraries_ready=1;runtime.frequency_cache=10000000;
        runtime.originals=originals();
        configuration.server_port=6000;configuration.registry_spoof=1;active_probe=probe.get();
    }
    ~Driver(){active_probe=nullptr;}
    b::CallbackStateView view(){return {configuration,runtime,execution};}
    nc::PolicyFactory policies(){return [p=probe](nc::PolicyView view){return std::make_unique<Policy>(view,p);};}
};
} // namespace b15_test
