#include "shutdown.hpp"
#include <cstdlib>
#include <stdexcept>

namespace anadius::pre_unload {
namespace {struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}};}
Controller::Controller() noexcept:controller_(::GetCurrentThreadId()),fiber_(::GetCurrentFiber()){}
std::shared_ptr<Controller> Controller::take(std::unique_ptr<bootstrap_runtime::Runtime>& runtime,
    std::shared_ptr<thread_domain::DomainStorage>& domain){
    Error error;
    if(!runtime||!domain||domain->phase()!=thread_domain::Phase::Ready||runtime->memory_domain()!=domain.get())
        throw std::invalid_argument("Owned Runtime and exact certified Ready thread Domain required");
    auto owner=std::shared_ptr<Controller>(new Controller);
    owner->domain_=std::move(domain);owner->runtime_=std::move(runtime);
    owner->phase_.store(Phase::Managed,std::memory_order_release);return owner;
}
Controller::~Controller() noexcept {
    Error error;const auto phase=phase_.load();
    if(busy_.load()||(phase!=Phase::Empty&&phase!=Phase::Closed)||runtime_)std::abort();
    phase_.store(Phase::Retiring);domain_.reset();
}
Snapshot Controller::snapshot()const noexcept{return {phase_.load(std::memory_order_acquire),error_.load(),busy_.load()};}
bool Controller::closed()const noexcept{return phase_.load(std::memory_order_acquire)==Phase::Closed;}
bool Controller::poisoned()const noexcept{return phase_.load()==Phase::Poisoned;}
BOOL Controller::result(DWORD error)noexcept{error_.store(error);::SetLastError(error);return FALSE;}
void Controller::start_application(HINSTANCE module,LPVOID reserved){
    Error error;
    if(!module)throw std::invalid_argument("Application attach requires its image module");
    if(::GetCurrentThreadId()!=controller_||::GetCurrentFiber()!=fiber_)
        throw std::logic_error("Application attach requires the original controller/fiber");
    if(busy_.exchange(true)){phase_.store(Phase::Poisoned);throw std::logic_error("Application attach lifecycle reentry");}
    struct Active {std::atomic<bool>& busy;~Active(){busy.store(false);}} active{busy_};
    if(phase_.load()!=Phase::Managed||!runtime_||runtime_->phase()!=bootstrap_owner::Phase::Constructed)
        throw std::logic_error("Application attach requires owned unpublished Runtime roots");
    try{
        phase_.store(Phase::PublishingApplication,std::memory_order_release);
        runtime_->publish();error.value=::GetLastError();
        if(poisoned())throw std::logic_error("Application publication was poisoned");
        phase_.store(Phase::StartingApplication,std::memory_order_release);
        const auto entry=runtime_->application_entry();
        ::SetLastError(error.value);
        const BOOL attached=entry(module,DLL_PROCESS_ATTACH,reserved);
        error.value=::GetLastError();
        if(poisoned())throw std::logic_error("Application attach was poisoned");
        if(!attached)throw std::runtime_error("Application attach failed without proven rollback; roots retained");
        phase_.store(Phase::ApplicationAttached,std::memory_order_release);error_.store(0);
    }catch(...){error.value=::GetLastError();error_.store(ERROR_UNHANDLED_EXCEPTION);phase_.store(Phase::Poisoned,std::memory_order_release);throw;}
}
BOOL Controller::shutdown()noexcept{
    const DWORD incoming=::GetLastError();
    if(::GetCurrentThreadId()!=controller_)return result(ERROR_INVALID_THREAD_ID);
    if(::GetCurrentFiber()!=fiber_)return result(ERROR_INVALID_STATE);
    if(phase_.load()==Phase::Retiring)return result(ERROR_INVALID_STATE);
    if(busy_.exchange(true)){phase_.store(Phase::Poisoned);return result(ERROR_BUSY);}
    struct Active {std::atomic<bool>& busy;~Active(){busy.store(false);}} active{busy_};
    if(poisoned())return result(ERROR_INVALID_STATE);
    if(closed()){error_.store(0);::SetLastError(incoming);return TRUE;}
    try{
        if(runtime_){
            phase_.store(Phase::ClosingRuntime);
            const bool stopped=runtime_->close();
            if(poisoned())return result(ERROR_INVALID_STATE);
            if(!stopped)return result(ERROR_IO_PENDING);
            phase_.store(Phase::RetiringRuntime);runtime_.reset();
            if(poisoned())return result(ERROR_INVALID_STATE);
        }
        phase_.store(Phase::ClosingDomain);
        const bool stopped=domain_->close();
        if(poisoned())return result(ERROR_INVALID_STATE);
        if(!stopped)return result(ERROR_IO_PENDING);
        phase_.store(Phase::Closed,std::memory_order_release);error_.store(0);::SetLastError(incoming);return TRUE;
    }catch(...){phase_.store(Phase::Poisoned);return result(ERROR_UNHANDLED_EXCEPTION);}
}
BOOL WINAPI Controller::invoke(void* context)noexcept{
    if(!context){::SetLastError(ERROR_INVALID_PARAMETER);return FALSE;}
    // Valid live Controller context is the caller's ABI/lifetime contract.
    return static_cast<Controller*>(context)->shutdown();
}
Entry Controller::entry()noexcept{return &invoke;}
static_assert(std::atomic<Phase>::is_always_lock_free&&std::atomic<DWORD>::is_always_lock_free&&std::atomic<bool>::is_always_lock_free);
}
