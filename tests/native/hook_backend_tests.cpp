// SPDX-License-Identifier: GPL-3.0-only

// Application integration with the actual selected MTH engine. Only the two
// explicitly named error returns and callback-drain decision are injected.
#define NOMINMAX
#include <windows.h>
#include <MithrilHook.h>
#include "laureondo/platform/hook_transaction.hpp"
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>

namespace engine = laureondo::platform::hook_engine;
namespace {
unsigned checks;
void require(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}
using Function = int(WINAPI*)();
struct Code final {
    unsigned char* bytes = static_cast<unsigned char*>(
        VirtualAlloc(nullptr, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    Code() {
        require(bytes != nullptr, "allocate owned leaf target arena");
        std::memset(bytes, 0x90, 4096);
        for (unsigned i = 0; i < 4; ++i) {
            auto* p = bytes + i * 64;
            const int result = std::array{7, 11, 19, 23}[i];
            p[0] = 0xb8; std::memcpy(p + 1, &result, sizeof result); p[5] = 0xc3;
        }
        DWORD previous = 0;
        require(VirtualProtect(bytes, 4096, PAGE_EXECUTE_READ, &previous) != FALSE &&
                FlushInstructionCache(GetCurrentProcess(), bytes, 4096) != FALSE,
                "publish executable leaf fixtures before using the real engine");
    }
    ~Code() { if (bytes) VirtualFree(bytes, 0, MEM_RELEASE); }
    void* address(unsigned i) const { return bytes + i * 64; }
    int call(unsigned i) const { return reinterpret_cast<Function>(address(i))(); }
};
struct Calls final : engine::Quiescence {
    bool quiet = true;
    unsigned drains = 0;
    bool drain(std::chrono::milliseconds) override { ++drains; return quiet; }
};
struct ForwardingApi final : engine::BatchApi {
    engine::NativeApi native;
    bool fail_after_real_apply = false, fail_before_real_uninitialize = false;
    unsigned applies = 0, disables = 0, uninitializes = 0;
    int initialize() override { return native.initialize(); }
    int create(void* t, void* d, void** o) override { return native.create(t, d, o); }
    int enable(void* t) override { return native.enable(t); }
    int queue_enable(void* t) override { return native.queue_enable(t); }
    int apply_queued() override {
        ++applies;
        const auto status = native.apply_queued();
        return status == engine::status_code::ok && fail_after_real_apply
            ? engine::status_code::memory_protect : status;
    }
    int disable(void* t) override { ++disables; return native.disable(t); }
    int uninitialize() override {
        ++uninitializes;
        if (fail_before_real_uninitialize) {
            fail_before_real_uninitialize = false;
            return engine::status_code::memory_protect;
        }
        return native.uninitialize();
    }
};
void external_owner() {
    engine::NativeApi api; Calls calls; engine::Transaction tx(api, calls);
    require(api.disable(nullptr) == engine::status_code::not_initialized,
            "native not-initialized status reaches the application unchanged");
    require(MTH_Initialize() == MTH_OK, "establish an independent engine owner");
    require(tx.begin() == engine::status_code::already_initialized && !tx.owns_engine(),
            "failed application initialization does not acquire external ownership");
    require(MTH_Initialize() == MTH_ERROR_ALREADY_INITIALIZED && calls.drains == 0,
            "application rejection neither drains nor uninitializes external engine");
    require(MTH_Uninitialize() == MTH_OK, "independent owner releases its engine");
}
void transaction(bool inject_failure) {
    Code code; ForwardingApi api; Calls calls; engine::Transaction tx(api, calls);
    std::array<void*, 2> originals{};
    try {
        require(tx.begin() == engine::status_code::ok, "application acquires native engine");
        for (unsigned i = 0; i < 2; ++i) {
            require(tx.create(code.address(i), code.address(i + 2), &originals[i]) ==
                    engine::status_code::ok && originals[i], "real create publishes application original slot");
            require(tx.queue(code.address(i)) == engine::status_code::ok,
                    "application queues genuine native target");
        }
        require(code.call(0) == 7 && code.call(1) == 11 && api.applies == 0,
                "staging both hooks does not individually patch targets");
        require(reinterpret_cast<Function>(originals[0])() == 7 &&
                reinterpret_cast<Function>(originals[1])() == 11,
                "real original-call slots execute the selected engine trampolines");
        api.fail_after_real_apply = inject_failure;
        const auto status = tx.activate();
        require(status == (inject_failure ? engine::status_code::memory_protect : engine::status_code::ok) &&
                api.applies == 1 && code.call(0) == 19 && code.call(1) == 23,
                "one real batch patches both targets; injected post-commit failure remains visible");
        if (inject_failure) {
            require(tx.owns_engine() && tx.state() == engine::TransactionState::RollbackIncomplete,
                    "failed apply retains the application engine and original slots");
            calls.quiet = false;
            require(!tx.rollback(std::chrono::milliseconds(0)) && tx.owns_engine() &&
                    code.call(0) == 7 && code.call(1) == 11 && api.uninitializes == 0,
                    "real disable restores targets; a busy callback gate prevents retirement");
            require(reinterpret_cast<Function>(originals[0])() == 7 &&
                    reinterpret_cast<Function>(originals[1])() == 11,
                    "original slots remain valid across incomplete callback drain");
            calls.quiet = true; api.fail_before_real_uninitialize = true;
            require(!tx.rollback(std::chrono::milliseconds(0)) && tx.owns_engine() &&
                    api.native.initialize() == engine::status_code::already_initialized,
                    "injected cleanup failure preserves the actual engine for retry");
            require(tx.rollback(std::chrono::milliseconds(0)) && !tx.owns_engine() &&
                    tx.state() == engine::TransactionState::Stopped && api.disables == 1,
                    "retry releases ownership only after the real engine retires successfully");
        } else {
            require(tx.finish(std::chrono::milliseconds(100)) && !tx.owns_engine() &&
                    code.call(0) == 7 && code.call(1) == 11 && calls.drains == 1,
                    "normal application shutdown disables, drains and retires the actual engine");
        }
        // Do not dereference original pointers after successful engine retirement.
        require(api.native.initialize() == engine::status_code::ok &&
                api.native.uninitialize() == engine::status_code::ok,
                "completed transaction leaves a reusable native engine state");
    } catch (...) {
        api.fail_after_real_apply = api.fail_before_real_uninitialize = false;
        calls.quiet = true;
        for (unsigned i = 0; i < 3 && tx.owns_engine(); ++i)
            (void)tx.rollback(std::chrono::milliseconds(100));
        throw;
    }
}
}
int main() try {
    external_owner(); transaction(false); transaction(true);
    std::cout << "PASS: " << checks << " Laureondo adapter/transaction checks with actual MTH patches and calls; "
                 "post-apply/cleanup return and drain failures explicitly injected\n";
    return 0;
} catch (const std::exception& e) {
    std::cerr << "FAIL after " << checks << ": " << e.what() << '\n'; return 1;
}
