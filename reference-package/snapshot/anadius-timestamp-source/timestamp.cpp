#include "timestamp.hpp"
#include <ctime>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace anadius::timestamp_recovered {
namespace {
bool classic_space(char c) noexcept {
    return c==' ' || (c>='\t' && c<='\r');
}
char ascii_lower(char c) noexcept {
    return c>='A' && c<='Z'?static_cast<char>(c+('a'-'A')):c;
}
struct Number {std::int32_t value;std::uint32_t state;};
Number number_03d8f0(std::string_view raw,std::size_t& pos,
                     unsigned width,std::int32_t minimum,std::int32_t maximum) {
    unsigned used=0;
    while(pos<raw.size() && used<width && classic_space(raw[pos])){++pos;++used;}
    bool negative=false;
    if(pos<raw.size() && used<width && (raw[pos]=='+' || raw[pos]=='-')){
        negative=raw[pos]=='-';++pos; // sign consumes no field width
    }
    while(pos<raw.size() && used<width && raw[pos]=='0'){++pos;++used;}
    // Native prepends a synthetic zero if spaces or leading zeros consumed
    // width. The bounded buffer has at most sign+4 digits; _Stolx cannot
    // overflow here. No libc numeric/locale behavior is needed in this domain.
    bool converted=used>0;
    std::int32_t result=0;
    while(pos<raw.size() && used<width && raw[pos]>='0' && raw[pos]<='9'){
        result=result*10+(raw[pos]-'0');++pos;++used;converted=true;
    }
    if(negative)result=-result;
    std::uint32_t state=pos==raw.size()?eof_bit:0;
    if(!converted || result<minimum || result>maximum)state|=fail_bit;
    return {result,state};
}
}
std::uint32_t ClassicIso::extract_03d310(std::string_view raw,std::string_view pattern,
                                       calendar_recovered::BrokenDown& value) {
    if(pattern!=format)throw std::invalid_argument("ClassicIso requires recovered ISO format");
    for(const unsigned char byte:raw)
        if(byte>=0x80)throw std::invalid_argument("ClassicIso requires ASCII input");
    std::size_t pos=0;
    while(pos<raw.size() && classic_space(raw[pos]))++pos; // 03ab40 sentry
    if(pos==raw.size())return eof_bit|fail_bit;
    struct Field {std::int32_t calendar_recovered::BrokenDown::*destination;
                  std::int32_t minimum,maximum,offset;unsigned width;char separator;};
    constexpr Field fields[]={
        {&calendar_recovered::BrokenDown::year,0,9999,-1900,4,'-'},
        {&calendar_recovered::BrokenDown::month,1,12,-1,2,'-'},
        {&calendar_recovered::BrokenDown::day,1,31,0,2,'T'},
        {&calendar_recovered::BrokenDown::hour,0,23,0,2,':'},
        {&calendar_recovered::BrokenDown::minute,0,59,0,2,':'},
        {&calendar_recovered::BrokenDown::second,0,60,0,2,'\0'}
    };
    for(const auto& field:fields){
        if(pos==raw.size())return eof_bit|fail_bit;
        const auto number=number_03d8f0(raw,pos,field.width,field.minimum,field.maximum);
        if((number.state&fail_bit)==0)value.*field.destination=number.value+field.offset;
        // 041800 exits on EOF as well as failure, before the rest of format.
        if(number.state!=0)return number.state;
        if(field.separator=='\0')return 0;
        if(pos==raw.size())return eof_bit|fail_bit;
        if(ascii_lower(raw[pos])!=ascii_lower(field.separator))return fail_bit;
        ++pos;
    }
    return 0;
}
std::uint32_t HostGetTime::extract_03d310(std::string_view raw,std::string_view pattern,
                                        calendar_recovered::BrokenDown& value) {
    // Preserve embedded NUL bytes and explicit input length (03efd0).
    std::istringstream stream{std::string(raw)};
    std::tm fields{};
    fields.tm_sec=value.second;fields.tm_min=value.minute;fields.tm_hour=value.hour;
    fields.tm_mday=value.day;fields.tm_mon=value.month;fields.tm_year=value.year;
    fields.tm_wday=value.weekday;fields.tm_yday=value.yearday;fields.tm_isdst=value.daylight;
    const std::string owned_pattern(pattern);
    stream>>std::get_time(&fields,owned_pattern.c_str());
    value={fields.tm_sec,fields.tm_min,fields.tm_hour,fields.tm_mday,fields.tm_mon,
           fields.tm_year,fields.tm_wday,fields.tm_yday,fields.tm_isdst};
    const auto state=stream.rdstate();
    std::uint32_t bits=0;
    if((state&std::ios_base::eofbit)!=0)bits|=eof_bit;
    if((state&std::ios_base::failbit)!=0)bits|=fail_bit;
    if((state&std::ios_base::badbit)!=0)bits|=bad_bit;
    return bits;
}
std::int64_t timestamp_to_epoch_041e10(std::string_view raw,Parser& parser) {
    calendar_recovered::BrokenDown fields{};
    const auto state=parser.extract_03d310(raw,format,fields);
    // 041f57 TEST native state,6: EOF alone is allowed; no full-consumption
    // check, timezone conversion, trimming or trailing-character check is added.
    if((state&(fail_bit|bad_bit))!=0)return 0;
    return calendar_recovered::epoch_041ff0(fields);
}
std::int64_t StorageEffects::timestamp_to_epoch_041e10(std::string_view value) {
    return timestamp_recovered::timestamp_to_epoch_041e10(value,parser_);
}
} // namespace anadius::timestamp_recovered
