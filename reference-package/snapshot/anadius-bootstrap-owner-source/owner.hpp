#pragma once
#include "../anadius-config-startup-source/startup.hpp"
#include "../anadius-bootstrap-native-source/native_entries.hpp"
#include <memory>
#include <optional>

namespace anadius::bootstrap_owner {
struct ConsoleState {std::uint8_t show_lsx,show_console,no_console;};
struct InitialState {configuration::State configuration;ConsoleState console;};
// Mandatory application/runtime policies. Forwarding to them does not recover
// their native implementations or close their lifecycle/ABI contracts.
class Services {
public:
    virtual ~Services()=default;
    virtual int register_exit(bootstrap_native_recovered::ExitCallback)=0;
    virtual void install_terminate(std::terminate_handler)=0;
    virtual void* log_insert(void*,std::uintptr_t)=0;
    virtual void log_finish(void*)=0;
    virtual void* allocate(std::size_t)=0;
    virtual void deallocate(void*,std::size_t)=0;
    virtual recovered::NoArgumentFunction worker(recovered::Worker)=0;
    virtual recovered::BooleanFunction boolean_worker()=0;
    [[noreturn]] virtual void thread_error(int)=0;
    virtual void between_workers()=0;
    virtual void guard_return()=0;
    virtual void cleanup_threads()=0;
    virtual void cleanup_first()=0;
    virtual void cleanup_second()=0;
    virtual void cleanup_network()=0;
    virtual void terminate_log()=0;
    [[noreturn]] virtual void unexpected_terminate_return()=0;
};
struct Providers {
    configuration::Dependencies& configuration;file_recovered::Factory& files;
    startup_recovered::InputPlatform& input;startup_recovered::ConsolePlatform& console;
    threads_recovered::Effects& threads;Services& services;
};
// One separately implemented owner keeps all lower providers and their shared
// runtime/worker/callback state alive. providers() is queried once. bind_state
// must not publish callbacks or create workers. It must not retain references
// beyond this owner's lifetime. No concrete OS/worker backend is supplied here.
class ProviderOwner {
public:
    virtual ~ProviderOwner()=default;
    virtual Providers providers()=0;
    virtual void bind_state(configuration::State&,config_startup::ConsoleFlags)=0;
    // True promises all admission sources stopped, pending/suspended workers
    // joined/cancelled, and future callbacks (including late exit/terminate
    // registrations) excluded. Must be retryable after
    // false/exception. Native active_calls()==0 alone is insufficient.
    virtual bool stop_and_join()=0;
};
// Exactly one BootstrapDependencies base. Reuse the three actual config source
// methods; bind the existing thread source directly instead of inheriting a
// second ThreadBridge. This is new composition, not a recovered native body.
class BootstrapFacade final:public config_startup::BootstrapConfiguration {
public:
    BootstrapFacade(configuration::State&,ConsoleState&,Providers);
    int register_exit(recovered::RegisteredCallback) override;
    void install_terminate(recovered::RegisteredCallback) override;
    void* log_insert_006880(void*,std::uintptr_t) override;
    void log_finish_0088e0(void*) override;
    void* allocate_04b0bc(std::size_t) override;
    void delete_04b0f8(void*,std::size_t) override;
    recovered::NoArgumentFunction worker(recovered::Worker) override;
    recovered::BooleanFunction boolean_worker_035850() override;
    recovered::Handle begin_thread_05e8e0(recovered::ThreadEntry,void*,std::uint32_t*) override;
    bool close_handle(recovered::Handle) override;
    [[noreturn]] void throw_thread_error_04a648(int) override;
    void between_workers_00e310() override;
    void guard_return_017a30() override;
    void cleanup_threads_04a47c() override;
    void cleanup_035ab0() override;
    void cleanup_0366f0() override;
    void cleanup_0102b0() override;
    void terminate_log_038920() override;
    [[noreturn]] void unexpected_terminate_return_trap() override;
private:Providers providers_;
};
enum class Phase {Constructed,Bound,Stopping,Closed};
// Application-layer composition only: no PE entry is installed and neither
// compiler CRT nor recovered CRT startup is invoked. A later DLL entry strategy
// must choose exactly one route. Configuration initial values are caller input.
// Permanent errors must outlive ALL future native-entry attempts, including
// attempts after close/destruction (native last_errors retains that pointer).
// Owner lifecycle methods belong to one control thread and reject reentry.
// Backend synchronization
// governs concurrent worker/config access; this wrapper adds no implicit locks.
class RuntimeOwner final {
public:
    RuntimeOwner(InitialState,std::unique_ptr<ProviderOwner>,bootstrap_native_recovered::Errors& permanent_errors);
    ~RuntimeOwner() noexcept;
    RuntimeOwner(const RuntimeOwner&)=delete;RuntimeOwner& operator=(const RuntimeOwner&)=delete;
    RuntimeOwner(RuntimeOwner&&)=delete;RuntimeOwner& operator=(RuntimeOwner&&)=delete;
    void publish();
    bool close();
    bootstrap_native_recovered::ApplicationEntry application_entry() const;
    Phase phase() const noexcept{return phase_;}
    const configuration::State& configuration() const noexcept{return configuration_;}
    const ConsoleState& console() const noexcept{return console_;}
private:
    // Reverse destruction keeps shared state alive while providers are freed.
    configuration::State configuration_;ConsoleState console_;
    std::unique_ptr<ProviderOwner> backend_;Providers providers_;
    BootstrapFacade facade_;recovered::Bootstrap bootstrap_;
    bootstrap_native_recovered::Errors& permanent_errors_;
    std::optional<bootstrap_native_recovered::Binding> binding_;
    Phase phase_=Phase::Constructed;
    bool control_busy_=false;
};
} // namespace anadius::bootstrap_owner
