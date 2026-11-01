#include "runtime.hpp"
#include <algorithm>
#include <bit>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace r = anadius::crt_runtime_recovered;
namespace t = anadius::threads_recovered;
namespace {
int checks = 0;
void check(bool condition, int line) {
    ++checks;
    if (!condition) throw std::runtime_error("check failed at " + std::to_string(line));
}
#define CHECK(x) check((x), __LINE__)
struct Fake final : r::Effects {
    std::vector<r::Module> loads{0x100};
    std::vector<std::uint32_t> load_flags;
    std::vector<std::u16string> load_names;
    std::vector<std::pair<r::Module, std::string>> lookups;
    std::vector<r::Module> frees;
    std::map<std::string, r::Address> exports{
        {"RoInitialize", 0x1010}, {"RoUninitialize", 0x1020},
        {"AppPolicyGetThreadInitializationType", 0x1030}};
    std::uint32_t error = 87, flags = 0, policy_status = 0;
    std::int32_t policy_value = 1, ro_result = 0;
    bool write_policy = true;
    int error_reads = 0, flag_reads = 0, policy_calls = 0, ro_calls = 0, unro_calls = 0;
    r::Address last_ro = 0, last_unro = 0, last_policy = 0;
    std::uint32_t last_mode = 0;
    std::uintptr_t last_token = 0;
    std::function<void()> on_load, on_getproc, on_policy, on_ro;
    r::Module load_library_ex_w(std::u16string_view name, r::Module file,
                                std::uint32_t fl) override {
        CHECK(file == 0);
        load_names.emplace_back(name);
        load_flags.push_back(fl);
        if (on_load) on_load();
        const auto n = load_flags.size() - 1;
        return loads[std::min(n, loads.size() - 1)];
    }
    std::uint32_t last_error() override { ++error_reads; return error; }
    bool free_library(r::Module value) override { frees.push_back(value); return false; }
    r::Address get_proc_address(r::Module module, std::string_view name) override {
        lookups.emplace_back(module, name);
        if (on_getproc) on_getproc();
        const auto it = exports.find(std::string(name));
        return it == exports.end() ? 0 : it->second;
    }
    std::uint32_t process_parameters_flags() override { ++flag_reads; return flags; }
    std::int32_t call_ro_initialize(r::Address address, std::uint32_t mode) override {
        ++ro_calls; last_ro = address; last_mode = mode;
        if (on_ro) on_ro();
        return ro_result;
    }
    void call_ro_uninitialize(r::Address address) override {
        ++unro_calls; last_unro = address;
    }
    std::uint32_t call_thread_initialization_policy(
        r::Address address, std::uintptr_t token, std::int32_t* result) override {
        ++policy_calls; last_policy = address; last_token = token;
        if (write_policy) *result = policy_value;
        if (on_policy) on_policy();
        return policy_status;
    }
};

void pointer_encoding() {
    constexpr std::uint64_t values[]{0, 1, ~0ULL, 0x8000000000000000ULL,
        0x0123456789abcdefULL, 0xffffffff00000001ULL};
    for (unsigned rotation = 0; rotation < 64; ++rotation) {
        const auto cookie = 0x123456789abcde00ULL | rotation;
        for (const auto value : values) {
            const auto rotated = rotation == 0 ? value
                : (value << rotation) | (value >> (64 - rotation));
            CHECK(r::encode_pointer(value, cookie) == (rotated ^ cookie));
            CHECK(r::decode_pointer(rotated ^ cookie, cookie) == value);
        }
        r::State state(cookie);
        for (const auto& entry : state.procedures)
            CHECK(r::decode_pointer(entry.load(), cookie) == 0);
    }
}

void resolver_paths() {
    constexpr std::uint32_t one[]{9};
    constexpr std::uint32_t two[]{9, 19};
    {
        r::State s(0xa3); Fake f;
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", one) == 0x1010);
        CHECK(f.load_flags == std::vector<std::uint32_t>{0x800});
        CHECK(f.load_names[0] == u"api-ms-win-core-winrt-l1-1-0");
        CHECK(f.error_reads == 0 && f.frees.empty());
        f.exports.clear();
        CHECK(r::resolve_0669b0(s, f, 23, "different-name", {}) == 0x1010);
        CHECK(f.lookups.size() == 1);
        CHECK(r::resolve_0669b0(s, f, 24, "RoUninitialize", one) == 0);
        CHECK(f.load_flags.size() == 1 && f.lookups.size() == 2);
        CHECK(r::decode_pointer(s.procedures[24].load(), s.cookie) == r::missing);
        f.exports["RoUninitialize"] = 0x1111;
        CHECK(r::resolve_0669b0(s, f, 24, "RoUninitialize", one) == 0);
        CHECK(f.lookups.size() == 2);
    }
    for (const auto prefix : {u"api-ms-example", u"ext-ms-example", u"ordinary.dll", u"API-MS-example"}) {
        for (const auto error : {87U, 5U}) {
            r::State s(0xabcdef); Fake f;
            s.module_names[9] = prefix;
            f.loads = {0, 0x200}; f.error = error;
            const bool fallback = error == 87 &&
                (std::u16string_view(prefix) == u"ordinary.dll" ||
                 std::u16string_view(prefix) == u"API-MS-example");
            CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", one) == (fallback ? 0x1010 : 0));
            CHECK(f.error_reads == 1);
            CHECK(f.load_flags.size() == (fallback ? 2U : 1U));
            CHECK(s.modules[9].load() == (fallback ? 0x200 : r::missing));
            if (fallback) CHECK(f.load_flags[1] == 0);
        }
    }
    {
        r::State s(3); Fake f;
        s.module_names[9] = u"ordinary.dll";
        f.loads = {0};
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", one) == 0);
        CHECK(f.load_flags == (std::vector<std::uint32_t>{0x800, 0}));
        CHECK(s.modules[9].load() == r::missing);
    }
    {
        r::State s(3); Fake f; f.loads = {0, 0x200}; f.error = 5;
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", two) == 0x1010);
        CHECK(s.modules[9].load() == r::missing && s.modules[19].load() == 0x200);
        CHECK(f.lookups.size() == 1 && f.lookups[0].first == 0x200);
    }
    {
        r::State s(3); Fake f; s.modules[9].store(r::missing);
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", two) == 0x1010);
        CHECK(f.load_names.size() == 1 && f.load_names[0] == s.module_names[19]);
    }
    {
        r::State s(3); Fake f; f.exports.clear();
        CHECK(r::resolve_0669b0(s, f, 23, "missing-export", two) == 0);
        CHECK(f.load_flags.size() == 1 && f.lookups.size() == 1);
        CHECK(s.modules[19].load() == 0); // no candidate fallback after missing export
    }
    {
        r::State s(3); Fake f;
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", {}) == 0);
        CHECK(f.load_flags.empty());
        CHECK(r::decode_pointer(s.procedures[23].load(), s.cookie) == r::missing);
    }
    for (const auto concurrent : {r::Module{0}, r::Module{0x400}, r::missing}) {
        r::State s(3); Fake f;
        f.on_load = [&] { s.modules[9].store(concurrent); };
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", one) == 0x1010);
        CHECK(s.modules[9].load() == 0x100);
        CHECK(f.frees.size() == (concurrent == 0 ? 0U : 1U));
        if (concurrent != 0) CHECK(f.frees[0] == 0x100);
        CHECK(f.lookups[0].first == 0x100); // queries local new handle even after free
    }
    {
        r::State s(3); Fake f; f.exports["RoInitialize"] = r::missing;
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", one) == r::missing);
        CHECK(r::resolve_0669b0(s, f, 23, "RoInitialize", one) == 0);
    }
    {
        r::State s(3); Fake f;
        f.on_getproc = [] { throw std::runtime_error("synthetic resolver failure"); };
        try { (void)r::resolve_0669b0(s, f, 23, "RoInitialize", one); CHECK(false); }
        catch (const std::runtime_error&) { CHECK(s.modules[9].load() == 0x100); }
        CHECK(r::decode_pointer(s.procedures[23].load(), s.cookie) == 0);
        CHECK(f.frees.empty());
    }
}

void runtime_paths() {
    constexpr std::uint32_t bit_patterns[]{0, 1, 0x7fffffff, 0x80000000, 0xffffffff};
    for (const auto flags : bit_patterns) {
        for (const auto output : {std::int32_t{-1}, 0, 1, 2, std::numeric_limits<std::int32_t>::min()}) {
            for (const auto status : {0U, 0xc0000225U, 0xffffffffU}) {
                r::State s(0x2d); Fake f;
                f.flags = flags; f.policy_value = output; f.policy_status = status;
                CHECK(r::process_parameters_flag_06c0b8(f) == (flags >> 31));
                CHECK(r::runtime_policy_06c0d0(s, f) == ((flags >> 31) == 0 && output == 1 ? 2U : 1U));
                CHECK(f.policy_calls == ((flags >> 31) == 0 ? 1 : 0));
                const auto reads = f.flag_reads;
                const auto cache = s.thread_policy.load();
                f.flags ^= 0x80000000; f.policy_value ^= 1;
                CHECK(r::runtime_policy_06c0d0(s, f) == cache);
                CHECK(f.flag_reads == reads);
                if (f.policy_calls) {
                    CHECK(f.last_token == r::missing - 5);
                    CHECK(f.last_policy == 0x1030);
                    CHECK(f.load_names[0] == u"api-ms-win-appmodel-runtime-l1-1-2");
                }
            }
        }
    }
    {
        r::State s(3); Fake f; f.exports.clear();
        std::int32_t value = 99;
        CHECK(r::query_thread_policy_066bc4(s, f, &value) == 0xc0000225U);
        CHECK(value == 99 && f.policy_calls == 0);
        CHECK(r::runtime_policy_06c0d0(s, f) == 1);
    }
    {
        r::State s(3); Fake f; f.write_policy = false;
        CHECK(r::runtime_policy_06c0d0(s, f) == 1);
    }
    for (const auto cached : {1U, 2U, 0xffffffffU}) {
        r::State s(3); Fake f; s.thread_policy.store(cached);
        CHECK(r::runtime_policy_06c0d0(s, f) == cached);
        CHECK(f.flag_reads == 0 && f.policy_calls == 0 && f.load_flags.empty());
    }
    for (const bool outer_value_one : {false, true}) {
        r::State s(3); Fake outer, nested;
        outer.policy_value = outer_value_one ? 1 : 0;
        nested.policy_value = outer_value_one ? 0 : 1;
        outer.on_policy = [&] {
            CHECK(r::runtime_policy_06c0d0(s, nested) == (outer_value_one ? 1U : 2U));
        };
        CHECK(r::runtime_policy_06c0d0(s, outer) == (outer_value_one ? 2U : 1U));
        CHECK(s.thread_policy.load() == (outer_value_one ? 2U : 1U));
    }
    for (const auto mode : bit_patterns) {
        for (const auto result_bits : bit_patterns) {
            r::State s(0); Fake f; f.ro_result = std::bit_cast<std::int32_t>(result_bits);
            CHECK(r::initialize_runtime_067320(s, f, mode) == f.ro_result);
            CHECK(f.last_ro == 0x1010 && f.last_mode == mode && f.ro_calls == 1);
            r::uninitialize_runtime_06736c(s, f);
            CHECK(f.unro_calls == 1 && f.last_unro == 0x1020);
            CHECK(f.load_flags.size() == 1 && f.lookups.size() == 2);
        }
    }
    {
        r::State s(3); Fake f; f.exports.clear();
        CHECK(r::initialize_runtime_067320(s, f, 1) == 0);
        r::uninitialize_runtime_06736c(s, f);
        CHECK(f.ro_calls == 0 && f.unro_calls == 0);
        CHECK(f.load_flags.size() == 1 && f.lookups.size() == 2);
    }
}

struct Exited { std::uint32_t code; };
struct Thread final : r::ThreadRuntimeBridge {
    Thread(r::State& s, Fake& f) : ThreadRuntimeBridge(s, f) {}
    t::Context* slot = nullptr;
    int entry_calls = 0;
    t::Context* calloc_context_066848() override { throw std::logic_error("not used"); }
    void free_0668c0(t::Context*) override { throw std::logic_error("not used"); }
    bool get_module_handle_ex_w(std::uint32_t, std::uintptr_t, t::Handle*) override { throw std::logic_error("not used"); }
    t::Handle create_thread(void*, std::uint64_t, t::Context*, std::uint32_t, std::uint32_t*) override { throw std::logic_error("not used"); }
    std::uint32_t last_error() override { throw std::logic_error("not used"); }
    void write_error_slot_05d724(std::uint32_t) override { throw std::logic_error("not used"); }
    void invalid_parameter_05c954() override { throw std::logic_error("not used"); }
    void map_os_error_05d698(std::uint32_t) override { throw std::logic_error("not used"); }
    bool close_handle(t::Handle) override { throw std::logic_error("not used"); }
    bool free_library(t::Handle) override { throw std::logic_error("not used"); }
    t::Context*& required_thread_slot_064b90() override { return slot; }
    t::Context* optional_thread_context_064d08() override { return slot; }
    std::uint32_t invoke_entry(std::uintptr_t entry, std::uintptr_t argument) override {
        CHECK(entry == 0x777 && argument == 0x888); ++entry_calls; return 0x1234abcd;
    }
    [[noreturn]] void free_library_and_exit_thread(t::Handle, std::uint32_t) override { throw std::logic_error("not used"); }
    [[noreturn]] void exit_thread(std::uint32_t code) override { throw Exited{code}; }
};
void thread_composition() {
    for (const bool missing_init : {false, true}) {
        for (const auto ro_result : {0, 1, -1}) {
            for (const auto policy : {0, 1, 2}) {
                r::State state(0x47); Fake effects;
                effects.policy_value = policy; effects.ro_result = ro_result;
                if (missing_init) effects.exports.erase("RoInitialize");
                Thread thread(state, effects);
                t::Context context{0x777, 0x888, 0, 0, 0, {}};
                try { t::thread_entry_05e7a8(&context, thread); CHECK(false); }
                catch (const Exited& exit) { CHECK(exit.code == 0x1234abcd); }
                const bool run_init = policy == 1;
                const bool initialized = run_init && (missing_init || ro_result == 0);
                CHECK(context.runtime_initialized == (initialized ? 1 : 0));
                CHECK(effects.ro_calls == (run_init && !missing_init ? 1 : 0));
                CHECK(effects.unro_calls == (initialized ? 1 : 0));
                CHECK(thread.slot == &context && thread.entry_calls == 1);
            }
        }
    }
}
} // namespace
int main() {
    try {
        pointer_encoding(); resolver_paths(); runtime_paths(); thread_composition();
        std::cout << "PASS " << checks << " synthetic checks\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n'; return 1;
    }
}
