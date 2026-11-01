#pragma once
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include "../anadius-library-effects-source/dialog.hpp"
#include "../anadius-encoding-source/encoding.hpp"
#include "../anadius-online-source/recovered_effects.hpp"
#include <memory>
#include <ostream>

namespace anadius::online_version {class Binding;}

namespace anadius::online_platform {
using Path=configuration::Path;
using Module=online_recovered::Module;
using Procedure=online_recovered::Procedure;
using Object=online_recovered::Object;
struct Resources {
    std::unique_ptr<recovered::libraries::SystemCalls> os=std::make_unique<recovered::libraries::NativeSystemCalls>();
    std::unique_ptr<encoding_recovered::Calls> encoding=std::make_unique<encoding_recovered::NativeCalls>();
    std::unique_ptr<encoding_recovered::Errors> errors=std::make_unique<encoding_recovered::PortableErrors>();
    std::unique_ptr<status_recovered::Stats> stats=std::make_unique<status_recovered::NativeStats>();
    std::unique_ptr<file_recovered::Factory> files=std::make_unique<file_recovered::NativeFactory>();
    decltype(&::GetProcAddress) get_proc=::GetProcAddress;
    // Optional owned version export binding. B25-only mode keeps6 lower calls.
    std::shared_ptr<online_version::Binding> version;
    // B26 opt-in uses SAME binding for factory/destroy + guarded lower queries.
    bool native_objects=false;
    // B27 requires the same adopted-object owner. Disabled preserves B26.
    bool native_queries=false;
};
// Selects15 OS/path/file/log/dialog policies. Optional version binding handles
// one observed typed export. Native object opt-in adds factory/destroy while
// four queries retain lower providers unless native_queries selects B27. B3 intercepts phase methods.
// Empty version option preserves all7 legacy foreign forwards. Object tokens
// are not dereferenced here. No raw module is implicitly freed by destruction.
class OnlinePlatform final:public online_recovered::Effects {
public:
    OnlinePlatform(std::shared_ptr<std::ostream>,std::shared_ptr<library_effects::Dialog>,
        std::unique_ptr<online_recovered::Effects>,Resources={});
    OnlinePlatform(const OnlinePlatform&)=delete;
    OnlinePlatform& operator=(const OnlinePlatform&)=delete;
    OnlinePlatform(OnlinePlatform&&)=delete;
    OnlinePlatform& operator=(OnlinePlatform&&)=delete;
    std::uint32_t suppress_errors_0450a0() override;
    void restore_error_mode(std::uint32_t) override;
    Path filesystem_widen_006d70(std::string_view) override;
    Path parent_path_019820(const Path&) override;
    Path join_016b00(const Path&, const Path&) override;
    Module load_copy_0450d0(const Path&) override;
    Procedure get_proc_address(Module, std::string_view) override;
    std::int32_t call_version_check(Procedure) override;
    Object call_create_online(Procedure, const char16_t*) override;
    std::int32_t status_virtual_08(Object) override;
    const char* username_virtual_10(Object) override;
    const char* persona_virtual_18(Object) override;
    const char* user_id_virtual_20(Object) override;
    std::string copy_username(Object) override;
    std::string copy_persona(Object) override;
    std::string copy_user_id(Object) override;
    void destroy_virtual_00(Object) override;
    bool free_library(Module) override;
    status_recovered::Status status_01ea20(const Path&, std::uint32_t flags) override;
    [[noreturn]] void filesystem_error_018130(std::string_view operation,
        std::uint32_t code, const Path&) override;
    std::string read_binary_03b3f0(const Path&) override;
    bool write_binary_03c1d0(const Path&, std::string_view) override;
    void log_literal_006880(std::string_view) override;
    void log_finish_0088e0() override;
    void dialog_038bc0(std::string_view title, std::string_view text,
                               std::uint32_t flags) override;
    void wait_steps_036880(std::int32_t count) override;
    void finish_flag_036820() override; // original flag at 1800a3c3c
private:
    Resources resources_;
    // Caller retains streambuf and serializes every stream/SDK/provider user.
    // Lower dies before stream/dialog/native resources it may depend on.
    std::shared_ptr<std::ostream> stream_;
    std::shared_ptr<library_effects::Dialog> dialog_;
    std::unique_ptr<online_recovered::Effects> lower_;
    // Stable borrowed references to this owner and its resources. Use the ten
    // already-recovered core methods; boundary overrides below never delegate
    // to a RecoveredEffects method that would forward back into this object.
    online_recovered::RecoveredEffects recovered_;
};
bootstrap_backend::Resources bind_online_platform(bootstrap_backend::Resources,
    std::shared_ptr<std::ostream>,std::shared_ptr<library_effects::Dialog>,Resources={});
} // namespace anadius::online_platform
