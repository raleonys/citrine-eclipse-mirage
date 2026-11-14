// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <windows.h>
#include <shlobj.h>
#include <wincrypt.h>
#include <array>
#include <exception>
#include <mutex>
#include "laureondo/platform/registry.hpp"
#include "laureondo/platform/hook_host_callbacks.hpp"
#include "laureondo/platform/hook_connect.hpp"
#include "laureondo/platform/hook_engine_adapters.hpp"
#include "laureondo/platform/crypt_query.hpp"

namespace laureondo::native {
using Callback=host_init::Callback;
enum class LifecycleFault { AlreadyBound, Busy, WrongOwner };
class Errors {
public:
    virtual ~Errors()=default;
    [[noreturn]] virtual void lifecycle(LifecycleFault)=0;
    // Never return or throw: policy may terminate/record/fail-fast; it must not
    // translate an unknown exception into a successful Windows API result.
    [[noreturn]] virtual void callback_exception(Callback,std::exception_ptr) noexcept=0;
};
class CryptQueryEffects {
public:
    virtual ~CryptQueryEffects()=default;
    virtual BOOL crypt_query(DWORD,const void*,DWORD,DWORD,DWORD,DWORD*,DWORD*,
                             DWORD*,HCERTSTORE*,HCRYPTMSG*,const void**)=0;
};
struct Context {
    registry::StateRefs registry;
    network_hook::StateRefs network;
    registry::Effects& registry_effects;
    host_callbacks::Effects& host_effects;
    network_hook::Effects& network_effects;
    CryptQueryEffects& crypt_effects;
    // Hook reference callback synchronization excludes connect and crypt. Recursive
    // permits native reentry. Tracked callback acquisitions
    // pause only across registry startup waits; the binding lease stays live.
    // This is not an application lock. The attempts counter has its own narrow
    // synchronization in OriginalEffects; no lock spans a network wait/connect.
    std::recursive_mutex registry_gate;
};
// Explicit per-image binding. Errors must outlive its Binding object and every
// admitted callback, including nonreturning exception reporting under its lease.
// Context/state/effects stay alive and stable until close succeeds. An unbound
// entry terminates without borrowing a retired policy; this does not extend the
// DLL image lifetime or make arbitrary late entry safe after image unload.
// Close is nonblocking and rejects in-flight calls; first disable hooks, obtain
// external callback quiescence, then close. It does not disable hooks itself.
class Binding final {
public:
    Binding(Context&,Errors&);
    ~Binding() noexcept;
    Binding(const Binding&)=delete;
    Binding& operator=(const Binding&)=delete;
    void close();
private:
    Context* context_;
    Errors& errors_;
};
bool is_bound();
std::size_t active_calls();
class Resolver final : public laureondo::platform::hook_engine::CallbackResolver {
public:
    std::uintptr_t rebuilt_address(Callback) override;
};

// Native callback W pointers are opaque UTF-16 storage, not char16_t objects.
// These operations MUST read/write code units bytewise or use a proven ABI
// adapter. Direct char16_t dereference of wchar_t backing is not permitted.
class WideOperations {
public:
    virtual ~WideOperations()=default;
    virtual int compare_utf16(const void*,const void*)=0;
    virtual int copy_utf16(void* destination,std::size_t characters,const void* source)=0;
};
// Loads original SDK-typed trampoline pointers from the same eleven slots populated
// by HostInitEffects. All allocation/log/wait/narrow/copy/UI effects stay pure.
// This class is intentionally abstract; no default success backend exists.
class OriginalEffects : public registry::Effects,
                        public host_callbacks::Effects,
                        public network_hook::Effects,
                        public CryptQueryEffects {
public:
    OriginalEffects(const std::array<std::uintptr_t,host_init::callback_count>& originals,
                    WideOperations& wide,laureondo::platform::CryptQueryHost& crypt_host)
        : originals_(originals),wide_(wide),crypt_host_(crypt_host) {}
    BOOL crypt_query(DWORD,const void*,DWORD,DWORD,DWORD,DWORD*,DWORD*,DWORD*,
                     HCERTSTORE*,HCRYPTMSG*,const void**) final;
    virtual void crypt_observed(hooks::Selection) noexcept=0;
    registry::Status original_open_a(const registry::OpenA&) final;
    registry::Status original_open_w(const registry::OpenW&) final;
    registry::Status original_query_a(const registry::QueryA&) final;
    registry::Status original_query_w(const registry::QueryW&) final;
    registry::Status original_close(registry::Handle) final;
    std::int32_t original_create_process_a(const host_callbacks::ProcessCall<char>&) final;
    std::int32_t original_create_process_w(const host_callbacks::ProcessCall<char16_t>&) final;
    std::uint32_t original_folder_a(const host_callbacks::FolderCall<char>&) final;
    std::uint32_t original_folder_w(const host_callbacks::FolderCall<char16_t>&) final;
    std::int32_t original_connect_0a3838(network_hook::Socket,std::span<const std::byte>,std::int32_t) final;
    std::uint32_t increment_attempts(std::uint32_t&) final;
    int crt_wcsicmp(const char16_t*,const char16_t*) final;
    int crt_wcscpy_s(char16_t*,std::size_t,const char16_t*) final;
private:
    const std::array<std::uintptr_t,host_init::callback_count>& originals_;
    WideOperations& wide_;
    laureondo::platform::CryptQueryHost& crypt_host_;
    std::mutex attempts_gate_;
    template<class Fn> Fn original(Callback c)const {
        const auto address=originals_.at(static_cast<std::size_t>(c));
        if(!address)throw std::logic_error("required SDK trampoline is unbound");
        return reinterpret_cast<Fn>(address);
    }
};

int WSAAPI connect_entry(SOCKET,const sockaddr*,int) noexcept;
LSTATUS WINAPI reg_open_a_entry(HKEY,LPCSTR,DWORD,REGSAM,PHKEY) noexcept;
LSTATUS WINAPI reg_open_w_entry(HKEY,LPCWSTR,DWORD,REGSAM,PHKEY) noexcept;
LSTATUS WINAPI reg_close_entry(HKEY) noexcept;
LSTATUS WINAPI reg_query_a_entry(HKEY,LPCSTR,LPDWORD,LPDWORD,LPBYTE,LPDWORD) noexcept;
LSTATUS WINAPI reg_query_w_entry(HKEY,LPCWSTR,LPDWORD,LPDWORD,LPBYTE,LPDWORD) noexcept;
BOOL WINAPI process_a_entry(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION) noexcept;
BOOL WINAPI process_w_entry(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION) noexcept;
HRESULT WINAPI folder_a_entry(HWND,int,HANDLE,DWORD,LPSTR) noexcept;
HRESULT WINAPI folder_w_entry(HWND,int,HANDLE,DWORD,LPWSTR) noexcept;
BOOL WINAPI crypt_query_entry(DWORD,const void*,DWORD,DWORD,DWORD,DWORD*,DWORD*,
                              DWORD*,HCERTSTORE*,HCRYPTMSG*,const void**) noexcept;
}
