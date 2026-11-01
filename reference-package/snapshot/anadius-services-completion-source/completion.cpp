#include "completion.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <cstdlib>
#include <stdexcept>
#include <type_traits>

namespace anadius::services_completion {
BootstrapLogger::BootstrapLogger(std::shared_ptr<std::ostream> stream):stream_(std::move(stream)){
    if(!stream_)throw std::invalid_argument("An explicit owned compiler output stream is required");
}
std::string_view BootstrapLogger::literal(std::uintptr_t address){
    switch(address){
    case 0x180083cd8ULL:return "LSX emu";
    case 0x180086720ULL:return " version ";
    case 0x180086718ULL:return "2.0.1.0";
    case 0x180086700ULL:return "Unhandled exception";
    }
    throw std::domain_error("Literal outside the retained bootstrap logging domain");
}
void* BootstrapLogger::token() noexcept{return &token_;}
void BootstrapLogger::validate(void* value) const{
    if(value!=reinterpret_cast<void*>(stream_identity)&&value!=&token_)
        throw std::domain_error("Stream identity outside the owned bootstrap logger");
}
void* BootstrapLogger::insert(void* stream,std::uintptr_t address){
    validate(stream);
    const auto text=literal(address);
    const auto shared=shared_log::acquire(*stream_);
    const std::lock_guard lock(gate_);
    // Each retained literal is a complete, immutable NUL-terminated C string.
    // Native006880 is a formatted C-string insertion; do not substitute write().
    *stream_<<text.data();
    return token();
}
void BootstrapLogger::finish(void* stream){
    validate(stream);const auto shared=shared_log::acquire(*stream_);const std::lock_guard lock(gate_);
    // Existing0088e0 widens newline, puts it and flushes: compiler endl is the
    // selected source counterpart, not an MSVC stream/facet/object ABI overlay.
    *stream_<<std::endl;
}
[[noreturn]] void BootstrapLogger::terminate_log(){
    auto* stream=insert(reinterpret_cast<void*>(stream_identity),0x180086700ULL);
    finish(stream);
    std::abort();
}
CompletionServices::CompletionServices(std::shared_ptr<BootstrapLogger> logger,
    std::shared_ptr<ExitWaitRegistry> exits,std::unique_ptr<bootstrap_owner::Services> lower)
    :logger_(std::move(logger)),exits_(std::move(exits)),lower_(std::move(lower)){
    if(!logger_||!exits_||!lower_)throw std::invalid_argument("Logger, exit-wait registry and remaining Services owner required");
}
int CompletionServices::register_exit(bootstrap_native_recovered::ExitCallback callback){return lower_->register_exit(callback);}
void CompletionServices::install_terminate(std::terminate_handler callback){lower_->install_terminate(callback);}
void* CompletionServices::log_insert(void* stream,std::uintptr_t address){return logger_->insert(stream,address);}
void CompletionServices::log_finish(void* stream){logger_->finish(stream);}
void* CompletionServices::allocate(std::size_t bytes){return lower_->allocate(bytes);}
void CompletionServices::deallocate(void* pointer,std::size_t bytes){lower_->deallocate(pointer,bytes);}
recovered::NoArgumentFunction CompletionServices::worker(recovered::Worker value){return lower_->worker(value);}
recovered::BooleanFunction CompletionServices::boolean_worker(){return lower_->boolean_worker();}
[[noreturn]] void CompletionServices::thread_error(int index){lower_->thread_error(index);std::terminate();}
void CompletionServices::between_workers(){lower_->between_workers();}
void CompletionServices::guard_return(){lower_->guard_return();}
void CompletionServices::cleanup_threads(){exits_->cleanup_current_thread();}
void CompletionServices::cleanup_first(){lower_->cleanup_first();}
void CompletionServices::cleanup_second(){lower_->cleanup_second();}
void CompletionServices::cleanup_network(){lower_->cleanup_network();}
void CompletionServices::terminate_log(){logger_->terminate_log();}
[[noreturn]] void CompletionServices::unexpected_terminate_return(){
    // Original038890 has INT3 if its fatal routine unexpectedly returns.
    // Select unconditional compiler abort: no debugger-dependent continuation
    // or invented success. Exact INT3/SEH behavior remains a native-ABI gate.
    std::abort();
}
static_assert(!std::is_abstract_v<BootstrapLogger>&&!std::is_abstract_v<CompletionServices>);
static_assert(!std::is_move_constructible_v<BootstrapLogger>);
} // namespace anadius::services_completion
