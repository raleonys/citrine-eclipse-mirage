// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/configuration/state.hpp"
#include "src/runtime/runtime_paths.hpp"
#include "src/application/logging.hpp"
#include "src/contracts/status.hpp"
#include "src/paths/paths.hpp"
#include <filesystem>
#include <mutex>
#include <string>
#include <string_view>
namespace preloader_l {
using Path=configuration::Path;
struct Environment {
    const RuntimePaths paths;
    const std::filesystem::path& executable;
    const std::filesystem::path& configuration_file;
    const std::filesystem::path& data_root;
    const std::filesystem::path& program_data;
    const std::filesystem::path profile_root;
    std::mutex log_mutex;
    explicit Environment(RuntimePaths value):paths(std::move(value)),executable(paths.executable()),configuration_file(paths.config()),data_root(paths.data()),program_data(paths.program_data()),profile_root(paths.profile_cache()){}
    void log(LogEvent) noexcept;
};
std::filesystem::path native_path(const Path&);
Path model_path(const std::filesystem::path&);
Path utf8_path(std::string_view);
Path filesystem_path(std::string_view);
std::string path_utf8(const Path&);
status_recovered::Status file_status(const Path&,std::uint32_t flags=3);
std::string read_file(const Path&,bool required=false);
void publish_environment(std::string_view,std::string_view);
#ifdef _WIN32
using PathEffects=paths::WindowsEffects;
#else
// Preserve the existing POSIX fixture projection. It is not a Windows oracle.
struct HostPathEffects {
    static Path parent(const Path&);
    static Path filename(const Path&);
    static Path join(const Path&,const Path&);
    static Path resolve(const Path&,const Path&,bool);
};
using PathEffects=HostPathEffects;
#endif
}
