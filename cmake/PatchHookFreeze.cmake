# SPDX-License-Identifier: GPL-3.0-only

if(NOT DEFINED MINHOOK_HOOK_C OR NOT EXISTS "${MINHOOK_HOOK_C}")
    message(FATAL_ERROR "MINHOOK_HOOK_C missing")
endif()
file(READ "${MINHOOK_HOOK_C}" _mh)
if(_mh MATCHES "CITRINE_ECLIPSE_MIRAGE_MH_SAFE_FREEZE_V1")
    return()
endif()

function(_replace_between start_marker end_marker replacement)
    string(FIND "${_mh}" "${start_marker}" _s)
    if(_s LESS 0)
        message(FATAL_ERROR "MinHook safe-freeze start marker not found: ${start_marker}")
    endif()
    string(FIND "${_mh}" "${end_marker}" _e)
    if(_e LESS 0 OR _e LESS _s)
        message(FATAL_ERROR "MinHook safe-freeze end marker not found: ${end_marker}")
    endif()
    string(LENGTH "${end_marker}" _el)
    math(EXPR _after "${_e}+${_el}")
    string(SUBSTRING "${_mh}" 0 ${_s} _prefix)
    string(LENGTH "${_mh}" _len)
    math(EXPR _tail_len "${_len}-${_after}")
    string(SUBSTRING "${_mh}" ${_after} ${_tail_len} _suffix)
    set(_mh "${_prefix}${replacement}${_suffix}" PARENT_SCOPE)
endfunction()

_replace_between(
"typedef struct _FROZEN_THREADS\n{"
"} FROZEN_THREADS, *PFROZEN_THREADS;"
[=[typedef struct _FROZEN_THREAD_ITEM
{
    HANDLE handle;
    BOOL   suspended;
} FROZEN_THREAD_ITEM, *PFROZEN_THREAD_ITEM;

typedef struct _FROZEN_THREADS
{
    PFROZEN_THREAD_ITEM pItems; // Exact physical owners retained until release succeeds.
    UINT                capacity;
    UINT                size;
} FROZEN_THREADS, *PFROZEN_THREADS;]=])

_replace_between(
"static VOID ProcessThreadIPs(HANDLE hThread, UINT pos, UINT action)"
"//-------------------------------------------------------------------------\nstatic BOOL EnumerateThreads(PFROZEN_THREADS pThreads)"
[=[typedef enum _PEER_PREPARATION_RESULT
{
    PEER_PREPARED = 0,
    PEER_TERMINATED = 1,
    PEER_UNRESOLVED = 2
} PEER_PREPARATION_RESULT;

static BOOL ThreadNoLongerLive(HANDLE hThread)
{
    DWORD exitCode = STILL_ACTIVE;
    if (GetExitCodeThread(hThread, &exitCode))
        return exitCode != STILL_ACTIVE;
    return GetLastError() == ERROR_INVALID_HANDLE;
}

static PEER_PREPARATION_RESULT ProcessThreadIPs(HANDLE hThread, UINT pos, UINT action)
{
    CONTEXT c;
#if defined(_M_X64) || defined(__x86_64__)
    DWORD64 *pIP = &c.Rip;
#else
    DWORD   *pIP = &c.Eip;
#endif
    UINT count;

    c.ContextFlags = CONTEXT_CONTROL;
    if (!GetThreadContext(hThread, &c))
        return ThreadNoLongerLive(hThread) ? PEER_TERMINATED : PEER_UNRESOLVED;

    if (pos == ALL_HOOKS_POS)
    {
        pos = 0;
        count = g_hooks.size;
    }
    else
    {
        count = pos + 1;
    }

    for (; pos < count; ++pos)
    {
        PHOOK_ENTRY pHook = &g_hooks.pItems[pos];
        BOOL        enable;
        DWORD_PTR   ip;

        switch (action)
        {
        case ACTION_DISABLE:
            enable = FALSE;
            break;
        case ACTION_ENABLE:
            enable = TRUE;
            break;
        default:
            enable = pHook->queueEnable;
            break;
        }
        if (pHook->isEnabled == enable)
            continue;

        if (enable)
            ip = FindNewIP(pHook, *pIP);
        else
            ip = FindOldIP(pHook, *pIP);

        if (ip != 0)
        {
            *pIP = ip;
            if (!SetThreadContext(hThread, &c))
                return ThreadNoLongerLive(hThread) ? PEER_TERMINATED : PEER_UNRESOLVED;
        }
    }
    return PEER_PREPARED;
}

//-------------------------------------------------------------------------
static BOOL EnumerateThreads(PFROZEN_THREADS pThreads)]=])

_replace_between(
"static BOOL EnumerateThreads(PFROZEN_THREADS pThreads)"
"//-------------------------------------------------------------------------\nstatic MH_STATUS Freeze"
[=[/* CITRINE_ECLIPSE_MIRAGE_MH_SAFE_FREEZE_V1
 *
 * Stock MinHook enumerates peer threads with CreateToolhelp32Snapshot.
 * In the target process that snapshot path can stall after loader startup.
 * Keep MinHook's suspend/context/IP-relocation contract, but enumerate stable
 * thread handles with NtGetNextThread instead. This code runs only post-loader.
 *
 * HF07 ownership rule: every duplicated peer HANDLE and the backing heap array
 * remain discoverable until ResumeThread/CloseHandle/HeapFree physically
 * succeed or the handle is proven terminated/invalid. A later MinHook operation
 * retries retained cleanup before acquiring a new peer-thread set.
 */
typedef LONG NTSTATUS;
typedef NTSTATUS (NTAPI *PFN_NT_GET_NEXT_THREAD)(
    HANDLE, HANDLE, ACCESS_MASK, ULONG, ULONG, PHANDLE);
#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) (((NTSTATUS)(Status)) >= 0)
#endif
#ifndef STATUS_NO_MORE_ENTRIES
#define STATUS_NO_MORE_ENTRIES ((NTSTATUS)0x8000001AL)
#endif

static FROZEN_THREADS g_retainedThreads = { NULL, 0, 0 };
static FROZEN_THREAD_ITEM g_emergencyThread = { NULL, FALSE };
static FROZEN_THREAD_ITEM g_emergencyNextThread = { NULL, FALSE };

static PFN_NT_GET_NEXT_THREAD ResolveNtGetNextThread(VOID)
{
    HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
    if (ntdll == NULL)
        return NULL;
    return (PFN_NT_GET_NEXT_THREAD)GetProcAddress(ntdll, "NtGetNextThread");
}

static BOOL AppendThreadHandle(PFROZEN_THREADS pThreads, HANDLE hThread)
{
    if (pThreads->pItems == NULL)
    {
        pThreads->capacity = INITIAL_THREAD_CAPACITY;
        pThreads->pItems = (PFROZEN_THREAD_ITEM)HeapAlloc(
            g_hHeap, HEAP_ZERO_MEMORY,
            pThreads->capacity * sizeof(FROZEN_THREAD_ITEM));
        if (pThreads->pItems == NULL)
            return FALSE;
    }
    else if (pThreads->size >= pThreads->capacity)
    {
        PFROZEN_THREAD_ITEM p;
        UINT oldCapacity = pThreads->capacity;
        pThreads->capacity *= 2;
        p = (PFROZEN_THREAD_ITEM)HeapReAlloc(
            g_hHeap, HEAP_ZERO_MEMORY, pThreads->pItems,
            pThreads->capacity * sizeof(FROZEN_THREAD_ITEM));
        if (p == NULL)
        {
            pThreads->capacity = oldCapacity;
            return FALSE;
        }
        pThreads->pItems = p;
    }
    pThreads->pItems[pThreads->size].handle = hThread;
    pThreads->pItems[pThreads->size].suspended = FALSE;
    ++pThreads->size;
    return TRUE;
}

static BOOL ResumeOwnedThread(PFROZEN_THREAD_ITEM pItem)
{
    DWORD resumeResult;
    if (pItem->handle == NULL || !pItem->suspended)
        return TRUE;
    if (ThreadNoLongerLive(pItem->handle))
    {
        pItem->suspended = FALSE;
        return TRUE;
    }
    resumeResult = ResumeThread(pItem->handle);
    if (resumeResult != 0xFFFFFFFF)
    {
        pItem->suspended = FALSE;
        return TRUE;
    }
    if (GetLastError() == ERROR_INVALID_HANDLE || ThreadNoLongerLive(pItem->handle))
    {
        pItem->suspended = FALSE;
        return TRUE;
    }
    return FALSE;
}

static BOOL CloseOwnedThread(PFROZEN_THREAD_ITEM pItem)
{
    if (pItem->handle == NULL)
        return TRUE;
    if (CloseHandle(pItem->handle))
    {
        pItem->handle = NULL;
        return TRUE;
    }
    if (GetLastError() == ERROR_INVALID_HANDLE)
    {
        pItem->handle = NULL;
        return TRUE;
    }
    return FALSE;
}

static BOOL CloseOrRetainEmergency(HANDLE hThread)
{
    FROZEN_THREAD_ITEM item = { hThread, FALSE };
    if (CloseOwnedThread(&item))
        return TRUE;
    if (g_emergencyThread.handle == NULL)
    {
        g_emergencyThread = item;
        return FALSE;
    }
    if (g_emergencyNextThread.handle == NULL)
    {
        g_emergencyNextThread = item;
        return FALSE;
    }
    return FALSE;
}

static BOOL ReleaseEmergencyThreadOwnership(VOID)
{
    // One failed cursor-close can coexist with a newly returned next handle.
    // Keep two physically independent emergency owner slots until both close.
    BOOL nextReleased = ResumeOwnedThread(&g_emergencyNextThread);
    if (nextReleased)
        nextReleased = CloseOwnedThread(&g_emergencyNextThread);
    BOOL cursorReleased = ResumeOwnedThread(&g_emergencyThread);
    if (cursorReleased)
        cursorReleased = CloseOwnedThread(&g_emergencyThread);
    return nextReleased && cursorReleased;
}

static BOOL ReleaseFrozenThreadOwnership(PFROZEN_THREADS pThreads)
{
    UINT i;
    BOOL released = TRUE;
    if (pThreads->pItems == NULL)
        return TRUE;
    for (i = 0; i < pThreads->size; ++i)
    {
        PFROZEN_THREAD_ITEM pItem = &pThreads->pItems[i];
        if (!ResumeOwnedThread(pItem))
        {
            released = FALSE;
            continue;
        }
        if (!CloseOwnedThread(pItem))
            released = FALSE;
    }
    if (!released)
        return FALSE;
    if (!HeapFree(g_hHeap, 0, pThreads->pItems))
        return FALSE;
    pThreads->pItems = NULL;
    pThreads->size = 0;
    pThreads->capacity = 0;
    return TRUE;
}

static BOOL PreserveFrozenThreadOwnership(PFROZEN_THREADS pThreads)
{
    if (ReleaseFrozenThreadOwnership(pThreads))
        return TRUE;
    if (g_retainedThreads.pItems != NULL)
        return FALSE; /* Freeze drains this before creating a new owner set. */
    g_retainedThreads = *pThreads;
    pThreads->pItems = NULL;
    pThreads->size = 0;
    pThreads->capacity = 0;
    return FALSE;
}

static BOOL RetryRetainedThreadOwnership(VOID)
{
    if (!ReleaseEmergencyThreadOwnership())
        return FALSE;
    return ReleaseFrozenThreadOwnership(&g_retainedThreads);
}

static BOOL HasRetainedThreadOwnership(VOID)
{
    return g_emergencyThread.handle != NULL || g_emergencyNextThread.handle != NULL || g_retainedThreads.pItems != NULL;
}

typedef enum _THREAD_ENUMERATION_RESULT
{
    THREAD_ENUMERATION_OK = 0,
    THREAD_ENUMERATION_ALLOC_FAILED = 1,
    THREAD_ENUMERATION_API_FAILED = 2,
    THREAD_ENUMERATION_OWNERSHIP_FAILED = 3
} THREAD_ENUMERATION_RESULT;

static THREAD_ENUMERATION_RESULT EnumerateThreads(PFROZEN_THREADS pThreads)
{
    PFN_NT_GET_NEXT_THREAD getNextThread = ResolveNtGetNextThread();
    HANDLE process = GetCurrentProcess();
    HANDLE cursor = NULL;
    DWORD currentThreadId = GetCurrentThreadId();
    if (getNextThread == NULL)
        return THREAD_ENUMERATION_API_FAILED;

    for (;;)
    {
        HANDLE next = NULL;
        NTSTATUS status = getNextThread(
            process, cursor, THREAD_ACCESS, 0, 0, &next);
        if (cursor != NULL && !CloseOrRetainEmergency(cursor))
        {
            // NtGetNextThread has already returned next. The failed cursor
            // close consumes the first emergency owner slot; retain next in
            // the second slot, even if it has not been suspended yet.
            if (NT_SUCCESS(status) && next != NULL)
                g_emergencyNextThread.handle = next;
            cursor = NULL;
            (void)PreserveFrozenThreadOwnership(pThreads);
            return THREAD_ENUMERATION_OWNERSHIP_FAILED;
        }
        cursor = NULL;
        if (!NT_SUCCESS(status))
        {
            if (status == STATUS_NO_MORE_ENTRIES)
                break;
            return PreserveFrozenThreadOwnership(pThreads)
                ? THREAD_ENUMERATION_API_FAILED
                : THREAD_ENUMERATION_OWNERSHIP_FAILED;
        }
        cursor = next;

        if (GetThreadId(next) != currentThreadId)
        {
            HANDLE owned = NULL;
            if (!DuplicateHandle(
                    process, next, process, &owned, 0, FALSE, DUPLICATE_SAME_ACCESS))
            {
                if (cursor != NULL && !CloseOrRetainEmergency(cursor))
                    cursor = NULL;
                return PreserveFrozenThreadOwnership(pThreads)
                    ? THREAD_ENUMERATION_API_FAILED
                    : THREAD_ENUMERATION_OWNERSHIP_FAILED;
            }
            if (!AppendThreadHandle(pThreads, owned))
            {
                BOOL cleanupOk = CloseOrRetainEmergency(owned);
                if (cursor != NULL && !CloseOrRetainEmergency(cursor))
                    cleanupOk = FALSE;
                if (!PreserveFrozenThreadOwnership(pThreads))
                    cleanupOk = FALSE;
                return cleanupOk
                    ? THREAD_ENUMERATION_ALLOC_FAILED
                    : THREAD_ENUMERATION_OWNERSHIP_FAILED;
            }
        }
    }
    if (cursor != NULL && !CloseOrRetainEmergency(cursor))
    {
        (void)PreserveFrozenThreadOwnership(pThreads);
        return THREAD_ENUMERATION_OWNERSHIP_FAILED;
    }
    return THREAD_ENUMERATION_OK;
}

//-------------------------------------------------------------------------
static MH_STATUS Freeze]=])

_replace_between(
"static MH_STATUS Freeze(PFROZEN_THREADS pThreads, UINT pos, UINT action)"
"//-------------------------------------------------------------------------\nstatic VOID Unfreeze"
[=[static MH_STATUS Freeze(PFROZEN_THREADS pThreads, UINT pos, UINT action)
{
    MH_STATUS status = MH_OK;
    pThreads->pItems   = NULL;
    pThreads->capacity = 0;
    pThreads->size     = 0;

    if (!RetryRetainedThreadOwnership())
        return MH_ERROR_MEMORY_PROTECT;
    {
        THREAD_ENUMERATION_RESULT enumeration = EnumerateThreads(pThreads);
        if (enumeration != THREAD_ENUMERATION_OK)
        {
            if (enumeration == THREAD_ENUMERATION_ALLOC_FAILED)
                return MH_ERROR_MEMORY_ALLOC;
            if (enumeration == THREAD_ENUMERATION_OWNERSHIP_FAILED ||
                HasRetainedThreadOwnership())
                return MH_ERROR_MEMORY_PROTECT;
            // Audit3 S0179: OS/API enumeration failure is not allocation
            // failure. Preserve MinHook's public ABI and report its generic
            // non-allocation failure status rather than lying about memory.
            return MH_UNKNOWN;
        }
    }
    if (pThreads->pItems != NULL)
    {
        UINT i;
        for (i = 0; i < pThreads->size; ++i)
        {
            PFROZEN_THREAD_ITEM pItem = &pThreads->pItems[i];
            if (pItem->handle == NULL)
                continue;
            if (SuspendThread(pItem->handle) != 0xFFFFFFFF)
            {
                PEER_PREPARATION_RESULT peerResult;
                pItem->suspended = TRUE;
                peerResult = ProcessThreadIPs(pItem->handle, pos, action);
                if (peerResult == PEER_UNRESOLVED)
                    status = MH_ERROR_MEMORY_PROTECT;
            }
            else if (!ThreadNoLongerLive(pItem->handle))
            {
                status = MH_ERROR_MEMORY_PROTECT;
            }
        }
    }
    if (status != MH_OK)
        (void)PreserveFrozenThreadOwnership(pThreads);
    return status;
}

//-------------------------------------------------------------------------
static VOID Unfreeze]=])

_replace_between(
"static VOID Unfreeze(PFROZEN_THREADS pThreads)"
"//-------------------------------------------------------------------------\nstatic MH_STATUS EnableHookLL"
[=[static MH_STATUS Unfreeze(PFROZEN_THREADS pThreads)
{
    return PreserveFrozenThreadOwnership(pThreads)
        ? MH_OK
        : MH_ERROR_MEMORY_PROTECT;
}

static MH_STATUS FinishFreeze(PFROZEN_THREADS pThreads, MH_STATUS status)
{
    MH_STATUS cleanupStatus = Unfreeze(pThreads);
    return status == MH_OK ? cleanupStatus : status;
}

//-------------------------------------------------------------------------
static MH_STATUS EnableHookLL]=])


string(REPLACE "Unfreeze(&threads);" "status = FinishFreeze(&threads, status);" _mh "${_mh}")
string(REPLACE
"static MH_STATUS EnableAllHooksLL(BOOL enable)\n{\n    MH_STATUS status = MH_OK;"
"static MH_STATUS EnableAllHooksLL(BOOL enable)\n{\n    MH_STATUS status = MH_OK;\n    if (!RetryRetainedThreadOwnership())\n        return MH_ERROR_MEMORY_PROTECT;"
_mh "${_mh}")

# Individual MH_DisableHook must map suspended peer IPs from trampoline back
# to target. Stock MinHook incorrectly requested ACTION_ENABLE for both modes.
# Replace only the proven single-hook call in the owned build copy.
string(REGEX MATCHALL [=[status = Freeze\(&threads, pos, ACTION_ENABLE\)]=]
    _individual_actions "${_mh}")
list(LENGTH _individual_actions _individual_action_count)
if(NOT _individual_action_count EQUAL 1)
    message(FATAL_ERROR "MinHook single-hook action anchor must occur exactly once")
endif()
string(REPLACE
    "status = Freeze(&threads, pos, ACTION_ENABLE);"
    "status = Freeze(&threads, pos, enable ? ACTION_ENABLE : ACTION_DISABLE);"
    _mh "${_mh}")

file(WRITE "${MINHOOK_HOOK_C}" "${_mh}")
