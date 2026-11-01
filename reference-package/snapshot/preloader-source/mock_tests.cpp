// Runs reconstructed C++ only. Does not load, execute, import, or hook a target DLL.
#include "preloader_core.hpp"
#include <array>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
using namespace recovered_preloader;
struct Trapped { TrapSite site; };
static int checks;
#define CHECK(x) do { ++checks; if (!(x)) throw std::runtime_error(#x); } while (0)

struct MockHost : Host {
    std::array<unsigned char, 512> image{};
    BOOL disable_result = 1;
    DWORD pid = 37;
    int disable_calls = 0, errors = 0, pid_calls = 0, image_calls = 0, locks = 0;
    std::atomic<int> waits{0}, wakes{0};
    bool null_image = false, fail_alloc = false, fail_realloc = false;
    void* last_mutex{};
    std::map<void*, std::size_t> allocations;
    std::vector<std::size_t> alloc_sizes, realloc_sizes;
    std::vector<unsigned char> released_bytes;
    int releases = 0;
    MockHost() {
        const std::uint32_t pe = 0x80, size = static_cast<std::uint32_t>(image.size());
        std::memcpy(image.data() + 0x3c, &pe, 4);
        std::memcpy(image.data() + pe + 0x50, &size, 4);
    }
    ~MockHost() { for (auto& [p, n] : allocations) { (void)n; std::free(p); } }
    BOOL disable_thread_library_calls(HMODULE) override { ++disable_calls; return disable_result; }
    DWORD get_last_error() override { ++errors; return 7; }
    DWORD current_process_id() override { ++pid_calls; return pid; }
    HMODULE main_module() override {
        ++image_calls;
        return null_image ? nullptr : reinterpret_cast<HMODULE>(image.data());
    }
    int mutex_lock(void* p) override { ++locks; last_mutex = p; return 123; }
    BOOL wait32(std::atomic<std::uint32_t>& state, std::uint32_t comparand) override {
        ++waits; state.wait(comparand); return 1;
    }
    void wake_all(std::atomic<std::uint32_t>& state) override { ++wakes; state.notify_all(); }
    void* allocate(std::size_t n) override {
        alloc_sizes.push_back(n);
        if (fail_alloc) return nullptr;
        void* p = std::malloc(n); if (p) allocations[p] = n; return p;
    }
    void* reallocate(void* p, std::size_t n) override {
        realloc_sizes.push_back(n);
        if (fail_realloc) return nullptr;
        void* next = std::realloc(p, n);
        if (next) { allocations.erase(p); allocations[next] = n; }
        return next;
    }
    void release(void* p) override {
        ++releases;
        const auto n = allocations.at(p);
        released_bytes.assign(static_cast<unsigned char*>(p), static_cast<unsigned char*>(p) + n);
        allocations.erase(p); std::free(p);
    }
    [[noreturn]] void invalid_instruction(TrapSite site) override { throw Trapped{site}; }
};

struct MockEngine : HookEngine {
    int initialized = 0, enabled = 0, status = 0, enable_status = 0;
    struct Creation { void* target; void* detour; };
    std::vector<Creation> creations;
    std::array<int,3> create_status{};
    std::array<void*,3> originals{};
    int initialize() override { ++initialized; return status; }
    int create(void* target, void* detour, void** output) override {
        const auto index = creations.size(); creations.push_back({target, detour});
        *output = originals.at(index); return create_status.at(index);
    }
    int enable(void* target) override { CHECK(target == nullptr); ++enabled; return enable_status; }
};
static BOOL process_result;
static int process_calls, crypt_calls, try_calls;
static HANDLE process_snapshot;
static PROCESSENTRY32W* process_output;
static BOOL WINAPI original_next(HANDLE snapshot, PROCESSENTRY32W* entry) {
    ++process_calls; process_snapshot = snapshot; process_output = entry; return process_result;
}
static int __cdecl original_try(void*) { ++try_calls; return 91; }
struct CryptSeen {
    DWORD type, expected_content, expected_format, flags;
    const void* object;
    DWORD* encoding; DWORD* content; DWORD* format;
    HCERTSTORE* store; HCRYPTMSG* message; const void** context;
} crypt_seen{};
static BOOL WINAPI original_query(DWORD type, const void* object, DWORD ec, DWORD ef,
    DWORD flags, DWORD* encoding, DWORD* content, DWORD* format, HCERTSTORE* store,
    HCRYPTMSG* message, const void** context) {
    ++crypt_calls;
    crypt_seen = {type, ec, ef, flags, object, encoding, content, format, store, message, context};
    return 19;
}
struct Fixture {
    MockHost host; MockEngine engine; Core core;
    Fixture() : core(host, engine, {}) {
        core.original_process_next = original_next;
        core.original_trylock = original_try;
        core.original_crypt = original_query;
    }
};
template<class F> static void expect_trap(TrapSite site, F&& action) {
    try { action(); } catch (const Trapped& trap) { CHECK(trap.site == site); return; }
    CHECK(false);
}
static void test_process() {
    Fixture f; PROCESSENTRY32W entry{};
    const int calls_before = process_calls;
    const auto snap = reinterpret_cast<HANDLE>(17);
    process_result = 9; entry.th32ProcessID = f.host.pid; entry.th32ParentProcessID = 4;
    CHECK(f.core.process_next(snap, &entry) == 9);
    CHECK(entry.th32ParentProcessID == f.host.pid);
    CHECK(process_snapshot == snap && process_output == &entry);
    entry.th32ProcessID = 600; entry.th32ParentProcessID = 4;
    CHECK(f.core.process_next(snap, &entry) == 9 && entry.th32ParentProcessID == 4);
    process_result = 0; entry.th32ProcessID = f.host.pid;
    CHECK(f.core.process_next(snap, &entry) == 0 && entry.th32ParentProcessID == 4);
    const auto pid_calls = f.host.pid_calls;
    process_result = 7; CHECK(f.core.process_next(snap, nullptr) == 7);
    CHECK(f.host.pid_calls == pid_calls);
    f.core.original_process_next = nullptr;
    expect_trap(TrapSite::MissingOriginal, [&] { f.core.process_next(snap, &entry); });
    CHECK(process_calls == calls_before + 4);
}
static BOOL call_crypt(Core& c, DWORD type, const void* object) {
    static DWORD a, b, d; static HCERTSTORE s; static HCRYPTMSG m; static const void* ctx;
    const auto result = c.crypt_query(type, object, 2, 3, 4, &a, &b, &d, &s, &m, &ctx);
    CHECK(crypt_seen.type == type && crypt_seen.expected_content == 2 &&
          crypt_seen.expected_format == 3 && crypt_seen.flags == 4);
    CHECK(crypt_seen.encoding == &a && crypt_seen.content == &b && crypt_seen.format == &d);
    CHECK(crypt_seen.store == &s && crypt_seen.message == &m && crypt_seen.context == &ctx);
    return result;
}
static bool replaced() {
    return crypt_seen.object && std::wcscmp(static_cast<const wchar_t*>(crypt_seen.object),
                                          L"EAAntiCheat.GameServiceLauncher.dll") == 0;
}
static void test_paths() {
    const std::vector<std::wstring> yes = {
        L"preloader_l.dll", L"C:\\bin\\preloader_l.dll", L"xpreloader_l.dll",
        L"\u00e9\\preloader_l.dll", L"\u0130\u4e2d\\preloader_l.dll",
        std::wstring(1, wchar_t(0xd83d)) + wchar_t(0xde00) + L"preloader_l.dll"
    };
    for (const auto& path : yes) {
        Fixture f; CHECK(call_crypt(f.core, 1, path.c_str()) == 19); CHECK(replaced());
        CHECK(f.host.alloc_sizes.at(0) == path.size()); CHECK(f.host.releases == 1);
    }
    for (const auto& path : std::vector<std::wstring>{L"", L"Preloader_l.dll", L"preloader_l.dllx", L"x.dll"}) {
        Fixture f; CHECK(call_crypt(f.core, 1, path.c_str()) == 19);
        CHECK(crypt_seen.object == path.c_str());
        CHECK(f.host.releases == (path.empty() ? 0 : 1));
    }
    { Fixture f; CHECK(call_crypt(f.core, 1, nullptr) == 19 && replaced()); CHECK(f.host.alloc_sizes.empty()); }
    { Fixture f; const std::uint64_t blob = 0xffff; CHECK(call_crypt(f.core, 2, &blob) == 19);
      CHECK(crypt_seen.object == &blob && f.host.alloc_sizes.empty()); }
    { Fixture f; CHECK(call_crypt(f.core, 0, nullptr) == 19); CHECK(crypt_seen.object == nullptr); }
    // Exact UTF-8 conversion, including valid surrogate-pair combination.
    { Fixture f; const wchar_t path[] = {0x00e9, 0xd83d, 0xde00, 'x', 0};
      call_crypt(f.core, 1, path);
      const std::array<unsigned char,7> expected{0xc3,0xa9,0xf0,0x9f,0x98,0x80,'x'};
      CHECK(std::memcmp(f.host.released_bytes.data(), expected.data(), expected.size()) == 0);
      CHECK(f.host.alloc_sizes == std::vector<std::size_t>{4});
      CHECK(f.host.realloc_sizes == std::vector<std::size_t>{8}); }
    // Unpaired surrogates trap only after complete conversion. Native UD2 does
    // not run destructors/free; mocks retain allocations for fixture cleanup.
    for (const auto& path : std::vector<std::wstring>{
            std::wstring(1,wchar_t(0xd800)), std::wstring(1,wchar_t(0xdc00)),
            std::wstring(1,wchar_t(0xd800)) + L"xpreloader_l.dll",
            std::wstring(1,wchar_t(0xd800)) + wchar_t(0xd800) + wchar_t(0xdc00)}) {
        Fixture f; const int before = crypt_calls;
        expect_trap(TrapSite::InvalidUtf16, [&] { call_crypt(f.core, 1, path.c_str()); });
        CHECK(crypt_calls == before && f.host.releases == 0);
        CHECK(!f.host.allocations.empty());
    }
    { Fixture f; f.host.fail_alloc = true;
      expect_trap(TrapSite::Allocation, [&] { call_crypt(f.core, 1, L"x"); }); }
    { Fixture f; f.host.fail_realloc = true;
      expect_trap(TrapSite::Allocation, [&] { call_crypt(f.core, 1, L"\u4e2d"); }); }
    { Fixture f; f.core.original_crypt = nullptr;
      expect_trap(TrapSite::MissingOriginal, [&] { call_crypt(f.core, 1, L"preloader_l.dll"); });
      CHECK(f.host.releases == 1); }
}
static void test_mutex() {
    Fixture f; void* inside = f.host.image.data() + 256;
    const int calls_before = try_calls;
    CHECK(f.core.mutex_trylock(inside) == 91);
    CHECK(f.host.image_calls == 1 && f.host.locks == 1 && f.host.last_mutex == inside);
    CHECK(f.core.mutex_trylock(inside) == 91);
    CHECK(f.host.image_calls == 1 && f.host.locks == 1);
    CHECK(try_calls == calls_before + 2);
    { Fixture b; CHECK(b.core.mutex_trylock(b.host.image.data() + b.host.image.size()) == 91);
      CHECK(b.host.locks == 0); CHECK(b.core.mutex_trylock(b.host.image.data()) == 91);
      CHECK(b.host.locks == 1); }
    { Fixture b; b.core.image_once.state = Once::Poisoned;
      expect_trap(TrapSite::PoisonedImageRange, [&] { b.core.mutex_trylock(inside); }); }
    { Fixture b; b.core.mutex_once.state = Once::Poisoned;
      CHECK(b.core.mutex_trylock(b.host.image.data()) == 91); CHECK(b.host.locks == 1); }
    { Fixture b; b.host.null_image = true;
      expect_trap(TrapSite::NullMainImage, [&] { b.core.mutex_trylock(inside); }); CHECK(b.host.errors == 1); }
    { Fixture b; b.core.original_trylock = nullptr;
      expect_trap(TrapSite::MissingOriginal, [&] { b.core.mutex_trylock(b.host.image.data()); });
      CHECK(b.host.locks == 1); }
}
static void test_attach() {
    Fixture f; f.host.disable_result = 0; f.engine.enable_status = 10;
    f.engine.originals = {reinterpret_cast<void*>(original_next), reinterpret_cast<void*>(original_try),
                          reinterpret_cast<void*>(original_query)};
    f.engine.create_status = {0, 8, 0};
    CHECK(f.core.dll_main(nullptr, DLL_PROCESS_ATTACH, nullptr) == 1);
    CHECK(f.engine.initialized == 1 && f.engine.enabled == 1 && f.engine.creations.size() == 3);
    CHECK(f.host.disable_calls == 1 && f.host.errors == 1);
    CHECK(f.core.original_process_next == original_next && f.core.original_trylock == original_try);
    CHECK(f.core.original_crypt == original_query);
    for (DWORD reason : {DWORD(DLL_PROCESS_DETACH), DWORD(DLL_THREAD_ATTACH), DWORD(DLL_THREAD_DETACH)})
        CHECK(f.core.dll_main(nullptr, reason, nullptr) == 1);
    CHECK(f.engine.creations.size() == 3 && f.engine.enabled == 1);
    { Fixture b; b.engine.status = 1; CHECK(b.core.dll_main(nullptr, 1, nullptr) == 1); }
    { Fixture b; b.engine.status = -2; CHECK(b.core.dll_main(nullptr, 1, nullptr) == 1); }
    { Fixture b; b.engine.status = 9;
      expect_trap(TrapSite::EngineInitialization, [&] { b.core.dll_main(nullptr, 1, nullptr); });
      CHECK(b.engine.creations.empty() && b.engine.enabled == 0); }
    { Fixture b; b.core.engine_once.state = Once::Poisoned;
      expect_trap(TrapSite::PoisonedOnce, [&] { b.core.dll_main(nullptr, 1, nullptr); }); }
}
static void test_once() {
    MockHost host; Once once; int calls = 0;
    auto increment = [](void* p, OnceCall&) { ++*static_cast<int*>(p); };
    once.call(host, false, &calls, increment); once.call(host, false, &calls, increment);
    CHECK(calls == 1 && once.state == Once::Complete);
    once.state = Once::Poisoned;
    expect_trap(TrapSite::PoisonedOnce, [&] { once.call(host, false, &calls, increment); });
    bool poison_seen = false;
    once.call(host, true, &poison_seen, [](void* p, OnceCall& c) { *static_cast<bool*>(p) = c.was_poisoned; });
    CHECK(poison_seen && once.state == Once::Complete);
    once.state = Once::Incomplete;
    once.call(host, false, nullptr, [](void*, OnceCall& c) { c.next_state = Once::Poisoned; });
    CHECK(once.state == Once::Poisoned);
    // Recovered completion exchanges caller-chosen next_state and wakes queued
    // waiters. Exercise real C++ concurrency using mock atomic wait/notify only.
    Once shared; std::atomic<int> runs{0}; std::vector<std::thread> threads;
    for (int i=0; i<8; ++i) threads.emplace_back([&] {
        shared.call(host, false, &runs, [](void* p, OnceCall&) {
            ++*static_cast<std::atomic<int>*>(p);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));
        });
    });
    for (auto& thread : threads) thread.join();
    CHECK(runs == 1 && shared.state == Once::Complete);
    CHECK(host.waits > 0 && host.wakes == 1);
    Once interrupted;
    try { interrupted.call(host, false, nullptr, [](void*, OnceCall&) { throw 5; }); } catch (int) {}
    CHECK(interrupted.state == Once::Running); // no invented unwind poison setter.
}
int main() {
    try { test_process(); test_paths(); test_mutex(); test_attach(); test_once(); }
    catch (const std::exception& e) { std::cerr << "FAILED: " << e.what() << '\n'; return 1; }
    catch (const Trapped& e) { std::cerr << "UNEXPECTED MOCK TRAP " << int(e.site) << '\n'; return 2; }
    std::cout << "PASS: " << checks << " checks; mocks only; no target DLL loaded or hooks installed\n";
}
