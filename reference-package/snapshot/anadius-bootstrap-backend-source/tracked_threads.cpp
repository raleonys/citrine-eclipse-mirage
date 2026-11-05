#include "tracked_threads.hpp"
#include <exception>
#include <list>
#include <mutex>
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_backend {
JoinApi native_join_api() noexcept {
    return {&::GetCurrentProcess,&::DuplicateHandle,&::WaitForSingleObject,
            &::CloseHandle,&::GetLastError,&::SetLastError};
}
namespace {
struct PreserveError {
    const JoinApi& api;DWORD value;
    explicit PreserveError(const JoinApi& a):api(a),value(a.error()){}
    ~PreserveError(){api.set_error(value);}
};
}
struct TrackingThreads::Impl {
    struct Ticket {HANDLE join=nullptr;bool pending=true;bool untracked=false;};
    threads_recovered::Effects& lower;const JoinApi api;
    mutable std::mutex mutex;std::list<Ticket> tickets;
    bool accepting=true,draining=false,poison=false;
    Impl(threads_recovered::Effects& l,JoinApi a):lower(l),api(a){
        if(!a.process||!a.duplicate||!a.wait||!a.close||!a.error||!a.set_error)
            throw std::invalid_argument("Complete thread join API required");
    }
};
TrackingThreads::TrackingThreads(threads_recovered::Effects& lower,JoinApi api)
    :impl_(std::make_unique<Impl>(lower,api)){}
TrackingThreads::~TrackingThreads() noexcept {
    std::lock_guard lock(impl_->mutex);
    if(!impl_->tickets.empty()||impl_->poison||impl_->draining)std::terminate();
}
threads_recovered::Handle TrackingThreads::create_thread(void* security,std::uint64_t stack,
    threads_recovered::Context* context,std::uint32_t flags,std::uint32_t* id){
    auto& s=*impl_;PreserveError preserve(s.api);
    std::list<Impl::Ticket>::iterator ticket;
    {
        std::lock_guard lock(s.mutex);
        if(!s.accepting||s.poison)throw std::logic_error("Thread creation admission is closed");
        // All tracking allocation precedes creation of a possibly-running job.
        s.tickets.emplace_back();ticket=std::prev(s.tickets.end());
    }
    threads_recovered::Handle handle;
    try {
        s.api.set_error(preserve.value);
        handle=s.lower.create_thread(security,stack,context,flags,id);
        preserve.value=s.api.error();
    }catch(...){
        preserve.value=s.api.error();
        // A generic lower Effects may have started a job before throwing. No
        // returned HANDLE is available to join it; never infer rollback.
        std::lock_guard lock(s.mutex);
        ticket->pending=false;ticket->untracked=true;s.poison=true;throw;
    }
    if(!handle){
        std::lock_guard lock(s.mutex);s.tickets.erase(ticket);return 0;
    }
    // The lower create may already have freed context. Only its returned handle
    // is touched from this point; original handle and ID remain unchanged.
    HANDLE joined=nullptr;BOOL duplicated=FALSE;
    try {
        const auto process=s.api.process();
        duplicated=s.api.duplicate(process,reinterpret_cast<HANDLE>(handle),process,
            &joined,0,FALSE,DUPLICATE_SAME_ACCESS);
    }catch(...){
        std::lock_guard lock(s.mutex);
        ticket->pending=false;ticket->untracked=true;s.poison=true;throw;
    }
    {
        std::lock_guard lock(s.mutex);ticket->pending=false;
        if(duplicated&&joined)ticket->join=joined;
        else {ticket->untracked=true;s.poison=true;}
    }
    return handle;
}
bool TrackingThreads::stop_and_join(std::uint32_t timeout_ms){
    auto& s=*impl_;PreserveError preserve(s.api);
    {
        std::lock_guard lock(s.mutex);s.accepting=false;
        if(s.draining||s.poison)return false;
        for(const auto& ticket:s.tickets)if(ticket.pending||ticket.untracked)return false;
        if(s.tickets.empty())return true;
        s.draining=true;
    }
    struct Drain {
        Impl& s;
        ~Drain(){std::lock_guard lock(s.mutex);s.draining=false;}
    } drain{s};
    for(;;){
        std::list<Impl::Ticket>::iterator ticket;HANDLE handle;
        {
            std::lock_guard lock(s.mutex);
            if(s.tickets.empty())return true;
            ticket=s.tickets.begin();handle=ticket->join;
        }
        try{
            if(s.api.wait(handle,timeout_ms)!=WAIT_OBJECT_0)return false;
            if(!s.api.close(handle))return false;
        }catch(...){
            // An injected boundary may throw AFTER closing the duplicate. Its
            // numeric HANDLE could already name a different object on retry.
            // Retain the unknown suffix and never repeat wait/close on it.
            std::lock_guard lock(s.mutex);s.poison=true;throw;
        }
        {
            std::lock_guard lock(s.mutex);s.tickets.erase(ticket);
        }
    }
}
std::size_t TrackingThreads::tracked_jobs() const {
    PreserveError preserve(impl_->api);std::lock_guard lock(impl_->mutex);return impl_->tickets.size();
}
bool TrackingThreads::poisoned() const {
    PreserveError preserve(impl_->api);std::lock_guard lock(impl_->mutex);return impl_->poison;
}
threads_recovered::Context* TrackingThreads::calloc_context_066848(){return impl_->lower.calloc_context_066848();}
void TrackingThreads::free_0668c0(threads_recovered::Context* p){impl_->lower.free_0668c0(p);}
bool TrackingThreads::get_module_handle_ex_w(std::uint32_t f,std::uintptr_t a,threads_recovered::Handle* p){return impl_->lower.get_module_handle_ex_w(f,a,p);}
std::uint32_t TrackingThreads::last_error(){return impl_->lower.last_error();}
void TrackingThreads::write_error_slot_05d724(std::uint32_t v){impl_->lower.write_error_slot_05d724(v);}
void TrackingThreads::invalid_parameter_05c954(){impl_->lower.invalid_parameter_05c954();}
void TrackingThreads::map_os_error_05d698(std::uint32_t v){impl_->lower.map_os_error_05d698(v);}
bool TrackingThreads::close_handle(threads_recovered::Handle v){return impl_->lower.close_handle(v);}
bool TrackingThreads::free_library(threads_recovered::Handle v){return impl_->lower.free_library(v);}
threads_recovered::Context*& TrackingThreads::required_thread_slot_064b90(){return impl_->lower.required_thread_slot_064b90();}
threads_recovered::Context* TrackingThreads::optional_thread_context_064d08(){return impl_->lower.optional_thread_context_064d08();}
int TrackingThreads::runtime_policy_06c0d0(){return impl_->lower.runtime_policy_06c0d0();}
int TrackingThreads::initialize_runtime_067320(int v){return impl_->lower.initialize_runtime_067320(v);}
void TrackingThreads::uninitialize_runtime_06736c(){impl_->lower.uninitialize_runtime_06736c();}
std::uint32_t TrackingThreads::invoke_entry(std::uintptr_t e,std::uintptr_t a){return impl_->lower.invoke_entry(e,a);}
[[noreturn]] void TrackingThreads::free_library_and_exit_thread(threads_recovered::Handle h,std::uint32_t c){impl_->lower.free_library_and_exit_thread(h,c);std::terminate();}
[[noreturn]] void TrackingThreads::exit_thread(std::uint32_t c){impl_->lower.exit_thread(c);std::terminate();}
static_assert(!std::is_abstract_v<TrackingThreads>);
} // namespace anadius::bootstrap_backend
