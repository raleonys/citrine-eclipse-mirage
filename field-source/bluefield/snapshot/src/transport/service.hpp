// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/application/application.hpp"
#include <memory>
#include <cstdint>
#include <functional>
namespace preloader_l {
class Service final {
public:
    explicit Service(Environment&);
    ~Service();
    Service(const Service&)=delete;
    Service& operator=(const Service&)=delete;
    void start();
    void prepare();
    void activate();
    // Source bootstrap order: provider worker, synchronous true phase,
    // then an owned worker for the false phase after server+provider finish.
    void activate_async(std::function<void(bool)> extra_loader);
    bool wait_steps(std::int32_t);
    void wait_startup();
    void request_stop();
    void wait_ready_or_stopping();
    void stop();
    bool is_ready() const noexcept;
    std::uint16_t port() const;
    Application& application();
private:
    struct Impl;std::unique_ptr<Impl> impl_;
};
}
