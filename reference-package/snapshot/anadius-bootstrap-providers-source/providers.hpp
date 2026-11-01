#pragma once
#include "../anadius-bootstrap-runtime-source/factory.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include <cstdio>

namespace anadius::bootstrap_providers {
struct ConfigApi {
    int (*space)(int);
    int (*lower)(int);
    decltype(&::SetEnvironmentVariableA) environment;
};
ConfigApi native_config_api() noexcept;
namespace configuration_routes {
// Lower-free leaves shared by the decorator and direct owning factory. Callers
// provide a complete API and serialize compiler-locale/environment operations.
bool is_space(ConfigApi,unsigned char);
int checked_tolower(ConfigApi,int);
bool set_environment(ConfigApi,std::string_view,std::string_view);
}
// Seven selected path/encoding/compiler-CRT/SDK routes. The other fourteen
// application/OS policies remain explicit in the owned lower provider.
class Configuration final:public configuration::Dependencies {
public:
    Configuration(std::unique_ptr<configuration::Dependencies>,
        std::unique_ptr<encoding_recovered::Calls> =std::make_unique<encoding_recovered::NativeCalls>(),
        std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>(),
        ConfigApi=native_config_api());
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
    std::unique_ptr<encoding_recovered::Errors> errors_;ConfigApi api_;
};
struct ConsoleApi {
    decltype(&::AttachConsole) attach;
    decltype(&::AllocConsole) allocate;
    decltype(&::GetConsoleWindow) window;
    decltype(&::MoveWindow) move;
    decltype(&::freopen_s) reopen;
};
ConsoleApi native_console_api() noexcept;
std::array<FILE*,3> compiler_streams() noexcept;
// Rebuilt compiler-CRT policy, NOT original 0x58-byte MSVC FILE ABI. Source
// stream identities become opaque slot tokens; no original FILE bytes are read
// or copied, and freopen_s receives a separate valid FILE* output cell.
// The containing object is immovable so tokens stay stable. Caller serializes
// console use and compiler process-wide stream changes; no implicit restoration.
class CompilerConsole final:public startup_recovered::ConsolePlatform {
public:
    explicit CompilerConsole(ConsoleApi=native_console_api(),std::array<FILE*,3> =compiler_streams());
    CompilerConsole(const CompilerConsole&)=delete;CompilerConsole& operator=(const CompilerConsole&)=delete;
    bool attach_console(std::uint32_t) override;
    bool allocate_console() override;
    startup_recovered::ConsoleHandle console_window() override;
    bool move_window(startup_recovered::ConsoleHandle,int,int,int,int,bool) override;
    std::uintptr_t stream_052238(unsigned) override;
    int reopen_05f024(std::uintptr_t,const char*,const char*,std::uintptr_t) override;
private:ConsoleApi api_;std::array<FILE*,3> streams_;
};
// Adds the selected configuration source routes, and supplies compiler-console
// only if absent. Explicit console is retained. No callback/worker/IO is started.
bootstrap_backend::Resources bind_configuration_and_console(bootstrap_backend::Resources,
    std::unique_ptr<encoding_recovered::Calls> =std::make_unique<encoding_recovered::NativeCalls>(),
    std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>(),
    ConfigApi=native_config_api(),ConsoleApi=native_console_api(),std::array<FILE*,3> =compiler_streams());
} // namespace anadius::bootstrap_providers
