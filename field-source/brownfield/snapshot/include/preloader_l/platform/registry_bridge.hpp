#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

using RegOpenAOriginal = int32_t (*)(void* root, const char* subkey,
                                     uint32_t options, uint32_t access,
                                     void** result) noexcept;
using RegOpenWOriginal = int32_t (*)(void* root, const uint16_t* subkey,
                                     uint32_t options, uint32_t access,
                                     void** result) noexcept;
using RegQueryAOriginal = int32_t (*)(void* key, const char* value_name,
                                      uint32_t* reserved, uint32_t* type,
                                      uint8_t* data, uint32_t* bytes) noexcept;
using RegQueryWOriginal = int32_t (*)(void* key, const uint16_t* value_name,
                                      uint32_t* reserved, uint32_t* type,
                                      uint8_t* data, uint32_t* bytes) noexcept;
using RegCloseOriginal = int32_t (*)(void* key) noexcept;

struct HostOriginalFunctions {
    RegOpenAOriginal reg_open_a;
    RegOpenWOriginal reg_open_w;
    RegQueryAOriginal reg_query_a;
    RegQueryWOriginal reg_query_w;
    RegCloseOriginal reg_close;
};

struct HostExecutablePath {
    const char* narrow;
    size_t narrow_length;
    const uint16_t* wide;
    size_t wide_units;
};

struct HostBridgeState {
    HostOriginalFunctions originals;
    HostExecutablePath executable_path;
};

void reset_host_bridge(HostBridgeState* state) noexcept;

int32_t bridge_reg_open_a(HostBridgeState* state, void* root, const char* subkey,
                          uint32_t options, uint32_t access, void** result) noexcept;
int32_t bridge_reg_open_w(HostBridgeState* state, void* root, const uint16_t* subkey,
                          uint32_t options, uint32_t access, void** result) noexcept;
int32_t bridge_reg_query_a(HostBridgeState* state, void* key, const char* value_name,
                           uint32_t* reserved, uint32_t* type,
                           uint8_t* data, uint32_t* bytes) noexcept;
int32_t bridge_reg_query_w(HostBridgeState* state, void* key, const uint16_t* value_name,
                           uint32_t* reserved, uint32_t* type,
                           uint8_t* data, uint32_t* bytes) noexcept;
int32_t bridge_reg_close(HostBridgeState* state, void* key) noexcept;

} }
