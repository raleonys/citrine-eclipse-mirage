#include "initial_locale_data.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>
#include <source_location>
#include <vector>

using namespace anadius::crt::initial_locale_data;
namespace {
void require(bool value, const std::source_location location=std::source_location::current()) { if (!value) { std::cerr << "Failed assertion at line " << location.line() << '\n'; std::abort(); } }
std::uint64_t read(const std::vector<std::uint8_t>& b, std::size_t p, unsigned n = 8) {
    require(p + n <= b.size());
    std::uint64_t v = 0;
    for (unsigned i = 0; i < n; ++i) v |= std::uint64_t(b[p+i]) << (8*i);
    return v;
}
}
int main() {
    const auto rr = regions(); const auto ee = relocations();
    require(rr.size() == 96 && ee.size() == 116);
    require(rr[Locale].bytes.size() == 0x160 && rr[Lconv].bytes.size() == 0x98);
    require(rr[Time].bytes.size() == 0x2c0 && rr[Category].bytes.size() == 4);
    std::vector<std::vector<std::uint8_t>> image;
    std::vector<std::uint64_t> synthetic;
    std::size_t total = 0;
    for (std::size_t i = 0; i < rr.size(); ++i) {
        require(rr[i].bytes.size() < 0x1000);
        image.emplace_back(rr[i].bytes.begin(), rr[i].bytes.end());
        synthetic.push_back(0x10000000ULL + i*0x1000);
        total += image.back().size();
    }
    require(total == 3546);
    std::set<std::pair<std::uint32_t,std::uint32_t>> slots;
    for (const auto& e : ee) {
        require(e.source_region < rr.size() && e.target_region < rr.size());
        require(e.source_offset + 8 <= image[e.source_region].size());
        require(e.target_offset < image[e.target_region].size());
        require(slots.emplace(e.source_region,e.source_offset).second);
        require(read(image[e.source_region],e.source_offset) == 0);
        auto v = synthetic[e.target_region] + e.target_offset;
        for (unsigned k = 0; k < 8; ++k) image[e.source_region][e.source_offset+k] = std::uint8_t(v >> (8*k));
    }
    require(read(image[Locale],0x158) == synthetic[Locale]);
    require(read(image[Locale],0xf8) == synthetic[Lconv]);
    require(read(image[Locale],0x120) == synthetic[Time]);
    require(read(image[Locale],0x28) == 0);
    for (unsigned c = 1; c < 6; ++c) require(read(image[Locale],0x28+c*0x20) == synthetic[Category]);
    require(read(image[Locale],0x10,4) == 1 && read(image[Locale],8,4) == 1);
    require(read(image[Locale],0x108) == 0 && read(image[Locale],0x100) == 0);
    require(read(image[Time],0x158,4) == 1 && read(image[Time],0x15c,4) == 0);
    require(read(image[Lconv],0x50) == 0x7f7f7f7f7f7f7f7fULL);
    // May is both the short and long month label, with original pointer identity retained.
    require(read(image[Time],18*8) == read(image[Time],30*8));
    require(read(image[Time],0x160+18*8) == read(image[Time],0x160+30*8));
    require(read(image[Locale],0) == synthetic[4]+0x100);
    require(read(image[Locale],0x110) == synthetic[5]+0x80);
    require(read(image[Locale],0x118) == synthetic[6]+0x80);
    require(image[5][0x80+'A'] == 'a' && image[6][0x80+'a'] == 'A');
    require(read(image[4],0x100+'A'*2,2) == 0x81);
    for (std::size_t r = 0; r < rr.size(); ++r) {
        for (std::size_t p = 0; p < rr[r].bytes.size(); ++p) {
            const bool pointer_byte = std::any_of(ee.begin(),ee.end(),[&](const auto& e) {
                return e.source_region == r && p >= e.source_offset && p < e.source_offset+8;
            });
            if (!pointer_byte) require(image[r][p] == rr[r].bytes[p]);
        }
    }
    std::cout << "PASS: 96 regions, 116 bounded unique relocation slots, 3546 bytes; synthetic rebase, self/category identities, scalar/refcount/May alias/table checks\n";
}
