#pragma once
#include "callbacks.hpp"
#include "../anadius-bootstrap-backend-source/backend.hpp"

namespace anadius::bootstrap_runtime {
// Exact SDK boundaries, injectable for synthetic tests. This is a new adapter,
// not a recovered native body or a claim about the original CRT exception ABI.
struct ExecutionApi {
    decltype(&::GetCurrentThreadId) thread_id;
    decltype(&::AcquireSRWLockExclusive) acquire;
    decltype(&::ReleaseSRWLockExclusive) release;
    decltype(&::QueryPerformanceFrequency) frequency;
    decltype(&::QueryPerformanceCounter) counter;
    decltype(&::Sleep) sleep;
};
ExecutionApi native_execution_api() noexcept;
class NativeExecution final:public execution_recovered::Platform {
public:
    explicit NativeExecution(bootstrap_owner::Services&,ExecutionApi=native_execution_api());
    std::uint32_t current_thread_id() override;
    void acquire_srw_exclusive(std::uintptr_t&) override;
    void release_srw_exclusive(std::uintptr_t&) override;
    std::int64_t query_performance_frequency() override;
    std::int64_t query_performance_counter() override;
    void sleep_milliseconds(std::uint32_t) override;
    [[noreturn]] void raise_thread_error_04a648(int) override;
private:bootstrap_owner::Services& errors_;ExecutionApi api_;
};
struct SelectedDefaults {
    bool files=false,input=false,library_os=false,execution=false,thread_join=false;
};
// Source-owned assembly, with explicit lower providers and native initial state.
// Null files/input/library_os/execution and an ENTIRELY empty JoinApi select the
// concrete adapters listed above/below. Partial JoinApi is rejected. All other
// Resources providers and stop_sources stay mandatory. Exactly one of legacy
// remaining Services or passive remaining_factory must be supplied. The latter
// is bound once to this Runtime's registry and retired after source cleanup.
// For the new route prefer make_runtime below: it consumes the caller donor
// and directly constructs the by-value sink, avoiding make_unique forwarding
// temporaries. Direct stack/new callers can pass take_resources(resources).
// std::move alone does not empty opaque SBO capture donors in this toolchain.
// stop_sources excludes NEW application, worker and hook admissions, including
// cached application-entry pointers. It must permit pending workers to finish
// and must not unload their dependencies. Callback admission is handled here.
//
// Control methods are non-reentrant, on one serialized control thread, outside
// loader lock. No DLL entry or loader-lock-safe shutdown is provided. Original
// recovered mutex/cache algorithms still require their own concurrency proof.
// Compiler CRT initializes this image once; only application_entry is exposed.
class Runtime final {
public:
    Runtime(bootstrap_owner::InitialState,bootstrap_backend::RuntimeSeed,
        bootstrap_backend::Resources,bootstrap_native_recovered::Errors& permanent_errors,
        HostApi=native_host_api());
    ~Runtime() noexcept;
    Runtime(const Runtime&)=delete;Runtime& operator=(const Runtime&)=delete;
    Runtime(Runtime&&)=delete;Runtime& operator=(Runtime&&)=delete;
    void publish();
    bool close();
    bootstrap_native_recovered::ApplicationEntry application_entry() const;
    bootstrap_owner::Phase phase() const noexcept;
    const configuration::State& configuration() const noexcept;
    const bootstrap_owner::ConsoleState& console() const noexcept;
    const SelectedDefaults& selected_defaults() const noexcept{return selected_;}
private:
    // Reverse destruction: backend and its RegisteredServices die before registry.
    CallbackRegistry registry_;SelectedDefaults selected_;
    std::unique_ptr<bootstrap_owner::RuntimeOwner> owner_;
};
// Supported owning construction for the narrow route. Resources is borrowed
// only for consuming transfer; its callable donors are cleared before factory
// result publication. Direct new intentionally avoids make_unique's additional
// by-value forwarding temporary. This remains OUTSIDE loader lock.
std::unique_ptr<Runtime> make_runtime(bootstrap_owner::InitialState,bootstrap_backend::RuntimeSeed,
    bootstrap_backend::Resources&,bootstrap_native_recovered::Errors&,
    HostApi=native_host_api());
} // namespace anadius::bootstrap_runtime
