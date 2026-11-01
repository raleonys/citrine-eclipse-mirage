#include "atomic.hpp"
#include <array>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <type_traits>
#include <vector>

namespace n = anadius::crt_atomic_native;
namespace {
unsigned checks = 0;
void require(bool condition, const char* description) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << description << '\n';
        std::exit(1);
    }
}
void values_and_wrap() {
    static_assert(!std::is_copy_constructible_v<n::Cell32>);
    static_assert(!std::is_move_constructible_v<n::Cell32>);
    static_assert(!std::is_copy_assignable_v<n::Cell32>);
    static_assert(!std::is_move_assignable_v<n::Cell32>);
    const std::array<std::uint32_t, 8> values{
        0U, 1U, 0x7fffffffU, 0x80000000U,
        0xfffffffeU, 0xffffffffU, 0x55555555U, 0xaaaaaaaaU};
    for (auto value : values) {
        n::Cell32 cell(value);
        require(cell.load() == value, "constructor/load retains all bits");
        require(cell.address() % 4 == 0, "LONG cell is aligned");
        require(cell.fetch_add(0) == value, "zero add returns all previous bits");
        require(cell.fetch_add(1) == value, "increment returns previous value");
        require(cell.load() == value + 1U, "increment wraps modulo 2^32");
        require(cell.fetch_add(0xffffffffU) == value + 1U,
                "decrement returns previous bits");
        require(cell.load() == value, "decrement wraps modulo 2^32");
        require(cell.exchange(~value) == value, "exchange previous full bits");
        require(cell.load() == ~value, "exchange stores full bits");
    }
    n::Cell32 cell(0xffffffffU);
    require(cell.fetch_add(0x80000001U) == 0xffffffffU,
            "large signed-bit increment returns previous value");
    require(cell.load() == 0x80000000U, "large increment wraps correctly");
    require(cell.fetch_add(0x80000000U) == 0x80000000U,
            "INT32_MIN bit increment returns previous value");
    require(cell.load() == 0, "INT32_MIN bit increment wraps to zero");
    const n::Cell32 constant(17);
    require(constant.load() == 17, "const logical load uses mutable LONG");
    n::Cell32 other;
    require(other.address() != cell.address(), "distinct owned identity");
}
void error_and_thread_tests() {
    n::Cell32 cell(7);
    constexpr DWORD sentinel = 0xfedcba98UL;
    ::SetLastError(sentinel);
    require(cell.load() == 7, "load result with error sentinel");
    require(::GetLastError() == sentinel, "load leaves LastError unchanged");
    require(cell.fetch_add(2) == 7, "add result with error sentinel");
    require(::GetLastError() == sentinel, "add leaves LastError unchanged");
    require(cell.exchange(0) == 9, "exchange result with error sentinel");
    require(::GetLastError() == sentinel, "exchange leaves LastError unchanged");

    constexpr std::uint32_t workers = 8, iterations = 50000;
    std::vector<std::thread> threads;
    threads.reserve(workers);
    std::array<std::uint64_t, workers> sums{};
    for (std::uint32_t index = 0; index < workers; ++index) {
        threads.emplace_back([&, index] {
            std::uint64_t sum = 0;
            for (std::uint32_t i = 0; i < iterations; ++i)
                sum += cell.fetch_add(1);
            sums[index] = sum;
        });
    }
    for (auto& worker : threads) worker.join();
    constexpr std::uint32_t total = workers * iterations;
    require(cell.load() == total, "concurrent additions lose no increments");
    std::uint64_t sum = 0;
    for (auto worker_sum : sums) sum += worker_sum;
    require(sum == std::uint64_t(total) * (total - 1) / 2,
            "concurrent previous values have expected aggregate");
    require(cell.exchange(0xffffffffU) == total,
            "exchange sees completed joined additions");

    threads.clear();
    for (std::uint32_t index = 0; index < workers; ++index) {
        threads.emplace_back([&] {
            for (std::uint32_t i = 0; i < iterations; ++i)
                (void)cell.fetch_add(0xffffffffU);
        });
    }
    for (auto& worker : threads) worker.join();
    require(cell.load() == 0xffffffffU - total,
            "concurrent decrements preserve modular arithmetic");
}
}
int main() {
    values_and_wrap();
    error_and_thread_tests();
    std::cout << "PASS: " << checks << " native atomic checks\n";
}
