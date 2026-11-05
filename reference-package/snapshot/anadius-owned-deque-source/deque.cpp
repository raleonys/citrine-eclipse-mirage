#include "deque.hpp"
#include <algorithm>
#include <bit>
#include <exception>
#include <limits>
#include <list>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace anadius::owned_deque {
namespace {
struct PreserveError {DWORD saved=::GetLastError();~PreserveError(){::SetLastError(saved);}};
[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* text){lock.unlock();throw std::logic_error(text);}
std::uintptr_t word(std::span<const std::uint8_t> bytes){std::uintptr_t result=0;for(unsigned i=0;i<8;++i)result|=std::uintptr_t(bytes[i])<<(i*8);return result;}
}
struct Deques::Impl {
    s::SessionMemoryDomain& parent;const bool concurrent;std::shared_ptr<void> lease;
    mutable std::mutex gate;Phase phase=Phase::Ready;
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    std::size_t successful=0,allocated=0;bool active=false;
    struct Call;std::unique_ptr<Call> retained;
    struct Admission {s::WriteFinishOwner* owner;DWORD thread;void* fiber;Admission* next=nullptr;};
    Admission* admissions=nullptr;std::list<std::unique_ptr<Call>> calls;
    explicit Impl(s::SessionMemoryDomain& p):parent(p),concurrent(p.operation_policy()==s::OperationPolicy::OriginalConcurrentActors),lease(p.acquire_provider_lease_from_operation()){}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Retiring)phase=Phase::Poisoned;}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Deque provider requires its bound worker thread/fiber");}
    void check(Call* call=nullptr);
    Snapshot snapshot()const;
    void remove_admission(Admission& admission)noexcept{
        auto** at=&admissions;while(*at&&*at!=&admission)at=&(*at)->next;
        if(!*at)std::terminate();*at=admission.next;
    }
    struct Call {
        Impl& v;s::WriteFinishOwner& owner;std::function<void()> checkpoint;
        const std::optional<s::SessionMemoryDomain::JobPermit> job;
        const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();bool running=false;
        s::DequeMapMemory maps;
        void validate(){v.check(this);if(job)job->check(v.parent);(void)v.parent.buffer_context();v.check(this);}
        void admit(){
            std::optional<PreserveError> error;if(v.concurrent)error.emplace();
            v.check(this);if(job)job->check(v.parent);checkpoint();v.check(this);if(job)job->check(v.parent);
            (void)v.parent.buffer_context();if(!v.parent.owns_output_queue(&owner))throw std::logic_error("Deque owner retired during growth");
            if(v.concurrent)maps.resolver=&v.parent.output_map_resolver();else maps.bindings=v.parent.output_map_bindings();
        }
        struct Bytes final:s::ReceiveBufferApi {
            Call& c;explicit Bytes(Call& value):c(value){}
            std::uintptr_t allocate_04b0bc(std::uint64_t n)override{
                c.admit();const bool large=n>=4096;
                if((large&&(n<4135||(n-39)%8))||(!large&&n%8))throw s::UnsupportedInput("Deque allocation requires native map byte extent");
                const auto slots=(large?n-39:n)/8;
                if(slots<8||!std::has_single_bit(slots))throw s::UnsupportedInput("Deque allocation requires power-of-two typed map");
                auto map=c.v.parent.allocate_output_map(slots);
                {PreserveError error;std::lock_guard lock(c.v.gate);++c.v.allocated;}
                c.admit();if(map.size()!=slots||!map.data())throw s::UnsupportedInput("Owned map allocation lost its projection");
                const auto data=reinterpret_cast<std::uintptr_t>(map.data());auto raw=data;
                if(large){raw=word(c.v.parent.buffer_context().memory.view(data-8,8));if(((raw+39)&~std::uintptr_t{31})!=data)throw s::UnsupportedInput("Owned map alignment does not match native growth");}
                (void)c.v.parent.buffer_context().memory.view(raw,n);return raw;
            }
            void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{c.admit();c.v.parent.buffer_context().api.copy_bytes(to,from,n);c.admit();}
            void fill_bytes(std::uintptr_t to,std::uint8_t byte,std::uint64_t n)override{c.admit();c.v.parent.buffer_context().api.fill_bytes(to,byte,n);c.admit();}
            void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{
                c.admit();c.v.parent.free_sized_04b0f8(p,n);c.admit();
            }
            [[noreturn]]void length_error_00ce80()override{c.admit();c.v.poison();throw std::length_error("Selected owned deque allocation length limit");}
            [[noreturn]]void allocation_overflow_00cd30()override{c.admit();c.v.poison();throw std::bad_array_new_length();}
            [[noreturn]]void invalid_parameter_05c974()override{c.admit();c.v.poison();throw std::invalid_argument("Selected owned deque allocation header check");}
        } bytes{*this};
        s::ReceiveBufferContext allocation;
        struct Slots final:s::BoundedDequeGrowthApi {
            Call& c;explicit Slots(Call& value):BoundedDequeGrowthApi(value.maps),c(value){}
            // Bounded copy/zero are final pure span operations. Admission occurs
            // before source entry and allocation/free callbacks; no foreign code
            // is invoked by pointer-slot copy/zero.
            [[noreturn]]void length_error_018460()override{c.admit();c.v.poison();throw std::length_error("Selected deque too long (018460 compatibility)");}
        } slots{*this};
        s::DequeGrowthContext growth;
        Call(Impl& value,s::WriteFinishOwner& q,std::function<void()>& donor,std::optional<s::SessionMemoryDomain::JobPermit> permit)
            :v(value),owner(q),job(std::move(permit)),allocation{value.parent.buffer_context().memory,bytes},growth{allocation,maps,slots}{
            // Swap only after construction: every internal donor becomes empty
            // while the receiving target and all backing metadata remain live.
            checkpoint.swap(donor);
        }
    };
};
void Deques::Impl::check(Call* call){
    PreserveError error;std::unique_lock lock(gate);
    if(!concurrent){if(phase!=Phase::Growing||!active)reject(lock,"Deque growth source admission was poisoned");return;}
    bool found=false;for(const auto& current:calls)if(current.get()==call){found=true;break;}
    if(phase!=Phase::Ready||!found||!call->running||call->thread!=::GetCurrentThreadId()||call->fiber!=::GetCurrentFiber())
        reject(lock,"Deque growth exact actor admission was poisoned");
}
Snapshot Deques::Impl::snapshot()const{
    PreserveError error;std::lock_guard lock(gate);bool busy=active;
    if(concurrent){busy=admissions!=nullptr;for(const auto& call:calls)busy=busy||call->running;}
    return {phase,successful,allocated,busy};
}
Deques::Deques(s::SessionMemoryDomain& parent){(void)parent.buffer_context();impl_=std::make_unique<Impl>(parent);}
Deques::~Deques()noexcept{auto& v=*impl_;PreserveError error;std::lock_guard lock(v.gate);if(v.phase!=Phase::Closed||v.active||v.retained||v.lease||v.admissions||!v.calls.empty())std::terminate();v.phase=Phase::Retiring;}
void Deques::grow(s::WriteFinishOwner& owner,std::uint64_t extra,std::function<void()> checkpoint){
    grow_impl(owner,extra,checkpoint,nullptr);
}
void Deques::grow(s::WriteFinishOwner& owner,std::uint64_t extra,std::function<void()> checkpoint,const s::SessionMemoryDomain::JobPermit& job){
    grow_impl(owner,extra,checkpoint,&job);
}
void Deques::grow_impl(s::WriteFinishOwner& owner,std::uint64_t extra,std::function<void()>& checkpoint,const s::SessionMemoryDomain::JobPermit* permission){
    auto& v=*impl_;
    if(v.concurrent){grow_concurrent(owner,extra,checkpoint,permission);return;}
    const auto job=permission?std::optional<s::SessionMemoryDomain::JobPermit>(*permission):std::nullopt;
    if(job)job->check(v.parent);
    {PreserveError error;std::lock_guard lock(v.gate);if(!job)v.affinity();if(v.active){v.phase=Phase::Poisoned;throw std::logic_error("Deque growth cannot reenter");}if(v.phase!=Phase::Ready)throw std::logic_error("Deque provider is closed or has a retained prefix");}
    if(!checkpoint||!v.parent.owns_output_queue(&owner))throw std::invalid_argument("A live exact Parent-owned output queue and source admission are required");
    if(owner.map_size_d8){
        if(owner.map_size_d8<8||!std::has_single_bit(owner.map_size_d8)||owner.map_d0.size()!=owner.map_size_d8||owner.offset_e0>=owner.map_size_d8||owner.count_e8>=owner.map_size_d8)
            throw s::UnsupportedInput("Owned growth requires a coherent bounded source deque prefix");
        bool matched=false;for(const auto& map:v.parent.output_map_bindings())if(map.slots.data()==owner.map_d0.data()&&map.slots.size()==owner.map_size_d8){if(matched)throw s::UnsupportedInput("Ambiguous owned queue map");matched=true;}
        if(!matched)throw s::UnsupportedInput("Output queue map is not a live typed Parent allocation");
    }else if(owner.map_d0.data()||owner.map_d0.size()||owner.offset_e0||owner.count_e8)throw s::UnsupportedInput("Empty owned deque prefix is inconsistent");
    {PreserveError error;std::lock_guard lock(v.gate);v.phase=Phase::Growing;v.active=true;}
    try{
        v.retained=std::make_unique<Impl::Call>(v,owner,checkpoint,job);auto& call=*v.retained;call.admit();
        s::grow_response_deque_026670(owner,extra,call.growth);call.admit();
        if(owner.map_d0.size()!=owner.map_size_d8||!owner.map_size_d8||!std::has_single_bit(owner.map_size_d8)||owner.offset_e0>=owner.map_size_d8||owner.count_e8>=owner.map_size_d8)
            throw s::UnsupportedInput("Deque growth returned an inconsistent live prefix");
        v.retained.reset();if(job)job->check(v.parent);v.check();{PreserveError error;std::lock_guard lock(v.gate);++v.successful;v.active=false;v.phase=Phase::Ready;}
    }catch(...){v.poison();{PreserveError error;std::lock_guard lock(v.gate);v.active=false;}throw;}
}
void Deques::grow_concurrent(s::WriteFinishOwner& owner,std::uint64_t extra,std::function<void()>& checkpoint,const s::SessionMemoryDomain::JobPermit* permission){
    auto& v=*impl_;const auto job=permission?std::optional<s::SessionMemoryDomain::JobPermit>(*permission):std::nullopt;
    {PreserveError error;if(job)job->check(v.parent);else v.affinity();}
    if(!checkpoint)throw std::invalid_argument("A live exact Parent-owned output queue and source admission are required");
    Impl::Admission admission{&owner,::GetCurrentThreadId(),::GetCurrentFiber()};
    {
        PreserveError error;std::unique_lock lock(v.gate);
        if(v.phase!=Phase::Ready)reject(lock,"Deque provider is closed or has a retained prefix");
        for(const auto& call:v.calls)if(call->running&&call->thread==admission.thread&&call->fiber==admission.fiber){v.phase=Phase::Poisoned;reject(lock,"Deque growth cannot reenter its actor");}
        for(auto* pending=v.admissions;pending;pending=pending->next)if(pending->thread==admission.thread&&pending->fiber==admission.fiber){v.phase=Phase::Poisoned;reject(lock,"Deque growth admission cannot reenter its actor");}
        for(const auto& call:v.calls)if(call->running&&&call->owner==&owner)reject(lock,"Deque queue already has an active growth owner");
        for(auto* pending=v.admissions;pending;pending=pending->next)if(pending->owner==&owner)reject(lock,"Deque queue already has a preparing growth owner");
        admission.next=v.admissions;v.admissions=&admission;
    }
    struct Pending {
        Impl& v;Impl::Admission& admission;bool linked=true;
        ~Pending(){PreserveError error;if(linked){std::lock_guard lock(v.gate);v.remove_admission(admission);}}
        void remove_locked(){v.remove_admission(admission);linked=false;}
    } pending{v,admission};
    // This exact-full-map check is only the initial preflight. Native026670
    // legitimately retains the old owner pointer briefly AFTER freeing it.
    if(!v.parent.owns_output_queue(&owner))throw std::invalid_argument("A live exact Parent-owned output queue and source admission are required");
    if(owner.map_size_d8){
        if(owner.map_size_d8<8||!std::has_single_bit(owner.map_size_d8)||owner.map_d0.size()!=owner.map_size_d8||owner.offset_e0>=owner.map_size_d8||owner.count_e8>=owner.map_size_d8)
            throw s::UnsupportedInput("Owned growth requires a coherent bounded source deque prefix");
        if(!v.parent.owns_output_map(owner.map_d0))throw s::UnsupportedInput("Output queue map is not a live exact typed Parent allocation");
    }else if(owner.map_d0.data()||owner.map_d0.size()||owner.offset_e0||owner.count_e8)throw s::UnsupportedInput("Empty owned deque prefix is inconsistent");
    std::list<std::unique_ptr<Impl::Call>> prepared,retired;Impl::Call* call=nullptr;
    try{
        {
            PreserveError error;
            prepared.emplace_back();prepared.front()=std::make_unique<Impl::Call>(v,owner,checkpoint,job);
            call=prepared.front().get();std::unique_lock lock(v.gate);
            // Always retain a fully constructed descriptor before reporting a
            // sticky failure encountered during compiler metadata allocation.
            v.calls.splice(v.calls.end(),prepared);pending.remove_locked();
            if(v.phase!=Phase::Ready)reject(lock,"Deque preparation lost source admission");
            call->running=true;
        }
        call->admit();s::grow_response_deque_026670(owner,extra,call->growth);call->admit();
        if(owner.map_d0.size()!=owner.map_size_d8||!owner.map_size_d8||!std::has_single_bit(owner.map_size_d8)||owner.offset_e0>=owner.map_size_d8||owner.count_e8>=owner.map_size_d8)
            throw s::UnsupportedInput("Deque growth returned an inconsistent live prefix");
        {
            PreserveError error;std::function<void()> retiring;
            retiring.swap(call->checkpoint);retiring={};
            call->validate(); // target retirement may reenter or revoke a permit
        }
        {
            PreserveError error;std::unique_lock lock(v.gate);
            if(v.phase!=Phase::Ready||!call->running)reject(lock,"Deque success retirement was poisoned");
            call->running=false;++v.successful;
            const auto row=std::find_if(v.calls.begin(),v.calls.end(),[&](const auto& item){return item.get()==call;});
            if(row==v.calls.end())std::terminate();retired.splice(retired.end(),v.calls,row);
        }
        {PreserveError error;retired.clear();}
    }catch(...){
        PreserveError error;std::unique_lock lock(v.gate);if(v.phase!=Phase::Retiring)v.phase=Phase::Poisoned;
        if(call)call->running=false;
        if(!prepared.empty()&&prepared.front())v.calls.splice(v.calls.end(),prepared);
        throw;
    }
}
bool Deques::close(){
    auto& v=*impl_;
    if(v.concurrent){
        PreserveError error;v.affinity();
        {
            std::unique_lock lock(v.gate);
            if(v.phase==Phase::Closed)return true;
            if(v.phase!=Phase::Ready)reject(lock,"Unknown deque prefix cannot close");
            if(v.admissions||!v.calls.empty())return false;
            v.phase=Phase::Retiring;
        }
        try{(void)v.parent.buffer_context();}
        catch(...){std::lock_guard lock(v.gate);if(v.phase==Phase::Retiring)v.phase=Phase::Ready;throw;}
        {std::lock_guard lock(v.gate);v.phase=Phase::Closed;}
        v.lease.reset();return true;
    }
    {PreserveError error;std::lock_guard lock(v.gate);v.affinity();if(v.active){v.phase=Phase::Poisoned;throw std::logic_error("Active deque growth cannot close");}if(v.phase==Phase::Closed)return true;if(v.phase!=Phase::Ready||v.retained)throw std::logic_error("Unknown deque prefix cannot close");}
    (void)v.parent.buffer_context();
    {PreserveError error;std::lock_guard lock(v.gate);v.phase=Phase::Closed;}
    v.lease.reset();return true;
}
Snapshot Deques::snapshot()const{return impl_->snapshot();}
} // namespace anadius::owned_deque
