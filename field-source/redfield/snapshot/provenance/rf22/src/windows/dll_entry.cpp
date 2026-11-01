// SPDX-License-Identifier: GPL-3.0-only
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "redfield/local_service.hpp"
#include "redfield/worker_owner.hpp"
#include "redfield/windows_hooks.hpp"
#include "redfield/session_retirement.hpp"
#include "redfield/platform/connect_redirect.hpp"
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace {
using namespace redfield;
constinit WorkerOwner owner;
constinit WindowsHooks hooks;
constinit std::atomic<unsigned> config_result{255};
constinit std::atomic<bool> online{false};
constinit SessionRetirement<WindowsHooks> retirement{hooks};
#if defined(REDFIELD_NATIVE_TEST)
constinit std::atomic<std::uint16_t> transport_port{0};
constinit std::atomic<std::uint64_t> transport_frames{0},transport_accepted{0},transport_active{0};
#endif
HMODULE own_module=nullptr;
#if defined(REDFIELD_NATIVE_TEST)
constinit std::atomic<unsigned> fail_close{0}, fail_unpin{0};
bool consume(std::atomic<unsigned>& remaining) noexcept {
    auto count=remaining.load();
    while (count && !remaining.compare_exchange_weak(count,count-1)) {}
    return count!=0;
}
#endif
bool finish_session(std::unique_ptr<LocalService>& session) noexcept {
    return retirement.finish(session);
}
bool read_port(void*,std::optional<std::string>& value) noexcept {
    try {
        for(unsigned attempt=0;attempt<3;++attempt) {
            SetLastError(ERROR_SUCCESS);
            const auto required=GetEnvironmentVariableA("EALsxPort",nullptr,0);
            if(!required) {
                const auto error=GetLastError();
                if(error==ERROR_ENVVAR_NOT_FOUND){value.reset();return true;}
                if(error==ERROR_SUCCESS){value=std::string{};return true;}
                return false;
            }
            if(required>65536)return false;
            std::string buffer(required,'\0');SetLastError(ERROR_SUCCESS);
            const auto count=GetEnvironmentVariableA("EALsxPort",buffer.data(),required);
            if(!count) {
                const auto error=GetLastError();
                if(error==ERROR_ENVVAR_NOT_FOUND){value.reset();return true;}
                if(error==ERROR_SUCCESS){value=std::string{};return true;}
                return false;
            }
            if(count>=required)continue;
            buffer.resize(count);value=std::move(buffer);return true;
        }
    }catch(...){}
    return false;
}
bool write_port(void*,const char* value) noexcept {return SetEnvironmentVariableA("EALsxPort",value)!=FALSE;}
DWORD WINAPI worker(void*) noexcept {
    std::unique_ptr<LocalService> session;
    try {
        if (owner.stop_requested()) {
            owner.publish_phase(WorkerPhase::cancelled); owner.mark_worker_clean(); return 0;
        }
        wchar_t filename[32768]{};
        const auto count=GetModuleFileNameW(own_module,filename,32768);
        if (!count || count>=32768) {
            config_result.store(static_cast<unsigned>(FileConfigResult::invalid_path));
            owner.publish_phase(WorkerPhase::failed); owner.mark_worker_clean(); return 1;
        }
        session=std::make_unique<LocalService>(std::filesystem::path(filename),PortBackend{nullptr,read_port,write_port});
        const auto started=session->start();
        config_result.store(static_cast<unsigned>(session->load_status()),std::memory_order_release);
        if (!started) {
            const bool clean=finish_session(session);
            owner.publish_phase(WorkerPhase::failed);
            if (clean) owner.mark_worker_clean();
            return 2;
        }
        const auto configuration=session->configuration();
        if (!configuration) {
            const bool clean=finish_session(session);
            owner.publish_phase(WorkerPhase::failed);
            if (clean) owner.mark_worker_clean();
            return 5;
        }
        if (owner.stop_requested() || !hooks.start(configuration,session->snapshot().port)) {
            const bool clean=finish_session(session);
            owner.publish_phase(owner.stop_requested()?WorkerPhase::cancelled:WorkerPhase::failed);
            if (clean) owner.mark_worker_clean();
            return 6;
        }
        online.store(configuration->online,std::memory_order_release);
#if defined(REDFIELD_NATIVE_TEST)
        transport_port.store(session->snapshot().port,std::memory_order_release);
#endif
        if (!owner.stop_requested()) owner.publish_phase(WorkerPhase::ready);
        else owner.publish_phase(WorkerPhase::cancelled);
        bool transport_failed=false;
        while (!owner.stop_requested()) {
            session->poll_for(std::chrono::milliseconds(1));
            const auto snapshot=session->snapshot();
#if defined(REDFIELD_NATIVE_TEST)
            transport_accepted.store(snapshot.accepted,std::memory_order_release);
            transport_active.store(snapshot.active,std::memory_order_release);
            transport_frames.store(snapshot.frames,std::memory_order_release);
#endif
            if(!snapshot.port){transport_failed=true;break;}
        }
        // A shutdown timeout leaves this worker and its module reference alive.
        // No io_context.stop() shortcut may discard cancelled completions.
        // Hook restoration precedes service/config retirement. Failed restore
        // returns a retryable owner to explicit DllShutdown; never unpin it.
        if (!hooks.stop(0)) {
            retirement.retain(session);
            owner.publish_phase(WorkerPhase::failed);
            return 7;
        }
        while (!session->stop_for(std::chrono::milliseconds(1))) Sleep(1);
#if defined(REDFIELD_NATIVE_TEST)
        transport_port.store(0,std::memory_order_release);
#endif
        const bool clean=finish_session(session);
        owner.publish_phase(clean && !transport_failed ? WorkerPhase::finished : WorkerPhase::failed);
        if (clean) owner.mark_worker_clean();
        return clean?0:3;
    } catch (...) {
        config_result.store(static_cast<unsigned>(FileConfigResult::resource));
        owner.publish_phase(WorkerPhase::failed);
        const bool clean=session ? finish_session(session) : hooks.stop(0);
        if (clean && !retirement.pending()) owner.mark_worker_clean();
        return 4;
    }
}
bool pin_module(void*, void* module, void** result) noexcept {
    HMODULE pinned=nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           reinterpret_cast<LPCWSTR>(module),&pinned)) return false;
    *result=pinned;
    return true;
}
void* spawn_thread(void*, WorkerOwner*) noexcept { return CreateThread(nullptr,0,worker,nullptr,0,nullptr); }
WaitResult wait_thread(void*, void* thread, std::uint32_t timeout_ms) noexcept {
    const auto result=WaitForSingleObject(static_cast<HANDLE>(thread),timeout_ms);
    return result==WAIT_OBJECT_0 ? WaitResult::signaled : result==WAIT_TIMEOUT ? WaitResult::timeout : WaitResult::failed;
}
bool close_thread(void*, void* thread) noexcept {
#if defined(REDFIELD_NATIVE_TEST)
    if (consume(fail_close)) return false;
#endif
    return CloseHandle(static_cast<HANDLE>(thread))!=FALSE;
}
bool unpin_module(void*, void* module) noexcept {
#if defined(REDFIELD_NATIVE_TEST)
    if (consume(fail_unpin)) return false;
#endif
    return FreeLibrary(static_cast<HMODULE>(module))!=FALSE;
}
bool cleanup_session(void*,std::uint32_t timeout_ms) noexcept {
    return retirement.retry(timeout_ms);
}
const WorkerOperations operations{nullptr,pin_module,spawn_thread,wait_thread,close_thread,unpin_module,cleanup_session};
}
extern "C" void SURGE() noexcept {}
extern "C" std::uint32_t __stdcall DllShutdown(std::uint32_t timeout_ms) noexcept {
    return static_cast<std::uint32_t>(owner.shutdown(timeout_ms));
}
extern "C" BOOL WINAPI DllMain(HINSTANCE module, DWORD reason, LPVOID) noexcept {
    if (reason==DLL_PROCESS_ATTACH) {
        own_module=module;
        // Return a module even on scheduling failure: DllShutdown must be able
        // to release any retained pin outside the loader lock. No waits or
        // config/heap/service initialization occurs in this entrypoint.
        (void)owner.schedule(operations,module);
    }
    return TRUE;
}
#if defined(REDFIELD_NATIVE_TEST)
// Only the separate test DLL has these exports. Production keeps three names.
extern "C" std::uint64_t RFTestStatus() noexcept {
    // Read the publication marker before the fields it publishes.
    const auto phase=owner.phase();
    return static_cast<std::uint64_t>(phase) |
        (static_cast<std::uint64_t>(config_result.load(std::memory_order_acquire))<<8) |
        (static_cast<std::uint64_t>(online.load(std::memory_order_acquire))<<16) |
        (static_cast<std::uint64_t>(transport_port.load(std::memory_order_acquire))<<17) |
        (static_cast<std::uint64_t>(hooks.phase())<<33) |
        (static_cast<std::uint64_t>(redfield::platform::connect_redirect_mapped_count()&0xffffu)<<40);
}
extern "C" void RFTestSetFaults(unsigned close_failures,unsigned unpin_failures) noexcept {
    fail_close.store(close_failures); fail_unpin.store(unpin_failures);
}
extern "C" std::uint64_t RFTestTransport() noexcept {
    const auto frames=transport_frames.load(std::memory_order_acquire);
    return ((transport_accepted.load(std::memory_order_acquire)&0xffffu)<<48) |
           ((transport_active.load(std::memory_order_acquire)&0xffffu)<<32) |
           (frames&0xffffffffu);
}
#endif
