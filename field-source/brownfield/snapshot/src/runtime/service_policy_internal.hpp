#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/runtime/service_policy.hpp"

namespace preloader_l { namespace runtime { namespace model_release_service_policy_internal {

void zero_bytes(void* p, size_t n) noexcept;
void* allocate_bytes(size_t size) noexcept;
bool release_bytes(void* p, size_t logical_size) noexcept;

void initialize_empty_string(model_abi::MsvcString32* value) noexcept;
const char* string_data(const model_abi::MsvcString32& value) noexcept;
bool release_string(model_abi::MsvcString32* value) noexcept;
bool assign_string(model_abi::MsvcString32* destination,
                   const char* source, size_t size) noexcept;
bool string_equal(const model_abi::MsvcString32& value,
                  const char* data, size_t size) noexcept;

bool initialize_settings_map(model_abi::ModelSettingsUnorderedMap40* map) noexcept;
bool destroy_settings_map(model_abi::ModelSettingsUnorderedMap40* map) noexcept;
model_abi::ModelSettingsNode50* find_settings_node(
    model_abi::ModelSettingsUnorderedMap40* map,
    const char* key, size_t key_size) noexcept;
const model_abi::ModelSettingsNode50* find_settings_node(
    const model_abi::ModelSettingsUnorderedMap40* map,
    const char* key, size_t key_size) noexcept;
bool insert_or_assign_setting(model_abi::ModelSettingsUnorderedMap40* map,
                              const ModelSettingInput& input) noexcept;

void initialize_model_xml_document(model_abi::ModelXmlDocument370* doc) noexcept;
void repair_model_xml_document_self(model_abi::ModelXmlDocument370* doc) noexcept;
bool release_model_xml_document(model_abi::ModelXmlDocument370* doc) noexcept;
bool assign_model_xml_document(model_abi::ModelXmlDocument370* doc,
                               const char* xml, size_t length) noexcept;

void initialize_empty_wide_path(model_abi::ModelWidePathString32* value) noexcept;
bool release_path(model_abi::ModelWidePathString32* value) noexcept;
bool assign_wide_path(model_abi::ModelWidePathString32* value,
                      const uint16_t* source, size_t size) noexcept;
bool assign_path_from_narrow_bytes(model_abi::ModelWidePathString32* value,
                                   const char* source, size_t size) noexcept;

} } }
