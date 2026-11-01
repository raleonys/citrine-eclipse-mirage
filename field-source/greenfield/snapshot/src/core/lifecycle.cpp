#include "greenfield/core/lifecycle.hpp"
namespace greenfield::core {
std::uint32_t Lifecycle::shutdown(std::uint32_t timeout_ms) noexcept {
    (void)timeout_ms; // No owned resources exist in GF00.
    phase_.store(Phase::stopped, std::memory_order_release);
    return 0u;
}
Phase Lifecycle::phase() const noexcept {
    return phase_.load(std::memory_order_acquire);
}
Lifecycle& runtime_lifecycle() noexcept {
    static Lifecycle instance;
    return instance;
}
}
