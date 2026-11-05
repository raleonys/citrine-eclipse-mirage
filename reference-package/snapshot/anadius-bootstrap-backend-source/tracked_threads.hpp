#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include "../anadius-thread-source/threads.hpp"
#include <memory>

namespace anadius::bootstrap_backend {
struct JoinApi {
    decltype(&::GetCurrentProcess) process;
    decltype(&::DuplicateHandle) duplicate;
    decltype(&::WaitForSingleObject) wait;
    decltype(&::CloseHandle) close;
    decltype(&::GetLastError) error;
    decltype(&::SetLastError) set_error;
};
JoinApi native_join_api() noexcept;
// New ownership adapter, not an original thread body. The lower Effects must
// remain alive until this adapter is drained. Source begin_thread still closes
// its original detached handle; only a duplicated join handle is retained here.
// Lifecycle/destruction belongs to one control thread. Creation may overlap
// draining; a reserved, not-yet-returned create prevents a successful drain.
// Once an unknown create/duplicate/wait/close outcome poisons this adapter, new
// create requests reject before reserving a ticket or invoking the lower. Work
// already reserved before poison remains owned and may still complete create.
// A known null create result removes its ticket and does not close admission.
class TrackingThreads final:public threads_recovered::Effects {
public:
    TrackingThreads(threads_recovered::Effects&,JoinApi);
    ~TrackingThreads() noexcept;
    TrackingThreads(const TrackingThreads&)=delete;
    TrackingThreads& operator=(const TrackingThreads&)=delete;
    // Caller first stops application admission/cancellation sources. Existing
    // jobs still execute; this method only rejects NEW create requests. False
    // retains unjoined handles, so the backend/owner must remain alive. A
    // successful-create/duplicate failure, or an unknown lower-create exception
    // outcome, permanently poisons safe quiescence. A throwing wait/close also
    // poisons: the duplicate may already be closed/reused, so its numeric value
    // must not be retried. Known WAIT_TIMEOUT/WAIT_FAILED/close FALSE retain
    // their existing retry behavior. Poison retains the owner; no drain proof.
    bool stop_and_join(std::uint32_t timeout_ms);
    std::size_t tracked_jobs() const;
    bool poisoned() const;
    threads_recovered::Context* calloc_context_066848() override;
    void free_0668c0(threads_recovered::Context*) override;
    bool get_module_handle_ex_w(std::uint32_t,std::uintptr_t,threads_recovered::Handle*) override;
    threads_recovered::Handle create_thread(void*,std::uint64_t,threads_recovered::Context*,std::uint32_t,std::uint32_t*) override;
    std::uint32_t last_error() override;
    void write_error_slot_05d724(std::uint32_t) override;
    void invalid_parameter_05c954() override;
    void map_os_error_05d698(std::uint32_t) override;
    bool close_handle(threads_recovered::Handle) override;
    bool free_library(threads_recovered::Handle) override;
    threads_recovered::Context*& required_thread_slot_064b90() override;
    threads_recovered::Context* optional_thread_context_064d08() override;
    int runtime_policy_06c0d0() override;
    int initialize_runtime_067320(int) override;
    void uninitialize_runtime_06736c() override;
    std::uint32_t invoke_entry(std::uintptr_t,std::uintptr_t) override;
    [[noreturn]] void free_library_and_exit_thread(threads_recovered::Handle,std::uint32_t) override;
    [[noreturn]] void exit_thread(std::uint32_t) override;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace anadius::bootstrap_backend
