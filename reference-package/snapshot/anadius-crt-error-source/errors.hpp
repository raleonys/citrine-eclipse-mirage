#pragma once
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace anadius::crt_errors_recovered {
// Borrowed four-byte storage. memcpy permits actual DWORD/int objects and
// unaligned byte fixtures without a forged uint32_t reference or aliasing UB.
class Cell32 {
public:
    explicit Cell32(void* storage) : storage_(storage) {}
    std::uint32_t load() const { std::uint32_t v; std::memcpy(&v, storage_, 4); return v; }
    void store(std::uint32_t v) const { std::memcpy(storage_, &v, 4); }
    void* raw() const { return storage_; }
private:
    void* storage_;
};
struct State {
    // Initial image bytes at 1800a17a8 / 1800a17ac, not zero initializers.
    std::uint32_t errno_fallback{12};
    std::uint32_t dos_fallback{8};
};
class Effects {
public:
    virtual ~Effects() = default;
    // Null or borrowed writable storage covering PTD offsets 0x20..0x27.
    // Lifetime extends through the selected cell's use. Lookup may throw;
    // caller owns TLS/FLS and any lifetime synchronization.
    virtual void* optional_ptd_064d08() = 0;
};
std::uint32_t errno_from_os_error_05d650(std::uint32_t code) noexcept;
Cell32 dos_error_cell_05d704(State&, Effects&);
Cell32 errno_cell_05d724(State&, Effects&);
void map_os_error_05d698(std::uint32_t code, State&, Effects&);
}
