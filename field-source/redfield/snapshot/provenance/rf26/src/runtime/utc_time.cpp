// SPDX-License-Identifier: GPL-3.0-only
#if !defined(_WIN32)
#include <chrono>
#endif
#include "preloader_l/runtime/utc_time.hpp"

#if defined(_WIN32)
extern "C" {
struct PreloaderFileTime302 { uint32_t low; uint32_t high; };
__declspec(dllimport) void __stdcall GetSystemTimeAsFileTime(PreloaderFileTime302*);
}
#endif

namespace preloader_l { namespace runtime {
namespace {
constexpr uint64_t k_filetime_epoch = UINT64_C(116444736000000000);
constexpr uint64_t k_ticks_per_second = UINT64_C(10000000);

void put2(char* p, unsigned v) noexcept {
    p[0]=static_cast<char>('0'+(v/10u)%10u);
    p[1]=static_cast<char>('0'+v%10u);
}
void put4(char* p, int v) noexcept {
    unsigned x=v<0?0u:static_cast<unsigned>(v);
    p[0]=static_cast<char>('0'+(x/1000u)%10u);
    p[1]=static_cast<char>('0'+(x/100u)%10u);
    p[2]=static_cast<char>('0'+(x/10u)%10u);
    p[3]=static_cast<char>('0'+x%10u);
}

// Proleptic-Gregorian civil date conversion for Unix-day zero = 1970-01-01.
void civil_from_days(int64_t z, int* year, unsigned* month, unsigned* day) noexcept {
    z += 719468;
    const int64_t era=(z>=0 ? z : z-146096)/146097;
    const unsigned doe=static_cast<unsigned>(z-era*146097);
    const unsigned yoe=(doe-doe/1460+doe/36524-doe/146096)/365;
    int y=static_cast<int>(yoe)+static_cast<int>(era*400);
    const unsigned doy=doe-(365*yoe+yoe/4-yoe/100);
    const unsigned mp=(5*doy+2)/153;
    const unsigned d=doy-(153*mp+2)/5+1;
    const int mi=static_cast<int>(mp)+(mp<10?3:-9);
    const unsigned m=static_cast<unsigned>(mi);
    y += (m<=2);
    *year=y; *month=m; *day=d;
}
}

uint64_t current_unix_seconds() noexcept {
#if defined(_WIN32)
    PreloaderFileTime302 ft{};
    GetSystemTimeAsFileTime(&ft);
    const uint64_t ticks=(static_cast<uint64_t>(ft.high)<<32)|ft.low;
    if(ticks<=k_filetime_epoch) return 0;
    const uint64_t seconds=(ticks-k_filetime_epoch)/k_ticks_per_second;
    // Behavioral time helper clamps values above 0x793582AFF to -1. Current system
    // time is safely below that boundary; retain a bounded invalid sentinel here.
    return seconds<=UINT64_C(0x793582AFF) ? seconds : UINT64_MAX;
#else
    const auto count=std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if(count<0) return 0;
    const auto seconds=static_cast<uint64_t>(count);
    return seconds<=UINT64_C(0x793582AFF) ? seconds : UINT64_MAX;
#endif
}

bool current_utc_timestamp(char* output,size_t capacity,size_t* written) noexcept {
    if(written) *written=19u;
    if(!output||capacity<20u) return false;
    const uint64_t seconds=current_unix_seconds();
    if(seconds==UINT64_MAX) return false;
    const int64_t days=static_cast<int64_t>(seconds/86400u);
    const unsigned sod=static_cast<unsigned>(seconds%86400u);
    int year=1970; unsigned month=1,day=1;
    civil_from_days(days,&year,&month,&day);
    if(year<0||year>9999) return false;
    const unsigned hour=sod/3600u;
    const unsigned minute=(sod%3600u)/60u;
    const unsigned second=sod%60u;
    put4(output,year); output[4]='-'; put2(output+5,month); output[7]='-'; put2(output+8,day);
    output[10]='T'; put2(output+11,hour); output[13]=':'; put2(output+14,minute); output[16]=':'; put2(output+17,second);
    output[19]='\0';
    return true;
}

} }
