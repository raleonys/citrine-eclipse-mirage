#include "native_threads.hpp"
#include <new>
#include <exception>
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_providers {
struct NativeThreads::Launch {NativeThreads* owner;threads_recovered::Context* context;};
NativeThreads::NativeThreads(std::unique_ptr<threads_recovered::Effects> lower,thread_native::Api api,std::shared_ptr<void> lifetime)
    :lifetime_(std::move(lifetime)),lower_(std::move(lower)),api_(api){
    if(!lower_||!api.create||!api.get_module||!api.last_error||!api.close||!api.release||!api.release_and_exit||!api.exit)
        throw std::invalid_argument("Thread lower provider and complete SDK API required");
}
NativeThreads::~NativeThreads() noexcept=default;
DWORD WINAPI NativeThreads::start(void* opaque) noexcept {
    const auto incoming=::GetLastError();
    const auto launch=*static_cast<Launch*>(opaque);
    delete static_cast<Launch*>(opaque);
    ::SetLastError(incoming);
    try{threads_recovered::thread_entry_05e7a8(launch.context,*launch.owner);}
    catch(...){std::terminate();} // no exception across the Win32 callback ABI
}
threads_recovered::Handle NativeThreads::create_thread(void* security,std::uint64_t stack,threads_recovered::Context* context,std::uint32_t flags,std::uint32_t* id){
    const auto incoming=::GetLastError();
    auto* launch=new(std::nothrow) Launch{this,context};
    if(!launch){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return 0;}
    ::SetLastError(incoming);DWORD native_id;
    // SDK FALSE promises no worker, so envelope can be freed. If an injected
    // provider throws with unknown worker outcome, retain the envelope: B3
    // marks that create poisoned and cannot release the containing provider.
    const auto handle=api_.create(static_cast<LPSECURITY_ATTRIBUTES>(security),static_cast<SIZE_T>(stack),&start,launch,flags,id?&native_id:nullptr);
    const auto result_error=::GetLastError();
    if(!handle)delete launch;
    else if(id)*id=static_cast<std::uint32_t>(native_id);
    ::SetLastError(result_error);return reinterpret_cast<threads_recovered::Handle>(handle);
}
threads_recovered::Context* NativeThreads::calloc_context_066848(){return lower_->calloc_context_066848();}
void NativeThreads::free_0668c0(threads_recovered::Context* p){lower_->free_0668c0(p);}
bool NativeThreads::get_module_handle_ex_w(std::uint32_t f,std::uintptr_t a,threads_recovered::Handle* p){return lower_->get_module_handle_ex_w(f,a,p);}
std::uint32_t NativeThreads::last_error(){return lower_->last_error();}
void NativeThreads::write_error_slot_05d724(std::uint32_t v){lower_->write_error_slot_05d724(v);}
void NativeThreads::invalid_parameter_05c954(){lower_->invalid_parameter_05c954();}
void NativeThreads::map_os_error_05d698(std::uint32_t v){lower_->map_os_error_05d698(v);}
bool NativeThreads::close_handle(threads_recovered::Handle v){return lower_->close_handle(v);}
bool NativeThreads::free_library(threads_recovered::Handle v){return lower_->free_library(v);}
threads_recovered::Context*& NativeThreads::required_thread_slot_064b90(){return lower_->required_thread_slot_064b90();}
threads_recovered::Context* NativeThreads::optional_thread_context_064d08(){return lower_->optional_thread_context_064d08();}
int NativeThreads::runtime_policy_06c0d0(){return lower_->runtime_policy_06c0d0();}
int NativeThreads::initialize_runtime_067320(int v){return lower_->initialize_runtime_067320(v);}
void NativeThreads::uninitialize_runtime_06736c(){lower_->uninitialize_runtime_06736c();}
std::uint32_t NativeThreads::invoke_entry(std::uintptr_t e,std::uintptr_t a){return lower_->invoke_entry(e,a);}
[[noreturn]] void NativeThreads::free_library_and_exit_thread(threads_recovered::Handle h,std::uint32_t v){lower_->free_library_and_exit_thread(h,v);std::terminate();}
[[noreturn]] void NativeThreads::exit_thread(std::uint32_t v){lower_->exit_thread(v);std::terminate();}
std::unique_ptr<NativeThreads> make_recovered_threads(ThreadDomain d,thread_native::Api api){
    if(!d.lifetime)throw std::invalid_argument("Recovered thread domain must have an explicit lifetime owner");
    // The private lower object's create_thread is replaced by NativeThreads;
    // only the outer method constructs the Launch consumed by this callback.
    auto lower=std::make_unique<crt_glue::ThreadEffects>(&NativeThreads::start,api,d.runtime,d.runtime_effects,d.errors,d.error_effects,d.dependencies);
    return std::make_unique<NativeThreads>(std::move(lower),api,std::move(d.lifetime));
}
static_assert(!std::is_abstract_v<NativeThreads>);
} // namespace anadius::bootstrap_providers
