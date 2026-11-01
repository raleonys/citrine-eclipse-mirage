#pragma once
#include "../anadius-source/bootstrap.hpp"
#include "../anadius-execution-source/execution.hpp"
#include <atomic>
#include <memory>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace anadius::services_completion {
struct ExitWaitApi {
    decltype(&::GetCurrentThreadId) thread_id;
    decltype(&::InitializeCriticalSectionEx) initialize;
    decltype(&::DeleteCriticalSection) destroy;
    decltype(&::EnterCriticalSection) enter;
    decltype(&::LeaveCriticalSection) leave;
    decltype(&::AcquireSRWLockExclusive) acquire;
    decltype(&::ReleaseSRWLockExclusive) release;
    decltype(&::WakeAllConditionVariable) wake_all;
    decltype(&::GetLastError) error;
};
ExitWaitApi native_exit_wait_api() noexcept;
// A selected nonrecursive, count1 owned target, not an adopted MSVC mutex/Cnd.
// It cannot be reused until its registered cleanup call has finished.
class WaitTarget final {
public:
    WaitTarget(std::uint32_t mutex_type,std::int32_t initial_ready,ExitWaitApi=native_exit_wait_api());
    ~WaitTarget() noexcept;
    WaitTarget(const WaitTarget&)=delete;WaitTarget& operator=(const WaitTarget&)=delete;
    void lock();
    void unlock();
    bool is_locked() const noexcept;
    bool registered() const noexcept;
    // Read only after join/quiescence or while holding this target's lock.
    std::int32_t ready_value_quiescent() const noexcept;
private:
    ExitWaitApi api_;
    execution_recovered::MutexState mutex_{};
    // Typed SDK storage is separate from the recovered logical uintptr cell.
    // The private ReleasePlatform maps that cell's identity to this lock.
    SRWLOCK native_lock_=SRWLOCK_INIT;
    CONDITION_VARIABLE condition_=CONDITION_VARIABLE_INIT;
    std::int32_t ready_;
    std::atomic<DWORD> owner_=0,registration_=0;
    struct ReleasePlatform;
    void release_registered(DWORD);
    friend class ExitWaitRegistry;
};
enum class InitialRegistry {empty_owned};
// A NEW explicitly empty source registry, not the compiler CRT's private list
// or a claim about original global initialization. Registration is source-only.
// Callers hold shared targets through any waiter activity. This API supplies
// cleanup/notification, not a complete condition-variable wait implementation.
class ExitWaitRegistry final {
public:
    explicit ExitWaitRegistry(InitialRegistry,ExitWaitApi=native_exit_wait_api());
    ~ExitWaitRegistry() noexcept;
    ExitWaitRegistry(const ExitWaitRegistry&)=delete;ExitWaitRegistry& operator=(const ExitWaitRegistry&)=delete;
    void register_current_thread(std::shared_ptr<WaitTarget>,bool publish_ready=true);
    void cleanup_current_thread();
    std::size_t active() const;
    std::size_t blocks() const;
    // On the constructor's controller thread, after B3/B4 stop/join. False
    // retains all state and stops new registration; existing cleanup may finish.
    // Native CS is destroyed only with the quiescent registry object.
    bool close();
private:
    struct Impl;std::unique_ptr<Impl> impl_;
};
} // namespace anadius::services_completion
