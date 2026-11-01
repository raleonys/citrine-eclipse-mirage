// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/server.hpp"
#include "preloader_l/net/session_policy.hpp"
#include <memory>
#include <exception>

namespace preloader_l::net {
class HandlerFactory {
public:
    virtual ~HandlerFactory() = default;
    virtual std::unique_ptr<server::Handler> create() = 0;
    virtual void error(std::string_view operation, std::exception_ptr) noexcept = 0;
};
// Concrete adapter for the existing RE server/Session bodies. The caller owns
// the factory and execution flags until stop returns. Control-thread start/stop
// are synchronous; sockets, IO completions and handler jobs are drained on stop.
// Two owned workers and a bounded nonblocking admission queue retain sessions.
// Each session submits one handler at a time, in wire order, protecting cipher
// state without a mutex race between independently launched message threads.
// Listener ports are selected automatically from listener_policy's fixed range.
// The port reference is output state after bind, never a requested config port.
class AsioService final {
public:
    AsioService(HandlerFactory&, execution::Execution&,
                std::uint16_t& port, std::uint8_t& ready,SessionPolicy policy={});
    ~AsioService();
    AsioService(const AsioService&) = delete;
    AsioService& operator=(const AsioService&) = delete;
    void start();
    void stop();
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
