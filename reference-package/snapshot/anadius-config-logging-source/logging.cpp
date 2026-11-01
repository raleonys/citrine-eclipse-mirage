#include "logging.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include "../anadius-path-source/paths.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::config_logging {
ConfigurationLogging::ConfigurationLogging(std::shared_ptr<std::ostream> stream,
    std::unique_ptr<configuration::Dependencies> lower,
    std::unique_ptr<encoding_recovered::Calls> calls,std::unique_ptr<encoding_recovered::Errors> errors)
    :stream_(std::move(stream)),calls_(std::move(calls)),errors_(std::move(errors)),lower_(std::move(lower)){
    if(!stream_||!calls_||!errors_||!lower_)
        throw std::invalid_argument("Owned stream, lower configuration and conversion providers are required");
}
configuration::Path ConfigurationLogging::executable_path_03c6d0(){return lower_->executable_path_03c6d0();}
configuration::Path ConfigurationLogging::program_data_03c920(){return lower_->program_data_03c920();}
configuration::Path ConfigurationLogging::local_app_data_03c6f0(){return lower_->local_app_data_03c6f0();}
configuration::Path ConfigurationLogging::widen_0145d0(std::string_view s){return lower_->widen_0145d0(s);}
std::string ConfigurationLogging::narrow_path(const configuration::Path& p){return lower_->narrow_path(p);}
configuration::Path ConfigurationLogging::resolve_path_0192d0(std::string_view s,const configuration::Path& p,bool relative){return lower_->resolve_path_0192d0(s,p,relative);}
configuration::Path ConfigurationLogging::parent_path_019820(const configuration::Path& p){return lower_->parent_path_019820(p);}
configuration::Path ConfigurationLogging::join_016b00(const configuration::Path& p,const configuration::Path& q){return lower_->join_016b00(p,q);}
configuration::Path ConfigurationLogging::filesystem_widen_010540(std::string_view s){return lower_->filesystem_widen_010540(s);}
configuration::Node ConfigurationLogging::load_document(const configuration::Path& p){return lower_->load_document(p);}
void log_config_path(std::ostream& stream,const configuration::Path& path,
    encoding_recovered::Calls& calls,encoding_recovered::Errors& errors){
    //019d7f..019dc7: prefix insertion precedes filename extraction/conversion.
    // A failed conversion must leave the already-written prefix visible.
    {const auto shared=shared_log::acquire(stream);stream<<"Parsing ";} // exact retained C string at0x180083940
    const auto filename=path_recovered::filename_00e120(path);
    const auto utf8=encoding_recovered::path_utf8_01e3f0(filename,calls,errors);
    //006860 ->007990 is a counted, formatted string insertion. Embedded NUL
    // and first-pass-sized trailing zero bytes remain data, not C-string ends.
    const auto shared=shared_log::acquire(stream);
    stream<<utf8;
    stream<<std::endl; //0088e0: widened newline, put, flush
}
void ConfigurationLogging::log_config_path(const configuration::Path& path){config_logging::log_config_path(*stream_,path,*calls_,*errors_);}
bool ConfigurationLogging::crt_is_space(unsigned char c){return lower_->crt_is_space(c);}
int ConfigurationLogging::crt_tolower(int c){return lower_->crt_tolower(c);}
void ConfigurationLogging::apply_license_key_02a830(std::string_view s){lower_->apply_license_key_02a830(s);}
void ConfigurationLogging::create_directories_018e00(const configuration::Path& p){lower_->create_directories_018e00(p);}
void ConfigurationLogging::write_embedded_png_03c020(const configuration::Path& p,configuration::EmbeddedPng i){lower_->write_embedded_png_03c020(p,i);}
std::vector<configuration::Path> ConfigurationLogging::directory_entries(const configuration::Path& p){return lower_->directory_entries(p);}
void ConfigurationLogging::visit_directory_entries(const configuration::Path& p,configuration::DirectoryVisitor visitor){lower_->visit_directory_entries(p,visitor);}
configuration::Path ConfigurationLogging::filename(const configuration::Path& p){return lower_->filename(p);}
void ConfigurationLogging::remove_04948c(const configuration::Path& p){lower_->remove_04948c(p);}
void ConfigurationLogging::update_achievement_storage_01eb00(configuration::State& state){lower_->update_achievement_storage_01eb00(state);}
bool ConfigurationLogging::set_environment_variable(std::string_view k,std::string_view v){return lower_->set_environment_variable(k,v);}
bootstrap_backend::Resources bind_configuration_logging(bootstrap_backend::Resources r,
    std::shared_ptr<std::ostream> stream,std::unique_ptr<encoding_recovered::Calls> calls,
    std::unique_ptr<encoding_recovered::Errors> errors){
    r.configuration=std::make_unique<ConfigurationLogging>(std::move(stream),std::move(r.configuration),std::move(calls),std::move(errors));
    return r;
}
static_assert(!std::is_abstract_v<ConfigurationLogging>);
static_assert(!std::is_copy_constructible_v<ConfigurationLogging>);
} // namespace anadius::config_logging
