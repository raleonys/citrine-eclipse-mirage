// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>
namespace preloader_l {
// One bounded response plus an optional event reserved before the first write.
// Views must stay immutable until destruction. Commit is called exactly once
// after a full successful response write; destruction before it cancels work.
// A successful commit cannot be undone if a later event write fails. TCP write
// completion is local delivery evidence, not a remote-consumption acknowledgement.
class FrameDelivery {
public:
    virtual ~FrameDelivery()=default;
    virtual std::string_view response() const noexcept=0;
    virtual std::string_view completion_event() const noexcept=0;
    virtual bool commit() noexcept=0;
};
struct FrameObserver {
    void* context=nullptr;
    // Synchronous raw-frame observation. Exactly one callback must be supplied.
    // Context must survive quiescence; false rejects and closes this peer.
    bool (*received)(void*,std::string_view) noexcept=nullptr;
    // Write an owned bounded reply; length zero means no wire response.
    bool (*respond)(void*,std::string_view,char*,std::size_t,std::size_t*) noexcept=nullptr;
    // Alternative owned reply. Null selects the output buffer; zero written
    // with null means no response. The per-peer event slot
    // is occupied by this same object before response transmission begins.
    bool (*prepare)(void*,std::string_view,char*,std::size_t,std::size_t*,std::unique_ptr<FrameDelivery>&) noexcept=nullptr;
};
struct TcpSnapshot {
    std::uint16_t port=0;
    std::uint64_t accepted=0,active=0,completed=0,frames=0;
    std::uint64_t malformed=0,capacity=0,truncated=0,timeouts=0,failures=0;
    std::uint64_t responses=0,bytes_written=0;
    std::uint64_t events=0,committed_deliveries=0;
};
enum class TcpStartResult { started, invalid_state, invalid_observer, failed, busy };
class TcpIngress final {
public:
    static constexpr std::size_t response_capacity=16384;
    TcpIngress();
    ~TcpIngress();
    TcpIngress(const TcpIngress&)=delete;
    TcpIngress& operator=(const TcpIngress&)=delete;
    TcpStartResult start(FrameObserver observer,std::uint16_t port=0) noexcept;
    // Exactly one executor driver at a time; a competing driver returns false.
    // A zero budget performs at most one ready completion.
    bool poll_for(std::chrono::milliseconds budget) noexcept;
    bool stop_for(std::chrono::milliseconds budget) noexcept;
    bool quiescent() const noexcept;
    TcpSnapshot snapshot() const noexcept;
private:
    struct State;
    std::unique_ptr<State> state_;
};
}
