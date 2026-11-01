#include "module_path.hpp"
#include <bit>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace mp = anadius::crt_module_path_recovered;
namespace cv = anadius::crt_conversion_recovered;
namespace er = anadius::crt_errors_recovered;
namespace {
unsigned checks{};
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
struct ConversionCall {
    std::uint32_t cp, flags;
    mp::Address source, destination;
    std::int32_t source_count, capacity;
    mp::Address default_char, used_default;
};
class Fixture final : public mp::Effects {
public:
    static constexpr mp::Address source = 0x1000, output = 0x3000;
    std::u16string wide = u"C:\\fixture\\module.dll";
    std::vector<std::uint8_t> bytes = std::vector<std::uint8_t>(512, 0xa5);
    std::vector<std::string> events;
    std::vector<ConversionCall> calls;
    std::vector<std::int32_t> results{22, 22};
    std::size_t result_index{};
    std::uint32_t error = 0xdead, mapped = 84, errno_value = 777;
    std::uint32_t module_count = 21, cp = 65001;
    mp::Address received_module{};
    std::uint32_t received_module_capacity{};
    cv::Buffer* inspect{};
    bool require_unowned_at_errno{};
    mp::WideResult get_module_filename_w(mp::Address module,
                                         std::uint32_t capacity) override {
        events.emplace_back("module");
        received_module = module;
        received_module_capacity = capacity;
        return {source, module_count};
    }
    std::uint32_t choose_code_page() override {
        events.emplace_back("choose");
        return cp;
    }
    std::uint8_t read_u8(mp::Address) override { throw std::runtime_error("unexpected read byte"); }
    std::uint16_t read_u16(mp::Address address) override {
        events.emplace_back("read");
        check(address == source, "source address");
        return static_cast<std::uint16_t>(wide[0]);
    }
    void write_zero_u8(mp::Address address) override {
        events.emplace_back("zero");
        check(address == output, "empty destination");
        bytes[0] = 0;
    }
    void write_zero_u16(mp::Address) override { throw std::runtime_error("unexpected wide write"); }
    mp::Address malloc_06a308(std::uint64_t) override { throw std::runtime_error("unexpected malloc"); }
    void free_0668c0(mp::Address) override { throw std::runtime_error("unexpected free"); }
    std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t, std::uint32_t,
        mp::Address, std::int32_t, mp::Address, std::int32_t) override {
        throw std::runtime_error("wrong conversion direction");
    }
    std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t code_page,
        std::uint32_t flags, mp::Address from, std::int32_t source_count,
        mp::Address destination, std::int32_t capacity,
        mp::Address default_char, mp::Address used_default) override {
        events.emplace_back(destination == 0 ? "query" : "fill");
        calls.push_back({code_page, flags, from, destination, source_count,
                         capacity, default_char, used_default});
        check(result_index < results.size(), "conversion call count bounded");
        const auto result = results[result_index++];
        if (destination != 0 && result > 0 && result <= 512) {
            check(destination == output, "conversion destination");
            for (std::int32_t i = 0; i < result - 1; ++i)
                bytes[static_cast<std::size_t>(i)] = static_cast<std::uint8_t>('A' + i % 26);
            bytes[static_cast<std::size_t>(result - 1)] = 0;
        }
        return result;
    }
    std::uint32_t last_error() override { events.emplace_back("last"); return error; }
    void map_os_error_05d698(std::uint32_t value) override {
        events.emplace_back("map");
        check(value == error, "mapped exact last error");
        errno_value = mapped;
    }
    er::Cell32 errno_cell_05d724() override {
        events.emplace_back("errno");
        if (require_unowned_at_errno) check(inspect && inspect->owned == 0, "ownership cleared before errno accessor");
        return er::Cell32(&errno_value);
    }
};
void verify_arguments(const Fixture& f, std::uint32_t cp, std::int32_t capacity) {
    check(f.calls.size() == 2, "query and fill");
    for (const auto& call : f.calls) {
        check(call.cp == cp && call.flags == 0, "conversion code page and flags");
        check(call.source == Fixture::source && call.source_count == -1, "NUL source count");
        check(call.default_char == 0 && call.used_default == 0, "default conversion arguments");
    }
    check(f.calls[0].destination == 0 && f.calls[0].capacity == 0, "sizing query");
    check(f.calls[1].destination == Fixture::output && f.calls[1].capacity == capacity, "fill exact capacity");
}
void fixed_cases() {
    {
        Fixture f;
        cv::Buffer b{Fixture::output, 99, 81, 7};
        check(mp::fixed_to_narrow_0601c8(0, b, 65001, f) == 0, "NULL success");
        check(b.data == 0 && b.capacity == 0 && b.length == 0 && b.owned == 0, "NULL clears fields");
        check(f.events.empty() && f.errno_value == 777 && f.bytes[0] == 0xa5, "NULL has no provider effects");
    }
    for (const auto owned : {std::uint8_t{0}, std::uint8_t{1}, std::uint8_t{255}}) {
        Fixture f;
        f.wide.clear();
        cv::Buffer b{Fixture::output, 0, 83, owned};
        f.inspect = &b; f.require_unowned_at_errno = true;
        check(mp::fixed_to_narrow_0601c8(Fixture::source, b, 1, f) == 34, "empty no capacity");
        check(b.data == Fixture::output && b.length == 83 && b.capacity == 0 && b.owned == 0, "capacity failure preserves stale fields");
        check(f.errno_value == 34 && f.bytes[0] == 0xa5, "34 errno and no output write");
        check(f.events == std::vector<std::string>{"read", "errno"}, "empty capacity error order");
    }
    for (std::uint64_t capacity = 1; capacity <= 32; ++capacity) {
        Fixture f;
        f.wide.clear();
        cv::Buffer b{Fixture::output, capacity, 83, 9};
        check(mp::fixed_to_narrow_0601c8(Fixture::source, b, 0, f) == 0, "empty within capacity");
        check(b.data == Fixture::output && b.capacity == capacity && b.length == 0 && b.owned == 9, "empty preserves ownership");
        check(f.bytes[0] == 0 && f.bytes[1] == 0xa5 && f.errno_value == 777, "single NUL write");
        check(f.events == std::vector<std::string>{"read", "zero"}, "empty order");
    }
    for (std::int32_t required = 1; required <= 64; ++required) {
        for (const std::uint64_t capacity : {std::uint64_t{0}, static_cast<std::uint64_t>(required - 1), static_cast<std::uint64_t>(required), static_cast<std::uint64_t>(required + 5)}) {
            Fixture f;
            f.results = {required, required};
            cv::Buffer b{Fixture::output, capacity, 83, 6};
            f.inspect = &b; f.require_unowned_at_errno = true;
            const auto status = mp::fixed_to_narrow_0601c8(Fixture::source, b, 65001, f);
            if (capacity < static_cast<std::uint64_t>(required)) {
                check(status == 34 && b.capacity == 0 && b.owned == 0, "short fixed destination");
                check(b.data == Fixture::output && b.length == 83, "short destination stale fields");
                check(f.bytes[0] == 0xa5 && f.calls.size() == 1, "no fill after size refusal");
                check(f.events == std::vector<std::string>{"read", "query", "errno"}, "short buffer effect order");
            } else {
                check(status == 0 && b.length == static_cast<std::uint64_t>(required - 1), "successful logical count excludes NUL");
                check(b.capacity == capacity && b.owned == 6 && b.data == Fixture::output, "success borrowed storage unchanged");
                check(f.bytes[static_cast<std::size_t>(required - 1)] == 0 && f.bytes[static_cast<std::size_t>(required)] == 0xa5, "fill NUL and untouched suffix");
                check(f.errno_value == 777, "successful conversion preserves errno");
                verify_arguments(f, 65001, static_cast<std::int32_t>(capacity));
            }
        }
    }
    for (const bool query_failure : {true, false}) {
        Fixture f;
        f.results = query_failure ? std::vector<std::int32_t>{0} : std::vector<std::int32_t>{22, 0};
        cv::Buffer b{Fixture::output, 100, 83, 4};
        check(mp::fixed_to_narrow_0601c8(Fixture::source, b, 0, f) == 84, "API error returns errno");
        check(b.data == Fixture::output && b.capacity == 100 && b.length == 83 && b.owned == 4, "API error preserves buffer fields");
        auto expected = std::vector<std::string>{"read", "query"};
        if (!query_failure) expected.emplace_back("fill");
        expected.insert(expected.end(), {"last", "map", "errno"});
        check(f.events == expected, "API error mapping order");
    }
    {
        Fixture f; f.results = {-1};
        cv::Buffer b{Fixture::output, 0x7fffffffffffffffULL, 83, 1};
        check(mp::fixed_to_narrow_0601c8(Fixture::source, b, 1, f) == 34, "negative query unsigned size");
        check(f.calls.size() == 1 && b.length == 83, "negative refused before fill");
    }
    {
        Fixture f; f.results = {-1, -3};
        cv::Buffer b{Fixture::output, UINT64_MAX, 83, 1};
        check(mp::fixed_to_narrow_0601c8(Fixture::source, b, 1, f) == 0, "huge capacity accepts signextended query");
        check(b.length == UINT64_MAX - 3 && b.capacity == UINT64_MAX && b.owned == 1, "negative fill signextension minus one");
        verify_arguments(f, 1, -1);
    }
    {
        Fixture f; f.results = {1, 1};
        cv::Buffer b{Fixture::output, 0x180000001ULL, 83, 0};
        check(mp::fixed_to_narrow_0601c8(Fixture::source, b, 0, f) == 0, "full-width capacity comparison");
        verify_arguments(f, 0, std::bit_cast<std::int32_t>(0x80000001U));
    }
}
void module_cases() {
    for (const auto cp : {0U, 1U, 65001U}) {
        Fixture f; f.cp = cp;
        check(mp::module_filename_06ecdc(0xabcdef, Fixture::output, 260, f) == 21, "module byte count excludes NUL");
        check(f.received_module == 0xabcdef && f.received_module_capacity == 261, "module provider has 261 wchar scratch");
        verify_arguments(f, cp, 260);
        check(f.events == std::vector<std::string>{"module", "choose", "read", "query", "fill"}, "module call/locale/convert order");
    }
    {
        Fixture f; f.module_count = 0;
        check(mp::module_filename_06ecdc(0, Fixture::output, 260, f) == 0, "module API zero returns zero");
        check(f.events == std::vector<std::string>{"module", "last", "map"}, "module failure no locale or errno accessor");
        check(f.errno_value == 84 && f.bytes[0] == 0xa5, "module failure maps error without output writes");
    }
    for (const auto count : {1U, 260U, 261U, UINT32_MAX}) {
        Fixture f; f.module_count = count;
        check(mp::module_filename_06ecdc(1, Fixture::output, 0x100000104ULL, f) == 21, "module count only tested against zero and low capacity DWORD used");
        verify_arguments(f, 65001, 260);
    }
    for (const auto capacity : {0ULL, 21ULL, 0x100000000ULL}) {
        Fixture f;
        check(mp::module_filename_06ecdc(1, Fixture::output, capacity, f) == 0, "capacity error discarded returns initial length zero");
        check(f.errno_value == 34 && f.calls.size() == 1 && f.bytes[0] == 0xa5, "module fixed capacity status effects");
    }
    for (const bool query_failure : {true, false}) {
        Fixture f; f.results = query_failure ? std::vector<std::int32_t>{0} : std::vector<std::int32_t>{22, 0};
        check(mp::module_filename_06ecdc(1, Fixture::output, 260, f) == 0, "conversion status ignored module length zero");
        check(f.errno_value == 84, "module conversion failure errno effects retained");
    }
    {
        Fixture f; f.wide.clear();
        check(mp::module_filename_06ecdc(0, Fixture::output, 260, f) == 0, "module empty string");
        check(f.bytes[0] == 0 && f.calls.empty(), "module empty no conversion calls");
    }
    {
        Fixture f; f.results = {22, -3};
        check(mp::module_filename_06ecdc(1, Fixture::output, 260, f) == UINT32_MAX - 3, "module returns only length low DWORD");
    }
}
}
int main() {
    try {
        fixed_cases(); module_cases();
        std::cout << "PASS: " << checks << " module-path checks\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL after " << checks << ": " << e.what() << '\n';
        return 1;
    }
}
