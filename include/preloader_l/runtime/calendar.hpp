// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdint>

namespace preloader_l::calendar {
// Logical counterpart of the nine accessed/initialized 32-bit tm cells.
// Month is zero based and year is relative to1900. No native CRT ABI cast.
struct BrokenDown {
    std::int32_t second,minute,hour,day,month,year,weekday,yearday,daylight;
};
// 0x180041ff0. All calendar/day intermediates wrap32 exactly as the recorded
// x86 instructions. The final signed day count and h/m/s widen to signed64.
// No validation, OS time conversion, timezone or DST adjustment is performed.
std::int64_t epoch_041ff0(const BrokenDown&) noexcept;
} // namespace preloader_l::calendar
