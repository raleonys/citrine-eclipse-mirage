#if !defined(_WIN32)
#error process_configuration.cpp is Windows-only.
#endif

#include "preloader_l/config/process_configuration.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/config/runtime_provider.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include "runtime_configuration_command_line.hpp"
#include "module_relative_filesystem.hpp"
#include <stddef.h>
#include <stdint.h>
#include <windows.h>  // Win32 API ABI must use Windows SDK prototypes.

namespace {
static_assert(sizeof(WCHAR)==sizeof(uint16_t),"Windows UTF-16 ABI mismatch");

char* allocate_path(size_t bytes) noexcept {
    if (!bytes) return nullptr;
    const auto r=preloader_l::runtime::allocate_block(
        bytes,preloader_l::runtime::AllocationRetryPolicy::outer_retry_only,
        preloader_l::runtime::windows_allocator_platform_ops());
    return r.terminal==preloader_l::runtime::AllocationTerminal::none
        ? static_cast<char*>(r.pointer) : nullptr;
}
void release_path(char* p,size_t bytes) noexcept {
    if(!p)return;
    preloader_l::runtime::release_block(
        p,bytes,preloader_l::runtime::windows_allocator_platform_ops());
}

preloader_l::config::RuntimeConfigurationFileStatus
load_module_relative_configuration() noexcept {
    constexpr size_t path_capacity=32768u;
    constexpr uint16_t config_name[]={
        's','u','r','g','e','.','c','f','g',0u
    };
    constexpr uint16_t override_name[]={
        's','u','r','g','e','_','o','v','e','r','r','i','d','e','.','c','f','g',0u
    };
    constexpr size_t path_bytes=path_capacity*sizeof(uint16_t);

    auto* path=reinterpret_cast<uint16_t*>(allocate_path(path_bytes));
    auto* override_path=reinterpret_cast<uint16_t*>(allocate_path(path_bytes));
    if(!path||!override_path){
        release_path(reinterpret_cast<char*>(path),path_bytes);
        release_path(reinterpret_cast<char*>(override_path),path_bytes);
        return preloader_l::config::RuntimeConfigurationFileStatus::resource_error;
    }

    const auto state=preloader_l::runtime::snapshot_state();
    if(!state.module_base){
        release_path(reinterpret_cast<char*>(path),path_bytes);
        release_path(reinterpret_cast<char*>(override_path),path_bytes);
        return preloader_l::config::RuntimeConfigurationFileStatus::io_error;
    }

    const DWORD n=GetModuleFileNameW(
        reinterpret_cast<HMODULE>(state.module_base),reinterpret_cast<LPWSTR>(path),
        static_cast<DWORD>(path_capacity));
    if(!n || static_cast<size_t>(n)>=path_capacity){
        release_path(reinterpret_cast<char*>(path),path_bytes);
        release_path(reinterpret_cast<char*>(override_path),path_bytes);
        return preloader_l::config::RuntimeConfigurationFileStatus::io_error;
    }

    for(size_t i=0;i<=static_cast<size_t>(n);++i)override_path[i]=path[i];
    size_t resulting_units=0;
    bool path_allocation_failed=false;
    constexpr size_t config_units=(sizeof(config_name)/sizeof(config_name[0]))-1u;
    if(!preloader_l::config::path_detail::join_module_relative(
           path,static_cast<size_t>(n),config_name,config_units,
           path,path_capacity,&resulting_units,&path_allocation_failed)){
        release_path(reinterpret_cast<char*>(path),path_bytes);
        release_path(reinterpret_cast<char*>(override_path),path_bytes);
        return path_allocation_failed
            ? preloader_l::config::RuntimeConfigurationFileStatus::resource_error
            : preloader_l::config::RuntimeConfigurationFileStatus::io_error;
    }

    size_t override_units=0;
    bool override_allocation_failed=false;
    constexpr size_t override_name_units=(sizeof(override_name)/sizeof(override_name[0]))-1u;
    if(!preloader_l::config::path_detail::join_module_relative(
           override_path,static_cast<size_t>(n),override_name,override_name_units,
           override_path,path_capacity,&override_units,&override_allocation_failed)){
        release_path(reinterpret_cast<char*>(path),path_bytes);
        release_path(reinterpret_cast<char*>(override_path),path_bytes);
        return override_allocation_failed
            ? preloader_l::config::RuntimeConfigurationFileStatus::resource_error
            : preloader_l::config::RuntimeConfigurationFileStatus::io_error;
    }
    const auto status=preloader_l::config::load_runtime_configuration_pair_wide(path,override_path);
    release_path(reinterpret_cast<char*>(path),path_bytes);
    release_path(reinterpret_cast<char*>(override_path),path_bytes);
    return status;
}

using preloader_l::config::runtime_command_line_detail::CommandToken;
using preloader_l::config::runtime_command_line_detail::CommandTokenStatus;
using preloader_l::config::runtime_command_line_detail::next_command_token;
using preloader_l::config::runtime_command_line_detail::token_equals;

}

namespace preloader_l { namespace config {

bool query_process_current_directory(char* output, size_t capacity,
                                     size_t* written) noexcept {
    if (written) *written = 0;
    if (!written || capacity > 0xffffffffu || (!output && capacity != 0))
        return false;

    if (!output) {
        const DWORD needed = GetCurrentDirectoryA(0, nullptr);
        if (!needed) return false;
        *written = static_cast<size_t>(needed - 1u);
        return true;
    }
    if (capacity == 0) return false;

    const DWORD n = GetCurrentDirectoryA(static_cast<DWORD>(capacity), output);
    if (!n || n >= capacity) {
        if (capacity) output[0] = 0;
        return false;
    }
    *written = static_cast<size_t>(n);
    return true;
}
bool query_process_emu_game_language(const char** value, size_t* length) noexcept {
    if(value) *value=nullptr;
    if(length) *length=0;
    if(!value || !length) return false;
    const char* cursor=GetCommandLineA();
    if(!cursor) return false;

    CommandToken token{};
    CommandToken selected{};
    // Skip argv[0]. Malformed lexical structure invalidates the query instead
    // of being accepted as a shortened locale value.
    if(next_command_token(&cursor,&token)!=CommandTokenStatus::token) return false;
    for(;;){
        const auto status=next_command_token(&cursor,&token);
        if(status==CommandTokenStatus::end) break;
        if(status==CommandTokenStatus::malformed) return false;
        if(!token_equals(token,"-emuGameLang")) continue;
        CommandToken candidate{};
        if(next_command_token(&cursor,&candidate)!=CommandTokenStatus::token) return false;
        selected=candidate; // repeated exact switches follow behavioral last-write wins
    }
    if(!selected.present) return false;
    *value=selected.data;
    *length=selected.size;
    return true;
}

void synchronize_process_language_policy() noexcept {
    const char* value=nullptr;
    size_t length=0;
    if(query_process_emu_game_language(&value,&length)) {
        set_runtime_language_override(ValueView{value,length,true});
        return;
    }
    set_runtime_language_override(missing_value());
}

bool query_process_current_directory_wide(uint16_t* output, size_t capacity,
                                          size_t* written) noexcept {
    if (written) *written = 0;
    if (!written || capacity > 0xffffffffu || (!output && capacity != 0))
        return false;
    if (!output) {
        const DWORD needed = GetCurrentDirectoryW(0, nullptr);
        if (!needed) return false;
        *written = static_cast<size_t>(needed - 1u);
        return true;
    }
    if (capacity == 0) return false;
    const DWORD n = GetCurrentDirectoryW(static_cast<DWORD>(capacity), reinterpret_cast<LPWSTR>(output));
    if (!n || n >= capacity) {
        output[0] = 0u;
        return false;
    }
    *written = static_cast<size_t>(n);
    return true;
}


ProcessRuntimeConfigurationResult load_process_runtime_configuration() noexcept {
    ProcessRuntimeConfigurationResult result{
        RuntimeConfigurationFileStatus::io_error,
        false,
        0u,
        runtime_configuration_present_sections()
    };

    size_t cwd_length = 0;
    if (query_process_current_directory(nullptr, 0, &cwd_length) &&
        cwd_length < static_cast<size_t>(-1)) {
        const size_t capacity=cwd_length+1u;
        char* cwd=allocate_path(capacity);
        size_t actual=0;
        if(cwd && query_process_current_directory(cwd,capacity,&actual)){
            result.current_directory_available =
                set_runtime_current_directory(cwd, actual) == RuntimeConfigurationStatus::ok;
            result.current_directory_length =
                result.current_directory_available && actual <= 0xffffffffu
                    ? static_cast<uint32_t>(actual) : 0u;
        }
        release_path(cwd,capacity);
    }

    // Active service+0x098 policy is a UTF-16 MSVC wide string. Capture it
    // independently from the narrow route-facing mirror above.
    size_t wide_length = 0;
    if (query_process_current_directory_wide(nullptr, 0, &wide_length) &&
        wide_length <= (static_cast<size_t>(-1) / sizeof(uint16_t)) - 1u) {
        const size_t wide_capacity = wide_length + 1u;
        const size_t wide_bytes = wide_capacity * sizeof(uint16_t);
        auto* wide = reinterpret_cast<uint16_t*>(allocate_path(wide_bytes));
        size_t wide_actual = 0;
        if (wide && query_process_current_directory_wide(wide, wide_capacity, &wide_actual))
            (void)set_runtime_current_directory_wide_policy(wide, wide_actual);
        release_path(reinterpret_cast<char*>(wide), wide_bytes);
    }

    result.file_status = load_module_relative_configuration();
    // Phase1396/1418 policy: CLI installed-language precedence is
    // reverse-published before any shared provider consumer (environment/LSX).
    synchronize_process_language_policy();
    result.present_sections = runtime_configuration_present_sections();
    return result;
}

} }
