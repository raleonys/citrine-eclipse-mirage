#include "cache.hpp"
#include <bit>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace c = anadius::crt_cache_recovered;
namespace r = anadius::crt_runtime_recovered;
static unsigned checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) throw std::runtime_error(#x); } while (false)
struct Entropy final : c::EntropyEffects {
    std::uint64_t time = 0x123456789abcdef0ULL, counter = 0xfedcba9876543210ULL;
    std::uintptr_t stack = 0x7ffddddd0100ULL;
    std::uint32_t tid = 0xdeadbeef, pid = 0xabcdef98;
    bool result = true, write = true, time_write = true;
    std::vector<int> calls;
    void system_time_as_file_time(std::uint64_t& out) override {
        CHECK(out == 0); calls.push_back(1); if (time_write) out = time;
    }
    std::uint32_t current_thread_id() override { calls.push_back(2); return tid; }
    std::uint32_t current_process_id() override { calls.push_back(3); return pid; }
    c::CounterObservation query_performance_counter() override {
        calls.push_back(4); return {result, write ? std::optional{counter} : std::nullopt};
    }
    std::uintptr_t entropy_stack_cell_address() override { calls.push_back(5); return stack; }
};
struct Free final : c::CacheEffects {
    r::State& state;
    std::vector<r::Module> calls;
    bool should_throw = false;
    explicit Free(r::State& s) : state(s) {}
    bool free_library(r::Module m) override {
        CHECK(m != 0 && m != r::missing);
        const auto index = static_cast<std::size_t>(m - 100);
        CHECK(state.modules[index].load() == m); // clear occurs after call
        for (std::size_t i = 0; i < index; ++i) CHECK(state.modules[i].load() == 0);
        calls.push_back(m);
        if (should_throw) throw std::runtime_error("synthetic dependency failure");
        return false; // return status is ignored
    }
};
struct Loader final : r::Effects {
    std::u16string_view loaded;
    r::Module load_library_ex_w(std::u16string_view s, r::Module f, std::uint32_t flags) override {
        CHECK(f == 0 && flags == 0x800); loaded = s; return 100;
    }
    std::uint32_t last_error() override { throw std::runtime_error("unexpected error"); }
    bool free_library(r::Module) override { throw std::runtime_error("unexpected free"); }
    r::Address get_proc_address(r::Module m, std::string_view n) override {
        CHECK(m == 100 && n == "synthetic_final_export"); return 0xabc01234;
    }
    std::uint32_t process_parameters_flags() override { return 0; }
    std::int32_t call_ro_initialize(r::Address, std::uint32_t) override { return 0; }
    void call_ro_uninitialize(r::Address) override {}
    std::uint32_t call_thread_initialization_policy(r::Address, std::uintptr_t, std::int32_t*) override { return 0; }
};
int main() {
    try {
        // Entropy order, DWORD zero extension, mask and complement; expected
        // arithmetic is deliberately independent of the reconstruction helper.
        for (const auto api_result : {false, true}) {
            c::CookieState s; Entropy f; f.result = api_result;
            const auto expected = ((std::uint64_t{0x76543210} << 32) ^ f.counter ^
                f.time ^ std::uint64_t{f.tid} ^ std::uint64_t{f.pid} ^ f.stack) & 0xffffffffffffULL;
            c::initialize_cookie_04c134(s, f);
            CHECK(s.cookie == expected && s.complement == ~expected);
            CHECK(f.calls == std::vector<int>({1,2,3,4,5}));
        }
        for (const auto cookie : {std::uint64_t{0}, std::uint64_t{1},
                                  ~std::uint64_t{0}, c::default_cookie + 1}) {
            c::CookieState s{cookie, 0}; Entropy f;
            c::initialize_cookie_04c134(s, f);
            CHECK(s.cookie == cookie && s.complement == ~cookie);
            CHECK(f.calls.empty());
        }
        {
            c::CookieState s; Entropy f;
            f.time = c::default_cookie; f.counter = f.tid = f.pid = f.stack = 0;
            c::initialize_cookie_04c134(s, f);
            CHECK(s.cookie == c::default_cookie + 1);
            CHECK(s.complement == ~(c::default_cookie + 1));
        }
        {
            c::CookieState s; Entropy f;
            f.time = f.counter = f.tid = f.pid = f.stack = 0; f.time_write = false;
            c::initialize_cookie_04c134(s, f);
            CHECK(s.cookie == 0 && s.complement == ~std::uint64_t{0});
        }
        {
            c::CookieState s{c::default_cookie, 0x1234}; Entropy f; f.write = false; f.result = false;
            try { c::initialize_cookie_04c134(s, f); CHECK(false); }
            catch (const c::UninitializedCounterDomain&) {}
            CHECK(s.cookie == c::default_cookie && s.complement == 0x1234);
            CHECK(f.calls == std::vector<int>({1,2,3,4}));
        }
        for (std::uint64_t rotation = 0; rotation < 64; ++rotation) {
            const auto cookie = 0x123456789abcde00ULL | rotation;
            r::State state(cookie);
            CHECK(state.modules.size() == 21 && state.procedures.size() == 31);
            for (auto& cell : state.procedures) cell.store(0x5555);
            CHECK(c::initialize_thunks_067538(state) == ((cookie & ~0xffULL) | 1));
            for (const auto& cell : state.procedures) {
                CHECK(cell.load() == cookie);
                CHECK(r::decode_pointer(cell.load(), cookie) == 0);
            }
            for (auto pointer : {std::uint64_t{0}, std::uint64_t{1}, ~std::uint64_t{0},
                                 std::uint64_t{0x0123456789abcdef}}) {
                auto independently_encoded = pointer;
                for (std::uint64_t bit = 0; bit < rotation; ++bit)
                    independently_encoded = (independently_encoded << 1) | (independently_encoded >> 63);
                independently_encoded ^= cookie;
                CHECK(r::encode_pointer(pointer, cookie) == independently_encoded);
                CHECK(r::decode_pointer(independently_encoded, cookie) == pointer);
            }
            c::EncodedGlobals globals;
            CHECK(c::initialize_encoded_globals_0644b0(globals, cookie));
            CHECK(globals.a4f00 == cookie && globals.a5178 == cookie);
            for (const auto cell : globals.a4f20) CHECK(cell == cookie);
            CHECK(globals.a5a18 == cookie && globals.a4f10 == cookie);
        }
        {
            r::State state(0x35); Free f(state);
            for (std::size_t i = 0; i < 21; ++i)
                state.modules[i].store(i % 3 == 0 ? 0 : i % 3 == 1 ? r::missing : 100+i);
            CHECK(c::uninitialize_thunks_06755c(state, f, 0));
            CHECK(f.calls == std::vector<r::Module>({102,105,108,111,114,117,120}));
            for (const auto& cell : state.modules) CHECK(cell.load() == 0);
            for (const auto& cell : state.procedures) CHECK(cell.load() == state.cookie);
            CHECK(c::uninitialize_thunks_06755c(state, f, 0));
            CHECK(f.calls.size() == 7);
        }
        for (const auto flag : {std::uint8_t{1}, std::uint8_t{2}, std::uint8_t{255}}) {
            r::State state(7); Free f(state);
            for (std::size_t i = 0; i < 21; ++i) state.modules[i].store(100+i);
            CHECK(c::uninitialize_thunks_06755c(state, f, flag)); CHECK(f.calls.empty());
            for (std::size_t i = 0; i < 21; ++i) CHECK(state.modules[i].load() == 100+i);
        }
        {
            r::State state(7); Free f(state); f.should_throw = true;
            state.modules[20].store(120);
            try { c::uninitialize_thunks_06755c(state, f, 0); CHECK(false); }
            catch (const std::runtime_error&) {}
            CHECK(state.modules[20].load() == 120);
        }
        {
            r::State state(7); Loader f;
            CHECK(state.module_names[0] == u"api-ms-win-core-datetime-l1-1-1");
            CHECK(state.module_names[20] == u"user32");
            for (std::size_t i = 0; i < 21; ++i) {
                CHECK(!state.module_names[i].empty());
                for (std::size_t j = 0; j < i; ++j) CHECK(state.module_names[i] != state.module_names[j]);
            }
            const std::uint32_t candidates[]{20};
            CHECK(r::resolve_0669b0(state, f, 30, "synthetic_final_export", candidates) == 0xabc01234);
            CHECK(f.loaded == u"user32" && state.modules[20].load() == 100);
            CHECK(r::decode_pointer(state.procedures[30].load(), state.cookie) == 0xabc01234);
        }
        std::cout << "PASS " << checks << " checks\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
