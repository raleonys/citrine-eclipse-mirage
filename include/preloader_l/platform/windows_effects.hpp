// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/core/license.hpp"
#include "preloader_l/runtime/status.hpp"
#include "preloader_l/codec/encoding.hpp"
#include "preloader_l/core/paths.hpp"

namespace preloader_l::windows {
// New source composition layer. Supplies recovered status/path/encoding and
// delegates every other application effect. Compose as LicenseFiles(inner,...)
// so token status lookups also reach this inner WindowsEffects object.
class WindowsEffects final:public license::Effects {
public:
    using Path=configuration::Path;
    WindowsEffects(license::Effects& remaining,status::Stats& stats,
                   encoding::Calls& encoding,encoding::Errors& errors)
        :remaining_(remaining),stats_(stats),encoding_(encoding),errors_(errors){}
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
    status::Stats& stats_;
    encoding::Calls& encoding_;
    encoding::Errors& errors_;
};
} // namespace preloader_l::windows
