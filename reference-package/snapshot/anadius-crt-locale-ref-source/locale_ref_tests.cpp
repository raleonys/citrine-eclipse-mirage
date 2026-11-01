#include "locale_ref.hpp"
#include <array>
#include <bit>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>

namespace r = anadius::crt_locale_ref_recovered;
using r::Address;
std::uint64_t checks = 0;
void require(bool value) {
    ++checks;
    if (!value) throw std::runtime_error("locale reference assertion failed");
}
struct Event {
    char kind;
    Address address;
    std::uint32_t increment;
    bool operator==(const Event&) const = default;
};
struct Memory final : r::Effects {
    std::map<Address, Address> addresses;
    std::map<Address, std::uint32_t> counters;
    std::vector<Event> events;
    std::function<void(Address)> after_update;
    Address read_address(Address address) override {
        events.push_back({'r', address, 0});
        return addresses.at(address);
    }
    std::uint32_t atomic_fetch_add_32(Address address, std::uint32_t increment) override {
        events.push_back({'a', address, increment});
        auto& cell = counters.at(address);
        const auto before = cell;
        cell += increment;
        if (after_update) after_update(address);
        return before;
    }
};
constexpr Address locale = 0x1000;
constexpr Address time_object = 0x4000;
constexpr r::Identities ids{0x1800a1948ULL, 0x18008ad20ULL};

Memory initial_memory() {
    Memory m;
    m.counters[locale + 0x10] = 7;
    for (Address offset : {0xe0, 0xf0, 0xe8, 0x100}) m.addresses[locale + offset] = 0;
    for (Address i = 0; i < 6; ++i) {
        const auto base = locale + i * 0x20;
        m.addresses[base + 0x28] = ids.category_name_0a1948;
        m.addresses[base + 0x20] = 0;
        m.addresses[base + 0x38] = 0;
        m.addresses[base + 0x30] = 0;
    }
    m.addresses[locale + 0x120] = 0;
    return m;
}
void call(bool release, Memory& m) {
    if (release) r::release_locale_ref_0708c0(locale, ids, m);
    else r::add_locale_ref_070634(locale, ids, m);
}

void exhaustive_branch_masks() {
    // The 4 optional counters are independent; each category independently
    // exercises both branch guards and null/non-null counter alternatives.
    for (bool release : {false, true}) {
        const auto delta = release ? 0xffffffffU : 1U;
        for (unsigned optional = 0; optional != 16; ++optional) {
            for (unsigned unicode = 0; unicode != 4; ++unicode) {
                for (unsigned narrow = 0; narrow != 4; ++narrow) {
                    for (unsigned active_category = 0; active_category != 6; ++active_category) {
                        Memory m = initial_memory();
                        std::vector<Event> want{{'a', locale + 0x10, delta}};
                        std::map<Address, std::uint32_t> expected = m.counters;
                        expected[locale + 0x10] += delta;
                        const std::array<Address, 4> offsets{0xe0, 0xf0, 0xe8, 0x100};
                        for (unsigned j = 0; j != 4; ++j) {
                            const auto counter = 0x5000ULL + 8 * j;
                            m.counters[counter] = expected[counter] = 0xffffffffU - j;
                            m.addresses[locale + offsets[j]] = (optional & (1U << j)) ? counter : 0;
                            want.push_back({'r', locale + offsets[j], 0});
                            if (optional & (1U << j)) {
                                want.push_back({'a', counter, delta});
                                expected[counter] += delta;
                            }
                        }
                        for (unsigned j = 0; j != 6; ++j) {
                            const auto base = locale + j * 0x20;
                            const auto uc = 0x6000ULL + 16 * j;
                            const auto nc = uc + 8;
                            m.counters[uc] = expected[uc] = 0x7fffffffU;
                            m.counters[nc] = expected[nc] = 0x80000000U;
                            const bool selected = j == active_category;
                            const bool u_guard = selected && (unicode & 1U);
                            const bool u_counter = selected && (unicode & 2U);
                            const bool n_guard = selected && (narrow & 1U);
                            const bool n_counter = selected && (narrow & 2U);
                            m.addresses[base + 0x28] = u_guard ? 0 : ids.category_name_0a1948;
                            m.addresses[base + 0x38] = u_counter ? uc : 0;
                            m.addresses[base + 0x20] = n_guard ? 0xdead : 0;
                            m.addresses[base + 0x30] = n_counter ? nc : 0;
                            want.push_back({'r', base + 0x28, 0});
                            if (u_guard) {
                                want.push_back({'r', base + 0x38, 0});
                                if (u_counter) {want.push_back({'a', uc, delta}); expected[uc] += delta;}
                            }
                            want.push_back({'r', base + 0x20, 0});
                            if (n_guard) {
                                want.push_back({'r', base + 0x30, 0});
                                if (n_counter) {want.push_back({'a', nc, delta}); expected[nc] += delta;}
                            }
                        }
                        m.addresses[locale + 0x120] = time_object;
                        m.counters[time_object + 0x15c] = expected[time_object + 0x15c] = 0;
                        expected[time_object + 0x15c] += delta;
                        want.push_back({'r', locale + 0x120, 0});
                        want.push_back({'a', time_object + 0x15c, delta});
                        call(release, m);
                        require(m.events == want);
                        require(m.counters == expected);
                    }
                }
            }
        }
    }
}

void special_time_and_wrap() {
    for (Address special : {Address{0}, ids.initial_time_08ad20}) {
        Memory m;
        require(r::add_time_ref_070838(special, ids, m) == 0x7fffffff);
        require(r::release_time_ref_070898(special, ids, m) == 0x7fffffff);
        require(m.events.empty());
        for (bool release : {false, true}) {
            m = initial_memory();
            m.addresses[locale + 0x120] = special;
            call(release, m);
            require(m.events.back() == Event{'r', locale + 0x120, 0});
        }
    }
    for (std::uint32_t start : {0U, 1U, 0x7ffffffeU, 0x7fffffffU, 0x80000000U, 0xfffffffeU, 0xffffffffU}) {
        for (bool release : {false, true}) {
            Memory m;
            m.counters[time_object + 0x15c] = start;
            const auto delta = release ? 0xffffffffU : 1U;
            const std::uint32_t expected = start + delta;
            const auto result = release ? r::release_time_ref_070898(time_object, ids, m)
                                        : r::add_time_ref_070838(time_object, ids, m);
            require(std::bit_cast<std::uint32_t>(result) == expected);
            require(m.counters.at(time_object + 0x15c) == expected);
            require(m.events == std::vector<Event>{{'a', time_object + 0x15c, delta}});
        }
    }
}

void aliases_and_late_reads() {
    for (bool release : {false, true}) {
        Memory m = initial_memory();
        constexpr Address alias = locale + 0x10;
        for (Address offset : {0xe0, 0xf0, 0xe8, 0x100}) m.addresses[locale + offset] = alias;
        for (Address i = 0; i != 6; ++i) {
            const auto base = locale + i * 0x20;
            m.addresses[base + 0x28] = 0;
            m.addresses[base + 0x38] = alias;
            m.addresses[base + 0x20] = 1;
            m.addresses[base + 0x30] = alias;
        }
        m.addresses[locale + 0x120] = alias - 0x15c;
        m.counters[alias] = release ? 0U : 0xffffffffU;
        call(release, m);
        require(m.counters.at(alias) == (release ? 0U - 18U : 17U));

        m = initial_memory();
        const Address first = 0x9000;
        const Address second = 0x9010;
        const Address third = 0x9020;
        m.counters[first] = m.counters[second] = m.counters[third] = 0;
        m.counters[time_object + 0x15c] = 0;
        m.after_update = [&](Address address) {
            if (address == locale + 0x10) m.addresses[locale + 0xe0] = first;
            if (address == first) m.addresses[locale + 0xf0] = second;
            if (address == second) {
                m.addresses[locale + 0x28] = 0;
                m.addresses[locale + 0x38] = third;
            }
            if (address == third) {
                m.addresses[locale + 0x20] = 1;
                m.addresses[locale + 0x30] = first;
                m.addresses[locale + 0x48] = 0;
                m.addresses[locale + 0x58] = second;
                m.addresses[locale + 0x120] = time_object;
            }
        };
        call(release, m);
        const auto delta = release ? 0xffffffffU : 1U;
        require(m.counters.at(first) == std::uint32_t(delta * 2U));
        require(m.counters.at(second) == std::uint32_t(delta * 2U));
        require(m.counters.at(third) == delta);
        require(m.counters.at(time_object + 0x15c) == delta);
    }
}

void null_and_fault_domain() {
    Memory m;
    r::release_locale_ref_0708c0(0, ids, m);
    require(m.events.empty());
    bool faulted = false;
    try {r::add_locale_ref_070634(0, ids, m);} catch (const std::out_of_range&) {faulted = true;}
    require(faulted);
    require(m.events == std::vector<Event>{{'a', 0x10, 1}});
    m = initial_memory();
    m.addresses.erase(locale + 0xf0);
    faulted = false;
    try {r::add_locale_ref_070634(locale, ids, m);} catch (const std::out_of_range&) {faulted = true;}
    require(faulted);
    require(m.counters.at(locale + 0x10) == 8);
    require(m.events.back() == Event{'r', locale + 0xf0, 0});
    // No rollback/free was invented after a provider failure.
}
int main() {
    try {
        exhaustive_branch_masks();
        special_time_and_wrap();
        aliases_and_late_reads();
        null_and_fault_domain();
        std::cout << "locale_ref: " << checks << " checks PASS\n";
        return 0;
    } catch (const std::exception& e) {std::cerr << e.what() << '\n'; return 1;}
}
