// SPDX-License-Identifier: GPL-3.0-only
#pragma once
// Test-only declaration subset. No Windows ABI/layout or SEH emulation claim.
#include <cstddef>
#include <cstdint>
using DWORD=std::uint32_t;using BOOL=int;using HANDLE=void*;using HMODULE=void*;
using LPCWSTR=const wchar_t*;
inline const HANDLE INVALID_HANDLE_VALUE=reinterpret_cast<HANDLE>(std::intptr_t(-1));
inline constexpr BOOL FALSE=0;
inline constexpr DWORD GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS=4;
inline constexpr DWORD GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT=2;
inline constexpr DWORD MEM_COMMIT=0x1000,PAGE_NOACCESS=1,PAGE_GUARD=0x100;
inline constexpr DWORD THREAD_SUSPEND_RESUME=2,THREAD_GET_CONTEXT=8,THREAD_QUERY_INFORMATION=0x40;
inline constexpr DWORD ERROR_INVALID_PARAMETER=87,ERROR_INVALID_HANDLE=6,ERROR_NO_MORE_FILES=18;
inline constexpr DWORD CONTEXT_FULL=0x10000b;
struct CONTEXT {
 std::uint64_t Rip{},Rax{},Rbx{},Rcx{},Rdx{},Rsi{},Rdi{},Rbp{},R8{},R9{},R10{},R11{},R12{},R13{},R14{},R15{},Rsp{};
 DWORD ContextFlags{};
};
struct MEMORY_BASIC_INFORMATION {
 void* BaseAddress{};void* AllocationBase{};std::size_t RegionSize{};DWORD State{},Protect{};
};
struct IMAGE_DOS_HEADER {std::int32_t e_lfanew{};};
struct IMAGE_NT_HEADERS64 {struct {DWORD SizeOfImage{};} OptionalHeader;};
std::size_t VirtualQuery(const void*,MEMORY_BASIC_INFORMATION*,std::size_t);
BOOL GetModuleHandleExW(DWORD,LPCWSTR,HMODULE*);
DWORD GetCurrentThreadId();DWORD GetCurrentProcessId();DWORD GetLastError();
HANDLE OpenThread(DWORD,BOOL,DWORD);DWORD SuspendThread(HANDLE);
BOOL GetThreadContext(HANDLE,CONTEXT*);DWORD ResumeThread(HANDLE);BOOL CloseHandle(HANDLE);
