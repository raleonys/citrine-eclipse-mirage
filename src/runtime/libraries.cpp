// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/libraries.hpp"
#include <climits>

namespace laureondo::system_services::libraries {
Conversion convert_wide_048b30(SystemCalls& os, std::uint32_t codepage,
    const wchar_t* input, int count, char* output, int capacity) {
    int converted;
    if (codepage == 65001 || codepage == 54936) {
        converted = os.wide_to_narrow(codepage, 0x80, input, count,
                                      output, capacity, nullptr);
    } else {
        bool used_default = false;
        converted = os.wide_to_narrow(codepage, 0x400, input, count,
                                      output, capacity, &used_default);
        if (used_default) return {0, 0x459};
    }
    std::uint32_t error = converted == 0 ? os.last_error() : 0;
    if (error == 0x3ec) {
        converted = os.wide_to_narrow(codepage, 0, input, count,
                                      output, capacity, nullptr);
        error = converted == 0 ? os.last_error() : 0;
    }
    return {converted, error};
}

std::uint32_t SystemCalls::begin_error_suppression() {
    const auto previous = get_error_mode();
    set_error_mode(previous | 1u);
    return previous;
}
void SystemCalls::end_error_suppression(std::uint32_t previous) {set_error_mode(previous);}
std::uint32_t suppress_errors_0450a0(SystemCalls& os) {return os.begin_error_suppression();}

Module load_copy_0450d0(SystemCalls& os, const std::wstring& path) {
    const std::wstring copy(path); // 00a140 copies, rather than canonicalizes path
    return os.load_library(copy.c_str());
}

static std::string utf8_path(SystemCalls& os, Effects& effects,
                              const std::wstring& path) {
    if (path.empty()) return {};
    if (path.size() > INT_MAX) effects.throw_runtime_00cd70(22);
    auto result = convert_wide_048b30(os, 65001, path.data(),
                                     static_cast<int>(path.size()), nullptr, 0);
    if (result.error != 0) effects.throw_filesystem_00cdb0(result.error);
    std::string converted(static_cast<std::size_t>(result.count), '\0');
    result = convert_wide_048b30(os, 65001, path.data(),
        static_cast<int>(path.size()), converted.data(), result.count);
    if (result.error != 0) effects.throw_filesystem_00cdb0(result.error);
    // Original string length remains the size returned by the first conversion.
    return converted;
}

std::vector<Module> load_list_035260(SystemCalls& os, Effects& effects,
                                    std::span<const std::wstring> paths) {
    std::vector<Module> loaded;
    const auto previous_mode = suppress_errors_0450a0(os);
    for (const auto& path : paths) {
        effects.begin_load_log_0863e0();
        effects.append_log_path_and_flush(utf8_path(os, effects, path));
        const auto module = load_copy_0450d0(os, path);
        if (module != 0) {
            loaded.push_back(module);
        } else {
            // Reconvert after LoadLibraryW failure: both conversion side effects
            // and possible failure precede the error dialog in the original.
            auto message = utf8_path(os, effects, path);
            message.insert(0, "Can't load ");
            effects.message_038bc0("error", std::move(message), 0x10);
        }
    }
    // Intentionally normal-path restoration, not RAII. The observed body has no
    // mode-restoration call on its throwing conversion/allocator paths.
    os.end_error_suppression(previous_mode);
    return loaded;
}

void load_phase_035850(SystemCalls& os, Effects& effects, State& state,
                       bool from_main) {
    if (!from_main) effects.wait_steps_036880(2);
    auto loaded = load_list_035260(os, effects,
                                  from_main ? state.from_main : state.from_worker);
    state.loaded.insert(state.loaded.end(), loaded.begin(), loaded.end());
    if (!from_main) effects.finish_flag_036820();
}

void unload_035ab0(SystemCalls& os, const State& state) {
    // Original code snapshots the end, iterates forwards, skips null, ignores
    // FreeLibrary results, and does not clear the vector or individual entries.
    const auto* const entries = state.loaded.data();
    const auto count = state.loaded.size();
    for (std::size_t i = 0; i < count; ++i) {
        if (entries[i] != 0) os.free_library(entries[i]);
    }
}
} // namespace laureondo::system_services::libraries
