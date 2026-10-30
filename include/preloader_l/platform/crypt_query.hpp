// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <wincrypt.h>
#include <string_view>
#include <cstdint>
#include <cstddef>

namespace preloader_l::hooks {
enum class Selection { original, companion };
inline constexpr char16_t companion_filename[]=u"EAAntiCheat.GameServiceLauncher.dll";
inline constexpr std::u16string_view target_suffix=u"preloader_l.dll";
// Includes the terminating NUL: at most 32767 non-NUL code units are valid.
inline constexpr std::size_t max_path_units=32768;
struct PathProbe {
    bool valid=false; // Readable, NUL-terminated and well-formed UTF-16.
    bool suffix_matches=false;
    std::size_t units_examined=0;
};
using ReadPathUnit=bool(*)(std::uintptr_t,char16_t&,void*) noexcept;
// Fixed-size suffix scratch; never rereads caller memory to decide a match.
PathProbe probe_path(const void*,ReadPathUnit,void*) noexcept;
Selection select_object(std::uint32_t,bool,const PathProbe&) noexcept;
}
namespace preloader_l::platform {
class CryptQueryHost {
public:
    virtual ~CryptQueryHost()=default;
    virtual hooks::PathProbe inspect_path(const void*) noexcept=0;
};
class WindowsCryptQueryHost final:public CryptQueryHost {
public:
    hooks::PathProbe inspect_path(const void*) noexcept override;
};
}
