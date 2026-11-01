#pragma once
#include <cstdint>
#include "../anadius-crt-runtime-source/runtime.hpp"

namespace anadius::crt_locks_recovered {
enum class Bank { acrt, vcrt };
struct LockRef {
    Bank bank;
    std::int64_t index;
    constexpr std::int64_t byte_offset() const noexcept { return index * 40; }
    constexpr std::uintptr_t target_base_va() const noexcept {
        return bank == Bank::acrt ? 0x1800a4f40ULL : 0x1800a4e78ULL;
    }
    bool operator==(const LockRef&) const = default;
};
struct State { std::uint32_t acrt_count{}; std::uint32_t vcrt_count{}; };
class Effects {
public:
    virtual ~Effects() = default;
    virtual std::uint64_t initialize_acrt_06707c(LockRef, std::uint32_t spin,
                                               std::uint32_t flags) = 0;
    virtual std::uint64_t initialize_vcrt_050c38(LockRef, std::uint32_t spin,
                                               std::uint32_t flags) = 0;
    virtual void delete_critical_section(LockRef) = 0;
    virtual void enter_critical_section(LockRef) = 0;
    virtual void leave_critical_section(LockRef) = 0;
};
// Defined result is AL. Incidental upper RAX bits are deliberately not an API.
// State is externally serialized. Cleanup entry count must fit its real bank
// (ACRT 0..14, VCRT 0..1); otherwise std::domain_error before any provider call.
// Initializers require an initially empty bank. Callbacks may alter counters;
// each INC/DEC uses the current uint32_t cell and wraps as the machine does.
bool initialize_acrt_0625b0(State&, Effects&);
bool uninitialize_acrt_062614(State&, Effects&);
void lock_acrt_0625f8(Effects&, std::int32_t index);
void unlock_acrt_06264c(Effects&, std::int32_t index);
bool initialize_vcrt_05093c(State&, Effects&);
bool uninitialize_vcrt_050984(State&, Effects&);

class InitializeEffects {
public:
    virtual ~InitializeEffects() = default;
    virtual std::uint64_t call_initialize_ex(crt_runtime_recovered::Address,
        LockRef, std::uint32_t spin, std::uint32_t flags) = 0;
    virtual std::uint64_t initialize_and_spin(LockRef, std::uint32_t spin) = 0;
};
// The LockRef must identify a live, correctly typed provider-owned object.
// No target VA is converted to a pointer; no byte array poses as native CS.
std::uint64_t initialize_acrt_api_06707c(crt_runtime_recovered::State&,
    crt_runtime_recovered::Effects&, InitializeEffects&, LockRef,
    std::uint32_t spin, std::uint32_t flags);
} // namespace anadius::crt_locks_recovered
