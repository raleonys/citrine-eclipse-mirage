#include "multibyte_tables.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

namespace m = anadius::crt_multibyte_tables_recovered;
static std::uint64_t checks;
static void check(bool value) {
    ++checks;
    if (!value) throw std::runtime_error("table fixture assertion " + std::to_string(checks));
}
struct Fixture final : m::Effects {
    std::array<std::uint8_t, 0x4000> memory{};
    m::Address initial = 0x1000, record = 0x2000;
    m::CPInfo info{2, {0x81, 0x82, 0xfe, 0xff, 0xc0, 0xbf, 0, 0, 0, 0, 0, 0}};
    std::uint32_t info_result = 0x80000000U, helper_result = 1;
    bool mutate_calls{}, omit_types{}, omit_lower{}, omit_upper{}, zero_types{};
    unsigned info_calls{}, type_calls{}, map_calls{};
    std::vector<std::string> events;
    std::array<std::uint8_t, 256> captured{};
    std::vector<std::uint32_t> call_cp;
    std::vector<m::Address> call_locale;
    std::uint8_t read_u8(m::Address a) override { events.push_back("r8:" + std::to_string(a)); return memory.at(a); }
    template<class T> T read(m::Address a) { T v; std::memcpy(&v, memory.data() + a, sizeof(v)); return v; }
    template<class T> void write(m::Address a, T v) { std::memcpy(memory.data() + a, &v, sizeof(v)); }
    std::uint16_t read_u16(m::Address a) override { return read<std::uint16_t>(a); }
    std::uint32_t read_u32(m::Address a) override { events.push_back("r32:" + std::to_string(a)); return read<std::uint32_t>(a); }
    m::Address read_address(m::Address a) override { events.push_back("ra:" + std::to_string(a)); return read<m::Address>(a); }
    void write_u8(m::Address a, std::uint8_t v) override { events.push_back("w8:" + std::to_string(a)); memory.at(a) = v; }
    void write_u16(m::Address a, std::uint16_t v) override { events.push_back("w16:" + std::to_string(a)); write(a, v); }
    void write_u32(m::Address a, std::uint32_t v) override { write(a, v); }
    void write_address(m::Address a, m::Address v) override { events.push_back("wa:" + std::to_string(a)); write(a, v); }
    std::uint32_t get_cp_info(std::uint32_t cp, m::CPInfo& out) override {
        ++info_calls; call_cp.push_back(cp); out = info;
        if (mutate_calls) { write<std::uint32_t>(record + 4, 1234); write<m::Address>(record + 0x220, 0x1111); }
        return info_result;
    }
    m::Address initial_multibyte_0a1c70() override { events.push_back("initial"); return initial; }
    std::uint32_t get_string_type_06e210(m::Address locale, std::uint32_t type,
        std::span<const std::uint8_t> input, std::span<m::WordOutput> output,
        std::uint32_t cp, std::uint32_t flags) override {
        ++type_calls; call_cp.push_back(cp);
        check(locale == 0 && type == 1 && flags == 0 && input.size() == 256 && output.size() == 256);
        std::copy(input.begin(), input.end(), captured.begin());
        if (!omit_types) for (std::size_t i = 0; i != 256; ++i)
            output[i].set(zero_types ? 0 : static_cast<std::uint16_t>((i % 4) | 0xa500));
        if (mutate_calls) { write<std::uint32_t>(record + 4, 2345); write<m::Address>(record + 0x220, 0x2222); }
        return helper_result;
    }
    std::uint32_t map_string_06cc30(m::Address locale_context, m::Address locale_name, std::uint32_t flags,
        std::span<const std::uint8_t> input, std::span<m::ByteOutput> output,
        std::uint32_t cp, std::uint32_t error_flags) override {
        ++map_calls; call_cp.push_back(cp); call_locale.push_back(locale_name);
        check(locale_context == 0 && error_flags == 0 && input.size() == 256 && output.size() == 256);
        check(std::equal(input.begin(), input.end(), captured.begin()));
        check(flags == (map_calls == 1 ? 0x100U : 0x200U));
        if (!(map_calls == 1 ? omit_lower : omit_upper))
            for (std::size_t i = 0; i != 256; ++i) output[i].set(static_cast<std::uint8_t>(i ^ (map_calls == 1 ? 0x55 : 0xaa)));
        if (mutate_calls && map_calls == 1) { write<std::uint32_t>(record + 4, 3456); write<m::Address>(record + 0x220, 0x3333); }
        return helper_result;
    }
    Fixture() {
        memory.fill(0x7b);
        write<std::uint32_t>(record + 4, 932);
        write<m::Address>(record + 0x220, 0x9876);
    }
};

static void fallback() {
    for (std::uint32_t cp : {65001U, 932U}) {
        Fixture f; f.info_result = 0; f.write<std::uint32_t>(f.record + 4, cp);
        std::array<std::uint8_t, 257> before;
        for (std::size_t i = 0; i != 257; ++i) {
            before[i] = static_cast<std::uint8_t>(i * 13);
            f.memory[f.record + 0x18 + i] = before[i];
        }
        m::build_case_tables_06f0e8(f.record, f);
        check(f.info_calls == (cp == 65001 ? 0U : 1U)); check(f.type_calls == 0 && f.map_calls == 0);
        check(f.memory[f.record + 0x18] == before[0]);
        for (std::uint32_t i = 0; i != 256; ++i) {
            const bool upper = i >= 'A' && i <= 'Z', lower = i >= 'a' && i <= 'z';
            check(f.memory[f.record + 0x19 + i] == static_cast<std::uint8_t>(before[i + 1] | (upper ? 0x10 : lower ? 0x20 : 0)));
            check(f.memory[f.record + 0x119 + i] == (upper ? i + 32 : lower ? i - 32 : 0));
        }
        for (std::size_t i = 0x219; i < 0x220; ++i) check(f.memory[f.record + i] == 0x7b);
        check(f.read<m::Address>(f.record + 0x220) == 0x9876);
    }
}
static void native_output_contract() {
    for (std::uint32_t result : {0U, 1U, 0x80000000U}) {
        Fixture f; f.helper_result = result; f.mutate_calls = true;
        std::fill_n(f.memory.begin() + static_cast<std::ptrdiff_t>(f.record + 0x18), 257, 0x84);
        m::build_case_tables_06f0e8(f.record, f);
        check(f.info_calls == 1 && f.type_calls == 1 && f.map_calls == 2);
        check((f.call_cp == std::vector<std::uint32_t>{932, 1234, 2345, 3456}));
        check((f.call_locale == std::vector<m::Address>{0x2222, 0x3333}));
        for (std::uint32_t i = 0; i != 256; ++i) {
            check(f.captured[i] == (i == 0 || i == 0x81 || i == 0x82 || i >= 0xfe ? 0x20 : i));
            const auto type = i % 4;
            const auto flag = (type & 1) ? 0x10 : (type & 2) ? 0x20 : 0;
            const auto mapped = (type & 1) ? (i ^ 0x55) : (type & 2) ? (i ^ 0xaa) : 0;
            check(f.memory[f.record + 0x19 + i] == static_cast<std::uint8_t>(0x84 | flag));
            check(f.memory[f.record + 0x119 + i] == mapped);
        }
    }
    { // Mapping outputs are not consumed for unclassified bytes.
        Fixture f; f.zero_types = true; f.omit_lower = f.omit_upper = true; f.helper_result = 0;
        m::build_case_tables_06f0e8(f.record, f);
        for (std::size_t i = 0; i != 256; ++i) check(f.memory[f.record + 0x119 + i] == 0);
    }
    for (unsigned missing = 0; missing != 3; ++missing) {
        Fixture f; f.omit_types = missing == 0; f.omit_lower = missing == 1; f.omit_upper = missing == 2;
        bool boundary = false;
        try { m::build_case_tables_06f0e8(f.record, f); }
        catch (const std::runtime_error& e) { boundary = std::string(e.what()).find("consumed undefined") != std::string::npos; }
        check(boundary); check(f.map_calls == 2);
    }
    { Fixture f; f.info.lead_bytes.fill(1); bool boundary = false;
      try { m::build_case_tables_06f0e8(f.record, f); }
      catch (const std::runtime_error& e) { boundary = std::string(e.what()).find("unterminated") != std::string::npos; }
      check(boundary && f.type_calls == 0); }
}
static void reset_copy() {
    for (std::uint64_t shift : {0U, 1U, 0x800U}) {
        Fixture f; f.record = f.initial + shift;
        for (std::size_t i = 0; i != 0x228; ++i) f.memory[f.initial + i] = static_cast<std::uint8_t>(i * 37 + 9);
        auto expected = f.memory;
        for (std::size_t i = 0; i != 257; ++i) expected[f.record + 0x18 + i] = 0;
        for (std::size_t i = 4; i != 0x18; ++i) expected[f.record + i] = 0;
        for (std::size_t i = 0x220; i != 0x228; ++i) expected[f.record + i] = 0;
        for (std::size_t i = 0; i != 257; ++i) expected[f.record + 0x18 + i] = expected[f.initial + 0x18 + i];
        for (std::size_t i = 0; i != 256; ++i) expected[f.record + 0x119 + i] = expected[f.initial + 0x119 + i];
        m::reset_sbcs_06f050(f.record, f);
        for (std::size_t i = 0; i != f.memory.size(); ++i) check(f.memory[i] == expected[i]);
        check(f.events.at(257) == "initial");
        check(f.events.at(258) == "wa:" + std::to_string(f.record + 4));
        check(f.events.at(259) == "wa:" + std::to_string(f.record + 0x220));
        for (std::size_t i = 0; i != 6; ++i) check(f.events.at(260 + i) == "w16:" + std::to_string(f.record + 0xc + 2 * i));
        for (std::size_t i = 0; i != 257; ++i) {
            check(f.events.at(266 + 2 * i) == "r8:" + std::to_string(f.initial + 0x18 + i));
            check(f.events.at(267 + 2 * i) == "w8:" + std::to_string(f.record + 0x18 + i));
        }
    }
}
int main() {
    try { fallback(); native_output_contract(); reset_copy(); std::cout << "PASS " << checks << " checks\n"; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
