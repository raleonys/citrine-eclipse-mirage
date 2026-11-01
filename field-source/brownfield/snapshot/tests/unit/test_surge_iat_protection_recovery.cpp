#include <cstdint>

enum class WriteOutcome : uint8_t {
    not_written = 0,
    written_restored = 1,
    rolled_back_restored = 2,
    rolled_back_protection_pending = 3,
};

enum class PointerState : uint8_t { original, replacement, foreign };

struct PatchState {
    PointerState pointer{PointerState::original};
    uint32_t original_protect{0x20u};
    bool protection_recovery_pending{false};
    bool active{false};
};

static WriteOutcome install_model(PatchState& patch,
                                  bool make_writable_ok,
                                  bool first_restore_ok,
                                  bool second_restore_ok) noexcept {
    if (patch.pointer != PointerState::original || !make_writable_ok)
        return WriteOutcome::not_written;
    patch.pointer = PointerState::replacement;
    if (first_restore_ok) {
        patch.active = true;
        patch.protection_recovery_pending = false;
        return WriteOutcome::written_restored;
    }

    patch.pointer = PointerState::original;
    if (second_restore_ok) {
        patch.active = false;
        patch.protection_recovery_pending = false;
        return WriteOutcome::rolled_back_restored;
    }

    // F1443-004 closure: functional rollback is complete, but protection
    // recovery ownership remains durable instead of disappearing.
    patch.active = true;
    patch.protection_recovery_pending = true;
    return WriteOutcome::rolled_back_protection_pending;
}

static bool recover_protection_model(PatchState& patch, bool restore_ok) noexcept {
    if (!patch.active || !patch.protection_recovery_pending ||
        patch.pointer != PointerState::original)
        return false;
    if (!restore_ok) return false;
    patch.protection_recovery_pending = false;
    patch.active = false;
    return true;
}

int main() {
    PatchState degraded{};
    if (install_model(degraded, true, false, false) !=
        WriteOutcome::rolled_back_protection_pending) return 1;
    if (degraded.pointer != PointerState::original) return 2;
    if (!degraded.active || !degraded.protection_recovery_pending) return 3;

    // A failed explicit recovery must preserve durable ownership for retry.
    if (recover_protection_model(degraded, false)) return 4;
    if (!degraded.active || !degraded.protection_recovery_pending) return 5;

    // A later teardown retry restores protection and clears the ledger.
    if (!recover_protection_model(degraded, true)) return 6;
    if (degraded.active || degraded.protection_recovery_pending) return 7;

    PatchState clean{};
    if (install_model(clean, true, true, true) != WriteOutcome::written_restored) return 8;
    if (!clean.active || clean.protection_recovery_pending ||
        clean.pointer != PointerState::replacement) return 9;

    PatchState rolled_back{};
    if (install_model(rolled_back, true, false, true) !=
        WriteOutcome::rolled_back_restored) return 10;
    if (rolled_back.active || rolled_back.protection_recovery_pending ||
        rolled_back.pointer != PointerState::original) return 11;

    PatchState foreign{};
    foreign.pointer = PointerState::foreign;
    if (install_model(foreign, true, true, true) != WriteOutcome::not_written) return 12;
    if (foreign.active || foreign.protection_recovery_pending ||
        foreign.pointer != PointerState::foreign) return 13;
    return 0;
}
