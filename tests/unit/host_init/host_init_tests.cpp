// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/platform/hook_setup.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

using namespace citrine_eclipse_mirage::host_init;
namespace {
int checks = 0;
void require(bool value, const char* detail) {
    ++checks;
    if (!value) throw std::runtime_error(detail);
}
struct Injected {};
struct HookCall { std::u16string module; std::string name; Callback callback; };
struct Fixture : Effects {
    citrine_eclipse_mirage::configuration::State config{};
    Values values{{"old", u"remove"}};
    std::vector<Address> enabled{0x111, 0x222};
    std::array<Address, callback_count> originals{};
    std::vector<std::string> events;
    std::vector<HookCall> hooks;
    std::vector<Address> enable_calls;
    std::string line;
    std::vector<std::string> lines;
    bool modern = false;
    int initialize_status = 0, create_status = 0, enable_status = 0;
    int create_failure = -1, enable_failure = -1, module_failure = -1, proc_failure = -1;
    int fail_event = -1, widen_count = 0, module_count = 0, proc_count = 0;
    std::size_t current_hook = 0;
    std::u16string module_name;
    std::string proc_name;
    StateRefs refs() { return {config, values, enabled, originals}; }
    Fixture() {
        originals.fill(0x777);
        config.executable_path = u"C:\\synthetic\\host.exe";
        config.registry_spoof = 0x81;
        config.language_registry_key = u"Software\\synthetic\\Language";
        config.game_attributes["INSTALLED_LANGUAGE"] = "synthetic-language";
        config.block_reruns = 2;
        config.documents = u"C:\\synthetic\\docs";
    }
    void tick(std::string event) {
        events.push_back(std::move(event));
        if (static_cast<int>(events.size()) == fail_event) throw Injected{};
    }
    std::u16string filesystem_widen_006d70(std::string_view text) override {
        ++widen_count;
        const char* key = widen_count == 1 ? "ClientKey" : widen_count == 2 ? "ClientKey2" :
                          widen_count == 3 ? "ClientValue" : "Language";
        require(values.contains(key) && values.at(key).empty(), "value inserted before conversion");
        tick("widen:" + std::string(text));
        return std::u16string(text.begin(), text.end());
    }
    std::uint64_t version_condition_mask(std::uint64_t previous, std::uint32_t type,
                                         std::uint8_t condition) override {
        require(condition == 3, "version uses greater or equal");
        if (type == 2) require(previous == 0, "initial mask zero");
        else if (type == 1) require(previous == 23, "minor mask chains");
        else require(type == 0x20 && previous == 243, "service mask chains");
        tick("mask:" + std::to_string(type));
        return previous * 10 + type * 10 + condition;
    }
    bool verify_version(const VersionRequest& request, std::uint32_t type,
                        std::uint64_t mask) override {
        require(request.size == 284 && request.major == 6 && request.minor == 2 &&
                request.build == 0 && request.platform == 0 && request.service_pack_major == 0 &&
                request.service_pack_minor == 0 && request.suite == 0 && request.product_type == 0 &&
                request.reserved == 0, "full OS version request");
        require(std::all_of(request.service_pack_text.begin(), request.service_pack_text.end(),
                [](auto c) { return c == 0; }), "version service pack text zero");
        require(type == 0x23 && mask == 2753, "version masks");
        require(enabled.empty(), "success vector cleared before version API");
        tick("verify");
        return modern;
    }
    std::int32_t initialize_engine_046a20() override { tick("initialize"); return initialize_status; }
    Address get_module_handle(std::u16string_view module) override {
        module_name = module;
        tick("module");
        return module_count++ == module_failure ? 0 : 0x500;
    }
    Address get_proc_address(Address handle, std::string_view name) override {
        require(handle == 0x500, "proc receives module handle");
        proc_name = name;
        tick("proc:" + proc_name);
        const auto ordinal = proc_count++;
        return ordinal == proc_failure ? 0 : static_cast<Address>(0x1000 + ordinal);
    }
    std::int32_t create_hook_0466e0(Address target, Callback callback, Address& original) override {
        current_hook = slot(callback);
        hooks.push_back({module_name, proc_name, callback});
        tick("create:" + std::to_string(current_hook));
        require(target >= 0x1000, "create receives proc address");
        original = 0x8000 + current_hook;
        return static_cast<int>(current_hook) == create_failure ? create_status : 0;
    }
    std::int32_t enable_hook_046a10(Address target) override {
        tick("enable:" + std::to_string(current_hook));
        enable_calls.push_back(target);
        return static_cast<int>(current_hook) == enable_failure ? enable_status : 0;
    }
    void log_text_006880(std::string_view text) override { tick("text:" + std::string(text)); line += text; }
    void log_integer_00af90(std::int32_t value) override { tick("integer"); line += std::to_string(value); }
    void finish_log_0088e0() override { tick("finish"); lines.push_back(std::move(line)); line.clear(); }
};
void combinations() {
    const std::array<std::string_view, callback_count> names = {"connect", "RegOpenKeyExA", "RegOpenKeyExW", "RegCloseKey",
        "RegQueryValueExA", "RegQueryValueExW", "CreateProcessA", "CreateProcessW", "SHGetFolderPathA", "SHGetFolderPathW", "CryptQueryObject"};
    for (unsigned bits = 0; bits < 16; ++bits) {
        Fixture f;
        f.modern = (bits & 1) != 0;
        f.config.block_reruns = (bits & 2) != 0 ? 0xff : 0;
        if (!(bits & 4)) f.config.documents.clear();
        if (!(bits & 8)) f.config.language_registry_key.clear();
        initialize_host_00e310(f.refs(), f);
        const std::size_t count = 7 + ((bits & 2) ? 2 : 0) + ((bits & 4) ? 2 : 0);
        require(f.hooks.size() == count && f.enabled.size() == count, "hook combination count");
        require(f.values.size() == 4 + ((bits & 8) ? 3u : 0u) + ((bits & 4) ? 1u : 0u), "replacement count");
        require(!f.values.contains("old"), "old values discarded");
        require(f.values.at("ClientKey") == u"SOFTWARE\\Wow6432Node\\Origin", "first key");
        require(f.values.at("ClientKey2") == u"SOFTWARE\\Electronic Arts\\EA Desktop", "second key");
        require(f.values.at("ClientValue") == u"ClientPath", "client value");
        require(f.values.at("ClientPath") == f.config.executable_path, "executable path copy");
        require(f.config.registry_spoof == ((bits & 8) ? 0x81 : 0), "empty language disables registry spoof");
        if (bits & 8) {
            require(f.values.at("LanguageKey") == u"Software\\synthetic", "language parent");
            require(f.values.at("LanguageValue") == u"Language", "language leaf");
            require(f.values.at("Language") == u"synthetic-language", "language conversion");
        }
        if (bits & 4) require(f.values.at("Documents") == f.config.documents, "documents copied");
        std::size_t ordinal = 0;
        for (std::size_t index = 0; index < callback_count; ++index) {
            const bool active = index < 6 || index == 10 || (index < 8 ? (bits & 2) : (bits & 4));
            require(f.originals[index] == (active ? 0x8000 + index : 0x777), "original slots");
            if (!active) continue;
            const auto& h = f.hooks.at(ordinal++);
            require(h.name == names[index] && slot(h.callback) == index, "ordered names and callback IDs");
            const std::u16string expected_module = index == 10 ? u"crypt32.dll" : index == 0 ? u"ws2_32.dll" : index < 6 ?
                (f.modern ? u"KernelBase.dll" : u"advapi32.dll") : index < 8 ?
                (f.modern ? u"KernelBase.dll" : u"Kernel32.dll") : u"shell32.dll";
            require(h.module == expected_module, "version selects module");
        }
        require(f.lines.at(0) == "MinHook initialization succeeded", "initialization success log");
    }
}
void errors() {
    for (const auto status : {1, 9, -1, INT32_MIN}) {
        Fixture f; f.initialize_status = status;
        initialize_host_00e310(f.refs(), f);
        require(f.hooks.empty() && f.enabled.empty(), "nonzero initialization prevents hooks");
        require(f.lines == std::vector<std::string>{"MinHook initialization failed"}, "failure log");
        require(!f.values.contains("Documents"), "documents inserted only after engine success");
    }
    for (int index = 0; index < static_cast<int>(callback_count); ++index) {
        for (int stage = 0; stage < 2; ++stage) {
            Fixture f;
            if (stage == 0) { f.create_failure = index; f.create_status = -17; }
            else { f.enable_failure = index; f.enable_status = -29; }
            initialize_host_00e310(f.refs(), f);
            require(f.hooks.size() == callback_count && f.enabled.size() == callback_count - 1, "hook error continues later hooks");
            require(f.enable_calls.size() == (stage == 0 ? callback_count - 1 : callback_count), "creation failure skips enable");
            const auto expected = std::string(stage == 0 ? "Hooking function " : "Enabling hook for ") +
                f.hooks.at(index).name + " failed; error " + std::to_string(stage == 0 ? -17 : -29);
            require(std::find(f.lines.begin(), f.lines.end(), expected) != f.lines.end(), "precise error log");
            require(f.originals.at(index) == 0x8000 + static_cast<unsigned>(index), "backend write survives error");
        }
    }
    for (int stage = 0; stage < 2; ++stage) {
        Fixture f;
        Address target = 0x123, original = 0x456;
        if (stage == 0) f.module_failure = 0; else f.proc_failure = 0;
        const auto result = create_named_hook_046970(f, u"synthetic.dll", "synthetic", Callback::connect_00fd00,
                                                    original, &target);
        require(result == (stage == 0 ? 11 : 12), "lookup error values");
        require(target == 0x123 && original == 0x456 && f.hooks.empty(), "lookup failure leaves outputs");
        require(f.proc_count == (stage == 0 ? 0 : 1), "missing module skips symbol lookup");
    }
    Fixture f; Address original = 0;
    require(create_named_hook_046970(f, u"synthetic.dll", "synthetic", Callback::reg_close_00f5b0,
                                   original, nullptr) == 0, "optional target pointer");
    require(original == 0x8003, "optional target still creates hook");
    Fixture failed; failed.create_failure = 0; failed.create_status = -7;
    Address failed_original = 0x33, failed_target = 0x44;
    require(create_named_hook_046970(failed, u"synthetic.dll", "synthetic", Callback::connect_00fd00,
                                    failed_original, &failed_target) == -7, "creation status passed through");
    require(failed_target == 0x1000 && failed_original == 0x8000, "target set before failed engine call");
    for (int stage = 0; stage < 2; ++stage) {
        Fixture missing;
        if (stage == 0) missing.module_failure = 0; else missing.proc_failure = 0;
        initialize_host_00e310(missing.refs(), missing);
        require(missing.hooks.size() == callback_count - 1 && missing.enabled.size() == callback_count - 1, "lookup failure continues remaining setup");
        require(missing.originals.at(0) == 0x777, "lookup failure keeps old original slot");
        const auto message = std::string("Hooking function connect failed; error ") + std::to_string(stage == 0 ? 11 : 12);
        require(std::find(missing.lines.begin(), missing.lines.end(), message) != missing.lines.end(), "lookup error logged");
    }
}
void exception_prefixes() {
    Fixture complete;
    initialize_host_00e310(complete.refs(), complete);
    const auto full = complete.events;
    for (std::size_t event = 1; event <= full.size(); ++event) {
        Fixture f; f.fail_event = static_cast<int>(event);
        bool thrown = false;
        try { initialize_host_00e310(f.refs(), f); } catch (const Injected&) { thrown = true; }
        require(thrown, "injected exception escapes");
        require(f.events == std::vector<std::string>(full.begin(), full.begin() + event), "no cleanup or later calls after throw");
    }
    for (int stage = 0; stage < 2; ++stage) {
        Fixture normal;
        if (stage == 0) { normal.create_failure = 0; normal.create_status = 7; }
        else { normal.enable_failure = 0; normal.enable_status = 8; }
        initialize_host_00e310(normal.refs(), normal);
        for (std::size_t event = 1; event <= normal.events.size(); ++event) {
            Fixture interrupted;
            if (stage == 0) { interrupted.create_failure = 0; interrupted.create_status = 7; }
            else { interrupted.enable_failure = 0; interrupted.enable_status = 8; }
            interrupted.fail_event = static_cast<int>(event);
            bool thrown = false;
            try { initialize_host_00e310(interrupted.refs(), interrupted); } catch (const Injected&) { thrown = true; }
            require(thrown, "error-path exception escapes");
            require(interrupted.events == std::vector<std::string>(normal.events.begin(), normal.events.begin() + event),
                    "error-path log exception has exact prefix");
        }
    }
    Fixture f; f.fail_event = 2;
    try { initialize_host_00e310(f.refs(), f); } catch (const Injected&) {}
    require(f.values.size() == 2 && !f.values.at("ClientKey").empty() && f.values.at("ClientKey2").empty(),
            "conversion failure keeps inserted partial map");
    require(f.enabled == std::vector<Address>({0x111, 0x222}), "vector not cleared before conversion completes");
    Fixture absent; absent.config.game_attributes.clear();
    initialize_host_00e310(absent.refs(), absent);
    require(absent.config.game_attributes.contains("INSTALLED_LANGUAGE") && absent.values.at("Language").empty(),
            "missing installed language inserted and converted empty");
    Fixture nul;
    nul.config.documents = std::u16string(u"synthetic\0suffix", 16);
    nul.config.executable_path = std::u16string(u"host\0tail", 9);
    nul.config.game_attributes["INSTALLED_LANGUAGE"] = std::string("lang\0tail", 9);
    initialize_host_00e310(nul.refs(), nul);
    require(nul.values.at("Documents") == nul.config.documents && nul.values.at("ClientPath") == nul.config.executable_path,
            "wide copies preserve embedded NUL");
    require(nul.values.at("Language").size() == 9 && nul.values.at("Language")[4] == 0, "language conversion explicit length");
}
}
int main() {
    try { combinations(); errors(); exception_prefixes(); }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
    catch (...) { std::cerr << "unexpected exception\n"; return 2; }
    std::cout << "PASS " << checks << " host initialization checks\n";
}
