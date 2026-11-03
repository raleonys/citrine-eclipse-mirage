// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/hook_engine/fake_state.hpp"
#include <MinHook.h>
extern "C" {
#include "third_party/minhook/src/trampoline.h"
#include "third_party/minhook/src/hde/hde64.h"
}
#include <algorithm>
#include <array>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
unsigned checks = 0;
void check(bool condition, const char* what) {
    ++checks;
    if (!condition) throw std::runtime_error(what);
}
template<class T> T read(const unsigned char* p) { T v; std::memcpy(&v, p, sizeof v); return v; }
template<class T> void write(unsigned char* p, T v) { std::memcpy(p, &v, sizeof v); }
unsigned char* target() { return fixture::targets.data() + 128; }
unsigned char* detour() { return fixture::targets.data() + 32768; }
void begin() {
    fixture::reset();
    fixture::state.threads.push_back({10, 7, 0}); // Fake current thread; enumeration succeeds and skips it.
    check(MH_Initialize() == MH_OK, "initialize");
}
void end() {
    check(MH_Uninitialize() == MH_OK, "uninitialize");
    check(fixture::live_heap_allocations() == 0, "heap allocations released");
    check(fixture::live_buffer_blocks() == 0, "buffer blocks released");
}
void lifecycle() {
    fixture::reset();
    check(MH_Uninitialize() == MH_ERROR_NOT_INITIALIZED, "uninitialized uninitialize");
    check(MH_CreateHook(target(), detour(), nullptr) == MH_ERROR_NOT_INITIALIZED, "uninitialized create");
    check(MH_EnableHook(target()) == MH_ERROR_NOT_INITIALIZED, "uninitialized enable");
    check(MH_DisableHook(target()) == MH_ERROR_NOT_INITIALIZED, "uninitialized disable");
    check(MH_RemoveHook(target()) == MH_ERROR_NOT_INITIALIZED, "uninitialized remove");
    check(MH_QueueEnableHook(target()) == MH_ERROR_NOT_INITIALIZED, "uninitialized queue");
    check(MH_ApplyQueued() == MH_ERROR_NOT_INITIALIZED, "uninitialized apply");
    fixture::state.heap_create_fail = true;
    check(MH_Initialize() == MH_ERROR_MEMORY_ALLOC, "heap creation failure");
    fixture::state.heap_create_fail = false;
    check(MH_Initialize() == MH_OK, "initialize after heap failure");
    check(MH_Initialize() == MH_ERROR_ALREADY_INITIALIZED, "duplicate initialize");
    check(MH_CreateHook(reinterpret_cast<void*>(1), detour(), nullptr) == MH_ERROR_NOT_EXECUTABLE, "nonowned target rejected");
    check(MH_CreateHook(target(), reinterpret_cast<void*>(1), nullptr) == MH_ERROR_NOT_EXECUTABLE, "nonowned detour rejected");
    check(MH_EnableHook(target()) == MH_ERROR_NOT_CREATED, "absent enable");
    check(MH_DisableHook(target()) == MH_ERROR_NOT_CREATED, "absent disable");
    check(MH_RemoveHook(target()) == MH_ERROR_NOT_CREATED, "absent remove");
    check(MH_QueueDisableHook(target()) == MH_ERROR_NOT_CREATED, "absent queue");
    check(MH_ApplyQueued() == MH_OK, "empty apply");
    check(MH_EnableHook(MH_ALL_HOOKS) == MH_OK, "empty enable all");
    check(MH_DisableHook(MH_ALL_HOOKS) == MH_OK, "empty disable all");
    end();
    for (int code = MH_UNKNOWN; code <= MH_ERROR_FUNCTION_NOT_FOUND; ++code)
        check(std::string(MH_StatusToString(static_cast<MH_STATUS>(code))).starts_with("MH_"), "known status spelling");
    check(std::string(MH_StatusToString(static_cast<MH_STATUS>(1000))) == "(unknown)", "unknown status spelling");
}
void patch_and_threads() {
    begin();
    void* original = nullptr;
    check(MH_CreateHook(target(), detour(), &original) == MH_OK, "create nop hook");
    auto* tr = static_cast<unsigned char*>(original);
    check(fixture::owned(tr, 33), "owned trampoline buffer");
    check(std::all_of(tr, tr + 5, [](auto c) { return c == 0x90; }), "copied five nop bytes");
    check(tr[5] == 0xff && tr[6] == 0x25 && read<std::uint32_t>(tr + 7) == 0, "absolute jump back encoding");
    check(read<std::uint64_t>(tr + 11) == reinterpret_cast<std::uint64_t>(target() + 5), "jump back destination");
    check(tr[19] == 0xff && tr[20] == 0x25, "relay absolute jump encoding");
    check(read<std::uint64_t>(tr + 25) == reinterpret_cast<std::uint64_t>(detour()), "relay detour destination");
    check(MH_CreateHook(target(), detour(), nullptr) == MH_ERROR_ALREADY_CREATED, "duplicate hook");
    check(MH_DisableHook(target()) == MH_ERROR_DISABLED, "already disabled");
    fixture::state.threads.push_back({11, 7, reinterpret_cast<std::uint64_t>(target() + 2)});
    fixture::state.threads.push_back({12, 99, reinterpret_cast<std::uint64_t>(target())});
    check(MH_EnableHook(target()) == MH_OK, "enable hook");
    check(target()[0] == 0xe9, "five byte patch opcode");
    check(reinterpret_cast<std::intptr_t>(target()) + 5 + read<std::int32_t>(target() + 1) == reinterpret_cast<std::intptr_t>(tr + 19), "patch reaches relay");
    check(fixture::state.last_patch_size == 5 && fixture::state.last_patch_address == target(), "five byte protection range");
    check(fixture::state.protects == 2 && fixture::state.flushes == 1, "protect restore and cache notification");
    check(fixture::state.threads[1].ip == reinterpret_cast<std::uint64_t>(tr + 2), "enable maps target instruction pointer");
    check(fixture::state.threads[1].suspended == 1 && fixture::state.threads[1].resumed == 1, "fake suspend resume balanced");
    check(fixture::state.threads[2].suspended == 0, "foreign process skipped");
    check(fixture::state.threads[0].suspended == 0, "current thread skipped");
    check(MH_EnableHook(target()) == MH_ERROR_ENABLED, "already enabled");
    check(MH_DisableHook(MH_ALL_HOOKS) == MH_OK, "disable all");
    check(std::all_of(target(), target() + 5, [](auto c) { return c == 0x90; }), "original five bytes restored");
    check(fixture::state.threads[1].ip == reinterpret_cast<std::uint64_t>(target() + 2), "disable all maps trampoline instruction pointer");
    check(MH_EnableHook(target()) == MH_OK, "enable before individual disable");
    check(MH_DisableHook(target()) == MH_OK, "individual disable");
    // Selected upstream hook.c uses ACTION_ENABLE for this path even when
    // disabling. Preserve the observed upstream behavior in this test; this
    // is not a claim about the target binary's bundled engine implementation.
    check(fixture::state.threads[1].ip == reinterpret_cast<std::uint64_t>(tr + 2), "selected upstream individual disable leaves mapped instruction pointer");
    check(MH_RemoveHook(target()) == MH_OK, "remove disabled hook");
    check(fixture::live_buffer_blocks() == 0, "last removal releases actual buffer allocator block");
    end();
}
void queued_hotpatch() {
    begin();
    target()[0] = 0xc3; target()[1] = 0x90; target()[2] = 0x40; // Too short for normal patch; padding above exists.
    std::array<unsigned char, 7> backup{}; std::memcpy(backup.data(), target() - 5, backup.size());
    void* original = nullptr;
    check(MH_CreateHook(target(), detour(), &original) == MH_OK, "hotpatch create");
    check(static_cast<unsigned char*>(original)[0] == 0xc3, "return retained in hotpatch trampoline");
    check(MH_QueueEnableHook(target()) == MH_OK, "queue enable");
    check(std::memcmp(target() - 5, backup.data(), backup.size()) == 0, "queue leaves target bytes unchanged");
    check(MH_ApplyQueued() == MH_OK, "apply queued enable");
    check(target()[-5] == 0xe9 && target()[0] == 0xeb && target()[1] == 0xf9, "seven byte hotpatch encoding");
    check(fixture::state.last_patch_address == target() - 5 && fixture::state.last_patch_size == 7, "seven byte protection range");
    fixture::state.threads.push_back({11, 7, reinterpret_cast<std::uint64_t>(target() - 5)});
    check(MH_QueueDisableHook(MH_ALL_HOOKS) == MH_OK, "queue disable all");
    check(MH_ApplyQueued() == MH_OK, "apply queued disable");
    check(std::memcmp(target() - 5, backup.data(), backup.size()) == 0, "seven byte backup restored");
    check(fixture::state.threads[1].ip == reinterpret_cast<std::uint64_t>(target()), "hotpatch area ip mapped to target");
    end();
}
void failures() {
    begin();
    fixture::state.virtual_alloc_fail = true;
    check(MH_CreateHook(target(), detour(), nullptr) == MH_ERROR_MEMORY_ALLOC, "buffer allocation failure");
    fixture::state.virtual_alloc_fail = false;
    fixture::state.heap_alloc_fail = true;
    check(MH_CreateHook(target(), detour(), nullptr) == MH_ERROR_MEMORY_ALLOC, "hook entry allocation failure");
    check(fixture::live_buffer_blocks() == 0, "failed create releases buffer");
    fixture::state.heap_alloc_fail = false;
    target()[0] = 0xe2; target()[1] = 0x40; // LOOP outside overwritten range: unsupported.
    check(MH_CreateHook(target(), detour(), nullptr) == MH_ERROR_UNSUPPORTED_FUNCTION, "external loop rejected");
    check(fixture::live_buffer_blocks() == 0, "unsupported create releases buffer");
    std::fill_n(target(), 16, 0x90);
    check(MH_CreateHook(target(), detour(), nullptr) == MH_OK, "create after failures");
    fixture::state.snapshot_fail = true;
    check(MH_EnableHook(target()) == MH_ERROR_MEMORY_ALLOC, "snapshot failure surfaced as memory allocation");
    check(target()[0] == 0x90 && fixture::state.protects == 0, "snapshot failure avoids patching");
    fixture::state.snapshot_fail = false;
    fixture::state.enumeration_fail = true;
    check(MH_EnableHook(target()) == MH_ERROR_MEMORY_ALLOC, "enumeration failure");
    fixture::state.enumeration_fail = false;
    fixture::state.threads.push_back({11, 7, reinterpret_cast<std::uint64_t>(target())});
    fixture::state.heap_alloc_fail = true;
    check(MH_EnableHook(target()) == MH_ERROR_MEMORY_ALLOC, "fake thread list allocation failure");
    check(fixture::state.threads[1].suspended == 0 && target()[0] == 0x90, "thread allocation failure avoids freeze and patch");
    fixture::state.heap_alloc_fail = false;
    fixture::state.threads.pop_back();
    fixture::state.protect_fail = true;
    check(MH_EnableHook(target()) == MH_ERROR_MEMORY_PROTECT, "protection failure");
    check(target()[0] == 0x90 && fixture::state.flushes == 0, "protection failure leaves code unchanged");
    fixture::state.protect_fail = false;
    fixture::state.threads.push_back({11, 7, reinterpret_cast<std::uint64_t>(target()), false});
    fixture::state.threads.push_back({12, 7, reinterpret_cast<std::uint64_t>(target()), true, false});
    fixture::state.threads.push_back({13, 7, reinterpret_cast<std::uint64_t>(target()), true, true, false});
    check(MH_EnableHook(target()) == MH_OK, "failed fake thread open/suspend/context do not fail patch");
    check(fixture::state.threads[1].resumed == 0 && fixture::state.threads[2].resumed == 0, "unsuspended threads not resumed");
    check(fixture::state.threads[3].resumed == 1 && fixture::state.threads[3].writes == 0, "failed context still resumed");
    check(MH_RemoveHook(target()) == MH_OK && target()[0] == 0x90, "remove enabled hook restores bytes");
    end();
}
void relocations() {
    begin();
    auto* tr = fixture::buffers.data() + 128;
    TRAMPOLINE ct{}; ct.pTarget = target(); ct.pDetour = detour(); ct.pTrampoline = tr;
    const unsigned char rip[] = {0x48, 0x8b, 0x05, 0x20, 0x00, 0x00, 0x00};
    std::memcpy(target(), rip, sizeof rip);
    check(CreateTrampolineFunction(&ct), "actual RIP relative trampoline builder");
    check(tr[0] == 0x48 && tr[1] == 0x8b && tr[2] == 0x05, "RIP instruction retained");
    check(reinterpret_cast<std::intptr_t>(tr) + 7 + read<std::int32_t>(tr + 3) == reinterpret_cast<std::intptr_t>(target() + 7 + 0x20), "RIP operand absolute target preserved");
    check(ct.nIP == 2 && ct.oldIPs[1] == 7 && ct.newIPs[1] == 7, "RIP instruction boundaries");
    check(ct.pRelay == tr + 21, "RIP trampoline relay placement");
    std::fill_n(target(), 32, 0x90); target()[0] = 0xe8; write<std::int32_t>(target() + 1, 123);
    check(CreateTrampolineFunction(&ct), "actual CALL relocation");
    check(tr[0] == 0xff && tr[1] == 0x15 && tr[6] == 0xeb && tr[7] == 8, "CALL absolute expansion encoding");
    check(read<std::uint64_t>(tr + 8) == reinterpret_cast<std::uint64_t>(target() + 128), "CALL absolute destination");
    check(ct.oldIPs[1] == 5 && ct.newIPs[1] == 16, "CALL expanded instruction boundary");
    std::fill_n(target(), 32, 0x90); target()[0] = 0x75; target()[1] = 32;
    check(CreateTrampolineFunction(&ct), "actual conditional jump relocation");
    check(tr[0] == 0x74 && tr[1] == 14 && tr[2] == 0xff && tr[3] == 0x25, "conditional inverse/absolute jump encoding");
    check(read<std::uint64_t>(tr + 8) == reinterpret_cast<std::uint64_t>(target() + 34), "conditional destination");
    std::fill_n(target(), 32, 0x90); target()[0] = 0xeb; target()[1] = 32;
    check(CreateTrampolineFunction(&ct), "actual external short jump relocation");
    check(tr[0] == 0xff && tr[1] == 0x25 && read<std::uint64_t>(tr + 6) == reinterpret_cast<std::uint64_t>(target() + 34), "short jump absolute expansion");
    target()[0] = 0xe2; target()[1] = 32;
    check(!CreateTrampolineFunction(&ct), "external LOOP builder rejection");
    std::fill_n(target(), 32, 0x90); target()[0] = 0xeb; target()[1] = 2;
    check(CreateTrampolineFunction(&ct), "internal short jump copied");
    check(tr[0] == 0xeb && tr[1] == 2, "internal branch displacement unchanged");
    hde64s decoded{};
    check(hde64_disasm(rip, &decoded) == 7 && decoded.modrm == 5 && !(decoded.flags & F_ERROR), "actual HDE64 RIP decoding");
    std::array<unsigned char, 32> invalid; invalid.fill(0x66);
    hde64_disasm(invalid.data(), &decoded);
    check((decoded.flags & F_ERROR_LENGTH) != 0, "actual HDE64 excessive prefix rejection");
    end();
}
void capacity_and_api() {
    begin();
    for (unsigned i = 0; i < 80; ++i)
        check(MH_CreateHook(fixture::targets.data() + 128 + 32 * i, detour(), nullptr) == MH_OK, "multiple hooks entry/buffer growth");
    check(fixture::state.allocations >= 2, "actual buffer pool grows beyond one page");
    check(MH_QueueEnableHook(MH_ALL_HOOKS) == MH_OK && MH_ApplyQueued() == MH_OK, "queue all 80 hooks");
    for (unsigned i = 0; i < 80; ++i)
        check(fixture::targets[128 + 32 * i] == 0xe9, "each fixture patched");
    check(MH_Uninitialize() == MH_OK, "uninitialize restores all enabled targets");
    for (unsigned i = 0; i < 80; ++i)
        check(fixture::targets[128 + 32 * i] == 0x90, "uninitialize each fixture restored");
    check(fixture::live_heap_allocations() == 0 && fixture::live_buffer_blocks() == 0, "growth cleanup");
    begin();
    check(MH_CreateHookApi(L"synthetic", "fixture", detour(), nullptr) == MH_ERROR_MODULE_NOT_FOUND, "module lookup miss");
    fixture::state.module_found = true;
    check(MH_CreateHookApi(L"synthetic", "fixture", detour(), nullptr) == MH_ERROR_FUNCTION_NOT_FOUND, "symbol lookup miss");
    fixture::state.proc_found = true;
    void* resolved = nullptr;
    check(MH_CreateHookApiEx(L"synthetic", "fixture", detour(), nullptr, &resolved) == MH_OK && resolved == target(), "fake module/symbol resolved to owned fixture");
    end();
}
}
int main() {
    try {
        lifecycle(); patch_and_threads(); queued_hotpatch(); failures(); relocations(); capacity_and_api();
        std::cout << "hook_engine_fixture: " << checks << " checks passed; upstream C engine, fake Win32, data buffers only\n";
    } catch (const std::exception& e) {
        std::cerr << "hook_engine_fixture failure after " << checks << " checks: " << e.what() << '\n';
        return 1;
    }
}
