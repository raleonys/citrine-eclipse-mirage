#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "service_policy_internal.hpp"

namespace preloader_l { namespace runtime {
namespace {
// Adapter/config readiness are clean runtime semantics. They intentionally live
// outside the curio 0x548 object so no private PLRDOC/PLRMAP metadata is
// hidden inside model-owned bytes.
model_abi::ReleaseServiceObject548* g_bound_service = nullptr;
ReleaseServerContext* g_bound_adapter = nullptr;
}

using namespace model_release_service_policy_internal;
using SettingsMap = model_abi::ModelSettingsUnorderedMap40;

bool initialize_model_service_policy(
    model_abi::ReleaseServiceObject548* service,
    ReleaseServerContext* adapter) noexcept {
    if (!service) return false;
    initialize_model_xml_document(&service->config.document);
    initialize_empty_wide_path(&service->config.path);
    initialize_empty_string(&service->content);
    initialize_empty_string(&service->entitlements);
    initialize_empty_string(&service->offers);
    initialize_empty_string(&service->achievements);
    if (!initialize_settings_map(&service->config.settings)) return false;
    if (adapter) {
        g_bound_service = service;
        g_bound_adapter = adapter;
        adapter->configuration_loaded = 0u;
        adapter->configuration_present_sections = 0u;
    }
    return true;
}

void swap_model_service_configuration_policy(
    model_abi::ReleaseServiceObject548* live,
    model_abi::ReleaseServiceObject548* candidate) noexcept {
    if (!live || !candidate || live == candidate) return;

    // Transfer the curio XMLDocument core/owned parse buffer together with
    // the Settings policy. Private model MemPoolT bytes remain opaque.
    for (size_t i = 0; i < sizeof(live->config.document); ++i) {
        uint8_t t = reinterpret_cast<uint8_t*>(&live->config.document)[i];
        reinterpret_cast<uint8_t*>(&live->config.document)[i] = reinterpret_cast<uint8_t*>(&candidate->config.document)[i];
        reinterpret_cast<uint8_t*>(&candidate->config.document)[i] = t;
    }
    repair_model_xml_document_self(&live->config.document);
    repair_model_xml_document_self(&candidate->config.document);

    auto settings = live->config.settings;
    live->config.settings = candidate->config.settings;
    candidate->config.settings = settings;

    auto content = live->content; live->content = candidate->content; candidate->content = content;
    auto entitlements = live->entitlements; live->entitlements = candidate->entitlements; candidate->entitlements = entitlements;
    auto offers = live->offers; live->offers = candidate->offers; candidate->offers = offers;
    auto achievements = live->achievements; live->achievements = candidate->achievements; candidate->achievements = achievements;
}

bool release_model_service_policy_retryable(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return true;
    bool ok = true;
    ok = destroy_settings_map(&service->config.settings) && ok;
    ok = release_path(&service->config.path) && ok;
    ok = release_string(&service->achievements) && ok;
    ok = release_string(&service->offers) && ok;
    ok = release_string(&service->entitlements) && ok;
    ok = release_string(&service->content) && ok;
    ok = release_model_xml_document(&service->config.document) && ok;
    if (ok && service == g_bound_service && g_bound_adapter) {
        g_bound_adapter->configuration_loaded = 0u;
        g_bound_adapter->configuration_present_sections = 0u;
    }
    return ok;
}

void release_model_service_policy(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!release_model_service_policy_retryable(service)) return;
    // The retryable preflight deliberately preserves the service->adapter
    // binding so shutdown can retry failed frees without losing the adapter
    // owner. The terminal/destructor path retires that binding only after all
    // policy-owned storage is physically released.
    if (service == g_bound_service) {
        g_bound_service = nullptr;
        g_bound_adapter = nullptr;
    }
}

ReleaseServerContext* model_service_adapter(
    model_abi::ReleaseServiceObject548* service) noexcept {
    return service && service == g_bound_service ? g_bound_adapter : nullptr;
}

bool commit_model_service_document(
    model_abi::ReleaseServiceObject548* service,
    const char* xml, size_t length) noexcept {
    // The custom parser has already validated XML before this boundary. Mirror
    // the active successful XMLDocument ownership state: the document owns an
    // independent NUL-terminated parse buffer at +0x98 while private pool/tree
    // bytes remain evidence-opaque rather than fabricated.
    return service && assign_model_xml_document(&service->config.document, xml, length);
}

bool commit_model_service_settings(
    model_abi::ReleaseServiceObject548* service,
    const ModelSettingInput* entries, size_t count) noexcept {
    if (!service || (count && !entries)) return false;

    SettingsMap candidate{};
    if (!initialize_settings_map(&candidate)) return false;
    for (size_t i = 0; i < count; ++i) {
        if (!insert_or_assign_setting(&candidate, entries[i])) {
            destroy_settings_map(&candidate);
            return false;
        }
    }

    auto previous = service->config.settings;
    service->config.settings = candidate;
    candidate = previous;
    destroy_settings_map(&candidate);
    return true;
}

void set_model_service_configuration_state(
    model_abi::ReleaseServiceObject548* service,
    bool loaded, uint32_t present_sections) noexcept {
    auto* adapter = model_service_adapter(service);
    if (!adapter) return;
    adapter->configuration_loaded = loaded ? 1u : 0u;
    adapter->configuration_present_sections = loaded ? present_sections : 0u;
}

bool commit_model_service_current_directory(
    model_abi::ReleaseServiceObject548* service,
    const char* value, size_t length) noexcept {
    return service && assign_path_from_narrow_bytes(&service->config.path, value, length);
}

bool commit_model_service_current_directory_wide(
    model_abi::ReleaseServiceObject548* service,
    const uint16_t* value, size_t length) noexcept {
    return service && assign_wide_path(&service->config.path, value, length);
}

bool model_service_configuration_loaded(
    const model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service || service != g_bound_service || !g_bound_adapter) return false;
    return g_bound_adapter->configuration_loaded != 0u;
}

preloader_l::config::ValueView model_service_setting(
    const model_abi::ReleaseServiceObject548* service,
    const char* key) noexcept {
    if (!service || !key) return preloader_l::config::ValueView{nullptr,0,false};
    size_t key_size = 0u;
    while (key[key_size]) ++key_size;
    const auto* node = find_settings_node(&service->config.settings, key, key_size);
    if (!node) return preloader_l::config::ValueView{nullptr,0,false};
    const char* value = string_data(node->value);
    return preloader_l::config::ValueView{
        value, static_cast<size_t>(node->value.size), value != nullptr
    };
}

uint32_t model_service_present_sections(
    const model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service || service != g_bound_service || !g_bound_adapter ||
        !g_bound_adapter->configuration_loaded) return 0u;
    return g_bound_adapter->configuration_present_sections;
}

preloader_l::config::ValueView model_service_string_view(
    const model_abi::MsvcString32& value) noexcept {
    const char* p = string_data(value);
    return value.size ? preloader_l::config::ValueView{p, static_cast<size_t>(value.size), p != nullptr}
                      : preloader_l::config::ValueView{nullptr,0,false};
}

bool assign_model_service_string(
    model_abi::MsvcString32* destination,
    preloader_l::config::ValueView source) noexcept {
    if (!destination || (source.size && !source.data)) return false;
    if (!source.present || !source.size) {
        release_model_service_string(destination);
        return true;
    }
    return assign_string(destination, source.data, source.size);
}

void release_model_service_string(model_abi::MsvcString32* value) noexcept {
    (void)release_string(value);
}

} }
