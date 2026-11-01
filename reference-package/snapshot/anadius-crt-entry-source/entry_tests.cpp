#include "entry.hpp"
#include <algorithm>
#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace c = anadius::crt_entry_recovered;
namespace {
unsigned checks{};
void check(bool condition, int line) {
    ++checks;
    if (!condition) throw std::runtime_error("check line " + std::to_string(line));
}
#define CHECK(...) check((__VA_ARGS__), __LINE__)
struct Failure { unsigned code; };
struct Escape {};
struct Fake : c::Effects {
    std::vector<std::string> events;
    std::uint64_t vinit{1}, ainit{1}, vtattach{1}, atattach{1}, validate{1};
    std::uint32_t inite{}, argv{}, cterm{}, app{1}, rollback_app{7};
    std::uintptr_t slot{}, replacement{};
    std::uintptr_t module{0x1234}, reserved{0x5678};
    bool mutate_slot{};
    std::string escape_event;
    c::State* observe{};
    std::uint32_t tls_count{};
    void event(const std::string& value) {
        events.push_back(value);
        if (value == escape_event) throw Escape{};
    }
    void security_init_cookie_04c134() override { event("cookie"); }
    void initialize_cpu_04b26c() override { event("cpu"); }
    std::uint64_t vcrt_initialize_04d55c() override { event("vinit"); return vinit; }
    std::uint64_t acrt_initialize_064594() override { event("ainit"); return ainit; }
    std::uint64_t vcrt_uninitialize_04d5a8(bool b) override {
        event(b ? "vuninit1" : "vuninit0"); return 0;
    }
    void acrt_uninitialize_0645cc(bool b) override { event(b ? "auninit1" : "auninit0"); }
    std::uint64_t vcrt_thread_attach_04d584() override { event("vtattach"); return vtattach; }
    std::uint64_t acrt_thread_attach_0645a8() override { event("atattach"); return atattach; }
    void acrt_thread_detach_0645bc() override { event("atdetach"); }
    void vcrt_thread_detach_04d598() override { event("vtdetach"); }
    void initialize_slist_04c1e0() override { event("slistinit"); }
    void configure_stdio_04c1fc() override { event("stdio"); }
    std::uint32_t initterm_e_064658() override { event("initterm_e"); return inite; }
    void initterm_064614() override { event("initterm"); }
    std::uint32_t configure_narrow_argv_0638e8(std::uint32_t mode) override {
        CHECK(mode == 1); event("argv"); return argv;
    }
    void initialize_narrow_environment_064054() override { event("env"); }
    std::uint32_t c_termination_complete_05cdb8() override { event("cterm"); return cterm; }
    void cexit_05cd9c() override { event("cexit"); }
    void release_slist_04c1f0() override { event("slistfree"); }
    void acrt_late_cleanup_064604(bool value) override { CHECK(!value); event("alate"); }
    void vcrt_late_cleanup_04d5c4() override { event("vlate"); }
    const std::uintptr_t* tls_callback_slot_04c218() override { event("slot"); return &slot; }
    std::uint64_t slot_in_nonwritable_image_04b64c(const std::uintptr_t* value) override {
        CHECK(value == &slot); event("validate");
        if (mutate_slot) slot = replacement;
        return validate;
    }
    void invoke_tls_callback(std::uintptr_t callback, std::uintptr_t m,
        std::uint32_t reason, std::uintptr_t r) override {
        CHECK(callback == slot); CHECK(m == module); CHECK(reason == 2); CHECK(r == reserved);
        if (observe) { CHECK(observe->startup_state == 2); CHECK(observe->attach_count_bits == tls_count); }
        event("tls");
    }
    std::uint32_t application_dll_main_038870(std::uintptr_t m,
        std::uint32_t reason, std::uintptr_t r) override {
        CHECK(m == module); CHECK(r == reserved);
        event("app" + std::to_string(reason));
        return reason == 0 ? rollback_app : app;
    }
    [[noreturn]] void fastfail_04bfec(std::uint32_t code) override {
        event("fail" + std::to_string(code)); throw Failure{code};
    }
};
struct Fixture {
    anadius::crt_onexit_recovered::Table application{};
    c::State state{application};
    Fake fx;
    c::Entry entry{state, fx};
    Fixture() { fx.observe = &state; }
    std::uint32_t call(std::uint32_t reason) {
        return entry.entry_04bcfc(fx.module, reason, fx.reserved);
    }
    void sequence(std::initializer_list<const char*> expected) {
        std::vector<std::string> want;
        for (auto* item : expected) want.emplace_back(item);
        CHECK(fx.events == want);
    }
};
void success_and_detach() {
    Fixture f;
    CHECK(f.call(1) == 1);
    f.sequence({"cookie","cpu","vinit","ainit","slistinit","stdio","initterm_e","argv","env","initterm","slot","app1"});
    CHECK(f.state.startup_state == 2); CHECK(f.state.attach_count_bits == 1);
    CHECK(f.state.initialized_as_dll == 1); CHECK(f.state.onexit_initialized == 1);
    CHECK(&f.state.onexit_table == &f.application);
    CHECK(f.application.begin == UINT64_MAX); CHECK(f.application.end == UINT64_MAX);
    CHECK(f.application.capacity_end == UINT64_MAX);
    CHECK(f.state.quickexit_table.begin == UINT64_MAX);
    CHECK(f.state.quickexit_table.end == UINT64_MAX);
    CHECK(f.state.quickexit_table.capacity_end == UINT64_MAX);
    f.fx.events.clear(); f.fx.rollback_app = 0;
    CHECK(f.call(0) == 1); // app FALSE is replaced by successful CRT detach
    f.sequence({"app0","cterm","cexit","slistfree","auninit1","vuninit1","alate","vlate"});
    CHECK(f.state.startup_state == 0); CHECK(f.state.attach_count_bits == 0);
    f.fx.events.clear(); CHECK(f.call(0) == 0); f.sequence({});
    CHECK(f.state.onexit_initialized == 1); // detach does not clear this flag
}
void low_byte_semantics() {
    constexpr std::array<std::uint64_t, 8> words{0,1,0x100,0x101,0x80000000,
        0x80000001,UINT64_MAX,0xffffffffffffff00ull};
    for (auto v : words) for (auto a : words) {
        Fixture f; f.fx.vinit = v; f.fx.ainit = a;
        CHECK(f.entry.initialize_crt_04b584(0) == ((v & 255) != 0 && (a & 255) != 0));
        CHECK(f.state.initialized_as_dll == 1);
        if ((v & 255) == 0) f.sequence({"cpu","vinit"});
        else if ((a & 255) == 0) f.sequence({"cpu","vinit","ainit","vuninit0"});
        else f.sequence({"cpu","vinit","ainit"});
        Fixture t; t.fx.vtattach = v; t.fx.atattach = a;
        CHECK(t.entry.thread_attach_04b4a0() == ((v & 255) != 0 && (a & 255) != 0));
        if ((v & 255) == 0) t.sequence({"vtattach"});
        else if ((a & 255) == 0) t.sequence({"vtattach","atattach","vtdetach"});
        else t.sequence({"vtattach","atattach"});
    }
    for (auto v : words) {
        Fixture f; f.fx.validate = v; f.fx.slot = 0x9090;
        CHECK(f.call(1) == 1);
        CHECK(std::count(f.fx.events.begin(),f.fx.events.end(),"tls") == ((v & 255) != 0));
    }
}
void initializer_failures() {
    for (auto mode : {1u,0x100u,0x80000000u,0xffffffffu}) {
        Fixture f; f.fx.inite = mode;
        CHECK(f.call(1) == 0); CHECK(f.state.startup_state == 1);
        CHECK(f.state.attach_count_bits == 0);
        f.sequence({"cookie","cpu","vinit","ainit","slistinit","stdio","initterm_e"});
        Fixture a; a.fx.argv = mode;
        CHECK(a.call(1) == 0); CHECK(a.state.startup_state == 1);
        CHECK(a.state.attach_count_bits == 0);
        a.sequence({"cookie","cpu","vinit","ainit","slistinit","stdio","initterm_e","argv"});
        Fixture d; d.state.attach_count_bits = 1; d.state.startup_state = 2;
        d.fx.cterm = mode; CHECK(d.call(0) == 1);
        d.sequence({"app0","cterm","slistfree","auninit1","vuninit1","alate","vlate"});
    }
    for (auto initfail : {0u,1u}) {
        Fixture f;
        if (initfail == 0) f.fx.vinit = 0x100; else f.fx.ainit = 0x100;
        CHECK(f.call(1) == 0); CHECK(f.state.startup_state == 0);
        CHECK(f.state.attach_count_bits == 0); CHECK(f.state.onexit_initialized == 0);
    }
}
void application_results() {
    for (auto r : {0u,1u,0x100u,0x80000000u,0xffffffffu}) {
        for (auto reserved : {std::uintptr_t{},std::uintptr_t{0x5678}}) {
            Fixture f; f.fx.app = r; f.fx.reserved = reserved;
            CHECK(f.call(1) == r);
            if (r != 0) {
                CHECK(f.state.startup_state == 2); CHECK(f.state.attach_count_bits == 1);
                CHECK(f.fx.events.back() == "app1");
            } else {
                CHECK(f.state.startup_state == 0); CHECK(f.state.attach_count_bits == 0);
                const auto& events = f.fx.events;
                const auto a = std::find(events.begin(),events.end(),"app1");
                CHECK(a != events.end()); CHECK(*(a+1) == "app0");
                CHECK(*(a+2) == "cterm"); CHECK(events.back() == "vlate");
                CHECK(std::find(events.begin(),events.end(),reserved ? "auninit1" : "auninit0") != events.end());
            }
        }
        Fixture t; t.fx.app = r;
        CHECK(t.call(2) == r); t.sequence({"vtattach","atattach","app2"});
        t.fx.events.clear(); CHECK(t.call(3) == 1); t.sequence({"app3","atdetach","vtdetach"});
    }
}
void counters_and_states() {
    for (auto n : {0u,0x80000000u,0xffffffffu}) {
        Fixture f; f.state.attach_count_bits = n; f.state.startup_state = 42;
        CHECK(!f.entry.process_detach_04bb48(true));
        CHECK(f.state.attach_count_bits == n); f.sequence({});
        CHECK(f.call(0) == 0); f.sequence({});
    }
    for (auto n : {1u,2u,0x7fffffffu}) {
        Fixture f; f.state.attach_count_bits = n; f.state.startup_state = 2;
        CHECK(f.entry.process_detach_04bb48(false));
        CHECK(f.state.attach_count_bits == n-1); CHECK(f.state.startup_state == 0);
        f.sequence({"cterm","cexit","slistfree","auninit0","vuninit0","alate","vlate"});
    }
    for (auto state : {0u,1u,3u,0xffffffffu}) {
        Fixture f; f.state.attach_count_bits = 2; f.state.startup_state = state;
        try { (void)f.entry.process_detach_04bb48(false); CHECK(false); }
        catch (Failure e) { CHECK(e.code == 7); }
        CHECK(f.state.attach_count_bits == 1); CHECK(f.state.startup_state == state);
        f.sequence({"fail7"});
    }
    for (auto state : {1u,2u,3u,0xffffffffu}) {
        Fixture f; f.state.startup_state = state;
        try { (void)f.call(1); CHECK(false); } catch (Failure e) { CHECK(e.code == 7); }
        CHECK(f.state.startup_state == state); CHECK(f.state.attach_count_bits == 0);
        f.sequence({"cookie","cpu","vinit","ainit","fail7"});
    }
    for (auto n : {0u,1u,0x7fffffffu,0x80000000u,0xffffffffu}) {
        Fixture f; f.state.attach_count_bits = n; f.fx.slot = 0x8989; f.fx.tls_count = n;
        CHECK(f.call(1) == 1); CHECK(f.state.attach_count_bits == n + 1u);
    }
}
void tables_and_misc() {
    for (auto mode : {0u,1u}) {
        Fixture f; CHECK(f.entry.initialize_onexit_tables_04b5c0(mode));
        CHECK(f.application.begin == UINT64_MAX); CHECK(f.state.onexit_initialized == 1);
        f.application = {11,22,33}; f.state.quickexit_table = {44,55,66};
        CHECK(f.entry.initialize_onexit_tables_04b5c0(0xffffffffu));
        CHECK(f.application.begin == 11); CHECK(f.application.end == 22);
        CHECK(f.application.capacity_end == 33); CHECK(f.state.quickexit_table.begin == 44);
        CHECK(f.state.quickexit_table.end == 55); CHECK(f.state.quickexit_table.capacity_end == 66);
    }
    for (auto mode : {2u,0x100u,0xffffffffu}) {
        Fixture f;
        try { (void)f.entry.initialize_onexit_tables_04b5c0(mode); CHECK(false); }
        catch (Failure e) { CHECK(e.code == 5); }
        CHECK(f.state.onexit_initialized == 0); CHECK(f.application.begin == 0);
    }
    for (auto flag : {0u,1u,2u,255u}) for (bool fromexit : {false,true})
        for (bool terminating : {false,true}) {
            Fixture f; f.state.initialized_as_dll = static_cast<std::uint8_t>(flag);
            CHECK(f.entry.uninitialize_crt_04b708(terminating,fromexit));
            if (flag == 0 || !fromexit)
                f.sequence(terminating ? std::initializer_list<const char*>{"auninit1","vuninit1"}
                                       : std::initializer_list<const char*>{"auninit0","vuninit0"});
            else f.sequence({});
        }
    for (auto mode : {1u,2u,0xffffffffu}) {
        Fixture f; CHECK(f.entry.initialize_crt_04b584(mode)); CHECK(f.state.initialized_as_dll == 0);
    }
    CHECK(!c::Entry::acquire_startup_lock_04b418());
    c::Entry::release_startup_lock_04b6e4(false); c::Entry::release_startup_lock_04b6e4(true);
    for (auto reason : {4u,255u,0x100u,0x80000000u,0xffffffffu}) {
        Fixture f; f.fx.app = 0x80000100;
        CHECK(f.call(reason) == 0x80000100); CHECK(f.fx.events.size() == 1);
        CHECK(f.fx.events[0] == "app"+std::to_string(reason));
        f.fx.events.clear(); CHECK(f.entry.crt_reason_04b9e0(0,reason,0) == 1); f.sequence({});
    }
}
void tls_and_escapes() {
    Fixture f; f.fx.slot = 0x1010; f.fx.mutate_slot = true; f.fx.replacement = 0x2020;
    CHECK(f.call(1) == 1); CHECK(f.fx.slot == 0x2020);
    CHECK(std::count(f.fx.events.begin(),f.fx.events.end(),"tls") == 1);
    for (auto event : {"cookie","cpu","vinit","ainit","slistinit","stdio","initterm_e","argv","env","initterm","slot","app1"}) {
        Fixture t; t.fx.escape_event = event;
        try { (void)t.call(1); CHECK(false); } catch (Escape) { CHECK(true); }
        CHECK(t.fx.events.back() == event);
        CHECK(std::count(t.fx.events.begin(),t.fx.events.end(),"auninit1") == 0);
        CHECK(std::count(t.fx.events.begin(),t.fx.events.end(),"cexit") == 0);
        if (std::string(event) == "app1") { CHECK(t.state.attach_count_bits == 1); CHECK(t.state.startup_state == 2); }
    }
    // Validation observes the slot address; even a replacement zero is reloaded
    // and passed to the mandatory invocation policy, as the assembly does.
    Fixture zero; zero.fx.slot = 1; zero.fx.mutate_slot = true; zero.fx.replacement = 0;
    CHECK(zero.call(1) == 1); CHECK(zero.fx.slot == 0);
    CHECK(std::count(zero.fx.events.begin(),zero.fx.events.end(),"tls") == 1);
}
}
int main() {
    try {
        success_and_detach(); low_byte_semantics(); initializer_failures();
        application_results(); counters_and_states(); tables_and_misc(); tls_and_escapes();
        std::cout << "PASS " << checks << " recovered CRT entry checks\n";
    } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
