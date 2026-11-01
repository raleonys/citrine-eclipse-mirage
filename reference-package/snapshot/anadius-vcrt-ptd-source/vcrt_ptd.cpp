#include "vcrt_ptd.hpp"
#include <limits>

namespace anadius::vcrt_ptd_recovered {
void* construction_sentinel() noexcept {
    return reinterpret_cast<void*>(std::numeric_limits<std::uintptr_t>::max());
}
void free_callback_04d5d4(State& state, Effects& effects, void* value) {
    if (value != nullptr && value != &state.static_ptd)
        effects.free_05caec(value);
}
void free_current_04d5f4(State& state, Effects& effects) {
    if (state.fls_index == invalid_fls_index) return;
    void* value = effects.fls_get_050b9c(state.fls_index);
    (void)effects.fls_set_050be4(state.fls_index, nullptr);
    free_callback_04d5d4(state, effects, value);
}
Ptd* get_noexit_04d658(State& state, Effects& effects) {
    if (state.fls_index == invalid_fls_index) return nullptr;
    const auto saved_error = effects.last_error();
    void* value = effects.fls_get_050b9c(state.fls_index);
    Ptd* result = nullptr;
    if (value != construction_sentinel()) {
        if (value != nullptr) {
            result = static_cast<Ptd*>(value);
        } else if (effects.fls_set_050be4(state.fls_index, construction_sentinel())) {
            Ptd* allocated = effects.calloc_05cae4(1, 0x80);
            void* discarded = allocated;
            if (allocated != nullptr && effects.fls_set_050be4(state.fls_index, allocated)) {
                allocated->frame_state = -2;
                result = allocated;
                discarded = nullptr;
            } else {
                (void)effects.fls_set_050be4(state.fls_index, nullptr);
            }
            // Includes free(nullptr) on both successful allocation and OOM.
            effects.free_05caec(discarded);
        }
    }
    effects.set_last_error(saved_error);
    return result;
}
Ptd* get_noinit_04d718(State& state, Effects& effects) {
    if (state.fls_index == invalid_fls_index) return nullptr;
    const auto saved_error = effects.last_error();
    void* value = effects.fls_get_050b9c(state.fls_index);
    effects.set_last_error(saved_error);
    return value == construction_sentinel() ? nullptr : static_cast<Ptd*>(value);
}
Ptd* get_required_04d63c(State& state, Effects& effects) {
    if (Ptd* value = get_noexit_04d658(state, effects)) return value;
    effects.abort_05f03c();
}
bool initialize_ptd_04d768(State& state, Effects& effects) {
    state.fls_index = effects.fls_alloc_050b0c(FlsCallback::destroy_04d5d4);
    if (state.fls_index == invalid_fls_index) return false;
    if (effects.fls_set_050be4(state.fls_index, &state.static_ptd)) {
        state.static_ptd.frame_state = -2;
        return true;
    }
    (void)uninitialize_ptd_04d7b0(state, effects);
    return false;
}
bool uninitialize_ptd_04d7b0(State& state, Effects& effects) {
    if (state.fls_index != invalid_fls_index) {
        (void)effects.fls_free_050b54(state.fls_index);
        state.fls_index = invalid_fls_index;
    }
    return true;
}
bool initialize_04d55c(State& state, Effects& effects) {
    if (!effects.initialize_locks_05093c()) return false;
    if (initialize_ptd_04d768(state, effects)) return true;
    effects.uninitialize_locks_050984();
    return false;
}
bool thread_attach_04d584(State& state, Effects& effects) {
    return get_noexit_04d658(state, effects) != nullptr;
}
bool thread_detach_04d598(State& state, Effects& effects) {
    free_current_04d5f4(state, effects);
    return true;
}
bool uninitialize_04d5a8(State& state, Effects& effects, std::uint8_t terminating) {
    if (terminating == 0) {
        (void)uninitialize_ptd_04d7b0(state, effects);
        effects.uninitialize_locks_050984();
    }
    return true;
}
bool late_uninitialize_04d5c4(State& state, Effects& effects) {
    (void)uninitialize_ptd_04d7b0(state, effects);
    return true;
}
}
