#include <cstdint>

enum class PointerState : uint8_t { replacement, original, foreign };

struct RestoreState {
    PointerState pointer;
    bool original_protection_restored;
};

static bool restore_slot_model(RestoreState& state,
                               bool make_writable_ok,
                               bool restore_protection_ok) noexcept {
    const bool pointer_needs_restore = state.pointer == PointerState::replacement;
    if (!pointer_needs_restore && state.pointer != PointerState::original) return false;
    if (!make_writable_ok) return false;
    if (pointer_needs_restore) state.pointer = PointerState::original;
    state.original_protection_restored = restore_protection_ok;
    return restore_protection_ok && state.pointer == PointerState::original;
}

int main() {
    // First rollback restores the pointer but loses the protection restore.
    RestoreState retry{PointerState::replacement, false};
    if (restore_slot_model(retry, true, false)) return 1;
    if (retry.pointer != PointerState::original || retry.original_protection_restored) return 2;

    // F1443-002 closure: a later remove/retry must accept the already-original
    // pointer and finish the protection transaction instead of rejecting it.
    if (!restore_slot_model(retry, true, true)) return 3;
    if (retry.pointer != PointerState::original || !retry.original_protection_restored) return 4;

    // A foreign pointer is still rejected; recovery does not overwrite an
    // unrelated mutation.
    RestoreState foreign{PointerState::foreign, false};
    if (restore_slot_model(foreign, true, true)) return 5;
    if (foreign.pointer != PointerState::foreign) return 6;

    // Failure to make the page writable remains retryable without pointer drift.
    RestoreState blocked{PointerState::replacement, false};
    if (restore_slot_model(blocked, false, true)) return 7;
    if (blocked.pointer != PointerState::replacement) return 8;
    return 0;
}
