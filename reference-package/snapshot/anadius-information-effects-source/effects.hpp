#pragma once
#include "../anadius-content-handlers-source/content_handlers.hpp"
#include "../anadius-achievement-runtime-source/utc.hpp"
#include <ostream>
#include <type_traits>

namespace anadius::information_effects {
struct Options {
    std::shared_ptr<void> lifetime; // captured backing retired last
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<achievement_runtime::Clock> clock=std::make_shared<achievement_runtime::HostClock>();
    std::shared_ptr<achievement_runtime::Formatter> formatter=std::make_shared<achievement_runtime::HostFormatter>();
};
// Concrete02f530 logging and02da90 system time. Serialized original Graph
// worker/fiber only. Uses existing synchronous CheckedCalls protocol shared
// with Content; no global State defaults, DLL entry or provider lower invented.
class Information final:public handlers_recovered::Effects,public content_handlers_recovered::CheckedCalls {
public:
    explicit Information(Options);
    ~Information()noexcept override;
    Information(const Information&)=delete;Information& operator=(const Information&)=delete;
    void log_profile_name_084f48(std::string_view)override;
    std::string current_time_041b60()override;
    void bind_content_checkpoint(content_handlers_recovered::CallCheckpoint)override;
    void clear_content_checkpoint()noexcept override;
private:
    struct Impl;std::unique_ptr<Impl> impl_;
    Impl& attached()const;
    template<class F>auto invoke(F&&)->std::invoke_result_t<F>;
};
}
