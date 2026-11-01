#include "locale_names.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace ln = anadius::crt_locale_names_recovered;
namespace {
std::uint64_t checks = 0;
void check(bool v) { ++checks; if (!v) throw std::runtime_error("locale names assertion " + std::to_string(checks)); }
struct Memory final : ln::Effects {
    std::map<ln::Address,std::uint16_t> words;
    std::vector<ln::Address> reads;
    ln::Address put(std::u16string_view s, ln::Address start=0x1000) {
        words.clear(); reads.clear();
        auto p=start;
        for (const auto c:s) { words[p]=static_cast<std::uint16_t>(c);p+=2; }
        words[p]=0;
        return start;
    }
    std::uint16_t read_u16(ln::Address p) override {
        reads.push_back(p);
        const auto found=words.find(p);
        if (found==words.end()) throw std::runtime_error("unmapped read");
        return found->second;
    }
};
std::uint32_t fold(std::uint32_t c) { return c>=65 && c<=90 ? c+32 : c; }
std::uint32_t expected(std::u16string_view s) {
    // Independently enumerate actual table instead of repeating binary search.
    // Two original rows cannot be reached because the shipped sequence is not
    // sorted under the routine's ASCII comparator. Preserve those outcomes.
    if (s==u"kok" || s==u"kok-in") return 0;
    for (const auto& row:ln::shipped_name_rows())
        if (row.name==s) return ln::shipped_lcid_values()[static_cast<std::size_t>(row.lcid_index)];
    return 0;
}
void all_names() {
    Memory m;
    check(ln::locale_name_to_lcid_07310c(0,m)==0);
    check(m.reads.empty());
    std::set<int> indices;
    unsigned misses=0,inversions=0;
    const auto rows=ln::shipped_name_rows();
    check(rows.size()==228);
    for (std::size_t i=0;i<rows.size();++i) {
        const auto& row=rows[i];
        check(row.lcid_index>=0 && row.lcid_index<228);
        indices.insert(row.lcid_index);
        check(row.original_name_address>=0x180000000ULL);
        const auto value=ln::locale_name_to_lcid_07310c(m.put(row.name),m);
        check(value==expected(row.name));
        if (value==0) ++misses;
        std::u16string mixed(row.name);
        for (std::size_t c=0;c<mixed.size();++c)
            if ((c%2)==0 && mixed[c]>=u'a'&&mixed[c]<=u'z') mixed[c]-=32;
        check(ln::locale_name_to_lcid_07310c(m.put(mixed),m)==value);
        std::u16string invalid(row.name);invalid+=u".invalid";
        check(ln::locale_name_to_lcid_07310c(m.put(invalid),m)==0);
        for (std::size_t length=0;length<row.name.size();++length) {
            const auto prefix=row.name.substr(0,length);
            check(ln::locale_name_to_lcid_07310c(m.put(prefix),m)==expected(prefix));
        }
        if (i && rows[i-1].name>row.name) ++inversions;
    }
    check(indices.size()==228);check(misses==2);check(inversions==1);
    check(rows[128].name==u"kok" && rows[129].name==u"kok-in" && rows[130].name==u"ko-kr");
    check(ln::locale_name_to_lcid_07310c(m.put(u""),m)==0x7f);
    check(ln::locale_name_to_lcid_07310c(m.put(u"en-US"),m)==0x409);
    check(ln::locale_name_to_lcid_07310c(m.put(u"tr-TR"),m)==0x41f);
    check(ln::locale_name_to_lcid_07310c(m.put(u"ko-KR"),m)==0x412);
    check(ln::locale_name_to_lcid_07310c(m.put(u"ja-JP"),m)==0x411);
    check(ln::locale_name_to_lcid_07310c(m.put(u"KOK-IN"),m)==0);
    for (const auto s:{u"EN_us",u"en-US-",u" en-us",u"en-us ",u"\u00e9n-us",u"\u0130",u"\uff45n-us",u"zz-ZZ"})
        check(ln::locale_name_to_lcid_07310c(m.put(s),m)==0);
}
void comparator() {
    Memory m;
    check(ln::ascii_wcsnicmp_0736d4(0,{},0,m)==0);check(m.reads.empty());
    // Full unsigned UTF16 domain versus NUL, both ASCII endpoints, and high
    // Unicode values. Independent integer oracle retains exact differences.
    constexpr std::array<std::uint16_t,9> rights{0,0x40,0x41,0x5a,0x5b,0x61,0xc4,0x8000,0xffff};
    for (std::uint32_t left=0;left<=0xffff;++left) {
        const std::u16string ls(1,static_cast<char16_t>(left));
        for (const auto right:rights) {
            const std::u16string rs(1,static_cast<char16_t>(right));
            const auto a=m.put(ls);
            const auto result=ln::ascii_wcsnicmp_0736d4(a,rs,1,m);
            check(result==static_cast<std::int32_t>(fold(left))-static_cast<std::int32_t>(fold(right)));
            check(m.reads==std::vector<ln::Address>{a});
        }
    }
    check(ln::ascii_wcsnicmp_0736d4(m.put(u"ABcX"),u"aBCy",3,m)==0);
    check(m.reads==std::vector<ln::Address>({0x1000,0x1002,0x1004}));
    check(ln::ascii_wcsnicmp_0736d4(m.put(u"ABcX"),u"aBCy",4,m)==-1);
    check(ln::ascii_wcsnicmp_0736d4(m.put(u"a"),u"AB",10,m)==-98);
    check(m.reads.size()==2);
    check(ln::ascii_wcsnicmp_0736d4(m.put(u"ab"),u"A",10,m)==98);
    check(ln::ascii_wcsnicmp_0736d4(m.put(u"a"),u"A",std::numeric_limits<std::uint64_t>::max(),m)==0);
    check(m.reads.size()==2);
    check(ln::ascii_wcsnicmp_0736d4(m.put(u"\u00c4"),u"\u00e4",1,m)==-32);
    // The source performs modular 64-bit pointer increments.
    const auto wrap=m.put(u"AZ",std::numeric_limits<ln::Address>::max()-1);
    check(ln::ascii_wcsnicmp_0736d4(wrap,u"az",3,m)==0);
    check(m.reads==std::vector<ln::Address>({wrap,0,2}));
    std::u16string eightyfive(85,u'a');
    auto other=eightyfive;other+=u'b';
    check(ln::ascii_wcsnicmp_0736d4(m.put(other),eightyfive,85,m)==0);
    check(m.reads.size()==85);
}
}
int main() {
    try { all_names();comparator();std::cout<<"locale names PASS "<<checks<<" checks\n";return 0; }
    catch (const std::exception& e) { std::cerr<<e.what()<<'\n';return 1; }
}
