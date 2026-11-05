#pragma once
#include "../anadius-content-handlers-source/content_handlers.hpp"
#include "../anadius-achievement-runtime-source/utc.hpp"
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-application-io-source/call.hpp"
#include <ostream>
#include <type_traits>

namespace anadius::information_effects {
struct Options {
    std::shared_ptr<void> lifetime; // captured backing retired last
    std::shared_ptr<std::ostream> stream;
    std::shared_ptr<achievement_runtime::Clock> clock=std::make_shared<achievement_runtime::HostClock>();
    std::shared_ptr<achievement_runtime::Formatter> formatter=std::make_shared<achievement_runtime::HostFormatter>();
};
// Concrete02f530 logging and02da90 system time. Default calls use original Graph
// worker/fiber. Explicit child calls require live Parent permit and synchronous
// caller admission; caller retains this facade and checkpoint backing.
// Uses existing synchronous CheckedCalls protocol shared
// with Content; no global State defaults, DLL entry or provider lower invented.
class Information final:public handlers_recovered::Effects,public content_handlers_recovered::CheckedCalls {
public:
    class CallView;
    explicit Information(Options);
    ~Information()noexcept override;
    Information(const Information&)=delete;Information& operator=(const Information&)=delete;
    void log_profile_name_084f48(std::string_view)override;
    std::string current_time_041b60()override;
    void log_profile_name_job(std::string_view,socket_recovered::SessionMemoryDomain&,
        const socket_recovered::SessionMemoryDomain::JobPermit&,content_handlers_recovered::CallCheckpoint);
    std::string current_time_job(socket_recovered::SessionMemoryDomain&,
        const socket_recovered::SessionMemoryDomain::JobPermit&,content_handlers_recovered::CallCheckpoint);
    void bind_content_checkpoint(content_handlers_recovered::CallCheckpoint)override;
    void clear_content_checkpoint()noexcept override;
    // Explicit per-message view over these SAME stream/clock/formatter roots.
    // Caller retains raw checkpoint backing and closes on the live child actor.
    // The creator-owned root outlives every open/failed view; no Graph cycle.
    std::shared_ptr<CallView> make_call_view(const application_io::CallContext&);
private:
    struct Impl;std::shared_ptr<Impl> impl_;
    Impl& attached()const;
    template<class F>auto invoke(F&&)->std::invoke_result_t<F>;
    template<class F>auto invoke_job(F&&,socket_recovered::SessionMemoryDomain&,
        const socket_recovered::SessionMemoryDomain::JobPermit&,content_handlers_recovered::CallCheckpoint)->std::invoke_result_t<F>;
};
class Information::CallView final:public handlers_recovered::Effects {
public:
    ~CallView()noexcept override;
    CallView(const CallView&)=delete;CallView& operator=(const CallView&)=delete;
    void log_profile_name_084f48(std::string_view)override;
    std::string current_time_041b60()override;
    // Normal retirement under the same live explicit context. Once closed,
    // destruction is metadata-only and does not call expired checkpoint data.
    void close();
private:
    friend class Information;friend struct Information::Impl;
    struct State;
    CallView(std::shared_ptr<Information::Impl>,std::shared_ptr<State>);
    std::shared_ptr<Information::Impl> owner_;
    std::shared_ptr<State> state_;
};
}
