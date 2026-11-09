// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/core/binary_io.hpp"
#include "citrine_eclipse_mirage/hash/hash.hpp"
#include "citrine_eclipse_mirage/core/license.hpp"

namespace citrine_eclipse_mirage::file {
// Compatibility implementation using ambient C++ stream locale and C tolower.
// Signed bytes below -1 are rejected as outside the C character API's defined
// domain; hash core itself preserves original signed-byte promotion exactly.
class NativeTextEnvironment final:public hash::TextEnvironment {
public:
    std::string format_digest_043270(const hash::DigestWords&)override;
    std::int32_t tolower_05fca0(std::int32_t)override;
};
// New composition adapter, not a separately identified original function.
// Three file operations are implemented locally; ALL remaining effects
// delegate to caller-supplied implementations, with no successful defaults.
class LicenseFiles final:public license::Effects {
public:
    LicenseFiles(license::Effects& remaining,Factory& files,
                 hash::TextEnvironment& text):remaining_(remaining),files_(files),text_(text){}
    const char* online_auth_virtual_30(const char*)override;
    license::FileStatus status_01ea20(const Path&,std::uint32_t)override;
    [[noreturn]] void filesystem_error_018130(std::string_view,std::int32_t,const Path&)override;
    bool file_hash_matches_042d20(const Path&,std::string)override;
    std::string load_token_02afa0(const Path&)override;
    Path filesystem_widen_006d70(std::string_view)override;
    Path filename_00e120(const Path&)override;
    Path parent_path_019820(const Path&)override;
    Path join_016b00(const Path&,const Path&)override;
    std::string path_utf8_01e3f0(const Path&)override;
    std::string current_time_041b60_null_true()override;
    bool wine_detected_045010()override;
    void log_literal_006880(std::string_view)override;
    void log_string_006860(std::string_view)override;
    bool write_ticket_03c1d0(const Path&,std::string_view)override;
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override;
    [[noreturn]] void exit_05cdc0(std::int32_t)override;
    [[noreturn]] void no_response_02bcd0_04d228()override;
private:
    license::Effects& remaining_;
    Factory& files_;
    hash::TextEnvironment& text_;
};
} // namespace citrine_eclipse_mirage::file
