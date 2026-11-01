#include <stddef.h>

// Compatibility comparison-shim runtime support for the superseded /NODEFAULTLIB build model.
// Normalized 3.1.8 shipping uses /MD + /GS + linker-selected _DllMainCRTStartup.
// Do NOT define the reserved MSVC intrinsic names memset/memcpy directly:
// cl.exe rejects definitions of those names with C2169.  Instead the linker
// maps unresolved memset/memcpy references to these ordinary C symbols via
// /alternatename (see the Windows and clang-cl build scripts).
extern "C" void* preloader_l_memset(void* dst, int value, size_t count) noexcept
{
    volatile unsigned char* p = static_cast<volatile unsigned char*>(dst);
    const unsigned char v = static_cast<unsigned char>(value);
    for (size_t i = 0; i < count; ++i) p[i] = v;
    return dst;
}

extern "C" void* preloader_l_memcpy(void* dst, const void* src, size_t count) noexcept
{
    volatile unsigned char* d = static_cast<volatile unsigned char*>(dst);
    const volatile unsigned char* s = static_cast<const volatile unsigned char*>(src);
    for (size_t i = 0; i < count; ++i) d[i] = s[i];
    return dst;
}
