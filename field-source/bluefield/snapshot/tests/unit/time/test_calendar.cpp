#include "src/time/calendar.hpp"
#include <array>
#include <bit>
#include <chrono>
#include <iostream>
#include <limits>
#include <stdexcept>

using preloader_l::calendar::BrokenDown;
using preloader_l::calendar::epoch_041ff0;
namespace {
unsigned checks=0;
void require(bool ok,const char* why){++checks;if(!ok)throw std::runtime_error(why);}
std::uint32_t bits(std::int32_t value){return std::bit_cast<std::uint32_t>(value);}
std::int32_t signed32(std::uint32_t value){return std::bit_cast<std::int32_t>(value);}
std::uint32_t sar(std::uint32_t value,unsigned amount) {
    if(!amount)return value;
    const auto fill=(value&0x80000000U)?(~std::uint32_t{0}<<(32-amount)):0U;
    return (value>>amount)|fill;
}
std::uint32_t high_product(std::uint32_t left,std::uint32_t right) {
    const auto product=static_cast<std::int64_t>(signed32(left))*signed32(right);
    return static_cast<std::uint32_t>(std::bit_cast<std::uint64_t>(product)>>32);
}
std::uint32_t magic_quotient(std::uint32_t value,std::uint32_t magic,unsigned shift) {
    auto result=sar(high_product(magic,value),shift);
    return result+(result>>31);
}
// Independent uint32 register-level oracle reproducing the actual instruction
// schedule and signed multiply-high constants, not source division expressions.
std::int64_t instruction_oracle(const BrokenDown& t) {
    auto r10=bits(t.year)+0x76cU;
    auto r8=bits(t.month);
    if(signed32(r8)>11) {
        const auto edx=magic_quotient(r8,0x2aaaaaabU,1);
        r10+=edx;r8+=edx*0xfffffff4U;
    }else if(signed32(r8)<0) {
        const auto edx=magic_quotient(11U-r8,0x2aaaaaabU,1);
        r10-=edx;r8+=edx*12U;
    }
    const auto ecx=r8+1U;
    r10-=signed32(ecx)<=2?1U:0U;
    r8=magic_quotient(r10,0x51eb851fU,7);
    r10-=r8*400U;
    const auto month_index=ecx+(signed32(ecx)>2?0xfffffffdU:9U);
    auto r9=magic_quotient(month_index*153U+2U,0x66666667U,1);
    r9+=r8*146097U;
    r9+=r10*365U;
    r9-=magic_quotient(r10,0x51eb851fU,5);
    r9+=sar(r10+((r10&0x80000000U)?3U:0U),2);
    const auto eax=bits(t.day)+0xfff50593U+r9;
    const auto signed_days=static_cast<std::int64_t>(signed32(eax));
    return ((signed_days*24+t.hour)*60+t.minute)*60+t.second;
}
BrokenDown date(int year,int month,int day,int hour=0,int minute=0,int second=0) {
    return {second,minute,hour,day,month-1,year-1900,0,0,0};
}
void known_dates() {
    require(epoch_041ff0(date(1970,1,1))==0,"Unix epoch");
    require(epoch_041ff0(date(1969,12,31,23,59,59))==-1,"before epoch");
    require(epoch_041ff0(date(2000,1,1))==946684800,"year2000");
    require(epoch_041ff0(date(2000,2,29))==951782400,"leap2000");
    require(epoch_041ff0(date(1900,3,1))==-2203891200LL,"century1900 not leap");
    require(epoch_041ff0(date(2038,1,19,3,14,8))==2147483648LL,"beyond signed32 epoch");
    require(epoch_041ff0(date(1970,1,0))==-86400,"day0 is arithmetic not rejected");
    require(epoch_041ff0(date(1970,1,1,25,-1,61))==90001,"out-of-range clock fields are arithmetic");
    auto ignored=date(2000,1,1);ignored.weekday=-100;ignored.yearday=123456;ignored.daylight=1;
    require(epoch_041ff0(ignored)==946684800,"weekday/yearday/DST fields ignored");
    auto normalized=date(2000,1,1);normalized.month=-1;
    require(epoch_041ff0(normalized)==epoch_041ff0(date(1999,12,1)),"negative month normalized");
    normalized.month=24;
    require(epoch_041ff0(normalized)==epoch_041ff0(date(2002,1,1)),"positive month normalized");
    normalized.month=-25;
    require(epoch_041ff0(normalized)==epoch_041ff0(date(1997,12,1)),"multiple negative months normalized");
    // Native era division truncates. It deliberately differs from floor-era
    // proleptic Gregorian arithmetic for some negative years.
    require(epoch_041ff0(date(-1,3,1))==-62193571200LL,"negative year native truncation golden");
}
void civil_oracle() {
    using namespace std::chrono;
    for(const int year_value:std::array<int,25>{1,4,100,400,800,1200,1500,1582,1600,1700,1800,1899,1900,1904,1969,1970,1999,2000,2001,2024,2038,2099,2100,2400,9999}) {
        for(unsigned month_value=1;month_value<=12;++month_value) {
            const auto last_day=static_cast<unsigned>((year{year_value}/month{month_value}/std::chrono::last).day());
            for(const unsigned day_value:std::array<unsigned,3>{1,13,last_day}) {
                const auto expected=duration_cast<seconds>(sys_days{year{year_value}/month{month_value}/day{day_value}}.time_since_epoch()).count();
                require(epoch_041ff0(date(year_value,static_cast<int>(month_value),static_cast<int>(day_value)))==expected,"independent C++chrono civil date");
            }
        }
    }
}
void full_width_instruction_oracle() {
    const auto min=std::numeric_limits<std::int32_t>::min(),max=std::numeric_limits<std::int32_t>::max();
    constexpr std::array<std::int32_t,13> boundary={std::numeric_limits<std::int32_t>::min(),std::numeric_limits<std::int32_t>::min()+11,-2401,-1901,-400,-1,0,11,12,399,400,std::numeric_limits<std::int32_t>::max()-11,std::numeric_limits<std::int32_t>::max()};
    for(auto year:boundary)for(auto month:boundary)for(auto day:std::array<std::int32_t,5>{min,-1,0,1,max}) {
        BrokenDown t{max,min,max,day,month,year,min,max,-1};
        require(epoch_041ff0(t)==instruction_oracle(t),"full-width boundary instruction equivalence");
    }
    std::uint32_t random=0x619041ffU;
    const auto next=[&]() {random=random*1664525U+1013904223U;return signed32(random);};
    for(unsigned i=0;i<30000;++i) {
        BrokenDown t{next(),next(),next(),next(),next(),next(),next(),next(),next()};
        require(epoch_041ff0(t)==instruction_oracle(t),"random full32-bit instruction equivalence");
    }
    // Explicit probes ensure wrap sites are not silently promoted to64-bit.
    BrokenDown overflow{0,0,0,1,min,max,0,0,0};
    require(epoch_041ff0(overflow)==instruction_oracle(overflow),"11-minus-INT_MIN wraps before month division");
    overflow={0,0,0,max,0,max,0,0,0};
    require(epoch_041ff0(overflow)==instruction_oracle(overflow),"year/day wraps before final sign extension");
}
}
int main() {
    try{known_dates();civil_oracle();full_width_instruction_oracle();std::cout<<"PASS: "<<checks<<" calendar arithmetic checks; no original execution or OS time calls\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
