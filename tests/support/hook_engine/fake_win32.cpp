// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/hook_engine/fake_state.hpp"
#include "tests/support/hook_engine/fake_win32.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <unordered_set>

namespace fixture {
alignas(4096) std::array<unsigned char, 65536> targets{};
alignas(4096) std::array<unsigned char, 65536> buffers{};
State state;
static std::unordered_set<void*> heap_allocations;
static std::array<bool, 16> allocated_blocks{};
static std::size_t enumeration_index;
static bool heap_exists;
static void invariant(bool value) {
    if (!value) { std::cerr << "fake Win32 ownership invariant failed\n"; std::abort(); }
}
static bool contains(const void* p, std::size_t n, const void* base, std::size_t size) {
    const auto address = reinterpret_cast<std::uintptr_t>(p);
    const auto begin = reinterpret_cast<std::uintptr_t>(base);
    return address >= begin && address - begin <= size && n <= size - (address - begin);
}
bool owned(const void* p, std::size_t n) {
    return contains(p, n, targets.data(), targets.size()) ||
        contains(p, n, buffers.data(), buffers.size());
}
void reset() {
    invariant(!heap_exists && heap_allocations.empty() && live_buffer_blocks() == 0);
    state = {};
    targets.fill(0x90);
    buffers.fill(0);
    allocated_blocks.fill(false);
    enumeration_index = 0;
}
std::size_t live_heap_allocations() { return heap_allocations.size(); }
std::size_t live_buffer_blocks() { return std::count(allocated_blocks.begin(), allocated_blocks.end(), true); }
static Thread* thread(HANDLE h) {
    for (auto& t : state.threads)
        if (reinterpret_cast<std::uintptr_t>(h) == 0x10000u + t.id) return &t;
    invariant(false);
    return nullptr;
}
static void fill_thread(THREADENTRY32* result) {
    const auto& thread = state.threads[enumeration_index];
    result->dwSize = sizeof(*result);
    result->th32ThreadID = thread.id;
    result->th32OwnerProcessID = thread.process;
}
}

using namespace fixture;
extern "C" {
HANDLE WINAPI ReaTestHeapCreate(DWORD, SIZE_T, SIZE_T) {
    if (state.heap_create_fail) return nullptr;
    invariant(!heap_exists); heap_exists = true;
    return reinterpret_cast<HANDLE>(0x11);
}
LPVOID WINAPI ReaTestHeapAlloc(HANDLE h, DWORD, SIZE_T bytes) {
    invariant(h == reinterpret_cast<HANDLE>(0x11) && heap_exists);
    if (state.heap_alloc_fail) return nullptr;
    void* p = std::malloc(bytes);
    if (p) heap_allocations.insert(p);
    return p;
}
LPVOID WINAPI ReaTestHeapReAlloc(HANDLE h, DWORD, LPVOID p, SIZE_T bytes) {
    invariant(h == reinterpret_cast<HANDLE>(0x11) && heap_allocations.contains(p));
    if (state.heap_alloc_fail) return nullptr;
    heap_allocations.erase(p);
    void* result = std::realloc(p, bytes);
    if (result) heap_allocations.insert(result); else heap_allocations.insert(p);
    return result;
}
BOOL WINAPI ReaTestHeapFree(HANDLE h, DWORD, LPVOID p) {
    invariant(h == reinterpret_cast<HANDLE>(0x11));
    if (!p) return FALSE;
    invariant(heap_allocations.erase(p) == 1); std::free(p); return TRUE;
}
BOOL WINAPI ReaTestHeapDestroy(HANDLE h) {
    invariant(h == reinterpret_cast<HANDLE>(0x11) && heap_exists && heap_allocations.empty());
    heap_exists = false; return TRUE;
}
HANDLE WINAPI ReaTestCreateToolhelp32Snapshot(DWORD flags, DWORD process) {
    invariant(flags == TH32CS_SNAPTHREAD && process == 0); ++state.snapshots;
    return state.snapshot_fail ? INVALID_HANDLE_VALUE : reinterpret_cast<HANDLE>(0x22);
}
BOOL WINAPI ReaTestThread32First(HANDLE h, LPTHREADENTRY32 result) {
    invariant(h == reinterpret_cast<HANDLE>(0x22)); enumeration_index = 0;
    if (state.threads.empty()) return FALSE;
    fill_thread(result); return TRUE;
}
BOOL WINAPI ReaTestThread32Next(HANDLE h, LPTHREADENTRY32 result) {
    invariant(h == reinterpret_cast<HANDLE>(0x22));
    if (++enumeration_index >= state.threads.size()) {
        state.last_error = state.enumeration_fail ? ERROR_ACCESS_DENIED : ERROR_NO_MORE_FILES;
        return FALSE;
    }
    fill_thread(result); return TRUE;
}
DWORD WINAPI ReaTestGetCurrentProcessId(void) { return 7; }
DWORD WINAPI ReaTestGetCurrentThreadId(void) { return 10; }
HANDLE WINAPI ReaTestGetCurrentProcess(void) { return reinterpret_cast<HANDLE>(0x33); }
DWORD WINAPI ReaTestGetLastError(void) { return state.last_error; }
BOOL WINAPI ReaTestCloseHandle(HANDLE h) { invariant(h != nullptr && h != INVALID_HANDLE_VALUE); ++state.closes; return TRUE; }
HANDLE WINAPI ReaTestOpenThread(DWORD, BOOL inherit, DWORD id) {
    invariant(!inherit);
    for (auto& t : state.threads) if (t.id == id)
        return t.open_ok ? reinterpret_cast<HANDLE>(std::uintptr_t{0x10000u} + static_cast<std::uintptr_t>(id)) : nullptr;
    invariant(false); return nullptr;
}
DWORD WINAPI ReaTestSuspendThread(HANDLE h) {
    auto* t = thread(h); if (!t->suspend_ok) return 0xffffffff;
    ++t->suspended; return 0;
}
DWORD WINAPI ReaTestResumeThread(HANDLE h) { ++thread(h)->resumed; return 1; }
BOOL WINAPI ReaTestGetThreadContext(HANDLE h, LPCONTEXT result) {
    auto* t = thread(h); if (!t->context_ok) return FALSE;
    result->Rip = t->ip; return TRUE;
}
BOOL WINAPI ReaTestSetThreadContext(HANDLE h, const CONTEXT* value) {
    invariant(owned(reinterpret_cast<void*>(value->Rip), 1));
    auto* t = thread(h); t->ip = value->Rip; ++t->writes; return TRUE;
}
BOOL WINAPI ReaTestVirtualProtect(LPVOID p, SIZE_T size, DWORD, PDWORD old) {
    invariant(owned(p, size)); ++state.protects; state.last_patch_address = p; state.last_patch_size = size;
    if (state.protect_fail) return FALSE;
    *old = PAGE_EXECUTE_READ; return TRUE;
}
BOOL WINAPI ReaTestFlushInstructionCache(HANDLE h, LPCVOID p, SIZE_T size) {
    invariant(h == reinterpret_cast<HANDLE>(0x33) && owned(p, size)); ++state.flushes; return TRUE;
}
LPVOID WINAPI ReaTestVirtualAlloc(LPVOID, SIZE_T bytes, DWORD flags, DWORD protection) {
    invariant(bytes == 4096 && flags == (MEM_COMMIT | MEM_RESERVE) && protection == PAGE_EXECUTE_READWRITE);
    if (state.virtual_alloc_fail) return nullptr;
    for (std::size_t i = 0; i < allocated_blocks.size(); ++i) if (!allocated_blocks[i]) {
        allocated_blocks[i] = true; ++state.allocations; return buffers.data() + i * 4096;
    }
    return nullptr;
}
BOOL WINAPI ReaTestVirtualFree(LPVOID p, SIZE_T bytes, DWORD flags) {
    invariant(bytes == 0 && flags == MEM_RELEASE && contains(p, 4096, buffers.data(), buffers.size()));
    auto offset = static_cast<unsigned char*>(p) - buffers.data();
    invariant(offset % 4096 == 0 && allocated_blocks[offset / 4096]);
    allocated_blocks[offset / 4096] = false; ++state.frees; return TRUE;
}
SIZE_T WINAPI ReaTestVirtualQuery(LPCVOID p, PMEMORY_BASIC_INFORMATION result, SIZE_T bytes) {
    invariant(bytes == sizeof(*result));
    std::memset(result, 0, sizeof(*result));
    result->BaseAddress = reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(p) & ~std::uintptr_t(4095));
    result->AllocationBase = result->BaseAddress;
    result->RegionSize = 4096;
    result->State = owned(p, 1) ? MEM_COMMIT : MEM_FREE;
    result->Protect = owned(p, 1) ? PAGE_EXECUTE_READ : PAGE_NOACCESS;
    return sizeof(*result);
}
VOID WINAPI ReaTestGetSystemInfo(LPSYSTEM_INFO result) {
    std::memset(result, 0, sizeof(*result));
    auto lo = std::min(reinterpret_cast<std::uintptr_t>(targets.data()), reinterpret_cast<std::uintptr_t>(buffers.data()));
    auto hi = std::max(reinterpret_cast<std::uintptr_t>(targets.data()), reinterpret_cast<std::uintptr_t>(buffers.data()));
    result->lpMinimumApplicationAddress = reinterpret_cast<void*>(lo - 65536);
    result->lpMaximumApplicationAddress = reinterpret_cast<void*>(hi + 131072);
    result->dwAllocationGranularity = 4096;
}
HMODULE WINAPI ReaTestGetModuleHandleW(LPCWSTR) { return state.module_found ? reinterpret_cast<HMODULE>(0x44) : nullptr; }
FARPROC WINAPI ReaTestGetProcAddress(HMODULE h, LPCSTR) {
    invariant(h == reinterpret_cast<HMODULE>(0x44));
    return state.proc_found ? reinterpret_cast<FARPROC>(targets.data() + 128) : nullptr;
}
VOID WINAPI ReaTestSleep(DWORD) { invariant(false); }
LONG WINAPI ReaTestInterlockedCompareExchange(volatile LONG* p, LONG value, LONG compare) {
    LONG old = *p; if (old == compare) *p = value; return old;
}
LONG WINAPI ReaTestInterlockedExchange(volatile LONG* p, LONG value) { LONG old = *p; *p = value; return old; }
}
