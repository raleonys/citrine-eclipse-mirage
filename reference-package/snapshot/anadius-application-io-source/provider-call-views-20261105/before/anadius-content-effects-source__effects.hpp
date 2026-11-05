#pragma once
#include "../anadius-achievement-runtime-source/runtime.hpp"
#include "../anadius-config-storage-source/filesystem.hpp"
#include "../anadius-online-factory-source/factory.hpp"
#include "../anadius-session-memory-source/memory.hpp"
#include <cstdlib>
#include <optional>
#include <type_traits>
#include <utility>

namespace anadius::content_effects {
struct Options {
    std::shared_ptr<void> lifetime; // captured lower backing, retired last
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<library_effects::Dialog> dialog;
    std::shared_ptr<config_storage::Filesystem> filesystem=std::make_shared<config_storage::Filesystem>();
    std::shared_ptr<file_recovered::Factory> files=std::make_shared<file_recovered::NativeFactory>();
    std::shared_ptr<achievement_runtime::Clock> clock=std::make_shared<achievement_runtime::HostClock>();
    std::shared_ptr<achievement_runtime::Formatter> formatter=std::make_shared<achievement_runtime::HostFormatter>();
    std::shared_ptr<encoding_recovered::Calls> encoding=std::make_shared<encoding_recovered::NativeCalls>();
    std::shared_ptr<encoding_recovered::Errors> errors=std::make_shared<encoding_recovered::PortableErrors>();
    int (*atoi)(const char*)=std::atoi;
    decltype(&::CopyFileW) copy_file=::CopyFileW;
};
// Concrete content/image/achievement effects, without a giant Effects lower.
// Default use and construction/retirement stay on the Graph worker/fiber.
// Explicit synchronous child scopes require a live Parent permit/admission.
// Actual State, authoritative
// Paths and matching Online must outlive this provider; lifetime is not a pin
// proof for these borrowed roots. Native SDK/file/CRT compatibility contracts
// remain explicit. Construction does no I/O/publication/provider operations.
class Content final:public content_handlers_recovered::Effects,
                    public content_handlers_recovered::AvatarRequest,
                    public content_handlers_recovered::CheckedCalls {
    struct JobAction {void* context;void (*call)(void*);};
public:
    Content(bootstrap_backend::CallbackStateView,const achievement_storage::Paths&,
            online_factory::Online&,Options);
    ~Content()noexcept override;
    Content(const Content&)=delete;Content& operator=(const Content&)=delete;
    std::int32_t atoi_05ee08(std::string_view)override;
    void unknown_entitlement_field(std::string_view)override;
    std::string utc_time_041b60(std::uint64_t&)override;
    std::unique_ptr<content_handlers_recovered::DocumentWriter> printer_0019b0()override;
    void write_xml_03c1d0(const configuration::Path&,std::string_view)override;
    void save_steam_01f9a0(const content_handlers_recovered::AchievementMap&)override;
    configuration::Path filesystem_widen_006d70(std::string_view)override;
    configuration::Path path_append_016b50(const configuration::Path&,const configuration::Path&)override;
    // Legacy one-path call cannot supply recovered owner/size; rejects rather
    // than inventing arguments. Actual Graph selects AvatarRequest below.
    bool online_avatar_virtual_28(const configuration::Path&)override;
    bool request_online_avatar(const configuration::Path&,const char*,std::int32_t)override;
    bool copy_overwrite_048d20(const configuration::Path&,const configuration::Path&)override;
    std::string path_to_utf8_031ca0(const configuration::Path&)override;
    void log_saved_size_085530(std::int32_t)override;
    void bind_content_checkpoint(content_handlers_recovered::CallCheckpoint)override;
    void clear_content_checkpoint()noexcept override;
    // No owned callable transfer: caller retains action, facade, checkpoint,
    // Runtime/Paths/Online and all lower backing throughout this whole call.
    // The copied permit never extends the caller's Parent Operation.
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
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& attached()const;
    template<class F>auto invoke(F&&)->std::invoke_result_t<F>;
    void run_job_action(socket_recovered::SessionMemoryDomain&,
        const socket_recovered::SessionMemoryDomain::JobPermit&,content_handlers_recovered::CallCheckpoint,JobAction);
};
}
