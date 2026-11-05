#include "factory.hpp"
#include "../anadius-bootstrap-providers-source/native_threads.hpp"
#include <stdexcept>
#include <system_error>
#include <type_traits>

namespace anadius::bootstrap_runtime {
namespace {
std::shared_ptr<thread_domain::DomainStorage> runtime_domain(const bootstrap_backend::Resources& resources)noexcept{
    const auto* threads=dynamic_cast<const bootstrap_providers::NativeThreads*>(resources.threads.get());
    return threads?threads->memory_domain():nullptr;
}
std::int64_t execution_frequency(ExecutionApi api){
    LARGE_INTEGER value{};
    if(!api.frequency(&value))throw std::system_error(static_cast<int>(::GetLastError()),std::system_category(),"QueryPerformanceFrequency");
    return value.QuadPart;
}
std::int64_t execution_counter(ExecutionApi api){
    LARGE_INTEGER value{};
    if(!api.counter(&value))throw std::system_error(static_cast<int>(::GetLastError()),std::system_category(),"QueryPerformanceCounter");
    return value.QuadPart;
}
}
ExecutionApi native_execution_api() noexcept{
    return {::GetCurrentThreadId,::AcquireSRWLockExclusive,::ReleaseSRWLockExclusive,
        ::QueryPerformanceFrequency,::QueryPerformanceCounter,::Sleep};
}
NativeExecution::NativeExecution(bootstrap_owner::Services& e,ExecutionApi a):errors_(e),api_(a){
    if(!a.thread_id||!a.acquire||!a.release||!a.frequency||!a.counter||!a.sleep)
        throw std::invalid_argument("All execution SDK functions are required");
}
std::uint32_t NativeExecution::current_thread_id(){return api_.thread_id();}
void NativeExecution::acquire_srw_exclusive(std::uintptr_t& lock){api_.acquire(reinterpret_cast<PSRWLOCK>(&lock));}
void NativeExecution::release_srw_exclusive(std::uintptr_t& lock){api_.release(reinterpret_cast<PSRWLOCK>(&lock));}
std::int64_t NativeExecution::query_performance_frequency(){
    return execution_frequency(api_);
}
std::int64_t NativeExecution::query_performance_counter(){
    return execution_counter(api_);
}
void NativeExecution::sleep_milliseconds(std::uint32_t value){api_.sleep(value);}
[[noreturn]] void NativeExecution::raise_thread_error_04a648(int value){errors_.thread_error(value);std::terminate();}
static_assert(sizeof(SRWLOCK)==sizeof(std::uintptr_t)&&alignof(SRWLOCK)<=alignof(std::uintptr_t));
namespace {
bool empty(bootstrap_backend::JoinApi a){return !a.process&&!a.duplicate&&!a.wait&&!a.close&&!a.error&&!a.set_error;}
class RemainingExecution final:public execution_recovered::Platform {
    bootstrap_backend::RemainingPolicies& errors_;ExecutionApi api_;
public:
    explicit RemainingExecution(bootstrap_backend::RemainingPolicies& errors,ExecutionApi api=native_execution_api())
        :errors_(errors),api_(api){
        if(!api.thread_id||!api.acquire||!api.release||!api.frequency||!api.counter||!api.sleep)
            throw std::invalid_argument("All execution SDK functions are required");
    }
    std::uint32_t current_thread_id()override{return api_.thread_id();}
    void acquire_srw_exclusive(std::uintptr_t& lock)override{api_.acquire(reinterpret_cast<PSRWLOCK>(&lock));}
    void release_srw_exclusive(std::uintptr_t& lock)override{api_.release(reinterpret_cast<PSRWLOCK>(&lock));}
    std::int64_t query_performance_frequency()override{return execution_frequency(api_);}
    std::int64_t query_performance_counter()override{return execution_counter(api_);}
    void sleep_milliseconds(std::uint32_t value)override{api_.sleep(value);}
    [[noreturn]]void raise_thread_error_04a648(int value)override{errors_.thread_error(value);std::terminate();}
};
struct Prepared {
    // All Resources and their opaque deleters retire while narrow is alive.
    std::unique_ptr<bootstrap_backend::RemainingPolicies> narrow;
    bootstrap_backend::Resources resources;
};
Prepared prepare(bootstrap_backend::Resources& resources,CallbackRegistry& registry,SelectedDefaults& selected){
    Prepared prepared{nullptr,bootstrap_backend::take_resources(resources)};auto& r=prepared.resources;
    if((static_cast<bool>(r.remaining)==static_cast<bool>(r.remaining_factory))||!bootstrap_backend::lifetime_configuration_valid(r))
        throw std::invalid_argument("Exactly one remaining Services/factory and selected lifetime configuration required");
    if(!r.files){r.files=std::make_unique<file_recovered::NativeFactory>();selected.files=true;}
    if(!r.input){r.input=std::make_unique<startup_recovered::NativeInputPlatform>();selected.input=true;}
    if(!r.library_os){r.library_os=std::make_unique<recovered::libraries::NativeSystemCalls>();selected.library_os=true;}
    if(r.remaining_factory){
        prepared.narrow=r.remaining_factory(registry); // adopt BEFORE capture retirement
        if(!prepared.narrow)throw std::invalid_argument("Remaining factory returned null");
        r.remaining_factory={};
        if(registry.published()||registry.paused()||registry.finished()||registry.pending_exit_callbacks()||registry.active_callbacks())
            throw std::logic_error("Remaining factory must not publish or change registry admission");
    }else r.remaining=std::make_unique<RegisteredServices>(registry,std::move(r.remaining));
    if(!r.execution){
        if(prepared.narrow)r.execution=std::make_unique<RemainingExecution>(*prepared.narrow);
        else r.execution=std::make_unique<NativeExecution>(*r.remaining);
        selected.execution=true;
    }
    if(empty(r.join_api)){r.join_api=bootstrap_backend::native_join_api();selected.thread_join=true;}
    if(r.stop_sources){
        auto external=std::move(r.stop_sources);
        r.stop_sources=[external=std::move(external),&registry]() mutable {
            if(!external())return false;
            return registry.pause_admission();
        };
    }
    return prepared;
}
class ClosingBackend final:public bootstrap_owner::ProviderOwner {
    CallbackRegistry& registry_;bootstrap_backend::SourceBackend source_;
    bool source_stopped_=false,exits_drained_=false,attachment_closed_=false,registry_finished_=false,online_closed_=false,aggregate_cleanup_ready_=false;
public:
    ClosingBackend(bootstrap_backend::RuntimeSeed seed,Prepared&& prepared,
        CallbackRegistry& registry):registry_(registry),source_(std::move(seed),bootstrap_backend::take_resources(prepared.resources),std::move(prepared.narrow)){}
    ~ClosingBackend() noexcept override {
        // RuntimeOwner owns configuration before this backend and destroys it
        // afterwards. Ordinary SourceBackend callers only borrow their state.
        // Constructor-failure and never-published destruction keep their
        // existing host cleanup; they have no completed application close.
        if(aggregate_cleanup_ready_)source_.retire_owned_state_037f70();
    }
    bootstrap_owner::Providers providers() override{return source_.providers();}
    void bind_state(configuration::State& c,config_startup::ConsoleFlags f) override{source_.bind_state(c,f);}
    bool stop_and_join() override{
        if(!source_stopped_){if(!source_.stop_and_join())return false;source_stopped_=true;}
        // Binding and providers remain alive here. Exit source order remains
        // library -> online -> hook, after joins, consumed exactly once.
        if(!exits_drained_){if(!registry_.drain_exit_callbacks())return false;exits_drained_=true;}
        // Exit registration can fail, leaving an empty queue without hook
        // cleanup. Attachment close therefore requires its own explicit
        // disabled-hooks/external-quiescence proof and retains state on false.
        if(!attachment_closed_){if(!source_.finish_callback_attachment())return false;attachment_closed_=true;}
        if(!registry_finished_){if(!registry_.finish())return false;registry_finished_=true;}
        if(!online_closed_){if(!source_.finish_online_component())return false;online_closed_=true;}
        if(!source_.finish_remaining_policies())return false;
        aggregate_cleanup_ready_=true;
        return true;
    }
};
}
Runtime::Runtime(bootstrap_owner::InitialState initial,bootstrap_backend::RuntimeSeed seed,
    bootstrap_backend::Resources resources,bootstrap_native_recovered::Errors& errors,HostApi host)
    :domain_(runtime_domain(resources)),registry_(host),owner_(std::make_unique<bootstrap_owner::RuntimeOwner>(std::move(initial),
        std::make_unique<ClosingBackend>(std::move(seed),prepare(resources,registry_,selected_),registry_),errors)){}
Runtime::~Runtime() noexcept=default;
void Runtime::publish(){owner_->publish();}
bool Runtime::close(){return owner_->close();}
bootstrap_native_recovered::ApplicationEntry Runtime::application_entry() const{return owner_->application_entry();}
bootstrap_owner::Phase Runtime::phase() const noexcept{return owner_->phase();}
const configuration::State& Runtime::configuration() const noexcept{return owner_->configuration();}
const bootstrap_owner::ConsoleState& Runtime::console() const noexcept{return owner_->console();}
std::unique_ptr<Runtime> make_runtime(bootstrap_owner::InitialState initial,bootstrap_backend::RuntimeSeed seed,
    bootstrap_backend::Resources& resources,bootstrap_native_recovered::Errors& errors,HostApi host){
    return std::unique_ptr<Runtime>(new Runtime(std::move(initial),std::move(seed),
        bootstrap_backend::take_resources(resources),errors,host));
}
static_assert(!std::is_abstract_v<NativeExecution>&&!std::is_abstract_v<Runtime>);
static_assert(!std::is_copy_constructible_v<Runtime>&&!std::is_move_constructible_v<Runtime>);
} // namespace anadius::bootstrap_runtime
