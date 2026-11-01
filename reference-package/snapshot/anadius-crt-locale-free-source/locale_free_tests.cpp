#include "locale_free.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

namespace lf = anadius::crt_locale_free_recovered;
namespace {
std::uint64_t checks{};
void require(bool condition) {
    ++checks;
    if (!condition) throw std::runtime_error("locale free assertion " + std::to_string(checks));
}
struct Fixture final : lf::Effects {
    std::map<lf::Address, lf::Address> words;
    std::map<lf::Address, std::uint32_t> refs;
    std::vector<lf::Address> reads, ref_reads, freed;
    std::function<void(lf::Address)> on_free;
    lf::Address read_address(lf::Address a) override { reads.push_back(a); return words[a]; }
    std::uint32_t read_u32(lf::Address a) override { ref_reads.push_back(a); return refs[a]; }
    void free_0668c0(lf::Address p) override { freed.push_back(p); if (on_free) on_free(p); }
};
lf::Defaults defaults() {
    lf::Defaults d{0x9000, 0x9010, 0x9020, {}};
    for (std::size_t i = 0; i != d.lconv_pointer_values.size(); ++i) d.lconv_pointer_values[i] = 0xa000 + i;
    return d;
}
std::vector<lf::Address> fill_time(Fixture& f, lf::Address base) {
    std::vector<lf::Address> expected;
    for (unsigned i = 0; i != 88; ++i) {
        f.words[base + 8 * i] = 0xb000 + i;
        if (i != 43) expected.push_back(0xb000 + i);
    }
    return expected;
}
void test_leaf_matrix() {
    const std::vector<unsigned> monetary{3,4,5,6,7,8,9,13,14,15,16,17,18};
    const std::vector<unsigned> numeric{0,1,2,11,12};
    for (unsigned which = 0; which != 2; ++which) {
        const auto& indexes = which == 0 ? monetary : numeric;
        for (unsigned mask = 0; mask != (1u << indexes.size()); ++mask) {
            Fixture f;
            auto d = defaults();
            std::vector<lf::Address> expected;
            for (unsigned j = 0; j != indexes.size(); ++j) {
                const auto i = indexes[j];
                const auto value = (mask & (1u << j)) ? d.lconv_pointer_values[i] : 0xc000 + i;
                f.words[0x1000 + 8 * i] = value;
                if (!(mask & (1u << j))) expected.push_back(value);
            }
            if (which == 0) lf::free_monetary_070a40(0x1000,d,f);
            else lf::free_numeric_071068(0x1000,d,f);
            require(f.freed == expected);
            require(f.reads.size() == indexes.size());
        }
    }
    Fixture f;
    auto d = defaults();
    lf::free_monetary_070a40(0,d,f);
    lf::free_numeric_071068(0,d,f);
    require(f.reads.empty() && f.freed.empty());
    // A NULL member unequal to its initial pointer is still passed to free.
    lf::free_numeric_071068(0x1000,d,f);
    require(f.freed == std::vector<lf::Address>(5,0));
    // Callback mutation is observed by the next field and next initial-pointer load.
    f = Fixture{};
    f.words[0x1000] = 0x111;
    f.on_free = [&](lf::Address p) {
        if (p == 0x111) { f.words[0x1008]=0x222; d.lconv_pointer_values[1]=0x222; }
    };
    lf::free_numeric_071068(0x1000,d,f);
    require(f.freed == std::vector<lf::Address>({0x111,0,0,0}));
    // Defaults may alias each other: comparisons are by the matching slot, not ownership sets.
    f = Fixture{};
    d.lconv_pointer_values.fill(0x555);
    f.words[0x1000]=0x555; f.words[0x1008]=0x555;
    lf::free_numeric_071068(0x1000,d,f);
    require(f.freed == std::vector<lf::Address>({0,0,0}));
}
void test_arrays_and_time() {
    Fixture f;
    lf::free_pointer_array_0713bc(0,0,f);
    lf::free_pointer_array_0713bc(0x1000,UINT64_C(1)<<61,f);
    require(f.reads.empty() && f.freed.empty());
    f.words[UINT64_MAX-7]=0x11; f.words[0]=0x22;
    lf::free_pointer_array_0713bc(UINT64_MAX-7,2,f);
    require(f.reads == std::vector<lf::Address>({UINT64_MAX-7,0}));
    require(f.freed == std::vector<lf::Address>({0x11,0x22}));
    f = Fixture{};
    f.words[0x1000]=1;
    lf::free_pointer_array_0713bc(0x1000,(UINT64_C(1)<<61)+1,f);
    require(f.freed == std::vector<lf::Address>({1}));
    f = Fixture{};
    f.words[0x1000]=1; f.words[0x1008]=2;
    f.on_free=[&](lf::Address p) { if(p==1) f.words[0x1008]=3; };
    lf::free_pointer_array_0713bc(0x1000,2,f);
    require(f.freed == std::vector<lf::Address>({1,3}));
    f = Fixture{};
    lf::free_time_071730(0,f);
    require(f.reads.empty() && f.freed.empty());
    auto expected=fill_time(f,0x2000);
    lf::free_time_071730(0x2000,f);
    require(expected.size()==87 && f.freed==expected);
    require(std::find(f.reads.begin(),f.reads.end(),0x2000+0x158)==f.reads.end());
    auto d=defaults();
    for (auto pointer : {lf::Address{0},d.initial_time_08ad20,lf::Address{0x2000}}) {
        for (auto ref : {0u,1u,0xffffffffu}) {
            f=Fixture{};
            expected=fill_time(f,0x2000);
            f.refs[0x215c]=ref;
            lf::free_time_if_unreferenced_070860(pointer,d,f);
            if(pointer==0x2000 && ref==0) { expected.push_back(0x2000); require(f.freed==expected); }
            else require(f.freed.empty());
            require(f.ref_reads.size()==(pointer==0x2000 ? 1 : 0));
        }
    }
}
void test_locale_branches() {
    auto d=defaults();
    // Guard combinations cover NULL/static/dynamic lconv, NULL/ref zero/nonzero,
    // and both monetary/numeric reference choices.
    for(unsigned l=0;l!=3;++l) for(unsigned common=0;common!=3;++common)
    for(unsigned mon=0;mon!=3;++mon) for(unsigned num=0;num!=3;++num) {
        Fixture f;
        std::vector<lf::Address> expected;
        const auto lc = l==0 ? 0 : l==1 ? d.lconv_0a1700 : 0x2000;
        f.words[0x10f8]=lc;
        f.words[0x10e0]=common==0 ? 0 : 0x3000;
        f.words[0x10f0]=mon==0 ? 0 : 0x3100;
        f.words[0x10e8]=num==0 ? 0 : 0x3200;
        f.refs[0x3000]=common==2; f.refs[0x3100]=mon==2; f.refs[0x3200]=num==2;
        for(std::size_t i=0;i!=19;++i) f.words[0x2000+8*i]=d.lconv_pointer_values[i];
        if(l==2 && common==1) {
            if(mon==1) expected.push_back(0x3100);
            if(num==1) expected.push_back(0x3200);
            expected.push_back(0x3000); expected.push_back(0x2000);
        }
        expected.push_back(0x1000);
        lf::free_locale_0706c0(0x1000,d,f);
        require(f.freed==expected);
    }
    for(auto ref : {0u,1u,0xffffffffu}) {
        Fixture f;
        f.words[0x1100]=0x3000; f.refs[0x3000]=ref;
        f.words[0x1108]=0; f.words[0x1110]=0x80; f.words[0x1118]=0x1000;
        lf::free_locale_0706c0(0x1000,d,f);
        const std::vector<lf::Address> expected=ref==0 ?
            std::vector<lf::Address>{lf::Address{0}-0xfe,0,0xf80,0x3000,0x1000} :
            std::vector<lf::Address>{0x1000};
        require(f.freed==expected);
    }
    // Each of six category records has independent static-name/name-ref and
    // nonempty alternate-name/alternate-ref guards.
    for(unsigned pattern=0;pattern!=729;++pattern) {
        unsigned v=pattern;
        Fixture f;
        std::vector<lf::Address> expected;
        for(unsigned i=0;i!=6;++i) {
            const unsigned mode=v%3; v/=3;
            const auto record=0x1038+0x20*i;
            f.words[record-0x10]=mode==0 ? d.category_name_0a1948 : 0x777;
            f.words[record]=0x4000+16*i;
            f.refs[0x4000+16*i]=mode==2;
            f.words[0x1128+8*i]=0x5000+i;
            f.words[record-0x18]=mode==0 ? 0 : 0x888;
            f.words[record-8]=0x6000+16*i;
            f.refs[0x6000+16*i]=mode==2;
            if(mode==1) {
                expected.push_back(0x4000+16*i); expected.push_back(0x5000+i);
                expected.push_back(0x6000+16*i);
            }
        }
        expected.push_back(0x1000);
        lf::free_locale_0706c0(0x1000,d,f);
        require(f.freed==expected);
    }
}
void test_mutations_and_null_locale() {
    Fixture f;
    auto d=defaults();
    f.words[0x10f8]=0x2000; f.words[0x10e0]=0x3000;
    f.words[0x10f0]=0x3100;
    f.on_free=[&](lf::Address p) {
        if(p==0x3100) f.words[0x10f8]=0; // next helper must see NULL
        if(p==0x3000) f.words[0x10f8]=0x2222; // final free reload
    };
    lf::free_locale_0706c0(0x1000,d,f);
    require(f.freed==std::vector<lf::Address>({0x3100,0x3000,0x2222,0x1000}));
    f=Fixture{};
    f.words[0x1100]=0x3000; f.words[0x1108]=0x1fe;
    f.on_free=[&](lf::Address p) {
        if(p==0x100) {f.words[0x1110]=0x280;f.words[0x1100]=0x3333;}
        if(p==0x200) f.words[0x1118]=0x380;
    };
    lf::free_locale_0706c0(0x1000,d,f);
    require(f.freed==std::vector<lf::Address>({0x100,0x200,0x300,0x3333,0x1000}));
    f=Fixture{};
    f.words[0x1028]=0x777; f.words[0x1038]=0x3000;
    f.on_free=[&](lf::Address p) {
        if(p==0x3000) {f.words[0x1128]=0x4444;f.words[0x1020]=0x777;f.words[0x1030]=0x5555;}
    };
    lf::free_locale_0706c0(0x1000,d,f);
    require(f.freed==std::vector<lf::Address>({0x3000,0x4444,0x5555,0x1000}));
    // free_locale has no NULL guard: provider-address zero is still read and
    // passed through. The fixture supplies zero storage without native access.
    f=Fixture{};
    lf::free_locale_0706c0(0,d,f);
    require(!f.reads.empty() && f.reads.front()==0xf8);
    require(f.freed==std::vector<lf::Address>{0});
    // Category guards with NULL reference values do not touch the reference.
    f=Fixture{};
    f.words[0x1028]=0x777; f.words[0x1020]=0x777;
    lf::free_locale_0706c0(0x1000,d,f);
    require(f.ref_reads.empty() && f.freed==std::vector<lf::Address>{0x1000});
}
}
int main() {
    try {
        test_leaf_matrix(); test_arrays_and_time(); test_locale_branches(); test_mutations_and_null_locale();
        std::cout<<"locale_free: "<<checks<<" checks PASS\n";
    } catch(const std::exception& error) { std::cerr<<error.what()<<'\n';return 1; }
}
