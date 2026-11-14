// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/runtime/online.hpp"
#include "laureondo/runtime/libraries.hpp"
#include "laureondo/codec/encoding.hpp"
#include "laureondo/core/binary_io.hpp"

namespace laureondo::online {
// Source composition, not an independently verified original function. The caller selects
// every backend. This class has no native DLL loading/provider implementation.
class DelegatingEffects final : public Effects {
public:
    DelegatingEffects(Effects& remaining, system_services::libraries::SystemCalls& libraries,
        status::Stats& stats, encoding::Calls& encoding,
        encoding::Errors& errors, file::Factory& files)
        : remaining_(remaining), libraries_(libraries), stats_(stats),
          encoding_(encoding), errors_(errors), files_(files) {}
    std::uint32_t suppress_errors_0450a0() override;
    void restore_error_mode(std::uint32_t) override;
    Path filesystem_widen_006d70(std::string_view) override;
    Path parent_path_019820(const Path&) override;
    Path join_016b00(const Path&, const Path&) override;
    Module load_copy_0450d0(const Path&) override;
    Procedure get_proc_address(Module, std::string_view) override;
    std::int32_t call_version_check(Procedure) override;
    Object call_create_online(Procedure, const char16_t*) override;
    std::int32_t status_virtual_08(Object) override;
    const char* username_virtual_10(Object) override;
    const char* persona_virtual_18(Object) override;
    const char* user_id_virtual_20(Object) override;
    void destroy_virtual_00(Object) override;
    bool free_library(Module) override;
    status::Status status_01ea20(const Path&, std::uint32_t) override;
    [[noreturn]] void filesystem_error_018130(std::string_view, std::uint32_t, const Path&) override;
    std::string read_binary_03b3f0(const Path&) override;
    bool write_binary_03c1d0(const Path&, std::string_view) override;
    void log_literal_006880(std::string_view) override;
    void log_finish_0088e0() override;
    void dialog_038bc0(std::string_view, std::string_view, std::uint32_t) override;
    void wait_steps_036880(std::int32_t) override;
    void finish_flag_036820() override;
private:
    Effects& remaining_;
    system_services::libraries::SystemCalls& libraries_;
    status::Stats& stats_;
    encoding::Calls& encoding_;
    encoding::Errors& errors_;
    file::Factory& files_;
};
} // namespace laureondo::online
