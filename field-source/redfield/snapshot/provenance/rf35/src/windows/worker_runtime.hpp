// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "preloader_l/local_service.hpp"
#include "preloader_l/worker_owner.hpp"
#include "preloader_l/windows_hooks.hpp"
#include "preloader_l/session_retirement.hpp"
#include "preloader_l/runtime/transient_allocator.hpp"
#include <memory>
#include <optional>
#include <string>

namespace preloader_l {
// Production owns no observation state. The separate composition may observe
// publication and gate physical close/unpin without changing cleanup ownership.
struct WorkerObservation final {
    constexpr void configuration_result(FileConfigResult) noexcept {}
    constexpr void service_ready(bool,std::uint16_t) noexcept {}
    constexpr void transport_progress(std::uint64_t,std::uint64_t,std::uint64_t) noexcept {}
    constexpr void service_stopped() noexcept {}
    constexpr bool allow_close() noexcept { return true; }
    constexpr bool allow_unpin() noexcept { return true; }
};

// One worker implementation, separately composed DLLs. Observation is never a
// lifetime owner; it must be allocation-free/noexcept and outlive the worker.
// attach only schedules under loader lock; shutdown remains an explicit call
// with an independent host-held module reference, as required by WorkerOwner.
template<class Observation=WorkerObservation>
class WindowsWorker final {
public:
    constexpr WindowsWorker() noexcept : retirement_(hooks_),
        operations_{this,pin_module,spawn_thread,wait_thread,close_thread,
                    unpin_module,cleanup_session,finalize_memory} {}
    void attach(HINSTANCE module) noexcept {
        own_module_=module;
        (void)owner_.schedule(operations_,module);
    }
    std::uint32_t shutdown(std::uint32_t milliseconds) noexcept {
        return static_cast<std::uint32_t>(owner_.shutdown(milliseconds));
    }
    WorkerPhase phase() const noexcept { return owner_.phase(); }
    HookPhase hook_phase() const noexcept { return hooks_.phase(); }
    Observation& observation() noexcept { return observation_; }
private:
    bool finish_session(std::unique_ptr<LocalService>& session) noexcept {
        return retirement_.finish(session);
    }
    static bool read_port(void*,std::optional<std::string>& value) noexcept {
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
    static bool write_port(void*,const char* value) noexcept {return SetEnvironmentVariableA("EALsxPort",value)!=FALSE;}
    DWORD run() noexcept {
        std::unique_ptr<LocalService> session;
        try {
            if (owner_.stop_requested()) {
                owner_.publish_phase(WorkerPhase::cancelled); owner_.mark_worker_clean(); return 0;
            }
            wchar_t filename[32768]{};
            const auto count=GetModuleFileNameW(own_module_,filename,32768);
            if (!count || count>=32768) {
                observation_.configuration_result(FileConfigResult::invalid_path);
                owner_.publish_phase(WorkerPhase::failed); owner_.mark_worker_clean(); return 1;
            }
            session=std::make_unique<LocalService>(std::filesystem::path(filename),PortBackend{nullptr,read_port,write_port});
            const auto started=session->start();
            observation_.configuration_result(session->load_status());
            if (!started) {
                const bool clean=finish_session(session);
                owner_.publish_phase(WorkerPhase::failed);
                if (clean) owner_.mark_worker_clean();
                return 2;
            }
            const auto configuration=session->configuration();
            if (!configuration) {
                const bool clean=finish_session(session);
                owner_.publish_phase(WorkerPhase::failed);
                if (clean) owner_.mark_worker_clean();
                return 5;
            }
            if (owner_.stop_requested() || !hooks_.start(configuration,session->snapshot().port)) {
                const bool clean=finish_session(session);
                owner_.publish_phase(owner_.stop_requested()?WorkerPhase::cancelled:WorkerPhase::failed);
                if (clean) owner_.mark_worker_clean();
                return 6;
            }
            observation_.service_ready(configuration->online,session->snapshot().port);
            if (!owner_.stop_requested()) owner_.publish_phase(WorkerPhase::ready);
            else owner_.publish_phase(WorkerPhase::cancelled);
            bool transport_failed=false;
            while (!owner_.stop_requested()) {
                session->poll_for(std::chrono::milliseconds(1));
                const auto snapshot=session->snapshot();
                observation_.transport_progress(snapshot.accepted,snapshot.active,snapshot.frames);
                if(!snapshot.port){transport_failed=true;break;}
            }
            // A shutdown timeout leaves this worker and its module reference alive.
            // No io_context.stop() shortcut may discard cancelled completions.
            // Hook restoration precedes service/config retirement. Failed restore
            // returns a retryable owner_ to explicit DllShutdown; never unpin it.
            if (!hooks_.stop(0)) {
                retirement_.retain(session);
                owner_.publish_phase(WorkerPhase::failed);
                return 7;
            }
            while (!session->stop_for(std::chrono::milliseconds(1))) Sleep(1);
            observation_.service_stopped();
            const bool clean=finish_session(session);
            owner_.publish_phase(clean && !transport_failed ? WorkerPhase::finished : WorkerPhase::failed);
            if (clean) owner_.mark_worker_clean();
            return clean?0:3;
        } catch (...) {
            observation_.configuration_result(FileConfigResult::resource);
            owner_.publish_phase(WorkerPhase::failed);
            const bool clean=session ? finish_session(session) : hooks_.stop(0);
            if (clean && !retirement_.pending()) owner_.mark_worker_clean();
            return 4;
        }
    }

    static WindowsWorker& self(void* user) noexcept { return *static_cast<WindowsWorker*>(user); }
    static DWORD WINAPI worker(void* user) noexcept { return self(user).run(); }
    static bool pin_module(void*,void* module,void** result) noexcept {
        HMODULE pinned=nullptr;
        if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                              reinterpret_cast<LPCWSTR>(module),&pinned)) return false;
        *result=pinned;return true;
    }
    static void* spawn_thread(void* user,WorkerOwner*) noexcept {
        return CreateThread(nullptr,0,worker,user,0,nullptr);
    }
    static WaitResult wait_thread(void*,void* thread,std::uint32_t milliseconds) noexcept {
        const auto result=WaitForSingleObject(static_cast<HANDLE>(thread),milliseconds);
        return result==WAIT_OBJECT_0?WaitResult::signaled:
               result==WAIT_TIMEOUT?WaitResult::timeout:WaitResult::failed;
    }
    static bool close_thread(void* user,void* thread) noexcept {
        return self(user).observation_.allow_close() && CloseHandle(static_cast<HANDLE>(thread))!=FALSE;
    }
    static bool unpin_module(void* user,void* module) noexcept {
        return self(user).observation_.allow_unpin() && FreeLibrary(static_cast<HMODULE>(module))!=FALSE;
    }
    static bool cleanup_session(void* user,std::uint32_t milliseconds) noexcept {
        return self(user).retirement_.retry(milliseconds);
    }
    static bool finalize_memory(void*,std::uint32_t milliseconds) noexcept {
        return runtime::drain_transient_releases(milliseconds);
    }
    WorkerOwner owner_;
    WindowsHooks hooks_;
    SessionRetirement<WindowsHooks> retirement_;
    [[no_unique_address]] Observation observation_;
    const WorkerOperations operations_;
    HMODULE own_module_=nullptr;
};
}
