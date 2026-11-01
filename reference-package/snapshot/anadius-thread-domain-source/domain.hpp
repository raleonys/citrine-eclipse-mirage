#pragma once
#include "initial_locale.hpp"
#include "runtime.hpp"
#include "../anadius-bootstrap-providers-source/native_threads.hpp"
#include "../anadius-crt-fls-native-source/fls.hpp"
#include "../anadius-crt-heap-native-bridge-source/bridge.hpp"
#include <atomic>

namespace anadius::thread_domain {
// Rebuilt callback ABIs, explicitly selected by caller. No preferred original
// image addresses are made callable. No handler means compiler abort policy.
using NewHandler=std::int32_t(*)(std::uint64_t);
using InvalidHandler=void(*)(const crt_invalid_recovered::Arguments&);
struct Seed {
    std::uint64_t security_cookie;
    std::int32_t new_mode;
    NewHandler new_handler;
    InvalidHandler invalid_handler;
    std::uint32_t thread_policy; // 0 asks existing source to query actual OS
    std::uintptr_t exception_table_identity;
    std::shared_ptr<void> exception_table_lifetime;
};
struct Apis {
    crt_heap_native::Api heap=crt_heap_native::native_api();
    crt_glue::FlsApi fls=crt_glue::native_fls_api();
    crt_lock_native::Api locks=crt_lock_native::native_api();
    NativeRuntimeApi runtime=native_runtime_api();
};
enum class Phase {Constructed,Ready,ReleasePending,Stopping,Closed,Poisoned};
class ApplicationMemory;
class CallbackThreads;
// An owned INITIAL-LOCALE application PTD domain. It does not run the recovered
// outer CRT, initializer tables or DLL entry. Compiler CRT starts image once.
// create/initialize/make_threads/close belong to one non-reentrant control
// thread, outside loader lock. Real workers use the same binding.state().
//
// B3/B4 first stop and join every worker, including FLS exit callbacks. Then
// destroy the NativeThreads providers before close(); retained provider leases
// make close return false. The caller keeps this root alive through retries.
// Native SDK callbacks must obey Win32 nonthrowing outcome contracts. Unknown
// exceptions during published PTD construction poison the domain; no guessed
// rollback of partial constructor/lock/ref effects is performed.
class DomainStorage final:public std::enable_shared_from_this<DomainStorage> {
public:
    static std::shared_ptr<DomainStorage> create(Seed,Apis={});
    ~DomainStorage() noexcept;
    DomainStorage(const DomainStorage&)=delete;DomainStorage& operator=(const DomainStorage&)=delete;
    DomainStorage(DomainStorage&&)=delete;DomainStorage& operator=(DomainStorage&&)=delete;
    bool initialize();
    std::unique_ptr<bootstrap_providers::NativeThreads> make_threads(thread_native::Api=thread_native::native_api());
    std::shared_ptr<ApplicationMemory> make_memory();
    std::shared_ptr<CallbackThreads> make_callback_threads();
    bool close();
    Phase phase() const noexcept;
    std::size_t provider_leases() const noexcept;
    std::uint32_t native_fls_index() const noexcept;
    std::uint32_t source_fls_index() const noexcept;
    crt_ptd_recovered::Ptd* current_ptd();
    crt_errors_recovered::Cell32 errno_cell();
    const InitialLocale& locale() const noexcept;
    std::uint32_t locale_references() const;
    std::uint32_t multibyte_references() const;
    std::size_t runtime_modules() const;
private:
    DomainStorage(Seed,Apis);
    struct Impl;std::unique_ptr<Impl> impl_;
    struct ProviderLease;
    friend class ApplicationMemory;
    friend class CallbackThreads;
};
// B7 bounded application allocator: same recovered heap, error/PTD and handler
// state as NativeThreads. A provider lease prevents domain close while this
// object lives. External B3/B4 admission stop and joins precede its destruction.
// Calls may run on the original controller or workers created by this same
// domain's recovered factory, whose PTD is present. Foreign/CRT-only threads
// are outside this contract; shared fallback/OOM concurrency is not established.
// Factory/lifecycle remain controller operations. The provider must outlive
// every allocation and active call.
// Outstanding/unknown native outcomes make destruction terminal. Byte storage
// only, no over-aligned support or original C++ exception ABI equivalence.
class ApplicationMemory final {
public:
    enum class AllocationState {Missing,Live,Busy,Unknown};
    ~ApplicationMemory() noexcept;
    ApplicationMemory(const ApplicationMemory&)=delete;
    ApplicationMemory& operator=(const ApplicationMemory&)=delete;
    void* allocate(std::size_t);
    // The observed delete ignores size. Foreign/already-released pointers are
    // rejected. Known native free failure retains ownership for explicit retry.
    void deallocate(void*,std::size_t);
    // Includes reserved in-progress tickets; exact live blocks when quiescent.
    std::size_t outstanding_allocations() const;
    // Unknown native/commit outcome forbids caller-side raw-free retries.
    // Read-only observation under the existing ticket gate; preserves LastError.
    bool tracking_poisoned() const;
    // Live proves retained quiescent ownership. Unknown forbids physical retry;
    // Missing/Busy are not evidence of a known failed HeapFree.
    AllocationState allocation_state(void*) const;
    const DomainStorage* domain_identity() const noexcept;
private:
    ApplicationMemory(DomainStorage&,std::shared_ptr<void>);
    struct Impl;std::unique_ptr<Impl> impl_;
    friend class DomainStorage;
};
// Explicit single-active callback PTD scope. Create on the ready controller;
// enter/leave on the same calling thread AND fiber. Existing PTD is borrowed.
// Fresh initial-locale PTD permits errno/doserrno changes only; other mutation
// poisons cleanup. Checked detach/destroy/raw-free stages retain known failures
// for same-thread retry without repeating destructors or creating replacement
// PTD through errno fallback. Unknown outcomes poison and require retention.
// Provider lease retains domain resources. No RO/worker attach/outer CRT entry.
// External state/worker/fallback/stream concurrency contracts remain required.
class CallbackThreads final {
public:
    ~CallbackThreads() noexcept;
    CallbackThreads(const CallbackThreads&)=delete;CallbackThreads& operator=(const CallbackThreads&)=delete;
    void enter();
    bool leave();
    bool active() const;
    bool poisoned() const;
    bool owned() const;
    std::uint32_t last_failure() const;
    // Body permission probes actual FLS identity outside metadata locks, then
    // checks fresh initial snapshot (except errno/DOS). Mismatch/unknown probe
    // poisons and retains ownership. Cleanup start permanently ends permission;
    // known cleanup retry still works. Raw free/ABA/foreign mutation is excluded.
    bool permits_current() const;
private:
    CallbackThreads(DomainStorage&,std::shared_ptr<void>);
    struct Impl;std::unique_ptr<Impl> impl_;
    friend class DomainStorage;
};
} // namespace anadius::thread_domain
