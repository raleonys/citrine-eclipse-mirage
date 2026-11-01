#include "api.hpp"
#include <array>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace api = anadius::crt_conversion_api;
namespace rt = anadius::crt_runtime_recovered;
namespace {
std::uint64_t checks = 0;
void check(bool good) {
    ++checks;
    if (!good) { std::cerr << "conversion API check failed: " << checks << '\n'; std::exit(1); }
}
struct Call {
    std::uint32_t cp{}, flags{};
    api::Address source{}, destination{}, default_char{}, used_default{};
    std::int32_t count{}, capacity{};
};
struct Conversion final : api::Effects {
    Call call;
    std::vector<int> events;
    api::Address written_address{};
    std::int32_t written_value{77};
    std::int32_t result{};
    std::int32_t multi_byte_to_wide(std::uint32_t cp, std::uint32_t flags,
        api::Address source, std::int32_t count, api::Address destination,
        std::int32_t capacity) override {
        events.push_back(1);
        call = {cp,flags,source,destination,0,0,count,capacity}; return result;
    }
    std::int32_t wide_char_to_multi_byte(std::uint32_t cp, std::uint32_t flags,
        api::Address source, std::int32_t count, api::Address destination,
        std::int32_t capacity, api::Address default_char, api::Address used) override {
        events.push_back(2);
        call = {cp,flags,source,destination,default_char,used,count,capacity}; return result;
    }
    void write_bool32(api::Address address, std::int32_t value) override {
        events.push_back(3); written_address = address; written_value = value;
    }
};
// Independent explicit classification oracle, including the three holes in
// the 50220..50229 family. The production source uses switch plus range.
std::array<bool,65536> zero_table() {
    std::array<bool,65536> result{};
    for (auto cp : {42,50220,50221,50222,50225,50227,50229,
                   57002,57003,57004,57005,57006,57007,57008,57009,57010,57011,65000})
        result[cp] = true;
    return result;
}
void conversion_case(std::uint32_t cp, std::uint32_t flags,
                     api::Address default_char, api::Address used) {
    static const auto zeros = zero_table();
    const auto is_zero = cp < zeros.size() && zeros[cp];
    const auto unicode = cp == 65000 || cp == 65001;
    Conversion effect;
    effect.result = std::bit_cast<std::int32_t>(cp * 0x10001U ^ flags ^ 0x80ff00a5U);
    constexpr api::Address source=0xfedcba9876543210ULL, destination=0x123456789abcdef0ULL;
    const auto count = std::bit_cast<std::int32_t>(flags ^ 0x80000000U);
    const auto capacity = std::bit_cast<std::int32_t>(cp ^ 0xffffffffU);
    auto result = api::multi_byte_to_wide_06ccc8(cp,flags,source,count,destination,capacity,effect);
    auto expected_flags = flags;
    if (is_zero) expected_flags=0;
    if (cp==54936 || cp==65001) expected_flags=flags & 8;
    check(result==effect.result);
    check(effect.events==std::vector<int>{1});
    check(effect.call.cp==cp && effect.call.flags==expected_flags);
    check(effect.call.source==source && effect.call.destination==destination);
    check(effect.call.count==count && effect.call.capacity==capacity);
    effect.events.clear();
    result=api::wide_char_to_multi_byte_06cd58(cp,flags,source,count,destination,capacity,
                                            default_char,used,effect);
    expected_flags = is_zero || cp==54936 || cp==65001 ? 0 : flags & ~0x80U;
    check(result==effect.result);
    check(effect.events==(unicode && used!=0 ? std::vector<int>{3,2}:std::vector<int>{2}));
    check(effect.call.cp==cp && effect.call.flags==expected_flags);
    check(effect.call.source==source && effect.call.destination==destination);
    check(effect.call.count==count && effect.call.capacity==capacity);
    check(effect.call.default_char==(unicode?0:default_char));
    check(effect.call.used_default==(unicode?0:used));
    if (unicode && used!=0) check(effect.written_address==used && effect.written_value==0);
}
struct Resolver final : rt::Effects {
    std::vector<std::string> events;
    rt::Module module{0xabc}; rt::Address address{0x1234567800000100ULL};
    std::uint32_t error{5};
    rt::Module load_library_ex_w(std::u16string_view name,rt::Module file,std::uint32_t flags) override {
        check(name==u"kernel32" && file==0);
        check(flags==0x800 || flags==0);
        events.push_back(flags==0?"load0":"load800"); return module;
    }
    std::uint32_t last_error() override {events.push_back("error");return error;}
    bool free_library(rt::Module) override {check(false);return false;}
    rt::Address get_proc_address(rt::Module handle,std::string_view name) override {
        check(handle==module && name=="AreFileApisANSI");events.push_back("get");return address;
    }
    std::uint32_t process_parameters_flags() override {check(false);return 0;}
    std::int32_t call_ro_initialize(rt::Address,std::uint32_t) override {check(false);return 0;}
    void call_ro_uninitialize(rt::Address) override {check(false);}
    std::uint32_t call_thread_initialization_policy(rt::Address,std::uintptr_t,std::int32_t*) override {
        check(false);return 0;
    }
};
struct File final : api::FileApiEffects {
    api::Address expected{0x1234567800000100ULL}; std::uint32_t result{}, calls{};
    std::uint32_t call_are_file_apis_ansi(api::Address address) override {
        check(address==expected);++calls;return result;
    }
};
void resolver_cases() {
    for (auto cookie : {0ULL,1ULL,63ULL,0x123456789abcdef0ULL}) {
        rt::State state(cookie); Resolver resolver; File file;
        for (auto value : {0U,1U,0x100U,0x80000000U,0xffffffffU}) {
            file.result=value;
            check(api::file_api_ansi_066c1c(state,resolver,file)==value);
        }
        check(file.calls==5);
        check(resolver.events==std::vector<std::string>{"load800","get"});
        check(state.modules[16].load()==resolver.module);
        check(rt::decode_pointer(state.procedures[0].load(),cookie)==resolver.address);
        for(std::size_t i=1;i<state.procedures.size();++i)
            check(rt::decode_pointer(state.procedures[i].load(),cookie)==0);
        for(std::size_t i=0;i<state.modules.size();++i)
            if(i!=16)check(state.modules[i].load()==0);
    }
    { // A module without the export permanently caches procedure-missing.
        rt::State state(0xabba); Resolver resolver; File file;resolver.address=0;
        check(api::file_api_ansi_066c1c(state,resolver,file)==1);
        check(api::file_api_ansi_066c1c(state,resolver,file)==1);
        check(resolver.events==std::vector<std::string>{"load800","get"});
        check(file.calls==0 && rt::decode_pointer(state.procedures[0].load(),state.cookie)==rt::missing);
    }
    { // A failed load has only the one recovered candidate.
        rt::State state(3); Resolver resolver; File file;resolver.module=0;
        check(api::file_api_ansi_066c1c(state,resolver,file)==1);
        check(api::file_api_ansi_066c1c(state,resolver,file)==1);
        check(resolver.events==std::vector<std::string>{"load800","error"});
        check(state.modules[16].load()==rt::missing && file.calls==0);
    }
    { // Existing resolver's native-name error87 fallback is retained.
        rt::State state(7); Resolver resolver; File file;resolver.module=0;resolver.error=87;
        check(api::file_api_ansi_066c1c(state,resolver,file)==1);
        check(resolver.events==std::vector<std::string>{"load800","error","load0"});
        check(file.calls==0);
    }
    { // Already loaded candidate and pre-encoded procedure both bypass loading.
        rt::State state(13); Resolver resolver; File file;state.modules[16].store(resolver.module);
        check(api::file_api_ansi_066c1c(state,resolver,file)==0);
        check(resolver.events==std::vector<std::string>{"get"});
        rt::State pre_cached(31);pre_cached.procedures[0].store(rt::encode_pointer(file.expected,31));
        resolver.events.clear();file.result=0x100;
        check(api::file_api_ansi_066c1c(pre_cached,resolver,file)==0x100);
        check(resolver.events.empty());
    }
}
}
int main() {
    for(std::uint32_t cp=0;cp<65536;++cp)
        for(auto flags : {0U,8U,0x80U,0x100U,0xffffffffU})
            conversion_case(cp,flags,0x3333444455556666ULL,0x777788889999aaaaULL);
    for(auto cp : {0U,42U,50223U,54936U,65000U,65001U,65002U,65536U,0xffff0000U,0xffffffffU})
        for(auto flags : {1U,2U,4U,0x7fU,0x88U,0xffffff7fU})
            for(auto default_char : {0ULL,0x8000000000000000ULL})
                for(auto used : {0ULL,0xffff000012345678ULL})
                    conversion_case(cp,flags,default_char,used);
    resolver_cases();
    std::cout<<"conversion API: "<<checks<<" checks PASS\n";
}
