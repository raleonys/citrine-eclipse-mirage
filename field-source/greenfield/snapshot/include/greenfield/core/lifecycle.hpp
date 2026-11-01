#pragma once
#include <atomic>
#include <cstdint>

namespace greenfield::core {
// ABI shape only: no WinSock, services, hooks, or background workers in GF00.
enum class Phase : std::uint32_t { dormant = 0, stopped = 1 };
// Mirrors the existing shutdown success result of 0 for the no-owner case.
// This does NOT implement shipping cleanup or runtime start.
class Lifecycle final {
public:
    Lifecycle() noexcept = default;
    Lifecycle(const Lifecycle&) = delete;
    Lifecycle& operator=(const Lifecycle&) = delete;
    [[nodiscard]] std::uint32_t shutdown(std::uint32_t timeout_ms) noexcept;
    [[nodiscard]] Phase phase() const noexcept;
private:
    std::atomic<Phase> phase_{Phase::dormant};
};
Lifecycle& runtime_lifecycle() noexcept;
}
