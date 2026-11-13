# SPDX-License-Identifier: GPL-3.0-only
# Included by the pinned freeze transform; operates only on its staged copy.

_replace_between("static FROZEN_THREADS g_retainedThreads" "static FROZEN_THREADS g_retainedThreads" [=[
/* Completion records outlive failed API calls. No allocation is needed while
 * peers are suspended. A seven-byte hotpatch can touch at most two pages. */
typedef struct _PATCH_PAGE
{
    LPVOID address;
    DWORD protection;
    BOOL pending;
} PATCH_PAGE;
static struct
{
    BOOL active, cachePending;
    LPBYTE target;
    SIZE_T size, pageSize;
    UINT count;
    PATCH_PAGE pages[2];
} g_patchCompletion;
static LPVOID g_retainedBuffer;
static BOOL g_shutdownPending;
static LPVOID g_retiringBuffer;
#define ACTION_RETIRE 3

static BOOL RetryPatchCompletion(VOID)
{
    UINT i;
    BOOL complete = TRUE;
    if (!g_patchCompletion.active)
        return TRUE;
    if (g_patchCompletion.cachePending)
    {
        if (FlushInstructionCache(GetCurrentProcess(), g_patchCompletion.target, g_patchCompletion.size))
            g_patchCompletion.cachePending = FALSE;
        else
            complete = FALSE;
    }
    for (i = 0; i < g_patchCompletion.count; ++i)
    {
        PATCH_PAGE *page = &g_patchCompletion.pages[i];
        if (page->pending)
        {
            DWORD ignored;
            if (VirtualProtect(page->address, g_patchCompletion.pageSize, page->protection, &ignored))
                page->pending = FALSE;
            else
                complete = FALSE;
        }
    }
    if (complete)
        memset(&g_patchCompletion, 0, sizeof(g_patchCompletion));
    return complete;
}

static BOOL IsRetiringAddress(DWORD64 address)
{
    if (g_retiringBuffer != NULL)
        return address >= (ULONG_PTR)g_retiringBuffer &&
            address - (ULONG_PTR)g_retiringBuffer < MEMORY_SLOT_SIZE;
    return IsBufferAddress((ULONG_PTR)address);
}

static PEER_PREPARATION_RESULT CheckRetiringThread(HANDLE thread)
{
    CONTEXT context;
    MEMORY_BASIC_INFORMATION first;
    ULONG_PTR cursor, allocation;
    SIZE_T scanned = 0;
    DWORD64 registers[17];
    UINT i;
    context.ContextFlags = CONTEXT_FULL;
    if (!GetThreadContext(thread, &context))
        return ThreadNoLongerLive(thread) ? PEER_TERMINATED : PEER_UNRESOLVED;
    registers[0]=context.Rip; registers[1]=context.Rax; registers[2]=context.Rbx;
    registers[3]=context.Rcx; registers[4]=context.Rdx; registers[5]=context.Rsi;
    registers[6]=context.Rdi; registers[7]=context.Rbp; registers[8]=context.Rsp;
    registers[9]=context.R8; registers[10]=context.R9; registers[11]=context.R10;
    registers[12]=context.R11; registers[13]=context.R12; registers[14]=context.R13;
    registers[15]=context.R14; registers[16]=context.R15;
    for (i = 0; i < ARRAYSIZE(registers); ++i)
        if (IsRetiringAddress(registers[i]))
            return PEER_UNRESOLVED;
    cursor = (ULONG_PTR)context.Rsp;
    if (!VirtualQuery((LPCVOID)cursor, &first, sizeof(first)))
        return PEER_UNRESOLVED;
    allocation = (ULONG_PTR)first.AllocationBase;
    while (scanned < 64 * 1024 * 1024)
    {
        MEMORY_BASIC_INFORMATION region;
        ULONG_PTR limit;
        if (!VirtualQuery((LPCVOID)cursor, &region, sizeof(region)))
            return PEER_UNRESOLVED;
        if ((ULONG_PTR)region.AllocationBase != allocation)
            return PEER_PREPARED;
        limit = (ULONG_PTR)region.BaseAddress + region.RegionSize;
        if (limit <= cursor || limit - cursor > 64 * 1024 * 1024 - scanned ||
            region.State != MEM_COMMIT || (region.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
            return PEER_UNRESOLVED;
        while (cursor < limit)
        {
            DWORD64 words[64];
            SIZE_T copied = 0, bytes = limit - cursor;
            if (bytes > sizeof(words)) bytes = sizeof(words);
            if (bytes % sizeof(words[0]) != 0 ||
                !ReadProcessMemory(GetCurrentProcess(), (LPCVOID)cursor, words, bytes, &copied) || copied != bytes)
                return PEER_UNRESOLVED;
            for (i = 0; i < bytes / sizeof(words[0]); ++i)
                if (IsRetiringAddress(words[i]))
                    return PEER_UNRESOLVED;
            cursor += bytes;
            scanned += bytes;
        }
    }
    return PEER_UNRESOLVED;
}

static FROZEN_THREADS g_retainedThreads]=])

_replace_between("static BOOL RetryRetainedThreadOwnership(VOID)" "static BOOL HasRetainedThreadOwnership(VOID)" [=[static BOOL RetryRetainedThreadOwnership(VOID)
{
    /* Never resume peers into an instruction stream whose completion failed. */
    if (!RetryPatchCompletion())
        return FALSE;
    if (!ReleaseEmergencyThreadOwnership() || !ReleaseFrozenThreadOwnership(&g_retainedThreads))
        return FALSE;
    if (g_retainedBuffer != NULL)
    {
        if (!FreeBuffer(g_retainedBuffer))
            return FALSE;
        g_retainedBuffer = NULL;
    }
    return TRUE;
}

static BOOL HasRetainedThreadOwnership(VOID)]=])

string(REPLACE "peerResult = ProcessThreadIPs(pItem->handle, pos, action);"
    "peerResult = action == ACTION_RETIRE ? CheckRetiringThread(pItem->handle) : ProcessThreadIPs(pItem->handle, pos, action);" _mh "${_mh}")

_replace_between("static MH_STATUS FinishFreeze(PFROZEN_THREADS pThreads, MH_STATUS status)" "static MH_STATUS EnableHookLL" [=[static MH_STATUS FinishFreeze(PFROZEN_THREADS pThreads, MH_STATUS status)
{
    MH_STATUS cleanupStatus;
    if (g_patchCompletion.active)
    {
        /* Freeze entered with no retained set. Transfer without resuming. */
        g_retainedThreads = *pThreads;
        memset(pThreads, 0, sizeof(*pThreads));
        return MH_ERROR_MEMORY_PROTECT;
    }
    cleanupStatus = Unfreeze(pThreads);
    return status == MH_OK ? cleanupStatus : status;
}

static MH_STATUS RetireHookEntry(UINT pos)
{
    FROZEN_THREADS threads;
    MH_STATUS status;
    g_retiringBuffer = g_hooks.pItems[pos].pTrampoline;
    status = Freeze(&threads, ALL_HOOKS_POS, ACTION_RETIRE);
    if (status == MH_OK)
    {
        if (FreeBuffer(g_hooks.pItems[pos].pTrampoline))
            DeleteHookEntry(pos);
        else
            status = MH_ERROR_MEMORY_PROTECT;
        status = FinishFreeze(&threads, status);
    }
    g_retiringBuffer = NULL;
    return status;
}

static MH_STATUS RetireAllBuffers(VOID)
{
    FROZEN_THREADS threads;
    MH_STATUS status;
    g_retiringBuffer = NULL;
    status = Freeze(&threads, ALL_HOOKS_POS, ACTION_RETIRE);
    if (status == MH_OK)
    {
        /* Some blocks may be released before a later release fails. */
        g_shutdownPending = TRUE;
        if (!UninitializeBuffer())
            status = MH_ERROR_MEMORY_PROTECT;
        status = FinishFreeze(&threads, status);
    }
    return status;
}

static MH_STATUS EnableHookLL]=])

_replace_between("static MH_STATUS EnableHookLL(UINT pos, BOOL enable)" "static MH_STATUS EnableAllHooksLL" [=[static MH_STATUS EnableHookLL(UINT pos, BOOL enable)
{
    PHOOK_ENTRY pHook = &g_hooks.pItems[pos];
    SIZE_T patchSize = sizeof(JMP_REL);
    LPBYTE target = (LPBYTE)pHook->pTarget;
    SYSTEM_INFO info;
    ULONG_PTR first, last;
    UINT i;
    if (g_patchCompletion.active)
        return MH_ERROR_MEMORY_PROTECT;
    if (pHook->patchAbove)
    {
        target -= sizeof(JMP_REL);
        patchSize += sizeof(JMP_REL_SHORT);
    }
    GetSystemInfo(&info);
    if (!info.dwPageSize || patchSize > info.dwPageSize || (ULONG_PTR)target > (ULONG_PTR)-1 - patchSize)
        return MH_ERROR_MEMORY_PROTECT;
    first = (ULONG_PTR)target / info.dwPageSize * info.dwPageSize;
    last = ((ULONG_PTR)target + patchSize - 1) / info.dwPageSize * info.dwPageSize;
    memset(&g_patchCompletion, 0, sizeof(g_patchCompletion));
    g_patchCompletion.active = TRUE;
    g_patchCompletion.target = target;
    g_patchCompletion.size = patchSize;
    g_patchCompletion.pageSize = info.dwPageSize;
    g_patchCompletion.count = first == last ? 1 : 2;
    for (i = 0; i < g_patchCompletion.count; ++i)
    {
        PATCH_PAGE *page = &g_patchCompletion.pages[i];
        page->address = (LPVOID)(first + i * info.dwPageSize);
        if (!VirtualProtect(page->address, info.dwPageSize, PAGE_EXECUTE_READWRITE, &page->protection))
        {
            (void)RetryPatchCompletion();
            return MH_ERROR_MEMORY_PROTECT;
        }
        page->pending = TRUE;
    }
    if (enable)
    {
        PJMP_REL jump = (PJMP_REL)target;
        jump->opcode = 0xE9;
        jump->operand = (INT32)((LPBYTE)pHook->pDetour - (target + sizeof(JMP_REL)));
        if (pHook->patchAbove)
        {
            PJMP_REL_SHORT shortJump = (PJMP_REL_SHORT)pHook->pTarget;
            shortJump->opcode = 0xEB;
            shortJump->operand = (INT8)(0 - (sizeof(JMP_REL_SHORT) + sizeof(JMP_REL)));
        }
    }
    else
        memcpy(target, pHook->backup, patchSize);
    /* Reflect the bytes already written even if subsequent completion fails. */
    pHook->isEnabled = enable;
    pHook->queueEnable = enable;
    g_patchCompletion.cachePending = TRUE;
    return RetryPatchCompletion() ? MH_OK : MH_ERROR_MEMORY_PROTECT;
}

static MH_STATUS EnableAllHooksLL]=])

_replace_between("MH_STATUS WINAPI MH_Uninitialize(VOID)" "MH_STATUS WINAPI MH_CreateHook" [=[MH_STATUS WINAPI MH_Uninitialize(VOID)
{
    MH_STATUS status = MH_ERROR_NOT_INITIALIZED;
    EnterSpinLock();
    if (g_hHeap != NULL)
    {
        status = RetryRetainedThreadOwnership() ? MH_OK : MH_ERROR_MEMORY_PROTECT;
        if (status == MH_OK && !g_shutdownPending)
            status = EnableAllHooksLL(FALSE);
        if (status == MH_OK)
            status = RetireAllBuffers();
        /* Frozen arrays belong to this heap: finish/retry their cleanup before
         * destroying it. HeapDestroy also owns the hook-entry allocation. */
        if (status == MH_OK)
        {
            if (!HeapDestroy(g_hHeap))
                status = MH_ERROR_MEMORY_PROTECT;
            else
            {
                g_hHeap = NULL;
                g_hooks.pItems = NULL;
                g_hooks.size = g_hooks.capacity = 0;
                g_shutdownPending = FALSE;
            }
        }
    }
    LeaveSpinLock();
    return status;
}

MH_STATUS WINAPI MH_CreateHook]=])

# Generated code must be coherent before publishing its address to the caller.
string(REPLACE "PHOOK_ENTRY pHook = AddHookEntry();"
    "PHOOK_ENTRY pHook = NULL;\n                        if (FlushInstructionCache(GetCurrentProcess(), pBuffer, MEMORY_SLOT_SIZE))\n                            pHook = AddHookEntry();\n                        else\n                            status = MH_ERROR_MEMORY_PROTECT;" _mh "${_mh}")
string(REPLACE "status = MH_ERROR_MEMORY_ALLOC;\n                        }\n                    }"
    "if (status == MH_OK) status = MH_ERROR_MEMORY_ALLOC;\n                        }\n                    }" _mh "${_mh}")
string(REPLACE "FreeBuffer(pBuffer);"
    "if (!FreeBuffer(pBuffer))\n                        {\n                            g_retainedBuffer = pBuffer;\n                            status = MH_ERROR_MEMORY_PROTECT;\n                        }" _mh "${_mh}")
string(REPLACE "FreeBuffer(g_hooks.pItems[pos].pTrampoline);\n                DeleteHookEntry(pos);"
    "status = RetireHookEntry(pos);" _mh "${_mh}")
string(REPLACE "if (g_hHeap != NULL && !RetryRetainedThreadOwnership())"
    "if (g_hHeap != NULL && (g_shutdownPending || !RetryRetainedThreadOwnership()))" _mh "${_mh}")

# The hook-only transform is also used by anchor-negative source fixtures.
# A complete stage supplies both sibling buffer files; verify() pins all three.
get_filename_component(_completion_dir "${MINHOOK_HOOK_C}" DIRECTORY)
if(EXISTS "${_completion_dir}/buffer.c" AND EXISTS "${_completion_dir}/buffer.h")
    file(READ "${_completion_dir}/buffer.c" _buffer)
    file(READ "${_completion_dir}/buffer.h" _buffer_header)
    string(REPLACE "VOID   UninitializeBuffer(VOID);" "BOOL   UninitializeBuffer(VOID);" _buffer_header "${_buffer_header}")
    string(REPLACE "VOID   FreeBuffer(LPVOID pBuffer);" "BOOL   FreeBuffer(LPVOID pBuffer);\nBOOL   IsBufferAddress(ULONG_PTR address);" _buffer_header "${_buffer_header}")
    set(_saved_hook "${_mh}")
    set(_mh "${_buffer}")
    _replace_between("VOID UninitializeBuffer(VOID)" "//-------------------------------------------------------------------------\n#if defined(_M_X64)" [=[BOOL UninitializeBuffer(VOID)
{
    while (g_pMemoryBlocks != NULL)
    {
        PMEMORY_BLOCK block = g_pMemoryBlocks;
        PMEMORY_BLOCK next = block->pNext;
        if (!VirtualFree(block, 0, MEM_RELEASE))
            return FALSE;
        g_pMemoryBlocks = next;
    }
    return TRUE;
}

BOOL IsBufferAddress(ULONG_PTR address)
{
    PMEMORY_BLOCK block;
    for (block = g_pMemoryBlocks; block != NULL; block = block->pNext)
        if (address >= (ULONG_PTR)block && address - (ULONG_PTR)block < MEMORY_BLOCK_SIZE)
            return TRUE;
    return FALSE;
}

//-------------------------------------------------------------------------
#if defined(_M_X64)]=])
    _replace_between("VOID FreeBuffer(LPVOID pBuffer)" "BOOL IsExecutableAddress" [=[BOOL FreeBuffer(LPVOID pBuffer)
{
    PMEMORY_BLOCK block = g_pMemoryBlocks, previous = NULL;
    ULONG_PTR target = (ULONG_PTR)pBuffer / MEMORY_BLOCK_SIZE * MEMORY_BLOCK_SIZE;
    while (block != NULL)
    {
        if ((ULONG_PTR)block == target)
        {
            if (block->usedCount == 1)
            {
                PMEMORY_BLOCK next = block->pNext;
                if (!VirtualFree(block, 0, MEM_RELEASE))
                    return FALSE;
                if (previous != NULL) previous->pNext = next;
                else g_pMemoryBlocks = next;
            }
            else
            {
                PMEMORY_SLOT slot = (PMEMORY_SLOT)pBuffer;
                slot->pNext = block->pFree;
                block->pFree = slot;
                --block->usedCount;
            }
            return TRUE;
        }
        previous = block;
        block = block->pNext;
    }
    return FALSE;
}

BOOL IsExecutableAddress]=])
    set(_completion_buffer "${_mh}")
    set(_mh "${_saved_hook}")
    file(WRITE "${_completion_dir}/buffer.c" "${_completion_buffer}")
    file(WRITE "${_completion_dir}/buffer.h" "${_buffer_header}")
endif()
