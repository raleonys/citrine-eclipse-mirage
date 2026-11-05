#include "originals.hpp"
#include <atomic>
#include <cstdlib>
#include <mutex>
#include <stdexcept>

namespace anadius::original_provider {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};
enum class Phase {Created,Setup,Ready,Stopped};
class Bound final:public native_recovered::OriginalProvider {
    std::shared_ptr<void> code_;
    const native_recovered::OriginalSlots& slots_;
    native_recovered::OriginalSlots sealed_{};
    const DWORD controller_=::GetCurrentThreadId();
    std::mutex gate_;Phase phase_=Phase::Created;
public:
    Bound(const native_recovered::OriginalSlots& slots,std::shared_ptr<void> code):code_(std::move(code)),slots_(slots){
        if(!code_)throw std::invalid_argument("Explicit original code owner required");
    }
    const native_recovered::OriginalSlots* slots_identity()const noexcept override{return &slots_;}
    std::uintptr_t load(native_recovered::Callback callback)override{
        Error error;std::lock_guard lock(gate_);
        const auto index=host_init_recovered::slot(callback);
        if(index>=slots_.size())throw std::out_of_range("Invalid original callback slot");
        if(phase_!=Phase::Ready&&(phase_!=Phase::Setup||::GetCurrentThreadId()!=controller_))throw std::logic_error("Original provider is not published for this caller");
        const auto address=slots_[index];
        if(phase_==Phase::Ready&&address!=sealed_[index]){phase_=Phase::Stopped;throw std::logic_error("Published original callback slot changed");}
        if(!address)throw std::logic_error("Original callback slot is empty");
        return address;
    }
    void setup_begin()override{
        Error error;std::lock_guard lock(gate_);
        if(::GetCurrentThreadId()!=controller_||phase_!=Phase::Created)throw std::logic_error("Original setup requires unused controller owner");
        phase_=Phase::Setup;
    }
    void setup_complete()override{
        Error error;std::lock_guard lock(gate_);
        if(::GetCurrentThreadId()!=controller_||phase_!=Phase::Setup)throw std::logic_error("Original publication requires controller setup");
        sealed_=slots_;phase_=Phase::Ready;
    }
    void stop()noexcept override{Error error;std::lock_guard lock(gate_);phase_=Phase::Stopped;}
};
}
std::shared_ptr<native_recovered::OriginalProvider> bind(const native_recovered::OriginalSlots& slots,std::shared_ptr<void> code){return std::make_shared<Bound>(slots,std::move(code));}
Factory make_factory(std::shared_ptr<void> code){
    if(!code)throw std::invalid_argument("Explicit original code owner required");
    return [code=std::move(code)](const native_recovered::OriginalSlots& slots){return bind(slots,code);};
}
struct Publication::Impl {
    std::shared_ptr<void> code;
    std::atomic<const native_recovered::OriginalSlots*> slots{nullptr};
    native_recovered::OriginalSlots sealed{},targets{};
    mutable std::mutex gate;PublicationPhase phase=PublicationPhase::Unbound;
    const DWORD controller=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    std::uint16_t mask=0;bool creating=false;
    std::size_t pending_index=0;std::uintptr_t pending_target=0;std::uintptr_t* pending_output=nullptr;
    explicit Impl(std::shared_ptr<void> c):code(std::move(c)){}
    void control()const{if(controller!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Original publication controller/fiber required");}
    static std::size_t index(native_recovered::Callback callback){
        const auto i=host_init_recovered::slot(callback);
        if(i>=10)throw std::out_of_range("Invalid original publication slot");return i;
    }
    [[noreturn]]void poison(const char* message){phase=PublicationPhase::Poisoned;throw std::logic_error(message);}
};
Publication::Publication(std::shared_ptr<void> code){
    if(!code)throw std::invalid_argument("Actual original code owner required");
    impl_=std::make_unique<Impl>(std::move(code));
}
Publication::~Publication()noexcept{
    Error error;auto& v=*impl_;
    {std::lock_guard lock(v.gate);
        if(v.creating||v.phase==PublicationPhase::Poisoned||(v.mask&&v.phase!=PublicationPhase::Stopped))std::abort();
        v.phase=PublicationPhase::Retiring;
    }
    v.code.reset(); // facade/gate remain attached through opaque backing deletion
}
void Publication::bind_slots(const native_recovered::OriginalSlots& slots){
    Error error;auto& v=*impl_;v.control();std::lock_guard lock(v.gate);
    if(v.phase!=PublicationPhase::Unbound)throw std::logic_error("Original publication binds once");
    v.slots.store(&slots,std::memory_order_release);v.phase=PublicationPhase::Created;
}
const native_recovered::OriginalSlots* Publication::slots_identity()const noexcept{return impl_->slots.load(std::memory_order_acquire);}
PublicationSnapshot Publication::snapshot()const{Error error;auto& v=*impl_;std::lock_guard lock(v.gate);return {v.phase,v.mask,v.creating};}
void Publication::setup_begin(){
    Error error;auto& v=*impl_;v.control();std::lock_guard lock(v.gate);
    if(v.phase!=PublicationPhase::Created)throw std::logic_error("Original publication requires unused bound slots");
    v.phase=PublicationPhase::Setup;
}
void Publication::before_create(native_recovered::Callback callback,std::uintptr_t target,std::uintptr_t& output){
    Error error;auto& v=*impl_;v.control();const auto i=Impl::index(callback);std::lock_guard lock(v.gate);
    if(v.phase!=PublicationPhase::Setup)throw std::logic_error("Original create publication is outside setup");
    if(v.creating)v.poison("Reentrant original create publication");
    const auto* slots=v.slots.load(std::memory_order_acquire);
    if(!target||!slots||&slots->at(i)!=&output)throw std::invalid_argument("Create output must be the exact bound original cell");
    if(v.mask&(1u<<i))throw std::logic_error("Published original cell cannot be rewritten");
    v.creating=true;v.pending_index=i;v.pending_target=target;v.pending_output=&output;
}
void Publication::after_create(native_recovered::Callback callback,std::uintptr_t target,std::uintptr_t& output,int status){
    Error error;auto& v=*impl_;v.control();const auto i=Impl::index(callback);std::lock_guard lock(v.gate);
    if(v.phase!=PublicationPhase::Setup||!v.creating||v.pending_index!=i||v.pending_target!=target||v.pending_output!=&output)
        v.poison("Original create receipt does not match reservation");
    if(status==0){
        if(!output)v.poison("Successful create returned no original pointer");
        v.sealed[i]=output;v.targets[i]=target;v.mask=static_cast<std::uint16_t>(v.mask|(1u<<i));
    }
    // Failed normal create publishes nothing, even if it changed a stale cell.
    v.creating=false;v.pending_output=nullptr;v.pending_target=0;
}
void Publication::before_enable(std::uintptr_t target){
    Error error;auto& v=*impl_;v.control();std::lock_guard lock(v.gate);
    if(v.phase!=PublicationPhase::Setup&&v.phase!=PublicationPhase::Ready)throw std::logic_error("Original enable publication unavailable");
    if(v.creating)v.poison("Enable reentered an unfinished original create");
    for(std::size_t i=0;i<v.targets.size();++i)if((v.mask&(1u<<i))&&v.targets[i]==target)return;
    throw std::logic_error("Enable requires a successfully published original for this target");
}
std::uintptr_t Publication::load(native_recovered::Callback callback){
    Error error;auto& v=*impl_;const auto i=Impl::index(callback);std::lock_guard lock(v.gate);
    if((v.phase!=PublicationPhase::Setup&&v.phase!=PublicationPhase::Ready)||!(v.mask&(1u<<i)))
        throw std::logic_error("Original slot has no live successful-create publication");
    // During setup read only our sealed copy. Other raw slot cells may be
    // written by the controller; published cells cannot enter create again.
    if(v.phase==PublicationPhase::Ready&&v.slots.load(std::memory_order_acquire)->at(i)!=v.sealed[i])
        v.poison("Published original slot changed after setup");
    return v.sealed[i];
}
void Publication::setup_complete(){
    Error error;auto& v=*impl_;v.control();std::lock_guard lock(v.gate);
    if(v.phase!=PublicationPhase::Setup)throw std::logic_error("Original publication cannot reseal");
    if(v.creating)v.poison("Original setup completed with a pending create");
    const auto* slots=v.slots.load(std::memory_order_acquire);
    for(std::size_t i=0;i<v.sealed.size();++i)if((v.mask&(1u<<i))&&slots->at(i)!=v.sealed[i])v.poison("Original slot changed before setup completed");
    v.phase=PublicationPhase::Ready;
}
void Publication::unknown()noexcept{Error error;auto& v=*impl_;std::lock_guard lock(v.gate);if(v.phase!=PublicationPhase::Retiring)v.phase=PublicationPhase::Poisoned;}
void Publication::stop()noexcept{
    Error error;auto& v=*impl_;std::lock_guard lock(v.gate);
    if(v.phase==PublicationPhase::Poisoned||v.creating){v.phase=PublicationPhase::Poisoned;return;}
    if(v.phase!=PublicationPhase::Retiring)v.phase=PublicationPhase::Stopped;
}
Factory make_publishing_factory(std::shared_ptr<Publication> publication){
    if(!publication)throw std::invalid_argument("Shared original publication required");
    return [publication=std::move(publication)](const native_recovered::OriginalSlots& slots)->std::shared_ptr<native_recovered::OriginalProvider>{
        publication->bind_slots(slots);return publication;
    };
}
} // namespace anadius::original_provider
