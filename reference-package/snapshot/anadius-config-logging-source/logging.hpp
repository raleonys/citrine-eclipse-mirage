#pragma once
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include <memory>
#include <ostream>

namespace anadius::config_logging {
// Lower-free selected019ad0 logging leaf. The caller owns all arguments and
// serializes stream use. Parsing prefix is inserted before conversion; no
// shared-log Lease spans the conversion provider call.
void log_config_path(std::ostream&,const configuration::Path&,
    encoding_recovered::Calls&,encoding_recovered::Errors&);
// Selected source counterpart of the019ad0 logging sequence. Own the supplied
// stream and its streambuf for the entire lifetime. The configuration controller
// serializes calls and all other uses of this stream (including B10 logging).
// No native MSVC stream layout or arbitrary original VA is adopted.
class ConfigurationLogging final:public configuration::Dependencies {
public:
    ConfigurationLogging(std::shared_ptr<std::ostream>,std::unique_ptr<configuration::Dependencies>,
        std::unique_ptr<encoding_recovered::Calls> =std::make_unique<encoding_recovered::NativeCalls>(),
        std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>());
    ConfigurationLogging(const ConfigurationLogging&)=delete;
    ConfigurationLogging& operator=(const ConfigurationLogging&)=delete;
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
    // Reverse destruction retains the stream/conversion providers until after
    // the owned lower provider's destructor. No I/O is performed at construction.
    std::shared_ptr<std::ostream> stream_;
    std::unique_ptr<encoding_recovered::Calls> calls_;
    std::unique_ptr<encoding_recovered::Errors> errors_;
    std::unique_ptr<configuration::Dependencies> lower_;
};
bootstrap_backend::Resources bind_configuration_logging(bootstrap_backend::Resources,
    std::shared_ptr<std::ostream>,
    std::unique_ptr<encoding_recovered::Calls> =std::make_unique<encoding_recovered::NativeCalls>(),
    std::unique_ptr<encoding_recovered::Errors> =std::make_unique<encoding_recovered::PortableErrors>());
} // namespace anadius::config_logging
