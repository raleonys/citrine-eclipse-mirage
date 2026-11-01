#pragma once
#include "src/hooks/registry.hpp"
#include <cstddef>
#include <cstdint>

namespace preloader_l::host_callbacks {
using Address = std::uintptr_t;
using Values = registry::Values;
template<class Char> struct ProcessCall {
    const Char* application;
    Char* command_line;
    Address process_attributes, thread_attributes;
    std::int32_t inherit_handles;
    std::uint32_t creation_flags;
    Address environment;
    const Char* current_directory;
    Address startup_info, process_information;
};
template<class Char> struct FolderCall {
    Address window;
    std::int32_t folder_id;
    Address token;
    std::uint32_t flags;
    Char* path;
};
class Effects : public registry::ValueEffects {
public:
    virtual std::int32_t original_create_process_a(const ProcessCall<char>&) = 0;
    virtual std::int32_t original_create_process_w(const ProcessCall<char16_t>&) = 0;
    virtual std::uint32_t original_folder_a(const FolderCall<char>&) = 0;
    virtual std::uint32_t original_folder_w(const FolderCall<char16_t>&) = 0;
};
// Return value is defined by assembly despite Ghidra's inferred void prototype.
// A matching application returns zero; the command line is not inspected.
std::int32_t create_process_a_00f3f0(const Values&, Effects&, const ProcessCall<char>&);
std::int32_t create_process_w_00f4d0(const Values&, Effects&, const ProcessCall<char16_t>&);
// Only full folder_id == 5 is eligible; no CSIDL flag masking. A nonempty
// explicit-length mapping is handled even when its first character is NUL.
// On a handled copy, *result is 0 for CRT success and 1 for any CRT error.
// Otherwise both path and *result remain untouched. Secure CRT invalid-argument
// and undersized-buffer policies belong to the mandatory ValueEffects backend.
bool folder_value_a_008db0(const Values&, registry::ValueEffects&,
    std::int32_t folder_id, char* path, std::uint32_t* result);
bool folder_value_w_008fa0(const Values&, registry::ValueEffects&,
    std::int32_t folder_id, char16_t* path, std::uint32_t* result);
std::uint32_t folder_a_00fc00(const Values&, Effects&, const FolderCall<char>&);
std::uint32_t folder_w_00fc80(const Values&, Effects&, const FolderCall<char16_t>&);
}
