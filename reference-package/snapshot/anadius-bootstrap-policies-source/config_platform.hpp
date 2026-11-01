#pragma once
#include "../anadius-bootstrap-providers-source/providers.hpp"

namespace anadius::bootstrap_policies {
struct ConfigPlatformApi {
    decltype(&::GetModuleFileNameW) module_filename;
    decltype(&::GetEnvironmentVariableW) environment;
    decltype(&::GetLastError) last_error;
    decltype(&::SetLastError) set_last_error;
};
ConfigPlatformApi native_config_platform_api() noexcept;
namespace configuration_routes {
// Lower-free selected routes. Complete SDK/encoding providers remain owned by
// the caller; use is serialized, with no factory-creator thread affinity.
configuration::Path executable_path(ConfigPlatformApi);
configuration::Path environment_path(ConfigPlatformApi,const wchar_t*);
configuration::Path program_data(ConfigPlatformApi);
configuration::Path local_app_data(ConfigPlatformApi);
configuration::Path widen_utf8(std::string_view,encoding_recovered::Calls&,encoding_recovered::Errors&);
std::string narrow_path(const configuration::Path&,encoding_recovered::Calls&,encoding_recovered::Errors&);
configuration::Path resolve_path(std::string_view,const configuration::Path&,bool,
    encoding_recovered::Calls&,encoding_recovered::Errors&);
}
// Six additional configuration routes. Place beneath B5 Configuration to keep
// its seven selected routes. All other policies retain their owned lower
// implementation. No file read/write, environment mutation or callback occurs
// at construction. SDK callbacks must obey their nonthrowing Win32 contract.
// ProgramData and LocalAppData are environment variables in retained code;
// shell known-folder discovery or fallback paths would change that behavior.
class ConfigurationPlatform final:public configuration::Dependencies {
public:
    ConfigurationPlatform(std::unique_ptr<configuration::Dependencies>,
        std::unique_ptr<encoding_recovered::Calls> =std::make_unique<encoding_recovered::NativeCalls>(),
        std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>(),
        ConfigPlatformApi=native_config_platform_api());
    configuration::Path executable_path_03c6d0() override;
    configuration::Path parent_path_019820(const configuration::Path&) override;
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&) override;
    configuration::Path widen_0145d0(std::string_view) override;
    configuration::Path filesystem_widen_010540(std::string_view) override;
    std::string narrow_path(const configuration::Path&) override;
    configuration::Path resolve_path_0192d0(std::string_view,const configuration::Path&,bool) override;
    configuration::Path program_data_03c920() override;
    configuration::Path local_app_data_03c6f0() override;
    configuration::Node load_document(const configuration::Path&) override;
    void log_config_path(const configuration::Path&) override;
    bool crt_is_space(unsigned char) override;
    int crt_tolower(int) override;
    void apply_license_key_02a830(std::string_view) override;
    void create_directories_018e00(const configuration::Path&) override;
    void write_embedded_png_03c020(const configuration::Path&,configuration::EmbeddedPng) override;
    std::vector<configuration::Path> directory_entries(const configuration::Path&) override;
    void visit_directory_entries(const configuration::Path&,configuration::DirectoryVisitor) override;
    configuration::Path filename(const configuration::Path&) override;
    void remove_04948c(const configuration::Path&) override;
    void update_achievement_storage_01eb00(configuration::State&) override;
    bool set_environment_variable(std::string_view,std::string_view) override;
private:
    std::unique_ptr<configuration::Dependencies> lower_;
    std::unique_ptr<encoding_recovered::Calls> encoding_;
    std::unique_ptr<encoding_recovered::Errors> errors_;
    ConfigPlatformApi api_;
    configuration::Path environment_path(const wchar_t*);
};
bootstrap_backend::Resources bind_configuration_platform(bootstrap_backend::Resources,
    std::unique_ptr<encoding_recovered::Calls> =std::make_unique<encoding_recovered::NativeCalls>(),
    std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>(),
    ConfigPlatformApi=native_config_platform_api());
} // namespace anadius::bootstrap_policies
