// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/application/environment.hpp"
#include "src/achievements/persistence.hpp"
#include "src/time/timestamp.hpp"
#include "src/licensing/license.hpp"
#include "src/configuration/parser.hpp"
#include "src/configuration/hash_table.hpp"
#include <cctype>
#include "src/application/avatars.hpp"
#include "src/files/atomic_io.hpp"
#include <cstdlib>
#include <system_error>
namespace preloader_l {
class OnlineSession;
struct NoResponse final:std::exception {};
struct SessionFailure final:std::runtime_error {using std::runtime_error::runtime_error;};
class StorageEffects final:public timestamps::StorageEffects {
    const RuntimePaths& paths_;
public:
    StorageEffects(timestamps::Parser& parser,const RuntimePaths& paths):timestamps::StorageEffects(parser),paths_(paths){}
    Path filesystem_widen_006d70(std::string_view s) override{return filesystem_path(s);}
    void create_directories_018e00(const Path& p) override{paths_.require_write(native_path(p)/".directory-check");std::filesystem::create_directories(native_path(p));}
    status_recovered::Status status_01ea20(const Path&,std::uint32_t) override;
    [[noreturn]] void filesystem_error_018130(std::string_view,std::uint32_t,const Path&) override;
};
class ConfigurationEffects final:public achievement_storage::ConfigurationDependencies {
    configuration::State& state_;Environment& env_;
public:
    ConfigurationEffects(configuration::State& s,Environment& e,achievement_storage::Storage& storage):achievement_storage::ConfigurationDependencies(storage),state_(s),env_(e){}
    Path executable_path_03c6d0() override{return model_path(env_.executable);}
    Path parent_path_019820(const Path& p) override{return PathEffects::parent(p);}
    Path join_016b00(const Path& a,const Path& b) override{return PathEffects::join(a,b);}
    Path widen_0145d0(std::string_view s) override{return utf8_path(s);}
    Path filesystem_widen_010540(std::string_view s) override{return filesystem_path(s);}
    std::string narrow_path(const Path& p) override{return path_utf8(p);}
    Path resolve_path_0192d0(std::string_view,const Path&,bool) override;
    Path program_data_03c920() override{return model_path(env_.program_data);}
    Path local_app_data_03c6f0() override{return model_path(env_.paths.local_app_data());}
    Path runtime_path(RuntimeFile kind) override{return env_.paths.model(kind);}
    configuration::Node load_document(const Path&) override;
    void log_config_path(const Path&) override{env_.log(LogEvent::phase);}
    bool crt_is_space(unsigned char c) override{return std::isspace(c)!=0;}
    int crt_tolower(int c) override{if(c<-1||c>255)throw std::invalid_argument("locale character domain");return std::tolower(c);}
    void apply_license_key_02a830(std::string_view) override;
    void create_directories_018e00(const Path& p) override{env_.paths.require_write(native_path(p)/".directory-check");std::filesystem::create_directories(native_path(p));}
    void write_embedded_png_03c020(const Path&,configuration::EmbeddedPng) override;
    std::vector<Path> directory_entries(const Path&) override;
    Path filename(const Path& p) override{return PathEffects::filename(p);}
    void remove_04948c(const Path& p) override{env_.paths.require_write(native_path(p));throw std::logic_error("automatic runtime cleanup is disabled");}
    bool set_environment_variable(std::string_view k,std::string_view v) override{publish_environment(k,v);return true;}
};
class ContentEffects final:public achievement_runtime::PersistenceEffects {
    Environment& env_;OnlineSession& online_;AvatarStore& avatars_;
public:
    ContentEffects(configuration::State& s,achievement_storage::Paths& p,achievement_runtime::Clock& c,achievement_runtime::Formatter& f,achievement_writer::Effects& d,files::Factory& files,Environment& e,OnlineSession& online,AvatarStore& avatars):achievement_runtime::PersistenceEffects(s,p,c,f,d,files),env_(e),online_(online),avatars_(avatars){}
    std::int32_t atoi_05ee08(std::string_view s) override{return std::atoi(std::string(s).c_str());}
    void unknown_entitlement_field(std::string_view) override{env_.log(LogEvent::unknown_field);}
    Path filesystem_widen_006d70(std::string_view s) override{return filesystem_path(s);}
    Path path_append_016b50(const Path& a,const Path& b) override{return PathEffects::join(a,b);}
    bool online_avatar_virtual_28(const Path&,std::string_view,std::int32_t) override;
    Path avatar_cache_path(const Path&,std::string_view owner,std::string_view width) override{return model_path(env_.paths.avatar_cache(owner,width));}
    Path resolve_image(const Path& p) override{return avatars_.resolve(p);}
    void persist_achievement(const Path&,std::string_view,const content::AchievementMap&) override;
    bool copy_overwrite_048d20(const Path& a,const Path& b) override{atomic_write(native_path(b),read_file(a,true),&env_.paths);return true;}
    std::string path_to_utf8_031ca0(const Path& p) override{return path_utf8(p);}
    void log_saved_size_085530(std::int32_t) override{env_.log(LogEvent::phase);}
};
class InformationEffects final:public handlers::Effects {
    Environment& env_;achievement_runtime::Clock& clock_;achievement_runtime::Formatter& formatter_;
public:
    InformationEffects(Environment& e,achievement_runtime::Clock& c,achievement_runtime::Formatter& f):env_(e),clock_(c),formatter_(f){}
    void log_profile_name_084f48(std::string_view) override{env_.log(LogEvent::phase);}
    std::string current_time_041b60() override{return achievement_runtime::utc_time_041b60(clock_,formatter_);}
};
class LicenseEffects final:public licensing::Effects {
    Environment& env_;achievement_runtime::Clock& clock_;achievement_runtime::Formatter& formatter_;OnlineSession& online_;
public:
    LicenseEffects(Environment& e,achievement_runtime::Clock& c,achievement_runtime::Formatter& f,OnlineSession& online):env_(e),clock_(c),formatter_(f),online_(online){}
    const char* online_auth_virtual_30(const char*) override;
    licensing::FileStatus status_01ea20(const Path&,std::uint32_t) override;
    [[noreturn]] void filesystem_error_018130(std::string_view,std::int32_t,const Path&) override;
    bool file_hash_matches_042d20(const Path&,std::string) override;
    std::string load_token_02afa0(const Path&) override;
    Path filesystem_widen_006d70(std::string_view s) override{return filesystem_path(s);}
    Path filename_00e120(const Path& p) override{return PathEffects::filename(p);}
    Path parent_path_019820(const Path& p) override{return PathEffects::parent(p);}
    Path join_016b00(const Path& a,const Path& b) override{return PathEffects::join(a,b);}
    std::string path_utf8_01e3f0(const Path& p) override{return path_utf8(p);}
    std::string current_time_041b60_null_true() override{return achievement_runtime::utc_time_041b60(clock_,formatter_,nullptr,1);}
    bool wine_detected_045010() override;
    void log_literal_006880(std::string_view) override{env_.log(LogEvent::phase);}
    void log_string_006860(std::string_view) override{env_.log(LogEvent::phase);}
    bool write_ticket_03c1d0(const Path& p,std::string_view s) override{atomic_write(native_path(p),s,&env_.paths);return true;}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t) override;
    [[noreturn]] void exit_05cdc0(std::int32_t n) override{throw SessionFailure("license processing stopped: "+std::to_string(n));}
    [[noreturn]] void no_response_02bcd0_04d228() override{throw NoResponse{};}
};
}
