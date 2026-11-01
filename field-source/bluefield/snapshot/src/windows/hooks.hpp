// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "src/transport/service.hpp"
#include <chrono>
#include <memory>
namespace preloader_l {
class NativeHooks final {
public:
    NativeHooks(Environment&,Service&);
    ~NativeHooks();
    NativeHooks(const NativeHooks&)=delete;
    NativeHooks& operator=(const NativeHooks&)=delete;
    void install();
    bool quiesce(std::chrono::milliseconds timeout);
private:
    struct Impl;std::unique_ptr<Impl> impl_;
};
}
