// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/application.hpp"
#include "laureondo/platform/hook_unload_guard.hpp"
#include "laureondo/runtime/control.h"
#include <atomic>
#include <stdexcept>
namespace {std::atomic<int> created{},attempts{},destroyed{};std::atomic<std::uint32_t> caller{};}
extern "C" __declspec(dllexport) std::uint32_t FixtureCaller(){return caller;}
extern "C" __declspec(dllexport) int FixtureCreated(){return created;}
extern "C" __declspec(dllexport) int FixtureAttempts(){return attempts;}
extern "C" __declspec(dllexport) int FixtureDestroyed(){return destroyed;}
namespace laureondo {
void set_shutdown_caller(std::uint32_t id) noexcept {caller=id;} // Test-only scan boundary.
namespace application {
struct Application::Impl {};
Application::Application(Options):impl_(std::make_unique<Impl>()){++created;}
Application::~Application(){++destroyed;}
void Application::start(){} // Fixture deliberately has no sockets/hooks.
void Application::stop(){if(++attempts<3)throw std::runtime_error("Injected cleanup failure");}
const configuration::State& Application::configuration()const {static const configuration::State state{};return state;}
}
}
namespace {DWORD WINAPI application_worker(void*);}
namespace fixture_module {
std::atomic<unsigned> acquisitions{};
std::atomic<bool> valid{true};
BOOL WINAPI acquire(DWORD flags,LPCWSTR address,HMODULE* module) {
    ++acquisitions;
    if(flags!=GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS||
       address!=reinterpret_cast<LPCWSTR>(&application_worker)||!module)valid=false;
    char selected[2]{};
    if(GetEnvironmentVariableA("LAUREONDO_TEST_FAIL_WORKER_IMAGE_LEASE",selected,2)==1&&selected[0]=='1') {
        if(module)*module=nullptr; // Controlled failure before acquiring any reference.
        SetLastError(ERROR_ACCESS_DENIED);return FALSE;
    }
    return ::GetModuleHandleExW(flags,address,module);
}
}
// Keep the production entry body byte-exact. Successful acquisition uses the
// real SDK; only this fixture's selected early worker failure is controlled.
#define GetModuleHandleExW fixture_module::acquire
#include "../../../src/platform/dll_entry.cpp"
#undef GetModuleHandleExW
extern "C" __declspec(dllexport) DWORD FixtureWorker(unsigned key) {
    if(key==0)return WaitForSingleObject(worker_thread,0);
    if(key==1){DWORD result=~DWORD{0};return GetExitCodeThread(worker_thread,&result)?result:~DWORD{0};}
    if(key==2)return image_lease!=nullptr;
    if(key==3)return fixture_module::acquisitions.load();
    if(key==4)return fixture_module::valid.load();
    return ~DWORD{0};
}
