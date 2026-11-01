#pragma once
#include "../anadius-achievement-runtime-source/runtime.hpp"
#include "../anadius-config-storage-source/filesystem.hpp"
#include "../anadius-online-factory-source/factory.hpp"
#include <cstdlib>
#include <type_traits>

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
// Construct/use/retire on the Graph worker/fiber. Actual State, authoritative
// Paths and matching Online must outlive this provider; lifetime is not a pin
// proof for these borrowed roots. Native SDK/file/CRT compatibility contracts
// remain explicit. Construction does no I/O/publication/provider operations.
class Content final:public content_handlers_recovered::Effects,
                    public content_handlers_recovered::AvatarRequest,
                    public content_handlers_recovered::CheckedCalls {
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
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& attached()const;
    template<class F>auto invoke(F&&)->std::invoke_result_t<F>;
};
}
