#pragma once
#include "../anadius-bootstrap-owner-source/owner.hpp"
#include <cstdlib>
#include <exception>
#include <memory>

namespace anadius::bootstrap_runtime {
struct HostApi {
    decltype(&std::atexit) at_exit;
    decltype(&std::set_terminate) set_terminate;
    decltype(&std::get_terminate) get_terminate;
};
HostApi native_host_api() noexcept;

// New compiler-CRT composition, not a recovered native function. Lifecycle and
// registration belong to one control thread. Natural CRT exit must not race
// registration, and host APIs must not invoke registered callbacks inline.
// Callback execution and pause may overlap; an admitted callback retains its
// lease until it returns. Owner shutdown must pause, join workers, drain, finish
// BEFORE releasing providers or the native Binding. Natural process exit alone
// does not establish worker quiescence. Image unloading/loader-lock safety is a
// separate host contract: permanent CRT thunks must remain executable.
// Paused/finished exit thunks return through a statically initialized lock-free
// admission check before accessing the hub. A dispatcher already past that
// check still rechecks under the hub mutex; pause is not its join/barrier.
class CallbackRegistry final {
public:
    explicit CallbackRegistry(HostApi = native_host_api());
    ~CallbackRegistry() noexcept;
    CallbackRegistry(const CallbackRegistry&)=delete;
    CallbackRegistry& operator=(const CallbackRegistry&)=delete;
    CallbackRegistry(CallbackRegistry&&)=delete;
    CallbackRegistry& operator=(CallbackRegistry&&)=delete;
    int register_exit(bootstrap_native_recovered::ExitCallback);
    void install_terminate(std::terminate_handler);
    bool pause_admission();
    bool drain_exit_callbacks();
    bool finish();
    std::size_t pending_exit_callbacks() const;
    std::size_t active_callbacks() const;
    bool published() const;
    bool paused() const;
    bool finished() const;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

// Concrete registrar policy; remaining policies retain their existing provider.
// Registry outlives this wrapper and its lower provider. No new virtual API.
class RegisteredServices final:public bootstrap_owner::Services {
public:
    RegisteredServices(CallbackRegistry&,std::unique_ptr<bootstrap_owner::Services>);
    int register_exit(bootstrap_native_recovered::ExitCallback) override;
    void install_terminate(std::terminate_handler) override;
    void* log_insert(void*,std::uintptr_t) override;
    void log_finish(void*) override;
    void* allocate(std::size_t) override;
    void deallocate(void*,std::size_t) override;
    recovered::NoArgumentFunction worker(recovered::Worker) override;
    recovered::BooleanFunction boolean_worker() override;
    [[noreturn]] void thread_error(int) override;
    void between_workers() override;
    void guard_return() override;
    void cleanup_threads() override;
    void cleanup_first() override;
    void cleanup_second() override;
    void cleanup_network() override;
    void terminate_log() override;
    [[noreturn]] void unexpected_terminate_return() override;
private:
    CallbackRegistry& registry_;
    std::unique_ptr<bootstrap_owner::Services> lower_;
};
} // namespace anadius::bootstrap_runtime
