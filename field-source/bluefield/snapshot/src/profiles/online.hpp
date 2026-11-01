#pragma once
#include "src/configuration/state.hpp"
#include "src/contracts/status.hpp"
#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace preloader_l::online {
using Path = configuration::Path;
using Module = std::uintptr_t;
using Object = std::uintptr_t;
using Procedure = std::uintptr_t;
struct OutOfEvidenceInput : std::invalid_argument { using std::invalid_argument::invalid_argument; };

// Caller-owned state: construction invents no original global initializer.
struct StateRefs {
    Path& executable_path_0a39e0;
    Path& cookie_0a3a20;
    Path& last_profile_0a3a40;
    Path& last_avatar_0a3a60;
    Path& avatar_0a3a90;
    Module& module_0a3a80;
    Object& object_0a3a88;
    std::string& username_0a3bd8;
    std::string& persona_id_0a3bf8;
    std::string& user_id_0a3c18;
    std::uint8_t& use_last_profile_0a3c38;
    std::uint8_t& online_0a3c3b;
};

// Explicit platform/application boundaries. Procedure and object tokens are
// opaque; this source never casts or invokes unknown DLL addresses itself.
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint32_t suppress_errors_0450a0() = 0;
    virtual void restore_error_mode(std::uint32_t) = 0;
    virtual Path filesystem_widen_006d70(std::string_view) = 0;
    virtual Path parent_path_019820(const Path&) = 0;
    virtual Path join_016b00(const Path&, const Path&) = 0;
    virtual Module load_copy_0450d0(const Path&) = 0;
    virtual Procedure get_proc_address(Module, std::string_view) = 0;
    virtual std::int32_t call_version_check(Procedure) = 0;
    virtual Object call_create_online(Procedure, const char16_t*) = 0;
    virtual std::int32_t status_virtual_08(Object) = 0;
    virtual const char* username_virtual_10(Object) = 0;
    virtual const char* persona_virtual_18(Object) = 0;
    virtual const char* user_id_virtual_20(Object) = 0;
    virtual void destroy_virtual_00(Object) = 0;
    virtual bool free_library(Module) = 0;
    virtual status_recovered::Status status_01ea20(const Path&, std::uint32_t flags) = 0;
    [[noreturn]] virtual void filesystem_error_018130(std::string_view operation,
        std::uint32_t code, const Path&) = 0;
    virtual std::string read_binary_03b3f0(const Path&) = 0;
    virtual bool write_binary_03c1d0(const Path&, std::string_view) = 0;
    virtual void log_literal_006880(std::string_view) = 0;
    virtual void log_finish_0088e0() = 0;
    virtual void dialog_038bc0(std::string_view title, std::string_view text,
                               std::uint32_t flags) = 0;
    virtual void wait_steps_036880(std::int32_t count) = 0;
    virtual void finish_flag_036820() = 0; // original flag at 1800a3c3c
};

bool exists_035b00(const Path&, Effects&); // regular-file test, not generic exists
// The unlimited semicolon specialization used by 035bd0. Empty fields and
// embedded NUL bytes are retained; there is always a final field.
std::vector<std::string> split_profile_fields_03cf00(std::string_view);
void load_online_035bd0(StateRefs, Effects&);
void cleanup_0366f0(StateRefs, Effects&);
void worker_035b60(StateRefs, Effects&);
} // namespace preloader_l::online
