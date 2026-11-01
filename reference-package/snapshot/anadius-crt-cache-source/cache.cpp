#include "cache.hpp"

namespace anadius::crt_cache_recovered {
void initialize_cookie_04c134(CookieState& state, EntropyEffects& effects) {
    if (state.cookie == default_cookie) {
        std::uint64_t time = 0;
        effects.system_time_as_file_time(time);
        time ^= effects.current_thread_id();
        time ^= effects.current_process_id();
        const auto observation = effects.query_performance_counter();
        // QueryPerformanceCounter's BOOL is ignored in the assembly. A failed
        // call that wrote a value still follows the same arithmetic. Without a
        // write, the original reads indeterminate stack storage: no value is
        // invented by this reconstruction.
        if (!observation.written_value) throw UninitializedCounterDomain{};
        const auto counter = *observation.written_value;
        const auto address = effects.entropy_stack_cell_address();
        auto value = ((counter << 32) ^ counter ^ time ^ address) &
                     std::uint64_t{0xffffffffffff};
        if (value == default_cookie) value = default_cookie + 1;
        state.cookie = value;
    }
    state.complement = ~state.cookie;
}

std::uint64_t initialize_thunks_067538(crt_runtime_recovered::State& state) noexcept {
    for (auto& cell : state.procedures)
        cell.store(state.cookie, std::memory_order_relaxed);
    return (state.cookie & ~std::uint64_t{0xff}) | 1;
}

bool uninitialize_thunks_06755c(crt_runtime_recovered::State& state,
                              CacheEffects& effects, std::uint8_t termination_flag) {
    if (termination_flag == 0) {
        for (auto& cell : state.modules) {
            const auto module = cell.load(std::memory_order_relaxed);
            if (module != 0) {
                if (module != crt_runtime_recovered::missing)
                    (void)effects.free_library(module);
                cell.store(0, std::memory_order_relaxed);
            }
        }
    }
    return true;
}

void initialize_05c7e4(EncodedGlobals& state, std::uint64_t cookie) noexcept {
    state.a4f00 = cookie;
}
void initialize_0634a8(EncodedGlobals& state, std::uint64_t cookie) noexcept {
    state.a5178 = cookie;
}
void initialize_05ea40(EncodedGlobals& state, std::uint64_t cookie) noexcept {
    state.a4f20[0] = cookie;
    state.a4f20[1] = cookie;
    state.a4f20[2] = cookie;
    state.a4f20[3] = cookie;
}
void initialize_0705f0(EncodedGlobals& state, std::uint64_t cookie) noexcept {
    state.a5a18 = cookie;
}
void initialize_05cd94(EncodedGlobals& state, std::uint64_t cookie) noexcept {
    state.a4f10 = cookie;
}
bool initialize_encoded_globals_0644b0(EncodedGlobals& state, std::uint64_t cookie) noexcept {
    initialize_05c7e4(state, cookie);
    initialize_0634a8(state, cookie);
    initialize_05ea40(state, cookie);
    initialize_0705f0(state, cookie);
    initialize_05cd94(state, cookie);
    return true;
}
} // namespace anadius::crt_cache_recovered
