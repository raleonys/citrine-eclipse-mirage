// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/achievement_utc.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace ar=citrine_eclipse_mirage::achievement_runtime;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>void rejects(F f,const char* why){bool caught=false;try{f();}catch(const std::runtime_error&){caught=true;}require(caught,why);}
bool equal(const ar::BrokenDown& a,const ar::BrokenDown& b){return a.second==b.second && a.minute==b.minute && a.hour==b.hour && a.day==b.day && a.month==b.month && a.year==b.year && a.weekday==b.weekday && a.yearday==b.yearday && a.daylight==b.daylight;}
struct Clock final:ar::Clock {
    std::vector<std::string> trace;
    std::int64_t now=123;ar::BrokenDown fields{56,34,12,1,0,100,4,99,1};
    bool absent=false,throw_now=false,throw_gmtime=false;
    std::int64_t time_05eeb8()override{trace.push_back("time");if(throw_now)throw std::runtime_error("now");return now;}
    std::optional<ar::BrokenDown> gmtime_05fffc(std::int64_t raw)override{trace.push_back("gmtime");require(raw==now,"raw clock value passed to gmtime");if(throw_gmtime)throw std::runtime_error("gmtime");if(absent)return std::nullopt;return fields;}
};
struct Formatter final:ar::Formatter {
    Clock& clock;std::uint64_t* out;std::uint64_t expected;bool throws=false;
    ar::BrokenDown snapshot{};std::string pattern,payload=std::string("bytes\0tail",10);
    Formatter(Clock& c,std::uint64_t* o,std::uint64_t e):clock(c),out(o),expected(e){}
    std::string put_time_03d610(const ar::BrokenDown& fields,std::string_view format)override{
        clock.trace.push_back("format");snapshot=fields;pattern=format;
        if(out)require(*out==expected,"epoch output is updated before formatting");
        clock.fields.second=17;
        require(fields.second==snapshot.second,"copied fields stable after provider storage changes");
        if(throws)throw std::runtime_error("format");return payload;
    }
};
void contracts(){
    for(const auto mode:{0U,1U,2U,128U,255U}){
        Clock clock;std::uint64_t out=999;Formatter formatter(clock,&out,946730096);
        const auto fields=clock.fields;
        const auto result=ar::utc_time_041b60(clock,formatter,&out,static_cast<std::uint8_t>(mode));
        require(result==formatter.payload,"formatter explicit bytes including NUL retained");
        require(equal(formatter.snapshot,fields),"all nine tm fields copied");
        require(formatter.pattern==(mode?"%Y-%m-%d_%H-%M-%S":"%Y-%m-%dT%H:%M:%S"),"any nonzero byte selects filename format");
        require(clock.trace==std::vector<std::string>{"time","gmtime","format"},"time then gmtime then format");
        require(out==946730096 && out!=static_cast<std::uint64_t>(clock.now),"epoch recomputed from returned tm rather than raw time");
    }
    {Clock clock;Formatter f(clock,nullptr,0);require(ar::utc_time_041b60(clock,f)==f.payload,"null optional output still formats");}
    for(bool output:{false,true}){
        Clock clock;clock.absent=true;std::uint64_t out=77;Formatter f(clock,output?&out:nullptr,0);
        require(ar::utc_time_041b60(clock,f,output?&out:nullptr,255).empty(),"gmtime null returns empty string");
        require(out==(output?0U:77U),"failure zeros only supplied optional output");
        require(clock.trace==std::vector<std::string>{"time","gmtime"},"gmtime failure skips formatter");
    }
    {Clock clock;clock.fields={59,59,23,31,11,69,0,0,0};std::uint64_t out=0;Formatter f(clock,&out,0xffffffffffffffffULL);(void)ar::utc_time_041b60(clock,f,&out);require(out==0xffffffffffffffffULL,"negative calendar epoch preserves uint64 bit pattern");}
    {Clock clock;std::uint64_t out=77;Formatter f(clock,&out,946730096);f.payload.clear();require(ar::utc_time_041b60(clock,f,&out).empty() && out==946730096,"empty formatter output does not roll back epoch");}
    for(unsigned stage=0;stage<3;++stage){
        Clock clock;std::uint64_t out=77;Formatter f(clock,&out,946730096);
        clock.throw_now=stage==0;clock.throw_gmtime=stage==1;f.throws=stage==2;
        rejects([&]{(void)ar::utc_time_041b60(clock,f,&out);},"provider and formatter exceptions propagate");
        require(out==(stage==2?946730096U:77U),"exception preserves output ordering, no rollback");
        require(clock.trace.size()==stage+1,"exception stops subsequent effects");
    }
}
void selected_host_compatibility(){
    ar::HostClock clock;ar::HostFormatter formatter;
    const auto epoch=clock.gmtime_05fffc(0),leap=clock.gmtime_05fffc(951827696);
    require(epoch.has_value() && leap.has_value(),"selected host gmtime accepts fixed supported epochs");
    require(epoch->year==70 && epoch->month==0 && epoch->day==1 && epoch->hour==0 && epoch->minute==0 && epoch->second==0,"host epoch fixture fields");
    require(formatter.put_time_03d610(*epoch,"%Y-%m-%dT%H:%M:%S")=="1970-01-01T00:00:00","selected host ISO formatting");
    require(formatter.put_time_03d610(*leap,"%Y-%m-%d_%H-%M-%S")=="2000-02-29_12-34-56","selected host filename formatting");
    // Compare the real adapter against the CRT for valid and rejected time64
    // values. Fixed UTC inputs avoid wall-clock and local timezone dependence.
    for(const std::int64_t value:{
        std::int64_t{-1},std::int64_t{0},std::int64_t{951827696},
        std::int64_t{253402300799LL},std::int64_t{253402300800LL},
        std::numeric_limits<std::int64_t>::max()}) {
        const __time64_t native_seconds=static_cast<__time64_t>(value);
        std::tm direct_fields{};
        const auto native_error=::_gmtime64_s(&direct_fields,&native_seconds);
        const auto actual=clock.gmtime_05fffc(value);
        require(actual.has_value()==(native_error==0),"native time64 success/error parity");
        if(native_error==0) {
            const ar::BrokenDown direct{direct_fields.tm_sec,direct_fields.tm_min,
                direct_fields.tm_hour,direct_fields.tm_mday,direct_fields.tm_mon,
                direct_fields.tm_year,direct_fields.tm_wday,direct_fields.tm_yday,
                direct_fields.tm_isdst};
            require(equal(*actual,direct),"native time64 copies all nine CRT tm fields");
        }
    }
}
}
int main(){try{contracts();selected_host_compatibility();std::cout<<"PASS: "<<checks<<" UTC wrapper checks; fixed providers and explicit host compatibility\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
