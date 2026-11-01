#include "realloc.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

using namespace anadius::crt_realloc_recovered;
namespace {
unsigned checks = 0;
void check(bool ok) { ++checks; if (!ok) throw std::runtime_error("realloc check " + std::to_string(checks)); }
struct Request { Heap heap; std::uint32_t flags; void* old; std::uint64_t bytes; };
struct Fake final : Effects {
    State& s;
    explicit Fake(State& state) : s(state) { old.fill(std::byte{0x73}); fresh.fill(std::byte{0xa5}); }
    std::array<std::byte,64> old{}, fresh{};
    std::vector<std::string> events;
    std::vector<Request> requests;
    std::vector<std::uint64_t> handler_bytes;
    unsigned failures = 0;
    bool always_fail = false, locked = false, free_ok = true;
    bool mutate_heap = false, clear_mode = false, clear_handler_after_snapshot = false;
    bool throw_alloc = false, throw_handler = false, throw_free = false;
    std::int32_t handler_result = 1;
    std::uint32_t error = 77;
    Heap get_process_heap() override { throw std::logic_error("unexpected heap init"); }
    void* allocate(Heap heap, std::uint32_t flags, void* p, std::uint64_t bytes, bool realloc) {
        events.push_back(realloc ? "realloc" : "alloc"); requests.push_back({heap,flags,p,bytes});
        if (throw_alloc) throw 1;
        if (always_fail) return nullptr;
        if (failures) { --failures; return nullptr; }
        if (realloc && bytes <= fresh.size()) std::copy_n(old.begin(), static_cast<std::size_t>(std::min<std::uint64_t>(bytes, old.size())), fresh.begin());
        return fresh.data();
    }
    void* heap_alloc(Heap h, std::uint32_t f, std::uint64_t n) override { return allocate(h,f,nullptr,n,false); }
    void* heap_realloc(Heap h, std::uint32_t f, void* p, std::uint64_t n) override { return allocate(h,f,p,n,true); }
    bool heap_free(Heap h, std::uint32_t f, void* p) override {
        events.push_back("free"); check(h == s.heap_0a5a10 && f == 0 && p == old.data());
        if (throw_free) throw 3;
        return free_ok;
    }
    std::uint32_t last_error() override { events.push_back("last_error"); return 87; }
    std::uint32_t errno_from_os_error_05d650(std::uint32_t n) override { events.push_back("map"); check(n == 87); return 22; }
    void write_error_slot_05d724(std::uint32_t n) override { events.push_back("errno"); error = n; }
    void lock_0625f8(std::uint32_t i) override { events.push_back("lock"); check(i == 0 && !locked); locked = true; }
    void unlock_06264c(std::uint32_t i) override {
        events.push_back("unlock"); check(i == 0 && locked); locked = false;
        if (clear_handler_after_snapshot) s.encoded_handler_0a5178 = s.cookie_0a1580;
    }
    std::int32_t invoke_new_handler(std::uintptr_t p, std::uint64_t n) override {
        events.push_back("handler"); check(p == 0x9876 && !locked); handler_bytes.push_back(n);
        if (throw_handler) throw 2;
        if (mutate_heap) s.heap_0a5a10 = 0x4455;
        if (clear_mode) s.new_mode_0a5a20 = 0;
        return handler_result;
    }
};
State state(std::uintptr_t handler = 0x9876, std::uint64_t cookie = 17) {
    return {0x1122, 1, cookie, std::rotl(static_cast<std::uint64_t>(handler), static_cast<int>(cookie & 63)) ^ cookie};
}
void boundaries() {
    constexpr std::uint64_t maximum = 0xffffffffffffffe0ULL;
    for (const bool realloc : {false,true}) {
        for (const auto bytes : {0ULL,1ULL,40ULL,64ULL,0x100000000ULL,maximum-1,maximum,maximum+1,UINT64_MAX}) {
            auto s = state(); Fake f(s);
            auto* p = realloc ? realloc_base_06e3a0(f.old.data(),bytes,s,f) : malloc_base_06a308(bytes,s,f);
            if (realloc && bytes == 0) {
                check(!p && f.requests.empty() && f.events == std::vector<std::string>{"free"} && f.error == 77);
            } else if (bytes <= maximum) {
                check(p == f.fresh.data() && f.requests.size() == 1 && f.error == 77);
                check(f.requests[0].heap == 0x1122 && f.requests[0].flags == 0);
                check(f.requests[0].old == (realloc ? f.old.data() : nullptr));
                check(f.requests[0].bytes == (bytes == 0 ? 1 : bytes));
                check(f.events == std::vector<std::string>{realloc ? "realloc" : "alloc"});
                if (!realloc) check(f.fresh[0] == std::byte{0xa5});
            } else check(!p && f.requests.empty() && f.error == 12 && f.events == std::vector<std::string>{"errno"});
            check(std::all_of(f.old.begin(),f.old.end(),[](auto b) { return b == std::byte{0x73}; }));
        }
        for (std::uint64_t delta = 0; delta < 32; ++delta) {
            auto s = state(); Fake f(s);
            const auto n = maximum + delta;
            auto* p = realloc ? realloc_base_06e3a0(f.old.data(),n,s,f) : malloc_base_06a308(n,s,f);
            check((p != nullptr) == (delta == 0));
            check(f.requests.size() == (delta == 0 ? 1U : 0U));
            check(f.error == (delta == 0 ? 77U : 12U));
        }
    }
    for (const auto bytes : {0ULL,1ULL,64ULL,maximum,maximum+1,UINT64_MAX}) {
        auto s = state(); Fake f(s);
        auto* p = realloc_base_06e3a0(nullptr,bytes,s,f);
        check((p != nullptr) == (bytes <= maximum));
        if (bytes <= maximum) check(f.events == std::vector<std::string>{"alloc"} && f.requests[0].bytes == (bytes ? bytes : 1));
        else check(f.events == std::vector<std::string>{"errno"});
    }
}
void retries() {
    for (const bool realloc : {false,true}) {
        for (const auto mode : {0,1,-1,std::numeric_limits<std::int32_t>::min()}) {
            for (const auto result : {0,1,2,0x100,-1,std::numeric_limits<std::int32_t>::min()}) {
                auto s = state(); s.new_mode_0a5a20 = mode; Fake f(s); f.failures = 1; f.handler_result = result;
                auto* p = realloc ? realloc_base_06e3a0(f.old.data(),40,s,f) : malloc_base_06a308(40,s,f);
                const bool retry = mode != 0 && result != 0;
                check((p != nullptr) == retry && f.requests.size() == (retry ? 2U : 1U));
                check(f.handler_bytes.size() == (mode != 0 ? 1U : 0U));
                check(f.error == (retry ? 77U : 12U));
                check(f.events.back() == (retry ? (realloc ? "realloc" : "alloc") : "errno"));
                check(std::none_of(f.events.begin(),f.events.end(),[](auto& event) { return event == "free" || event == "last_error" || event == "map"; }));
            }
        }
        for (const auto cookie : {0ULL,1ULL,17ULL,63ULL,64ULL,UINT64_MAX}) {
            auto s = state(0x9876,cookie); Fake f(s); f.failures = 3; f.mutate_heap = true; f.clear_handler_after_snapshot = true;
            auto* p = realloc ? realloc_base_06e3a0(f.old.data(),40,s,f) : malloc_base_06a308(40,s,f);
            // First decoded callback remains usable after its cache is changed on unlock;
            // second failure sees the now-null callback and terminates.
            check(p == nullptr && f.requests.size() == 2 && f.handler_bytes.size() == 1 && f.error == 12);
            check(f.requests[0].heap == 0x1122 && f.requests[1].heap == 0x4455);
        }
        {
            auto s = state(); Fake f(s); f.failures = 3; f.mutate_heap = true;
            auto* p = realloc ? realloc_base_06e3a0(f.old.data(),40,s,f) : malloc_base_06a308(0,s,f);
            check(p == f.fresh.data() && f.requests.size() == 4 && f.handler_bytes.size() == 3);
            check(f.requests[0].heap == 0x1122 && f.requests[1].heap == 0x4455 && f.error == 77);
            check(std::all_of(f.handler_bytes.begin(),f.handler_bytes.end(),[&](auto n){ return n == (realloc ? 40U : 1U); }));
            if (realloc) check(std::all_of(f.requests.begin(),f.requests.end(),[&](auto& r){ return r.old == f.old.data(); }));
        }
        {
            auto s = state(); Fake f(s); f.always_fail = true; f.clear_mode = true;
            check((realloc ? realloc_base_06e3a0(f.old.data(),40,s,f) : malloc_base_06a308(40,s,f)) == nullptr);
            check(f.requests.size() == 2 && f.handler_bytes.size() == 1 && f.error == 12);
        }
        {
            auto s = state(0); Fake f(s); f.failures = 1;
            check((realloc ? realloc_base_06e3a0(f.old.data(),40,s,f) : malloc_base_06a308(40,s,f)) == nullptr);
            check(f.handler_bytes.empty() && f.events == std::vector<std::string>{realloc ? "realloc" : "alloc","lock","unlock","errno"});
        }
    }
}
void free_and_exceptions() {
    {
        auto s = state(); Fake f(s); f.free_ok = false;
        check(realloc_base_06e3a0(f.old.data(),0,s,f) == nullptr);
        check(f.events == std::vector<std::string>{"free","last_error","map","errno"} && f.error == 22);
    }
    for (const bool realloc : {false,true}) for (const bool handler : {false,true}) {
        auto s = state(); Fake f(s); f.throw_handler = handler; f.throw_alloc = !handler; f.failures = handler ? 1U : 0U;
        try { (void)(realloc ? realloc_base_06e3a0(f.old.data(),40,s,f) : malloc_base_06a308(40,s,f)); check(false); }
        catch (int n) { check(n == (handler ? 2 : 1)); }
        check(f.error == 77 && f.events.back() == (handler ? "handler" : (realloc ? "realloc" : "alloc")));
        check(!f.locked);
    }
    auto s = state(); Fake f(s); f.throw_free = true;
    try { (void)realloc_base_06e3a0(f.old.data(),0,s,f); check(false); } catch (int n) { check(n == 3); }
    check(f.error == 77 && f.events == std::vector<std::string>{"free"});
}
}
int main() {
    try { boundaries(); retries(); free_and_exceptions(); std::cout << "PASS " << checks << " malloc/realloc checks\n"; }
    catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
