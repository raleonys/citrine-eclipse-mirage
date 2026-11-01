// SPDX-License-Identifier: GPL-3.0-only
#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
int wmain(int argc, wchar_t** argv) {
    if (argc!=2) return 2;
    for (int run=0; run<3; ++run) {
        auto module=LoadLibraryExW(argv[1],nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        if (!module) return 3;
        const char* names[]{"DllMain","SURGE","DllShutdown"};
        for (WORD i=0; i<3; ++i)
            if (!GetProcAddress(module,names[i]) || GetProcAddress(module,names[i])!=GetProcAddress(module,MAKEINTRESOURCEA(i+1))) return 4;
        // Calling convention/export shell test; never call DllMain directly.
        auto surge=reinterpret_cast<void(*)() noexcept>(GetProcAddress(module,"SURGE"));
        auto shutdown=reinterpret_cast<std::uint32_t(__stdcall*)(std::uint32_t) noexcept>(GetProcAddress(module,"DllShutdown"));
        if (GetProcAddress(module,"RFTestStatus") || GetProcAddress(module,"RFTestSetFaults") || GetProcAddress(module,"RFTestTransport")) return 7;
        surge();
        const auto first=shutdown(0);
        if ((first!=0 && first!=2) || shutdown(10000)!=0 || shutdown(0)!=0) return 5;
        if (!FreeLibrary(module)) return 6;
    }
    std::puts("ABI_SMOKE_PASS:3_LOAD_SHUTDOWN_UNLOAD_CYCLES");
    return 0;
}
