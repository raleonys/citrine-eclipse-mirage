#include "handoff.hpp"
#include <exception>
#include <mutex>
#include <stdexcept>

namespace anadius::worker_handoff {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
}
struct PreparedMemory::State {
    const DWORD controller=::GetCurrentThreadId();void* const controller_fiber=::GetCurrentFiber();
    const std::shared_ptr<thread_domain::DomainStorage> domain;
    const DWORD slot;void* const controller_ptd;
    std::shared_ptr<thread_domain::ApplicationMemory> backing;
    mutable std::mutex gate;Phase phase=Phase::Prepared;
    DWORD worker=0;void* worker_fiber=nullptr;void* worker_ptd=nullptr;
    HANDLE handle=nullptr;bool verified=false;
    explicit State(std::shared_ptr<thread_domain::DomainStorage> d)
        :domain(std::move(d)),slot(domain->native_fls_index()),controller_ptd(domain->current_ptd()),backing(domain->make_memory()){}
    void control_locked(){
        if(::GetCurrentThreadId()!=controller||::GetCurrentFiber()!=controller_fiber)
            throw std::logic_error("PreparedMemory lifecycle belongs to its original controller/fiber");
        if(::FlsGetValue(slot)!=controller_ptd){phase=Phase::Poisoned;throw std::logic_error("Controller Domain PTD changed");}
    }
    void worker_locked(){
        if(::GetCurrentThreadId()!=worker)throw std::logic_error("Memory borrow belongs to its original Domain worker");
        if(::GetCurrentFiber()!=worker_fiber||::FlsGetValue(slot)!=worker_ptd){phase=Phase::Poisoned;throw std::logic_error("Worker fiber/Domain PTD changed");}
        if(phase!=Phase::Borrowed)throw std::logic_error("Memory capability is no longer borrowed");
    }
};
PreparedMemory::PreparedMemory(std::shared_ptr<State> s)noexcept:state_(std::move(s)){}
std::unique_ptr<PreparedMemory> PreparedMemory::prepare(std::shared_ptr<thread_domain::DomainStorage> domain){
    PreserveError error;
    if(!domain||domain->phase()!=thread_domain::Phase::Ready)throw std::invalid_argument("A ready controller-owned Domain is required");
    return std::unique_ptr<PreparedMemory>(new PreparedMemory(std::make_shared<State>(std::move(domain))));
}
PreparedMemory::~PreparedMemory()noexcept{
    PreserveError error;auto& s=*state_;std::lock_guard lock(s.gate);
    if(::GetCurrentThreadId()!=s.controller||::GetCurrentFiber()!=s.controller_fiber||s.phase!=Phase::Closed||s.backing||s.handle)std::terminate();
}
PreparedMemory::Borrow::Borrow(std::shared_ptr<State> s)noexcept:state_(std::move(s)){}
PreparedMemory::Borrow::Borrow(Borrow&& b)noexcept:state_(std::move(b.state_)){}
PreparedMemory::Borrow::~Borrow()noexcept{
    PreserveError error;if(!state_)return;std::lock_guard lock(state_->gate);state_->phase=Phase::Poisoned;
}
PreparedMemory::Borrow PreparedMemory::acquire(){
    PreserveError error;auto& s=*state_;void* const fiber=::GetCurrentFiber();const auto thread=::GetCurrentThreadId();
    {
        std::lock_guard lock(s.gate);
        if(s.phase!=Phase::Prepared)throw std::logic_error("Dedicated memory capability may be acquired only once");
        auto* ptd=static_cast<crt_ptd_recovered::Ptd*>(::FlsGetValue(s.slot));
        if(thread==s.controller||!ptd||ptd==crt_ptd_recovered::construction_sentinel()||ptd==s.controller_ptd||!ptd->thread_context)
            throw std::logic_error("A distinct ready same-Domain NativeThreads worker is required");
        s.worker=thread;s.worker_fiber=fiber;s.worker_ptd=ptd;s.phase=Phase::Acquiring;
    }
    HANDLE handle=nullptr;
    if(!::DuplicateHandle(::GetCurrentProcess(),::GetCurrentThread(),::GetCurrentProcess(),&handle,SYNCHRONIZE,FALSE,0)){
        std::lock_guard lock(s.gate);
        if(s.phase==Phase::Acquiring&&s.worker==thread&&s.worker_fiber==fiber&&::FlsGetValue(s.slot)==s.worker_ptd){
            s.worker=0;s.worker_fiber=nullptr;s.worker_ptd=nullptr;s.phase=Phase::Prepared;
        }else s.phase=Phase::Poisoned;
        throw std::runtime_error("Cannot retain the actual worker thread handle");
    }
    {std::lock_guard lock(s.gate);s.handle=handle;
        if(s.phase!=Phase::Acquiring||s.worker!=thread||s.worker_fiber!=fiber||::FlsGetValue(s.slot)!=s.worker_ptd){
            s.phase=Phase::Poisoned;throw std::logic_error("Worker admission changed while retaining its actual handle");
        }
        s.phase=Phase::Borrowed;
    }
    return Borrow(state_);
}
thread_domain::ApplicationMemory& PreparedMemory::Borrow::memory()const{
    PreserveError error;if(!state_)throw std::logic_error("Memory borrow was moved or returned");
    std::lock_guard lock(state_->gate);state_->worker_locked();return *state_->backing;
}
const thread_domain::DomainStorage* PreparedMemory::Borrow::domain_identity()const noexcept{return state_?state_->domain.get():nullptr;}
bool PreparedMemory::Borrow::retire(){
    PreserveError error;if(!state_)throw std::logic_error("Memory borrow was moved or returned");
    auto held=state_;
    {
        std::lock_guard lock(held->gate);held->worker_locked();
        if(held->backing->tracking_poisoned()){held->phase=Phase::Poisoned;return false;}
        if(held->backing->outstanding_allocations())return false;
        held->phase=Phase::Returned;
    }
    state_.reset();return true;
}
bool PreparedMemory::close(DWORD timeout){
    PreserveError error;auto& s=*state_;HANDLE handle;
    {
        std::lock_guard lock(s.gate);s.control_locked();
        if(s.phase==Phase::Closed)return true;
        if(s.phase!=Phase::Returned)return false;
        s.phase=Phase::Retiring;handle=s.handle;
    }
    const auto result=::WaitForSingleObject(handle,timeout);
    if(result!=WAIT_OBJECT_0){std::lock_guard lock(s.gate);s.phase=result==WAIT_TIMEOUT?Phase::Returned:Phase::Poisoned;return false;}
    // Native thread signaled means its FLS callbacks have completed as well.
    {std::lock_guard lock(s.gate);s.verified=true;}
    if(!::CloseHandle(handle)){std::lock_guard lock(s.gate);s.phase=Phase::Poisoned;return false;}
    std::shared_ptr<thread_domain::ApplicationMemory> retiring;
    {std::lock_guard lock(s.gate);s.handle=nullptr;retiring=std::move(s.backing);}
    retiring.reset(); // sole provider release on the original controller ONLY
    {std::lock_guard lock(s.gate);s.phase=Phase::Closed;}return true;
}
bool PreparedMemory::cancel(){
    PreserveError error;auto& s=*state_;std::shared_ptr<thread_domain::ApplicationMemory> retiring;
    {std::lock_guard lock(s.gate);s.control_locked();if(s.phase==Phase::Closed)return true;if(s.phase!=Phase::Prepared)return false;s.phase=Phase::Retiring;retiring=std::move(s.backing);}
    retiring.reset();{std::lock_guard lock(s.gate);s.phase=Phase::Closed;}return true;
}
Snapshot PreparedMemory::snapshot()const{
    PreserveError error;auto& s=*state_;std::lock_guard lock(s.gate);return {s.phase,s.controller,s.worker,s.handle!=nullptr,s.verified};
}
const thread_domain::DomainStorage* PreparedMemory::domain_identity()const noexcept{return state_->domain.get();}
} // namespace anadius::worker_handoff
