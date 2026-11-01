#include "ptd.hpp"
#include <limits>

namespace anadius::crt_ptd_recovered {
Ptd* construction_sentinel() noexcept {
    return reinterpret_cast<Ptd*>(std::numeric_limits<std::uintptr_t>::max());
}
namespace {
Ptd* acquire(State& state, Effects& fx) {
    if (state.fls_index != invalid_fls_index) {
        Ptd* existing = fx.fls_get_value(state.fls_index);
        if (existing != nullptr)
            return existing == construction_sentinel() ? nullptr : existing;
    }
    // The invalid-index case deliberately still attempts FlsSetValue.
    if (!fx.fls_set_value(state.fls_index, construction_sentinel())) return nullptr;
    Ptd* allocated = fx.calloc_ptd_066848();
    if (allocated == nullptr) {
        (void)fx.fls_set_value(state.fls_index, nullptr);
        fx.free_0668c0(nullptr);
        return nullptr;
    }
    if (!fx.fls_set_value(state.fls_index, allocated)) {
        (void)fx.fls_set_value(state.fls_index, nullptr);
        fx.free_0668c0(allocated);
        return nullptr;
    }
    // Publication precedes construction. No inferred exception rollback.
    fx.construct_ptd_array_0648fc(allocated);
    fx.free_0668c0(nullptr);
    return allocated;
}
}
void free_ptd_064b4c(State& state, Effects& fx) {
    if (state.fls_index == invalid_fls_index) return;
    Ptd* value = fx.fls_get_value(state.fls_index);
    if (value == nullptr) return;
    (void)fx.fls_set_value(state.fls_index, nullptr);
    fx.destroy_ptd_array_0649ec(value);
    fx.free_0668c0(value);
}
Ptd* optional_preserve_error_064d08(State& state, Effects& fx) {
    const auto saved = fx.last_error();
    Ptd* value = acquire(state, fx);
    fx.set_last_error(saved);
    return value;
}
Ptd* required_preserve_error_064b90(State& state, Effects& fx) {
    Ptd* value = optional_preserve_error_064d08(state, fx);
    if (value == nullptr) fx.abort_05f03c();
    return value;
}
Ptd* required_raw_064c64(State& state, Effects& fx) {
    Ptd* value = acquire(state, fx);
    if (value == nullptr) fx.abort_05f03c();
    return value;
}
Ptd* optional_raw_state0_064dd0(State& state, Effects& fx) {
    return acquire(state, fx);
}
bool initialize_064e90(State& state, Effects& fx) {
    state.fls_index = fx.fls_alloc(FlsCallback::destroy_0649cc);
    if (state.fls_index == invalid_fls_index) return false;
    if (optional_preserve_error_064d08(state, fx) != nullptr) return true;
    (void)uninitialize_064ecc(state, fx);
    return false;
}
bool uninitialize_064ecc(State& state, Effects& fx) {
    if (state.fls_index != invalid_fls_index) {
        (void)fx.fls_free(state.fls_index);
        state.fls_index = invalid_fls_index;
    }
    return true;
}
}
