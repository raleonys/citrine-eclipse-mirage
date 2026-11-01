#include "originals.hpp"
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
} // namespace anadius::original_provider
