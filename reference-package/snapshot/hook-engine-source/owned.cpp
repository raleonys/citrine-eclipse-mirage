#include "owned.hpp"
#include <cstdlib>
#include <mutex>
#include <stdexcept>

namespace recovered_hooks {
namespace {
struct Error {DWORD value=::GetLastError();~Error(){::SetLastError(value);}void restore()const{::SetLastError(value);}void capture(){value=::GetLastError();}};
struct Owners {std::shared_ptr<void> code;std::unique_ptr<Api> lower;};
}
struct OwnedEngine::Impl {
    // Destroy lower Api while its code/backing is still retained.
    std::shared_ptr<void> code;std::unique_ptr<Api> lower;
    mutable std::mutex gate;const DWORD controller=::GetCurrentThreadId();
    OwnedPhase phase=OwnedPhase::Fresh;bool active=false,has_result=false,original_callers=false;
    Operation operation=Operation::Initialize;int status=0;
    explicit Impl(Owners&& owners):code(std::move(owners.code)),lower(std::move(owners.lower)){}
};
OwnedEngine::OwnedEngine(std::unique_ptr<Api> lower,std::shared_ptr<void> code){
    Error error;Owners owners{std::move(code),std::move(lower)};
    if(!owners.lower||!owners.code)throw std::invalid_argument("Owned lower engine and actual code backing required");
    impl_=std::make_unique<Impl>(std::move(owners));
}
OwnedEngine::~OwnedEngine() noexcept {
    Error error;auto& v=*impl_;bool unresolved;
    {std::lock_guard lock(v.gate);unresolved=v.active||v.phase==OwnedPhase::NeedsCleanup||v.phase==OwnedPhase::Poisoned;if(!unresolved){v.active=true;v.phase=OwnedPhase::Retiring;}}
    if(unresolved)std::abort();
    // Keep metadata/gate alive during opaque deleters; reentry is rejected.
    v.lower.reset();v.code.reset();
}
EngineSnapshot OwnedEngine::snapshot()const{Error error;const auto& v=*impl_;std::lock_guard lock(v.gate);return {v.phase,v.active,v.has_result,v.operation,v.status};}
bool OwnedEngine::empty()const{const auto s=snapshot();return !s.active&&(s.phase==OwnedPhase::Fresh||s.phase==OwnedPhase::Closed);}
void OwnedEngine::select_original_callers(){
    Error error;auto& v=*impl_;if(::GetCurrentThreadId()!=v.controller)throw std::logic_error("Original engine profile selects on its creator");
    std::lock_guard lock(v.gate);if(v.phase!=OwnedPhase::Fresh||v.active||v.original_callers)throw std::logic_error("Original engine callers require a fresh unselected engine");v.original_callers=true;
}
template<class F>int OwnedEngine::run(Operation op,F&& lower){
    Error error;auto& v=*impl_;
    {std::lock_guard lock(v.gate);
     if(!v.original_callers&&::GetCurrentThreadId()!=v.controller)throw std::logic_error("Hook engine controller required");
     if(v.phase==OwnedPhase::Retiring)throw std::logic_error("Retiring hook engine cannot be called");
     if(v.active){v.phase=OwnedPhase::Poisoned;throw std::logic_error("Hook engine reentry");}
     if(v.phase==OwnedPhase::Poisoned)throw std::logic_error("Unknown hook engine outcome requires retention");
     if(v.phase==OwnedPhase::Closed&&op!=Operation::Uninitialize)throw std::logic_error("Closed hook engine cannot restart");
     v.active=true;v.has_result=false;v.operation=op;v.phase=OwnedPhase::NeedsCleanup;}
    bool returned=false;
    try{
        error.restore();const int result=lower();error.capture();returned=true;
        {std::lock_guard lock(v.gate);v.active=false;
         if(v.phase==OwnedPhase::Poisoned)throw std::logic_error("Lower engine swallowed reentry");
         v.status=result;v.has_result=true;
         if(op==Operation::Uninitialize&&(result==MH_OK||result==MH_ERROR_NOT_INITIALIZED))v.phase=OwnedPhase::Closed;}
        return result;
    }catch(...){if(!returned)error.capture();std::lock_guard lock(v.gate);v.active=false;v.phase=OwnedPhase::Poisoned;throw;}
}
int OwnedEngine::initialize(){return run(Operation::Initialize,[&]{return impl_->lower->initialize();});}
int OwnedEngine::create(void* t,void* d,void** o){return run(Operation::Create,[&]{return impl_->lower->create(t,d,o);});}
int OwnedEngine::enable(void* t){return run(Operation::Enable,[&]{return impl_->lower->enable(t);});}
int OwnedEngine::disable(void* t){return run(Operation::Disable,[&]{return impl_->lower->disable(t);});}
int OwnedEngine::uninitialize(){return run(Operation::Uninitialize,[&]{return impl_->lower->uninitialize();});}
std::shared_ptr<OwnedEngine> make_owned_engine(std::unique_ptr<Api>& lower,std::shared_ptr<void>& code){
    Error error;if(!lower||!code)throw std::invalid_argument("Owned lower engine and actual code backing required");
    Owners owners{std::move(code),std::move(lower)};
    // Allocation is sequenced before constructor arguments in this new-expression;
    // local ordered ownership therefore also covers facade allocation failure.
    return std::shared_ptr<OwnedEngine>(new OwnedEngine(std::move(owners.lower),std::move(owners.code)));
}
}
