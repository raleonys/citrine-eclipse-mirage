#include "services.hpp"
#include "../anadius-config-iteration-source/thread_error.hpp"
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_policies {
SourceServices::SourceServices(std::shared_ptr<thread_domain::ApplicationMemory> memory,
    std::unique_ptr<bootstrap_owner::Services> lower):memory_(std::move(memory)),lower_(std::move(lower)){
    if(!memory_||!lower_)throw std::invalid_argument("Owned application memory and remaining Services are required");
}
int SourceServices::register_exit(bootstrap_native_recovered::ExitCallback callback){return lower_->register_exit(callback);}
void SourceServices::install_terminate(std::terminate_handler callback){lower_->install_terminate(callback);}
void* SourceServices::log_insert(void* stream,std::uintptr_t value){return lower_->log_insert(stream,value);}
void SourceServices::log_finish(void* stream){lower_->log_finish(stream);}
void* SourceServices::allocate(std::size_t bytes){return memory_->allocate(bytes);}
void SourceServices::deallocate(void* pointer,std::size_t bytes){memory_->deallocate(pointer,bytes);}
recovered::NoArgumentFunction SourceServices::worker(recovered::Worker value){return lower_->worker(value);}
recovered::BooleanFunction SourceServices::boolean_worker(){return lower_->boolean_worker();}
[[noreturn]] void SourceServices::thread_error(int index){throw_thread_error_04a648(index);}
void SourceServices::between_workers(){lower_->between_workers();}
// 0x180017a30 is an actual RET in retained assembly, not an invented successful
// unresolved operation. This does not implement PE Control Flow Guard policy.
void SourceServices::guard_return(){}
void SourceServices::cleanup_threads(){lower_->cleanup_threads();}
void SourceServices::cleanup_first(){lower_->cleanup_first();}
void SourceServices::cleanup_second(){lower_->cleanup_second();}
void SourceServices::cleanup_network(){lower_->cleanup_network();}
void SourceServices::terminate_log(){lower_->terminate_log();}
[[noreturn]] void SourceServices::unexpected_terminate_return(){lower_->unexpected_terminate_return();std::terminate();}
static_assert(!std::is_abstract_v<SourceServices>);
} // namespace anadius::bootstrap_policies
