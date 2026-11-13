// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/platform/hook_engine_api.hpp"
#include "citrine_eclipse_mirage/platform/hook_setup.hpp"
#include "citrine_eclipse_mirage/platform/hook_cleanup.hpp"

namespace citrine_eclipse_mirage::platform::hook_engine {
// Caller must provide ABI-correct addresses of rebuilt callback wrappers.
// Preferred original-image VAs in callback-bindings.json are never substitutes.
class CallbackResolver {
public:
    virtual ~CallbackResolver() = default;
    virtual std::uintptr_t rebuilt_address(citrine_eclipse_mirage::host_init::Callback) = 0;
};
class HostInitEffects final : public citrine_eclipse_mirage::host_init::Effects {
public:
    HostInitEffects(citrine_eclipse_mirage::host_init::Effects& remaining, Api& engine,
                    CallbackResolver& callbacks)
        : remaining_(remaining), engine_(engine), callbacks_(callbacks) {}
    std::u16string filesystem_widen_006d70(std::string_view) override;
    std::uint64_t version_condition_mask(std::uint64_t,std::uint32_t,std::uint8_t) override;
    bool verify_version(const citrine_eclipse_mirage::host_init::VersionRequest&,std::uint32_t,std::uint64_t) override;
    std::int32_t initialize_engine_046a20() override;
    std::uintptr_t get_module_handle(std::u16string_view) override;
    std::uintptr_t get_proc_address(std::uintptr_t,std::string_view) override;
    std::int32_t create_hook_0466e0(std::uintptr_t,citrine_eclipse_mirage::host_init::Callback,std::uintptr_t&) override;
    std::int32_t enable_hook_046a10(std::uintptr_t) override;
    void log_text_006880(std::string_view) override;
    void log_integer_00af90(std::int32_t) override;
    void finish_log_0088e0() override;
private:
    citrine_eclipse_mirage::host_init::Effects& remaining_;
    Api& engine_;
    CallbackResolver& callbacks_;
};
class CleanupEffects final : public citrine_eclipse_mirage::network_hook::CleanupEffects {
public:
    CleanupEffects(citrine_eclipse_mirage::network_hook::CleanupEffects& remaining,Api& engine)
        : remaining_(remaining),engine_(engine) {}
    std::int32_t disable_hook_046a00(std::uintptr_t) override;
    std::int32_t uninitialize_engine_046ac0() override;
    void log_literal_006880(std::string_view) override;
    void log_finish_0088e0() override;
private:
    citrine_eclipse_mirage::network_hook::CleanupEffects& remaining_;
    Api& engine_;
};
}
