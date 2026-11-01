#if !defined(_WIN32)
#error native_acceptance_windows.cpp is Windows-only.
extern "C" uint32_t preloader_l_native_acceptance_campaign18_scenario(uint32_t selector) noexcept
{
    return campaign18_scenario_impl(selector);
}

#endif

#include "preloader_l/runtime/bootstrap_thread.hpp"
#include "preloader_l/runtime/release_service_lifetime.hpp"
#include "preloader_l/runtime/release_server_io.hpp"
#include "../../src/runtime/asio_acceptance_resource_fault.hpp"

namespace preloader_l { namespace runtime { namespace release_server_windows_detail {
#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
void configure_release_acceptance_resource_fault_windows(uint32_t selector) noexcept;
uint32_t release_acceptance_resource_fault_metric_windows(uint32_t selector) noexcept;
#endif
} } }
#include "preloader_l/runtime/service_abi.hpp"
#include "preloader_l/runtime/ptd_runtime.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/platform/crypt_observer.hpp"
#include "preloader_l/platform/connect_redirect.hpp"
#include "preloader_l/platform/registry_hook_runtime.hpp"
#include "preloader_l/platform/minhook_runtime.hpp"
#include <stdint.h>

#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)
namespace {


uintptr_t load_path_pointer_bits(const uint16_t storage[8]) noexcept
{
    uintptr_t raw = 0u;
    auto* out = reinterpret_cast<uint8_t*>(&raw);
    const auto* in = reinterpret_cast<const uint8_t*>(storage);
    for (size_t i = 0; i < sizeof(raw); ++i) out[i] = in[i];
    return raw;
}

const uint16_t* wide_path_data(
    const preloader_l::runtime::model_abi::ModelWidePathString32& path) noexcept
{
    if (path.capacity <= 7u) return path.storage;
    return reinterpret_cast<const uint16_t*>(load_path_pointer_bits(path.storage));
}

bool wide_path_equals(
    const preloader_l::runtime::model_abi::ModelWidePathString32& path,
    const uint16_t* expected, size_t count) noexcept
{
    if (path.size != count || !expected) return false;
    const uint16_t* data = wide_path_data(path);
    if (!data) return false;
    for (size_t i = 0; i < count; ++i) if (data[i] != expected[i]) return false;
    return data[count] == 0u;
}

bool bytes_equal(const char* a, const char* b, size_t count) noexcept
{
    if ((!a || !b) && count) return false;
    for (size_t i = 0; i < count; ++i) if (a[i] != b[i]) return false;
    return true;
}

preloader_l::runtime::model_abi::ReleaseServiceObject548* acceptance_service() noexcept
{
    const auto snapshot = preloader_l::runtime::release_service_lifetime_snapshot();
    return reinterpret_cast<preloader_l::runtime::model_abi::ReleaseServiceObject548*>(
        snapshot.model_service);
}

uint32_t config_scenario_impl(uint32_t selector) noexcept
{
    using preloader_l::config::RuntimeConfigurationStatus;
    auto* service = acceptance_service();
    if (!service) return 0x2901u;

    if (selector == 0u) {
        const uint16_t path[7] = {'C',':','\\','T','E','S','T'};
        if (!preloader_l::config::set_runtime_current_directory_wide_policy(path, 7u)) return 0x2910u;
        return service->config.path.capacity == 7u && wide_path_equals(service->config.path, path, 7u) ? 0u : 0x2911u;
    }
    if (selector == 1u) {
        const uint16_t path[8] = {'C',':','\\',0x03A9u,'T','E','S','T'};
        if (!preloader_l::config::set_runtime_current_directory_wide_policy(path, 8u)) return 0x2920u;
        return service->config.path.capacity > 7u && wide_path_equals(service->config.path, path, 8u) ? 0u : 0x2921u;
    }
    if (selector == 2u) {
        uint16_t long_path[96]{};
        long_path[0]='C'; long_path[1]=':'; long_path[2]='\\';
        for (size_t i=3;i<95u;++i) long_path[i]=static_cast<uint16_t>('A'+(i%20u));
        if (!preloader_l::config::set_runtime_current_directory_wide_policy(long_path, 95u)) return 0x2930u;
        if (service->config.path.capacity <= 7u || !wide_path_equals(service->config.path,long_path,95u)) return 0x2931u;
        const uint16_t short_path[7]={'C',':','\\','R','E','P','L'};
        if (!preloader_l::config::set_runtime_current_directory_wide_policy(short_path,7u)) return 0x2932u;
        return service->config.path.capacity==7u && wide_path_equals(service->config.path,short_path,7u) ? 0u : 0x2933u;
    }

    static constexpr char kCfgA[] = R"CFG("Config2" { "User" { "Value" "A" } })CFG";
    static constexpr char kCfgB[] = R"CFG("Config2" { "User" { "Value" "B" } })CFG";
    if (selector == 3u) {
        if (preloader_l::config::set_runtime_configuration_cfg(kCfgA,sizeof(kCfgA)-1u)!=RuntimeConfigurationStatus::ok) return 0x2940u;
        const auto value=preloader_l::config::runtime_configuration_setting("Value");
        return service->config.document.char_buffer==0u && value.present && value.size==1u && value.data && value.data[0]=='A' ? 0u : 0x2941u;
    }
    if (selector == 4u) {
        static constexpr char bom_cfg[] = "\xEF\xBB\xBF\"Config2\" { \"User\" { \"Value\" \"BOM\" } }";
        if (preloader_l::config::set_runtime_configuration_cfg(bom_cfg,sizeof(bom_cfg)-1u)!=RuntimeConfigurationStatus::ok) return 0x2950u;
        const auto value=preloader_l::config::runtime_configuration_setting("Value");
        return service->config.document.char_buffer==0u && value.present && value.size==3u && bytes_equal(value.data,"BOM",3u) ? 0u : 0x2951u;
    }
    if (selector == 5u) {
        static char large_cfg[0x1400]{};
        static constexpr char prefix[]="\"Config2\" { \"User\" { \"Big\" \"";
        static constexpr char suffix[]="\" } }";
        size_t pos=0u;
        for(size_t i=0;i<sizeof(prefix)-1u;++i) large_cfg[pos++]=prefix[i];
        const size_t value_begin=pos;
        while(pos<0x1200u) large_cfg[pos++]='X';
        const size_t value_size=pos-value_begin;
        for(size_t i=0;i<sizeof(suffix)-1u;++i) large_cfg[pos++]=suffix[i];
        large_cfg[pos]=0;
        if (preloader_l::config::set_runtime_configuration_cfg(large_cfg,pos)!=RuntimeConfigurationStatus::ok) return 0x2960u;
        const auto value=preloader_l::config::runtime_configuration_setting("Big");
        return service->config.document.char_buffer==0u && value.present && value.size==value_size ? 0u : 0x2961u;
    }
    if (selector == 6u) {
        if (preloader_l::config::set_runtime_configuration_cfg(kCfgA,sizeof(kCfgA)-1u)!=RuntimeConfigurationStatus::ok) return 0x2970u;
        static constexpr char invalid[]="\"Config2\" { \"User\" { \"Value\" \"BAD\" }";
        if (preloader_l::config::set_runtime_configuration_cfg(invalid,sizeof(invalid)-1u)!=RuntimeConfigurationStatus::invalid_cfg) return 0x2971u;
        const auto value=preloader_l::config::runtime_configuration_setting("Value");
        return value.present && value.size==1u && value.data && value.data[0]=='A' ? 0u : 0x2972u;
    }
    if (selector == 7u) {
        if (preloader_l::config::set_runtime_configuration_cfg(kCfgA,sizeof(kCfgA)-1u)!=RuntimeConfigurationStatus::ok) return 0x2980u;
        if (preloader_l::config::set_runtime_configuration_cfg(kCfgB,sizeof(kCfgB)-1u)!=RuntimeConfigurationStatus::ok) return 0x2981u;
        const auto value=preloader_l::config::runtime_configuration_setting("Value");
        return service->config.document.char_buffer==0u && value.present && value.size==1u && value.data && value.data[0]=='B' ? 0u : 0x2982u;
    }
    if (selector == 8u) {
        if (preloader_l::config::set_runtime_configuration_cfg(kCfgA,sizeof(kCfgA)-1u)!=RuntimeConfigurationStatus::ok) return 0x2990u;
        preloader_l::config::clear_runtime_configuration();
        const auto value=preloader_l::config::runtime_configuration_setting("Value");
        return service->config.document.char_buffer==0u && service->config.document.node.document==reinterpret_cast<uintptr_t>(&service->config.document) && !preloader_l::config::runtime_configuration_loaded() && !value.present ? 0u : 0x2991u;
    }
    return 0x29ffu;
}


uint32_t campaign18_scenario_impl(uint32_t selector) noexcept
{
    // MUSTORIN-MH: custom IAT/inline-engine campaign selectors are deliberately
    // not carried forward. MinHook mechanism/lifetime failure paths are covered
    // by the fork QA and native 9/9 early-install evidence. This test-only
    // export retains the application-level PTD/FLS lifecycle scenario.
    if (selector == 0u) {
        if (preloader_l::runtime::ptd_fls_index() ==
            preloader_l::runtime::ptd_fls_out_of_indexes) return 0x2A40u;
        void* first = preloader_l::runtime::current_ptd(true);
        int* first_errno = preloader_l::runtime::current_errno_slot();
        if (!first || !first_errno) return 0x2A41u;
        *first_errno = 0x321;
        if (!preloader_l::runtime::detach_current_thread_ptd()) return 0x2A42u;
        if (preloader_l::runtime::current_ptd(false) != nullptr) return 0x2A43u;
        void* second = preloader_l::runtime::current_ptd(true);
        int* second_errno = preloader_l::runtime::current_errno_slot();
        if (!second || !second_errno) return 0x2A44u;
        return *second_errno == 0 ? 0u : 0x2A45u;
    }
    return 0x2AFFu;
}

uint32_t controlled_teardown_impl(uint32_t timeout_ms) noexcept
{
    // Official V6 no-fault/fault teardown is only meaningful after a healthy
    // startup. Cleanup idempotence and acceptance readiness are intentionally
    // separate contracts: not_initialized is never an acceptance PASS.
    const auto ready = preloader_l::runtime::wait_release_bootstrap_ready(timeout_ms);
    if (ready != preloader_l::runtime::ReleaseBootstrapReadyResult::ready)
        return 0x2810u + static_cast<uint32_t>(ready);

    const auto before = preloader_l::runtime::release_service_lifetime_snapshot();
    if (before.initialized == 0 || before.context == 0 ||
        before.model_service == 0 || before.owner_control == 0 ||
        before.strong_count < 1)
        return 0x2818u;

    // Acceptance must exercise the same explicit pre-unload lifecycle owner as
    // shipping. The current runtime owns the nine-hook surface as one committed
    // MinHook transaction, so duplicating the old subsystem-by-subsystem teardown here
    // would bypass the global rollback contract and release service policy in a
    // different order from DllShutdown.
    const auto shutdown =
        preloader_l::runtime::shutdown_release_runtime_for_unload(timeout_ms);
    if (shutdown != preloader_l::runtime::ReleaseRuntimeShutdownResult::shutdown)
        return 0x2820u + static_cast<uint32_t>(shutdown);

    const auto after = preloader_l::runtime::release_service_lifetime_snapshot();
    if (after.initialized != 0 || after.context != 0 ||
        after.model_service != 0 || after.owner_control != 0 ||
        after.destroy_dispatches <= before.destroy_dispatches ||
        after.delete_this_dispatches <= before.delete_this_dispatches ||
        after.scalar_delete_dispatches <= before.scalar_delete_dispatches)
        return 0x2840u;

    return 0u;
}
}

extern "C" uint32_t preloader_l_debug_wait_ready() noexcept
{
    return static_cast<uint32_t>(
        preloader_l::runtime::wait_release_bootstrap_ready(30000u));
}

extern "C" uint32_t preloader_l_debug_controlled_teardown() noexcept
{
    return controlled_teardown_impl(30000u);
}

extern "C" uint32_t preloader_l_native_acceptance_quiesce(uint32_t timeout_ms) noexcept
{
    return controlled_teardown_impl(timeout_ms);
}

extern "C" uint32_t preloader_l_native_acceptance_worker_metric(uint32_t selector) noexcept
{
    const auto snapshot = preloader_l::runtime::release_service_lifetime_snapshot();
    auto* service = reinterpret_cast<preloader_l::runtime::model_abi::ReleaseServiceObject548*>(
        snapshot.model_service);
    return preloader_l::runtime::release_worker_acceptance_metric_windows(service, selector);
}

extern "C" uint32_t preloader_l_native_acceptance_config_scenario(uint32_t selector) noexcept
{
    return config_scenario_impl(selector);
}
extern "C" uint32_t preloader_l_native_acceptance_f02_control(uint32_t selector) noexcept
{
    switch (selector) {
    case 0u:
        preloader_l::runtime::configure_release_acceptance_send_block_windows(true);
        return 1u;
    case 1u:
        preloader_l::runtime::configure_release_acceptance_send_block_windows(false);
        return 0u;
    case 2u:
        return preloader_l::runtime::release_acceptance_send_block_metric_windows(2u);
    case 3u:
        return preloader_l::runtime::release_acceptance_send_block_metric_windows(1u);
    case 4u:
        return preloader_l::runtime::release_acceptance_send_block_metric_windows(0u);
    case 5u:
        return preloader_l::runtime::release_acceptance_send_block_metric_windows(5u);
    default:
        return 0xffffffffu;
    }
}
extern "C" uint32_t preloader_l_native_acceptance_wave4_control(uint32_t selector) noexcept
{
    // HF13 native shipping uses Asio socket/acceptor owners, never the old
    // ModelJthread or raw WinSock pending socket registry. Keep those old
    // hooks isolated to mock/legacy service tests; native exercises IOCP.
    if (selector<=2u) {
        preloader_l::runtime::configure_asio_acceptance_resource_fault(selector);
        return selector==0u ? 0u : 1u;
    }
    if (selector>=3u && selector<=5u)
        return preloader_l::runtime::asio_acceptance_resource_fault_metric(selector-2u);
    return 0xffffffffu;
}
extern "C" uint64_t preloader_l_native_acceptance_connect_metric(uint32_t selector) noexcept
{
    switch (selector) {
    case 0u: return static_cast<uint64_t>(preloader_l::platform::connect_selected_port());
    case 1u: return static_cast<uint64_t>(preloader_l::platform::connect_redirect_call_count());
    case 2u: return static_cast<uint64_t>(preloader_l::platform::connect_redirect_mapped_count());
    case 3u: return static_cast<uint64_t>(preloader_l::platform::connect_redirect_target_address());
    case 4u: return static_cast<uint64_t>(preloader_l::platform::connect_redirect_trampoline_address());
    default: return 0u;
    }
}

extern "C" uint32_t preloader_l_native_acceptance_campaign18_scenario(uint32_t selector) noexcept
{
    return campaign18_scenario_impl(selector);
}

#endif
