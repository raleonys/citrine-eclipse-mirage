#include "providers.hpp"
#include "../anadius-path-source/paths.hpp"
#include <cctype>
#include <climits>
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_providers {
ConfigApi native_config_api() noexcept{return {&::isspace,&::tolower,&::SetEnvironmentVariableA};}
namespace configuration_routes {
bool is_space(ConfigApi api,unsigned char c){return api.space(c)!=0;}
int checked_tolower(ConfigApi api,int c){
    // Calling the compiler C tolower on arbitrary negative signed chars is UB.
    // No guessed MSVC multibyte behavior is substituted for that open domain.
    if(c<EOF||c>UCHAR_MAX)throw std::domain_error("Signed non-EOF byte requires original CRT locale policy");
    return api.lower(c);
}
bool set_environment(ConfigApi api,std::string_view key,std::string_view value){
    const std::string k(key),v(value);return api.environment(k.c_str(),v.c_str())!=FALSE;
}
}
Configuration::Configuration(std::unique_ptr<configuration::Dependencies> lower,
    std::unique_ptr<encoding_recovered::Calls> encoding,std::unique_ptr<encoding_recovered::Errors> errors,ConfigApi api)
    :lower_(std::move(lower)),encoding_(std::move(encoding)),errors_(std::move(errors)),api_(api){
    if(!lower_||!encoding_||!errors_||!api.space||!api.lower||!api.environment)
        throw std::invalid_argument("Configuration policies, encoding and complete SDK/CRT API required");
}
configuration::Path Configuration::executable_path_03c6d0(){return lower_->executable_path_03c6d0();}
configuration::Path Configuration::parent_path_019820(const configuration::Path& p){return path_recovered::parent_path_019820(p);}
configuration::Path Configuration::join_016b00(const configuration::Path& a,const configuration::Path& b){return path_recovered::join_016b00(a,b);}
configuration::Path Configuration::widen_0145d0(std::string_view s){return lower_->widen_0145d0(s);}
configuration::Path Configuration::filesystem_widen_010540(std::string_view s){return encoding_recovered::filesystem_widen_006d70(s,*encoding_,*errors_);}
std::string Configuration::narrow_path(const configuration::Path& p){return lower_->narrow_path(p);}
configuration::Path Configuration::resolve_path_0192d0(std::string_view s,const configuration::Path& p,bool r){return lower_->resolve_path_0192d0(s,p,r);}
configuration::Path Configuration::program_data_03c920(){return lower_->program_data_03c920();}
configuration::Path Configuration::local_app_data_03c6f0(){return lower_->local_app_data_03c6f0();}
configuration::Node Configuration::load_document(const configuration::Path& p){return lower_->load_document(p);}
void Configuration::log_config_path(const configuration::Path& p){lower_->log_config_path(p);}
bool Configuration::crt_is_space(unsigned char c){return configuration_routes::is_space(api_,c);}
int Configuration::crt_tolower(int c){return configuration_routes::checked_tolower(api_,c);}
void Configuration::apply_license_key_02a830(std::string_view s){lower_->apply_license_key_02a830(s);}
void Configuration::create_directories_018e00(const configuration::Path& p){lower_->create_directories_018e00(p);}
void Configuration::write_embedded_png_03c020(const configuration::Path& p,configuration::EmbeddedPng i){lower_->write_embedded_png_03c020(p,i);}
std::vector<configuration::Path> Configuration::directory_entries(const configuration::Path& p){return lower_->directory_entries(p);}
void Configuration::visit_directory_entries(const configuration::Path& p,configuration::DirectoryVisitor visitor){lower_->visit_directory_entries(p,visitor);}
configuration::Path Configuration::filename(const configuration::Path& p){return path_recovered::filename_00e120(p);}
void Configuration::remove_04948c(const configuration::Path& p){lower_->remove_04948c(p);}
void Configuration::update_achievement_storage_01eb00(configuration::State& s){lower_->update_achievement_storage_01eb00(s);}
bool Configuration::set_environment_variable(std::string_view key,std::string_view value){
    return configuration_routes::set_environment(api_,key,value);
}
ConsoleApi native_console_api() noexcept{return {::AttachConsole,::AllocConsole,::GetConsoleWindow,::MoveWindow,::freopen_s};}
std::array<FILE*,3> compiler_streams() noexcept{return {stdin,stdout,stderr};}
CompilerConsole::CompilerConsole(ConsoleApi api,std::array<FILE*,3> streams):api_(api),streams_(streams){
    if(!api.attach||!api.allocate||!api.window||!api.move||!api.reopen||!streams[0]||!streams[1]||!streams[2])
        throw std::invalid_argument("Complete compiler-console API and three streams required");
}
bool CompilerConsole::attach_console(std::uint32_t id){return api_.attach(id)!=FALSE;}
bool CompilerConsole::allocate_console(){return api_.allocate()!=FALSE;}
startup_recovered::ConsoleHandle CompilerConsole::console_window(){return reinterpret_cast<std::uintptr_t>(api_.window());}
bool CompilerConsole::move_window(startup_recovered::ConsoleHandle h,int x,int y,int w,int height,bool repaint){return api_.move(reinterpret_cast<HWND>(h),x,y,w,height,repaint)!=FALSE;}
std::uintptr_t CompilerConsole::stream_052238(unsigned i){
    if(i>=streams_.size())throw std::out_of_range("Only compiler stdin/stdout/stderr are bound");
    return reinterpret_cast<std::uintptr_t>(&streams_[i]);
}
int CompilerConsole::reopen_05f024(std::uintptr_t output,const char* filename,const char* mode,std::uintptr_t source){
    // Recovered caller passes the same stream identity twice. Preserve that
    // condition but re-express its unknown FILE**/FILE* alias in rebuilt CRT.
    if(output!=source)throw std::invalid_argument("Reopen requires the observed identical stream identities");
    for(auto& stream:streams_)if(source==reinterpret_cast<std::uintptr_t>(&stream)){
        if(!stream)throw std::logic_error("Compiler stream was closed by a previous failed reopen");
        return api_.reopen(&stream,filename,mode,stream);
    }
    throw std::invalid_argument("Foreign compiler stream token");
}
bootstrap_backend::Resources bind_configuration_and_console(bootstrap_backend::Resources r,
    std::unique_ptr<encoding_recovered::Calls> calls,std::unique_ptr<encoding_recovered::Errors> errors,
    ConfigApi config,ConsoleApi console,std::array<FILE*,3> streams){
    r.configuration=std::make_unique<Configuration>(std::move(r.configuration),std::move(calls),std::move(errors),config);
    if(!r.console)r.console=std::make_unique<CompilerConsole>(console,streams);
    return r;
}
static_assert(!std::is_abstract_v<Configuration>&&!std::is_abstract_v<CompilerConsole>);
static_assert(!std::is_move_constructible_v<CompilerConsole>);
} // namespace anadius::bootstrap_providers
