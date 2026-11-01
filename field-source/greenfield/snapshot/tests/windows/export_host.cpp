#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
int wmain(int argc, wchar_t** argv) {
    if(argc != 2) return 2;
    HMODULE m=LoadLibraryW(argv[1]);
    if(!m) { std::cerr << "LoadLibrary failed\n"; return 3; }
    FARPROC e1=GetProcAddress(m,"DllMain");
    FARPROC e2=GetProcAddress(m,"SURGE");
    FARPROC e3=GetProcAddress(m,"DllShutdown");
    const auto byOrdinal=[&](WORD ord){return GetProcAddress(m,MAKEINTRESOURCEA(ord));};
    if(!e1 || !e2 || !e3 || e1!=byOrdinal(1) || e2!=byOrdinal(2) || e3!=byOrdinal(3)) {
        FreeLibrary(m); return 4;
    }
    using Shutdown = std::uint32_t (__stdcall *)(std::uint32_t);
    const auto stop=reinterpret_cast<Shutdown>(e3);
    if(stop(0)!=0u || stop(15)!=0u){FreeLibrary(m);return 5;}
    if(!FreeLibrary(m)) return 6;
    std::cout << "GF00_WINDOWS_EXPORT_SHAPE_PASS_NOT_RUNTIME_PARITY\n";
    return 0;
}
