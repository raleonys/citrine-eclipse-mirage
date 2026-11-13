# SPDX-License-Identifier: GPL-3.0-only
# Derived from the BSD-licensed MinHook source; preserve its original header.
get_filename_component(_rel_src "${MINHOOK_HOOK_C}" DIRECTORY)
set(_rel_path "${_rel_src}/trampoline.c")
file(READ "${_rel_path}" _relocator)
function(_rel_replace before after)
    string(FIND "${_relocator}" "${before}" position)
    if(position LESS 0)
        message(FATAL_ERROR "MinHook relocator anchor missing")
    endif()
    string(REPLACE "${before}" "" remainder "${_relocator}")
    string(LENGTH "${_relocator}" original_length)
    string(LENGTH "${remainder}" remaining_length)
    string(LENGTH "${before}" anchor_length)
    math(EXPR removed_length "${original_length}-${remaining_length}")
    if(NOT removed_length EQUAL anchor_length)
        message(FATAL_ERROR "MinHook relocator anchor is ambiguous")
    endif()
    string(REPLACE "${before}" "${after}" result "${_relocator}")
    set(_relocator "${result}" PARENT_SCOPE)
endfunction()

_rel_replace([=[#endif

//-------------------------------------------------------------------------
static BOOL IsCodePadding(LPBYTE pInst, UINT size)
{
    UINT i;

    if (pInst[0] != 0x00 && pInst[0] != 0x90 && pInst[0] != 0xCC)
        return FALSE;

    for (i = 1; i < size; ++i)
    {
        if (pInst[i] != pInst[0])
            return FALSE;
    }
    return TRUE;
]=] [=[#endif

//-------------------------------------------------------------------------
static BOOL IsReadableProtection(DWORD protect)
{
    if (protect & (PAGE_GUARD | PAGE_NOACCESS))
        return FALSE;

    switch (protect & 0xFF)
    {
    case PAGE_READONLY:
    case PAGE_READWRITE:
    case PAGE_WRITECOPY:
    case PAGE_EXECUTE_READ:
    case PAGE_EXECUTE_READWRITE:
    case PAGE_EXECUTE_WRITECOPY:
        return TRUE;
    default:
        return FALSE;
    }
}

//-------------------------------------------------------------------------
static SIZE_T CopyReadableCode(LPBYTE pBuffer, LPCVOID pAddress, SIZE_T size)
{
    ULONG_PTR cursor = (ULONG_PTR)pAddress;
    SIZE_T copied = 0;

    if (size == 0 || cursor + size < cursor)
        return 0;

    while (copied < size)
    {
        MEMORY_BASIC_INFORMATION mi;
        ULONG_PTR regionEnd;
        SIZE_T available;
        SIZE_T transferred;

        if (!VirtualQuery((LPCVOID)cursor, &mi, sizeof(mi))
            || mi.State != MEM_COMMIT
            || !IsReadableProtection(mi.Protect))
        {
            break;
        }

        regionEnd = (ULONG_PTR)mi.BaseAddress + mi.RegionSize;
        if (regionEnd <= cursor)
            break;

        available = regionEnd - cursor;
        if (available > size - copied)
            available = size - copied;

        if (!ReadProcessMemory(
                GetCurrentProcess(),
                (LPCVOID)cursor,
                pBuffer + copied,
                available,
                &transferred)
            || transferred != available)
        {
            break;
        }
        cursor += available;
        copied += available;
    }

    return copied;
}

//-------------------------------------------------------------------------
static BOOL IsCodePadding(LPBYTE pInst, UINT size)
{
    UINT8 buffer[sizeof(JMP_REL)];
    UINT i;

    if (size == 0 || size > sizeof(buffer)
        || CopyReadableCode(buffer, pInst, size) != size
        || (buffer[0] != 0x00 && buffer[0] != 0x90 && buffer[0] != 0xCC))
    {
        return FALSE;
    }

    for (i = 1; i < size; ++i)
    {
        if (buffer[i] != buffer[0])
            return FALSE;
    }
    return TRUE;
]=])

_rel_replace([=[    UINT8     newPos   = 0;
    ULONG_PTR jmpDest  = 0;     // Destination address of an internal jump.
    BOOL      finished = FALSE; // Is the function completed?
#if defined(_M_X64) || defined(__x86_64__)
    UINT8     instBuf[16];
#endif

    ct->patchAbove = FALSE;
    ct->nIP        = 0;
]=] [=[    UINT8     newPos   = 0;
    ULONG_PTR jmpDest  = 0;     // Destination address of an internal jump.
    BOOL      finished = FALSE; // Is the function completed?
    UINT8     instBuf[16];

    ct->patchAbove = FALSE;
    ct->nIP        = 0;
]=])

_rel_replace([=[        ULONG_PTR pOldInst = (ULONG_PTR)ct->pTarget     + oldPos;
        ULONG_PTR pNewInst = (ULONG_PTR)ct->pTrampoline + newPos;

        copySize = HDE_DISASM((LPVOID)pOldInst, &hs);
        if (hs.flags & F_ERROR)
            return FALSE;

        pCopySrc = (LPVOID)pOldInst;
        if (oldPos >= sizeof(JMP_REL))
        {
            // The trampoline function is long enough.
]=] [=[        ULONG_PTR pOldInst = (ULONG_PTR)ct->pTarget     + oldPos;
        ULONG_PTR pNewInst = (ULONG_PTR)ct->pTrampoline + newPos;

        SIZE_T    readableSize;

        memset(instBuf, 0, sizeof(instBuf));
        readableSize = CopyReadableCode(instBuf, (LPCVOID)pOldInst, sizeof(instBuf));
        if (readableSize == 0)
            return FALSE;

        copySize = HDE_DISASM(instBuf, &hs);
        if ((hs.flags & F_ERROR) || copySize > readableSize)
            return FALSE;

        pCopySrc = instBuf;
        if (oldPos >= sizeof(JMP_REL))
        {
            // The trampoline function is long enough.
]=])

_rel_replace([=[            // Modify the RIP relative address.
            PUINT32 pRelAddr;

            // Avoid using memcpy to reduce the footprint.
#ifndef ALLOW_INTRINSICS
            memcpy(instBuf, (LPBYTE)pOldInst, copySize);
#else
            __movsb(instBuf, (LPBYTE)pOldInst, copySize);
#endif
            pCopySrc = instBuf;

            // Relative address is stored at (instruction length - immediate value length - 4).
            pRelAddr = (PUINT32)(instBuf + hs.len - ((hs.flags & 0x3C) >> 2) - 4);
            *pRelAddr
                = (UINT32)((pOldInst + hs.len + (INT32)hs.disp.disp32) - (pNewInst + hs.len));

            // Complete the function if JMP (FF /4).
            if (hs.opcode == 0xFF && hs.modrm_reg == 4)
]=] [=[            // Modify the RIP relative address.
            PUINT32 pRelAddr;

            // Relative address is stored at (instruction length - immediate value length - 4).
            pRelAddr = (PUINT32)(instBuf + hs.len - ((hs.flags & 0x3C) >> 2) - 4);
            if (hs.flags & F_PREFIX_67)
            {
                // EIP-relative addressing wraps at 32 bits in 64-bit mode.
                *pRelAddr = (UINT32)(
                    (pOldInst + hs.len + (INT32)hs.disp.disp32)
                    - (pNewInst + hs.len));
            }
            else
            {
                INT64 absoluteAddress = (INT64)pOldInst + hs.len + (INT32)hs.disp.disp32;
                INT64 newDisplacement = absoluteAddress - ((INT64)pNewInst + hs.len);

                if (newDisplacement < (-2147483647LL - 1)
                    || newDisplacement > 2147483647LL)
                {
                    return FALSE;
                }
                *pRelAddr = (UINT32)(INT32)newDisplacement;
            }

            // Complete the function if JMP (FF /4).
            if (hs.opcode == 0xFF && hs.modrm_reg == 4)
]=])

_rel_replace([=[        }
        else if ((hs.opcode & 0xF0) == 0x70
            || (hs.opcode & 0xFC) == 0xE0
            || (hs.opcode2 & 0xF0) == 0x80)
        {
            // Direct relative Jcc
            ULONG_PTR dest = pOldInst + hs.len;
]=] [=[        }
        else if ((hs.opcode & 0xF0) == 0x70
            || (hs.opcode & 0xFC) == 0xE0
            || (hs.opcode == 0x0F && (hs.opcode2 & 0xF0) == 0x80))
        {
            // Direct relative Jcc
            ULONG_PTR dest = pOldInst + hs.len;
]=])

_rel_replace([=[
            // Complete the function if not in a branch.
            finished = (pOldInst >= jmpDest);
        }

        // Can't alter the instruction length in a branch.
]=] [=[
            // Complete the function if not in a branch.
            finished = (pOldInst >= jmpDest);
        }
        else if (hs.flags & F_RELATIVE)
        {
            // Never copy an unhandled PC-relative instruction (e.g. XBEGIN)
            // with its original displacement into a different code address.
            return FALSE;
        }

        // Can't alter the instruction length in a branch.
]=])

file(WRITE "${_rel_path}" "${_relocator}")
