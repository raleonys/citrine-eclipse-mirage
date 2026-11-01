#include "nls_dispatch.hpp"
#include <bit>
#include <iostream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

namespace n = anadius::crt_nls_dispatch_recovered;
namespace {
std::uint64_t checks{};
void check(bool yes) {
    ++checks;
    if (!yes) throw std::runtime_error("NLS dispatch fixture mismatch");
}
using MapArgs = std::tuple<n::Address, std::uint32_t, n::Address, std::int32_t,
                          n::Address, std::int32_t, n::Address, n::Address, n::Address>;
using WideArgs = std::tuple<std::uint32_t, std::uint32_t, n::Address,
                           std::int32_t, n::Address, std::int32_t>;
struct Fixture final : n::Effects {
    n::Address map_function{0x1020304050607080}, locale_function{0x8877665544332211};
    std::int32_t map_result{}, wide_result{};
    std::uint32_t locale_result{}, fallback_result{};
    MapArgs map_args{};
    WideArgs wide_args{};
    n::Address locale_arg{};
    std::uint32_t locale_flags{};
    std::vector<std::string> events;
    bool throw_native{};
    n::Address resolve_0669b0(std::uint32_t procedure, std::string_view name,
                              std::span<const std::uint32_t> modules) override {
        check(modules.size() == 2);
        check(modules[0] == 3 && modules[1] == 16);
        if (procedure == 0x11) {
            check(name == "LCMapStringEx");
            events.emplace_back("resolve-map");
            return map_function;
        }
        check(procedure == 0x13);
        check(name == "LocaleNameToLCID");
        events.emplace_back("resolve-locale");
        return locale_function;
    }
    std::int32_t call_lc_map_string_ex(n::Address function, n::Address locale,
        std::uint32_t flags, n::Address source, std::int32_t source_count,
        n::Address destination, std::int32_t destination_count, n::Address version,
        n::Address reserved, n::Address sort_handle) override {
        check(function == map_function);
        map_args = {locale, flags, source, source_count, destination,
                    destination_count, version, reserved, sort_handle};
        events.emplace_back("map-ex");
        if (throw_native) throw std::logic_error("synthetic native endpoint");
        return map_result;
    }
    std::int32_t call_lc_map_string_w(std::uint32_t locale, std::uint32_t flags,
        n::Address source, std::int32_t source_count, n::Address destination,
        std::int32_t destination_count) override {
        wide_args = {locale, flags, source, source_count, destination, destination_count};
        events.emplace_back("map-w");
        return wide_result;
    }
    std::uint32_t call_locale_name_to_lcid(n::Address function, n::Address locale,
                                         std::uint32_t flags) override {
        check(function == locale_function);
        locale_arg = locale;
        locale_flags = flags;
        events.emplace_back("locale-native");
        return locale_result;
    }
    std::uint32_t fallback_locale_name_07310c(n::Address locale) override {
        locale_arg = locale;
        events.emplace_back("locale-fallback");
        return fallback_result;
    }
};
std::uint64_t random(std::uint64_t& state) {
    state ^= state << 13;
    state ^= state >> 7;
    state ^= state << 17;
    return state;
}
void branch_and_argument_cases() {
    std::uint64_t seed = 0x3c6ef372fe94f82b;
    for (std::uint32_t i = 0; i < 256; ++i) {
        const auto locale = i == 0 ? 0 : random(seed);
        const auto flags = static_cast<std::uint32_t>(random(seed));
        const auto source = i == 1 ? 0 : random(seed);
        const auto source_count = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(random(seed)));
        const auto destination = i == 2 ? 0 : random(seed);
        const auto destination_count = std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(random(seed)));
        const auto version = random(seed), reserved = random(seed), sort = random(seed);
        // Include every isolated EAX bit, zero, and all ones; random full-width results follow.
        const auto result = i < 32 ? (std::uint32_t{1} << i)
            : i == 32 ? 0 : i == 33 ? UINT32_MAX : static_cast<std::uint32_t>(random(seed));
        const auto signed_result = std::bit_cast<std::int32_t>(result);
        const auto lcid = i == 0 ? 0 : i == 1 ? UINT32_MAX : static_cast<std::uint32_t>(random(seed));
        const MapArgs expected{locale, flags, source, source_count, destination,
                               destination_count, version, reserved, sort};
        for (unsigned branch = 0; branch < 3; ++branch) {
            Fixture f;
            f.map_result = signed_result;
            f.wide_result = signed_result;
            f.locale_result = lcid;
            f.fallback_result = lcid;
            // A nonzero all-ones resolved function is forwarded as a token, not rejected here.
            if (i == 3) f.map_function = UINT64_MAX;
            if (branch != 0) f.map_function = 0;
            if (branch == 2) f.locale_function = 0;
            const auto actual = n::lc_map_string_0671d4(locale, flags, source, source_count,
                destination, destination_count, version, reserved, sort, f);
            check(std::bit_cast<std::uint32_t>(actual) == result);
            if (branch == 0) {
                check(f.map_args == expected);
                check(f.events == std::vector<std::string>{"resolve-map", "map-ex"});
            } else {
                check(f.locale_arg == locale);
                check(f.locale_flags == 0);
                check(f.wide_args == WideArgs{lcid, flags, source, source_count, destination, destination_count});
                check(f.events == (branch == 1
                    ? std::vector<std::string>{"resolve-map", "resolve-locale", "locale-native", "map-w"}
                    : std::vector<std::string>{"resolve-map", "resolve-locale", "locale-fallback", "map-w"}));
            }
        }
        for (bool available : {false, true}) {
            Fixture f;
            if (!available) f.locale_function = 0;
            f.locale_result = result;
            f.fallback_result = result;
            f.locale_flags = 0xa5a5a5a5;
            check(n::locale_name_to_lcid_0672c0(locale, flags, f) == result);
            check(f.locale_arg == locale);
            check(f.locale_flags == (available ? flags : 0xa5a5a5a5));
            check(f.events == (available
                ? std::vector<std::string>{"resolve-locale", "locale-native"}
                : std::vector<std::string>{"resolve-locale", "locale-fallback"}));
        }
    }
}
void repeated_and_exception_cases() {
    Fixture f;
    for (auto count : {INT32_MIN, -1, 0, 1, INT32_MAX}) {
        check(n::lc_map_string_0671d4(0, 0, 0, count, 0, count, 0, 0, 0, f) == 0);
        check(std::get<3>(f.map_args) == count && std::get<5>(f.map_args) == count);
    }
    check(f.events.size() == 10); // Wrapper invokes resolver on each call; cache belongs to 0669b0.
    f.events.clear();
    f.throw_native = true;
    bool threw = false;
    try { (void)n::lc_map_string_0671d4(0, 0, 0, -1, 0, 0, 0, 0, 0, f); }
    catch (const std::logic_error&) { threw = true; }
    check(threw);
    check(f.events == std::vector<std::string>{"resolve-map", "map-ex"});
}
}
int main() {
    try {
        branch_and_argument_cases();
        repeated_and_exception_cases();
        std::cout << "PASS NLS dispatch: " << checks << " checks; typed synthetic endpoints only\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
