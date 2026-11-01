// Direct SDK function-pointer calls with synthetic data only; no target loading.
#include "native_shims.hpp"
#include <array>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
using namespace recovered_preloader;
namespace shim = recovered_preloader::native;
static int checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) throw std::runtime_error(#x); } while (0)
struct Rejection { shim::Failure reason; };
struct ErrorPolicy final : shim::Errors {
    bool fatal_reject = false;
    [[noreturn]] void reject(shim::Failure why) override {
        if (fatal_reject) std::_Exit(70 + static_cast<int>(why));
        throw Rejection{why};
    }
    [[noreturn]] void escaped_exception(std::exception_ptr value) noexcept override {
        // This synthetic policy terminates our harness child, never returns a
        // fabricated BOOL/int to an SDK caller. Check that the exception exists.
        if (!value) std::_Exit(95);
        try { std::rethrow_exception(value); }
        catch (const Rejection&) { std::_Exit(93); }
        catch (const std::runtime_error&) { std::_Exit(93); }
        catch (...) { std::_Exit(95); }
    }
};
static ErrorPolicy errors;
struct MockHost final : Host {
    std::array<unsigned char, 512> image{};
    DWORD pid = 71;
    int locks = 0, disabled = 0;
    MockHost() {
        const std::uint32_t offset = 0x80, size = 512;
        std::memcpy(image.data() + 0x3c, &offset, 4);
        std::memcpy(image.data() + offset + 0x50, &size, 4);
    }
    BOOL disable_thread_library_calls(HMODULE) override { ++disabled; return 1; }
    DWORD get_last_error() override { return 123; }
    DWORD current_process_id() override { return pid; }
    HMODULE main_module() override { return reinterpret_cast<HMODULE>(image.data()); }
    int mutex_lock(void*) override { ++locks; return 812; }
    BOOL wait32(std::atomic<std::uint32_t>& state, std::uint32_t compare) override {
        state.wait(compare); return 1;
    }
    void wake_all(std::atomic<std::uint32_t>& state) override { state.notify_all(); }
    void* allocate(std::size_t n) override { return std::malloc(n); }
    void* reallocate(void* p, std::size_t n) override { return std::realloc(p, n); }
    void release(void* p) override { std::free(p); }
    [[noreturn]] void invalid_instruction(TrapSite) override { throw std::runtime_error("mock trap"); }
};
static std::atomic<bool> block_next{false}, entered_next{false}, release_next{false};
static BOOL next_result = 17;
static HANDLE seen_snapshot{};
static LPPROCESSENTRY32W seen_entry{};
static BOOL WINAPI mock_next(HANDLE snapshot, LPPROCESSENTRY32W entry) {
    seen_snapshot = snapshot; seen_entry = entry;
    if (block_next.load()) {
        entered_next = true; entered_next.notify_all();
        release_next.wait(false);
    }
    return next_result;
}
static BOOL WINAPI throwing_next(HANDLE, LPPROCESSENTRY32W) { throw std::runtime_error("original mock"); }
static void* seen_mutex{};
static int __cdecl mock_mutex(void* mutex) { seen_mutex = mutex; return -127; }
struct QuerySeen {
    DWORD type, content_flags, format_flags, flags;
    const void* object;
    DWORD *encoding, *content, *format;
    HCERTSTORE* store;
    HCRYPTMSG* message;
    const void** context;
};
static QuerySeen query{};
static BOOL WINAPI mock_query(DWORD type, const void* object, DWORD cf, DWORD ff,
    DWORD flags, DWORD* encoding, DWORD* content, DWORD* format, HCERTSTORE* store,
    HCRYPTMSG* message, const void** context) {
    query = {type, cf, ff, flags, object, encoding, content, format, store, message, context};
    if (encoding) *encoding = 101;
    if (content) *content = 102;
    if (format) *format = 103;
    return -29;
}
struct MockEngine final : HookEngine {
    int initialized = 0, enables = 0;
    std::vector<void*> targets, detours;
    int initialize() override { ++initialized; return 0; }
    int create(void* target, void* detour, void** result) override {
        targets.push_back(target); detours.push_back(detour);
        if (targets.size() == 1) *result = reinterpret_cast<void*>(&mock_next);
        else if (targets.size() == 2) *result = reinterpret_cast<void*>(&mock_mutex);
        else *result = reinterpret_cast<void*>(&mock_query);
        return 0;
    }
    int enable(void* target) override { if (target) throw std::runtime_error("expected all"); ++enables; return 0; }
};
struct Fixture {
    MockHost host;
    MockEngine engine;
    Core core;
    Fixture() : core(host, engine, shim::with_detours(&mock_next, &mock_mutex, &mock_query)) {
        core.original_process_next = &mock_next;
        core.original_trylock = &mock_mutex;
        core.original_crypt = &mock_query;
    }
};
template<class F> void rejects(shim::Failure why, F&& call) {
    try { call(); }
    catch (const Rejection& r) { CHECK(r.reason == why); return; }
    CHECK(false);
}
static void process_and_lifetime() {
    Fixture first, second;
    second.host.pid = 91;
    const auto b = shim::with_detours(&mock_next, &mock_mutex, &mock_query);
    CHECK(b.process_next_target == &mock_next && b.process_next_detour == &shim::process_next);
    CHECK(b.trylock_target == &mock_mutex && b.trylock_detour == &shim::mutex_trylock);
    CHECK(b.crypt_target == &mock_query && b.crypt_detour == &shim::crypt_query);
    ProcessNext callback = &shim::process_next;
    const auto snapshot = reinterpret_cast<HANDLE>(std::uintptr_t{0x1234});
    {
        shim::Binding binding(first.core, errors);
        shim::configure_errors(errors); // same policy is idempotent.
        rejects(shim::Failure::AlreadyBound, [&] { shim::Binding other(second.core, errors); });
        rejects(shim::Failure::AlreadyBound, [&] { shim::Binding other(first.core, errors); });
        ErrorPolicy different;
        rejects(shim::Failure::DifferentErrors, [&] { shim::configure_errors(different); });
        PROCESSENTRY32W entry{};
        entry.th32ProcessID = first.host.pid; entry.th32ParentProcessID = 15;
        CHECK(callback(snapshot, &entry) == 17);
        CHECK(entry.th32ParentProcessID == 71 && seen_snapshot == snapshot && seen_entry == &entry);
        entry.th32ProcessID = 92; entry.th32ParentProcessID = 15;
        CHECK(callback(snapshot, &entry) == 17 && entry.th32ParentProcessID == 15);
        entry.th32ProcessID = 71; next_result = 0;
        CHECK(callback(snapshot, &entry) == 0 && entry.th32ParentProcessID == 15);
        next_result = -13;
        CHECK(callback(snapshot, nullptr) == -13 && seen_entry == nullptr);
        BOOL threaded_result{};
        entry.th32ParentProcessID = 15;
        std::thread thread([&] { threaded_result = callback(snapshot, &entry); });
        thread.join();
        CHECK(threaded_result == -13 && entry.th32ParentProcessID == 71);
        binding.close(); binding.close();
    }
    { // Closed old context's destructor cannot clear the newer binding.
        auto old = std::make_unique<shim::Binding>(first.core, errors);
        old->close();
        shim::Binding replacement(second.core, errors);
        old.reset();
        PROCESSENTRY32W entry{}; entry.th32ProcessID = 91;
        CHECK(callback(snapshot, &entry) == -13 && entry.th32ParentProcessID == 91);
    }
    next_result = 17;
}
static void constructor_race() {
    Fixture f;
    std::atomic<int> ready{0}, winners{0}, losers{0};
    std::atomic<bool> start{false}, finish{false};
    auto compete = [&] {
        ++ready; ready.notify_all(); start.wait(false);
        try {
            shim::Binding binding(f.core, errors);
            ++winners; winners.notify_all(); finish.wait(false);
        } catch (const Rejection& r) {
            if (r.reason == shim::Failure::AlreadyBound) { ++losers; losers.notify_all(); }
            else std::terminate();
        }
    };
    std::thread one(compete), two(compete);
    for (;;) { const int seen = ready.load(); if (seen == 2) break; ready.wait(seen); }
    start = true; start.notify_all();
    winners.wait(0); losers.wait(0);
    CHECK(winners == 1 && losers == 1);
    CHECK(shim::process_next(nullptr, nullptr) == 17);
    finish = true; finish.notify_all(); one.join(); two.join();
    shim::Binding after(f.core, errors);
    CHECK(shim::process_next(nullptr, nullptr) == 17);
}
static void concurrent_close() {
    Fixture f;
    shim::Binding binding(f.core, errors);
    block_next = true; entered_next = false; release_next = false;
    PROCESSENTRY32W entry{}; entry.th32ProcessID = 71;
    BOOL result{};
    std::thread worker([&] { ProcessNext callback = &shim::process_next; result = callback(nullptr, &entry); });
    entered_next.wait(false);
    rejects(shim::Failure::CallsInFlight, [&] { binding.close(); });
    rejects(shim::Failure::AlreadyBound, [&] { shim::Binding replacement(f.core, errors); });
    release_next = true; release_next.notify_all();
    worker.join(); block_next = false;
    CHECK(result == 17 && entry.th32ParentProcessID == 71);
    CHECK(shim::process_next(nullptr, nullptr) == 17); // failed close kept binding.
    binding.close();
}
static void query_and_mutex() {
    Fixture f;
    shim::Binding binding(f.core, errors);
    CryptQuery callback = &shim::crypt_query;
    DWORD encoding{}, content{}, format{};
    HCERTSTORE store{}; HCRYPTMSG message{}; const void* context{};
    const std::array<unsigned char, 3> blob{1, 2, 3};
    CHECK(callback(2, blob.data(), 0x111, 0x222, 0x333, &encoding, &content,
                   &format, &store, &message, &context) == -29);
    CHECK(query.type == 2 && query.object == blob.data());
    CHECK(query.content_flags == 0x111 && query.format_flags == 0x222 && query.flags == 0x333);
    CHECK(query.encoding == &encoding && query.content == &content && query.format == &format);
    CHECK(query.store == &store && query.message == &message && query.context == &context);
    CHECK(encoding == 101 && content == 102 && format == 103);
    const wchar_t path[] = L"synthetic/preloader_l.dll";
    CHECK(callback(1, path, 4, 5, 6, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == -29);
    CHECK(std::wstring(static_cast<const wchar_t*>(query.object)) == L"EAAntiCheat.GameServiceLauncher.dll");
    CHECK(query.type == 1 && query.content_flags == 4 && query.format_flags == 5 && query.flags == 6);
    CHECK(!query.encoding && !query.content && !query.format && !query.store && !query.message && !query.context);
    CHECK(callback(1, nullptr, 0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == -29);
    CHECK(std::wstring(static_cast<const wchar_t*>(query.object)) == L"EAAntiCheat.GameServiceLauncher.dll");
    const wchar_t unchanged[] = L"Synthetic.DLL";
    CHECK(callback(1, unchanged, 0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr) == -29);
    CHECK(query.object == unchanged);
    MutexRoutine mutex = &shim::mutex_trylock;
    auto* inside = f.host.image.data() + 16;
    CHECK(mutex(inside) == -127 && seen_mutex == inside && f.host.locks == 1);
    CHECK(mutex(inside) == -127 && f.host.locks == 1);
    auto* end = f.host.image.data() + f.host.image.size();
    CHECK(mutex(end) == -127 && seen_mutex == end && f.host.locks == 1);
}
static void application_entry() {
    Fixture f;
    shim::Binding binding(f.core, errors);
    shim::ApplicationDllMain callback = &shim::application_dll_main;
    const auto module = reinterpret_cast<HMODULE>(std::uintptr_t{0x42});
    CHECK(callback(module, DLL_THREAD_ATTACH, reinterpret_cast<void*>(1)) == 1);
    CHECK(f.host.disabled == 0 && f.engine.initialized == 0);
    CHECK(callback(module, DLL_PROCESS_ATTACH, reinterpret_cast<void*>(2)) == 1);
    CHECK(f.host.disabled == 1 && f.engine.initialized == 1 && f.engine.enables == 1);
    CHECK(f.engine.targets.size() == 3 && f.engine.detours.size() == 3);
    CHECK(f.engine.targets[0] == reinterpret_cast<void*>(&mock_next));
    CHECK(f.engine.targets[1] == reinterpret_cast<void*>(&mock_mutex));
    CHECK(f.engine.targets[2] == reinterpret_cast<void*>(&mock_query));
    CHECK(f.engine.detours[0] == reinterpret_cast<void*>(&shim::process_next));
    CHECK(f.engine.detours[1] == reinterpret_cast<void*>(&shim::mutex_trylock));
    CHECK(f.engine.detours[2] == reinterpret_cast<void*>(&shim::crypt_query));
    CHECK(f.core.original_process_next == &mock_next && f.core.original_trylock == &mock_mutex &&
          f.core.original_crypt == &mock_query);
    CHECK(callback(module, DLL_PROCESS_DETACH, nullptr) == 1);
    CHECK(f.engine.enables == 1);
}
int main(int argc, char** argv) {
    try {
        if (argc == 2) {
            const std::string mode = argv[1];
            if (mode == "--unconfigured") {
                std::set_terminate([] { std::_Exit(94); });
                (void)shim::process_next(nullptr, nullptr);
            }
            shim::configure_errors(errors);
            errors.fatal_reject = true;
            if (mode == "--unbound-next") (void)shim::process_next(nullptr, nullptr);
            if (mode == "--unbound-mutex") (void)shim::mutex_trylock(nullptr);
            if (mode == "--unbound-query") (void)shim::crypt_query(2, nullptr, 0, 0, 0, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
            if (mode == "--unbound-entry") (void)shim::application_dll_main(nullptr, 0, nullptr);
            if (mode == "--escaped") {
                Fixture f; shim::Binding binding(f.core, errors);
                f.core.original_process_next = &throwing_next;
                (void)shim::process_next(nullptr, nullptr);
            }
            if (mode == "--rebind") {
                Fixture f; shim::Binding first(f.core, errors), second(f.core, errors);
            }
            if (mode == "--closed") {
                Fixture f; shim::Binding binding(f.core, errors); binding.close();
                (void)shim::process_next(nullptr, nullptr);
            }
            return 96; // mode unexpectedly returned or was misspelled.
        }
        shim::configure_errors(errors);
        process_and_lifetime(); constructor_race(); concurrent_close(); query_and_mutex(); application_entry();
        std::cout << checks << " preloader native shim mock checks passed\n";
        return 0;
    } catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    catch (...) { std::cerr << "unexpected test failure\n"; return 2; }
}
