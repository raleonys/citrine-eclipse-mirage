#include "handoff.hpp"
#include <exception>
#include <list>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <vector>

namespace anadius::worker_handoff {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
}
struct PreparedMemory::JobState {
    enum class Phase {Offered,Acquiring,Borrowed,Returned,Canceled};
    Phase phase=Phase::Offered;
    DWORD thread=0;void* fiber=nullptr;void* ptd=nullptr;
    HANDLE handle=nullptr;bool verified=false;
    std::size_t scopes=0;
};
struct PreparedMemory::State {
    const DWORD controller=::GetCurrentThreadId();void* const controller_fiber=::GetCurrentFiber();
    const std::shared_ptr<thread_domain::DomainStorage> domain;
    const DWORD slot;void* const controller_ptd;
    std::shared_ptr<thread_domain::ApplicationMemory> backing;
    mutable std::mutex gate;Phase phase=Phase::Prepared;
    DWORD worker=0;void* worker_fiber=nullptr;void* worker_ptd=nullptr;
    HANDLE handle=nullptr;bool verified=false;
    std::list<std::shared_ptr<JobState>> jobs;bool job_control=false;
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
    void job_locked(const JobState& job){
        if(::GetCurrentThreadId()!=job.thread)throw std::logic_error("Job memory belongs to its acquired Domain worker");
        if(::GetCurrentFiber()!=job.fiber||::FlsGetValue(slot)!=job.ptd){phase=Phase::Poisoned;throw std::logic_error("Job worker fiber/Domain PTD changed");}
        if(phase!=Phase::Borrowed||job.phase!=JobState::Phase::Borrowed)
            throw std::logic_error("Job memory capability is no longer borrowed");
    }
    void job_control_locked(){
        worker_locked();if(job_control){phase=Phase::Poisoned;throw std::logic_error("Job memory lifecycle reentry");}
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
    if(::GetCurrentThreadId()!=s.controller||::GetCurrentFiber()!=s.controller_fiber||s.phase!=Phase::Closed||s.backing||s.handle||!s.jobs.empty()||s.job_control)std::terminate();
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
PreparedMemory::JobMemory::JobMemory(std::shared_ptr<State> state,std::shared_ptr<JobState> job)noexcept:state_(std::move(state)),job_(std::move(job)){}
PreparedMemory::JobMemory::JobMemory(JobMemory&& from)noexcept:state_(std::move(from.state_)),job_(std::move(from.job_)){}
PreparedMemory::JobMemory::~JobMemory()noexcept{
    PreserveError error;if(!state_)return;std::lock_guard lock(state_->gate);state_->phase=Phase::Poisoned;
}
PreparedMemory::JobMemory PreparedMemory::Borrow::prepare_job(){
    PreserveError error;if(!state_)throw std::logic_error("Memory borrow was moved or returned");
    auto held=state_;
    {std::lock_guard lock(held->gate);held->job_control_locked();held->job_control=true;}
    struct Preparing {State& state;~Preparing(){std::lock_guard lock(state.gate);state.job_control=false;}} preparing{*held};
    // No capability exists until publication. An allocation failure can retire
    // these unpublished nodes outside gate without guessing a job outcome.
    auto job=std::make_shared<JobState>();std::list<std::shared_ptr<JobState>> prepared;
    prepared.push_back(job);
    {std::lock_guard lock(held->gate);held->worker_locked();
        if(!held->job_control){held->phase=Phase::Poisoned;throw std::logic_error("Job offer preparation lost its reservation");}
        if(held->jobs.size()==held->jobs.max_size())throw std::length_error("Job offers exceed container capacity");
        held->jobs.splice(held->jobs.end(),prepared);
    }
    return JobMemory(std::move(held),std::move(job));
}
void PreparedMemory::JobMemory::acquire(){
    PreserveError error;if(!state_||!job_)throw std::logic_error("Job memory offer was moved or returned");
    auto& s=*state_;auto& job=*job_;const auto thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    {
        std::lock_guard lock(s.gate);
        if(s.phase!=Phase::Borrowed||job.phase!=JobState::Phase::Offered)throw std::logic_error("Job memory offer is not available");
        auto* ptd=static_cast<crt_ptd_recovered::Ptd*>(::FlsGetValue(s.slot));
        if(thread==s.controller||thread==s.worker||!ptd||ptd==crt_ptd_recovered::construction_sentinel()||
           ptd==s.controller_ptd||ptd==s.worker_ptd||!ptd->thread_context)
            throw std::logic_error("A distinct ready same-Domain NativeThreads job worker is required");
        job.thread=thread;job.fiber=fiber;job.ptd=ptd;job.phase=JobState::Phase::Acquiring;
    }
    HANDLE handle=nullptr;
    if(!::DuplicateHandle(::GetCurrentProcess(),::GetCurrentThread(),::GetCurrentProcess(),&handle,SYNCHRONIZE,FALSE,0)){
        std::lock_guard lock(s.gate);
        if(s.phase==Phase::Borrowed&&job.phase==JobState::Phase::Acquiring&&::FlsGetValue(s.slot)==job.ptd){
            job.thread=0;job.fiber=nullptr;job.ptd=nullptr;job.phase=JobState::Phase::Offered;
        }else s.phase=Phase::Poisoned;
        throw std::runtime_error("Cannot retain the actual job worker handle");
    }
    {
        std::lock_guard lock(s.gate);job.handle=handle;
        if(s.phase!=Phase::Borrowed||job.phase!=JobState::Phase::Acquiring||job.thread!=thread||job.fiber!=fiber||::FlsGetValue(s.slot)!=job.ptd){
            s.phase=Phase::Poisoned;throw std::logic_error("Job worker admission changed while retaining its actual handle");
        }
        job.phase=JobState::Phase::Borrowed;
    }
}
thread_domain::ApplicationMemory& PreparedMemory::JobMemory::memory()const{
    PreserveError error;if(!state_||!job_)throw std::logic_error("Job memory offer was moved or returned");
    std::lock_guard lock(state_->gate);state_->job_locked(*job_);return *state_->backing;
}
const thread_domain::DomainStorage* PreparedMemory::JobMemory::domain_identity()const noexcept{return state_?state_->domain.get():nullptr;}
bool PreparedMemory::JobMemory::retire(){
    PreserveError error;if(!state_||!job_)throw std::logic_error("Job memory offer was moved or returned");
    auto held=state_;{std::lock_guard lock(held->gate);held->job_locked(*job_);if(job_->scopes)return false;job_->phase=JobState::Phase::Returned;}
    job_.reset();state_.reset();return true;
}
bool PreparedMemory::JobMemory::cancel(){
    PreserveError error;if(!state_||!job_)throw std::logic_error("Job memory offer was moved or returned");
    auto held=state_;{
        std::lock_guard lock(held->gate);held->job_control_locked();
        if(job_->phase!=JobState::Phase::Offered)return false;
        job_->phase=JobState::Phase::Canceled;
    }
    job_.reset();state_.reset();return true;
}
PreparedMemory::JobScope::JobScope(std::shared_ptr<State> state,std::shared_ptr<JobState> job):state_(std::move(state)),job_(std::move(job)){
    PreserveError error;std::lock_guard lock(state_->gate);state_->job_locked(*job_);
    if(job_->scopes==std::numeric_limits<std::size_t>::max())throw std::overflow_error("Job permission pins exhausted");
    ++job_->scopes;
}
PreparedMemory::JobScope::~JobScope()noexcept{
    PreserveError error;std::lock_guard lock(state_->gate);
    if(!job_->scopes)std::terminate();
    if(state_->phase!=Phase::Borrowed||job_->phase!=JobState::Phase::Borrowed||
       ::GetCurrentThreadId()!=job_->thread||::GetCurrentFiber()!=job_->fiber||::FlsGetValue(state_->slot)!=job_->ptd)
        state_->phase=Phase::Poisoned;
    --job_->scopes;
}
bool PreparedMemory::JobScope::valid()const noexcept{
    PreserveError error;std::lock_guard lock(state_->gate);
    const bool valid=job_->scopes&&state_->phase==Phase::Borrowed&&job_->phase==JobState::Phase::Borrowed&&
        ::GetCurrentThreadId()==job_->thread&&::GetCurrentFiber()==job_->fiber&&::FlsGetValue(state_->slot)==job_->ptd;
    if(!valid)state_->phase=Phase::Poisoned;
    return valid;
}
thread_domain::ApplicationMemory& PreparedMemory::JobScope::memory()const{
    PreserveError error;std::lock_guard lock(state_->gate);state_->job_locked(*job_);
    if(!job_->scopes)throw std::logic_error("Job permission pin is no longer active");
    return *state_->backing;
}
std::unique_ptr<PreparedMemory::JobScope> PreparedMemory::Borrow::pin_job(JobMemory& job)const{
    PreserveError error;
    if(!state_||state_!=job.state_||!job.job_)throw std::invalid_argument("Job capability must name this exact prepared backing root");
    return std::unique_ptr<JobScope>(new JobScope(state_,job.job_));
}
bool PreparedMemory::Borrow::jobs_returned()const{
    PreserveError error;if(!state_)throw std::logic_error("Memory borrow was moved or returned");
    std::lock_guard lock(state_->gate);state_->job_control_locked();
    for(const auto& job:state_->jobs)if(job->scopes||(job->phase!=JobState::Phase::Returned&&job->phase!=JobState::Phase::Canceled))return false;
    return true;
}
std::size_t PreparedMemory::Borrow::reap_jobs(){
    PreserveError error;if(!state_)throw std::logic_error("Memory borrow was moved or returned");
    auto held=state_;std::vector<std::shared_ptr<JobState>> jobs;
    {
        std::lock_guard lock(held->gate);held->job_control_locked();
        for(const auto& job:held->jobs)if(job->phase==JobState::Phase::Returned||job->phase==JobState::Phase::Canceled)jobs.push_back(job);
        held->job_control=true;
    }
    struct Active {State& state;~Active(){std::lock_guard lock(state.gate);state.job_control=false;}} active{*held};
    std::size_t reaped=0;
    try{
        for(const auto& job:jobs){
            HANDLE handle;
            {
                std::lock_guard lock(held->gate);
                if(held->phase!=Phase::Borrowed||!held->job_control)throw std::logic_error("Job reaping was poisoned");
                handle=job->handle;
            }
            if(handle){
                const auto result=::WaitForSingleObject(handle,0);
                if(result==WAIT_TIMEOUT)continue;
                if(result!=WAIT_OBJECT_0)throw std::runtime_error("Job worker wait outcome is unknown");
                {
                    std::lock_guard lock(held->gate);
                    if(held->phase!=Phase::Borrowed||!held->job_control||job->handle!=handle)throw std::logic_error("Job reaping changed during wait");
                    job->verified=true;
                }
                if(!::CloseHandle(handle))throw std::runtime_error("Job worker handle release failed");
                {std::lock_guard lock(held->gate);job->handle=nullptr;}
            }
            std::lock_guard lock(held->gate);
            if(held->phase!=Phase::Borrowed)throw std::logic_error("Job reaping was poisoned");
            held->jobs.remove(job);++reaped;
        }
        return reaped;
    }catch(...){std::lock_guard lock(held->gate);held->phase=Phase::Poisoned;throw;}
}
bool PreparedMemory::Borrow::retire(){
    PreserveError error;if(!state_)throw std::logic_error("Memory borrow was moved or returned");
    auto held=state_;
    {
        std::lock_guard lock(held->gate);held->job_control_locked();
        for(const auto& job:held->jobs)if(job->scopes||(job->phase!=JobState::Phase::Returned&&job->phase!=JobState::Phase::Canceled))return false;
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
    const auto started=::GetTickCount64();
    const auto remaining=[&]() -> DWORD {
        if(timeout==INFINITE)return INFINITE;
        const auto elapsed=::GetTickCount64()-started;
        return elapsed>=timeout?0:timeout-static_cast<DWORD>(elapsed);
    };
    const auto result=::WaitForSingleObject(handle,remaining());
    if(result!=WAIT_OBJECT_0){std::lock_guard lock(s.gate);s.phase=result==WAIT_TIMEOUT?Phase::Returned:Phase::Poisoned;return false;}
    // Native thread signaled means its FLS callbacks have completed as well.
    {std::lock_guard lock(s.gate);s.verified=true;}
    // Logical job return is not actual thread/FLS exit. Retain the backing
    // through every admitted sibling; already closed prefixes are not replayed.
    for(const auto& job:s.jobs){
        if(!job->handle)continue;
        const auto result=::WaitForSingleObject(job->handle,remaining());
        if(result!=WAIT_OBJECT_0){std::lock_guard lock(s.gate);s.phase=result==WAIT_TIMEOUT?Phase::Returned:Phase::Poisoned;return false;}
        {std::lock_guard lock(s.gate);job->verified=true;}
        if(!::CloseHandle(job->handle)){std::lock_guard lock(s.gate);s.phase=Phase::Poisoned;return false;}
        {std::lock_guard lock(s.gate);job->handle=nullptr;}
    }
    if(!::CloseHandle(handle)){std::lock_guard lock(s.gate);s.phase=Phase::Poisoned;return false;}
    std::shared_ptr<thread_domain::ApplicationMemory> retiring;
    {std::lock_guard lock(s.gate);s.handle=nullptr;s.jobs.clear();retiring=std::move(s.backing);}
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
std::size_t PreparedMemory::outstanding_jobs()const{PreserveError error;std::lock_guard lock(state_->gate);return state_->jobs.size();}
} // namespace anadius::worker_handoff
