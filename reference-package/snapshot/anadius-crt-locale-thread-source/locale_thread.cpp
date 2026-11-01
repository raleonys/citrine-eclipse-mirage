#include "locale_thread.hpp"
#include <limits>

namespace anadius::crt_locale_thread_recovered {
Address update_thread_multibyte_data_internal_06f540(Address ptd, Address current_slot, Effects& fx) {
    const auto mask = fx.locale_mask_0a1c6c();
    const auto flags = fx.read_u32(ptd + 0x3a8);
    Address result;
    if ((flags & mask) != 0 && fx.read_address(ptd + 0x90) != 0) {
        result = fx.read_address(ptd + 0x88);
    } else {
        fx.lock_0625f8(5);
        result = fx.read_address(ptd + 0x88);
        if (result != fx.read_address(current_slot)) {
            if (result != 0) {
                const auto before = fx.atomic_fetch_add_32(result, std::numeric_limits<std::uint32_t>::max());
                if (before == 1 && result != fx.initial_multibyte_0a1c70())
                    fx.free_0668c0(result);
            }
            // The binary reloads the current pointer after the release/free effects.
            result = fx.read_address(current_slot);
            fx.write_address(ptd + 0x88, result);
            fx.atomic_fetch_add_32(result, 1);
        }
        fx.unlock_06264c(5);
    }
    if (result == 0) fx.abort_05f03c();
    return result;
}

Address update_thread_multibyte_06f658(Effects& fx) {
    const auto ptd = fx.required_ptd_064b90();
    return update_thread_multibyte_data_internal_06f540(ptd, fx.current_multibyte_slot_0a59d0(), fx);
}

Address update_locale_nolock_0709d8(Address destination, Address replacement, Effects& fx) {
    if (replacement == 0 || destination == 0) return 0;
    const auto old = fx.read_address(destination);
    if (old == replacement) return replacement;
    fx.write_address(destination, replacement);
    fx.add_locale_ref_070634(replacement);
    if (old != 0) {
        fx.release_locale_ref_0708c0(old);
        if (fx.read_u32(old + 0x10) == 0 && old != fx.initial_locale_0a17e0())
            fx.free_locale_0706c0(old);
    }
    return replacement;
}

Address update_thread_locale_070968(Effects& fx) {
    const auto ptd = fx.required_ptd_064b90();
    const auto flags = fx.read_u32(ptd + 0x3a8);
    const auto mask = fx.locale_mask_0a1c6c();
    Address result = 0;
    if ((mask & flags) != 0) result = fx.read_address(ptd + 0x90);
    if (result == 0) {
        fx.lock_0625f8(4);
        const auto replacement = fx.current_locale_0a52e0();
        result = update_locale_nolock_0709d8(ptd + 0x90, replacement, fx);
        fx.unlock_06264c(4);
        if (result == 0) fx.abort_05f03c();
    }
    return result;
}
}
