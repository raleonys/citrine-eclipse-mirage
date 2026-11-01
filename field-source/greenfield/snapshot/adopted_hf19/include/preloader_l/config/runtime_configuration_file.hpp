#pragma once
#include <stdint.h>
namespace preloader_l { namespace config {
enum class RuntimeConfigurationFileStatus : uint8_t {
    ok=0,
    invalid_argument,
    not_found,
    io_error,
    too_large,
    invalid_cfg,
    resource_error,
};
// Normalized startup configuration is behavioral-style Config2 text in surge.cfg.
// A null path selects "surge.cfg" relative to the process current directory.
RuntimeConfigurationFileStatus load_runtime_configuration_file(const char* path) noexcept;
// Unicode-safe explicit-path loader used by the DLL-relative startup policy.
// The UTF-16 path is passed directly to CreateFileW; unlike the narrow helper,
// a missing path does not imply process-CWD fallback.
RuntimeConfigurationFileStatus load_runtime_configuration_file_wide(const uint16_t* path) noexcept;
// Single-commit module-adjacent Config2 base + optional override. Later scalar
// keys override earlier keys; nested behavioral fields are parsed from one
// combined document. Missing both files returns not_found. Any read/parse/
// publication failure is fail-closed; no intermediate base is published.
RuntimeConfigurationFileStatus load_runtime_configuration_pair_wide(
    const uint16_t* base_path,const uint16_t* override_path) noexcept;
} }
