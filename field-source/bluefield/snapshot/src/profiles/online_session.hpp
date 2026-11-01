// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/profiles/online.hpp"
#include "src/application/environment.hpp"
#include <optional>
#include "src/files/atomic_io.hpp"
namespace preloader_l {
// Native version-2 provider adapter. No original DLL or plugin is bundled.
// On systems without that optional provider, the actual source offline branch runs.
class OnlineSession final:public online::Effects {
    configuration::State& state_;Environment& env_;std::uint8_t& online_;
    online::Module module_=0;online::Object object_=0;bool finished_=false;
    std::optional<std::uint32_t> error_mode_;std::string auth_storage_;std::wstring cookie_parameter_;
public:
    struct ProfileResult {
        std::string username,persona_id,user_id;
        Path avatar;
        std::uint8_t online=0;
    };
    OnlineSession(configuration::State&,Environment&,std::uint8_t&);
    ~OnlineSession();
    ProfileResult initialize();
    void close();
    const char* auth(const char*);
    bool avatar(const Path&,std::string_view owner,std::int32_t size);
    std::uint32_t suppress_errors_0450a0() override;
    void restore_error_mode(std::uint32_t) override;
    Path filesystem_widen_006d70(std::string_view s) override{return filesystem_path(s);}
    Path parent_path_019820(const Path& p) override{return PathEffects::parent(p);}
    Path join_016b00(const Path& a,const Path& b) override{return PathEffects::join(a,b);}
    online::Module load_copy_0450d0(const Path&) override;
    online::Procedure get_proc_address(online::Module,std::string_view) override;
    std::int32_t call_version_check(online::Procedure) override;
    online::Object call_create_online(online::Procedure,const char16_t*) override;
    std::int32_t status_virtual_08(online::Object) override;
    const char* username_virtual_10(online::Object) override;
    const char* persona_virtual_18(online::Object) override;
    const char* user_id_virtual_20(online::Object) override;
    void destroy_virtual_00(online::Object) override;
    bool free_library(online::Module) override;
    status_recovered::Status status_01ea20(const Path&,std::uint32_t) override;
    [[noreturn]] void filesystem_error_018130(std::string_view,std::uint32_t,const Path&) override;
    std::string read_binary_03b3f0(const Path& p) override;
    bool write_binary_03c1d0(const Path& p,std::string_view s) override{env_.paths.require_write(native_path(p));atomic_write(native_path(p),s,&env_.paths);return true;}
    void log_literal_006880(std::string_view) override{env_.log(LogEvent::provider);}
    void log_finish_0088e0() override{env_.log(LogEvent::provider);}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t) override;
    void wait_steps_036880(std::int32_t) override;
    void finish_flag_036820() override{finished_=true;}
};
}
