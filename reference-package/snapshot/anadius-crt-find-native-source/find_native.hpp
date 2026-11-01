#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <span>

namespace anadius::crt_find_native {
using Address = std::uint64_t;

// A new SDK boundary for the three import call sites in search_06e984.
// These are typed host function pointers, never original image addresses.
struct Api {
    decltype(&::FindFirstFileExW) first;
    decltype(&::FindNextFileW) next;
    decltype(&::FindClose) close;
};
Api system_api() noexcept;

class Session final {
public:
    explicit Session(Api api = system_api());
    // Deliberately releases host bookkeeping only. Search owns explicit close
    // order; this bridge does not invent original SEH or unwind cleanup.
    ~Session() = default;
    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
    Session(Session&&) = delete;
    Session& operator=(Session&&) = delete;

    Address first(Address wide_pattern, Address& filename);
    std::int32_t next(Address handle, Address& filename);
    void close(Address handle);
    std::span<const std::byte> filename_storage(Address handle) const;
    bool contains(Address address, std::uint64_t bytes) const noexcept;
    std::uint8_t read_u8(Address address) const;
    std::uint16_t read_u16(Address address) const;
    std::size_t active_count() const noexcept { return active_.size(); }

private:
    using Records = std::map<Address, std::unique_ptr<WIN32_FIND_DATAW>>;
    const WIN32_FIND_DATAW& record(Address handle) const;
    Api api_;
    Records active_;
};
} // namespace anadius::crt_find_native
