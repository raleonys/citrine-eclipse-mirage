// SPDX-License-Identifier: GPL-3.0-only
#include "worker_runtime.hpp"

namespace { constinit preloader_l::WindowsWorker<> worker; }
extern "C" void SURGE() noexcept {}
extern "C" std::uint32_t __stdcall DllShutdown(std::uint32_t milliseconds) noexcept {
    return worker.shutdown(milliseconds);
}
extern "C" BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) noexcept {
    // Scheduling failures retain a module for explicit shutdown outside loader lock.
    if(reason==DLL_PROCESS_ATTACH) worker.attach(module);
    return TRUE;
}
