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
#include <optional>
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
// Automatic native-wrapper token for a selected argument-only original call.
// Domain owns its metadata while admitted. Stable stack identity, no heap/PTD.
// Parent links mask outer permissions and are restored on exact LIFO leave.
struct OriginalForwardFrame {
    OriginalForwardFrame()=default;
    OriginalForwardFrame(const OriginalForwardFrame&)=delete;
    OriginalForwardFrame& operator=(const OriginalForwardFrame&)=delete;
    const void* owner=nullptr;
    OriginalForwardFrame* parent=nullptr;
    DWORD thread=0;void* fiber=nullptr;
    Callback callback{};bool admitted=false;
};
struct RegistryNestedFrame;
// Exact typed SDK-call interval, after original resolution and before its
// postcheck. Body identity prevents an ancestor SDK call from admitting child
// CRT/log/set reentry. These automatic tokens never allocate or own a PTD.
struct RegistrySdkFrame {
    RegistrySdkFrame()=default;
    RegistrySdkFrame(const RegistrySdkFrame&)=delete;
    RegistrySdkFrame& operator=(const RegistrySdkFrame&)=delete;
    const void* owner=nullptr;RegistrySdkFrame* parent=nullptr;
    const RegistryNestedFrame* body=nullptr;
    DWORD thread=0;void* fiber=nullptr;Callback callback{};bool admitted=false;
};
struct RegistryNestedFrame {
    RegistryNestedFrame()=default;
    RegistryNestedFrame(const RegistryNestedFrame&)=delete;
    RegistryNestedFrame& operator=(const RegistryNestedFrame&)=delete;
    const void* owner=nullptr;RegistryNestedFrame* parent=nullptr;
    RegistrySdkFrame* sdk=nullptr;
    DWORD thread=0;void* fiber=nullptr;
    Callback callback{},parent_callback{};bool admitted=false;
};
// Optional source admission policy. enter must either return with one owned
// admission or throw without one. leave is nonthrowing and releases that entry.
// The native wrapper enters it before ordinary callback body/state/buffer access and
// releases it BEFORE the native binding lease. Stable for Context lifetime.
// Selected connect probes may then inspect only the frozen length snapshot and
// caller family prefix, with a separate classification checkpoint before SDK.
// The explicitly selected concurrent-original lane may perform only this same
// immutable classification under the Context lease before admission, including
// the caller's four-byte port prefix for Original route selection. That lane
// retains the selected wait/body/original phase and exact live port identity.
class Admission {
public:
    virtual ~Admission()=default;
    virtual void enter(Callback)=0;
    virtual void leave(Callback) noexcept=0;
    // Only null-subkey open A/W, null-application process A/W and exact
    // folder-id!=5 wrappers select this route. False keeps ordinary admission.
    // A true result owns one exact frame leave, including nested SDK calls.
    virtual bool try_enter_original_forward(Callback,OriginalForwardFrame&) {return false;}
    virtual void leave_original_forward(OriginalForwardFrame&) noexcept {std::terminate();}
    // Optional OriginalCrt-only same-thread general registry nesting. The root
    // admission retains its existing PTD; each child restores its exact parent.
    virtual bool begin_registry_sdk(Callback,RegistrySdkFrame&) {return false;}
    virtual void end_registry_sdk(RegistrySdkFrame&) noexcept {std::terminate();}
    virtual bool try_enter_registry_nested(Callback,RegistryNestedFrame&) {return false;}
    virtual void leave_registry_nested(RegistryNestedFrame&) noexcept {std::terminate();}
    // Original-profile immutable length/family classification may precede
    // ordinary body admission. A successful counted connect entry owns one
    // leave through its wait, redirect-only and original-only phases. General
    // registry/allocator/private recovered CRT permission is never granted.
    virtual bool has_connect_original_lane() const noexcept {return false;}
    // Exact Context port identity must match the passive before-publish bind.
    // Web compares the live scalar without waiting for Server completion;
    // non-web waits first, with3216 bypassing the comparison as in native.
    virtual bool try_enter_connect_original(std::uint16_t&,std::optional<std::uint16_t> ={},bool=false) {return false;}
    virtual bool connect_original_wait_pending()const {return false;}
    virtual void complete_connect_original_wait() {throw std::logic_error("Concurrent original wait is not selected");}
    virtual bool connect_original_redirect_pending()const {return false;}
    virtual void complete_connect_original_redirect() {throw std::logic_error("Concurrent original redirect is not selected");}
    virtual void host_setup_begin() {}
    virtual void host_setup_complete() {}
    // Source lifecycle checkpoint BEFORE worker completion/exit-hook cleanup.
    // Successful return still does not prove external hook quiescence.
    virtual bool pause_before_cleanup() {return true;}
    // Optional staged connect admission. A probe may inspect only the immutable
    // Context length snapshot and caller's family prefix. No general body or
    // provider permission until connect_classified accepts original-only flow.
    virtual bool connect_probe() const {return false;}
    virtual void connect_classified(bool) {throw std::logic_error("Connect probe is not selected");}
    // Optional no-redirect80/443 route. False grants no SDK/body permission.
    virtual bool connect_web_passthrough(std::uint16_t) {return false;}
    // Optional foreign no-redirect route through the existing native wait.
    // Begin grants wait ONLY; complete follows a normal wait return and must
    // independently recheck original eligibility before any SDK dispatch.
    virtual bool connect_wait_begin(std::uint16_t) {return false;}
    virtual void connect_wait_complete() {throw std::logic_error("Connect wait is not selected");}
    virtual bool connect_redirect_begin(std::uint16_t) {return false;}
    // ready follows required non-web wait; body excludes SDK permission until
    // complete. Returned port is a published copy, never a raw config borrow.
    virtual std::uint16_t connect_redirect_ready() {throw std::logic_error("Connect redirect is not selected");}
    virtual void connect_redirect_complete() {throw std::logic_error("Connect redirect is not selected");}
};
struct Context {
    registry_recovered::StateRefs registry;
    network_hook_recovered::StateRefs network;
    registry_recovered::Effects& registry_effects;
    host_callbacks_recovered::Effects& host_effects;
    network_hook_recovered::Effects& network_effects;
    Admission* admission=nullptr; // null preserves existing source path
    // Seeded once when Context is constructed, before publish/hooks. Selected
    // owning callers keep the seed lengths constant throughout Context life.
    // Source Context layout and Admission vtable change: rebuild consumers.
    const std::int32_t probe_ipv4_length=network.ipv4_length_0a10d0;
    const std::int32_t probe_ipv6_length=network.ipv6_length_0a10d4;
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
    // Passive, once-only binding before Context publication. The owner keeps
    // Admission alive beyond Effects; standalone policies may leave it null.
    void bind_registry_sdk_admission(Admission* value) {
        if(sdk_admission_bound_)throw std::logic_error("Original SDK admission already bound");
        sdk_admission_=value;sdk_admission_bound_=true;
    }
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
    Admission* sdk_admission_=nullptr;
    bool sdk_admission_bound_=false;
    struct RegistrySdkLease {
        RegistrySdkFrame frame;
        Admission* policy=nullptr;
        RegistrySdkLease(Admission* value,Callback callback) {
            if(value&&value->begin_registry_sdk(callback,frame))policy=value;
        }
        ~RegistrySdkLease(){if(policy)policy->end_registry_sdk(frame);}
    };
    template<class Fn,class... A>auto invoke_sdk(Callback c,Fn fn,A... args)const {
        RegistrySdkLease lease(sdk_admission_,c);
        return fn(args...);
    }
    template<class Fn> Fn original(Callback c)const {
        const auto address=provider_?provider_->load(c):originals_.at(static_cast<std::size_t>(c));
        if(!address)throw std::logic_error("required SDK trampoline is unbound");
        if(provider_&&address!=originals_.at(static_cast<std::size_t>(c)))throw std::logic_error("Original provider returned a different runtime slot");
        return reinterpret_cast<Fn>(address);
    }
    template<class Fn,class... A>auto original_call(Callback c,A... args)const {
        if(!checkpoint_)return invoke_sdk(c,original<Fn>(c),args...);
        checkpoint_->verify(); // guard resolution before any provider access
        Fn fn;
        try{fn=original<Fn>(c);}catch(...){checkpoint_->verify();throw;}
        // Recheck after virtual resolver: it may have changed PTD on return.
        return checkpoint_->invoke([&]{return invoke_sdk(c,fn,args...);});
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
