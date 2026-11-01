#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace anadius::vcrt_ptd_recovered {
// VCRT exception state, distinct from ACRT's 0x3c8-byte PTD.
struct alignas(8) Ptd {
    std::array<std::byte, 0x78> prefix{};
    std::int32_t frame_state{};
    std::array<std::byte, 4> suffix{};
};
static_assert(sizeof(Ptd) == 0x80);
static_assert(offsetof(Ptd, frame_state) == 0x78);
inline constexpr std::uint32_t invalid_fls_index = 0xffffffffu;
struct State {
    explicit State(Ptd& initial) : static_ptd(initial) {}
    std::uint32_t fls_index = invalid_fls_index;
    Ptd& static_ptd; // Rebuilt identity for original DAT_1800a4d90.
};
enum class FlsCallback { destroy_04d5d4 };
// A comparison/Effects token only. Never dereference this pointer.
void* construction_sentinel() noexcept;
struct Effects {
    virtual ~Effects() = default;
    virtual std::uint32_t fls_alloc_050b0c(FlsCallback) = 0;
    virtual bool fls_free_050b54(std::uint32_t index) = 0;
    virtual void* fls_get_050b9c(std::uint32_t index) = 0;
    virtual bool fls_set_050be4(std::uint32_t index, void* value) = 0;
    // Must return a zeroed live 128-byte Ptd, or null.
    virtual Ptd* calloc_05cae4(std::size_t count, std::size_t bytes) = 0;
    // 05caec is a direct forwarding wrapper to 0668c0.
    virtual void free_05caec(void* value) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual void set_last_error(std::uint32_t value) = 0;
    virtual bool initialize_locks_05093c() = 0;
    virtual void uninitialize_locks_050984() = 0;
    [[noreturn]] virtual void abort_05f03c() = 0;
};
void free_callback_04d5d4(State&, Effects&, void*);
void free_current_04d5f4(State&, Effects&);
Ptd* get_required_04d63c(State&, Effects&);
Ptd* get_noexit_04d658(State&, Effects&);
Ptd* get_noinit_04d718(State&, Effects&);
bool initialize_ptd_04d768(State&, Effects&);
bool uninitialize_ptd_04d7b0(State&, Effects&);
// Boolean AL contracts only: upper RAX bits are not recovered return values.
bool initialize_04d55c(State&, Effects&);
bool thread_attach_04d584(State&, Effects&);
bool thread_detach_04d598(State&, Effects&);
bool uninitialize_04d5a8(State&, Effects&, std::uint8_t terminating);
bool late_uninitialize_04d5c4(State&, Effects&);
}
