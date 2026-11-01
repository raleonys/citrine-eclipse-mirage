#include "parse.hpp"
#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
namespace recovered = anadius::crt_argv_parse_recovered;
namespace {
std::uint64_t checks = 0;
std::string context;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("parser check failed at " + std::to_string(checks) + " input: " + context); }
struct Probe : recovered::Effects {
    bool dbcs = false;
    std::vector<std::int32_t> seen;
    std::int32_t is_mb_lead_06f9a8(std::int32_t value) override {
        seen.push_back(value);
        return dbcs && value == -127 ? static_cast<std::int32_t>(0x100) : 0;
    }
};
void vector_case(std::string command, const std::vector<std::string>& expected, bool dbcs = false) {
    context = command;
    command.push_back('\0');
    std::uint64_t characters = 0;
    for (const auto& item : expected) characters += item.size() + 1;
    recovered::Counts counted{901, 902}; Probe counter; counter.dbcs = dbcs;
    recovered::parse_0636c0(command, {}, {}, counted, counter);
    check(counted.pointers == expected.size() + 1);
    check(counted.characters == characters);
    for (int mode = 1; mode != 4; ++mode) {
        std::vector<char*> argv(expected.size() + 2, reinterpret_cast<char*>(std::uintptr_t{1}));
        std::vector<char> text(static_cast<std::size_t>(characters) + 1, '\x7f');
        Probe probe; probe.dbcs = dbcs; recovered::Counts got{999, 998};
        recovered::parse_0636c0(command,
            (mode & 1) ? std::span<char*>(argv.data(), argv.size() - 1) : std::span<char*>{},
            (mode & 2) ? std::span<char>(text.data(), text.size() - 1) : std::span<char>{}, got, probe);
        check(got.pointers == counted.pointers && got.characters == counted.characters);
        check(probe.seen == counter.seen);
        check(text.back() == '\x7f');
        check(argv.back() == reinterpret_cast<char*>(std::uintptr_t{1}));
        std::size_t offset = 0;
        for (std::size_t i = 0; i != expected.size(); ++i) {
            if (mode & 1) check(argv[i] == ((mode & 2) ? text.data() + offset : nullptr));
            if (mode & 2) {
                check(std::equal(expected[i].begin(), expected[i].end(), text.begin() + static_cast<std::ptrdiff_t>(offset)));
                check(text[offset + expected[i].size()] == '\0');
            }
            offset += expected[i].size() + 1;
        }
        if (mode & 1) check(argv[expected.size()] == nullptr);
    }
}
// Independent encoder property: quote every argument, escape literal quotes,
// and double only trailing runs of backslashes or runs before a quote.
std::string quote_literal(const std::string& value) {
    std::string encoded = "\"";
    std::size_t slashes = 0;
    for (char byte : value) {
        if (byte == '\\') { ++slashes; continue; }
        encoded.append(byte == '"' ? slashes * 2 + 1 : slashes, '\\');
        slashes = 0;
        encoded.push_back(byte);
    }
    encoded.append(slashes * 2, '\\');
    encoded.push_back('"');
    return encoded;
}
void order_cases() {
    struct Order : recovered::Effects {
        recovered::Counts& counts; std::span<char> text; std::span<char*> argv;
        std::size_t step = 0;
        Order(recovered::Counts& c, std::span<char> t, std::span<char*> a) : counts(c), text(t), argv(a) {}
        std::int32_t is_mb_lead_06f9a8(std::int32_t byte) override {
            check(argv[0] == text.data());
            if (step == 0) { check(byte == 'p'); check(counts.pointers == 1 && counts.characters == 1); check(text[0] == 'p'); }
            if (step == 1) { check(byte == ' '); check(counts.pointers == 1 && counts.characters == 2); check(text[1] == ' '); }
            if (step == 2) { check(byte == 'a'); check(counts.pointers == 2 && counts.characters == 2); check(text[1] == 0 && text[2] == 'a'); check(argv[1] == text.data() + 2); }
            ++step; return 0;
        }
    };
    recovered::Counts counts; std::array<char, 4> text{}; std::array<char*, 3> argv{};
    Order order(counts, text, argv);
    recovered::parse_0636c0(std::string("p a\0", 4), argv, text, counts, order);
    check(order.step == 3); check(counts.pointers == 3 && counts.characters == 4);
    Probe probe;
    recovered::parse_0636c0(std::string("p\0", 2), {}, {}, counts, probe);
    check(probe.seen == std::vector<std::int32_t>{'p', 0});
    probe.seen.clear();
    recovered::parse_0636c0(std::string("p \"a\"\0", 6), {}, {}, counts, probe);
    check(probe.seen == std::vector<std::int32_t>{'p', ' ', 'a'});
}
template<class F> void guarded(F&& f) {
    bool caught = false; try { f(); } catch (const std::out_of_range&) { caught = true; }
    check(caught);
}
void guard_cases() {
    Probe probe; recovered::Counts counts;
    guarded([&]{ recovered::parse_0636c0({}, {}, {}, counts, probe); });
    guarded([&]{ recovered::parse_0636c0(std::string("abc"), {}, {}, counts, probe); });
    std::array<char, 1> text{};
    guarded([&]{ recovered::parse_0636c0(std::string("p\0", 2), {}, text, counts, probe); });
    std::array<char*, 1> argv{};
    guarded([&]{ recovered::parse_0636c0(std::string("p\0", 2), argv, {}, counts, probe); });
    probe.dbcs = true;
    guarded([&]{ recovered::parse_0636c0(std::string(1, static_cast<char>(0x81)), {}, {}, counts, probe); });
    guarded([&]{ recovered::parse_0636c0(std::string("p ") + char(0x81), {}, {}, counts, probe); });
    guarded([&]{ recovered::parse_0636c0(std::string("p \""), {}, {}, counts, probe); });
}
}
int main() {
    try {
        vector_case("", {""});
        vector_case("p", {"p"});
        vector_case(" p", {"", "p"});
        vector_case("\t  p\tq  ", {"", "p", "q"});
        vector_case("p a b\tc", {"p", "a", "b", "c"});
        vector_case("\"program name\" \"a b\" \"\"", {"program name", "a b", ""});
        vector_case("ab\"cd ef\"gh z", {"abcd efgh", "z"});
        vector_case("\"a\"\"b\" x", {"ab", "x"}); // program quotes toggle; they never emit a quote.
        vector_case("p a\"b c\"d", {"p", "ab cd"});
        vector_case("p \"a\"\"b\"", {"p", "a\"b"});
        vector_case("p \"unterminated space ", {"p", "unterminated space "});
        vector_case("p a\"\"b", {"p", "ab"});
        vector_case("p \"\"\"\"", {"p", "\""});
        vector_case("p \"a\n\rb\"", {"p", "a\n\rb"});
        vector_case("p a\n\rb", {"p", "a\n\rb"}); // only SPACE and TAB separate.
        for (std::size_t n = 0; n != 33; ++n) {
            const std::string slash(n, '\\');
            vector_case("p " + slash + "\"x", {"p", std::string(n / 2, '\\') + ((n & 1) ? "\"x" : "x")});
            vector_case("p x" + slash, {"p", "x" + slash});
            vector_case("p " + slash + " x", n ? std::vector<std::string>{"p", slash, "x"} : std::vector<std::string>{"p", "x"});
            vector_case("p" + slash, {"p" + slash});
        }
        const std::string lead(1, static_cast<char>(0x81));
        for (const char trail : std::array<char, 5>{' ', '\t', '"', '\\', static_cast<char>(0xff)}) {
            const std::string pair = lead + trail;
            vector_case(pair + " q", {pair, "q"}, true);
            vector_case("p " + pair + " q", {"p", pair, "q"}, true);
            vector_case("p \"" + pair + " q\"", {"p", pair + " q"}, true);
        }
        // Every possible byte reaches the signed provider unchanged where syntax permits.
        for (unsigned value = 1; value != 256; ++value) {
            const char byte = static_cast<char>(value);
            if (byte == '"' || byte == ' ' || byte == '\t') continue;
            Probe probe; recovered::Counts counts;
            const std::string input = std::string(1, byte) + '\0';
            recovered::parse_0636c0(input, {}, {}, counts, probe);
            const auto signed_value = value < 128 ? static_cast<std::int32_t>(value) : static_cast<std::int32_t>(value) - 256;
            check(probe.seen.size() == 2 && probe.seen.front() == signed_value && probe.seen.back() == 0);
        }
        // Finite literal domain, lengths zero through four; inverse encoding expectation.
        const std::array<char, 6> alphabet{'a', ' ', '\t', '\\', '"', static_cast<char>(0x80)};
        std::vector<std::string> values{std::string{}};
        std::size_t begin = 0, end = 1;
        for (unsigned length = 0; length != 4; ++length) {
            const auto next_begin = values.size();
            for (auto i = begin; i != end; ++i) for (char byte : alphabet) values.push_back(values[i] + byte);
            begin = next_begin; end = values.size();
        }
        for (const auto& value : values)
            vector_case("\"program path\\name\" " + quote_literal(value) + " tail", {"program path\\name", value, "tail"});
        order_cases(); guard_cases();
        std::cout << "PASS " << checks << " checks: argv parser vectors, inverse quoting, DBCS, count/output modes, order and guards\n";
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
