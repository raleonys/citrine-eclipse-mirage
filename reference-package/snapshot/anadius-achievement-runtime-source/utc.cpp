#include "utc.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>

namespace anadius::achievement_runtime {
std::int64_t HostClock::time_05eeb8() {
    return static_cast<std::int64_t>(std::time(nullptr));
}
std::optional<BrokenDown> HostClock::gmtime_05fffc(std::int64_t value) {
    const __time64_t seconds=value;
    std::tm fields{};
    if(::_gmtime64_s(&fields,&seconds)!=0)return std::nullopt;
    return BrokenDown{fields.tm_sec,fields.tm_min,fields.tm_hour,fields.tm_mday,
        fields.tm_mon,fields.tm_year,fields.tm_wday,fields.tm_yday,fields.tm_isdst};
}
std::string HostFormatter::put_time_03d610(const BrokenDown& value,std::string_view pattern) {
    const std::tm fields{value.second,value.minute,value.hour,value.day,value.month,
        value.year,value.weekday,value.yearday,value.daylight};
    std::ostringstream stream;
    const std::string owned_pattern(pattern);
    stream<<std::put_time(&fields,owned_pattern.c_str());
    // Original reads the string buffer without testing stream fail/bad bits.
    return stream.str();
}
std::string utc_time_041b60(Clock& clock,Formatter& formatter,std::uint64_t* epoch,
                          std::uint8_t filename_mode) {
    const auto now=clock.time_05eeb8();
    const auto fields=clock.gmtime_05fffc(now);
    if(!fields){
        if(epoch)*epoch=0;
        return {};
    }
    if(epoch)*epoch=static_cast<std::uint64_t>(calendar_recovered::epoch_041ff0(*fields));
    return formatter.put_time_03d610(*fields,filename_mode?
        "%Y-%m-%d_%H-%M-%S":"%Y-%m-%dT%H:%M:%S");
}
} // namespace anadius::achievement_runtime
