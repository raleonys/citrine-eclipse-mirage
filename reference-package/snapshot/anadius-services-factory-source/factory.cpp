#include "factory.hpp"
#include "../anadius-config-iteration-source/thread_error.hpp"
#include <cstdlib>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace anadius::services_factory {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
void close_unpublished_options(bootstrap_runtime::CallbackRegistry& registry,Options& options){
    PreserveError error;
    if(registry.published()||registry.pending_exit_callbacks()||registry.active_callbacks())return;
    if(options.memory&&(options.memory->tracking_poisoned()||options.memory->outstanding_allocations()))return;
    if(options.waits){if(options.waits->active())return;(void)options.waits->close();}
}
}
struct Remaining::Impl {
    bootstrap_runtime::CallbackRegistry& registry;
    Options options;
    const DWORD controller=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    mutable std::mutex gate;Phase phase=Phase::Open;std::size_t active=0;bool used=false;
    Impl(bootstrap_runtime::CallbackRegistry& r,Options o):registry(r),options(std::move(o)){}
    void affinity()const{
        if(controller!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())
            throw std::logic_error("Remaining policy close/retirement requires its controller and fiber");
    }
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Closed&&phase!=Phase::Retiring)phase=Phase::Poisoned;}
    struct Call {
        Impl& value;
        explicit Call(Impl& v):value(v){PreserveError error;std::lock_guard lock(value.gate);
            if(value.phase!=Phase::Open){if(value.phase==Phase::Checking)value.phase=Phase::Poisoned;throw std::logic_error("Remaining source admission is closed");}
            ++value.active;value.used=true;
        }
        void complete(){PreserveError error;std::lock_guard lock(value.gate);if(value.phase!=Phase::Open)throw std::logic_error("Remaining source call lost admission during its provider");}
        ~Call(){PreserveError error;std::lock_guard lock(value.gate);--value.active;}
    };
    bool empty_memory(){return !options.memory->tracking_poisoned()&&!options.memory->outstanding_allocations();}
    bool known_close(bool unused){
        affinity();PreserveError error;
        {std::lock_guard lock(gate);
            if(phase==Phase::Closed)return true;
            if(phase==Phase::Retiring)throw std::logic_error("Remaining provider is retiring");
            if(phase==Phase::Checking){phase=Phase::Poisoned;throw std::logic_error("Reentrant Remaining close");}
            if(phase==Phase::Poisoned||active)return false;
            phase=Phase::Checking;
        }
        try{
            const bool registry_done=unused?(!registry.published()&&!registry.pending_exit_callbacks()&&!registry.active_callbacks()):registry.finished();
            if(options.memory->tracking_poisoned()){poison();return false;}
            bool ready=registry_done&&!options.memory->outstanding_allocations()&&!options.waits->active();
            {std::lock_guard lock(gate);if(phase!=Phase::Checking)return false;}
            if(ready)ready=options.waits->close();
            {std::lock_guard lock(gate);if(phase!=Phase::Checking)return false;phase=ready?Phase::Closed:Phase::Open;}
            return ready;
        }catch(...){poison();throw;}
    }
    void validate_stream(void* stream){
        if(stream!=reinterpret_cast<void*>(services_completion::BootstrapLogger::stream_identity)&&stream!=options.logger->token())
            throw std::domain_error("Stream identity outside the owned bootstrap logger");
    }
};
Remaining::Remaining(bootstrap_runtime::CallbackRegistry& registry,Options options):impl_(nullptr){
    try{
        if(!options.memory||!options.logger||!options.waits)
            throw std::invalid_argument("Owned ApplicationMemory, BootstrapLogger and ExitWaitRegistry required");
        impl_=std::make_unique<Impl>(registry,std::move(options));
    }catch(...){close_unpublished_options(registry,options);throw;}
}
Remaining::Impl& Remaining::attached()const{if(!impl_)throw std::logic_error("Remaining provider is unattached");return *impl_;}
Remaining::~Remaining()noexcept{
    if(!impl_)return;auto& v=*impl_;
    try{
        v.affinity();bool unused;
        {PreserveError error;std::lock_guard lock(v.gate);unused=!v.used;if(v.active||v.phase==Phase::Checking||v.phase==Phase::Poisoned)std::terminate();}
        if(v.phase!=Phase::Closed&&(!unused||!v.known_close(true)))std::terminate();
        // Recheck the actual owners, not a stale successful-close snapshot.
        if(!v.empty_memory()||v.options.waits->active())std::terminate();
        {PreserveError error;std::lock_guard lock(v.gate);if(v.active||v.phase!=Phase::Closed)std::terminate();v.phase=Phase::Retiring;}
        // Keep facade/Impl/gate attached through every opaque deleter. Closed
        // owners were intentionally retained until after Resources retirement.
        {Options retiring=std::move(v.options);}
    }catch(...){std::terminate();}
}
int Remaining::register_exit(bootstrap_native_recovered::ExitCallback callback){auto& v=attached();Impl::Call call(v);try{const auto result=v.registry.register_exit(callback);call.complete();return result;}catch(...){v.poison();throw;}}
void Remaining::install_terminate(std::terminate_handler callback){auto& v=attached();Impl::Call call(v);try{v.registry.install_terminate(callback);call.complete();}catch(...){v.poison();throw;}}
void* Remaining::log_insert(void* stream,std::uintptr_t literal){auto& v=attached();Impl::Call call(v);v.validate_stream(stream);(void)services_completion::BootstrapLogger::literal(literal);try{auto* result=v.options.logger->insert(stream,literal);call.complete();return result;}catch(...){v.poison();throw;}}
void Remaining::log_finish(void* stream){auto& v=attached();Impl::Call call(v);v.validate_stream(stream);try{v.options.logger->finish(stream);call.complete();}catch(...){v.poison();throw;}}
void* Remaining::allocate(std::size_t bytes){auto& v=attached();Impl::Call call(v);try{auto* result=v.options.memory->allocate(bytes);call.complete();return result;}catch(...){if(v.options.memory->tracking_poisoned())v.poison();throw;}}
void Remaining::deallocate(void* pointer,std::size_t bytes){auto& v=attached();Impl::Call call(v);try{v.options.memory->deallocate(pointer,bytes);call.complete();}catch(...){if(v.options.memory->tracking_poisoned())v.poison();throw;}}
[[noreturn]]void Remaining::thread_error(int index){auto& v=attached();Impl::Call call(v);bootstrap_policies::throw_thread_error_04a648(index);}
// Actual017a30 retained body is RET; this is not invented Control Flow Guard.
void Remaining::guard_return(){auto& v=attached();Impl::Call call(v);call.complete();}
void Remaining::cleanup_threads(){auto& v=attached();Impl::Call call(v);try{v.options.waits->cleanup_current_thread();call.complete();}catch(...){v.poison();throw;}}
void Remaining::terminate_log(){auto& v=attached();Impl::Call call(v);v.poison();v.options.logger->terminate_log();}
[[noreturn]]void Remaining::unexpected_terminate_return(){auto& v=attached();Impl::Call call(v);v.poison();std::abort();}
bool Remaining::close_after_cleanup(){return attached().known_close(false);}
Snapshot Remaining::snapshot()const{PreserveError error;auto& v=attached();std::lock_guard lock(v.gate);return {v.phase,v.active,v.used};}
std::unique_ptr<bootstrap_backend::RemainingPolicies> make_remaining(bootstrap_runtime::CallbackRegistry& registry,Options options){return std::make_unique<Remaining>(registry,std::move(options));}
static_assert(!std::is_abstract_v<Remaining>&&!std::is_copy_constructible_v<Remaining>&&!std::is_move_constructible_v<Remaining>);
} // namespace anadius::services_factory
