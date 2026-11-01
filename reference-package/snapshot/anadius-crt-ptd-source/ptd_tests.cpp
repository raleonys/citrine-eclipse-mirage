#include "ptd.hpp"
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace p = anadius::crt_ptd_recovered;
namespace {
int checks = 0;
void check(bool value, int line) {
    ++checks;
    if (!value) throw std::runtime_error("failed line " + std::to_string(line));
}
#define CHECK(value) check((value), __LINE__)
struct Aborted {};
struct Escaped {};
struct Fake final : p::Effects {
    p::Ptd allocated{}, existing{};
    p::Ptd* slot = nullptr;
    bool sentinel_ok = true, publish_ok = true, clear_ok = true;
    bool alloc_ok = true, free_ok = true;
    std::uint32_t alloc_index = 7, error = 123;
    std::vector<std::string> events;
    std::vector<std::uint32_t> indices;
    std::vector<p::Ptd*> freed, destroyed;
    std::function<void()> on_alloc, on_construct, on_destroy, on_free;
    std::function<void()> on_fls_free;
    void event(const char* text) { events.emplace_back(text); ++error; }
    p::Ptd* fls_get_value(std::uint32_t i) override {
        event("get"); indices.push_back(i); return slot;
    }
    bool fls_set_value(std::uint32_t i, p::Ptd* value) override {
        indices.push_back(i);
        const bool sentinel = value == p::construction_sentinel();
        event(sentinel ? "sentinel" : value ? "publish" : "clear");
        const bool ok = sentinel ? sentinel_ok : value ? publish_ok : clear_ok;
        if (ok) slot = value;
        return ok;
    }
    std::uint32_t fls_alloc(p::FlsCallback callback) override {
        CHECK(callback == p::FlsCallback::destroy_0649cc);
        event("flsalloc"); return alloc_index;
    }
    bool fls_free(std::uint32_t i) override {
        event("flsfree"); indices.push_back(i);
        if (on_fls_free) on_fls_free();
        return free_ok;
    }
    std::uint32_t last_error() override { events.emplace_back("last"); return error; }
    void set_last_error(std::uint32_t value) override {
        events.emplace_back("restore"); error = value;
    }
    p::Ptd* calloc_ptd_066848() override {
        event("calloc");
        if (on_alloc) on_alloc();
        return alloc_ok ? &allocated : nullptr;
    }
    void free_0668c0(p::Ptd* value) override {
        event(value ? "free" : "free-null"); freed.push_back(value);
        if (on_free) on_free();
    }
    void construct_ptd_array_0648fc(p::Ptd* value) override {
        CHECK(value == &allocated); event("construct");
        if (on_construct) on_construct();
    }
    void destroy_ptd_array_0649ec(p::Ptd* value) override {
        event("destroy"); destroyed.push_back(value);
        if (on_destroy) on_destroy();
    }
    [[noreturn]] void abort_05f03c() override { events.emplace_back("abort"); throw Aborted{}; }
};
using Acquire = p::Ptd* (*)(p::State&, p::Effects&);
void acquisition_matrix() {
    const Acquire functions[]{p::optional_preserve_error_064d08,
        p::required_preserve_error_064b90, p::required_raw_064c64,
        p::optional_raw_state0_064dd0};
    for (int api = 0; api != 4; ++api) {
        for (int invalid = 0; invalid != 2; ++invalid) {
            for (int outcome = 0; outcome != 7; ++outcome) {
                Fake fx; p::State state{invalid ? p::invalid_fls_index : 7};
                // Invalid index skips get; an existing value is consequently ignored.
                if (outcome == 0) fx.sentinel_ok = false;
                if (outcome == 1) fx.alloc_ok = false;
                if (outcome == 2) fx.publish_ok = false;
                if (outcome == 3) { fx.publish_ok = false; fx.clear_ok = false; }
                if (outcome == 4) fx.slot = p::construction_sentinel();
                if (outcome == 5) fx.slot = &fx.existing;
                const bool has_existing = !invalid && outcome >= 4 && outcome <= 5;
                const bool fails = outcome <= 3 || (!invalid && outcome == 4);
                p::Ptd* result = nullptr; bool aborted = false;
                try { result = functions[api](state, fx); }
                catch (const Aborted&) { aborted = true; }
                CHECK(aborted == (fails && (api == 1 || api == 2)));
                if (!aborted) CHECK(result == (fails ? nullptr : has_existing ? &fx.existing : &fx.allocated));
                if (api < 2) CHECK(fx.error == 123);
                else CHECK(fx.error != 123);
                CHECK(fx.events.front() == (api < 2 ? "last" : invalid ? "sentinel" : "get"));
                if (api < 2 || aborted || !fails)
                    CHECK(fx.events.back() == (aborted ? "abort" : api < 2 ? "restore" : has_existing ? "get" : "free-null"));
                if (!invalid) CHECK(fx.indices.front() == 7);
                else for (auto index : fx.indices) CHECK(index == p::invalid_fls_index);
                const bool alloc_failed = outcome == 1;
                const bool publish_failed = outcome == 2 || outcome == 3;
                if (alloc_failed || publish_failed) {
                    CHECK(fx.freed.size() == 1);
                    CHECK(fx.freed.front() == (alloc_failed ? nullptr : &fx.allocated));
                } else if (!fails && !has_existing) {
                    CHECK(fx.freed == std::vector<p::Ptd*>{nullptr});
                    CHECK(fx.slot == &fx.allocated);
                } else CHECK(fx.freed.empty());
                if (outcome == 3) CHECK(fx.slot == p::construction_sentinel());
                if (aborted && api == 1) CHECK(fx.events[fx.events.size() - 2] == "restore");
            }
        }
    }
}
void reentry_and_exceptions() {
    Fake fx; p::State state{7};
    fx.on_alloc = [&] {
        CHECK(fx.slot == p::construction_sentinel());
        CHECK(p::optional_preserve_error_064d08(state, fx) == nullptr);
        bool aborted = false;
        try { (void)p::required_preserve_error_064b90(state, fx); }
        catch (const Aborted&) { aborted = true; }
        CHECK(aborted);
    };
    fx.on_construct = [&] {
        CHECK(fx.slot == &fx.allocated);
        CHECK(p::required_raw_064c64(state, fx) == &fx.allocated);
        CHECK(fx.allocated.thread_context == nullptr);
        for (auto value : fx.allocated.storage) CHECK(value == std::byte{});
    };
    CHECK(p::optional_preserve_error_064d08(state, fx) == &fx.allocated);
    CHECK(fx.error == 123);
    for (bool at_free : {false, true}) {
        Fake thrown; p::State s{7}; bool escaped = false;
        if (at_free) thrown.on_free = [] { throw Escaped{}; };
        else thrown.on_construct = [] { throw Escaped{}; };
        try { (void)p::required_preserve_error_064b90(s, thrown); }
        catch (const Escaped&) { escaped = true; }
        CHECK(escaped); CHECK(thrown.slot == &thrown.allocated);
        CHECK(thrown.error != 123);
        CHECK(thrown.events.back() == (at_free ? "free-null" : "construct"));
        CHECK(thrown.freed.size() == (at_free ? 1u : 0u));
    }
}
void destruction() {
    for (int value = 0; value != 4; ++value) {
        for (bool clear : {false, true}) {
            Fake fx; p::State state{value == 0 ? p::invalid_fls_index : 7};
            fx.slot = value == 2 ? &fx.existing : value == 3 ? p::construction_sentinel() : nullptr;
            p::Ptd* before = fx.slot;
            fx.clear_ok = clear;
            p::free_ptd_064b4c(state, fx);
            if (value == 0) CHECK(fx.events.empty());
            if (value == 1) CHECK(fx.events == std::vector<std::string>{"get"});
            if (value >= 2) {
                CHECK((fx.events == std::vector<std::string>{"get", "clear", "destroy", "free"}));
                CHECK(fx.destroyed == std::vector<p::Ptd*>{before});
                CHECK(fx.freed == std::vector<p::Ptd*>{before});
                CHECK(fx.slot == (clear ? nullptr : before));
            }
        }
    }
    Fake fx; p::State state{7}; fx.slot = &fx.existing;
    fx.on_destroy = [] { throw Escaped{}; };
    bool escaped = false;
    try { p::free_ptd_064b4c(state, fx); } catch (const Escaped&) { escaped = true; }
    CHECK(escaped); CHECK(fx.slot == nullptr); CHECK(fx.freed.empty());
    CHECK(fx.events.back() == "destroy");
}
void lifetime() {
    for (int failure = 0; failure != 4; ++failure) {
        Fake fx; p::State state;
        fx.alloc_index = failure == 0 ? p::invalid_fls_index : 9;
        fx.sentinel_ok = failure != 1;
        fx.alloc_ok = failure != 2;
        fx.free_ok = false;
        fx.on_fls_free = [&] { CHECK(state.fls_index == 9); };
        CHECK(p::initialize_064e90(state, fx) == (failure == 3));
        if (failure == 0) {
            CHECK(fx.events == std::vector<std::string>{"flsalloc"});
            CHECK(state.fls_index == p::invalid_fls_index);
        } else if (failure < 3) {
            CHECK(fx.events.back() == "flsfree");
            CHECK(state.fls_index == p::invalid_fls_index);
        } else {
            CHECK(state.fls_index == 9); CHECK(fx.slot == &fx.allocated);
            CHECK(fx.error == 124); // FlsAlloc precedes optional's error capture.
            CHECK(p::uninitialize_064ecc(state, fx));
            CHECK(state.fls_index == p::invalid_fls_index);
        }
        fx.events.clear(); CHECK(p::uninitialize_064ecc(state, fx));
        CHECK(fx.events.empty());
    }
    Fake fx; p::State state{9};
    fx.on_fls_free = [] { throw Escaped{}; };
    bool escaped = false;
    try { (void)p::uninitialize_064ecc(state, fx); } catch (const Escaped&) { escaped = true; }
    CHECK(escaped); CHECK(state.fls_index == 9);
    p::Ptd value;
    anadius::threads_recovered::Context context{};
    value.thread_context = &context;
    CHECK(reinterpret_cast<const std::byte*>(&value.thread_context) - reinterpret_cast<const std::byte*>(&value) == 0x3c0);
    CHECK(value.thread_context == &context);
}
}
int main() {
    try {
        acquisition_matrix(); reentry_and_exceptions(); destruction(); lifetime();
        std::cout << "PASS " << checks << " PTD checks\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
