// Outer DllMain reconstruction, NOT original source recovery.
// Original entry VA 0x180005469; evidence:
// ev_dfd20118a4db8cf9932db18a1fc7452d57fcbee254114f5d52c1988eaa8b92f5
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static_assert(sizeof(void*) == 8, "This reconstruction requires Windows x64.");

// Assembly addresses 0x18000e000..0x18000e088 become contiguous state slots.
// 0..14: cached version functions; 15: winmm; 16: kernel32; 17: version.
extern "C" {
UINT_PTR OuterState[18];
}

namespace {
// This array preserves the original embedded component verbatim. It is NOT
// decompiled/reconstructed source for that component.
alignas(16) constexpr BYTE kOriginalEmbeddedPE[] = {
#include "embedded_original_machine_code.inc"
};
static_assert(sizeof(kOriginalEmbeddedPE) == 21504, "Embedded file length mismatch.");

DWORD Read32(const BYTE* p) { return *reinterpret_cast<const DWORD*>(p); }
WORD Read16(const BYTE* p) { return *reinterpret_cast<const WORD*>(p); }

// Volatile stores retain the observed byte-copy behavior without a CRT memcpy.
void CopyObservedBytes(BYTE* destination, const BYTE* source, SIZE_T count) {
    volatile BYTE* out = destination;
    for (SIZE_T i = 0; i != count; ++i) out[i] = source[i];
}

void InitializeObservedOuterBehavior() {
    // 0x180005482..0x1800054b6. Preserve the observed hardcoded paths.
    OuterState[15] = reinterpret_cast<UINT_PTR>(LoadLibraryW(L"C:\\Windows\\System32\\winmm.dll"));
    OuterState[16] = reinterpret_cast<UINT_PTR>(GetModuleHandleW(L"C:\\Windows\\System32\\kernel32.dll"));
    OuterState[17] = reinterpret_cast<UINT_PTR>(LoadLibraryW(L"C:\\Windows\\System32\\version.dll"));

    // 0x1800054bd..0x180005546. Fixed offsets come from the embedded PE
    // at outer VA 0x180006d80, not from a newly inferred generic PE loader.
    BYTE* image = static_cast<BYTE*>(VirtualAlloc(nullptr, 0xa000, 0x3000, 4));
    if (!image) return;
    CopyObservedBytes(image, kOriginalEmbeddedPE, 0x400);
    for (SIZE_T i = 0; i != 5; ++i) {
        const BYTE* section = kOriginalEmbeddedPE + 0x210 + i * 0x28;
        DWORD rawSize = Read32(section + 0x10);
        if (rawSize) {
            CopyObservedBytes(image + Read32(section + 0x0c),
                              kOriginalEmbeddedPE + Read32(section + 0x14), rawSize);
        }
    }

    // 0x180005548..0x1800055c6: only IMAGE_REL_BASED_DIR64 is applied.
    // Preserve the original block termination and arithmetic without adding
    // validation/recovery branches that were absent from the retained code.
    BYTE* block = image + 0x9000;
    if (Read32(block)) {
        do {
            DWORD blockSize = Read32(block + 4);
            if (((static_cast<ULONGLONG>(blockSize) + 0x1fffffff8ULL) &
                 0x1fffffffeULL) != 0) {
                DWORD count = static_cast<DWORD>((static_cast<ULONGLONG>(blockSize) +
                                                 0x1fffffff8ULL) >> 1);
                for (ULONGLONG i = 0; i != count; ++i) {
                    WORD relocation = Read16(block + 8 + i * 2);
                    if ((relocation & 0xf000) == 0xa000) {
                        auto slot = reinterpret_cast<ULONGLONG*>(image + Read32(block) +
                                                                (relocation & 0xfff));
                        *slot += reinterpret_cast<ULONGLONG>(image) - 0x180000000ULL;
                    }
                }
                blockSize = Read32(block + 4);
            }
            block += blockSize;
        } while (Read32(block));
    }

    // 0x1800055c8..0x180005663. Retain LoadLibraryA -> GetModuleHandleA
    // fallback and ordinal/name import resolution with no extra failure policy.
    BYTE* descriptor = image + 0x5c1c;
    DWORD dllName = Read32(descriptor + 12);
    if (dllName) {
        do {
            const char* moduleName = reinterpret_cast<const char*>(image + dllName);
            HMODULE module = LoadLibraryA(moduleName);
            if (!module) module = GetModuleHandleA(moduleName);
            DWORD firstThunk = Read32(descriptor + 16);
            DWORD originalThunk = Read32(descriptor);
            if (!originalThunk) originalThunk = firstThunk;
            BYTE* cursor = image;
            ULONGLONG thunk = *reinterpret_cast<ULONGLONG*>(cursor + originalThunk);
            while (thunk) {
                const char* symbol = (thunk & 0x8000000000000000ULL)
                    ? reinterpret_cast<const char*>(static_cast<UINT_PTR>(thunk & 0xffff))
                    : reinterpret_cast<const char*>(image + thunk + 2);
                *reinterpret_cast<FARPROC*>(cursor + firstThunk) = GetProcAddress(module, symbol);
                thunk = *reinterpret_cast<ULONGLONG*>(cursor + originalThunk + 8);
                cursor += 8;
            }
            dllName = Read32(descriptor + 32); // Name in the next 20-byte descriptor.
            descriptor += 20;
        } while (dllName);
    }

    // 0x180005665..0x1800056dc. Header RO; section protection is selected
    // from EXECUTE (bit 29) and WRITE (bit 31), exactly as the original.
    DWORD previousProtection;
    VirtualProtect(image, 0x400, 2, &previousProtection);
    for (SIZE_T i = 0; i != 5; ++i) {
        const BYTE* section = kOriginalEmbeddedPE + 0x210 + i * 0x28;
        DWORD flags = Read32(section + 0x24);
        bool executable = (flags & 0x20000000) != 0;
        bool writable = (flags & 0x80000000) != 0;
        DWORD protection = writable ? (executable ? 0x40 : 4) : (executable ? 0x20 : 2);
        VirtualProtect(image + Read32(section + 0x0c), Read32(section + 8),
                       protection, &previousProtection);
    }

    // 0x1800056de..0x1800056f2. Original ignores the embedded return value.
    using EmbeddedEntry = BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID);
    auto entry = reinterpret_cast<EmbeddedEntry>(image + 0x3338);
    entry(reinterpret_cast<HINSTANCE>(image), DLL_PROCESS_ATTACH, nullptr);
}
}

// Explicit linker entry bypasses CRT startup, matching the observed entry.
// Original reads EDX only, ignores instance/reserved and always returns 1.
extern "C" BOOL WINAPI ReconstructedDllMain(HINSTANCE, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) InitializeObservedOuterBehavior();
    return TRUE;
}
