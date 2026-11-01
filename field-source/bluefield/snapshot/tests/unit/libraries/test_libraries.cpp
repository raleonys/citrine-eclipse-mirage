// SPDX-License-Identifier: GPL-3.0-only
#include "src/libraries/libraries.hpp"
#include <algorithm>
#include <cstdio>
#include <deque>
#include <functional>
#include <stdexcept>
#include <tuple>

using namespace preloader_l::libraries;
static unsigned checks;
static void check(bool value) {
    ++checks;
    if (!value) throw std::runtime_error("check failed: " + std::to_string(checks));
}
struct ConversionFailure { std::uint32_t code; };
struct LogFailure {};

struct FakeSystem final : SystemCalls {
    struct Reply { int count; std::uint32_t error; bool used_default = false; };
    struct Call { std::uint32_t page, flags; int count, capacity; bool has_output, has_default; };
    std::vector<std::string>& trace;
    std::vector<Call> conversions;
    std::deque<Reply> replies;
    std::deque<Module> modules;
    std::vector<std::wstring> paths;
    std::vector<Module> frees;
    std::function<void(Module)> on_free;
    std::uint32_t mode = 0x8000, error = 0;
    explicit FakeSystem(std::vector<std::string>& events) : trace(events) {}
    std::uint32_t get_error_mode() override { trace.push_back("get-mode"); return mode; }
    std::uint32_t set_error_mode(std::uint32_t value) override {
        trace.push_back("mode:" + std::to_string(value));
        auto previous = mode; mode = value; return previous;
    }
    Module load_library(const wchar_t* path) override {
        trace.push_back("load"); paths.emplace_back(path);
        check(!modules.empty()); auto result = modules.front(); modules.pop_front(); return result;
    }
    bool free_library(Module module) override {
        trace.push_back("free:" + std::to_string(module)); frees.push_back(module);
        if (on_free) on_free(module);
        return false;
    }
    int wide_to_narrow(std::uint32_t page, std::uint32_t flags, const wchar_t* input,
        int length, char* output, int capacity, bool* used_default) override {
        trace.push_back("convert");
        conversions.push_back({page, flags, length, capacity, output != nullptr, used_default != nullptr});
        if (!replies.empty()) {
            const auto reply = replies.front(); replies.pop_front(); error = reply.error;
            if (used_default) *used_default = reply.used_default;
            if (output && reply.count > 0) std::fill_n(output, std::min(capacity, reply.count), 'x');
            return reply.count;
        }
        // Fixture codec, deliberately only handles the ASCII paths used below;
        // Win32 UTF behavior belongs to the separately compiled native adapter.
        check(page == 65001 && flags == 0x80);
        if (output) {
            check(capacity >= length);
            for (int i = 0; i < length; ++i) {
                check(input[i] <= 127); output[i] = static_cast<char>(input[i]);
            }
        }
        return length;
    }
    std::uint32_t last_error() override { trace.push_back("last-error"); return error; }
};
struct FakeEffects final : Effects {
    std::vector<std::string>& trace;
    std::vector<std::string> logs;
    std::vector<std::tuple<std::string,std::string,std::uint32_t>> messages;
    bool fail_log = false;
    explicit FakeEffects(std::vector<std::string>& events) : trace(events) {}
    void begin_load_log_0863e0() override { trace.push_back("log-begin"); if (fail_log) throw LogFailure{}; }
    void append_log_path_and_flush(std::string_view value) override {
        trace.push_back("log-end"); logs.emplace_back(value);
    }
    void message_038bc0(std::string title, std::string text, std::uint32_t flags) override {
        trace.push_back("dialog"); messages.emplace_back(title,text,flags);
    }
    [[noreturn]] void throw_runtime_00cd70(int code) override { throw ConversionFailure{static_cast<std::uint32_t>(code)}; }
    [[noreturn]] void throw_filesystem_00cdb0(std::uint32_t code) override { throw ConversionFailure{code}; }
    void wait_steps_036880(int count) override { check(count == 2); trace.push_back("wait:2"); }
    void finish_flag_036820() override { trace.push_back("finished"); }
};

static void test_conversion() {
    for (auto page : {65001u,54936u,1252u}) {
        std::vector<std::string> trace; FakeSystem os(trace);
        os.replies.push_back({4,0,false});
        auto r = convert_wide_048b30(os,page,L"test",4,nullptr,0);
        check(r.count == 4 && r.error == 0);
        check(os.conversions.size() == 1);
        const auto& c = os.conversions[0];
        check(c.page == page && c.count == 4 && c.capacity == 0 && !c.has_output);
        check(c.flags == (page == 1252 ? 0x400u : 0x80u));
        check(c.has_default == (page == 1252));
        check(trace.size() == 1); // successful API conversion must not query LastError
    }
    {
        std::vector<std::string> trace; FakeSystem os(trace);
        os.replies = {{7,55,true}};
        auto r = convert_wide_048b30(os,1252,L"test",4,nullptr,0);
        check(r.count == 0 && r.error == 0x459);
        check(trace == std::vector<std::string>{"convert"});
    }
    for (auto fallback_error : {0u,0x57u}) {
        std::vector<std::string> trace; FakeSystem os(trace);
        os.replies = {{0,0x3ec,false},{fallback_error == 0 ? 4 : 0,fallback_error,false}};
        char output[4]{};
        auto r = convert_wide_048b30(os,65001,L"test",4,output,4);
        check(r.error == fallback_error && r.count == (fallback_error == 0 ? 4 : 0));
        check(os.conversions.size() == 2);
        check(os.conversions[1].flags == 0 && !os.conversions[1].has_default);
        check(os.conversions[1].has_output && os.conversions[1].capacity == 4);
    }
    {
        std::vector<std::string> trace; FakeSystem os(trace);
        os.replies = {{0,0x57,false}};
        auto r = convert_wide_048b30(os,65001,L"test",4,nullptr,0);
        check(r.count == 0 && r.error == 0x57 && os.conversions.size() == 1);
    }
}

static void test_list() {
    {
        std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
        auto handles = load_list_035260(os,effects,{});
        check(handles.empty()); check(os.mode == 0x8000);
        check(trace == std::vector<std::string>{"get-mode","mode:32769","mode:32768"});
    }
    {
        std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
        os.modules = {10,0,20,10};
        std::vector<std::wstring> paths{L"a.dll",L"b.dll",L"c.dll",L"a.dll"};
        auto handles = load_list_035260(os,effects,paths);
        check(handles == std::vector<Module>{10,20,10}); // duplicates retained
        check(os.paths == paths);
        check(effects.logs == std::vector<std::string>{"a.dll","b.dll","c.dll","a.dll"});
        check(effects.messages.size() == 1);
        check(effects.messages[0] == std::tuple<std::string,std::string,std::uint32_t>{"error","Can't load b.dll",0x10});
        check(os.conversions.size() == 10); // failed path reconverted after load
        check(os.mode == 0x8000 && trace.back() == "mode:32768");
        auto dialog = std::find(trace.begin(),trace.end(),"dialog");
        check(dialog != trace.end() && dialog[-1] == "convert");
        check(std::count(trace.begin(),trace.end(),"load") == 4);
    }
    {
        std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
        os.modules = {0}; std::vector<std::wstring> paths{L""};
        check(load_list_035260(os,effects,paths).empty());
        check(os.conversions.empty() && os.paths == paths);
        check(std::get<1>(effects.messages[0]) == "Can't load ");
    }
    {
        std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
        os.modules = {42}; std::vector<std::wstring> paths{std::wstring(L"a\0hidden",8)};
        check(load_list_035260(os,effects,paths) == std::vector<Module>{42});
        check(os.paths[0] == L"a"); // API sees c_str terminator, log uses full length
        check(effects.logs[0].size() == 8 && effects.logs[0][1] == '\0');
        check(os.conversions[0].count == 8);
    }
    for (int fail_at : {1,2,3,4}) {
        std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
        // Fail first sizing/writing conversion or second sizing/writing on load failure.
        for (int i = 1; i < fail_at; ++i) os.replies.push_back({5,0,false});
        os.replies.push_back({0,1113,false});
        os.modules = {0}; std::vector<std::wstring> paths{L"x.dll"};
        bool threw = false;
        try { (void)load_list_035260(os,effects,paths); }
        catch (const ConversionFailure& failure) { threw = true; check(failure.code == 1113); }
        check(threw && os.mode == 0x8001); // no synthesized RAII restoration
        check(effects.messages.empty());
        check(os.paths.size() == (fail_at > 2 ? 1u : 0u));
    }
    {
        std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
        effects.fail_log = true; std::vector<std::wstring> paths{L"x.dll"};
        bool threw = false;
        try { (void)load_list_035260(os,effects,paths); } catch (LogFailure&) { threw = true; }
        check(threw && os.mode == 0x8001 && os.paths.empty() && os.conversions.empty());
    }
}

static void test_phases_and_unload() {
    std::vector<std::string> trace; FakeSystem os(trace); FakeEffects effects(trace);
    State state{{L"m.dll"},{L"w.dll"},{99}};
    os.modules = {10,20};
    load_phase_035850(os,effects,state,true);
    check(state.loaded == std::vector<Module>{99,10});
    check(trace.front() == "get-mode");
    check(std::count(trace.begin(),trace.end(),"finished") == 0);
    trace.clear();
    load_phase_035850(os,effects,state,false);
    check(state.loaded == std::vector<Module>{99,10,20});
    check(trace.front() == "wait:2" && trace.back() == "finished");
    state.loaded.insert(state.loaded.begin()+1,0);
    state.loaded.push_back(10);
    unload_035ab0(os,state);
    check(os.frees == std::vector<Module>{99,10,20,10}); // forward order, no deduplication
    check(state.loaded == std::vector<Module>{99,0,10,20,10});
    unload_035ab0(os,state); // original does not clear; second call repeats releases
    check(os.frees == std::vector<Module>{99,10,20,10,99,10,20,10});
    trace.clear();
    State empty;
    load_phase_035850(os,effects,empty,false);
    check(trace.front() == "wait:2" && trace.back() == "finished");
    trace.clear(); effects.fail_log = true;
    bool threw = false;
    try { load_phase_035850(os,effects,state,false); } catch (LogFailure&) { threw = true; }
    check(threw && std::count(trace.begin(),trace.end(),"finished") == 0);
}

static void test_unload_snapshots_storage() {
    std::vector<std::string> trace;
    FakeSystem os(trace);
    State state;
    state.loaded = {10,20};
    // Synthetic reentry swaps the vector while retaining its old allocation.
    // Assembly snapshots begin/end before calling FreeLibrary, so the second
    // handle still comes from the original allocation rather than global data.
    std::vector<Module> retained{30,40};
    os.on_free = [&](Module module) {
        if (module == 10) state.loaded.swap(retained);
    };
    unload_035ab0(os,state);
    check(os.frees == std::vector<Module>{10,20});
    check(state.loaded == std::vector<Module>{30,40});
}

int main() {
    test_conversion(); test_list(); test_phases_and_unload(); test_unload_snapshots_storage();
    std::printf("PASS: %u checks; mocks only; no DLL loaded or freed\n",checks);
}
