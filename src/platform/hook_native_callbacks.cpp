// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/platform/hook_native_callbacks.hpp"
#include "preloader_l/platform/callback_lock.hpp"
#include <array>
#include <bit>
#include <mutex>
#include <atomic>
#include <type_traits>
#include <utility>

namespace preloader_l::native {
namespace {
std::mutex gate;
Context* current{};
std::size_t calls{};
std::atomic<Errors*> last_errors{};
struct Lease {
    Context* context{};
    Errors* errors{};
    Lease(){std::lock_guard lock(gate);if(!current)throw std::logic_error("native callback without Context binding");context=current;errors=last_errors.load(std::memory_order_acquire);++calls;}
    void release(){if(context){const auto error=GetLastError();{std::lock_guard lock(gate);--calls;context=nullptr;}SetLastError(error);}}
    ~Lease(){release();}
};
template<class F> auto invoke(Callback callback,F&& f) noexcept -> decltype(f(std::declval<Context&>())) {
    const auto incoming=GetLastError();
    Errors* invocation_errors=last_errors.load(std::memory_order_acquire);
    try {
        Lease lease;
        invocation_errors=lease.errors; // retain this binding's policy after release
        const auto result=[&]{
            callback_lock::Guard state_lock(lease.context->registry_gate,
                callback!=Callback::connect_00fd00&&callback!=Callback::crypt_query_001264);
            SetLastError(incoming); // binding synchronization adds no API error state
            auto value=f(*lease.context);
            return std::pair{value,GetLastError()};
        }();
        lease.release();
        SetLastError(result.second);
        return result.first;
    } catch(...) {
        if(invocation_errors)
            invocation_errors->callback_exception(callback,std::current_exception());
        std::terminate(); // explicit out-of-contract entry before any binding
    }
}
std::uintptr_t address(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
LSTATUS status(std::uint32_t n){return std::bit_cast<LSTATUS>(n);}
}
Binding::Binding(Context& context,Errors& errors):context_(&context),errors_(errors){
    bool busy;
    {std::lock_guard lock(gate);busy=current!=nullptr;if(!busy){last_errors.store(&errors,std::memory_order_release);current=&context;}}
    if(busy){context_=nullptr;errors_.lifecycle(LifecycleFault::AlreadyBound);}
}
void Binding::close(){
    if(!context_)return;
    LifecycleFault failure{};bool rejected=false;
    {std::lock_guard lock(gate);
     if(current!=context_){failure=LifecycleFault::WrongOwner;rejected=true;}
     else if(calls){failure=LifecycleFault::Busy;rejected=true;}
     else{current=nullptr;context_=nullptr;}}
    if(rejected)errors_.lifecycle(failure);
}
Binding::~Binding() noexcept{close();}
bool is_bound(){std::lock_guard lock(gate);return current!=nullptr;}
std::size_t active_calls(){std::lock_guard lock(gate);return calls;}

int WSAAPI connect_entry(SOCKET socket,const sockaddr* pointer,int length) noexcept {
    return invoke(Callback::connect_00fd00,[&](Context& c){
        const auto* bytes=reinterpret_cast<const std::byte*>(pointer);
        // The original sees the caller's *opaque* pointer, even when unreadable.
        // A zero-length span never grants the hook permission to dereference it.
        const std::span<const std::byte> original_address{bytes,std::size_t{0}};
        const bool recognized=length==c.network.ipv4_length_0a10d0||
                              length==c.network.ipv6_length_0a10d4;
        if(!recognized || !pointer || length<=0 || length>28)
            return c.network_effects.original_connect_0a3838(socket,original_address,length);
        std::array<std::byte,28> observed{};
        SIZE_T copied=0;
        const auto incoming=GetLastError();
        const BOOL complete=ReadProcessMemory(GetCurrentProcess(),pointer,observed.data(),
            static_cast<SIZE_T>(length),&copied);
        SetLastError(incoming); // probing is not the SDK call and must not alter its error state
        if(!complete || copied!=static_cast<SIZE_T>(length))
            return c.network_effects.original_connect_0a3838(socket,original_address,length);
        // Only the bounded local observation is inspected. Non-redirecting calls
        // still forward pointer/length unchanged; redirection uses a full copy.
        return network_hook::connect_00fd00_inspected(c.network,c.network_effects,
            socket,original_address,std::span<const std::byte>{observed}.first(static_cast<std::size_t>(length)),length);
    });
}
LSTATUS WINAPI reg_open_a_entry(HKEY root,LPCSTR key,DWORD options,REGSAM access,PHKEY result) noexcept {
    return invoke(Callback::reg_open_a_00f800,[&](Context& c){return status(registry::open_a_00f800(c.registry,c.registry_effects,
        {address(root),key,options,access,registry::HandleCell::from_raw(result)}));});
}
LSTATUS WINAPI reg_open_w_entry(HKEY root,LPCWSTR key,DWORD options,REGSAM access,PHKEY result) noexcept {
    return invoke(Callback::reg_open_w_00f960,[&](Context& c){return status(registry::open_w_00f960(c.registry,c.registry_effects,
        {address(root),reinterpret_cast<const char16_t*>(key),options,access,registry::HandleCell::from_raw(result)}));});
}
LSTATUS WINAPI reg_close_entry(HKEY key) noexcept {
    return invoke(Callback::reg_close_00f5b0,[&](Context& c){return status(registry::close_00f5b0(c.registry,c.registry_effects,address(key)));});
}
LSTATUS WINAPI reg_query_a_entry(HKEY key,LPCSTR name,LPDWORD reserved,LPDWORD type,LPBYTE data,LPDWORD size) noexcept {
    return invoke(Callback::reg_query_a_00fac0,[&](Context& c){return status(registry::query_a_00fac0(c.registry,c.registry_effects,
        {address(key),name,reserved,registry::DwordCell::from_raw(type),data,registry::DwordCell::from_raw(size)}));});
}
LSTATUS WINAPI reg_query_w_entry(HKEY key,LPCWSTR name,LPDWORD reserved,LPDWORD type,LPBYTE data,LPDWORD size) noexcept {
    return invoke(Callback::reg_query_w_00fb60,[&](Context& c){return status(registry::query_w_00fb60(c.registry,c.registry_effects,
        {address(key),reinterpret_cast<const char16_t*>(name),reserved,registry::DwordCell::from_raw(type),data,registry::DwordCell::from_raw(size)}));});
}
BOOL WINAPI process_a_entry(LPCSTR app,LPSTR cmd,LPSECURITY_ATTRIBUTES p,LPSECURITY_ATTRIBUTES t,BOOL inherit,DWORD flags,LPVOID env,LPCSTR cwd,LPSTARTUPINFOA start,LPPROCESS_INFORMATION info) noexcept {
    return invoke(Callback::create_process_a_00f3f0,[&](Context& c){return host_callbacks::create_process_a_00f3f0(c.registry.values,c.host_effects,
        {app,cmd,address(p),address(t),inherit,flags,address(env),cwd,address(start),address(info)});});
}
BOOL WINAPI process_w_entry(LPCWSTR app,LPWSTR cmd,LPSECURITY_ATTRIBUTES p,LPSECURITY_ATTRIBUTES t,BOOL inherit,DWORD flags,LPVOID env,LPCWSTR cwd,LPSTARTUPINFOW start,LPPROCESS_INFORMATION info) noexcept {
    return invoke(Callback::create_process_w_00f4d0,[&](Context& c){return host_callbacks::create_process_w_00f4d0(c.registry.values,c.host_effects,
        {reinterpret_cast<const char16_t*>(app),reinterpret_cast<char16_t*>(cmd),address(p),address(t),inherit,flags,address(env),reinterpret_cast<const char16_t*>(cwd),address(start),address(info)});});
}
HRESULT WINAPI folder_a_entry(HWND window,int id,HANDLE token,DWORD flags,LPSTR path) noexcept {
    return invoke(Callback::folder_a_00fc00,[&](Context& c){return std::bit_cast<HRESULT>(host_callbacks::folder_a_00fc00(c.registry.values,c.host_effects,{address(window),id,address(token),flags,path}));});
}
HRESULT WINAPI folder_w_entry(HWND window,int id,HANDLE token,DWORD flags,LPWSTR path) noexcept {
    return invoke(Callback::folder_w_00fc80,[&](Context& c){return std::bit_cast<HRESULT>(host_callbacks::folder_w_00fc80(c.registry.values,c.host_effects,{address(window),id,address(token),flags,reinterpret_cast<char16_t*>(path)}));});
}
BOOL WINAPI crypt_query_entry(DWORD type,const void* object,DWORD content,DWORD format,
    DWORD flags,DWORD* encoding,DWORD* content_type,DWORD* format_type,
    HCERTSTORE* store,HCRYPTMSG* message,const void** context) noexcept {
    return invoke(Callback::crypt_query_001264,[&](Context& c){
        return c.crypt_effects.crypt_query(type,object,content,format,flags,
            encoding,content_type,format_type,store,message,context);
    });
}
std::uintptr_t Resolver::rebuilt_address(Callback c){
    switch(c){
    case Callback::connect_00fd00:return reinterpret_cast<std::uintptr_t>(&connect_entry);
    case Callback::reg_open_a_00f800:return reinterpret_cast<std::uintptr_t>(&reg_open_a_entry);
    case Callback::reg_open_w_00f960:return reinterpret_cast<std::uintptr_t>(&reg_open_w_entry);
    case Callback::reg_close_00f5b0:return reinterpret_cast<std::uintptr_t>(&reg_close_entry);
    case Callback::reg_query_a_00fac0:return reinterpret_cast<std::uintptr_t>(&reg_query_a_entry);
    case Callback::reg_query_w_00fb60:return reinterpret_cast<std::uintptr_t>(&reg_query_w_entry);
    case Callback::create_process_a_00f3f0:return reinterpret_cast<std::uintptr_t>(&process_a_entry);
    case Callback::create_process_w_00f4d0:return reinterpret_cast<std::uintptr_t>(&process_w_entry);
    case Callback::folder_a_00fc00:return reinterpret_cast<std::uintptr_t>(&folder_a_entry);
    case Callback::folder_w_00fc80:return reinterpret_cast<std::uintptr_t>(&folder_w_entry);
    case Callback::crypt_query_001264:return reinterpret_cast<std::uintptr_t>(&crypt_query_entry);
    }
    throw std::out_of_range("invalid callback enum");
}
static_assert(std::is_convertible_v<decltype(&connect_entry),decltype(&::connect)>);
static_assert(std::is_convertible_v<decltype(&reg_open_a_entry),decltype(&::RegOpenKeyExA)>);
static_assert(std::is_convertible_v<decltype(&reg_open_w_entry),decltype(&::RegOpenKeyExW)>);
static_assert(std::is_convertible_v<decltype(&reg_close_entry),decltype(&::RegCloseKey)>);
static_assert(std::is_convertible_v<decltype(&reg_query_a_entry),decltype(&::RegQueryValueExA)>);
static_assert(std::is_convertible_v<decltype(&reg_query_w_entry),decltype(&::RegQueryValueExW)>);
static_assert(std::is_convertible_v<decltype(&process_a_entry),decltype(&::CreateProcessA)>);
static_assert(std::is_convertible_v<decltype(&process_w_entry),decltype(&::CreateProcessW)>);
static_assert(std::is_convertible_v<decltype(&folder_a_entry),decltype(&::SHGetFolderPathA)>);
static_assert(std::is_convertible_v<decltype(&folder_w_entry),decltype(&::SHGetFolderPathW)>);
static_assert(std::is_convertible_v<decltype(&crypt_query_entry),decltype(&::CryptQueryObject)>);
static_assert(sizeof(DWORD)==4&&sizeof(HKEY)==8&&sizeof(wchar_t)==2&&sizeof(SOCKET)==8);
}
