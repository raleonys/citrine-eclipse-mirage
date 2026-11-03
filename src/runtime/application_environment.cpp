// SPDX-License-Identifier: GPL-3.0-only

#include "application_internal.hpp"
#include "preloader_l/config/application.hpp"
#include "preloader_l/core/paths.hpp"
#include "preloader_l/core/product_identity.hpp"
#include "preloader_l/platform/file_copy.hpp"
#include "preloader_l/codec/crypto.hpp"
#include "preloader_l/core/token_constants.hpp"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <system_error>
#include <limits>
#include "avatar_data.hpp"

namespace preloader_l::application::detail {
namespace {
Path environment_path(const wchar_t* name) {
    // RE039bc0 grows a UTF16 buffer in260-code-unit steps and reads the
    // environment. No Shell known-folder allocation or COM allocator is used.
    std::wstring value(260,L'\0');
    for(;;) {
        const auto size=GetEnvironmentVariableW(name,value.data(),static_cast<DWORD>(value.size()));
        if(size<value.size()){value.resize(size);return utf16(value);}
        value.resize(value.size()+260);
    }
}
}
void Events::construct_mutex_04a698(content_handlers::Mutex& supplied,std::uint32_t flags) {
    if(&supplied!=&mutex||flags!=2)throw std::invalid_argument("Event owner mutex identity/type");
    // The RAII mutex already exists and exposes precisely the RE type2 contract.
}
int Events::register_atexit(event_queue::Cleanup cleanup) {
    // Owned storage/mutex destruction replaces CRT registrations, preserving
    // reverse member destruction after all handler/IO workers have joined.
    if(cleanup!=event_queue::Cleanup::mutex_07e030&&cleanup!=event_queue::Cleanup::queue_07e000)
        throw std::invalid_argument("Unknown event cleanup");
    return 0;
}
Environment::Environment(configuration::State& s,Options& o,file::NativeFactory& files,
        xml_reader_bridge::Backend& xml,achievement_storage::Paths& paths,
        achievement_runtime::HostClock& clock,achievement_runtime::HostFormatter& formatter)
    :PersistenceEffects(s,paths,clock,formatter,*this,files),state_(s),options_(o),files_(files),
     clock_(clock),formatter_(formatter),storage_(paths,xml,numbers_,*this,files) {
    // Constructor036d20 param_1[0x27] pointed to these five little-endian qwords.
    constexpr std::uint64_t words[]={0x3542434c35564753ULL,0x304e484973553362ULL,0x6e6c6d6367413362ULL,0x6c68476467514861ULL,0x7073444968556d63ULL};
    for(auto word:words)for(unsigned i=0;i<8;++i)auth_code.push_back(static_cast<char>(word>>(i*8)));
}
Path Environment::executable_path_03c6d0() {
    if(!options_.executable_path.empty())return options_.executable_path;
    std::wstring buffer(512,L'\0');
    for(;;){const auto n=GetModuleFileNameW(nullptr,buffer.data(),static_cast<DWORD>(buffer.size()));
        if(!n)throw std::system_error(GetLastError(),std::system_category(),"GetModuleFileNameW");
        if(n<buffer.size()){buffer.resize(n);return utf16(buffer);}buffer.resize(buffer.size()*2);}
}
Path Environment::filesystem_widen_006d70(std::string_view v){return encoding::filesystem_widen_006d70(v,encoding_,errors_);}
Path Environment::filesystem_widen_010540(std::string_view v){return filesystem_widen_006d70(v);}
Path Environment::widen_0145d0(std::string_view v) {
    if(v.size()>static_cast<std::size_t>(std::numeric_limits<int>::max()))errors_.runtime_00cd70(22);
    if(v.empty())return {};auto result=encoding::convert_narrow_048ae8(encoding_,65001,v.data(),static_cast<int>(v.size()),nullptr,0);
    if(result.error)errors_.filesystem_00cdb0(result.error);
    Path out(result.count,u'\0');result=encoding::convert_narrow_048ae8(encoding_,65001,v.data(),static_cast<int>(v.size()),out.data(),static_cast<int>(out.size()));
    if(result.error)errors_.filesystem_00cdb0(result.error);return out;
}
Path Environment::parent_path_019820(const Path& p){return path::parent_path_019820(p);}
Path Environment::join_016b00(const Path& a,const Path& b){auto p=a;path::append_016b50(p,b);return p;}
Path Environment::path_append_016b50(const Path& a,const Path& b){return join_016b00(a,b);}
Path Environment::filename(const Path& p){return path::filename_00e120(p);}
Path Environment::resolve_path_0192d0(std::string_view s,const Path& executable,bool relative) {
    auto path=widen_0145d0(s);
    if(relative&&!path::is_absolute_0195b0(path))path=join_016b00(parent_path_019820(executable),path);
    return path;
}
Path Environment::program_data_03c920(){return options_.program_data.empty()?environment_path(L"ProgramData"):options_.program_data;}
Path Environment::local_app_data_03c6f0(){return environment_path(L"LocalAppData");}
std::string Environment::narrow_path(const Path& p){return encoding::narrow_006e60(p,encoding::filesystem_codepage_048ac0(encoding_),encoding_,errors_);}
std::string Environment::path_to_utf8_031ca0(const Path& p){return encoding::path_utf8_01e3f0(p,encoding_,errors_);}
configuration::Node Environment::load_document(const Path& p){
    // The document interface expects a root containing Config2; read_config2
    // already returns the selected subtree for the active mutable hash path.
    configuration::Node document;
    document.children.emplace("Config2",config_hash::to_configuration(config_application::read_config2(p,files_)));
    document.iteration_order={"Config2"};
    return document;
}
void Environment::log(std::string_view event) {
    log({event});
}
void Environment::log(std::initializer_list<std::string_view> parts) {
#ifndef NDEBUG
    std::lock_guard lock(log_mutex_);
    std::string event;
    for(const auto part:parts)event.append(part);
    if(state_.user_directory.empty()) {
        if(parts.size())pending_logs_.push_back(std::move(event));
        return;
    }
    std::ofstream out(fs::path(state_.user_directory)/"surge.log",std::ios::binary|std::ios::app);
    for(const auto& pending:pending_logs_){out.write(pending.data(),static_cast<std::streamsize>(pending.size()));out.put('\n');}
    pending_logs_.clear();
    if(parts.size()){out.write(event.data(),static_cast<std::streamsize>(event.size()));out.put('\n');}
#else
    (void)parts;
#endif
}
void Environment::log_config_path(const Path& path){
#ifndef NDEBUG
    const auto text=path_to_utf8_031ca0(path);log({"config loaded: ",text});
#else
    (void)path;
#endif
}
void Environment::log_number(std::string_view event,std::int64_t value){
#ifndef NDEBUG
    const auto text=std::to_string(value);log({event,text});
#else
    (void)event;(void)value;
#endif
}
bool Environment::crt_is_space(unsigned char c){return std::isspace(c)!=0;}
int Environment::crt_tolower(int c){return std::tolower(static_cast<unsigned char>(c));}
void Environment::create_directories_018e00(const Path& p){fs::create_directories(fs::path(p));}
void Environment::write_embedded_png_03c020(const Path& p,configuration::EmbeddedPng which) {
    if(which!=configuration::EmbeddedPng::default_user_1800809d0&&
       which!=configuration::EmbeddedPng::default_avatar_180082710)
        throw std::invalid_argument("Unknown embedded PNG identity");
    const auto* bytes=which==configuration::EmbeddedPng::default_user_1800809d0
        ? avatar_data::profile:avatar_data::fallback;
    const auto size=which==configuration::EmbeddedPng::default_user_1800809d0
        ? sizeof(avatar_data::profile):sizeof(avatar_data::fallback);
    // Initial open failure returns false in03c020 and is ignored by its caller.
    // Write/close exceptions still propagate through the existing file helper.
    (void)file::write_binary_03c1d0(p,{reinterpret_cast<const char*>(bytes),size},files_);
}
std::vector<Path> Environment::directory_entries(const Path& p){
    std::vector<Path> out;
    visit_directory_entries(p,{&out,[](void* context,const Path& entry){static_cast<std::vector<Path>*>(context)->push_back(entry);}});
    return out;
}
void Environment::visit_directory_entries(const Path& p,configuration::DirectoryVisitor visitor){
    if(!visitor.visit)throw std::invalid_argument("Directory visitor callback required");
    for(const auto& entry:fs::directory_iterator(fs::path(p))) {
        const auto path=entry.path().u16string();
        visitor.visit(visitor.context,path); // Complete side effects before increment.
    }
}
void Environment::remove_04948c(const Path& p){fs::remove(fs::path(p));}
void Environment::update_achievement_storage_01eb00(configuration::State& s){storage_.update_01eb00(s);}
bool Environment::set_environment_variable(std::string_view name,std::string_view value){return SetEnvironmentVariableA(std::string(name).c_str(),std::string(value).c_str())!=0;}
status::Status Environment::status_01ea20(const Path& p,std::uint32_t flags){return status::status_01ea20(p,flags,stats_);}
[[noreturn]] void Environment::filesystem_error_018130(std::string_view op,std::uint32_t code,const Path& p){throw fs::filesystem_error(std::string(op),fs::path(p),std::error_code(code,std::system_category()));}
std::int64_t Environment::timestamp_to_epoch_041e10(std::string_view s){return timestamp::timestamp_to_epoch_041e10(s,timestamp_);}
void Environment::log_profile_name_084f48(std::string_view name){debug_log({"profile requested: ",name});}
std::string Environment::current_time_041b60(){return achievement_runtime::utc_time_041b60(clock_,formatter_);}
std::string Environment::ticket_time_041b60(){return achievement_runtime::utc_time_041b60(clock_,formatter_,nullptr,1);}
std::int32_t Environment::atoi_05ee08(std::string_view s){return std::atoi(std::string(s).c_str());}
void Environment::unknown_entitlement_field(std::string_view) {
    // This callback is reached while the shared handler state lock is held.
    // Do not display a synchronous modal or copy caller-controlled fields
    // (which may include secrets or line breaks) into diagnostic storage.
    debug_log("QueryEntitlements ignored unknown field");
}
bool Environment::online_avatar_virtual_28(const Path& p) {
    if(!provider)throw std::logic_error("Online avatar requested without provider");
    using Fn=bool(*)(void*,const wchar_t*);auto object=reinterpret_cast<void*>(provider);
    auto table=*reinterpret_cast<std::uintptr_t**>(object);return reinterpret_cast<Fn>(table[5])(object,wide(p).c_str());
}
bool Environment::copy_overwrite_048d20(const Path& from,const Path& to){return file_copy::copy_file_windows_048d20(from,to,2).copied;}
void Environment::log_saved_size_085530(std::int32_t size){
    debug_log_number("avatar selected: ",size);
}

void Environment::configure() {
    state_.executable_path=executable_path_03c6d0();
    // Both projections use the same product filenames, base -> override order
    // and executable-relative root. Missing configuration remains optional.
    for(bool over:{false,true})
        config_application::load_and_apply(state_,over,*this,files_);
    state_.user_directory=options_.data_root.empty()?join_016b00(local_app_data_03c6f0(),filesystem_widen_010540(product_identity::data_directory)):options_.data_root;
    create_directories_018e00(state_.user_directory);
    debug_log({}); // Flush config paths buffered before the data directory existed.
    state_.cookie_path=join_016b00(state_.user_directory,u"remid_cookie");
    state_.last_profile_path=join_016b00(state_.user_directory,u"last_profile");
    state_.last_avatar_path=join_016b00(state_.user_directory,u"last_avatar");
    if(state_.avatar.empty()) {
        state_.avatar=join_016b00(state_.user_directory,filesystem_widen_010540(product_identity::avatar_filename));
        write_embedded_png_03c020(state_.avatar,configuration::EmbeddedPng::default_user_1800809d0);
    }
    state_.default_avatar=join_016b00(state_.user_directory,u"default.png");
    write_embedded_png_03c020(state_.default_avatar,configuration::EmbeddedPng::default_avatar_180082710);
    configuration::cleanup_cached_avatars(state_.user_directory,*this);
    state_.game_attributes["AVAILABLE_VERSION"]=state_.game_attributes["INSTALLED_VERSION"];
    update_achievement_storage_01eb00(state_);
    (void)set_environment_variable("ContentId",state_.content_id);
    (void)set_environment_variable("EAGameLocale",state_.game_attributes["INSTALLED_LANGUAGE"]);
}
void Environment::apply_license_key_02a830(std::string_view key) {
    // Independent source projection of supplied FUN_18002a830. Keep its existing
    // regular file unchanged; original concatenates XML text without escaping.
    auto path=state_.license_directory+filesystem_widen_006d70(".dlf");
    const auto status=status_01ea20(path,3);
    if(status.error&&((static_cast<std::uint32_t>(status.kind)-1U)&0xfffffff7U)!=0)
        filesystem_error_018130("status",status.error,path);
    if(status.kind==2)return;
    std::string document(0x41,'\0');
    constexpr std::uint64_t header=0x1020001020630ULL;
    for(unsigned i=0;i<8;++i)document[i]=static_cast<char>(header>>(i*8));
    document+="<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><License xmlns=\"http://ea.com/license\"><CipherKey>";
    document.append(key);
    document+="</CipherKey><MachineHash>1</MachineHash><ContentId>";
    document+=state_.content_id;
    document+="</ContentId><UserId>";document+=state_.user_id;
    document+="</UserId><GrantTime>2024-01-01T00:00:00Z</GrantTime><StartTime>2024-01-01T00:00:00Z</StartTime></License>";
    crypto::Cipher cipher;
    cipher.set_key_and_iv_042760(token::constants::key_084a50,token::constants::iv_084a60);
    (void)cipher.encrypt_with_offset(document,0x41);
    create_directories_018e00(parent_path_019820(path));
    (void)file::write_binary_03c1d0(path,document,files_);
}
}
