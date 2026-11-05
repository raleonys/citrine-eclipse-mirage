#include "callbacks.hpp"
#include <atomic>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace anadius::bootstrap_runtime {
namespace {
using Callback=bootstrap_native_recovered::ExitCallback;
struct State {
    explicit State(HostApi value):api(value){}
    HostApi api;
    std::vector<Callback> exits;
    std::terminate_handler terminate_callback=nullptr,previous_terminate=nullptr;
    std::size_t active=0;
    bool published=false,paused=false,finished=false,draining=false,control_busy=false;
    bool installed_terminate=false;
};
struct Hub {std::mutex gate;std::shared_ptr<State> current;};
// Survives C++ static teardown. CRT may retain these image-scope thunk addresses
// after a registry is finished; the thunks never retain a raw provider pointer.
Hub& hub(){static auto* value=new Hub;return *value;}
// CRT retains image-scope thunks after manual finish. Once admission is paused,
// a later exit thunk must not initialize/access Hub or acquire its mutex merely
// to discover that no callback may run. This scalar has static initialization
// and trivial destruction; the image itself must still remain mapped.
constinit std::atomic<bool> dispatch_admitted{false};
static_assert(std::atomic<bool>::is_always_lock_free);
struct LastError {
    DWORD value=GetLastError();
    ~LastError(){SetLastError(value);}
};
void leave(const std::shared_ptr<State>& state) noexcept {
    LastError error;
    std::lock_guard lock(hub().gate);--state->active;
}
struct Lease {
    std::shared_ptr<State> state;
    ~Lease(){if(state)leave(state);}
};
void exit_dispatch() noexcept {
    if(!dispatch_admitted.load(std::memory_order_acquire))return;
    std::shared_ptr<State> state;Callback callback=nullptr;
    {
        LastError error;
        std::lock_guard lock(hub().gate);state=hub().current;
        if(!state||state->paused||state->finished||state->exits.empty())return;
        callback=state->exits.back();state->exits.pop_back();++state->active;
    }
    Lease lease{std::move(state)};
    try{callback();}catch(...){std::terminate();}
}
[[noreturn]] void terminate_dispatch() noexcept {
    if(!dispatch_admitted.load(std::memory_order_acquire))std::abort();
    std::shared_ptr<State> state;std::terminate_handler callback=nullptr;
    {
        LastError error;
        std::lock_guard lock(hub().gate);state=hub().current;
        if(state&&!state->paused&&!state->finished&&state->terminate_callback){
            callback=state->terminate_callback;++state->active;
        }
    }
    if(!callback)std::abort();
    Lease lease{std::move(state)};
    try{callback();}catch(...){std::abort();}
    std::abort();
}
void activate(const std::shared_ptr<State>& state){
    if(state->finished||state->paused)throw std::logic_error("Callback admission is closed");
    if(hub().current&&hub().current!=state)throw std::logic_error("Another callback registry is published");
    hub().current=state;state->published=true;
    dispatch_admitted.store(true,std::memory_order_release);
}
// Only control methods use this guard. pause may run while an actual callback
// is active; no provider callback is invoked while the hub mutex is held.
struct Control {
    std::shared_ptr<State> state;
    explicit Control(std::shared_ptr<State> value):state(std::move(value)){
        std::lock_guard lock(hub().gate);
        if(state->control_busy)throw std::logic_error("Reentrant callback registry control");
        state->control_busy=true;
    }
    ~Control(){LastError error;std::lock_guard lock(hub().gate);state->control_busy=false;}
};
}
struct CallbackRegistry::Impl {std::shared_ptr<State> state;explicit Impl(HostApi api):state(std::make_shared<State>(api)){}};
HostApi native_host_api() noexcept{return {&std::atexit,&std::set_terminate,&std::get_terminate};}
CallbackRegistry::CallbackRegistry(HostApi api){
    if(!api.at_exit||!api.set_terminate||!api.get_terminate)throw std::invalid_argument("Complete CRT host API required");
    impl_=std::make_unique<Impl>(api);
}
CallbackRegistry::~CallbackRegistry() noexcept {
    bool unfinished;
    {std::lock_guard lock(hub().gate);const auto& state=*impl_->state;unfinished=state.published&&!state.finished;}
    // Do not enter our possibly installed terminate dispatcher from a registry
    // destructor. There is no safe implicit shutdown of still-live providers.
    if(unfinished)std::abort();
}
int CallbackRegistry::register_exit(Callback callback){
    if(!callback)throw std::invalid_argument("Null exit callback");
    LastError error;auto state=impl_->state;Control control(state);
    {
        std::lock_guard lock(hub().gate);activate(state);state->exits.push_back(callback);
    }
    // Every registration has a real CRT position. Registering only one thunk
    // would reorder interleaved foreign callbacks. Native at_exit never calls
    // the thunk synchronously; concurrent process exit is excluded by contract.
    int result;
    try{result=state->api.at_exit(&exit_dispatch);}
    catch(...){std::lock_guard lock(hub().gate);state->exits.pop_back();throw;}
    if(result){std::lock_guard lock(hub().gate);state->exits.pop_back();}
    return result;
}
void CallbackRegistry::install_terminate(std::terminate_handler callback){
    if(!callback)throw std::invalid_argument("Null terminate callback");
    LastError error;auto state=impl_->state;Control control(state);
    {std::lock_guard lock(hub().gate);activate(state);state->terminate_callback=callback;}
    const auto previous=state->api.set_terminate(&terminate_dispatch);
    std::lock_guard lock(hub().gate);
    if(!state->installed_terminate||previous!=&terminate_dispatch)state->previous_terminate=previous;
    state->installed_terminate=true;
}
bool CallbackRegistry::pause_admission(){
    LastError error;auto state=impl_->state;
    std::lock_guard lock(hub().gate);
    if(state->finished)return true;
    state->paused=true;
    // An unpublished contender must not disable the current owner's thunks.
    if(hub().current==state)dispatch_admitted.store(false,std::memory_order_release);
    return !state->active&&!state->draining&&!state->control_busy;
}
bool CallbackRegistry::drain_exit_callbacks(){
    LastError error;auto state=impl_->state;
    {
        std::lock_guard lock(hub().gate);
        if(state->finished)return true;
        if(!state->paused)throw std::logic_error("Pause callback admission before drain");
        if(state->active||state->draining||state->control_busy)return false;
        state->draining=true;
    }
    try{
        for(;;){
            Callback callback;
            {
                std::lock_guard lock(hub().gate);
                if(state->exits.empty()){state->draining=false;return true;}
                callback=state->exits.back();state->exits.pop_back();++state->active;
            }
            Lease lease{state};
            // Manual drain is an ordinary source call: if a synthetic callback
            // throws, the consumed prefix stays consumed and remaining entries
            // are retryable. Native exit_entry itself is an exception boundary.
            SetLastError(error.value);
            try{callback();}catch(...){error.value=GetLastError();throw;}
            error.value=GetLastError();
        }
    }catch(...){std::lock_guard lock(hub().gate);state->draining=false;throw;}
}
bool CallbackRegistry::finish(){
    LastError error;auto state=impl_->state;Control control(state);
    {
        std::lock_guard lock(hub().gate);
        if(state->finished)return true;
        if(!state->paused||state->active||state->draining||!state->exits.empty())return false;
    }
    // No atomic compare/exchange exists for the process terminate handler.
    // Host handler mutations must be serialized with this lifecycle control.
    if(state->installed_terminate&&state->api.get_terminate()==&terminate_dispatch)
        state->api.set_terminate(state->previous_terminate);
    {
        std::lock_guard lock(hub().gate);
        if(hub().current==state){
            dispatch_admitted.store(false,std::memory_order_release);
            hub().current.reset();
        }
        state->published=false;state->finished=true;state->terminate_callback=nullptr;
    }
    return true;
}
std::size_t CallbackRegistry::pending_exit_callbacks()const{LastError error;std::lock_guard lock(hub().gate);return impl_->state->exits.size();}
std::size_t CallbackRegistry::active_callbacks()const{LastError error;std::lock_guard lock(hub().gate);return impl_->state->active;}
bool CallbackRegistry::published()const{LastError error;std::lock_guard lock(hub().gate);return impl_->state->published;}
bool CallbackRegistry::paused()const{LastError error;std::lock_guard lock(hub().gate);return impl_->state->paused;}
bool CallbackRegistry::finished()const{LastError error;std::lock_guard lock(hub().gate);return impl_->state->finished;}

RegisteredServices::RegisteredServices(CallbackRegistry& registry,std::unique_ptr<bootstrap_owner::Services> lower):registry_(registry),lower_(std::move(lower)){
    if(!lower_)throw std::invalid_argument("Remaining Services provider required");
}
int RegisteredServices::register_exit(Callback callback){return registry_.register_exit(callback);}
void RegisteredServices::install_terminate(std::terminate_handler callback){registry_.install_terminate(callback);}
void* RegisteredServices::log_insert(void* stream,std::uintptr_t value){return lower_->log_insert(stream,value);}
void RegisteredServices::log_finish(void* stream){lower_->log_finish(stream);}
void* RegisteredServices::allocate(std::size_t bytes){return lower_->allocate(bytes);}
void RegisteredServices::deallocate(void* value,std::size_t bytes){lower_->deallocate(value,bytes);}
recovered::NoArgumentFunction RegisteredServices::worker(recovered::Worker value){return lower_->worker(value);}
recovered::BooleanFunction RegisteredServices::boolean_worker(){return lower_->boolean_worker();}
[[noreturn]] void RegisteredServices::thread_error(int index){lower_->thread_error(index);std::terminate();}
void RegisteredServices::between_workers(){lower_->between_workers();}
void RegisteredServices::guard_return(){lower_->guard_return();}
void RegisteredServices::cleanup_threads(){lower_->cleanup_threads();}
void RegisteredServices::cleanup_first(){lower_->cleanup_first();}
void RegisteredServices::cleanup_second(){lower_->cleanup_second();}
void RegisteredServices::cleanup_network(){lower_->cleanup_network();}
void RegisteredServices::terminate_log(){lower_->terminate_log();}
[[noreturn]] void RegisteredServices::unexpected_terminate_return(){lower_->unexpected_terminate_return();std::terminate();}
} // namespace anadius::bootstrap_runtime
