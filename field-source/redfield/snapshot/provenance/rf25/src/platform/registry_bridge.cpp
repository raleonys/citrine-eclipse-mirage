// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/platform/registry_bridge.hpp"
#include "redfield/platform/registry_policy.hpp"

namespace redfield { namespace platform {

void reset_host_bridge(HostBridgeState* state) noexcept {
    if (!state) return;
    *state = HostBridgeState{};
}

int32_t bridge_reg_open_a(HostBridgeState* state, void* root, const char* subkey,
                          uint32_t options, uint32_t access, void** result) noexcept {
    if (!state) return host_error_invalid_parameter;
    const int32_t original = state->originals.reg_open_a
        ? state->originals.reg_open_a(root,subkey,options,access,result)
        : host_error_invalid_parameter;
    const uintptr_t original_result=(result&&original==host_error_success)
        ? reinterpret_cast<uintptr_t>(*result) : 0u;
    uintptr_t final_result=original_result;
    const int32_t status=registry_after_open_a(subkey,original,original_result,result?&final_result:nullptr);
    if(result&&final_result!=original_result)*result=reinterpret_cast<void*>(final_result);
    return status;
}

int32_t bridge_reg_open_w(HostBridgeState* state, void* root, const uint16_t* subkey,
                          uint32_t options, uint32_t access, void** result) noexcept {
    if (!state) return host_error_invalid_parameter;
    const int32_t original = state->originals.reg_open_w
        ? state->originals.reg_open_w(root,subkey,options,access,result)
        : host_error_invalid_parameter;
    const uintptr_t original_result=(result&&original==host_error_success)
        ? reinterpret_cast<uintptr_t>(*result) : 0u;
    uintptr_t final_result=original_result;
    const int32_t status=registry_after_open_w(subkey,original,original_result,result?&final_result:nullptr);
    if(result&&final_result!=original_result)*result=reinterpret_cast<void*>(final_result);
    return status;
}

int32_t bridge_reg_query_a(HostBridgeState* state, void* key, const char* value_name,
                           uint32_t* reserved, uint32_t* type,
                           uint8_t* data, uint32_t* bytes) noexcept {
    if (!state) return host_error_invalid_parameter;
    const uint32_t input_capacity=bytes?*bytes:0u;
    const int32_t original=state->originals.reg_query_a
        ? state->originals.reg_query_a(key,value_name,reserved,type,data,bytes)
        : host_error_invalid_parameter;
    return registry_after_query_a(reinterpret_cast<uintptr_t>(key),value_name,original,input_capacity,
                                  state->executable_path.narrow,state->executable_path.narrow_length,
                                  type,data,bytes);
}

int32_t bridge_reg_query_w(HostBridgeState* state, void* key, const uint16_t* value_name,
                           uint32_t* reserved, uint32_t* type,
                           uint8_t* data, uint32_t* bytes) noexcept {
    if (!state) return host_error_invalid_parameter;
    const uint32_t input_capacity=bytes?*bytes:0u;
    const int32_t original=state->originals.reg_query_w
        ? state->originals.reg_query_w(key,value_name,reserved,type,data,bytes)
        : host_error_invalid_parameter;
    return registry_after_query_w(reinterpret_cast<uintptr_t>(key),value_name,original,input_capacity,
                                  state->executable_path.wide,state->executable_path.wide_units,
                                  type,data,bytes);
}

int32_t bridge_reg_close(HostBridgeState* state, void* key) noexcept {
    if (!state) return host_error_invalid_parameter;
    int32_t local=host_error_success;
    if(registry_close_pseudo(reinterpret_cast<uintptr_t>(key),&local))return local;
    const int32_t original=state->originals.reg_close?state->originals.reg_close(key):host_error_invalid_parameter;
    registry_after_real_close(reinterpret_cast<uintptr_t>(key),original);return original;
}


} }
