// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/service_session.hpp"
#include "preloader_l/auth_code_session.hpp"
#include "preloader_l/achievement_session.hpp"
#include "preloader_l/tcp_ingress.hpp"
#include "preloader_l/port_publication.hpp"
namespace preloader_l {
// One serialized owner drives start/poll/stop. Snapshot counters alone may be
// read concurrently. Transport drains before its backing ServiceSession stops.
class LocalService final {
public:
    explicit LocalService(std::filesystem::path module_filename,PortBackend backend={}):session_(std::move(module_filename)),publication_(backend){}
    bool start(std::uint16_t port=0);
    void poll_for(std::chrono::milliseconds budget) noexcept {transport_.poll_for(budget);}
    bool stop_for(std::chrono::milliseconds budget);
    FileConfigResult load_status() const noexcept{return session_.load_status();}
    std::shared_ptr<const ConfigSnapshot> configuration(){return session_.configuration();}
    std::shared_ptr<const ConfigSnapshot> achievements() const{return achievements_.snapshot();}
    TcpSnapshot snapshot() const noexcept{return transport_.snapshot();}
    bool quiescent() const {
        const auto state=session_.snapshot().state;
        return !publication_.pending() && transport_.quiescent() && (state==State::idle || state==State::stopped);
    }
private:
    static bool frame(void*,std::string_view,char*,std::size_t,std::size_t*,std::unique_ptr<FrameDelivery>&) noexcept;
    ServiceSession session_;
    AuthCodeSession auth_code_;
    AchievementSession achievements_;
    TcpIngress transport_;
    PortPublication publication_;
};
}
