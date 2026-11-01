// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <windows.h>
inline constexpr DWORD TH32CS_SNAPTHREAD=4;
struct THREADENTRY32 {DWORD dwSize{},th32ThreadID{},th32OwnerProcessID{};};
HANDLE CreateToolhelp32Snapshot(DWORD,DWORD);
BOOL Thread32First(HANDLE,THREADENTRY32*);BOOL Thread32Next(HANDLE,THREADENTRY32*);
