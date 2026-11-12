// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/platform/execution_windows.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <exception>

namespace citrine_eclipse_mirage::execution {
std::uint32_t WindowsPlatform::current_thread_id() { return GetCurrentThreadId(); }
void WindowsPlatform::acquire_srw_exclusive(std::uintptr_t& lock) {
    static_assert(sizeof(SRWLOCK) == sizeof(lock));
    AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock));
}
void WindowsPlatform::release_srw_exclusive(std::uintptr_t& lock) {
    ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&lock));
}
std::int64_t WindowsPlatform::query_performance_frequency() {
    LARGE_INTEGER value;
    QueryPerformanceFrequency(&value); // original ignores BOOL result
    return value.QuadPart;
}
std::int64_t WindowsPlatform::query_performance_counter() {
    LARGE_INTEGER value;
    QueryPerformanceCounter(&value); // original ignores BOOL result
    return value.QuadPart;
}
void WindowsPlatform::sleep_milliseconds(std::uint32_t milliseconds) { Sleep(milliseconds); }
[[noreturn]] void WindowsPlatform::raise_thread_error_04a648(int index) {
    error_raiser_(index);
    std::terminate(); // returning violates the required noreturn ABI contract
}
}
