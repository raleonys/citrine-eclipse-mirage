#include "vcrt_ptd.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <string>
#include <vector>
using namespace anadius::vcrt_ptd_recovered;
namespace {
int checks;
void check(bool value, int line) {
    ++checks;
    if (!value) { std::fprintf(stderr, "FAIL line %d\n", line); std::exit(1); }
}
#define CHECK(x) check((x), __LINE__)
struct Abort {};
struct Fake final : Effects {
    Ptd initial{}, allocation{};
    State state{initial};
    void* slot{};
    std::uint32_t error = 0x12345678;
    std::uint32_t allocated_index = 17;
    bool locks_ok = true, oom = false, free_ok = true;
    std::vector<bool> set_results;
    std::size_t set_calls{};
    std::vector<std::string> log;
    std::vector<std::uint32_t> indices;
    std::vector<void*> set_values, freed;
    std::function<void(void*)> on_set;
    std::function<void()> on_calloc, on_fls_free;
    std::uint32_t fls_alloc_050b0c(FlsCallback callback) override {
        CHECK(callback == FlsCallback::destroy_04d5d4);
        log.emplace_back("alloc-index"); error = 101; return allocated_index;
    }
    bool fls_free_050b54(std::uint32_t index) override {
        log.emplace_back("free-index"); indices.push_back(index); error = 102;
        if (on_fls_free) on_fls_free();
        return free_ok;
    }
    void* fls_get_050b9c(std::uint32_t index) override {
        log.emplace_back("get"); indices.push_back(index); error = 103; return slot;
    }
    bool fls_set_050be4(std::uint32_t index, void* value) override {
        log.emplace_back("set"); indices.push_back(index); set_values.push_back(value);
        const bool result = set_calls < set_results.size() ? set_results[set_calls] : true;
        ++set_calls; error = 104;
        if (result) slot = value;
        if (on_set) on_set(value);
        return result;
    }
    Ptd* calloc_05cae4(std::size_t count, std::size_t bytes) override {
        CHECK(count == 1); CHECK(bytes == 0x80);
        log.emplace_back("calloc"); error = 105;
        if (on_calloc) on_calloc();
        return oom ? nullptr : &allocation;
    }
    void free_05caec(void* value) override {
        log.emplace_back("free"); freed.push_back(value); error = 106;
    }
    std::uint32_t last_error() override { log.emplace_back("last-error"); return error; }
    void set_last_error(std::uint32_t value) override { log.emplace_back("restore-error"); error = value; }
    bool initialize_locks_05093c() override { log.emplace_back("locks-init"); return locks_ok; }
    void uninitialize_locks_050984() override { log.emplace_back("locks-uninit"); }
    [[noreturn]] void abort_05f03c() override { log.emplace_back("abort"); throw Abort{}; }
};
void basics() {
    Fake f;
    CHECK(sizeof(Ptd) == 128); CHECK(offsetof(Ptd, frame_state) == 120);
    CHECK(f.state.fls_index == invalid_fls_index);
    CHECK(get_noexit_04d658(f.state, f) == nullptr);
    CHECK(get_noinit_04d718(f.state, f) == nullptr);
    CHECK(!thread_attach_04d584(f.state, f));
    CHECK(thread_detach_04d598(f.state, f));
    CHECK(uninitialize_ptd_04d7b0(f.state, f));
    CHECK(f.log.empty()); CHECK(f.error == 0x12345678);
    try { (void)get_required_04d63c(f.state, f); CHECK(false); }
    catch (const Abort&) { CHECK(f.log == std::vector<std::string>{"abort"}); }
    f.log.clear();
    for (void* value : {static_cast<void*>(nullptr), static_cast<void*>(&f.initial),
                       static_cast<void*>(&f.allocation), construction_sentinel()}) {
        free_callback_04d5d4(f.state, f, value);
    }
    CHECK(f.freed == std::vector<void*>({&f.allocation, construction_sentinel()}));
}
void existing_values() {
    for (unsigned mode = 0; mode != 4; ++mode) {
        for (std::uint32_t incoming : {0u, 1u, 0xffffffffu, 0xa5a5a5a5u}) {
            Fake f;
            f.state.fls_index = 23;
            f.slot = mode == 0 ? nullptr : mode == 1 ? construction_sentinel()
                   : mode == 2 ? static_cast<void*>(&f.initial) : static_cast<void*>(&f.allocation);
            f.error = incoming;
            void* expected = mode < 2 ? nullptr : f.slot;
            CHECK(get_noinit_04d718(f.state, f) == expected);
            CHECK(f.error == incoming);
            CHECK(f.log == std::vector<std::string>({"last-error", "get", "restore-error"}));
            CHECK(f.set_values.empty()); CHECK(f.freed.empty());
            if (mode != 0) {
                f.log.clear();
                CHECK(get_noexit_04d658(f.state, f) == expected);
                CHECK(f.error == incoming); CHECK(f.set_values.empty()); CHECK(f.freed.empty());
                CHECK(f.log == std::vector<std::string>({"last-error", "get", "restore-error"}));
            }
        }
    }
}
void allocation_paths() {
    for (bool sentinel_ok : {false, true}) for (bool oom : {false, true})
    for (bool publication_ok : {false, true}) for (bool reset_ok : {false, true}) {
        Fake f;
        f.state.fls_index = 31; f.oom = oom;
        f.set_results = sentinel_ok ? (oom ? std::vector<bool>{true, reset_ok}
                                   : std::vector<bool>{true, publication_ok, reset_ok})
                                   : std::vector<bool>{false};
        const bool success = sentinel_ok && !oom && publication_ok;
        CHECK(get_noexit_04d658(f.state, f) == (success ? &f.allocation : nullptr));
        CHECK(f.error == 0x12345678);
        CHECK(f.allocation.frame_state == (success ? -2 : 0));
        CHECK(f.set_values.front() == construction_sentinel());
        if (!sentinel_ok) {
            CHECK(f.set_calls == 1); CHECK(f.freed.empty()); CHECK(f.slot == nullptr);
        } else {
            CHECK(f.freed.size() == 1);
            CHECK(f.freed[0] == (success || oom ? nullptr : &f.allocation));
            CHECK(f.set_calls == (oom || success ? 2u : 3u));
            if (success) CHECK(f.slot == &f.allocation);
            else {
                CHECK(f.set_values.back() == nullptr);
                CHECK(f.slot == (reset_ok ? nullptr : construction_sentinel()));
            }
        }
        CHECK(f.log.back() == "restore-error");
        CHECK(std::all_of(f.indices.begin(), f.indices.end(), [](auto i) { return i == 31; }));
    }
    Fake f;
    f.state.fls_index = 44;
    f.on_calloc = [&] {
        CHECK(f.slot == construction_sentinel());
        const auto error = f.error;
        CHECK(get_noexit_04d658(f.state, f) == nullptr);
        CHECK(get_noinit_04d718(f.state, f) == nullptr);
        CHECK(f.error == error);
    };
    f.on_set = [&](void* value) {
        if (value == &f.allocation) {
            CHECK(f.allocation.frame_state == 0); // Write occurs after successful publish.
            CHECK(get_noexit_04d658(f.state, f) == &f.allocation);
        }
    };
    CHECK(get_required_04d63c(f.state, f) == &f.allocation);
    CHECK(f.allocation.frame_state == -2); CHECK(f.error == 0x12345678);
}
void initialization() {
    for (bool locks : {false, true}) for (bool index : {false, true})
    for (bool publish : {false, true}) for (bool freed : {false, true}) {
        Fake f;
        f.locks_ok = locks; f.allocated_index = index ? 7 : invalid_fls_index;
        f.set_results = {publish}; f.free_ok = freed;
        f.initial.frame_state = 92;
        f.on_set = [&](void* value) { CHECK(value == &f.initial); CHECK(f.initial.frame_state == 92); };
        f.on_fls_free = [&] { CHECK(f.state.fls_index == 7); };
        const bool success = locks && index && publish;
        CHECK(initialize_04d55c(f.state, f) == success);
        CHECK(f.initial.frame_state == (success ? -2 : 92));
        CHECK(f.state.fls_index == (success ? 7u : invalid_fls_index));
        CHECK(f.log.front() == "locks-init");
        CHECK(std::count(f.log.begin(), f.log.end(), "locks-uninit") == (locks && !success ? 1 : 0));
        CHECK(std::count(f.log.begin(), f.log.end(), "free-index") == (locks && index && !publish ? 1 : 0));
        CHECK(f.freed.empty());
        if (!locks) CHECK(f.log.size() == 1);
    }
    Fake f;
    CHECK(initialize_ptd_04d768(f.state, f));
    f.allocated_index = 19;
    CHECK(initialize_ptd_04d768(f.state, f)); // No inferred initialized guard.
    CHECK(f.state.fls_index == 19);
    CHECK(std::count(f.log.begin(), f.log.end(), "alloc-index") == 2);
    CHECK(std::count(f.log.begin(), f.log.end(), "free-index") == 0);
}
void cleanup() {
    for (unsigned mode = 0; mode != 4; ++mode) for (bool set_ok : {false, true}) {
        Fake f;
        f.state.fls_index = 13;
        f.slot = mode == 0 ? nullptr : mode == 1 ? construction_sentinel()
                   : mode == 2 ? static_cast<void*>(&f.initial) : static_cast<void*>(&f.allocation);
        const void* before = f.slot;
        f.set_results = {set_ok};
        CHECK(thread_detach_04d598(f.state, f));
        CHECK(f.set_values == std::vector<void*>{nullptr});
        CHECK(f.freed.size() == (mode == 1 || mode == 3 ? 1u : 0u));
        if (!f.freed.empty()) CHECK(f.freed.front() == before);
        CHECK(f.error == (f.freed.empty() ? 104u : 106u)); // No preservation here.
        CHECK(f.slot == (set_ok ? nullptr : before));
        CHECK(f.state.fls_index == 13);
    }
    for (std::uint8_t terminating : {0u, 1u, 2u, 255u}) for (bool free_ok : {false, true}) {
        Fake f; f.state.fls_index = 22; f.free_ok = free_ok;
        CHECK(uninitialize_04d5a8(f.state, f, terminating));
        CHECK(f.state.fls_index == (terminating ? 22u : invalid_fls_index));
        CHECK(f.log == (terminating ? std::vector<std::string>{}
                      : std::vector<std::string>{"free-index", "locks-uninit"}));
    }
    Fake f; f.state.fls_index = 24;
    f.on_fls_free = [&] {
        CHECK(f.state.fls_index == 24);
        free_callback_04d5d4(f.state, f, &f.initial);
        free_callback_04d5d4(f.state, f, &f.allocation);
    };
    CHECK(late_uninitialize_04d5c4(f.state, f));
    CHECK(f.state.fls_index == invalid_fls_index);
    CHECK(f.log == std::vector<std::string>({"free-index", "free"}));
    CHECK(f.freed == std::vector<void*>{&f.allocation});
    f.log.clear(); CHECK(late_uninitialize_04d5c4(f.state, f)); CHECK(f.log.empty());
}
}
int main() {
    basics(); existing_values(); allocation_paths(); initialization(); cleanup();
    std::printf("PASS: %d checks; VCRT 128-byte PTD and lifecycle synthetic effects only\n", checks);
}
