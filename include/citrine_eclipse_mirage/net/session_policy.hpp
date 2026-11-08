// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <chrono>
#include <cstddef>
#include <optional>
#include <stdexcept>

namespace citrine_eclipse_mirage::net {
struct SessionPolicy {
    std::size_t max_sessions=128;
    std::size_t max_frame_bytes=1024*1024; // Wire bytes, including the NUL.
    std::size_t max_response_bytes=8*1024*1024; // Response plus event, or greeting.
    std::chrono::milliseconds receive_idle{30000},receive_frame{10000},send_idle{5000};
    void validate() const {
        if(!max_sessions||max_sessions>65535||!max_frame_bytes||!max_response_bytes||
           max_frame_bytes>1024ull*1024*1024||max_response_bytes>1024ull*1024*1024)
            throw std::invalid_argument("Invalid LSX resource policy");
        for(auto t:{receive_idle,receive_frame,send_idle})
            if(t<=std::chrono::milliseconds::zero()||t>std::chrono::hours(24))
                throw std::invalid_argument("Invalid LSX deadline policy");
    }
};
// Monotonic deadlines. Fragments never extend a started frame's fixed deadline;
// only positive socket progress extends idle deadlines. Paused handler work does
// not masquerade as socket receive progress.
class SessionDeadlines final {
public:
    using Clock=std::chrono::steady_clock;
    using Time=Clock::time_point;
    explicit SessionDeadlines(const SessionPolicy& policy):policy_(policy){}
    void begin_receive(Time now,bool buffered) {
        if(receive_)return;
        receive_=now+policy_.receive_idle;
        if(buffered)frame_=now+policy_.receive_frame;
    }
    void receive_progress(Time now,std::size_t bytes) {
        if(!bytes||!receive_)return;
        receive_=now+policy_.receive_idle;
        if(!frame_)frame_=now+policy_.receive_frame;
    }
    void end_receive(){receive_.reset();frame_.reset();}
    void begin_send(Time now){if(!send_)send_=now+policy_.send_idle;}
    void send_progress(Time now,std::size_t bytes){if(bytes&&send_)send_=now+policy_.send_idle;}
    void end_send(){send_.reset();}
    void clear(){end_receive();end_send();}
    std::optional<Time> next() const {
        std::optional<Time> result;
        for(auto t:{receive_,frame_,send_})if(t&&(!result||*t<*result))result=t;
        return result;
    }
    bool expired(Time now) const {const auto deadline=next();return deadline&&now>=*deadline;}
private:
    const SessionPolicy& policy_;
    std::optional<Time> receive_,frame_,send_;
};
} // namespace citrine_eclipse_mirage::net
