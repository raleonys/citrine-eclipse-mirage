// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
namespace redfield {
enum class HookPhase : unsigned { idle, staging, committed, retained, stopped };
// Serialized worker/explicit-cleanup owner. A failed stop retains modules,
// immutable configuration and every underlying MinHook ticket for retry.
class WindowsHooks final {
public:
    constexpr WindowsHooks() noexcept = default;
    bool start(std::shared_ptr<const ConfigSnapshot>,std::uint16_t) noexcept;
    bool stop(std::uint32_t timeout_ms) noexcept;
    HookPhase phase() const noexcept { return phase_.load(); }
private:
    void* modules_[3]{};
    bool configuration_bound_=false;
    std::atomic<HookPhase> phase_{HookPhase::idle};
};
}
