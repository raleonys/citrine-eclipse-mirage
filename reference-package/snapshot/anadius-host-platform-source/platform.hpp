#pragma once
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include "../hook-engine-source/adapters.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include <memory>
#include <ostream>

namespace anadius::host_platform {
struct HostApi {
    decltype(&::VerSetConditionMask) condition;
    decltype(&::VerifyVersionInfoW) verify;
    decltype(&::GetModuleHandleW) module;
    decltype(&::GetProcAddress) procedure;
};
HostApi native_host_api() noexcept;
struct Resources {
    std::unique_ptr<encoding_recovered::Calls> encoding=std::make_unique<encoding_recovered::NativeCalls>();
    std::unique_ptr<encoding_recovered::Errors> errors=std::make_unique<encoding_recovered::PortableErrors>();
    HostApi api=native_host_api();
};
// Explicit shared owner, not a native Context factory. callback_lifetime must
// retain resolver/context/binding/state as needed. Its presence does not prove
// that binding is published, original slots share identity, or calls quiesced.
// Caller must establish those contracts before installing hooks and retain all
// state until disabling hooks, external quiescence and Binding.close succeed.
// Construction/destruction performs no engine/SDK/log/shutdown operation.
class Domain final {
public:
    Domain(std::shared_ptr<std::ostream>,std::shared_ptr<recovered_hooks::Api>,
        std::shared_ptr<recovered_hooks::CallbackResolver>,std::shared_ptr<void> callback_lifetime,Resources={});
    Domain(const Domain&)=delete;Domain& operator=(const Domain&)=delete;
private:
    friend class HostPlatform;friend class HookCleanup;
    std::u16string widen(std::string_view);
    std::uint64_t condition(std::uint64_t,std::uint32_t,std::uint8_t);
    bool verify(const host_init_recovered::VersionRequest&,std::uint32_t,std::uint64_t);
    std::uintptr_t module(std::u16string_view);
    std::uintptr_t procedure(std::uintptr_t,std::string_view);
    void text(std::string_view);void integer(std::int32_t);void finish();
    // Reverse destruction retains lifetime/resources/stream through engine and
    // resolver destruction. streambuf must live; caller serializes all users.
    std::shared_ptr<void> callback_lifetime_;
    Resources resources_;
    std::shared_ptr<std::ostream> stream_;
    std::shared_ptr<recovered_hooks::Api> engine_;
    std::shared_ptr<recovered_hooks::CallbackResolver> resolver_;
};
class HostPlatform final:public host_init_recovered::Effects {
public:
    explicit HostPlatform(std::shared_ptr<Domain>);
    HostPlatform(const HostPlatform&)=delete;HostPlatform& operator=(const HostPlatform&)=delete;
    HostPlatform(HostPlatform&&)=delete;HostPlatform& operator=(HostPlatform&&)=delete;
    std::u16string filesystem_widen_006d70(std::string_view) override;
    std::uint64_t version_condition_mask(std::uint64_t,std::uint32_t,std::uint8_t) override;
    bool verify_version(const host_init_recovered::VersionRequest&,std::uint32_t,std::uint64_t) override;
    std::int32_t initialize_engine_046a20() override;
    std::uintptr_t get_module_handle(std::u16string_view) override;
    std::uintptr_t get_proc_address(std::uintptr_t,std::string_view) override;
    std::int32_t create_hook_0466e0(std::uintptr_t,host_init_recovered::Callback,std::uintptr_t&) override;
    std::int32_t enable_hook_046a10(std::uintptr_t) override;
    void log_text_006880(std::string_view) override;
    void log_integer_00af90(std::int32_t) override;
    void finish_log_0088e0() override;
private:
    std::shared_ptr<Domain> domain_;
    recovered_hooks::HostInitEffects bridge_; // stable references to this/domain
};
class HookCleanup final:public network_hook_recovered::CleanupEffects {
public:
    explicit HookCleanup(std::shared_ptr<Domain>);
    HookCleanup(const HookCleanup&)=delete;HookCleanup& operator=(const HookCleanup&)=delete;
    HookCleanup(HookCleanup&&)=delete;HookCleanup& operator=(HookCleanup&&)=delete;
    std::int32_t disable_hook_046a00(std::uintptr_t) override;
    std::int32_t uninitialize_engine_046ac0() override;
    void log_literal_006880(std::string_view) override;
    void log_finish_0088e0() override;
private:
    std::shared_ptr<Domain> domain_;
    recovered_hooks::CleanupEffects bridge_;
};
bootstrap_backend::Resources bind_host_platform(bootstrap_backend::Resources,std::shared_ptr<Domain>);
} // namespace anadius::host_platform
