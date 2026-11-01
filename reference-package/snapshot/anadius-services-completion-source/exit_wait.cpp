#include "exit_wait.hpp"
#include "../anadius-config-iteration-source/thread_error.hpp"
#include <array>
#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include <vector>

namespace anadius::services_completion {
namespace {
template<class F>auto sdk(F&& f) noexcept {try{return f();}catch(...){std::abort();}}
ExitWaitApi checked(ExitWaitApi a){
    if(!a.thread_id||!a.initialize||!a.destroy||!a.enter||!a.leave||!a.acquire||!a.release||!a.wake_all||!a.error)
        throw std::invalid_argument("Complete exit-wait SDK API required");
    return a;
}
}
ExitWaitApi native_exit_wait_api() noexcept{
    return {::GetCurrentThreadId,::InitializeCriticalSectionEx,::DeleteCriticalSection,
        ::EnterCriticalSection,::LeaveCriticalSection,::AcquireSRWLockExclusive,
        ::ReleaseSRWLockExclusive,::WakeAllConditionVariable,::GetLastError};
}
WaitTarget::WaitTarget(std::uint32_t type,std::int32_t ready,ExitWaitApi a):api_(checked(a)),ready_(ready){
    execution_recovered::initialize_mutex_04a698(mutex_,type);
}
WaitTarget::~WaitTarget() noexcept{if(owner_.load()||registration_.load())std::abort();}
void WaitTarget::lock(){
    const auto tid=sdk([&]{return api_.thread_id();});
    if(owner_.load()==tid||registration_.load())throw std::logic_error("Target is recursive or still registered");
    sdk([&]{api_.acquire(&native_lock_);});
    if(registration_.load()){
        sdk([&]{api_.release(&native_lock_);});
        throw std::logic_error("Target cleanup must finish before reuse");
    }
    mutex_.owner_48=tid;mutex_.count_4c=1;owner_.store(tid);
}
struct WaitTarget::ReleasePlatform final:execution_recovered::Platform {
    WaitTarget& target;
    explicit ReleasePlatform(WaitTarget& t):target(t){}
    std::uint32_t current_thread_id() override{return sdk([&]{return target.api_.thread_id();});}
    void acquire_srw_exclusive(std::uintptr_t&) override{throw std::logic_error("Release-only platform");}
    void release_srw_exclusive(std::uintptr_t& cell) override{
        if(&cell!=&target.mutex_.srw_10)std::abort();
        target.owner_.store(0);
        sdk([&]{target.api_.release(&target.native_lock_);});
    }
    std::int64_t query_performance_frequency() override{throw std::logic_error("Release-only platform");}
    std::int64_t query_performance_counter() override{throw std::logic_error("Release-only platform");}
    void sleep_milliseconds(std::uint32_t) override{throw std::logic_error("Release-only platform");}
    [[noreturn]] void raise_thread_error_04a648(int i) override{bootstrap_policies::throw_thread_error_04a648(i);}
};
void WaitTarget::unlock(){
    const auto tid=sdk([&]{return api_.thread_id();});
    if(owner_.load()!=tid||registration_.load())throw std::logic_error("Only the unregistered owning thread may unlock");
    ReleasePlatform platform(*this);(void)execution_recovered::unlock_04a6b4(mutex_,platform);
}
void WaitTarget::release_registered(DWORD tid){
    if(owner_.load()!=tid||registration_.load()!=tid||mutex_.count_4c!=1)std::abort();
    ReleasePlatform platform(*this);(void)execution_recovered::unlock_04a6b4(mutex_,platform);
}
bool WaitTarget::is_locked() const noexcept{return owner_.load()!=0;}
bool WaitTarget::registered() const noexcept{return registration_.load()!=0;}
std::int32_t WaitTarget::ready_value_quiescent() const noexcept{return ready_;}
struct ExitWaitRegistry::Impl {
    struct Block {recovered::ExitWaitBlock source{};std::array<std::shared_ptr<WaitTarget>,20> owners;};
    ExitWaitApi api;CRITICAL_SECTION gate{};Block head;std::vector<std::unique_ptr<Block>> tail;
    DWORD controller;bool stopping=false,closed=false;
    explicit Impl(ExitWaitApi a):api(checked(a)),controller(sdk([&]{return api.thread_id();})){
        if(sdk([&]{return api.initialize(&gate,0x400,0);})==FALSE)
            throw std::system_error(static_cast<int>(sdk([&]{return api.error();})),std::system_category(),"Exit-wait CS initialization");
    }
    ~Impl() noexcept{
        if(!closed||active_unlocked())std::abort();
        sdk([&]{api.destroy(&gate);});
    }
    struct Guard {
        Impl& value;
        explicit Guard(Impl& v):value(v){sdk([&]{value.api.enter(&value.gate);});}
        ~Guard(){sdk([&]{value.api.leave(&value.gate);});}
    };
    template<class F>void each(F&& f){f(head);for(auto& b:tail)f(*b);}
    std::size_t active_unlocked() const{
        std::size_t n=static_cast<std::size_t>(head.source.active_count);
        for(const auto& b:tail)n+=static_cast<std::size_t>(b->source.active_count);
        return n;
    }
    WaitTarget* target(void* pointer,bool condition){
        WaitTarget* result=nullptr;
        each([&](Block& b){for(const auto& p:b.owners)if(p&&pointer==(condition?static_cast<void*>(&p->condition_):static_cast<void*>(&p->mutex_)))result=p.get();});
        if(!result)std::abort();return result;
    }
    void reclaim(){
        each([](Block& b){for(std::size_t i=0;i<20;++i)if(b.owners[i]&&!b.source.entries[i].mutex){
            b.owners[i]->registration_.store(0);b.owners[i].reset();
        }});
    }
    struct Effects final:recovered::ExitWaitDependencies {
        Impl& owner;DWORD tid=0;bool locked=false;
        explicit Effects(Impl& v):owner(v){}
        std::uint32_t current_thread_id() override{tid=sdk([&]{return owner.api.thread_id();});return tid;}
        void lock_047df8() override{
            sdk([&]{owner.api.enter(&owner.gate);});locked=true;
            if(owner.closed){sdk([&]{owner.api.leave(&owner.gate);});locked=false;throw std::logic_error("Closed exit-wait registry");}
        }
        void unlock_047e04() override{
            owner.reclaim();sdk([&]{owner.api.leave(&owner.gate);});locked=false;
        }
        void unlock_mutex_04a6b4(void* p) override{owner.target(p,false)->release_registered(tid);}
        void notify_all_04ae28(void* p) override{
            auto* t=owner.target(p,true);sdk([&]{owner.api.wake_all(&t->condition_);});
        }
    };
};
ExitWaitRegistry::ExitWaitRegistry(InitialRegistry seed,ExitWaitApi api){
    if(seed!=InitialRegistry::empty_owned)throw std::invalid_argument("Explicit owned empty seed required");
    impl_=std::make_unique<Impl>(api);
}
ExitWaitRegistry::~ExitWaitRegistry() noexcept=default;
void ExitWaitRegistry::register_current_thread(std::shared_ptr<WaitTarget> target,bool publish){
    if(!target)throw std::invalid_argument("Owned target required");
    auto& v=*impl_;const auto tid=sdk([&]{return v.api.thread_id();});Impl::Guard guard(v);
    if(v.stopping||v.closed)throw std::logic_error("Exit-wait registration stopped");
    if(target->owner_.load()!=tid||target->mutex_.count_4c!=1||target->registration_.load())
        throw std::logic_error("Registration requires this thread's unregistered count1 target");
    Impl::Block* block=nullptr;std::size_t index=0;
    v.each([&](Impl::Block& b){if(!block)for(std::size_t i=0;i<20;++i)if(!b.source.entries[i].mutex&&!b.owners[i]){block=&b;index=i;break;}});
    if(!block){
        auto fresh=std::make_unique<Impl::Block>();block=fresh.get();
        auto* previous=v.tail.empty()?&v.head:v.tail.back().get();
        v.tail.push_back(std::move(fresh));previous->source.next=&block->source;
    }
    DWORD expected=0;
    if(!target->registration_.compare_exchange_strong(expected,tid))throw std::logic_error("Target belongs to another registry");
    // No allocation or user callback after publication begins.
    block->owners[index]=target;auto& entry=block->source.entries[index];
    entry.thread_id=tid;entry.mutex=&target->mutex_;entry.condition=&target->condition_;
    entry.ready=publish?&target->ready_:nullptr;++block->source.active_count;
}
void ExitWaitRegistry::cleanup_current_thread(){
    auto& v=*impl_;Impl::Effects effects(v);
    try{recovered::cleanup_exit_waits(v.head.source,effects);}
    catch(...){if(effects.locked)std::abort();throw;}
}
std::size_t ExitWaitRegistry::active() const{auto& v=*impl_;Impl::Guard guard(v);return v.active_unlocked();}
std::size_t ExitWaitRegistry::blocks() const{auto& v=*impl_;Impl::Guard guard(v);return 1+v.tail.size();}
bool ExitWaitRegistry::close(){
    auto& v=*impl_;
    if(sdk([&]{return v.api.thread_id();})!=v.controller)throw std::logic_error("Registry close requires original controller");
    Impl::Guard guard(v);v.stopping=true;
    if(v.active_unlocked())return false;
    v.closed=true;return true;
}
static_assert(sizeof(SRWLOCK)==sizeof(std::uintptr_t));
} // namespace anadius::services_completion
