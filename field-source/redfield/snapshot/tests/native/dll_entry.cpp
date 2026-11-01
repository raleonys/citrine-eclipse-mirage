// SPDX-License-Identifier: GPL-3.0-only
#include "worker_observation.hpp"
#include "../../src/windows/worker_runtime.hpp"
#include "preloader_l/platform/connect_redirect.hpp"

namespace {
preloader_l::WindowsWorker<preloader_l::testing::WorkerObservation> worker{};
}
extern "C" void SURGE() noexcept {}
extern "C" std::uint32_t __stdcall DllShutdown(std::uint32_t milliseconds) noexcept {
    return worker.shutdown(milliseconds);
}
extern "C" BOOL WINAPI DllMain(HINSTANCE module,DWORD reason,LPVOID) noexcept {
    if(reason==DLL_PROCESS_ATTACH) worker.attach(module);
    return TRUE;
}
extern "C" std::uint64_t RFTestStatus() noexcept {
    const auto phase=worker.phase();
    return worker.observation().status(phase,worker.hook_phase(),
        preloader_l::platform::connect_redirect_mapped_count());
}
extern "C" void RFTestSetFaults(unsigned close_failures,unsigned unpin_failures) noexcept {
    worker.observation().set_faults(close_failures,unpin_failures);
}
extern "C" std::uint64_t RFTestTransport() noexcept { return worker.observation().transport(); }
