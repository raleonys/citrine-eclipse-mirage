#include "adapters.hpp"
namespace recovered_hooks {
std::u16string HostInitEffects::filesystem_widen_006d70(std::string_view s) {return remaining_.filesystem_widen_006d70(s);}
std::uint64_t HostInitEffects::version_condition_mask(std::uint64_t m,std::uint32_t t,std::uint8_t c) {return remaining_.version_condition_mask(m,t,c);}
bool HostInitEffects::verify_version(const anadius::host_init_recovered::VersionRequest& v,std::uint32_t t,std::uint64_t m) {return remaining_.verify_version(v,t,m);}
std::int32_t HostInitEffects::initialize_engine_046a20() {return engine_.initialize();}
std::uintptr_t HostInitEffects::get_module_handle(std::u16string_view m) {return remaining_.get_module_handle(m);}
std::uintptr_t HostInitEffects::get_proc_address(std::uintptr_t m,std::string_view s) {return remaining_.get_proc_address(m,s);}
std::int32_t HostInitEffects::create_hook_0466e0(std::uintptr_t t,anadius::host_init_recovered::Callback callback,std::uintptr_t& original) {
    const auto rebuilt = callbacks_.rebuilt_address(callback);
    void* output = reinterpret_cast<void*>(original);
    const auto status = engine_.create(reinterpret_cast<void*>(t),reinterpret_cast<void*>(rebuilt),&output);
    // Copy back on every normal return, including failures; avoid aliasing an
    // integer output object as void**. A throwing synthetic backend skips this.
    original = reinterpret_cast<std::uintptr_t>(output);
    return status;
}
std::int32_t HostInitEffects::enable_hook_046a10(std::uintptr_t t) {return engine_.enable(reinterpret_cast<void*>(t));}
void HostInitEffects::log_text_006880(std::string_view s) {remaining_.log_text_006880(s);}
void HostInitEffects::log_integer_00af90(std::int32_t n) {remaining_.log_integer_00af90(n);}
void HostInitEffects::finish_log_0088e0() {remaining_.finish_log_0088e0();}
std::int32_t CleanupEffects::disable_hook_046a00(std::uintptr_t t) {return engine_.disable(reinterpret_cast<void*>(t));}
std::int32_t CleanupEffects::uninitialize_engine_046ac0() {return engine_.uninitialize();}
void CleanupEffects::log_literal_006880(std::string_view s) {remaining_.log_literal_006880(s);}
void CleanupEffects::log_finish_0088e0() {remaining_.log_finish_0088e0();}
}
