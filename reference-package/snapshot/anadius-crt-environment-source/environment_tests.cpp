#include "environment.hpp"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>
#include <source_location>
#include <stdexcept>
#include <string>
#include <vector>
using namespace anadius::crt_environment_recovered;
namespace {
std::size_t checks{};
void check(bool value, std::source_location location = std::source_location::current()) {
    ++checks;
    if (!value) throw std::runtime_error("environment check failed at line " + std::to_string(location.line()));
}
struct Watson {};
struct Fixture final : Effects {
    std::vector<std::string> events;
    std::vector<std::unique_ptr<char[]>> bytes;
    std::vector<std::unique_ptr<char*[]>> arrays;
    std::vector<void*> frees;
    std::u16string wide = std::u16string(u"A=1\0B=2\0\0", 9);
    std::string narrow = std::string("A=1\0B=2\0\0", 9);
    int allocation{}, fail_at{}, query_result{}, fill_result{1}, copy_result{};
    bool acquisition_null{}, override_query{};
    std::uint64_t malloc_requested{};
    State* mutate_state{};
    Environment mutation{};
    void initialize_multibyte_06f5f8() override { events.emplace_back("multibyte"); }
    std::span<const char16_t> get_environment_strings_w() override {
        events.emplace_back("get"); return acquisition_null ? std::span<const char16_t>{} : std::span<const char16_t>{wide};
    }
    void free_environment_strings_w(std::span<const char16_t> input) override {
        check(input.data() == wide.data()); events.emplace_back("release-wide");
    }
    std::int32_t convert_06cd58(std::span<const char16_t> input, std::int32_t count,
                              Buffer destination, std::int32_t capacity) override {
        check(input.data() == wide.data());
        // Both the last entry terminator and the following final NUL are included.
        check(count == static_cast<std::int32_t>(wide.size()));
        if (!destination.data) {
            check(capacity == 0); events.emplace_back("query");
            return override_query ? query_result : static_cast<std::int32_t>(narrow.size());
        }
        check(capacity == static_cast<std::int32_t>(narrow.size()));
        events.emplace_back("fill");
        if (fill_result) {
            check(destination.capacity >= narrow.size());
            std::memcpy(destination.data, narrow.data(), narrow.size());
        }
        return fill_result;
    }
    Buffer allocate(std::uint64_t count, bool zero) {
        if (++allocation == fail_at) return {};
        auto storage = std::make_unique<char[]>(static_cast<std::size_t>(count));
        std::fill_n(storage.get(), static_cast<std::size_t>(count), zero ? '\0' : '\x7f');
        Buffer result{storage.get(), static_cast<std::size_t>(count)};
        bytes.push_back(std::move(storage)); return result;
    }
    Buffer malloc_06a308(std::uint64_t size) override {
        events.emplace_back("malloc"); malloc_requested = size;
        if (size > 1000000) { ++allocation; return {}; }
        return allocate(size, false);
    }
    Environment calloc_pointers_066848(std::uint64_t count, std::uint64_t size) override {
        check(size == 8); events.emplace_back("calloc-array");
        if (++allocation == fail_at) {
            if (mutate_state) { mutate_state->current_0a5288 = mutation; mutate_state->initial_0a52a0 = mutation; }
            return {};
        }
        auto storage = std::make_unique<char*[]>(static_cast<std::size_t>(count));
        Environment result{storage.get(), static_cast<std::size_t>(count)};
        arrays.push_back(std::move(storage)); return result;
    }
    Buffer calloc_chars_066848(std::uint64_t count, std::uint64_t size) override {
        check(size == 1); events.emplace_back("calloc-string"); return allocate(count, true);
    }
    void free_0668c0(void* pointer) override {
        events.emplace_back(pointer ? "free-value" : "free-null");
        if (pointer) check(std::find(frees.begin(), frees.end(), pointer) == frees.end());
        frees.push_back(pointer);
    }
    std::int32_t strcpy_s_05cff0(Buffer destination, std::span<const char> input) override {
        events.emplace_back("copy"); check(input.size() == destination.capacity);
        check(input.back() == 0);
        if (copy_result == 0) std::memcpy(destination.data, input.data(), input.size());
        return copy_result;
    }
    [[noreturn]] void invoke_watson_05c9a4() override { events.emplace_back("watson"); throw Watson{}; }
};
void trace(const Fixture& f, std::initializer_list<const char*> expected) {
    check(f.events.size() == expected.size());
    std::size_t i = 0; for (auto event : expected) check(f.events[i++] == event);
}
void initializer_cases() {
    Fixture f; State s;
    check(initialize_063a70(s, f) == 0);
    check(s.current_0a5288.data == s.initial_0a52a0.data);
    check(s.current_0a5288.slots == 3);
    check(std::string(s.current_0a5288.data[0]) == "A=1");
    check(std::string(s.current_0a5288.data[1]) == "B=2");
    check(s.current_0a5288.data[2] == nullptr);
    trace(f, {"multibyte","get","query","malloc","fill","free-null","release-wide",
              "calloc-array","calloc-string","copy","free-null","calloc-string","copy",
              "free-null","free-null","free-null","free-value"});
    const auto n = f.events.size();
    check(thunk_initialize_064054(s, f) == 0); check(f.events.size() == n);
    free_environment_063d80(s.current_0a5288, f);
    check(f.frees[f.frees.size()-3] == s.current_0a5288.data[0]);
    check(f.frees[f.frees.size()-2] == s.current_0a5288.data[1]);
    check(f.frees.back() == s.current_0a5288.data);

    for (int failure = 1; failure <= 4; ++failure) {
        Fixture g; State state; g.fail_at = failure;
        check(initialize_063a70(state, g) == 0xffffffffu);
        check(!state.current_0a5288.data && !state.initial_0a52a0.data);
        check(g.allocation == failure);
        if (failure >= 3) {
            check(g.frees[g.frees.size()-5] == g.arrays[0].get());
            check(g.events[g.events.size()-1] == "free-value");
        }
    }
    Fixture g; State state; char* sentinel[1]{};
    g.mutate_state = &state; g.mutation = {sentinel, 1}; g.fail_at = 2;
    check(initialize_063a70(state, g) == 0xffffffffu);
    check(state.current_0a5288.data == sentinel && state.initial_0a52a0.data == sentinel);
}
void acquisition_cases() {
    Fixture f; f.acquisition_null = true; State s;
    check(initialize_063a70(s, f) == 0xffffffffu);
    trace(f, {"multibyte","get","free-null"});
    Fixture q; q.override_query = true;
    check(!convert_block_06f9e4(q).data); trace(q, {"get","query","release-wide"});
    Fixture m; m.fail_at = 1;
    check(!convert_block_06f9e4(m).data);
    trace(m, {"get","query","malloc","free-null","release-wide"});
    Fixture b; b.fill_result = 0;
    check(!convert_block_06f9e4(b).data);
    trace(b, {"get","query","malloc","fill","free-value","release-wide"});
    Fixture neg; neg.override_query = true; neg.query_result = -2;
    check(!convert_block_06f9e4(neg).data);
    check(neg.malloc_requested == 0xfffffffffffffffeULL);
    Fixture e; e.wide = std::u16string(1, u'\0'); e.narrow = std::string(1, '\0');
    check(convert_block_06f9e4(e).capacity == 1);
}
void arrays_and_guards() {
    const std::vector<std::vector<std::string>> cases{{}, {"=C:=C:\\"}, {"=C:=C:\\","=D:=D:\\"},
        {"no_equals"},{"A="},{"A=1","A=2"},{"=skip","x","=middle","y=3"}};
    for (const auto& entries : cases) {
        Fixture f; std::string block; std::vector<std::string> expected;
        for (const auto& entry : entries) { block += entry; block.push_back(0); if (entry[0] != '=') expected.push_back(entry); }
        block.push_back(0);
        auto environment = build_array_063b54(block, f);
        check(environment.slots == expected.size()+1);
        for (std::size_t i=0;i<expected.size();++i) check(std::string(environment.data[i]) == expected[i]);
        check(environment.data[expected.size()] == nullptr);
        free_environment_063d80(environment, f);
        check(f.frees.back() == environment.data);
    }
    for (int n = 1; n <= 128; ++n) {
        Fixture f; std::string block;
        for (int i = 0; i < n; ++i) { block += (i%3 == 0 ? "=skip" : "name=value"); block.push_back(0); }
        block.push_back(0);
        auto env = build_array_063b54(block, f);
        const auto included = static_cast<std::size_t>(n-(n+2)/3);
        check(env.slots == included+1);
        for (std::size_t i=0;i<included;++i) check(std::string(env.data[i]) == "name=value");
        free_environment_063d80(env, f);
    }
    Fixture failed; failed.fail_at = 1;
    const std::string block("x\0\0", 3);
    check(!build_array_063b54(block, failed).data); trace(failed, {"calloc-array","free-null"});
    Fixture fatal; fatal.copy_result = 22;
    try { (void)build_array_063b54(block, fatal); check(false); } catch (const Watson&) { check(true); }
    trace(fatal, {"calloc-array","calloc-string","copy","watson"});
    // No invented cleanup or array publication occurs after the fatal call.
    check(fatal.frees.empty()); check(fatal.arrays[0][0] == nullptr);
    Fixture invalid;
    try { (void)build_array_063b54(std::string("bad"), invalid); check(false); }
    catch (const std::domain_error&) { check(true); }
    check(invalid.events.empty());
    free_environment_063d80({}, invalid); check(invalid.events.empty());
    char value[]="x"; char* corrupt[]{value};
    try { free_environment_063d80({corrupt,1}, invalid); check(false); }
    catch (const std::domain_error&) { check(true); }
    check(invalid.frees.size()==1 && invalid.frees[0]==value);
}
}
int main() {
    initializer_cases(); acquisition_cases(); arrays_and_guards();
    std::cout << "PASS " << checks << " environment checks\n";
}
