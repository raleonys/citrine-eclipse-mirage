#include "runtime.hpp"
#include <bit>

namespace anadius::crt_runtime_recovered {
static_assert(sizeof(Address) == 8);
State::State(std::uint64_t security_cookie) : cookie(security_cookie) {
    for (auto& entry : procedures) entry.store(encode_pointer(0, cookie));
    module_names[0] = u"api-ms-win-core-datetime-l1-1-1";
    module_names[1] = u"api-ms-win-core-file-l1-2-4";
    module_names[2] = u"api-ms-win-core-file-l1-2-2";
    module_names[3] = u"api-ms-win-core-localization-l1-2-1";
    module_names[4] = u"api-ms-win-core-localization-obsolete-l1-2-0";
    module_names[5] = u"api-ms-win-core-processthreads-l1-1-2";
    module_names[6] = u"api-ms-win-core-string-l1-1-0";
    module_names[7] = u"api-ms-win-core-synch-l1-2-0";
    module_names[8] = u"api-ms-win-core-sysinfo-l1-2-1";
    module_names[9] = u"api-ms-win-core-winrt-l1-1-0";
    module_names[10] = u"api-ms-win-core-xstate-l2-1-0";
    module_names[11] = u"api-ms-win-rtcore-ntuser-window-l1-1-0";
    module_names[12] = u"api-ms-win-security-systemfunctions-l1-1-0";
    module_names[13] = u"ext-ms-win-ntuser-dialogbox-l1-1-0";
    module_names[14] = u"ext-ms-win-ntuser-windowstation-l1-1-0";
    module_names[15] = u"advapi32";
    module_names[16] = u"kernel32";
    module_names[17] = u"kernelbase";
    module_names[18] = u"ntdll";
    module_names[19] = u"api-ms-win-appmodel-runtime-l1-1-2";
    module_names[20] = u"user32";
}

Address encode_pointer(Address value, std::uint64_t cookie) noexcept {
    return std::rotl(value, static_cast<int>(cookie & 63)) ^ cookie;
}
Address decode_pointer(Address value, std::uint64_t cookie) noexcept {
    return std::rotr(value ^ cookie, static_cast<int>(cookie & 63));
}

Address resolve_0669b0(State& state, Effects& effects, std::uint32_t procedure,
                      std::string_view name,
                      std::span<const std::uint32_t> candidates) {
    const auto cached = decode_pointer(
        state.procedures[procedure].load(std::memory_order_relaxed), state.cookie);
    if (cached == missing) return 0;
    if (cached != 0) return cached;
    for (const auto index : candidates) {
        auto module = state.modules[index].load(std::memory_order_relaxed);
        if (module == 0) {
            const auto module_name = state.module_names[index];
            module = effects.load_library_ex_w(module_name, 0, 0x800);
            if (module == 0 && effects.last_error() == 0x57 &&
                !module_name.starts_with(u"api-ms-") &&
                !module_name.starts_with(u"ext-ms-")) {
                module = effects.load_library_ex_w(module_name, 0, 0);
            }
            if (module == 0) {
                state.modules[index].exchange(missing);
                continue;
            }
            const auto previous = state.modules[index].exchange(module);
            // Original code frees the newly loaded handle, then queries that
            // same local handle when any prior cache value was nonzero.
            if (previous != 0) effects.free_library(module);
        } else if (module == missing) {
            continue;
        }
        const auto address = effects.get_proc_address(module, name);
        if (address != 0) {
            state.procedures[procedure].exchange(encode_pointer(address, state.cookie));
            return address;
        }
        // A loaded module lacking the export terminates candidate iteration.
        break;
    }
    state.procedures[procedure].exchange(encode_pointer(missing, state.cookie));
    return 0;
}

std::uint32_t process_parameters_flag_06c0b8(Effects& effects) {
    return effects.process_parameters_flags() >> 31;
}

std::uint32_t query_thread_policy_066bc4(State& state, Effects& effects,
                                       std::int32_t* output) {
    constexpr std::uint32_t modules[]{19};
    const auto address = resolve_0669b0(state, effects, 0x1a,
        "AppPolicyGetThreadInitializationType", modules);
    if (address == 0) return 0xc0000225U;
    return effects.call_thread_initialization_policy(address, missing - 5, output);
}

std::uint32_t runtime_policy_06c0d0(State& state, Effects& effects) {
    const auto cached = state.thread_policy.load(std::memory_order_relaxed);
    if (cached != 0) return cached;
    std::int32_t policy = 0;
    if (process_parameters_flag_06c0b8(effects) == 0)
        (void)query_thread_policy_066bc4(state, effects, &policy);
    const auto result = std::uint32_t{1} + (policy == 1 ? 1U : 0U);
    state.thread_policy.exchange(result);
    // EAX retains this invocation's computed value; do not reload the cache.
    return result;
}

std::int32_t initialize_runtime_067320(State& state, Effects& effects,
                                      std::uint32_t mode) {
    constexpr std::uint32_t modules[]{9};
    const auto address = resolve_0669b0(state, effects, 0x17, "RoInitialize", modules);
    return address == 0 ? 0 : effects.call_ro_initialize(address, mode);
}

void uninitialize_runtime_06736c(State& state, Effects& effects) {
    constexpr std::uint32_t modules[]{9};
    const auto address = resolve_0669b0(state, effects, 0x18, "RoUninitialize", modules);
    if (address != 0) effects.call_ro_uninitialize(address);
}

int ThreadRuntimeBridge::runtime_policy_06c0d0() {
    return std::bit_cast<std::int32_t>(
        crt_runtime_recovered::runtime_policy_06c0d0(state_, effects_));
}
int ThreadRuntimeBridge::initialize_runtime_067320(int mode) {
    return crt_runtime_recovered::initialize_runtime_067320(
        state_, effects_, std::bit_cast<std::uint32_t>(mode));
}
void ThreadRuntimeBridge::uninitialize_runtime_06736c() {
    crt_runtime_recovered::uninitialize_runtime_06736c(state_, effects_);
}
} // namespace anadius::crt_runtime_recovered
