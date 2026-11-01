// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/platform/registry_policy.hpp"
#include "registry_policy_internal.hpp"

namespace redfield { namespace platform {
using namespace host_policy_detail;

int32_t registry_after_open_a(const char* subkey,int32_t original_status,uintptr_t original_result,uintptr_t* result) noexcept {
    return after_open(classify_a(subkey),original_status,original_result,result);
}
int32_t registry_after_open_w(const uint16_t* subkey,int32_t original_status,uintptr_t original_result,uintptr_t* result) noexcept {
    return after_open(classify_w(subkey),original_status,original_result,result);
}
int32_t registry_after_query_a(uintptr_t key,const char* value_name,int32_t original_status,uint32_t input_capacity,
                               const char* executable_path,size_t executable_path_length,
                               uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    const uint32_t flags=handle_flags(key);if(!flags)return original_status;
    if(flags&k_client_active){const auto name=active_client_value_name();if(value_name_a(value_name,name)){const auto path=cfg("ClientPath");return write_a(path,executable_path,executable_path_length,input_capacity,type,data,bytes);}}
    if(flags&k_client_secondary){const auto name=configured_client_value_name();if(name.present&&value_name_a(value_name,name)){const auto path=cfg("ClientPath");return write_a(path,executable_path,executable_path_length,input_capacity,type,data,bytes);}}
    if(flags&k_language){const auto lang=language_policy();if(lang.enabled&&value_name_a(value_name,lang.value)){const auto value=cfg("Language");if(value.present)return write_a(value,nullptr,0,input_capacity,type,data,bytes);}}
    return (flags&k_pseudo)?host_error_file_not_found:original_status;
}
int32_t registry_after_query_w(uintptr_t key,const uint16_t* value_name,int32_t original_status,uint32_t input_capacity,
                               const uint16_t* executable_path,size_t executable_path_units,
                               uint32_t* type,uint8_t* data,uint32_t* bytes) noexcept {
    const uint32_t flags=handle_flags(key);if(!flags)return original_status;
    if(flags&k_client_active){const auto name=active_client_value_name();if(value_name_w(value_name,name)){const auto path=cfg("ClientPath");return write_w(path,executable_path,executable_path_units,input_capacity,type,data,bytes);}}
    if(flags&k_client_secondary){const auto name=configured_client_value_name();if(name.present&&value_name_w(value_name,name)){const auto path=cfg("ClientPath");return write_w(path,executable_path,executable_path_units,input_capacity,type,data,bytes);}}
    if(flags&k_language){const auto lang=language_policy();if(lang.enabled&&value_name_w(value_name,lang.value)){const auto value=cfg("Language");if(value.present)return write_w(value,nullptr,0,input_capacity,type,data,bytes);}}
    return (flags&k_pseudo)?host_error_file_not_found:original_status;
}
bool registry_close_pseudo(uintptr_t key,int32_t* status) noexcept {
    if (status) *status = host_error_success;
    const uint32_t flags = handle_flags(key);
    if (!(flags & k_pseudo)) return false;
    uint32_t removed = 0;
    if (!erase_handle(key, &removed)) return false;
    heap_free(reinterpret_cast<void*>(key));
    return true;
}
void registry_after_real_close(uintptr_t key,int32_t original_status) noexcept { if(original_status==host_error_success){uint32_t ignored=0;(void)erase_handle(key,&ignored);} }

} }

namespace redfield { namespace platform { namespace host_policy_detail {
int32_t after_open(uint32_t cls,int32_t original_status,uintptr_t original_result,uintptr_t* result) noexcept {
    if(!cls)return original_status;
    if(original_status==host_error_success&&original_result){(void)track_handle(original_result,cls);if(result)*result=original_result;return original_status;}
    if (!result) return original_status;
    const uintptr_t pseudo = allocate_pseudo(cls);
    if (!pseudo) return original_status;
    *result = pseudo;
    return host_error_success;
}
} } }
