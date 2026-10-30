#include "preloader_l/runtime/execution.hpp"
#include <algorithm>
#include <bit>
#include <cstring>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace preloader_l::execution_recovered;
namespace {
int checks = 0;
void require(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
struct ThreadError { int index; };
struct Fake final : Platform {
    std::uint32_t thread = 17;
    bool native_held = false;
    int acquires = 0, releases = 0, frequencies = 0;
    std::int64_t frequency = 10'000'000;
    std::vector<std::int64_t> counters;
    std::size_t counter_index = 0;
    std::vector<std::uint32_t> sleeps;
    std::vector<std::string> events;
    std::function<void()> on_sleep;
    std::uint32_t current_thread_id() override {events.emplace_back("thread"); return thread;}
    void acquire_srw_exclusive(std::uintptr_t&) override {
        require(!native_held, "unexpected recursive native SRW acquisition");
        native_held = true; ++acquires; events.emplace_back("acquire");
    }
    void release_srw_exclusive(std::uintptr_t&) override {
        require(native_held, "release without native ownership");
        native_held = false; ++releases; events.emplace_back("release");
    }
    std::int64_t query_performance_frequency() override {
        ++frequencies; events.emplace_back("frequency"); return frequency;
    }
    std::int64_t query_performance_counter() override {
        require(!native_held, "polling timer called with application lock held");
        require(counter_index < counters.size(), "mock counter script exhausted");
        events.emplace_back("counter"); return counters[counter_index++];
    }
    void sleep_milliseconds(std::uint32_t milliseconds) override {
        require(!native_held, "sleep with application lock held");
        sleeps.push_back(milliseconds); events.emplace_back("sleep");
        if (on_sleep) on_sleep();
    }
    [[noreturn]] void raise_thread_error_04a648(int index) override {throw ThreadError{index};}
};
struct Fixture {
    MutexState mutex{};
    std::uint8_t first = 0, second = 0, third = 0, unrelated = 0;
    std::int64_t frequency_cache = 0;
    Fake platform;
    Execution execution{mutex, {first, second, third}, frequency_cache, platform};
    Fixture() {initialize_mutex_04a698(mutex, 2);}
};
std::int64_t wrap_add(std::int64_t a, std::int64_t b) {
    return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(a) + static_cast<std::uint64_t>(b));
}
std::int64_t wrap_mul(std::int64_t a, std::int64_t b) {
    return std::bit_cast<std::int64_t>(static_cast<std::uint64_t>(a) * static_cast<std::uint64_t>(b));
}
std::int64_t high_mul(std::int64_t a, std::int64_t b) {
    // Independent IMUL high-half oracle using only standard unsigned64 math.
    // MSVC x64 has no __int128 language type. Four32-bit products retain the
    // full carry before the signed two's-complement high-half correction.
    const auto ua=std::bit_cast<std::uint64_t>(a);
    const auto ub=std::bit_cast<std::uint64_t>(b);
    const auto a0=static_cast<std::uint64_t>(static_cast<std::uint32_t>(ua));
    const auto a1=ua>>32;
    const auto b0=static_cast<std::uint64_t>(static_cast<std::uint32_t>(ub));
    const auto b1=ub>>32;
    const auto p00=a0*b0;
    const auto p01=a0*b1;
    const auto p10=a1*b0;
    const auto carry=(p00>>32)+static_cast<std::uint32_t>(p01)+static_cast<std::uint32_t>(p10);
    auto high=a1*b1+(p01>>32)+(p10>>32)+(carry>>32);
    if(a<0)high-=ub;
    if(b<0)high-=ua;
    return std::bit_cast<std::int64_t>(high);
}
// Independent arithmetic oracle transcribed from 036784..0367d5 IMUL/SAR/SHR.
std::int64_t assembly_24mhz(std::int64_t counter) {
    constexpr auto magic = -0x4d0b03f86b6f730dLL;
    auto sum = wrap_add(counter, high_mul(counter, magic));
    const auto quotient = (sum >> 24) - (sum >> 63);
    auto remainder = wrap_add(counter, -wrap_mul(quotient, 24'000'000));
    remainder = wrap_mul(remainder, 1'000'000'000);
    sum = wrap_add(remainder, high_mul(remainder, magic));
    const auto fraction = (sum >> 24) - (sum >> 63);
    return wrap_add(fraction, wrap_mul(quotient, 1'000'000'000));
}

void mutex_tests() {
    MutexState preserved;
    std::memset(&preserved, 0xa5, sizeof preserved);
    initialize_mutex_04a698(preserved, 0x102);
    require(preserved.reserved_04 == 0xa5a5a5a5, "initializer preserves offset04");
    require(std::all_of(std::begin(preserved.reserved_18), std::end(preserved.reserved_18),
                       [](std::byte b){return b == std::byte{0xa5};}),
            "initializer preserves opaque bytes18..47");
    require(preserved.type == 0x102 && preserved.storage_08 == 0 && preserved.srw_10 == 0,
            "initializer writes observed type and two zero qwords");
    Fixture f;
    require(sizeof(MutexState) == 0x50, "mutex size");
    require(f.mutex.owner_48 == 0xffffffff && f.mutex.count_4c == 0, "explicit mutex initializer");
    require(lock_04a6ac(f.mutex, f.platform) == 0, "first mutex acquisition");
    require(f.mutex.owner_48 == 17 && f.mutex.count_4c == 1, "owner/count assigned");
    require(lock_04a6ac(f.mutex, f.platform) == 3, "nonrecursive relock error3");
    require(f.mutex.count_4c == 1 && f.platform.acquires == 1, "failed relock restores count");
    require(unlock_04a6b4(f.mutex, f.platform) == 0 && !f.platform.native_held, "final release");
    require(f.mutex.owner_48 == 0xffffffff, "owner cleared");
    for (auto type : {1u, 0x101u, 0x102u}) {
        Fixture nested;
        initialize_mutex_04a698(nested.mutex, type);
        require(lock_04a6ac(nested.mutex, nested.platform) == 0, "recursive first lock");
        require(lock_04a6ac(nested.mutex, nested.platform) == 0, "recursive second lock");
        require(nested.platform.acquires == 1 && nested.mutex.count_4c == 2, "one SRW acquire for recursion");
        unlock_04a6b4(nested.mutex, nested.platform);
        require(nested.platform.releases == 0 && nested.mutex.count_4c == 1, "inner unlock retains SRW");
        unlock_04a6b4(nested.mutex, nested.platform);
        require(nested.platform.releases == 1, "outer unlock releases SRW");
    }
}
void flag_and_error_tests() {
    Fixture f;
    f.execution.finish_flag_036820(f.unrelated);
    require(f.unrelated == 1 && f.first == 0 && f.second == 0 && f.third == 0, "supplied flag only");
    require(f.platform.acquires == 1 && f.platform.releases == 1, "finish acquires/releases");
    Fixture failed;
    lock_04a6ac(failed.mutex, failed.platform);
    try {failed.execution.finish_flag_036820(failed.first); require(false,"expected error5");}
    catch (ThreadError e) {require(e.index == 5, "lock failure maps to index5");}
    require(failed.first == 0 && failed.mutex.count_4c == 1, "failure retains flag/count");
    Fixture overflow;
    overflow.mutex.type = 0x102;
    overflow.mutex.owner_48 = overflow.platform.thread;
    overflow.mutex.count_4c = 0x7ffffffe;
    try {overflow.execution.finish_flag_036820(overflow.first);require(false,"expected error6");}
    catch (ThreadError e) {require(e.index == 6, "count guard maps to index6");}
    require(overflow.first == 0 && overflow.mutex.count_4c == 0x7ffffffe && overflow.platform.releases == 0, "overflow guard restores max-1 and does not unlock");
    Fixture wait_failed;
    lock_04a6ac(wait_failed.mutex, wait_failed.platform);
    try {wait_failed.execution.wait_steps_036880(0);require(false,"expected wait error5");}
    catch (ThreadError e) {require(e.index == 5, "wait checks mutex failure even when count0");}
    require(wait_failed.platform.counter_index == 0 && wait_failed.platform.releases == 0,
            "wait lock failure does not sample timer or release existing lock");
}
void predicate_tests() {
    Fixture f;
    for (int mask=0;mask != 8;++mask) {
        f.first = (mask&1) ? 2 : 0; f.second = (mask&2) ? 255 : 0; f.third = (mask&4) ? 3 : 0;
        lock_04a6ac(f.mutex,f.platform);
        for (auto n : {-3,-2,0,1,2,3,4,-1}) {
            const int limit = n == -1 ? 3 : n;
            const bool expected = limit <= 0 || ((mask&1) && (limit<2 || (mask&2)) && (limit<3 || (mask&4)));
            require(f.execution.steps_finished_initializing_under_lock(n) == expected, "step flag ordering/prefix/nonzero semantics");
        }
        unlock_04a6b4(f.mutex,f.platform);
    }
    Fixture immediate;
    immediate.execution.wait_steps_036880(0);
    immediate.execution.wait_steps_036880(-2);
    require(immediate.platform.sleeps.empty() && immediate.platform.counter_index == 0, "nonpositive other than-1 returns without timer");
}
void clock_tests() {
    // Independently calculated signed128 product high halves, including signs.
    constexpr auto low=std::numeric_limits<std::int64_t>::min();
    constexpr auto high=std::numeric_limits<std::int64_t>::max();
    require(high_mul(low,low)==0x4000000000000000LL,"IMUL minimum squared high half");
    require(high_mul(high,high)==0x3fffffffffffffffLL,"IMUL maximum squared high half");
    require(high_mul(low,high)==-0x4000000000000000LL,"IMUL mixed-sign extremes high half");
    require(high_mul(-1,-1)==0 && high_mul(-1,1)==-1,"IMUL unit signs high half");
    require(counter_to_nanoseconds(123,10'000'000) == 12300, "10MHz path");
    require(counter_to_nanoseconds(24'000'001,24'000'000) == 1'000'000'041, "24MHz fraction truncates");
    require(counter_to_nanoseconds(-24'000'001,24'000'000) == -1'000'000'041, "24MHz signed truncation");
    require(counter_to_nanoseconds(31,3) == 10'333'333'333LL, "generic quotient/remainder path");
    require(counter_to_nanoseconds(-31,3) == -10'333'333'333LL, "generic negative counter");
    const std::int64_t extremes[] = {0,1,-1,23'999'999,24'000'000,-24'000'000,
        std::numeric_limits<std::int64_t>::max(), std::numeric_limits<std::int64_t>::min()};
    for (auto c : extremes) require(counter_to_nanoseconds(c,24'000'000) == assembly_24mhz(c), "24MHz extreme assembly oracle");
    std::uint64_t random=0x384e3e512ca167;
    for(int i=0;i != 10000;++i) {
        random ^= random<<13;random ^= random>>7;random ^= random<<17;
        auto c=std::bit_cast<std::int64_t>(random);
        require(counter_to_nanoseconds(c,24'000'000) == assembly_24mhz(c), "24MHz random assembly oracle");
    }
    const auto max=std::numeric_limits<std::int64_t>::max();
    require(twenty_ms_deadline(max-20'000'001) == max-1, "deadline below saturation");
    require(twenty_ms_deadline(max-20'000'000) == max, "deadline saturation boundary");
    require(twenty_ms_deadline(max) == max, "deadline overflow saturation");
    require(observed_sleep_milliseconds(1) == 1 && observed_sleep_milliseconds(1'000'001) == 2, "sleep ceil-to-ms");
    require(observed_sleep_milliseconds(86'400'000'000'000LL) == 86'400'000, "one-day sleep clamp");
    require(observed_sleep_milliseconds(86'399'999'999'999LL) == 86'400'000,
            "subday fractional millisecond rounds up to full day");
    Fixture cached;
    cached.platform.counters = {1,2};
    require(cached.execution.now_nanoseconds_036740() == 100, "first QPC sample");
    require(cached.execution.now_nanoseconds_036740() == 200 && cached.platform.frequencies == 1, "frequency cached across calls");
    Fixture supplied;
    supplied.frequency_cache = 24'000'000;
    supplied.platform.counters = {48'000'001};
    require(supplied.execution.now_nanoseconds_036740() == 2'000'000'041 && supplied.platform.frequencies == 0,
            "caller supplied nonzero frequency cache is respected");
}
void polling_tests() {
    Fixture f;
    f.platform.counters = {1'000'000, 1'000'000, 1'100'000, 1'200'000};
    f.platform.on_sleep = [&]{if(!f.first) f.execution.finish_flag_036820(f.first);};
    f.execution.wait_steps_036880(1);
    require(f.platform.sleeps == std::vector<std::uint32_t>({20,10}), "finish does not interrupt original20ms deadline loop");
    require(f.platform.counter_index == 4 && f.platform.frequencies == 1, "timer sample count and cache");
    require(f.platform.acquires == 3 && f.platform.releases == 3 && !f.platform.native_held, "poll-check/finish/recheck lock ordering");
    Fixture prefix;
    prefix.first = 1; prefix.second = 1;
    prefix.execution.wait_steps_036880(2);
    require(prefix.platform.sleeps.empty() && !prefix.third, "wait2 ignores unfinished third");
    Fixture all;
    all.first = 1; all.second = 1;
    all.platform.counters = {0,1,200'000};
    all.platform.on_sleep = [&]{all.execution.finish_flag_036820(all.third);};
    all.execution.wait_steps_036880(-1);
    require(all.third == 1 && all.platform.sleeps == std::vector<std::uint32_t>({20}), "-1 waits all three");
    Fixture huge;
    huge.platform.frequency = 1'000'000'000;
    huge.platform.counters = {0,86'400'000'000'000LL};
    huge.execution.sleep_until_observed_deadline(86'400'000'000'000LL);
    require(huge.platform.sleeps == std::vector<std::uint32_t>({86'400'000}), "inner timing loop clamps large gap");
}
}
int main() {
    try {
        mutex_tests();flag_and_error_tests();predicate_tests();clock_tests();polling_tests();
        std::cout << "PASS " << checks << " checks; synthetic/mock tests only; no original DLL loaded\n";
        return 0;
    } catch(const std::exception& e) {std::cerr << "FAIL " << e.what() << '\n';return 1;}
      catch(...) {std::cerr << "FAIL unexpected exception\n";return 1;}
}
