#include "domain.hpp"
#include <array>
#include <bit>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <algorithm>
#include <list>
#include <limits>
#include <mutex>
#include <new>
#include <stdexcept>
#include <system_error>

namespace anadius::thread_domain {
namespace {
Seed checked(Seed s){
    if(!s.exception_table_identity||!s.exception_table_lifetime)
        throw std::invalid_argument("A live relocated exception identity and lifetime owner are required");
    return s;
}
class ErrorProxy final:public crt_errors_recovered::Effects {
    crt_errors_recovered::Effects* target_=nullptr;
public:
    void bind(crt_errors_recovered::Effects& target){if(target_)throw std::logic_error("Error/PTD proxy may bind only once");target_=&target;}
    void* optional_ptd_064d08() override{if(!target_)throw std::logic_error("Error/PTD proxy is not bound");return target_->optional_ptd_064d08();}
};
class Abort final:public crt_glue::AbortPolicy {
    [[noreturn]] void abort_05f03c() override{std::abort();}
};
class Invalid final:public crt_glue::InvalidPtd {
    const Seed& seed_;const InitialLocale& data_;
public:
    Invalid(crt_ptd_recovered::State& state,crt_ptd_recovered::Effects& fx,const Seed& seed,const InitialLocale& data)
        :InvalidPtd(state,fx),seed_(seed),data_(data){}
    std::uint32_t locale_changed_0a52d8() override{return data_.image().globals().locale_changed;}
    std::array<crt_invalid_recovered::Address,2> locale_pair_0a1938() override{return data_.pair();}
    crt_invalid_recovered::Address encoded_global_handler_0a4f00(std::uint64_t i) override{
        if(i)throw std::out_of_range("Only state-index0 invalid handler is supported");
        return crt_runtime_recovered::encode_pointer(reinterpret_cast<std::uintptr_t>(seed_.invalid_handler),seed_.security_cookie);
    }
    crt_invalid_recovered::Address security_cookie_0a1580() override{return seed_.security_cookie;}
    void invoke_handler(crt_invalid_recovered::Address h,const crt_invalid_recovered::Arguments& a) override{
        if(!seed_.invalid_handler||h!=reinterpret_cast<std::uintptr_t>(seed_.invalid_handler))
            throw std::domain_error("Only the explicitly selected rebuilt invalid handler may be invoked");
        seed_.invalid_handler(a);
    }
    [[noreturn]] void watson_05c9a4(const crt_invalid_recovered::Arguments&) override{std::abort();}
    [[noreturn]] void missing_ptd_05f03c() override{std::abort();}
};
class HeapPolicy final:public crt_heap_native_bridge::Policy {
    crt_lock_native::Banks& banks_;const Seed& seed_;crt_invalid_recovered::Effects* invalid_=nullptr;
public:
    HeapPolicy(crt_lock_native::Banks& b,const Seed& s):banks_(b),seed_(s){}
    void bind(crt_invalid_recovered::Effects& invalid){if(invalid_)throw std::logic_error("Invalid policy may bind only once");invalid_=&invalid;}
    void lock_0625f8(std::uint32_t i) override{if(i!=0)throw std::out_of_range("Only heap handler lock0 is supported");banks_.enter({crt_locks_recovered::Bank::acrt,i});}
    void unlock_06264c(std::uint32_t i) override{if(i!=0)throw std::out_of_range("Only heap handler lock0 is supported");banks_.leave({crt_locks_recovered::Bank::acrt,i});}
    std::int32_t invoke_new_handler(std::uintptr_t h,std::uint64_t n) override{
        if(!seed_.new_handler||h!=reinterpret_cast<std::uintptr_t>(seed_.new_handler))throw std::domain_error("Unselected rebuilt new handler");
        return seed_.new_handler(n);
    }
    void invalid_parameter_05c954() override{if(!invalid_)throw std::logic_error("Invalid policy unbound");crt_invalid_recovered::invalid_parameter_noinfo_05c954(*invalid_);}
};
}
struct DomainStorage::Impl {
    Seed seed;NativeRuntime runtime;crt_runtime_recovered::State runtime_state;
    crt_errors_recovered::State errors;ErrorProxy error_proxy;
    crt_lock_native::Banks banks;HeapPolicy heap_policy;crt_heap_native::Calls calls;
    crt_heap_recovered::State heap;crt_heap_native_bridge::Effects heap_fx;
    InitialLocale locale;Abort abort;crt_fls_native::Binding binding;
    crt_glue::ErrorPtd error_ptd;Invalid invalid;
    Phase phase=Phase::Constructed;const DWORD controller=::GetCurrentThreadId();
    bool busy=false;std::size_t initialized_locks=0;std::atomic<std::size_t> leases=0;
    static constexpr std::array<std::uint32_t,3> lock_ids{0,4,5};
    Impl(Seed s,Apis a):seed(checked(std::move(s))),runtime(a.runtime),runtime_state(seed.security_cookie),
        banks(a.locks),heap_policy(banks,seed),calls(a.heap),
        heap{0,seed.new_mode,seed.security_cookie,crt_runtime_recovered::encode_pointer(reinterpret_cast<std::uintptr_t>(seed.new_handler),seed.security_cookie)},
        heap_fx(calls,errors,error_proxy,heap_policy),locale(heap,heap_fx,banks,seed.exception_table_identity),
        binding(heap,heap_fx,locale.ids(),locale,abort,a.fls),error_ptd(binding.state(),binding),invalid(binding.state(),binding,seed,locale){
        error_proxy.bind(error_ptd);heap_policy.bind(invalid);runtime_state.thread_policy.store(seed.thread_policy);
    }
    struct Control {
        Impl& v;
        explicit Control(Impl& x):v(x){if(v.controller!=::GetCurrentThreadId()||v.busy)throw std::logic_error("Domain control requires its original non-reentrant thread");v.busy=true;}
        ~Control(){v.busy=false;}
    };
    void destroy_locks(){while(initialized_locks){const auto id=lock_ids[initialized_locks-1];banks.destroy({crt_locks_recovered::Bank::acrt,id});--initialized_locks;}}
    bool release_fls(){
        if(binding.state().fls_index!=crt_ptd_recovered::invalid_fls_index){
            (void)crt_ptd_recovered::uninitialize_064ecc(binding.state(),binding);
            return binding.native_index()==crt_ptd_recovered::invalid_fls_index;
        }
        return binding.native_index()==crt_ptd_recovered::invalid_fls_index||binding.retry_release();
    }
};
struct DomainStorage::ProviderLease {
    std::shared_ptr<DomainStorage> owner;
    explicit ProviderLease(std::shared_ptr<DomainStorage> v):owner(std::move(v)){++owner->impl_->leases;}
    ~ProviderLease(){--owner->impl_->leases;}
};
namespace {
// Stack observer preserves the existing source allocator/free algorithms. Its
// flags distinguish a known failed SDK call from a throwing injected boundary
// whose resource outcome cannot safely be inferred.
class MemoryEffects final:public crt_realloc_recovered::Effects {
    crt_heap_native_bridge::Effects& e_;
public:
    bool pending=false,free_succeeded=false;
    std::uint32_t free_error=0;
    explicit MemoryEffects(crt_heap_native_bridge::Effects& e):e_(e){}
    crt_heap_recovered::Heap get_process_heap() override{return e_.get_process_heap();}
    void* heap_alloc(crt_heap_recovered::Heap h,std::uint32_t f,std::uint64_t n) override{
        pending=true;auto* p=e_.heap_alloc(h,f,n);pending=false;return p;
    }
    bool heap_free(crt_heap_recovered::Heap h,std::uint32_t f,void* p) override{
        pending=true;free_succeeded=e_.heap_free(h,f,p);pending=false;return free_succeeded;
    }
    void* heap_realloc(crt_heap_recovered::Heap h,std::uint32_t f,void* p,std::uint64_t n) override{return e_.heap_realloc(h,f,p,n);}
    std::uint32_t last_error() override{free_error=e_.last_error();return free_error;}
    std::uint32_t errno_from_os_error_05d650(std::uint32_t n) override{return e_.errno_from_os_error_05d650(n);}
    void write_error_slot_05d724(std::uint32_t n) override{e_.write_error_slot_05d724(n);}
    void lock_0625f8(std::uint32_t n) override{e_.lock_0625f8(n);}
    void unlock_06264c(std::uint32_t n) override{e_.unlock_06264c(n);}
    std::int32_t invoke_new_handler(std::uintptr_t h,std::uint64_t n) override{return e_.invoke_new_handler(h,n);}
};
struct PreserveError {
    DWORD value=::GetLastError();
    ~PreserveError(){::SetLastError(value);}
    void restore() const{::SetLastError(value);}
    void capture(){value=::GetLastError();}
};
}
struct ApplicationMemory::Impl {
    struct Ticket {void* pointer=nullptr;bool busy=true;};
    DomainStorage& domain;std::shared_ptr<void> lease;
    mutable std::mutex gate;std::list<Ticket> tickets;bool poisoned=false;
    Impl(DomainStorage& d,std::shared_ptr<void> l):domain(d),lease(std::move(l)){}
};
ApplicationMemory::ApplicationMemory(DomainStorage& d,std::shared_ptr<void> lease):impl_(std::make_unique<Impl>(d,std::move(lease))){}
ApplicationMemory::~ApplicationMemory() noexcept{
    const auto& v=*impl_;std::lock_guard lock(v.gate);
    if(v.poisoned||!v.tickets.empty())std::terminate();
}
void* ApplicationMemory::allocate(std::size_t bytes){
    // The retained native bridge admits byte-array extents only to PTRDIFF_MAX.
    // Above the original malloc maximum no native allocation is attempted and
    // the recovered failed-allocation/new-handler path remains representable.
    constexpr auto maximum=0xffffffffffffffe0ULL;
    if(bytes>static_cast<std::uint64_t>(std::numeric_limits<std::ptrdiff_t>::max())&&bytes<=maximum)
        throw std::domain_error("Application allocation extent exceeds rebuilt byte-array domain");
    auto& v=*impl_;auto& d=*v.domain.impl_;PreserveError error;
    std::list<Impl::Ticket>::iterator ticket;
    {
        std::lock_guard lock(v.gate);
        if(v.poisoned)throw std::logic_error("Application memory has an unknown native outcome");
        ticket=v.tickets.emplace(v.tickets.end());
    }
    MemoryEffects effects(d.heap_fx);void* observed_pointer=nullptr;bool captured=false;error.restore();
    try{
        do{
            observed_pointer=crt_realloc_recovered::malloc_base_06a308(bytes,d.heap,effects);
            if(observed_pointer)break;
            if(!crt_heap_recovered::call_new_handler_0634b0(bytes,d.heap,effects)){
                error.capture();captured=true;
                if(bytes==std::numeric_limits<std::size_t>::max())throw std::bad_array_new_length();
                throw std::bad_alloc();
            }
        }while(true);
        error.capture();captured=true;
        {std::lock_guard lock(v.gate);ticket->pointer=observed_pointer;ticket->busy=false;}
        return observed_pointer;
    }catch(...){
        if(!captured)error.capture();std::lock_guard lock(v.gate);
        // A successfully returned allocation must stay owned even if the
        // subsequent ticket-commit lock throws. If this recovery lock also
        // throws, the reserved busy ticket still makes destruction terminal.
        if(effects.pending||observed_pointer){ticket->pointer=observed_pointer;v.poisoned=true;}
        else v.tickets.erase(ticket);
        throw;
    }
}
void ApplicationMemory::deallocate(void* pointer,std::size_t){
    if(!pointer)return;
    auto& v=*impl_;auto& d=*v.domain.impl_;PreserveError error;
    std::list<Impl::Ticket>::iterator ticket;
    {
        std::lock_guard lock(v.gate);
        if(v.poisoned)throw std::logic_error("Application memory has an unknown native outcome");
        ticket=std::find_if(v.tickets.begin(),v.tickets.end(),[pointer](const auto& t){return t.pointer==pointer;});
        if(ticket==v.tickets.end()||ticket->busy)throw std::invalid_argument("Foreign, active or already released application allocation");
        ticket->busy=true;
    }
    MemoryEffects effects(d.heap_fx);bool captured=false;error.restore();
    try{
        crt_heap_recovered::free_0668c0(pointer,d.heap,effects);
        error.capture();captured=true;
        if(!effects.free_succeeded)throw std::system_error(static_cast<int>(effects.free_error),std::system_category(),"Application HeapFree");
        std::lock_guard lock(v.gate);v.tickets.erase(ticket);
    }catch(...){
        if(!captured)error.capture();std::lock_guard lock(v.gate);
        if(effects.pending||effects.free_succeeded)v.poisoned=true;else ticket->busy=false;
        throw;
    }
}
std::size_t ApplicationMemory::outstanding_allocations() const{
    PreserveError error;const auto& v=*impl_;std::lock_guard lock(v.gate);return v.tickets.size();
}
bool ApplicationMemory::tracking_poisoned() const{
    PreserveError error;const auto& v=*impl_;std::lock_guard lock(v.gate);return v.poisoned;
}
ApplicationMemory::AllocationState ApplicationMemory::allocation_state(void* pointer) const{
    PreserveError error;const auto& v=*impl_;std::lock_guard lock(v.gate);
    if(v.poisoned)return AllocationState::Unknown;
    const auto ticket=std::find_if(v.tickets.begin(),v.tickets.end(),[pointer](const auto& t){return t.pointer==pointer;});
    if(ticket==v.tickets.end())return AllocationState::Missing;
    return ticket->busy?AllocationState::Busy:AllocationState::Live;
}
const DomainStorage* ApplicationMemory::domain_identity() const noexcept{return &impl_->domain;}
struct CallbackThreads::Impl {
    DomainStorage& domain;std::shared_ptr<void> lease;mutable std::mutex gate;
    bool active=false,poison=false,fresh=false,detached=false,destroyed=false,freed=false,operating=false;
    bool checking=false,cleanup_started=false;
    DWORD thread=0;void* fiber=nullptr;crt_ptd_recovered::Ptd* ptd=nullptr;
    std::array<std::byte,sizeof(crt_ptd_recovered::Ptd)> initial{};std::uint32_t failure=0;
    Impl(DomainStorage& d,std::shared_ptr<void> l):domain(d),lease(std::move(l)){}
};
CallbackThreads::CallbackThreads(DomainStorage& d,std::shared_ptr<void> lease):impl_(std::make_unique<Impl>(d,std::move(lease))){}
CallbackThreads::~CallbackThreads() noexcept{std::lock_guard lock(impl_->gate);if(impl_->active||impl_->poison)std::abort();}
bool CallbackThreads::active() const{PreserveError error;std::lock_guard lock(impl_->gate);return impl_->active;}
bool CallbackThreads::poisoned() const{PreserveError error;std::lock_guard lock(impl_->gate);return impl_->poison;}
bool CallbackThreads::owned() const{PreserveError error;std::lock_guard lock(impl_->gate);return impl_->active&&!impl_->operating&&impl_->fresh&&!impl_->freed;}
std::uint32_t CallbackThreads::last_failure() const{PreserveError error;std::lock_guard lock(impl_->gate);return impl_->operating?0:impl_->failure;}
bool CallbackThreads::permits_current() const{
    PreserveError error;auto& v=*impl_;auto& d=*v.domain.impl_;
    crt_ptd_recovered::Ptd* expected;
    {
        std::lock_guard lock(v.gate);
        if(!v.active||v.poison||v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber())return false;
        // A provider may catch a nested false result. Keep the violation sticky
        // so the outer probe cannot turn that reentry into successful permission.
        if(v.checking){v.poison=true;return false;}
        if(v.operating||v.cleanup_started||v.detached||v.destroyed||v.freed||!v.ptd)return false;
        v.operating=v.checking=true;expected=v.ptd;
    }
    try{
        const auto slot=d.binding.fls_get_value(d.binding.state().fls_index);
        bool valid=slot==expected&&v.thread==::GetCurrentThreadId()&&v.fiber==::GetCurrentFiber();
        // Never dereference null/sentinel/other slot values, or inspect a borrowed
        // PTD's contents. Owned storage is valid only under the no-raw-free rule.
        if(valid&&v.fresh){
            const auto* bytes=reinterpret_cast<const std::byte*>(expected);
            for(std::size_t i=0;i<v.initial.size();++i)
                if((i<0x20||i>=0x28)&&bytes[i]!=v.initial[i]){valid=false;break;}
        }
        std::lock_guard lock(v.gate);
        if(!valid)v.poison=true;
        v.operating=v.checking=false;
        return valid&&!v.poison&&!v.cleanup_started;
    }catch(...){std::lock_guard lock(v.gate);v.poison=true;v.operating=v.checking=false;throw;}
}
void CallbackThreads::enter(){
    PreserveError error;auto& v=*impl_;auto& d=*v.domain.impl_;
    {
        std::lock_guard lock(v.gate);
        if(v.active||v.poison)throw std::logic_error("Callback PTD scope is active or poisoned");
        v.active=true;v.operating=true;v.thread=::GetCurrentThreadId();v.fiber=::GetCurrentFiber();
        v.fresh=v.detached=v.destroyed=v.freed=v.checking=v.cleanup_started=false;v.ptd=nullptr;v.failure=0;
    }
    bool known_clean=false;
    try{
        auto* previous=d.binding.fls_get_value(d.binding.state().fls_index);
        if(previous==crt_ptd_recovered::construction_sentinel()){known_clean=true;throw std::logic_error("Callback encountered construction sentinel");}
        if(previous){v.ptd=previous;std::lock_guard lock(v.gate);v.operating=false;return;}
        if(!d.binding.fls_set_value(d.binding.state().fls_index,crt_ptd_recovered::construction_sentinel())){
            const auto code=d.binding.last_error();v.failure=code;known_clean=true;
            throw std::system_error(static_cast<int>(code),std::system_category(),"Callback PTD sentinel");
        }
        // Source-only observed raw allocation: do not recurse through shared
        // errno fallback while this scope has only a construction sentinel.
        v.fresh=true;
        auto* storage=d.calls.alloc(d.heap.heap_0a5a10,8,sizeof(crt_ptd_recovered::Ptd));
        if(!storage){
            v.failure=d.calls.last_error();
            if(!d.binding.fls_set_value(d.binding.state().fls_index,nullptr))throw std::runtime_error("Failed to clear callback PTD sentinel");
            known_clean=true;throw std::bad_alloc();
        }
        v.ptd=std::construct_at(static_cast<crt_ptd_recovered::Ptd*>(storage));
        if(!d.binding.fls_set_value(d.binding.state().fls_index,v.ptd))throw std::runtime_error("Callback PTD publication failed; ownership retained");
        d.binding.construct_ptd_array_0648fc(v.ptd);
        std::memcpy(v.initial.data(),v.ptd,v.initial.size());
        std::lock_guard lock(v.gate);v.operating=false;
    }catch(...){
        std::lock_guard lock(v.gate);
        if(known_clean)v.active=false;else v.poison=true;
        v.operating=false;
        throw;
    }
}
bool CallbackThreads::leave(){
    PreserveError error;auto& v=*impl_;auto& d=*v.domain.impl_;
    {
        std::lock_guard lock(v.gate);
        if(!v.active||v.poison||v.operating||v.thread!=::GetCurrentThreadId()||v.fiber!=::GetCurrentFiber())
            throw std::logic_error("Callback PTD leave requires its active thread/fiber and known state");
        v.operating=true;v.cleanup_started=true;
    }
    auto failed=[&](std::uint32_t code){std::lock_guard lock(v.gate);v.failure=code;v.operating=false;return false;};
    try{
        const auto slot=d.binding.fls_get_value(d.binding.state().fls_index);
        if(slot!=(v.detached?crt_ptd_recovered::construction_sentinel():v.ptd))throw std::logic_error("Callback PTD identity changed");
        if(!v.fresh){std::lock_guard lock(v.gate);v.active=false;v.operating=false;v.ptd=nullptr;return true;}
        if(!v.detached){
            const auto* bytes=reinterpret_cast<const std::byte*>(v.ptd);
            for(std::size_t i=0;i<v.initial.size();++i)
                if((i<0x20||i>=0x28)&&bytes[i]!=v.initial[i])throw std::logic_error("Callback mutated PTD outside errno/doserrno domain");
            if(!d.binding.fls_set_value(d.binding.state().fls_index,crt_ptd_recovered::construction_sentinel()))return failed(d.binding.last_error());
            v.detached=true;
        }
        if(!v.destroyed){d.binding.destroy_ptd_array_0649ec(v.ptd);v.destroyed=true;}
        if(!v.freed){
            if(!d.calls.free(d.heap.heap_0a5a10,0,v.ptd))return failed(d.calls.last_error());
            v.freed=true;v.ptd=nullptr;
        }
        if(!d.binding.fls_set_value(d.binding.state().fls_index,nullptr))return failed(d.binding.last_error());
        std::lock_guard lock(v.gate);v.active=false;v.operating=false;v.failure=0;return true;
    }catch(...){std::lock_guard lock(v.gate);v.poison=true;v.operating=false;throw;}
}
std::shared_ptr<DomainStorage> DomainStorage::create(Seed seed,Apis api){return std::shared_ptr<DomainStorage>(new DomainStorage(std::move(seed),api));}
DomainStorage::DomainStorage(Seed seed,Apis api):impl_(std::make_unique<Impl>(std::move(seed),api)){}
DomainStorage::~DomainStorage() noexcept{
    const auto& v=*impl_;
    if(v.busy||v.leases||v.initialized_locks||v.binding.native_index()!=crt_ptd_recovered::invalid_fls_index||
        (v.phase!=Phase::Constructed&&v.phase!=Phase::Closed))std::terminate();
}
bool DomainStorage::initialize(){
    auto& v=*impl_;Impl::Control control(v);
    if(v.phase!=Phase::Constructed)throw std::logic_error("Domain initialization requires a clean constructed phase");
    if(!crt_heap_recovered::initialize_heap_0704cc(v.heap,v.heap_fx))return false;
    try{
        for(;v.initialized_locks<v.lock_ids.size();++v.initialized_locks){
            if(v.banks.initialize_and_spin({crt_locks_recovered::Bank::acrt,v.lock_ids[v.initialized_locks]},0x400)==0){
                v.destroy_locks();return false;
            }
        }
    }catch(...){v.destroy_locks();throw;}
    try{
        if(crt_ptd_recovered::initialize_064e90(v.binding.state(),v.binding)){v.phase=Phase::Ready;return true;}
    }catch(...){v.phase=Phase::Poisoned;throw;}
    // Normal initializer failure already invoked source uninitialize. Retain
    // native slot/locks if that release failed, rather than losing its owner.
    if(v.binding.native_index()!=crt_ptd_recovered::invalid_fls_index){v.phase=Phase::ReleasePending;return false;}
    v.destroy_locks();return false;
}
std::unique_ptr<bootstrap_providers::NativeThreads> DomainStorage::make_threads(thread_native::Api api){
    auto& v=*impl_;Impl::Control control(v);
    if(v.phase!=Phase::Ready)throw std::logic_error("Thread providers require a ready domain");
    auto lease=std::make_shared<ProviderLease>(shared_from_this());
    return bootstrap_providers::make_recovered_threads({lease,v.runtime_state,v.runtime,v.errors,v.error_ptd,
        {v.binding.state(),v.binding,v.heap,v.heap_fx,v.invalid}},api);
}
std::shared_ptr<ApplicationMemory> DomainStorage::make_memory(){
    auto& v=*impl_;Impl::Control control(v);
    if(v.phase!=Phase::Ready)throw std::logic_error("Application memory requires a ready domain");
    auto lease=std::make_shared<ProviderLease>(shared_from_this());
    return std::shared_ptr<ApplicationMemory>(new ApplicationMemory(*this,std::move(lease)));
}
std::shared_ptr<CallbackThreads> DomainStorage::make_callback_threads(){
    auto& v=*impl_;Impl::Control control(v);
    if(v.phase!=Phase::Ready)throw std::logic_error("Callback PTD provider requires a ready domain");
    auto lease=std::make_shared<ProviderLease>(shared_from_this());
    return std::shared_ptr<CallbackThreads>(new CallbackThreads(*this,std::move(lease)));
}
bool DomainStorage::close(){
    auto& v=*impl_;Impl::Control control(v);
    if(v.phase==Phase::Closed)return true;
    if(v.phase==Phase::Poisoned)throw std::logic_error("Unknown published PTD constructor outcome requires retaining the poisoned domain");
    v.phase=Phase::Stopping;
    if(v.leases)return false;
    if(!v.release_fls())return false;
    if(!v.runtime.close())return false;
    v.destroy_locks();v.phase=Phase::Closed;return true;
}
Phase DomainStorage::phase() const noexcept{return impl_->phase;}
std::size_t DomainStorage::provider_leases() const noexcept{return impl_->leases.load();}
std::uint32_t DomainStorage::native_fls_index() const noexcept{return impl_->binding.native_index();}
std::uint32_t DomainStorage::source_fls_index() const noexcept{return impl_->binding.state().fls_index;}
crt_ptd_recovered::Ptd* DomainStorage::current_ptd(){auto& v=*impl_;Impl::Control control(v);if(v.phase!=Phase::Ready)throw std::logic_error("PTD observation requires ready domain");return crt_ptd_recovered::required_preserve_error_064b90(v.binding.state(),v.binding);}
crt_errors_recovered::Cell32 DomainStorage::errno_cell(){auto& v=*impl_;Impl::Control control(v);if(v.phase!=Phase::Ready)throw std::logic_error("Error cell requires ready domain");return crt_errors_recovered::errno_cell_05d724(v.errors,v.error_ptd);}
const InitialLocale& DomainStorage::locale() const noexcept{return impl_->locale;}
std::uint32_t DomainStorage::locale_references() const{return impl_->locale.image().counter_value(impl_->locale.ids().initial_locale+0x10);}
std::uint32_t DomainStorage::multibyte_references() const{return impl_->locale.image().counter_value(impl_->locale.ids().multibyte_info);}
std::size_t DomainStorage::runtime_modules() const{return impl_->runtime.outstanding_modules();}
} // namespace anadius::thread_domain
