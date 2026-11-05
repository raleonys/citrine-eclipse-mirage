#pragma once
#include "../anadius-license-source/license.hpp"
#include "../anadius-file-source/license_files.hpp"
#include "../anadius-achievement-runtime-source/utc.hpp"
#include "../anadius-online-factory-source/factory.hpp"
#include "../anadius-messages-source/control.hpp"
#include "../anadius-session-memory-source/memory.hpp"
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <utility>

namespace anadius::licensing_effects {
// Selected host C++ control marker; not original MSVC exception/type/unwind ABI.
using NoResponseRequiredError=messages_recovered::NoResponseRequiredError;
struct AttemptState {std::int32_t attempted_files;std::uint8_t blocked;};
struct Options {
    std::shared_ptr<void> lifetime;
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<library_effects::Dialog> dialog;
    std::shared_ptr<file_recovered::Factory> files=std::make_shared<file_recovered::NativeFactory>();
    std::shared_ptr<hash_recovered::TextEnvironment> text=std::make_shared<file_recovered::NativeTextEnvironment>();
    std::shared_ptr<status_recovered::Stats> stats=std::make_shared<status_recovered::NativeStats>();
    std::shared_ptr<encoding_recovered::Calls> encoding=std::make_shared<encoding_recovered::NativeCalls>();
    std::shared_ptr<encoding_recovered::Errors> errors=std::make_shared<encoding_recovered::PortableErrors>();
    std::shared_ptr<achievement_runtime::Clock> clock=std::make_shared<achievement_runtime::HostClock>();
    std::shared_ptr<achievement_runtime::Formatter> formatter=std::make_shared<achievement_runtime::HostFormatter>();
    decltype(&::GetModuleHandleA) module=::GetModuleHandleA;
    decltype(&::GetProcAddress) procedure=::GetProcAddress;
    void (*exit_process)(int)=std::exit; // one compiler CRT; never recovered outer CRT
};
// Default calls and construction/retirement use the original Graph worker/fiber;
// a live explicit with_job grants only synchronous child calls. Exact matching
// Online/Runtime and every borrowed root outlive this provider. No constructor I/O.
class Core final:public license_recovered::Effects,public license_recovered::OwnedAuthEffects,
                 public content_handlers_recovered::CheckedCalls,public messages_recovered::UnknownMessageEffects {
    struct JobAction {void* context;void (*call)(void*);};
public:
    Core(bootstrap_backend::CallbackStateView,online_factory::Online&,Options);
    ~Core()noexcept override;
    Core(const Core&)=delete;Core& operator=(const Core&)=delete;
    const char* online_auth_virtual_30(const char*)override;
    std::string copy_online_auth(const char*)override;
    license_recovered::FileStatus status_01ea20(const configuration::Path&,std::uint32_t)override;
    [[noreturn]] void filesystem_error_018130(std::string_view,std::int32_t,const configuration::Path&)override;
    bool file_hash_matches_042d20(const configuration::Path&,std::string)override;
    std::string load_token_02afa0(const configuration::Path&)override;
    configuration::Path filesystem_widen_006d70(std::string_view)override;
    configuration::Path filename_00e120(const configuration::Path&)override;
    configuration::Path parent_path_019820(const configuration::Path&)override;
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&)override;
    std::string path_utf8_01e3f0(const configuration::Path&)override;
    std::string current_time_041b60_null_true()override;
    bool wine_detected_045010()override;
    void log_literal_006880(std::string_view)override;
    void log_string_006860(std::string_view)override;
    bool write_ticket_03c1d0(const configuration::Path&,std::string_view)override;
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override;
    [[noreturn]] void exit_05cdc0(std::int32_t)override;
    [[noreturn]] void no_response_02bcd0_04d228()override;
    // Independent message033d70 diagnostics, using this same owned stream/UI.
    void unknown_message_07bd70(const messages_recovered::UnknownMessageInput&)override;
    void bind_content_checkpoint(content_handlers_recovered::CallCheckpoint)override;
    void clear_content_checkpoint()noexcept override;
    // Synchronous only: the caller retains action, facade, Parent Operation,
    // Runtime/Online and every lower backing until this call has returned.
    template<class F>auto with_job(socket_recovered::SessionMemoryDomain& parent,
        const socket_recovered::SessionMemoryDomain::JobPermit& permit,
        content_handlers_recovered::CallCheckpoint checkpoint,F&& action)->std::invoke_result_t<F&>{
        using R=std::invoke_result_t<F&>;static_assert(!std::is_reference_v<R>);
        if constexpr(std::is_void_v<R>){
            struct Payload {F& action;static void run(void* p){static_cast<Payload*>(p)->action();}} payload{action};
            run_job_action(parent,permit,checkpoint,{&payload,Payload::run});
        }else{
            struct Payload {F& action;std::optional<R> result;static void run(void* p){auto& v=*static_cast<Payload*>(p);v.result.emplace(v.action());}} payload{action,{}};
            run_job_action(parent,permit,checkpoint,{&payload,Payload::run});return std::move(*payload.result);
        }
    }
private:
    friend class Handlers;
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& attached()const;
    template<class F>auto invoke(F&&,bool control=false)->std::invoke_result_t<F>;
    void run_job_action(socket_recovered::SessionMemoryDomain&,
        const socket_recovered::SessionMemoryDomain::JobPermit&,content_handlers_recovered::CallCheckpoint,JobAction);
};
// Actual Graph licensing assembly. Attempt/block roots are mandatory explicit
// inputs outside036d20. AuthenticationBinding builds the real Handlers only
// after Worker supplies Runtime's stable authentication cell; no dummy value.
class Handlers final:public dispatch_recovered::LicenseHandlers,public license_recovered::AuthenticationBinding,
                     public content_handlers_recovered::CheckedCalls,public messages_recovered::UnknownMessageEffects {
    struct JobAction {void* context;void (*call)(void*);};
public:
    Handlers(bootstrap_backend::CallbackStateView,online_factory::Online&,std::shared_ptr<AttemptState>,Options);
    ~Handlers()noexcept override;
    Handlers(const Handlers&)=delete;Handlers& operator=(const Handlers&)=delete;
    void bind_authentication_code(const license_recovered::AuthenticationCode&)override;
    bool auth_code_02e3b0(const messages_recovered::XmlNode&,handlers_recovered::Xml&)override;
    bool request_license_0320c0(const messages_recovered::XmlNode&,handlers_recovered::Xml&)override;
    bool invalidate_license_030c50(const messages_recovered::XmlNode&,handlers_recovered::Xml&)override;
    void unknown_message_07bd70(const messages_recovered::UnknownMessageInput&)override;
    void bind_content_checkpoint(content_handlers_recovered::CallCheckpoint)override;
    void clear_content_checkpoint()noexcept override;
    // Authentication binds on the creator before any child scope. Both this
    // facade and its Core reserve the same live, exact Parent proof.
    template<class F>auto with_job(socket_recovered::SessionMemoryDomain& parent,
        const socket_recovered::SessionMemoryDomain::JobPermit& permit,
        content_handlers_recovered::CallCheckpoint checkpoint,F&& action)->std::invoke_result_t<F&>{
        using R=std::invoke_result_t<F&>;static_assert(!std::is_reference_v<R>);
        if constexpr(std::is_void_v<R>){
            struct Payload {F& action;static void run(void* p){static_cast<Payload*>(p)->action();}} payload{action};
            run_job_action(parent,permit,checkpoint,{&payload,Payload::run});
        }else{
            struct Payload {F& action;std::optional<R> result;static void run(void* p){auto& v=*static_cast<Payload*>(p);v.result.emplace(v.action());}} payload{action,{}};
            run_job_action(parent,permit,checkpoint,{&payload,Payload::run});return std::move(*payload.result);
        }
    }
private:struct Impl;std::unique_ptr<Impl> impl_;Impl& attached()const;
    template<class F>bool invoke_request(const messages_recovered::XmlNode&,handlers_recovered::Xml&,F&&);
    void run_job_action(socket_recovered::SessionMemoryDomain&,
        const socket_recovered::SessionMemoryDomain::JobPermit&,content_handlers_recovered::CallCheckpoint,JobAction);
};
}
