#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace anadius::crt_invalid_recovered {
using Address = std::uintptr_t;
struct Arguments {
    const void* expression{};
    const void* function{};
    const void* file{};
    std::uint32_t line{};
    Address reserved{};
    bool operator==(const Arguments&) const = default;
};
// These are borrowed opaque addresses. No original VA is made callable.
struct Context {
    void* ptd{};
    std::uint64_t state_index{};
    std::uint8_t state_index_ready{};
    std::array<std::uint8_t, 7> padding17{};
    std::array<Address, 2> locale_snapshot{};
    std::uint8_t locale_mode{};
    std::array<std::uint8_t, 3> padding41{};
    std::uint32_t saved_errno{};
    std::uint8_t restore_errno{};
    std::array<std::uint8_t, 3> padding49{};
    std::uint32_t saved_doserrno{};
    std::uint8_t restore_doserrno{};
    std::array<std::uint8_t, 7> padding57{};
};
static_assert(sizeof(Address) == 8 && sizeof(Context) == 0x40);
static_assert(offsetof(Context, state_index_ready) == 0x10);
static_assert(offsetof(Context, locale_snapshot) == 0x18);
static_assert(offsetof(Context, locale_mode) == 0x28);
static_assert(offsetof(Context, saved_errno) == 0x2c);
static_assert(offsetof(Context, restore_errno) == 0x30);
static_assert(offsetof(Context, saved_doserrno) == 0x34);
static_assert(offsetof(Context, restore_doserrno) == 0x38);

class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint32_t last_error() = 0;
    virtual void set_last_error(std::uint32_t) = 0;
    // 064dd0 ignores its first argument in the bounded assembly. It is kept
    // as an explicit ABI forwarding parameter; production must not invent
    // writes to the saved error. Returned storage covers at least 0x3c0 bytes.
    virtual void* optional_ptd_064dd0(std::uint32_t* saved_error,
                                     std::uint64_t state_index) = 0;
    virtual std::uint32_t locale_changed_0a52d8() = 0;
    virtual std::array<Address, 2> locale_pair_0a1938() = 0;
    virtual Address encoded_global_handler_0a4f00(std::uint64_t state_index) = 0;
    virtual Address security_cookie_0a1580() = 0;
    // Must dispatch a rebuilt valid handler with its real ABI. This module
    // models the guarded call; it does not recreate XFG/CFG machinery.
    virtual void invoke_handler(Address, const Arguments&) = 0;
    [[noreturn]] virtual void watson_05c9a4(const Arguments&) = 0;
    [[noreturn]] virtual void missing_ptd_05f03c() = 0;
};

Context make_context(Effects&);
void* optional_context_ptd_05c5d0(Context&, Effects&);
void* required_context_ptd_052500(Context&, Effects&);
std::uint64_t state_index_05c63c(Context&, Effects&);
void release_context(Context&, Effects&);
void select_handler_05c888(const Arguments&, Context&, Effects&);
void invalid_parameter_05c7ec(const Arguments&, Effects&);
void invalid_parameter_noinfo_05c954(Effects&);
[[noreturn]] void invalid_parameter_noreturn_05c974(Effects&);
} // namespace anadius::crt_invalid_recovered
