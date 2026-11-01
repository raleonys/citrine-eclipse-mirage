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
#include <array>
#include <exception>
#include <memory>
#include "../anadius-registry-source/registry.hpp"
#include "../anadius-host-callbacks-source/callbacks.hpp"
#include "../anadius-network-hook-source/connect.hpp"
#include "../hook-engine-source/adapters.hpp"
#include "../anadius-original-checkpoint-source/checkpoint.hpp"

namespace anadius::native_recovered {
using Callback=host_init_recovered::Callback;
enum class LifecycleFault { AlreadyBound, Busy, WrongOwner };
class Errors {
public:
    virtual ~Errors()=default;
    [[noreturn]] virtual void lifecycle(LifecycleFault)=0;
    // Never return or throw: policy may terminate/record/fail-fast; it must not
    // translate an unknown exception into a successful Windows API result.
    [[noreturn]] virtual void callback_exception(Callback,std::exception_ptr) noexcept=0;
};
// Optional source admission policy. enter must either return with one owned
// admission or throw without one. leave is nonthrowing and releases that entry.
// The native wrapper calls it before any callback body/state/buffer access and
// releases it BEFORE the native binding lease. Stable for Context lifetime.
class Admission {
public:
    virtual ~Admission()=default;
    virtual void enter(Callback)=0;
    virtual void leave(Callback) noexcept=0;
    virtual void host_setup_begin() {}
    virtual void host_setup_complete() {}
    // Source lifecycle checkpoint BEFORE worker completion/exit-hook cleanup.
    // Successful return still does not prove external hook quiescence.
    virtual bool pause_before_cleanup() {return true;}
};
struct Context {
    registry_recovered::StateRefs registry;
    network_hook_recovered::StateRefs network;
    registry_recovered::Effects& registry_effects;
    host_callbacks_recovered::Effects& host_effects;
    network_hook_recovered::Effects& network_effects;
    Admission* admission=nullptr; // null preserves existing source path
};
// Explicit per-image binding. Errors must outlive all bindings and native entry
// attempts. Context/state/effects stay alive and stable until close succeeds.
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
class Resolver final : public recovered_hooks::CallbackResolver {
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
using OriginalSlots=std::array<std::uintptr_t,10>;
// Optional source owner/publication policy; does not prove hook success or
// executable/native ABI provenance. Caller supplies actual code lifetime owner.
class OriginalProvider {
public:
    virtual ~OriginalProvider()=default;
    virtual const OriginalSlots* slots_identity() const noexcept=0;
    virtual std::uintptr_t load(Callback)=0;
    virtual void setup_begin()=0;
    virtual void setup_complete()=0;
    virtual void stop() noexcept=0;
};
// Loads original SDK-typed trampoline pointers from the same ten slots populated
// by HostInitEffects. All allocation/log/wait/narrow/copy/UI effects stay pure.
// This class is intentionally abstract; no default success backend exists.
class OriginalEffects : public registry_recovered::Effects,
                        public host_callbacks_recovered::Effects,
                        public network_hook_recovered::Effects {
public:
    OriginalEffects(const OriginalSlots& originals,WideOperations& wide,std::shared_ptr<OriginalProvider> provider={},
        std::shared_ptr<original_checkpoint::Boundary> checkpoint={})
        : originals_(originals),wide_(wide),provider_(std::move(provider)),checkpoint_(std::move(checkpoint)) {
        if(provider_&&provider_->slots_identity()!=&originals_)
            throw std::invalid_argument("Original provider requires exact runtime slot identity");
    }
    ~OriginalEffects() override;
    const std::array<std::uintptr_t,10>* original_slots_identity() const noexcept{return &originals_;}
    OriginalProvider* original_provider() const noexcept{return provider_.get();}
    registry_recovered::Status original_open_a(const registry_recovered::OpenA&) final;
    registry_recovered::Status original_open_w(const registry_recovered::OpenW&) final;
    registry_recovered::Status original_query_a(const registry_recovered::QueryA&) final;
    registry_recovered::Status original_query_w(const registry_recovered::QueryW&) final;
    registry_recovered::Status original_close(registry_recovered::Handle) final;
    std::int32_t original_create_process_a(const host_callbacks_recovered::ProcessCall<char>&) final;
    std::int32_t original_create_process_w(const host_callbacks_recovered::ProcessCall<char16_t>&) final;
    std::uint32_t original_folder_a(const host_callbacks_recovered::FolderCall<char>&) final;
    std::uint32_t original_folder_w(const host_callbacks_recovered::FolderCall<char16_t>&) final;
    std::int32_t original_connect_0a3838(network_hook_recovered::Socket,std::span<std::byte>,std::int32_t) final;
    int crt_wcsicmp(const char16_t*,const char16_t*) final;
    int crt_wcscpy_s(char16_t*,std::size_t,const char16_t*) final;
private:
    const std::array<std::uintptr_t,10>& originals_;
    WideOperations& wide_;
    std::shared_ptr<OriginalProvider> provider_;
    std::shared_ptr<original_checkpoint::Boundary> checkpoint_;
    template<class Fn> Fn original(Callback c)const {
        const auto address=provider_?provider_->load(c):originals_.at(static_cast<std::size_t>(c));
        if(!address)throw std::logic_error("required SDK trampoline is unbound");
        if(provider_&&address!=originals_.at(static_cast<std::size_t>(c)))throw std::logic_error("Original provider returned a different runtime slot");
        return reinterpret_cast<Fn>(address);
    }
    template<class Fn,class... A>auto original_call(Callback c,A... args)const {
        if(!checkpoint_)return original<Fn>(c)(args...);
        checkpoint_->verify(); // guard resolution before any provider access
        Fn fn;
        try{fn=original<Fn>(c);}catch(...){checkpoint_->verify();throw;}
        // Recheck after virtual resolver: it may have changed PTD on return.
        return checkpoint_->invoke([&]{return fn(args...);});
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
}
