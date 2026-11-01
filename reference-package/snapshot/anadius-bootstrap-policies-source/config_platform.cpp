#include "config_platform.hpp"
#include "../anadius-path-source/paths.hpp"
#include <climits>
#include <exception>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_policies {
namespace {
// Rebuilt compatibility storage is wchar_t for SDK writes; conversion into
// Path copies code units instead of aliasing char16_t buffers as wchar_t.
static_assert(sizeof(wchar_t)==sizeof(char16_t));
struct LastError {
    ConfigPlatformApi api;DWORD value;
    explicit LastError(ConfigPlatformApi a):api(a),value(a.last_error()){}
    ~LastError() noexcept{api.set_last_error(value);}
};
template<class Read>
configuration::Path growing_path(ConfigPlatformApi api,Read read){
    LastError error(api);
    std::wstring buffer;
    for(;;){
        // Retained03c7b0/039bc0 grow by0x104 rather than using the API's
        // returned requirement directly. Guard wrap as an explicit rebuilt
        // extreme-size policy, not a claim about original allocation failure.
        if(buffer.size()>std::numeric_limits<DWORD>::max()-0x104ULL)
            throw std::length_error("Configuration SDK path exceeds DWORD capacity");
        buffer.resize(buffer.size()+0x104,L'\0');
        api.set_last_error(error.value);
        const auto count=read(buffer.data(),static_cast<DWORD>(buffer.size()));
        error.value=api.last_error();
        if(count<buffer.size())
            return configuration::Path(buffer.begin(),buffer.begin()+count);
    }
}
void checked(encoding_recovered::Result r,encoding_recovered::Errors& errors){
    if(r.error){errors.filesystem_00cdb0(r.error);std::terminate();}
}
}
ConfigPlatformApi native_config_platform_api() noexcept{
    return {::GetModuleFileNameW,::GetEnvironmentVariableW,::GetLastError,::SetLastError};
}
namespace configuration_routes {
configuration::Path executable_path(ConfigPlatformApi api){
    return growing_path(api,[&](wchar_t* p,DWORD n){return api.module_filename(nullptr,p,n);});
}
configuration::Path environment_path(ConfigPlatformApi api,const wchar_t* name){
    return growing_path(api,[&](wchar_t* p,DWORD n){return api.environment(name,p,n);});
}
configuration::Path program_data(ConfigPlatformApi api){return environment_path(api,L"ProgramData");}
configuration::Path local_app_data(ConfigPlatformApi api){return environment_path(api,L"LocalAppData");}
configuration::Path widen_utf8(std::string_view text,encoding_recovered::Calls& encoding,encoding_recovered::Errors& errors){
    if(text.empty())return {};
    if(text.size()>INT_MAX){errors.runtime_00cd70(22);std::terminate();}
    const auto required=encoding_recovered::convert_narrow_048ae8(encoding,65001,text.data(),static_cast<int>(text.size()),nullptr,0);
    checked(required,errors);
    configuration::Path result(static_cast<std::size_t>(required.count),u'\0');
    checked(encoding_recovered::convert_narrow_048ae8(encoding,65001,text.data(),static_cast<int>(text.size()),result.data(),required.count),errors);
    return result; // Retained first-pass length survives a shorter second pass.
}
std::string narrow_path(const configuration::Path& path,encoding_recovered::Calls& encoding,encoding_recovered::Errors& errors){
    const auto cp=encoding_recovered::filesystem_codepage_048ac0(encoding);
    return encoding_recovered::narrow_006e60(path,cp,encoding,errors);
}
configuration::Path resolve_path(std::string_view text,const configuration::Path& executable,bool relative,
    encoding_recovered::Calls& encoding,encoding_recovered::Errors& errors){
    auto result=widen_utf8(text,encoding,errors);
    if(relative&&!path_recovered::is_absolute_0195b0(result))
        result=path_recovered::join_016b00(path_recovered::parent_path_019820(executable),result);
    return result;
}
}
ConfigurationPlatform::ConfigurationPlatform(std::unique_ptr<configuration::Dependencies> lower,
    std::unique_ptr<encoding_recovered::Calls> encoding,
    std::unique_ptr<encoding_recovered::Errors> errors,ConfigPlatformApi api)
    :lower_(std::move(lower)),encoding_(std::move(encoding)),errors_(std::move(errors)),api_(api){
    if(!lower_||!encoding_||!errors_||!api.module_filename||!api.environment||!api.last_error||!api.set_last_error)
        throw std::invalid_argument("Configuration platform requires lower policies, encoding, errors and complete SDK API");
}
configuration::Path ConfigurationPlatform::executable_path_03c6d0(){
    return configuration_routes::executable_path(api_);
}
configuration::Path ConfigurationPlatform::environment_path(const wchar_t* name){
    return configuration_routes::environment_path(api_,name);
}
configuration::Path ConfigurationPlatform::program_data_03c920(){return environment_path(L"ProgramData");}
configuration::Path ConfigurationPlatform::local_app_data_03c6f0(){return environment_path(L"LocalAppData");}
configuration::Path ConfigurationPlatform::widen_0145d0(std::string_view text){
    return configuration_routes::widen_utf8(text,*encoding_,*errors_);
}
std::string ConfigurationPlatform::narrow_path(const configuration::Path& path){
    return configuration_routes::narrow_path(path,*encoding_,*errors_);
}
configuration::Path ConfigurationPlatform::resolve_path_0192d0(std::string_view text,const configuration::Path& executable,bool relative){
    return configuration_routes::resolve_path(text,executable,relative,*encoding_,*errors_);
}
configuration::Path ConfigurationPlatform::parent_path_019820(const configuration::Path& p){return lower_->parent_path_019820(p);}
configuration::Path ConfigurationPlatform::join_016b00(const configuration::Path& p,const configuration::Path& q){return lower_->join_016b00(p,q);}
configuration::Path ConfigurationPlatform::filesystem_widen_010540(std::string_view s){return lower_->filesystem_widen_010540(s);}
configuration::Node ConfigurationPlatform::load_document(const configuration::Path& p){return lower_->load_document(p);}
void ConfigurationPlatform::log_config_path(const configuration::Path& p){lower_->log_config_path(p);}
bool ConfigurationPlatform::crt_is_space(unsigned char c){return lower_->crt_is_space(c);}
int ConfigurationPlatform::crt_tolower(int c){return lower_->crt_tolower(c);}
void ConfigurationPlatform::apply_license_key_02a830(std::string_view s){lower_->apply_license_key_02a830(s);}
void ConfigurationPlatform::create_directories_018e00(const configuration::Path& p){lower_->create_directories_018e00(p);}
void ConfigurationPlatform::write_embedded_png_03c020(const configuration::Path& p,configuration::EmbeddedPng i){lower_->write_embedded_png_03c020(p,i);}
std::vector<configuration::Path> ConfigurationPlatform::directory_entries(const configuration::Path& p){return lower_->directory_entries(p);}
void ConfigurationPlatform::visit_directory_entries(const configuration::Path& p,configuration::DirectoryVisitor visitor){lower_->visit_directory_entries(p,visitor);}
configuration::Path ConfigurationPlatform::filename(const configuration::Path& p){return lower_->filename(p);}
void ConfigurationPlatform::remove_04948c(const configuration::Path& p){lower_->remove_04948c(p);}
void ConfigurationPlatform::update_achievement_storage_01eb00(configuration::State& state){lower_->update_achievement_storage_01eb00(state);}
bool ConfigurationPlatform::set_environment_variable(std::string_view k,std::string_view v){return lower_->set_environment_variable(k,v);}
bootstrap_backend::Resources bind_configuration_platform(bootstrap_backend::Resources r,
    std::unique_ptr<encoding_recovered::Calls> calls,std::unique_ptr<encoding_recovered::Errors> errors,ConfigPlatformApi api){
    r.configuration=std::make_unique<ConfigurationPlatform>(std::move(r.configuration),std::move(calls),std::move(errors),api);
    return r;
}
static_assert(!std::is_abstract_v<ConfigurationPlatform>);
} // namespace anadius::bootstrap_policies
