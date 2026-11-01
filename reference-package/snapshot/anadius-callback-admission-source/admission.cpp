#include "admission.hpp"
#include <cstdlib>
#include <type_traits>

namespace anadius::callback_admission {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
}
Rejected::Rejected(Fault value):std::logic_error(value==Fault::ForeignThread?"Foreign callback thread is outside the selected PTD domain":
    value==Fault::Reentry?"Native callback reentry is outside the selected controller domain":"Native callback admission is stopped"),fault(value){}
Controller::Controller():controller_(::GetCurrentThreadId()){}
Controller::~Controller() noexcept{std::lock_guard lock(gate_);if(active_)std::abort();}
void Controller::enter(native_recovered::Callback value){
    PreserveError error;
    if(::GetCurrentThreadId()!=controller_)throw Rejected(Fault::ForeignThread);
    const std::lock_guard lock(gate_);
    if(stopped_)throw Rejected(Fault::Stopped);
    if(active_)throw Rejected(Fault::Reentry);
    callback_=value;active_=true;
}
void Controller::leave(native_recovered::Callback value) noexcept{
    PreserveError error;
    if(::GetCurrentThreadId()!=controller_)std::abort();
    const std::lock_guard lock(gate_);
    if(!active_||callback_!=value)std::abort();
    active_=false;
}
bool Controller::stop(){
    PreserveError error;
    if(::GetCurrentThreadId()!=controller_)throw Rejected(Fault::ForeignThread);
    const std::lock_guard lock(gate_);
    if(active_)return false;
    stopped_=true;return true;
}
bool Controller::stopped() const{PreserveError error;const std::lock_guard lock(gate_);return stopped_;}
bool Controller::active() const{PreserveError error;const std::lock_guard lock(gate_);return active_;}
bootstrap_backend::CallbackAttachmentFactory make_factory(native_context::CallbackSeed seed,
    native_context::PolicyFactory policies,native_recovered::Errors& errors,native_context::Quiescence proof){
    if(!policies||!proof)throw std::invalid_argument("Explicit callback policy and external quiescence proof required");
    auto admission=std::make_unique<Controller>();auto* observer=admission.get();
    auto checked=[observer,proof=std::move(proof)]() mutable {
        if(!proof())return false; // no gate mutation on failed/throwing proof
        return observer->stop();
    };
    return native_context::make_factory(std::move(seed),std::move(policies),errors,std::move(checked),std::move(admission));
}
static_assert(!std::is_abstract_v<Controller>&&!std::is_move_constructible_v<Controller>);
} // namespace anadius::callback_admission
