#pragma once
#include "../anadius-thread-source/threads.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace anadius::crt_ptd_recovered {
struct Ptd {
    std::array<std::byte, 0x3c0> storage{};
    threads_recovered::Context* thread_context{};
};
static_assert(sizeof(Ptd) == 0x3c8);
static_assert(offsetof(Ptd, thread_context) == 0x3c0);
inline constexpr std::uint32_t invalid_fls_index = 0xffffffffu;
struct State { std::uint32_t fls_index = invalid_fls_index; };
enum class FlsCallback { destroy_0649cc };
// This token is only compared or passed through Effects; never dereference it.
Ptd* construction_sentinel() noexcept;
struct Effects {
    virtual ~Effects() = default;
    virtual Ptd* fls_get_value(std::uint32_t index) = 0;
    virtual bool fls_set_value(std::uint32_t index, Ptd* value) = 0;
    virtual std::uint32_t fls_alloc(FlsCallback callback) = 0;
    virtual bool fls_free(std::uint32_t index) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual void set_last_error(std::uint32_t error) = 0;
    // Must return a zeroed, live Ptd or null. No allocator is substituted here.
    virtual Ptd* calloc_ptd_066848() = 0;
    virtual void free_0668c0(Ptd* value) = 0;
    virtual void construct_ptd_array_0648fc(Ptd* value) = 0;
    virtual void destroy_ptd_array_0649ec(Ptd* value) = 0;
    [[noreturn]] virtual void abort_05f03c() = 0;
};
void free_ptd_064b4c(State&, Effects&);
Ptd* required_preserve_error_064b90(State&, Effects&);
Ptd* required_raw_064c64(State&, Effects&);
Ptd* optional_preserve_error_064d08(State&, Effects&);
// 064dd0's ignored first parameter and state index zero only. Other indices
// imply storage outside the single allocated Ptd and remain outside this model.
Ptd* optional_raw_state0_064dd0(State&, Effects&);
// Only AL is defined as a boolean by the original initializer/uninitializer.
bool initialize_064e90(State&, Effects&);
bool uninitialize_064ecc(State&, Effects&);
}
