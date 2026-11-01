// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <cstdint>
#include <string_view>

namespace preloader_l::crypt_query {
enum class Selection { original, companion };
inline constexpr char16_t companion_filename[]=u"EAAntiCheat.GameServiceLauncher.dll";
// Input path is a bounded copy of readable UTF-16. No caller memory is touched.
// Derived from preloader_core.cpp Core::crypt_query, original RVA 0x1264.
Selection select_object(std::uint32_t object_type,bool null_object,
                        std::u16string_view path,bool readable) noexcept;
}
