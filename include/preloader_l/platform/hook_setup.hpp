// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/config/configuration.hpp"
#include <array>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace preloader_l::host_init {
using Address = std::uintptr_t;
using Values = std::map<std::string, std::u16string, std::less<>>;
enum class Callback : std::uint8_t {
    connect_00fd00, reg_open_a_00f800, reg_open_w_00f960,
    reg_close_00f5b0, reg_query_a_00fac0, reg_query_w_00fb60,
    create_process_a_00f3f0, create_process_w_00f4d0,
    folder_a_00fc00, folder_w_00fc80, crypt_query_001264
};
constexpr std::size_t callback_count = 11;
constexpr std::size_t slot(Callback callback) { return static_cast<std::size_t>(callback); }
struct StateRefs {
    configuration::State& configuration;
    Values& values_0a3828;
    std::vector<Address>& enabled_targets_0a3810;
    // First ten slots preserve original globals 0a3838..0a3880. Slot 10 is the
    // SURGE CryptQueryObject extension, sourced from preloader RVA 0x1264.
    std::array<Address, callback_count>& originals;
};
// Zero-initialized OSVERSIONINFOEXW-equivalent application values. The ABI
// adapter must construct the real 284-byte Windows structure, including padding.
struct VersionRequest {
    std::uint32_t size = 0x11c, major = 6, minor = 2, build = 0, platform = 0;
    std::array<char16_t, 128> service_pack_text{};
    std::uint16_t service_pack_major = 0, service_pack_minor = 0, suite = 0;
    std::uint8_t product_type = 0, reserved = 0;
};
class Effects {
public:
    virtual ~Effects() = default;
    // Filesystem code-page conversion, NOT always UTF-8; existing encoding-source
    // can implement this. Explicit string lengths are preserved.
    virtual std::u16string filesystem_widen_006d70(std::string_view) = 0;
    virtual std::uint64_t version_condition_mask(std::uint64_t, std::uint32_t type,
                                                 std::uint8_t condition) = 0;
    virtual bool verify_version(const VersionRequest&, std::uint32_t type_mask,
                                std::uint64_t condition_mask) = 0;
    // Actual patch engine remains mandatory. Nonzero is failure, including 1
    // (already initialized). No default returns success or installs any hook.
    virtual std::int32_t initialize_engine_046a20() = 0;
    virtual Address get_module_handle(std::u16string_view) = 0; // does not LoadLibrary
    virtual Address get_proc_address(Address module, std::string_view name) = 0;
    // Resolve Callback to a rebuilt callback; never invoke preferred original VAs.
    // original is a reference: writes made by the backend survive a failed return.
    virtual std::int32_t create_hook_0466e0(Address target, Callback, Address& original) = 0;
    virtual std::int32_t enable_hook_046a10(Address target) = 0;
    virtual void log_text_006880(std::string_view) = 0;
    virtual void log_integer_00af90(std::int32_t) = 0;
    virtual void finish_log_0088e0() = 0;
};
// 180046970: returns 11/12 for missing module/symbol. Target output is optional;
// it is assigned before the patch-engine call, and untouched for lookup failures.
std::int32_t create_named_hook_046970(Effects&, std::u16string_view module,
    std::string_view name, Callback, Address& original, Address* target);
// 18000f120: enable only after successful creation; append only after both succeed.
void install_hook_00f120(StateRefs, Effects&, std::u16string_view module,
    std::string_view name, Callback);
// 18000e310: application setup between the first and second bootstrap workers.
void initialize_host_00e310(StateRefs, Effects&);
}
