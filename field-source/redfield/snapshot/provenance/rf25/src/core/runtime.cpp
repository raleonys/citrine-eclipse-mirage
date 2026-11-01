// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime.hpp"
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>

namespace redfield {
Runtime::Runtime(std::span<const Resource> resources)
    : resources_(resources.begin(), resources.end()) {
    for (const auto& resource : resources_)
        if (!resource.start || !resource.stop) throw std::invalid_argument("resource callbacks required");
}
Runtime::~Runtime() {
    // No hidden blocking cleanup (in particular no destructor-time unload).
    if (owned_ || active_) std::terminate();
}
Runtime::Lease::Lease(Lease&& other) noexcept : owner_(std::exchange(other.owner_, nullptr)) {}
Runtime::Lease& Runtime::Lease::operator=(Lease&& other) noexcept {
    if (this != &other) { release(); owner_ = std::exchange(other.owner_, nullptr); }
    return *this;
}
Runtime::Lease::~Lease() { release(); }
void Runtime::Lease::release() noexcept {
    if (auto* owner = std::exchange(owner_, nullptr)) owner->leave();
}
void Runtime::leave() noexcept {
    std::lock_guard lock(state_mutex_);
    --active_;
    if (!active_) drained_.notify_all();
}
Runtime::Lease Runtime::acquire() {
    std::lock_guard lock(state_mutex_);
    if (state_ != State::running || active_ == std::numeric_limits<std::size_t>::max()) return {};
    ++active_;
    return Lease(this);
}
RuntimeSnapshot Runtime::snapshot() const {
    std::lock_guard lock(state_mutex_);
    return {state_, owned_, active_};
}
bool Runtime::cleanup() {
    // operation_mutex_ serializes all cleanup; state_mutex_ protects observers.
    for (;;) {
        std::size_t count;
        { std::lock_guard lock(state_mutex_); count = owned_; }
        if (!count) {
            std::lock_guard lock(state_mutex_);
            state_ = State::stopped;
            return true;
        }
        const auto& resource = resources_[count - 1];
        if (!resource.stop(resource.context)) {
            std::lock_guard lock(state_mutex_);
            state_ = State::cleanup_pending;
            return false; // retain this resource AND all its dependencies
        }
        { std::lock_guard lock(state_mutex_); --owned_; }
    }
}
StartResult Runtime::start() {
    std::unique_lock operation(operation_mutex_, std::try_to_lock);
    if (!operation.owns_lock()) return StartResult::busy;
    {
        std::lock_guard lock(state_mutex_);
        if (state_ == State::running) return StartResult::already_running;
        if (state_ != State::idle) return StartResult::invalid_state;
        state_ = State::starting;
    }
    for (std::size_t i = 0; i < resources_.size(); ++i) {
        { std::lock_guard lock(state_mutex_); owned_ = i + 1; }
        if (!resources_[i].start(resources_[i].context))
            return cleanup() ? StartResult::start_failed : StartResult::cleanup_pending;
    }
    { std::lock_guard lock(state_mutex_); state_ = State::running; }
    return StartResult::started;
}
StopResult Runtime::stop(std::chrono::milliseconds drain_budget) {
    std::unique_lock operation(operation_mutex_, std::try_to_lock);
    if (!operation.owns_lock()) return StopResult::busy;
    {
        std::unique_lock lock(state_mutex_);
        if (state_ == State::stopped) return StopResult::stopped;
        state_ = State::draining;
        if (!drained_.wait_for(lock, drain_budget, [this] { return active_ == 0; }))
            return StopResult::timeout; // admission stays closed until retry
    }
    return cleanup() ? StopResult::stopped : StopResult::cleanup_pending;
}
}
