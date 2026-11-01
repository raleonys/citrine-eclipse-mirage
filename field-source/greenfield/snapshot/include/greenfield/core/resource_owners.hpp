#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>

namespace greenfield::core {
// An independent model of durable ownership for GF01, not linked to any
// Windows handle or the exported DllShutdown yet.
class ResourceOwners final {
public:
    using Cleanup = bool(*)(void* context, std::uintptr_t ticket) noexcept;
    static constexpr std::size_t capacity = 12;
    enum class Drain : std::uint32_t { complete = 0, retry_required = 1, busy = 2 };
    ResourceOwners() noexcept = default;
    ResourceOwners(const ResourceOwners&) = delete;
    ResourceOwners& operator=(const ResourceOwners&) = delete;
    [[nodiscard]] bool adopt(std::uintptr_t ticket, Cleanup cleanup, void* context) noexcept;
    [[nodiscard]] Drain drain() noexcept;
    [[nodiscard]] std::size_t remaining() const noexcept;
private:
    struct Entry { std::uintptr_t ticket{}; Cleanup cleanup{}; void* context{}; };
    mutable std::mutex mutex_;
    std::array<Entry,capacity> entries_{};
    std::size_t size_{};
    bool in_flight_{};
};
}
