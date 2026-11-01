// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/runtime/path_kind.hpp"
#include <filesystem>
#include <string_view>
#include <string>
namespace preloader_l {
class RuntimePaths final {
    std::filesystem::path module_,executable_,local_,data_,program_,config_;
    RuntimePaths(std::filesystem::path,std::filesystem::path,std::filesystem::path,std::filesystem::path,std::filesystem::path);
public:
    static RuntimePaths from_system(std::filesystem::path module,std::filesystem::path executable,std::filesystem::path local_app_data,std::filesystem::path program_data);
    // Explicit synthetic roots for standalone test hosts; never an environment fallback.
    static RuntimePaths fixture(std::filesystem::path executable,std::filesystem::path config,std::filesystem::path local_app_data,std::filesystem::path program_data);
    const std::filesystem::path& executable() const noexcept{return executable_;}
    const std::filesystem::path& module() const noexcept{return module_;}
    const std::filesystem::path& config() const noexcept{return config_;}
    const std::filesystem::path& data() const noexcept{return data_;}
    const std::filesystem::path& program_data() const noexcept{return program_;}
    const std::filesystem::path& local_app_data() const noexcept{return local_;}
    std::filesystem::path override_config() const;
    std::filesystem::path provider() const;
    std::filesystem::path sidecar_avatar() const;
    std::filesystem::path default_avatar() const;
    std::filesystem::path profile_cache() const;
    std::filesystem::path diagnostic_log() const;
    std::filesystem::path file(RuntimeFile) const;
    std::u16string model(RuntimeFile f) const{return file(f).u16string();}
    std::filesystem::path avatar_cache(std::string_view owner,std::string_view width) const;
    bool owns(const std::filesystem::path&) const;
    bool external_contract(const std::filesystem::path&) const;
    void require_write(const std::filesystem::path&) const;
};
}
