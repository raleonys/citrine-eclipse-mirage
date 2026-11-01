#include "api.hpp"

namespace anadius::vcrt_api_recovered {
static_assert(sizeof(Address) == 8 && sizeof(Module) == 8);
Address resolve_0509bc(State& state, Effects& effects, std::uint32_t procedure,
                      std::string_view name, std::span<const std::uint32_t> candidates) {
    const Address cached = state.procedures[procedure].load(std::memory_order_relaxed);
    if (cached == missing) return 0;
    if (cached != 0) return cached;
    for (const auto index : candidates) {
        Module local = state.modules[index].load(std::memory_order_relaxed);
        if (local == missing) continue;
        if (local == 0) {
            const auto module_name = state.module_names[index];
            local = effects.load_library_ex_w(module_name, 0, 0x800);
            if (local == 0 && effects.last_error() == 87 &&
                !module_name.starts_with(u"api-ms-")) {
                local = effects.load_library_ex_w(module_name, 0, 0);
            }
            if (local == 0) {
                state.modules[index].exchange(missing);
                continue;
            }
            const Module previous = state.modules[index].exchange(local);
            if (previous != 0) effects.free_library(local);
            // The observed code uses local even when its reference was freed;
            // it does not substitute previous or compare_exchange the cell.
        }
        const Address result = effects.get_proc_address(local, name);
        if (result != 0) {
            state.procedures[procedure].exchange(result);
            return result;
        }
        // A loaded module without this export stops the candidate scan.
        break;
    }
    state.procedures[procedure].exchange(missing);
    return 0;
}

namespace {
constexpr std::array<std::uint32_t, 2> fibers{0, 2};
constexpr std::array<std::uint32_t, 2> synchronization{1, 2};
}
std::uint32_t fls_alloc_050b0c(State& s, Effects& e, FlsCallback callback) {
    const auto address = resolve_0509bc(s, e, 0, "FlsAlloc", fibers);
    // Original TLS fallback discards the destructor callback.
    return address ? e.call_fls_alloc(address, callback) : e.tls_alloc();
}
std::int32_t fls_free_050b54(State& s, Effects& e, std::uint32_t index) {
    const auto address = resolve_0509bc(s, e, 1, "FlsFree", fibers);
    return address ? e.call_fls_free(address, index) : e.tls_free(index);
}
void* fls_get_value_050b9c(State& s, Effects& e, std::uint32_t index) {
    const auto address = resolve_0509bc(s, e, 2, "FlsGetValue", fibers);
    return address ? e.call_fls_get_value(address, index) : e.tls_get_value(index);
}
std::int32_t fls_set_value_050be4(State& s, Effects& e, std::uint32_t index, void* value) {
    const auto address = resolve_0509bc(s, e, 3, "FlsSetValue", fibers);
    return address ? e.call_fls_set_value(address, index, value) : e.tls_set_value(index, value);
}
std::int32_t initialize_critical_section_ex_050c38(State& s, Effects& e, void* section,
                                                 std::uint32_t spin, std::uint32_t flags) {
    const auto address = resolve_0509bc(s, e, 4, "InitializeCriticalSectionEx", synchronization);
    return address ? e.call_initialize_critical_section_ex(address, section, spin, flags)
                   : e.initialize_critical_section_and_spin_count(section, spin);
}
} // namespace anadius::vcrt_api_recovered
