#pragma once
#include "preloader_l/runtime/calendar.hpp"
#include <optional>
#include <string>
#include <string_view>

namespace preloader_l::achievement_runtime {
using BrokenDown=calendar_recovered::BrokenDown;
class Clock {
public:
    virtual ~Clock()=default;
    virtual std::int64_t time_05eeb8()=0;
    // Own copy of the original gmtime result; nullopt models its null pointer.
    virtual std::optional<BrokenDown> gmtime_05fffc(std::int64_t)=0;
};
class Formatter {
public:
    virtual ~Formatter()=default;
    // Returns the explicit-length stream buffer, even if empty or partial.
    // Native STL locale, stream state, allocation and exceptions belong here.
    virtual std::string put_time_03d610(const BrokenDown&,std::string_view)=0;
};
// Explicit host-runtime compatibility choices, not recovered original CRT/ABI.
class HostClock final:public Clock {
public:
    std::int64_t time_05eeb8() override;
    std::optional<BrokenDown> gmtime_05fffc(std::int64_t) override;
};
class HostFormatter final:public Formatter {
public:
    std::string put_time_03d610(const BrokenDown&,std::string_view) override;
};
// 041b60 normal application flow. Nonzero low-byte mode chooses filename form.
// Optional output is recomputed from copied tm, before formatting can throw.
std::string utc_time_041b60(Clock&,Formatter&,std::uint64_t* epoch=nullptr,
                          std::uint8_t filename_mode=0);
} // namespace preloader_l::achievement_runtime
