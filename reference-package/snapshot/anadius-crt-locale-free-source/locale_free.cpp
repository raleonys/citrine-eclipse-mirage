#include "locale_free.hpp"

namespace anadius::crt_locale_free_recovered {
void free_pointer_array_0713bc(Address current, std::uint64_t count, Effects& fx) {
    // LEA RCX+RDX*8 and ADD RBX,8 operate modulo 2^64. The endpoint,
    // rather than a count-decrement loop, preserves the wrapped-zero case.
    const Address end = current + count * 8;
    while (current != end) {
        fx.free_0668c0(fx.read_address(current));
        current += 8;
    }
}

void free_time_071730(Address time, Effects& fx) {
    if (time == 0) return;
    free_pointer_array_0713bc(time, 7, fx);
    free_pointer_array_0713bc(time + 0x38, 7, fx);
    free_pointer_array_0713bc(time + 0x70, 12, fx);
    free_pointer_array_0713bc(time + 0xd0, 12, fx);
    free_pointer_array_0713bc(time + 0x130, 2, fx);
    fx.free_0668c0(fx.read_address(time + 0x140));
    fx.free_0668c0(fx.read_address(time + 0x148));
    fx.free_0668c0(fx.read_address(time + 0x150));
    free_pointer_array_0713bc(time + 0x160, 7, fx);
    free_pointer_array_0713bc(time + 0x198, 7, fx);
    free_pointer_array_0713bc(time + 0x1d0, 12, fx);
    free_pointer_array_0713bc(time + 0x230, 12, fx);
    free_pointer_array_0713bc(time + 0x290, 2, fx);
    fx.free_0668c0(fx.read_address(time + 0x2a0));
    fx.free_0668c0(fx.read_address(time + 0x2a8));
    fx.free_0668c0(fx.read_address(time + 0x2b0));
    fx.free_0668c0(fx.read_address(time + 0x2b8));
}

void free_time_if_unreferenced_070860(Address time, const Defaults& defaults, Effects& fx) {
    if (time != 0 && time != defaults.initial_time_08ad20 && fx.read_u32(time + 0x15c) == 0) {
        free_time_071730(time, fx);
        fx.free_0668c0(time);
    }
}

void free_monetary_070a40(Address lconv, const Defaults& defaults, Effects& fx) {
    if (lconv == 0) return;
    constexpr std::array<std::uint64_t, 13> offsets{
        0x18, 0x20, 0x28, 0x30, 0x38, 0x40, 0x48,
        0x68, 0x70, 0x78, 0x80, 0x88, 0x90};
    for (const auto offset : offsets) {
        const auto pointer = fx.read_address(lconv + offset);
        if (pointer != defaults.lconv_pointer_values[offset / 8]) fx.free_0668c0(pointer);
    }
}

void free_numeric_071068(Address lconv, const Defaults& defaults, Effects& fx) {
    if (lconv == 0) return;
    constexpr std::array<std::uint64_t, 5> offsets{0, 8, 0x10, 0x58, 0x60};
    for (const auto offset : offsets) {
        const auto pointer = fx.read_address(lconv + offset);
        if (pointer != defaults.lconv_pointer_values[offset / 8]) fx.free_0668c0(pointer);
    }
}

void free_locale_0706c0(Address locale, const Defaults& defaults, Effects& fx) {
    const Address lconv = fx.read_address(locale + 0xf8);
    if (lconv != 0 && lconv != defaults.lconv_0a1700) {
        const auto ref = fx.read_address(locale + 0xe0);
        if (ref != 0 && fx.read_u32(ref) == 0) {
            const auto monetary = fx.read_address(locale + 0xf0);
            if (monetary != 0 && fx.read_u32(monetary) == 0) {
                fx.free_0668c0(monetary);
                free_monetary_070a40(fx.read_address(locale + 0xf8), defaults, fx);
            }
            const auto numeric = fx.read_address(locale + 0xe8);
            if (numeric != 0 && fx.read_u32(numeric) == 0) {
                fx.free_0668c0(numeric);
                free_numeric_071068(fx.read_address(locale + 0xf8), defaults, fx);
            }
            fx.free_0668c0(fx.read_address(locale + 0xe0));
            fx.free_0668c0(fx.read_address(locale + 0xf8));
        }
    }
    const auto ctype_ref = fx.read_address(locale + 0x100);
    if (ctype_ref != 0 && fx.read_u32(ctype_ref) == 0) {
        fx.free_0668c0(fx.read_address(locale + 0x108) - 0xfe);
        fx.free_0668c0(fx.read_address(locale + 0x110) - 0x80);
        fx.free_0668c0(fx.read_address(locale + 0x118) - 0x80);
        fx.free_0668c0(fx.read_address(locale + 0x100));
    }
    free_time_if_unreferenced_070860(fx.read_address(locale + 0x120), defaults, fx);
    Address name_ref = locale + 0x38;
    Address category_extra = locale + 0x128;
    for (unsigned category = 0; category != 6; ++category) {
        const auto category_name = fx.read_address(name_ref - 0x10);
        if (category_name != defaults.category_name_0a1948) {
            const auto ref = fx.read_address(name_ref);
            if (ref != 0 && fx.read_u32(ref) == 0) {
                fx.free_0668c0(ref);
                fx.free_0668c0(fx.read_address(category_extra));
            }
        }
        if (fx.read_address(name_ref - 0x18) != 0) {
            const auto ref = fx.read_address(name_ref - 8);
            if (ref != 0 && fx.read_u32(ref) == 0) fx.free_0668c0(ref);
        }
        name_ref += 0x20;
        category_extra += 8;
    }
    fx.free_0668c0(locale);
}
}
