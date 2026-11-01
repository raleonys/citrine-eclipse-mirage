#include "win32_effects.hpp"
#include <array>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace tn = anadius::thread_native;
namespace tr = anadius::threads_recovered;
namespace {
unsigned checks;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("check " + std::to_string(checks)); }
struct Exit { DWORD code; bool module; };
struct Fixture {
    std::vector<std::string> trace;
    BOOL module_result = TRUE, close_result = TRUE, release_result = TRUE;
    std::uintptr_t module = 0x1234567887654321ULL;
    HANDLE handle = reinterpret_cast<HANDLE>(0x8877665544332211ULL);
    DWORD error = 0xfedcba98, id = 0x81234567;
    DWORD flags = 0; SIZE_T stack = 0;
    void* security = nullptr; void* payload = nullptr;
    LPTHREAD_START_ROUTINE start = nullptr;
    bool start_inline = false;
    std::uintptr_t address = 0, last_handle = 0;
    DWORD terminal_code = 0;
};
Fixture* fixture;
BOOL WINAPI module_api(DWORD flags, LPCWSTR address, HMODULE* out) {
    fixture->trace.emplace_back("module"); fixture->flags = flags;
    fixture->address = reinterpret_cast<std::uintptr_t>(address);
    *out = reinterpret_cast<HMODULE>(fixture->module); return fixture->module_result;
}
HANDLE WINAPI create_api(LPSECURITY_ATTRIBUTES security, SIZE_T stack,
    LPTHREAD_START_ROUTINE start, LPVOID payload, DWORD flags, LPDWORD id) {
    fixture->trace.emplace_back("create"); fixture->security = security;
    fixture->stack = stack; fixture->start = start; fixture->payload = payload;
    fixture->flags = flags;
    if (id) *id = fixture->id;
    if (fixture->start_inline && fixture->handle) {
        try { (void)start(payload); check(false); }
        catch (const Exit& end) { fixture->terminal_code = end.code; }
    }
    return fixture->handle;
}
DWORD WINAPI error_api() { fixture->trace.emplace_back("error"); return fixture->error; }
BOOL WINAPI close_api(HANDLE handle) {
    fixture->trace.emplace_back("close"); fixture->last_handle = reinterpret_cast<std::uintptr_t>(handle);
    return fixture->close_result;
}
BOOL WINAPI release_api(HMODULE handle) {
    fixture->trace.emplace_back("release"); fixture->last_handle = reinterpret_cast<std::uintptr_t>(handle);
    return fixture->release_result;
}
DECLSPEC_NORETURN void WINAPI release_exit_api(HMODULE handle, DWORD code) {
    fixture->trace.emplace_back("release_exit"); fixture->last_handle = reinterpret_cast<std::uintptr_t>(handle);
    throw Exit{code, true};
}
DECLSPEC_NORETURN void WINAPI exit_api(DWORD code) {
    fixture->trace.emplace_back("exit");
    throw Exit{code, false};
}
tn::Api api() { return {module_api, create_api, error_api, close_api, release_api, release_exit_api, exit_api}; }
class Effects;
tr::Effects* running;
DWORD WINAPI wrapper(void* context);
class Effects final : public tn::Win32Effects {
public:
    explicit Effects(tn::Api methods = api(), LPTHREAD_START_ROUTINE start = wrapper)
        : Win32Effects(start, methods) {}
    std::unique_ptr<tr::Context> owned;
    tr::Context* slot = nullptr;
    bool allocation_fails = false, invalid_returns = true;
    std::uint32_t mapped = 0, errno_value = 0, runtime_result = 0;
    int policy = 2;
    unsigned initialize_calls = 0, uninitialize_calls = 0, free_calls = 0;
    tr::Context* calloc_context_066848() override {
        fixture->trace.emplace_back("calloc");
        if (allocation_fails) return nullptr;
        owned = std::make_unique<tr::Context>(); return owned.get();
    }
    void free_0668c0(tr::Context* value) override {
        fixture->trace.emplace_back(value ? "free" : "free_null"); ++free_calls;
        if (value) { check(value == owned.get()); owned.reset(); }
    }
    void write_error_slot_05d724(std::uint32_t value) override { errno_value = value; fixture->trace.emplace_back("errno"); }
    void invalid_parameter_05c954() override {
        fixture->trace.emplace_back("invalid"); if (!invalid_returns) throw std::runtime_error("invalid");
    }
    void map_os_error_05d698(std::uint32_t value) override { mapped = value; fixture->trace.emplace_back("map"); }
    tr::Context*& required_thread_slot_064b90() override { fixture->trace.emplace_back("required_ptd"); return slot; }
    tr::Context* optional_thread_context_064d08() override { fixture->trace.emplace_back("optional_ptd"); return slot; }
    int runtime_policy_06c0d0() override { return policy; }
    int initialize_runtime_067320(int kind) override { check(kind == 1); ++initialize_calls; return static_cast<int>(runtime_result); }
    void uninitialize_runtime_06736c() override { ++uninitialize_calls; }
    std::uint32_t invoke_entry(std::uintptr_t entry, std::uintptr_t argument) override {
        fixture->trace.emplace_back("application");
        using Entry = unsigned (__stdcall*)(void*);
        return reinterpret_cast<Entry>(entry)(reinterpret_cast<void*>(argument));
    }
};
DWORD WINAPI wrapper(void* context) { tr::thread_entry_05e7a8(static_cast<tr::Context*>(context), *running); }
unsigned __stdcall application(void* payload) {
    check(payload == reinterpret_cast<void*>(0x345678)); return 0xf1234567;
}
void test_forwarding() {
    Fixture f; fixture = &f; Effects e;
    for (const auto value : std::array<std::uintptr_t, 5>{0, 1, 0xffffffffffffffffULL, 0x8000000000000000ULL, 0x1234567887654321ULL}) {
        f.module = value;
        for (BOOL result : {FALSE, TRUE, static_cast<BOOL>(-1)}) {
            f.module_result = result; tr::Handle out = 0xaabb;
            check(e.get_module_handle_ex_w(4, 0xabcdef01fedcba09ULL, &out) == (result != 0));
            check(out == value); check(f.flags == 4); check(f.address == 0xabcdef01fedcba09ULL);
            f.close_result = result; check(e.close_handle(value) == (result != 0)); check(f.last_handle == value);
            f.release_result = result; check(e.free_library(value) == (result != 0)); check(f.last_handle == value);
        }
    }
    check(e.last_error() == f.error);
    tr::Context context{}; std::uint32_t id = 0x33333333;
    auto* security = reinterpret_cast<void*>(0x2222);
    check(e.create_thread(security, 0x100000123ULL, &context, 0x80000004, &id) == reinterpret_cast<tr::Handle>(f.handle));
    check(f.security == security); check(f.stack == 0x100000123ULL);
    check(f.payload == &context); check(f.start == wrapper); check(f.flags == 0x80000004); check(id == f.id);
    check(e.create_thread(nullptr, 0, &context, 0, nullptr) == reinterpret_cast<tr::Handle>(f.handle));
    f.handle = nullptr; id = 0x44444444;
    check(e.create_thread(nullptr, 0, &context, 0, &id) == 0); check(id == 0x44444444);
    // Seven missing function pointers and a missing wrapper fail at composition.
    for (unsigned missing = 0; missing < 8; ++missing) {
        auto broken = api(); auto start = wrapper;
        switch (missing) {
        case 0: broken.get_module = nullptr; break; case 1: broken.create = nullptr; break;
        case 2: broken.last_error = nullptr; break; case 3: broken.close = nullptr; break;
        case 4: broken.release = nullptr; break; case 5: broken.release_and_exit = nullptr; break;
        case 6: broken.exit = nullptr; break; default: start = nullptr;
        }
        try { Effects bad(broken, start); check(false); } catch (const std::invalid_argument&) { check(true); }
    }
    const auto native = tn::native_api();
    check(native.create == &::CreateThread); check(native.get_module == &::GetModuleHandleExW);
    check(native.exit == &::ExitThread); // Imported native calls are never invoked.
}
void test_core_integration() {
    for (int policy : {0, 1, 2, 3}) for (std::uint32_t runtime_result : {0U, 1U, 0x80000000U}) {
        Fixture f; fixture = &f; f.start_inline = true; Effects e; running = &e;
        e.policy = policy; e.runtime_result = runtime_result;
        std::uint32_t id = 0;
        const auto result = tr::begin_thread_05e8e0(reinterpret_cast<void*>(0x1234), 0xffffffffU,
            reinterpret_cast<std::uintptr_t>(application), 0x345678, 4, &id, e);
        check(result == reinterpret_cast<tr::Handle>(f.handle)); check(id == f.id);
        check(f.start == wrapper && reinterpret_cast<std::uintptr_t>(f.start) != reinterpret_cast<std::uintptr_t>(application));
        check(f.stack == 0xffffffffULL); check(f.payload == e.owned.get());
        check(e.slot == e.owned.get()); check(e.slot->handle == 0);
        check(e.initialize_calls == static_cast<unsigned>(policy == 2));
        check(e.uninitialize_calls == static_cast<unsigned>(policy == 2 && runtime_result == 0));
        check(f.last_handle == f.module); check(f.terminal_code == 0xf1234567);
        check(e.free_calls == 1); // Only the observed free(nullptr), no invented PTD destruction.
        check(f.trace.front() == "calloc" && f.trace.back() == "release_exit");
    }
    Fixture f; fixture = &f; Effects e; running = &e; f.handle = nullptr;
    std::uint32_t id = 0x77777777;
    check(tr::begin_thread_05e8e0(nullptr, 0, reinterpret_cast<std::uintptr_t>(application), 0x345678, 0, &id, e) == 0);
    check(id == 0x77777777); check(e.mapped == f.error); check(!e.owned);
    check(f.trace == std::vector<std::string>({"calloc", "free_null", "module", "create", "error", "map", "release", "free"}));
    f.trace.clear(); check(tr::begin_thread_05e8e0(nullptr, 0, 0, 0, 0, nullptr, e) == 0);
    check(e.errno_value == 22); check(f.trace == std::vector<std::string>({"errno", "invalid"}));
    e.allocation_fails = true; f.trace.clear();
    check(tr::begin_thread_05e8e0(nullptr, 0, 1, 0, 0, nullptr, e) == 0);
    check(f.trace == std::vector<std::string>({"calloc", "free_null"}));
    try { (void)wrapper(nullptr); check(false); } catch (const Exit& end) { check(!end.module && end.code == f.error); }
}
void test_terminal_paths() {
    Fixture f; fixture = &f; Effects e;
    for (DWORD code : {0UL, 1UL, 0xffffffffUL, 0x80000000UL}) {
        try { e.exit_thread(code); } catch (const Exit& end) { check(!end.module && end.code == code); }
        try { e.free_library_and_exit_thread(0xffffffffffffffffULL, code); }
        catch (const Exit& end) { check(end.module && end.code == code); check(f.last_handle == 0xffffffffffffffffULL); }
    }
}
struct RuntimeProvider final : anadius::crt_runtime_recovered::Effects {
    unsigned initialize_calls = 0, uninitialize_calls = 0, policy_calls = 0;
    anadius::crt_runtime_recovered::Module load_library_ex_w(std::u16string_view name,
        anadius::crt_runtime_recovered::Module file, std::uint32_t flags) override {
        check(file == 0 && flags == 0x800);
        check(name == u"api-ms-win-core-winrt-l1-1-0" || name == u"api-ms-win-appmodel-runtime-l1-1-2");
        return 0x7777;
    }
    std::uint32_t last_error() override { check(false); return 0; }
    bool free_library(anadius::crt_runtime_recovered::Module) override { check(false); return false; }
    std::uintptr_t get_proc_address(std::uintptr_t module, std::string_view name) override {
        check(module == 0x7777);
        if (name == "RoInitialize") return 0x101;
        if (name == "RoUninitialize") return 0x102;
        check(name == "AppPolicyGetThreadInitializationType"); return 0x103;
    }
    std::uint32_t process_parameters_flags() override { return 0; }
    std::int32_t call_ro_initialize(std::uintptr_t address, std::uint32_t mode) override {
        check(address == 0x101 && mode == 1); ++initialize_calls; return 0;
    }
    void call_ro_uninitialize(std::uintptr_t address) override { check(address == 0x102); ++uninitialize_calls; }
    std::uint32_t call_thread_initialization_policy(std::uintptr_t address,
        std::uintptr_t token, std::int32_t* result) override {
        check(address == 0x103 && token == std::uintptr_t(-6));
        ++policy_calls; *result = 1; return 0;
    }
};
struct ErrorProvider final : anadius::crt_errors_recovered::Effects {
    std::array<std::byte, 0x28> ptd{};
    unsigned lookups = 0;
    void* optional_ptd_064d08() override { ++lookups; return ptd.data(); }
    std::uint32_t at(std::size_t offset) { return anadius::crt_errors_recovered::Cell32(ptd.data() + offset).load(); }
};
struct Combined final : tn::RecoveredWin32Effects {
    Combined(anadius::crt_runtime_recovered::State& state, RuntimeProvider& runtime,
        anadius::crt_errors_recovered::State& errors, ErrorProvider& ptd)
        : RecoveredWin32Effects(wrapper, api(), state, runtime, errors, ptd) {}
    std::unique_ptr<tr::Context> owned;
    tr::Context* slot = nullptr;
    bool invalid_called = false;
    tr::Context* calloc_context_066848() override { owned = std::make_unique<tr::Context>(); return owned.get(); }
    void free_0668c0(tr::Context* value) override { if (value) { check(value == owned.get()); owned.reset(); } }
    void invalid_parameter_05c954() override { invalid_called = true; }
    tr::Context*& required_thread_slot_064b90() override { return slot; }
    tr::Context* optional_thread_context_064d08() override { return slot; }
    std::uint32_t invoke_entry(std::uintptr_t entry, std::uintptr_t payload) override {
        using Entry = unsigned (__stdcall *)(void*);
        return reinterpret_cast<Entry>(entry)(reinterpret_cast<void*>(payload));
    }
};
void test_recovered_composition() {
    Fixture f; fixture = &f; f.start_inline = true;
    anadius::crt_runtime_recovered::State runtime_state(0x1234567890abcdefULL);
    RuntimeProvider runtime;
    anadius::crt_errors_recovered::State error_state; ErrorProvider ptd;
    Combined e(runtime_state, runtime, error_state, ptd); running = &e;
    std::uint32_t id = 0;
    const auto entry = reinterpret_cast<std::uintptr_t>(application);
    check(tr::begin_thread_05e8e0(nullptr, 0, entry, 0x345678, 0, &id, e) != 0);
    check(id == f.id && f.terminal_code == 0xf1234567);
    check(runtime.policy_calls == 1 && runtime.initialize_calls == 1 && runtime.uninitialize_calls == 1);
    check(e.slot->runtime_initialized == 1 && runtime_state.thread_policy == 2);
    // Repeat uses both recovered procedure caches and the recovered policy cache.
    check(tr::begin_thread_05e8e0(nullptr, 0, entry, 0x345678, 0, nullptr, e) != 0);
    check(runtime.policy_calls == 1 && runtime.initialize_calls == 2 && runtime.uninitialize_calls == 2);
    check(ptd.lookups == 0);
    e.slot = nullptr; f.handle = nullptr; f.error = 5;
    check(tr::begin_thread_05e8e0(nullptr, 0, entry, 0x345678, 0, &id, e) == 0);
    check(ptd.lookups == 2 && ptd.at(0x24) == 5 && ptd.at(0x20) == 13);
    check(!e.owned);
    check(tr::begin_thread_05e8e0(nullptr, 0, 0, 0, 0, nullptr, e) == 0);
    check(e.invalid_called && ptd.lookups == 3);
    check(ptd.at(0x20) == 22 && ptd.at(0x24) == 5);
    check(error_state.errno_fallback == 12 && error_state.dos_fallback == 8);
}
}
int main() {
    try { test_forwarding(); test_core_integration(); test_terminal_paths(); test_recovered_composition(); std::cout << "PASS " << checks << " synthetic Win32 thread bridge checks\n"; }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
