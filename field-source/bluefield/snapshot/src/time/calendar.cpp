#include "src/time/calendar.hpp"
#include <bit>

namespace preloader_l::calendar {
namespace {
std::int32_t add(std::int32_t a,std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(a)+std::bit_cast<std::uint32_t>(b));
}
std::int32_t sub(std::int32_t a,std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(a)-std::bit_cast<std::uint32_t>(b));
}
std::int32_t mul(std::int32_t a,std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(a)*std::bit_cast<std::uint32_t>(b));
}
}
std::int64_t epoch_041ff0(const BrokenDown& time) noexcept {
    auto month=time.month;
    auto year=add(time.year,1900);
    if(month>11) {
        const auto years=month/12;
        year=add(year,years);
        month=sub(month,mul(years,12));
    }else if(month<0) {
        // SUB ECX,R8D wraps before the compiler's signed division by12.
        const auto years=sub(11,month)/12;
        year=sub(year,years);
        month=add(month,mul(years,12));
    }
    month=add(month,1);
    year=sub(year,month<=2?1:0);
    const auto era=year/400; // Native truncation towardzero, including BC years.
    const auto year_in_era=sub(year,mul(era,400));
    const auto month_index=add(month,month>2?-3:9);
    auto days=add(mul(month_index,153),2)/5;
    days=add(days,mul(era,146097));
    days=add(days,mul(year_in_era,365));
    days=sub(days,year_in_era/100);
    days=add(days,year_in_era/4);
    days=add(add(time.day,-719469),days);
    // CDQE at0420f1 ends the32-bit wrapping domain. This range cannot overflow64.
    return (((static_cast<std::int64_t>(days)*24+time.hour)*60+time.minute)*60+time.second);
}
} // namespace preloader_l::calendar_recovered
