#include "locks.hpp"
#include <array>
#include <stdexcept>

namespace anadius::crt_locks_recovered {
namespace {
std::uint32_t& count(State& state, Bank bank) {
    return bank == Bank::acrt ? state.acrt_count : state.vcrt_count;
}
std::uint32_t capacity(Bank bank) { return bank == Bank::acrt ? 14u : 1u; }
bool uninitialize(State& state, Effects& effects, Bank bank) {
    auto& cell = count(state, bank);
    // Original EBX snapshots once: mutations of the global cell by callbacks
    // cannot change the number or the indices of the subsequent deletions.
    std::uint32_t remaining = cell;
    if (remaining > capacity(bank))
        throw std::domain_error("CRT lock cleanup count exceeds physical bank");
    while (remaining != 0) {
        --remaining;
        effects.delete_critical_section({bank, remaining});
        --cell;
    }
    return true;
}
bool initialize(State& state, Effects& effects, Bank bank) {
    auto& cell = count(state, bank);
    if (cell != 0)
        throw std::domain_error("CRT lock initialization requires empty bank");
    for (std::uint32_t index = 0; index < capacity(bank); ++index) {
        const LockRef lock{bank, index};
        const auto result = bank == Bank::acrt
            ? effects.initialize_acrt_06707c(lock, 4000, 0)
            : effects.initialize_vcrt_050c38(lock, 4000, 0);
        // TEST EAX, EAX, not TEST AL, AL, nor a 64-bit pointer test.
        if (static_cast<std::uint32_t>(result) == 0) {
            uninitialize(state, effects, bank);
            return false;
        }
        ++cell;
    }
    return true;
}
}
bool initialize_acrt_0625b0(State& s, Effects& e) { return initialize(s,e,Bank::acrt); }
bool uninitialize_acrt_062614(State& s, Effects& e) { return uninitialize(s,e,Bank::acrt); }
void lock_acrt_0625f8(Effects& e, std::int32_t index) {
    e.enter_critical_section({Bank::acrt, static_cast<std::int64_t>(index)});
}
void unlock_acrt_06264c(Effects& e, std::int32_t index) {
    e.leave_critical_section({Bank::acrt, static_cast<std::int64_t>(index)});
}
bool initialize_vcrt_05093c(State& s, Effects& e) { return initialize(s,e,Bank::vcrt); }
bool uninitialize_vcrt_050984(State& s, Effects& e) { return uninitialize(s,e,Bank::vcrt); }

std::uint64_t initialize_acrt_api_06707c(crt_runtime_recovered::State& state,
    crt_runtime_recovered::Effects& runtime, InitializeEffects& effects,
    LockRef lock, std::uint32_t spin, std::uint32_t flags) {
    constexpr std::array<std::uint32_t,2> candidates{7,16};
    const auto procedure = crt_runtime_recovered::resolve_0669b0(
        state, runtime, 15, "InitializeCriticalSectionEx", candidates);
    if (procedure == 0) return effects.initialize_and_spin(lock, spin);
    return effects.call_initialize_ex(procedure, lock, spin, flags);
}
} // namespace anadius::crt_locks_recovered
