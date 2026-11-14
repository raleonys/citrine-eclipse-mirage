// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/hook_native_callbacks.hpp"
#include "laureondo/platform/callback_lock.hpp"
#include "../../support/fixture_ascii.hpp"
#include <array>
#include <cerrno>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <functional>
#include <iostream>
#include <new>
#include <stdexcept>
#include <string>
#include <thread>

namespace {
using namespace laureondo::native;
using laureondo::registry::Handle;
int checks;
void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
Handle identity(HKEY key) { return reinterpret_cast<Handle>(key); }

// These are real SDK calls, without installing detours. Capture their actual
// outgoing error state before the production adapter/model can change it.
struct Observation {
    LSTATUS status{};
    DWORD error{};
    unsigned opens{}, queries{}, closes{};
};
thread_local Observation observed;
LSTATUS WINAPI open_a(HKEY key, LPCSTR name, DWORD options, REGSAM access, PHKEY out) {
    const auto status = RegOpenKeyExA(key, name, options, access, out);
    const auto error = GetLastError();
    observed.status = status; observed.error = error; ++observed.opens;
    return status;
}
LSTATUS WINAPI open_w(HKEY key, LPCWSTR name, DWORD options, REGSAM access, PHKEY out) {
    const auto status = RegOpenKeyExW(key, name, options, access, out);
    const auto error = GetLastError();
    observed.status = status; observed.error = error; ++observed.opens;
    return status;
}
LSTATUS WINAPI query_a(HKEY key, LPCSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size) {
    const auto status = RegQueryValueExA(key, name, reserved, type, data, size);
    const auto error = GetLastError();
    observed.status = status; observed.error = error; ++observed.queries;
    return status;
}
LSTATUS WINAPI query_w(HKEY key, LPCWSTR name, LPDWORD reserved, LPDWORD type, LPBYTE data, LPDWORD size) {
    const auto status = RegQueryValueExW(key, name, reserved, type, data, size);
    const auto error = GetLastError();
    observed.status = status; observed.error = error; ++observed.queries;
    return status;
}
LSTATUS WINAPI close_key(HKEY key) {
    const auto status = RegCloseKey(key);
    const auto error = GetLastError();
    observed.status = status; observed.error = error; ++observed.closes;
    return status;
}
struct Lifecycle { LifecycleFault fault; };
struct Faults final : Errors {
    [[noreturn]] void lifecycle(LifecycleFault fault) override { throw Lifecycle{fault}; }
    [[noreturn]] void callback_exception(Callback, std::exception_ptr) noexcept override { std::_Exit(99); }
};
struct Wide final : WideOperations {
    static std::wstring read(const void* source) {
        std::wstring result;
        for (std::size_t offset = 0;; offset += sizeof(wchar_t)) {
            wchar_t unit;
            std::memcpy(&unit, static_cast<const std::byte*>(source) + offset, sizeof(unit));
            if (!unit) return result;
            result += unit;
        }
    }
    int compare_utf16(const void* a, const void* b) override {
        return _wcsicmp(read(a).c_str(), read(b).c_str());
    }
    int copy_utf16(void* out, std::size_t capacity, const void* source) override {
        const auto value = read(source);
        if (!out || capacity < value.size() + 1) return ERANGE;
        std::memcpy(out, value.c_str(), (value.size() + 1) * sizeof(wchar_t));
        return 0;
    }
};
struct Effects final : OriginalEffects {
    unsigned allocations{}, frees{};
    std::function<void()> startup_wait;
    Effects(const std::array<std::uintptr_t, laureondo::host_init::callback_count>& originals,
            Wide& wide, laureondo::platform::CryptQueryHost& crypt)
        : OriginalEffects(originals, wide, crypt) {}
    std::string filesystem_narrow(std::u16string_view value) override {
        return laureondo::test_support::fixture_ascii(value);
    }
    int crt_stricmp(const char* a, const char* b) override { return _stricmp(a, b); }
    int crt_strcpy_s(char* out, std::size_t capacity, const char* value) override {
        return strcpy_s(out, capacity, value);
    }
    Handle allocate_4_04b0bc() override {
        const auto key = reinterpret_cast<Handle>(::operator new(4));
        ++allocations;
        return key;
    }
    void free_04b0f8(Handle key) override {
        ::operator delete(reinterpret_cast<void*>(key));
        ++frees;
    }
    void wait_036880(std::int32_t) override {
        const auto error = GetLastError();
        { laureondo::callback_lock::Pause pause; if (startup_wait) startup_wait(); }
        SetLastError(error);
    }
    void log_literal_006880(std::uint64_t) override {}
    void log_text_007990(std::string_view) override {}
    void finish_log_0088e0() override {}
    void crypt_observed(laureondo::hooks::Selection) noexcept override {}
    void wait_steps_036880(std::int32_t) override { throw std::logic_error("unexpected network wait"); }
    void log_literal_006880(std::string_view) override {}
    void log_port_00ad70(std::uint16_t) override {}
    void log_finish_0088e0() override {}
    void dialog_038bc0(std::string_view, std::string_view, std::uint32_t) override { throw std::logic_error("unexpected UI"); }
    [[noreturn]] void exit_05cdc0(std::uint32_t) override { throw std::logic_error("unexpected exit"); }
};
struct Fixture {
    laureondo::registry::Values values{{"ClientKey", u"Client"}, {"ClientKey2", u"MissingClient"},
        {"ClientValue", u"ClientPath"}, {"ClientPath", u"C:\\overlay.exe"},
        {"LanguageKey", u"Language"}, {"LanguageValue", u"Locale"}, {"Language", u"fr_FR"}};
    laureondo::registry::Handles clients, languages, fabricated;
    std::uint8_t language_enabled = 1;
    std::int32_t v4 = 16, v6 = 28;
    std::uint16_t port = 6099;
    std::uint32_t attempts = 0;
    std::array<std::uintptr_t, laureondo::host_init::callback_count> originals{
        0, reinterpret_cast<std::uintptr_t>(&open_a), reinterpret_cast<std::uintptr_t>(&open_w),
        reinterpret_cast<std::uintptr_t>(&close_key), reinterpret_cast<std::uintptr_t>(&query_a),
        reinterpret_cast<std::uintptr_t>(&query_w)};
    Wide wide;
    laureondo::platform::WindowsCryptQueryHost crypt;
    Effects effects{originals, wide, crypt};
    Faults faults;
    Context context{{values, clients, languages, fabricated, language_enabled},
        {v4, v6, port, attempts}, effects, effects, effects, effects};
    Binding binding{context, faults};
};
struct Key {
    HKEY value{};
    bool callback = false;
    Key() = default;
    Key(const Key&) = delete;
    ~Key() { if (value) { if (callback) reg_close_entry(value); else RegCloseKey(value); } }
    LSTATUS close() {
        const auto status = callback ? reg_close_entry(value) : RegCloseKey(value);
        if (status == ERROR_SUCCESS) value = nullptr;
        return status;
    }
};
struct Tree {
    std::wstring path;
    Key root;
    Tree() {
        static unsigned sequence;
        path = L"Software\\LaureondoEclipseMirage.RegistryTests." + std::to_wstring(GetCurrentProcessId())
            + L"." + std::to_wstring(GetTickCount64()) + L"." + std::to_wstring(++sequence);
        DWORD disposition = 0;
        require(RegCreateKeyExW(HKEY_CURRENT_USER, path.c_str(), 0, nullptr, REG_OPTION_VOLATILE,
            KEY_ALL_ACCESS, nullptr, &root.value, &disposition) == ERROR_SUCCESS, "create isolated volatile HKCU tree");
        require(disposition == REG_CREATED_NEW_KEY, "test tree must be new");
        for (const auto name : {L"Client", L"Language", L"Other"}) {
            Key child;
            require(RegCreateKeyExW(root.value, name, 0, nullptr, REG_OPTION_VOLATILE,
                KEY_ALL_ACCESS, nullptr, &child.value, nullptr) == ERROR_SUCCESS, "create native test child");
            const wchar_t text[] = L"native-value";
            for (const auto value : {L"ClientPath", L"Locale", L"NativeOnly"})
                require(RegSetValueExW(child.value, value, 0, REG_SZ, reinterpret_cast<const BYTE*>(text),
                    sizeof(text)) == ERROR_SUCCESS, "seed native Registry value");
        }
    }
    ~Tree() {
        root.close();
        RegDeleteTreeW(HKEY_CURRENT_USER, path.c_str());
        RegDeleteKeyW(HKEY_CURRENT_USER, path.c_str());
    }
};
struct Result {
    LSTATUS status{};
    DWORD error{}, type = 0, size = 256;
    std::array<BYTE, 256> data{};
    Observation original;
};
Result query(HKEY key, bool wide, bool overlay_name = true) {
    Result result;
    SetLastError(0x7351);
    result.status = wide
        ? reg_query_w_entry(key, overlay_name ? L"ClientPath" : L"NativeOnly", nullptr,
            &result.type, result.data.data(), &result.size)
        : reg_query_a_entry(key, overlay_name ? "ClientPath" : "NativeOnly", nullptr,
            &result.type, result.data.data(), &result.size);
    result.error = GetLastError();
    result.original = observed;
    return result;
}
bool text_is(const Result& result, bool wide, const char* narrow, const wchar_t* wide_text) {
    const auto size = wide ? (std::wcslen(wide_text) + 1) * sizeof(wchar_t) : std::strlen(narrow) + 1;
    const void* expected = wide ? static_cast<const void*>(wide_text) : static_cast<const void*>(narrow);
    return result.status == ERROR_SUCCESS && result.type == REG_SZ && result.size == size
        && std::memcmp(result.data.data(), expected, size) == 0;
}
LSTATUS open(HKEY root, bool wide, const char* narrow, const wchar_t* name, Key& key) {
    key.callback = true;
    return wide ? reg_open_w_entry(root, name, 0, KEY_READ, &key.value)
        : reg_open_a_entry(root, narrow, 0, KEY_READ, &key.value);
}
void check_clean(const Fixture& fixture) {
    require(fixture.clients.empty() && fixture.languages.empty() && fixture.fabricated.empty(), "all Registry tracking drained");
    require(active_calls() == 0, "all callback leases drained");
}
void lifecycle(bool wide, bool fabricated) {
    Tree tree;
    Fixture fixture;
    Key key;
    SetLastError(0x7352);
    require(open(tree.root.value, wide, fabricated ? "MissingClient" : "Client",
        fabricated ? L"MissingClient" : L"Client", key) == ERROR_SUCCESS, "matched native or fabricated open");
    require(GetLastError() == observed.error, "open preserves actual original LastError");
    require((observed.status != ERROR_SUCCESS) == fabricated, "fabrication follows real missing-key failure only");
    require(fixture.clients.contains(identity(key.value)) && fixture.fabricated.contains(identity(key.value)) == fabricated,
        "opened key registered with expected ownership");
    const auto replacement = query(key.value, wide);
    require(text_is(replacement, wide, "C:\\overlay.exe", L"C:\\overlay.exe"), "A/W ClientPath overlay bytes, type and size");
    require(replacement.error == replacement.original.error, "overlay preserves actual original query LastError");
    require((replacement.original.status != ERROR_SUCCESS) == fabricated, "query reached real SDK on native or fabricated identity");
    const auto forwarded = query(key.value, wide, false);
    require(fabricated ? forwarded.status == ERROR_FILE_NOT_FOUND
        : text_is(forwarded, wide, "native-value", L"native-value"), "unmatched value follows native/fabricated policy");
    require(forwarded.error == forwarded.original.error, "unmatched query preserves original LastError");
    const auto calls = observed.closes;
    const auto stale = key.value;
    require(key.close() == ERROR_SUCCESS, "close matched key");
    const auto close_error = GetLastError();
    require(observed.closes == calls + (fabricated ? 0 : 1), "fabricated close never reaches SDK; native close reaches it once");
    if (!fabricated) {
        require(close_error == observed.error, "native close preserves original LastError");
        const auto after_close = query(stale, wide);
        require(after_close.status == after_close.original.status && after_close.status != ERROR_SUCCESS,
            "query after real close gets native failure without stale overlay");
    }
    require(fixture.effects.allocations == static_cast<unsigned>(fabricated)
        && fixture.effects.frees == fixture.effects.allocations, "real operator-new/delete ownership balanced");
    check_clean(fixture);
}
void language_and_foreign(bool wide) {
    Tree tree;
    Fixture fixture;
    Key language;
    require(open(tree.root.value, wide, "Language", L"Language", language) == ERROR_SUCCESS, "open native language key");
    DWORD type = 0, size = 256;
    std::array<BYTE, 256> data{};
    const auto status = wide
        ? reg_query_w_entry(language.value, L"Locale", nullptr, &type, data.data(), &size)
        : reg_query_a_entry(language.value, "Locale", nullptr, &type, data.data(), &size);
    require(status == ERROR_SUCCESS && type == REG_SZ, "native language key overlay");
    require(wide ? size == sizeof(L"fr_FR") && std::memcmp(data.data(), L"fr_FR", size) == 0
        : size == sizeof("fr_FR") && std::memcmp(data.data(), "fr_FR", size) == 0, "language overlay exact A/W bytes");
    require(language.close() == ERROR_SUCCESS, "close language key");
    Key foreign;
    require(RegOpenKeyExW(tree.root.value, L"Client", 0, KEY_READ, &foreign.value) == ERROR_SUCCESS, "open key through unhooked SDK route");
    foreign.callback = true;
    require(text_is(query(foreign.value, wide), wide, "native-value", L"native-value"), "untracked real HKEY receives native value");
    require(foreign.close() == ERROR_SUCCESS, "forward close of untracked real HKEY");
    check_clean(fixture);
}
void concurrent_close(bool wide, bool fabricated) {
    Tree tree;
    Fixture fixture;
    Key key;
    require(open(tree.root.value, wide, fabricated ? "MissingClient" : "Client",
        fabricated ? L"MissingClient" : L"Client", key) == ERROR_SUCCESS, "open concurrent test key");
    const auto old_identity = key.value;
    std::mutex mutex;
    std::condition_variable cv;
    bool waiting = false, release = false, peer_done = false;
    fixture.effects.startup_wait = [&] {
        std::unique_lock lock(mutex);
        waiting = true; cv.notify_all();
        cv.wait(lock, [&] { return release; });
        SetLastError(0x9988);
    };
    Result pending, peer_query;
    LSTATUS closed = -1, opened = -1;
    Key peer;
    std::thread query_thread([&] { pending = query(old_identity, wide); });
    bool entered;
    { std::unique_lock lock(mutex); entered = cv.wait_for(lock, std::chrono::seconds(5), [&] { return waiting; }); }
    bool busy = false;
    if (entered) {
        try { fixture.binding.close(); }
        catch (const Lifecycle& failure) { busy = failure.fault == LifecycleFault::Busy; }
    }
    std::thread close_thread([&] {
        closed = key.close();
        opened = open(tree.root.value, wide, "Other", L"Other", peer);
        if (opened == ERROR_SUCCESS) peer_query = query(peer.value, wide);
        { std::lock_guard lock(mutex); peer_done = true; cv.notify_all(); }
    });
    bool progressed;
    { std::unique_lock lock(mutex);
        progressed = cv.wait_for(lock, std::chrono::seconds(5), [&] { return peer_done; });
        release = true; cv.notify_all();
    }
    query_thread.join(); close_thread.join();
    require(entered && busy, "paused query retains binding lease and rejects unbinding");
    require(progressed && closed == ERROR_SUCCESS && opened == ERROR_SUCCESS, "same-key close and peer open progress during startup wait");
    require(text_is(pending, wide, "C:\\overlay.exe", L"C:\\overlay.exe") && pending.error == pending.original.error,
        "already admitted query completes its selected overlay and preserves original LastError");
    require(text_is(peer_query, wide, "native-value", L"native-value"), "replacement key never receives old routing");
    require(text_is(query(peer.value, wide), wide, "native-value", L"native-value"), "later replacement query remains native");
    require(peer.close() == ERROR_SUCCESS, "close replacement key");
    require(fixture.effects.frees == fixture.effects.allocations, "concurrent fabricated allocation freed exactly once");
    check_clean(fixture);
}
void alternate_sdk_routes(bool wide) {
    // D03 bounded SDK-origin evidence. This executable calls entrypoints
    // directly: it does not install MinHook or observe Consumer call paths.
    Tree tree;
    Fixture fixture;
    {
        Key legacy;
        const auto result = wide
            ? RegOpenKeyW(tree.root.value, L"Client", &legacy.value)
            : RegOpenKeyA(tree.root.value, "Client", &legacy.value);
        require(result == ERROR_SUCCESS && legacy.value,
            "legacy RegOpenKey A/W returns a real native HKEY");
        legacy.callback = true;
        require(fixture.clients.empty() && fixture.languages.empty() && fixture.fabricated.empty(),
            "direct legacy SDK open does not claim a callback-owned identity");
        const auto reply = query(legacy.value, wide, false);
        require(reply.original.status == ERROR_SUCCESS &&
            text_is(reply, wide, "native-value", L"native-value"),
            "SDK-origin legacy key query forwards native data, not an overlay");
        require(reply.error == reply.original.error,
            "legacy SDK query preserves the original LastError");
        const auto before_close = observed.closes;
        require(legacy.close() == ERROR_SUCCESS && observed.closes == before_close + 1,
            "legacy SDK key close forwards to native RegCloseKey once");
        check_clean(fixture);
    }
    {
        Key created;
        const wchar_t* name = wide ? L"D03CreatedW" : L"D03CreatedA";
        DWORD disposition = 0;
        const auto result = wide
            ? RegCreateKeyExW(tree.root.value, name, 0, nullptr, REG_OPTION_VOLATILE,
                KEY_ALL_ACCESS, nullptr, &created.value, &disposition)
            : RegCreateKeyExA(tree.root.value, "D03CreatedA", 0, nullptr, REG_OPTION_VOLATILE,
                KEY_ALL_ACCESS, nullptr, &created.value, &disposition);
        require(result == ERROR_SUCCESS && created.value && disposition == REG_CREATED_NEW_KEY,
            "alternate RegCreateKeyEx A/W returns a new native HKEY");
        created.callback = true;
        constexpr wchar_t value[] = L"alternate-native";
        require(RegSetValueExW(created.value, L"NativeOnly", 0, REG_SZ,
            reinterpret_cast<const BYTE*>(value), sizeof(value)) == ERROR_SUCCESS,
            "seed a native value through a direct SDK write");
        require(fixture.clients.empty() && fixture.languages.empty() && fixture.fabricated.empty(),
            "alternate SDK creation never silently registers a fabricated key");
        const auto reply = query(created.value, wide, false);
        require(reply.original.status == ERROR_SUCCESS &&
            text_is(reply, wide, "alternate-native", L"alternate-native"),
            "SDK-created native value reaches original RegQueryValueEx A/W");
        require(reply.error == reply.original.error,
            "SDK-created query preserves original LastError");
        const auto before_close = observed.closes;
        require(created.close() == ERROR_SUCCESS && observed.closes == before_close + 1,
            "SDK-created native HKEY close forwards exactly once");
        require(RegDeleteKeyW(tree.root.value, name) == ERROR_SUCCESS,
            "remove the SDK-created volatile child");
        check_clean(fixture);
    }
    require(fixture.effects.allocations == 0 && fixture.effects.frees == 0,
        "alternate native SDK routes never allocate or release fabricated handles");
    std::cout << "PASS: alternate SDK Registry routes " << (wide ? "W" : "A") << '\n';
}

void numeric_reuse(bool wide) {
    Tree tree;
    Fixture fixture;
    bool reused = false;
    unsigned attempts = 0;
    for (; attempts < 256 && !reused; ++attempts) {
        Key tracked, untracked;
        require(open(tree.root.value, wide, "Client", L"Client", tracked) == ERROR_SUCCESS, "open tracked reuse probe");
        const auto retired = tracked.value;
        require(tracked.close() == ERROR_SUCCESS, "close tracked reuse probe");
        require(open(tree.root.value, wide, "Other", L"Other", untracked) == ERROR_SUCCESS, "open untracked reuse probe");
        reused = untracked.value == retired;
        require(text_is(query(untracked.value, wide), wide, "native-value", L"native-value"), "native reuse probe has no stale overlay");
        require(untracked.close() == ERROR_SUCCESS, "close untracked reuse probe");
        check_clean(fixture);
    }
    // No reuse is an inconclusive observation, never evidence of alias freedom.
    std::cout << "SDK numeric reuse " << (wide ? "W" : "A") << ": "
        << (reused ? "OBSERVED" : "NOT_OBSERVED") << " attempts=" << attempts << '\n';
}
}
int main() {
    try {
        static_assert(sizeof(wchar_t) == 2 && sizeof(HKEY) == sizeof(Handle));
        for (const bool wide : {false, true}) {
            for (const bool fabricated : {false, true}) {
                lifecycle(wide, fabricated);
                concurrent_close(wide, fabricated);
            }
            language_and_foreign(wide);
            alternate_sdk_routes(wide);
            numeric_reuse(wide);
        }
        std::cout << checks << " native Registry lifecycle checks passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
