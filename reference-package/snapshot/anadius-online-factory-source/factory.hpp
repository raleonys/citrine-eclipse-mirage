#pragma once
#include "../anadius-bootstrap-backend-source/backend.hpp"
#include "../anadius-online-version-source/version.hpp"
#include "../anadius-online-source/recovered_effects.hpp"
#include "../anadius-library-effects-source/dialog.hpp"
#include <functional>
#include <ostream>

namespace anadius::online_factory {
enum class LifetimeProfile {Cooperative,OriginalCrt};
struct Options {
    std::shared_ptr<void> lifetime;
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<library_effects::Dialog> dialog;
    std::unique_ptr<recovered::libraries::SystemCalls> os=std::make_unique<recovered::libraries::NativeSystemCalls>();
    std::unique_ptr<encoding_recovered::Calls> encoding=std::make_unique<encoding_recovered::NativeCalls>();
    std::unique_ptr<encoding_recovered::Errors> errors=std::make_unique<encoding_recovered::PortableErrors>();
    std::unique_ptr<status_recovered::Stats> stats=std::make_unique<status_recovered::NativeStats>();
    std::unique_ptr<file_recovered::Factory> files=std::make_unique<file_recovered::NativeFactory>();
    decltype(&::GetProcAddress) get_proc=::GetProcAddress;
    online_version::CodeLease code_lease;
    // Cooperative profile: explicit foreign callback/job admission stop AND
    // completion attestation. OriginalCrt requires this slot to remain empty.
    // Invoked after source joins/exit cleanup, outside all component gates.
    // A true result is this selected host contract, not native/global proof.
    // stop_sources/the foreign-object contract MUST ALREADY exclude foreign
    // users before cleanup destroys the object/frees the module. This later
    // predicate cannot retroactively make those earlier operations safe.
    std::function<bool()> foreign_quiescent;
    // OriginalCrt uses actual local module/object/call retirement, without an
    // added external unload promise. It does not certify foreign/image safety.
    // Appended HOST field: rebuild all by-value Options consumers together.
    LifetimeProfile lifetime_profile=LifetimeProfile::Cooperative;
};
// Shared passive lifetime validation for this facade and Resources assembly.
bool lifetime_configuration_valid(const Options&)noexcept;
// Select original lifetime and the real native module-reference CodeLease.
// Requires unselected profile and empty lease/proof slots. Construction calls
// no module SDK; actual references are acquired only for loaded Online code.
void select_native_original_online(Options&);
// Consume callable donors while their receiving lifetime owner is alive.
// A plain std::move of libc++ std::function may leave a small callable behind.
Options take_options(Options&);
enum class Phase {Open,Checking,Closed,Retiring,Poisoned};
struct Snapshot {
    Phase phase;std::size_t active_calls;
    online_version::Snapshot binding;
    const configuration::State* configuration;
    const bootstrap_backend::RuntimeSeed* runtime;
    const execution_recovered::Execution* execution;
};
// Bound only after Backend config/runtime/Execution reach stable addresses.
// All27 Effects routes are real: fifteen selected platform/core leaves, seven
// typed foreign operations, three owned getter copies and two actual phases.
// No lower Effects provider is accepted. Ordinary worker routes have no creator
// affinity; close and physical retirement require the constructor controller
// thread and fiber.
class Online final:public bootstrap_backend::OnlineComponent {
public:
    Online(bootstrap_backend::CallbackStateView,Options&&);
    ~Online()noexcept override;
    Online(const Online&)=delete;Online& operator=(const Online&)=delete;
    Online(Online&&)=delete;Online& operator=(Online&&)=delete;
    std::uint32_t suppress_errors_0450a0()override;
    void restore_error_mode(std::uint32_t)override;
    configuration::Path filesystem_widen_006d70(std::string_view)override;
    configuration::Path parent_path_019820(const configuration::Path&)override;
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&)override;
    online_recovered::Module load_copy_0450d0(const configuration::Path&)override;
    online_recovered::Procedure get_proc_address(online_recovered::Module,std::string_view)override;
    std::int32_t call_version_check(online_recovered::Procedure)override;
    online_recovered::Object call_create_online(online_recovered::Procedure,const char16_t*)override;
    std::int32_t status_virtual_08(online_recovered::Object)override;
    const char* username_virtual_10(online_recovered::Object)override;
    const char* persona_virtual_18(online_recovered::Object)override;
    const char* user_id_virtual_20(online_recovered::Object)override;
    std::string copy_username(online_recovered::Object)override;
    std::string copy_persona(online_recovered::Object)override;
    std::string copy_user_id(online_recovered::Object)override;
    // Application leaves beyond the bootstrap08..20 getter interface. Existing
    // OnlineComponent/Effects vtables and this facade's pimpl layout stay fixed.
    bool avatar_virtual_28(online_recovered::Object,const online_recovered::Path&,const char*,std::int32_t);
    const char* auth_virtual_30(online_recovered::Object,const char*);
    std::string copy_auth(online_recovered::Object,const char*);
    void destroy_virtual_00(online_recovered::Object)override;
    bool free_library(online_recovered::Module)override;
    status_recovered::Status status_01ea20(const configuration::Path&,std::uint32_t)override;
    [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const configuration::Path&)override;
    std::string read_binary_03b3f0(const configuration::Path&)override;
    bool write_binary_03c1d0(const configuration::Path&,std::string_view)override;
    void log_literal_006880(std::string_view)override;
    void log_finish_0088e0()override;
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override;
    void wait_steps_036880(std::int32_t)override;
    void finish_flag_036820()override;
    bool close_after_cleanup()override;
    bool matches(bootstrap_backend::CallbackStateView)const noexcept override;
    Snapshot snapshot()const;
private:struct Impl;std::unique_ptr<Impl> impl_;Impl& attached()const;
};
// Named sinks consume Options& without a forwarding by-value temporary.
std::unique_ptr<bootstrap_backend::OnlineComponent> make_online(
    bootstrap_backend::CallbackStateView,Options&);
void install_online(bootstrap_backend::Resources&,Options&);
} // namespace anadius::online_factory
