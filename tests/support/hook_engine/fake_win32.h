// SPDX-License-Identifier: GPL-3.0-only

#ifndef REA_TEST_FAKE_WIN32_H
#define REA_TEST_FAKE_WIN32_H
/* Test-only API seam: include SDK declarations before redirecting call sites.
 * Never include this file in the production engine build. */
#include <windows.h>
#include <tlhelp32.h>
#ifdef __cplusplus
extern "C" {
#endif
HANDLE WINAPI ReaTestHeapCreate(DWORD, SIZE_T, SIZE_T);
LPVOID WINAPI ReaTestHeapAlloc(HANDLE, DWORD, SIZE_T);
LPVOID WINAPI ReaTestHeapReAlloc(HANDLE, DWORD, LPVOID, SIZE_T);
BOOL WINAPI ReaTestHeapFree(HANDLE, DWORD, LPVOID);
BOOL WINAPI ReaTestHeapDestroy(HANDLE);
HANDLE WINAPI ReaTestCreateToolhelp32Snapshot(DWORD, DWORD);
BOOL WINAPI ReaTestThread32First(HANDLE, LPTHREADENTRY32);
BOOL WINAPI ReaTestThread32Next(HANDLE, LPTHREADENTRY32);
DWORD WINAPI ReaTestGetCurrentProcessId(void);
DWORD WINAPI ReaTestGetCurrentThreadId(void);
HANDLE WINAPI ReaTestGetCurrentProcess(void);
DWORD WINAPI ReaTestGetLastError(void);
BOOL WINAPI ReaTestCloseHandle(HANDLE);
HANDLE WINAPI ReaTestOpenThread(DWORD, BOOL, DWORD);
DWORD WINAPI ReaTestSuspendThread(HANDLE);
DWORD WINAPI ReaTestResumeThread(HANDLE);
BOOL WINAPI ReaTestGetThreadContext(HANDLE, LPCONTEXT);
BOOL WINAPI ReaTestSetThreadContext(HANDLE, const CONTEXT*);
BOOL WINAPI ReaTestVirtualProtect(LPVOID, SIZE_T, DWORD, PDWORD);
BOOL WINAPI ReaTestFlushInstructionCache(HANDLE, LPCVOID, SIZE_T);
LPVOID WINAPI ReaTestVirtualAlloc(LPVOID, SIZE_T, DWORD, DWORD);
BOOL WINAPI ReaTestVirtualFree(LPVOID, SIZE_T, DWORD);
SIZE_T WINAPI ReaTestVirtualQuery(LPCVOID, PMEMORY_BASIC_INFORMATION, SIZE_T);
VOID WINAPI ReaTestGetSystemInfo(LPSYSTEM_INFO);
HMODULE WINAPI ReaTestGetModuleHandleW(LPCWSTR);
FARPROC WINAPI ReaTestGetProcAddress(HMODULE, LPCSTR);
VOID WINAPI ReaTestSleep(DWORD);
LONG WINAPI ReaTestInterlockedCompareExchange(volatile LONG*, LONG, LONG);
LONG WINAPI ReaTestInterlockedExchange(volatile LONG*, LONG);
#ifdef __cplusplus
}
#endif
#define HeapCreate ReaTestHeapCreate
#define HeapAlloc ReaTestHeapAlloc
#define HeapReAlloc ReaTestHeapReAlloc
#define HeapFree ReaTestHeapFree
#define HeapDestroy ReaTestHeapDestroy
#define CreateToolhelp32Snapshot ReaTestCreateToolhelp32Snapshot
#define Thread32First ReaTestThread32First
#define Thread32Next ReaTestThread32Next
#define GetCurrentProcessId ReaTestGetCurrentProcessId
#define GetCurrentThreadId ReaTestGetCurrentThreadId
#define GetCurrentProcess ReaTestGetCurrentProcess
#define GetLastError ReaTestGetLastError
#define CloseHandle ReaTestCloseHandle
#define OpenThread ReaTestOpenThread
#define SuspendThread ReaTestSuspendThread
#define ResumeThread ReaTestResumeThread
#define GetThreadContext ReaTestGetThreadContext
#define SetThreadContext ReaTestSetThreadContext
#define VirtualProtect ReaTestVirtualProtect
#define FlushInstructionCache ReaTestFlushInstructionCache
#define VirtualAlloc ReaTestVirtualAlloc
#define VirtualFree ReaTestVirtualFree
#define VirtualQuery ReaTestVirtualQuery
#define GetSystemInfo ReaTestGetSystemInfo
#define GetModuleHandleW ReaTestGetModuleHandleW
#define GetProcAddress ReaTestGetProcAddress
#define Sleep ReaTestSleep
#ifdef InterlockedCompareExchange
#undef InterlockedCompareExchange
#endif
#ifdef InterlockedExchange
#undef InterlockedExchange
#endif
#define InterlockedCompareExchange ReaTestInterlockedCompareExchange
#define InterlockedExchange ReaTestInterlockedExchange
#endif
