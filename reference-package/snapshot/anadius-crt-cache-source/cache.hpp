#pragma once
#include "../anadius-crt-runtime-source/runtime.hpp"
#include <optional>
#include <stdexcept>

namespace anadius::crt_cache_recovered {
inline constexpr std::uint64_t default_cookie = 0x2b992ddfa232;
struct CookieState {
    std::uint64_t cookie = default_cookie;       // original A1580
    std::uint64_t complement = ~default_cookie; // original A15C0
};
struct CounterObservation {
    bool api_result;
    std::optional<std::uint64_t> written_value;
};
class EntropyEffects {
public:
    virtual ~EntropyEffects() = default;
    // Argument starts as zero, matching the original FILETIME stack cell.
    virtual void system_time_as_file_time(std::uint64_t&) = 0;
    virtual std::uint32_t current_thread_id() = 0;
    virtual std::uint32_t current_process_id() = 0;
    virtual CounterObservation query_performance_counter() = 0;
    // Original LEA RBP-10; opaque provider value, not a guessed preferred VA.
    virtual std::uintptr_t entropy_stack_cell_address() = 0;
};
class UninitializedCounterDomain final : public std::domain_error {
public:
    UninitializedCounterDomain() : std::domain_error(
        "original QPC stack cell had no defined observed value") {}
};
void initialize_cookie_04c134(CookieState&, EntropyEffects&);

// RAX preserves cookie bits 8..63 and sets AL=1; callers that consume a
// Boolean must inspect AL, not require the full return register to equal 1.
std::uint64_t initialize_thunks_067538(crt_runtime_recovered::State&) noexcept;
class CacheEffects {
public:
    virtual ~CacheEffects() = default;
    virtual bool free_library(crt_runtime_recovered::Module) = 0;
};
// Only the low-byte true result is specified. A nonzero termination flag skips
// all 21 entries. Calls must be externally serialized against resolution.
bool uninitialize_thunks_06755c(crt_runtime_recovered::State&,
                              CacheEffects&, std::uint8_t termination_flag);

struct EncodedGlobals {
    std::uint64_t a4f00{}, a5178{};
    std::array<std::uint64_t, 4> a4f20{};
    std::uint64_t a5a18{}, a4f10{};
};
void initialize_05c7e4(EncodedGlobals&, std::uint64_t) noexcept;
void initialize_0634a8(EncodedGlobals&, std::uint64_t) noexcept;
void initialize_05ea40(EncodedGlobals&, std::uint64_t) noexcept;
void initialize_0705f0(EncodedGlobals&, std::uint64_t) noexcept;
void initialize_05cd94(EncodedGlobals&, std::uint64_t) noexcept;
bool initialize_encoded_globals_0644b0(EncodedGlobals&, std::uint64_t) noexcept;
} // namespace anadius::crt_cache_recovered
