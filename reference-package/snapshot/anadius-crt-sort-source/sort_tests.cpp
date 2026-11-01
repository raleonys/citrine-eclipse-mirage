#include "sort.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace s = anadius::crt_sort_recovered;
namespace {
std::size_t checks = 0;
struct InvalidFailure {};
struct WriteFailure {};
void require(bool condition, const char* what) {
    ++checks;
    if (!condition) throw std::runtime_error(what);
}
struct Fixture final : s::Effects {
    static constexpr s::Address base = 0x10000;
    static constexpr s::Address comparator = 0xabc;
    std::vector<std::uint8_t> bytes;
    std::uint64_t width = 1;
    std::uint32_t error = 99;
    std::size_t reads = 0, writes = 0, invalid = 0, error_queries = 0;
    bool addresses = false, throw_invalid = false;
    int writes_until_throw = -1;
    std::vector<std::pair<s::Address, s::Address>> comparisons;
    std::vector<std::string> events;
    explicit Fixture(std::vector<std::uint8_t> b = {}) : bytes(std::move(b)) {}
    std::size_t offset(s::Address p) const {
        if (p < base || p - base >= bytes.size()) throw std::out_of_range("bounded sort fixture");
        return static_cast<std::size_t>(p - base);
    }
    std::uint8_t read_u8(s::Address p) override {
        const auto i = offset(p);
        ++reads;
        events.push_back("r" + std::to_string(i));
        return bytes[i];
    }
    void write_u8(s::Address p, std::uint8_t v) override {
        const auto i = offset(p);
        if (writes_until_throw == 0) throw WriteFailure{};
        if (writes_until_throw > 0) --writes_until_throw;
        ++writes;
        events.push_back("w" + std::to_string(i));
        bytes[i] = v;
    }
    std::int32_t compare(s::Address callback, s::Address left, s::Address right) override {
        require(callback == comparator, "comparator identity");
        const auto l = offset(left), r = offset(right);
        require(l % width == 0 && r % width == 0, "comparator receives element start");
        comparisons.emplace_back(left, right);
        if (addresses) return s::compare_slot_addresses_06e41c(left, right);
        return bytes[l] < bytes[r] ? -73 : bytes[l] > bytes[r] ? 256 : 0;
    }
    anadius::crt_errors_recovered::Cell32 errno_cell_05d724() override {
        ++error_queries;
        events.push_back("errno");
        return anadius::crt_errors_recovered::Cell32(&error);
    }
    void invalid_parameter_05c954() override {
        require(error == 22, "errno stored before invalid handler");
        ++invalid;
        events.push_back("invalid");
        if (throw_invalid) throw InvalidFailure{};
    }
    void run(std::size_t n) { s::qsort_0768f0(base, n, width, comparator, *this); }
};
void invalid_cases() {
    for (const auto count : {0ULL, 1ULL, 2ULL}) {
        for (const auto width : {0ULL, 1ULL}) {
            for (const auto base : {0ULL, Fixture::base}) {
                for (const auto callback : {0ULL, Fixture::comparator}) {
                    Fixture f({1, 2});
                    const bool bad = (base == 0 && count != 0) || width == 0 || callback == 0;
                    s::qsort_0768f0(base, count, width, callback, f);
                    require(f.invalid == static_cast<std::size_t>(bad), "invalid argument branch");
                    require(f.error_queries == static_cast<std::size_t>(bad), "errno queried once only invalid");
                    require(f.error == (bad ? 22U : 99U), "errno unchanged for valid calls");
                    if (bad) require(f.events == std::vector<std::string>{"errno", "invalid"}, "invalid event order");
                }
            }
        }
    }
    Fixture f;
    f.throw_invalid = true;
    try { s::qsort_0768f0(0, 1, 1, Fixture::comparator, f); require(false, "invalid handler should throw"); }
    catch (const InvalidFailure&) { require(f.error == 22 && f.invalid == 1, "no invented invalid cleanup"); }
    Fixture empty;
    s::qsort_0768f0(0, 0, 1, Fixture::comparator, empty);
    require(empty.events.empty() && empty.comparisons.empty(), "null zero count valid no effects");
}
void permutations() {
    for (std::size_t n = 2; n <= 8; ++n) {
        std::vector<std::uint8_t> values(n);
        std::iota(values.begin(), values.end(), std::uint8_t{0});
        do {
            Fixture f(values);
            f.run(n);
            require(std::is_sorted(f.bytes.begin(), f.bytes.end()), "all shortsort permutations sorted");
        } while (std::next_permutation(values.begin(), values.end()));
    }
}
void generic_partition_cases() {
    std::uint32_t seed = 0x78563412;
    for (const auto width : {1ULL, 2ULL, 3ULL, 8ULL}) {
        for (std::size_t n = 9; n <= 160; ++n) {
            for (unsigned mode = 0; mode < 4; ++mode) {
                std::vector<std::uint8_t> records(n * width);
                for (std::size_t i = 0; i < n; ++i) {
                    seed = seed * 1664525U + 1013904223U;
                    const auto value = static_cast<std::uint8_t>(mode == 0 ? n - i : mode == 1 ? i : mode == 2 ? 7 : seed >> 25);
                    std::fill_n(records.begin() + static_cast<std::ptrdiff_t>(i * width), width, value);
                }
                Fixture f(records);
                f.width = width;
                f.run(n);
                auto expected = records;
                std::sort(expected.begin(), expected.end());
                require(f.bytes == expected, "partition sorting and record byte integrity");
                require(f.invalid == 0 && f.error_queries == 0, "valid partition does not touch errno");
            }
        }
    }
}
void address_comparator_cases() {
    for (const auto a : {0ULL, 1ULL, 0x7fffffffffffffffULL, 0x8000000000000000ULL, 0xffffffffffffffffULL}) {
        for (const auto b : {0ULL, 1ULL, 0x7fffffffffffffffULL, 0x8000000000000000ULL, 0xffffffffffffffffULL}) {
            require(s::compare_slot_addresses_06e41c(a, b) == (a < b ? -1 : a > b ? 1 : 0), "unsigned address comparator extremes");
        }
    }
    for (const auto n : {2U, 3U, 8U, 9U, 10U, 17U, 62U, 127U, 512U}) {
        for (const auto width : {1ULL, 3ULL, 8ULL}) {
            std::vector<std::uint8_t> records(n * width);
            for (std::size_t i = 0; i < records.size(); ++i) records[i] = static_cast<std::uint8_t>((i * 73U) ^ 0xa5U);
            Fixture f(records);
            f.width = width;
            f.addresses = true;
            f.run(n);
            require(f.bytes == records, "address callback preserves arbitrary slot bytes");
            require(f.reads == 0 && f.writes == 0, "address callback no slot dereference or swap");
            require(!f.comparisons.empty(), "generic algorithm actually invokes comparator");
        }
    }
    Fixture nine(std::vector<std::uint8_t>(9, 7));
    nine.addresses = true;
    nine.run(9);
    const std::array<std::pair<unsigned, unsigned>, 23> trace{{
        {0,4},{0,8},{4,8},{1,4},{2,4},{3,4},{5,4},{7,4},{6,4},{5,4},{3,4},
        {6,5},{7,6},{8,7},{6,5},{7,6},{6,5},
        {1,0},{2,1},{3,2},{1,0},{2,1},{1,0}}};
    // Independently enumerated median/partition/right-shortsort/left-shortsort path.
    require(nine.comparisons.size() == 23, "nine-element address callback count");
    for (std::size_t i = 0; i < 23; ++i) {
        require(nine.comparisons[i] == std::pair<s::Address, s::Address>{Fixture::base + trace[i].first, Fixture::base + trace[i].second}, "nine-element exact callback sequence");
    }
}
void byte_order_and_failures() {
    Fixture f({9, 0x99, 1, 0x11});
    f.width = 2;
    f.run(2);
    require(f.bytes == std::vector<std::uint8_t>{1,0x11,9,0x99}, "multi-byte swap payload");
    require(f.events == std::vector<std::string>{"r2","r0","w0","w2","r3","r1","w1","w3"}, "byte swap access order");
    Fixture fail({9, 1});
    fail.writes_until_throw = 1;
    try { fail.run(2); require(false, "write fault required"); }
    catch (const WriteFailure&) { require(fail.bytes == std::vector<std::uint8_t>{1,1}, "partial writes no invented rollback"); }
    Fixture bounded({1,2});
    try { bounded.run(3); require(false, "bounded provider rejection"); }
    catch (const std::out_of_range&) { require(bounded.invalid == 0, "memory rejection is provider domain not CRT invalid path"); }
}
}
int main() {
    try {
        invalid_cases();
        permutations();
        generic_partition_cases();
        address_comparator_cases();
        byte_order_and_failures();
        std::cout << "PASS " << checks << " checks: recovered generic qsort and unsigned slot-address comparator\n";
    } catch (const std::exception& e) {
        std::cerr << "FAIL after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
