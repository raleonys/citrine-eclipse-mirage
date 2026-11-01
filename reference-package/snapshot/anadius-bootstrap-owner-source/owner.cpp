#include "owner.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_owner {
namespace {
Providers checked_providers(const std::unique_ptr<ProviderOwner>& owner){
    if(!owner)throw std::invalid_argument("Bootstrap provider owner is required");
    return owner->providers();
}
config_startup::ConsoleFlags flags(ConsoleState& state){return {state.show_lsx,state.show_console,state.no_console};}
class ControlScope {
public:
    explicit ControlScope(bool& busy):busy_(busy){
        if(busy_)throw std::logic_error("Bootstrap lifecycle reentry is unsupported");
        busy_=true;
    }
    ~ControlScope(){busy_=false;}
private:bool& busy_;
};
}
BootstrapFacade::BootstrapFacade(configuration::State& state,ConsoleState& console,Providers p)
    :BootstrapConfiguration(state,p.configuration,p.files,flags(console),p.input,p.console),providers_(p){}
int BootstrapFacade::register_exit(recovered::RegisteredCallback id){
    if(id!=recovered::RegisteredCallback::exit_038950)throw std::invalid_argument("Unsupported exit callback identity");
    return providers_.services.register_exit(bootstrap_native_recovered::Resolver{}.exit_callback());
}
void BootstrapFacade::install_terminate(recovered::RegisteredCallback id){
    if(id!=recovered::RegisteredCallback::terminate_038890)throw std::invalid_argument("Unsupported terminate callback identity");
    providers_.services.install_terminate(bootstrap_native_recovered::Resolver{}.terminate_callback());
}
void* BootstrapFacade::log_insert_006880(void* stream,std::uintptr_t literal){return providers_.services.log_insert(stream,literal);}
void BootstrapFacade::log_finish_0088e0(void* stream){providers_.services.log_finish(stream);}
void* BootstrapFacade::allocate_04b0bc(std::size_t size){return providers_.services.allocate(size);}
void BootstrapFacade::delete_04b0f8(void* value,std::size_t size){providers_.services.deallocate(value,size);}
recovered::NoArgumentFunction BootstrapFacade::worker(recovered::Worker value){return providers_.services.worker(value);}
recovered::BooleanFunction BootstrapFacade::boolean_worker_035850(){return providers_.services.boolean_worker();}
recovered::Handle BootstrapFacade::begin_thread_05e8e0(recovered::ThreadEntry entry,void* payload,std::uint32_t* id){
    const auto code=bootstrap_native_recovered::Resolver{}.thread_entry(entry);
    return threads_recovered::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(code),
        reinterpret_cast<std::uintptr_t>(payload),0,id,providers_.threads);
}
bool BootstrapFacade::close_handle(recovered::Handle value){return providers_.threads.close_handle(value);}
[[noreturn]] void BootstrapFacade::throw_thread_error_04a648(int index){providers_.services.thread_error(index);std::terminate();}
void BootstrapFacade::between_workers_00e310(){providers_.services.between_workers();}
void BootstrapFacade::guard_return_017a30(){providers_.services.guard_return();}
void BootstrapFacade::cleanup_threads_04a47c(){providers_.services.cleanup_threads();}
void BootstrapFacade::cleanup_035ab0(){providers_.services.cleanup_first();}
void BootstrapFacade::cleanup_0366f0(){providers_.services.cleanup_second();}
void BootstrapFacade::cleanup_0102b0(){providers_.services.cleanup_network();}
void BootstrapFacade::terminate_log_038920(){providers_.services.terminate_log();}
[[noreturn]] void BootstrapFacade::unexpected_terminate_return_trap(){providers_.services.unexpected_terminate_return();std::terminate();}

RuntimeOwner::RuntimeOwner(InitialState initial,std::unique_ptr<ProviderOwner> backend,
    bootstrap_native_recovered::Errors& errors)
    :configuration_(std::move(initial.configuration)),console_(initial.console),backend_(std::move(backend)),
     providers_(checked_providers(backend_)),facade_(configuration_,console_,providers_),bootstrap_(facade_),permanent_errors_(errors){
    backend_->bind_state(configuration_,flags(console_));
}
RuntimeOwner::~RuntimeOwner() noexcept {
    // Do not silently unpublish before pending workers are drained. Binding's
    // own destructor cannot establish backend quiescence or join ownership.
    if(control_busy_||phase_==Phase::Bound||phase_==Phase::Stopping)std::terminate();
}
void RuntimeOwner::publish(){
    ControlScope control(control_busy_);
    if(phase_!=Phase::Constructed)throw std::logic_error("Bootstrap owner cannot publish in this phase");
    binding_.emplace(bootstrap_,permanent_errors_);
    phase_=Phase::Bound;
}
bool RuntimeOwner::close(){
    ControlScope control(control_busy_);
    if(phase_==Phase::Closed)return true;
    if(phase_==Phase::Constructed){phase_=Phase::Closed;return true;}
    phase_=Phase::Stopping;
    // An admitted startup may still register/create workers. Seal only new
    // Application calls, then retry after that startup returns; backend stop
    // and thread admission sealing must not race its remaining source work.
    if(!binding_->pause_application_admission())return false;
    if(!backend_->stop_and_join())return false;
    binding_->close(); // Busy/reentrant failure leaves owner and binding alive.
    binding_.reset();
    phase_=Phase::Closed;
    return true;
}
bootstrap_native_recovered::ApplicationEntry RuntimeOwner::application_entry() const {
    if(phase_!=Phase::Bound)throw std::logic_error("Bootstrap application entry is not available");
    return bootstrap_native_recovered::Resolver{}.application();
}
static_assert(!std::is_abstract_v<BootstrapFacade>);
static_assert(!std::is_abstract_v<RuntimeOwner>);
static_assert(std::is_convertible_v<BootstrapFacade*,recovered::BootstrapDependencies*>);
static_assert(!std::is_copy_constructible_v<RuntimeOwner>&&!std::is_move_constructible_v<RuntimeOwner>);
} // namespace anadius::bootstrap_owner
