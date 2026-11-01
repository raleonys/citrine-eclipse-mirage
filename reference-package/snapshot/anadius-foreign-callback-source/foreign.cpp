#include "foreign.hpp"
#include "../anadius-callback-identity-source/identity.hpp"
#include "../anadius-callback-provider-source/provider.hpp"
#include <cstdlib>
#include <type_traits>

namespace anadius::foreign_callback {
namespace {struct Error {DWORD saved=::GetLastError();~Error(){::SetLastError(saved);}};}
Rejected::Rejected(Fault value):std::logic_error(value==Fault::Warming?"Foreign callback publication is not ready":
    value==Fault::Overlap?"Overlapping callback is outside the single-active domain":value==Fault::Reentry?"Callback reentry is unsupported":"Callback domain is closing"),fault(value){}
Domain::Domain(std::shared_ptr<thread_domain::DomainStorage> domain,std::function<bool()> proof)
    :domain_(std::move(domain)),ready_proof_(std::move(proof)),controller_(::GetCurrentThreadId()){
    if(!domain_||!ready_proof_)throw std::invalid_argument("Owned PTD domain and explicit shared-state publication proof required");
    threads_=domain_->make_callback_threads();
}
Domain::~Domain() noexcept{std::lock_guard lock(gate_);if(active_||poison_||control_)std::abort();}
const thread_domain::DomainStorage* Domain::memory_domain() const noexcept{return domain_.get();}
bool Domain::active() const{Error error;std::lock_guard lock(gate_);return active_;}
bool Domain::ready() const{Error error;std::lock_guard lock(gate_);return ready_&&!closing_&&!poison_;}
bool Domain::permits_current() const{
    Error error;
    {std::lock_guard lock(gate_);if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber())return false;}
    bool permitted,poison;
    try{permitted=threads_->permits_current();poison=threads_->poisoned();}
    catch(...){std::lock_guard lock(gate_);poison_=true;ready_=false;throw;}
    std::lock_guard lock(gate_);
    if(poison){poison_=true;ready_=false;}
    return permitted&&active_&&!poison_&&thread_==::GetCurrentThreadId()&&fiber_==::GetCurrentFiber();
}
void Domain::begin(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Host setup controller required");
    std::lock_guard lock(gate_);if(active_||closing_||control_||poison_)throw std::logic_error("Host setup cannot mutate an active/closing callback domain");
    setup_done_=false;ready_=false;
}
void Domain::complete(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Host setup controller required");
    {std::lock_guard lock(gate_);if(active_||closing_||poison_||control_)throw std::logic_error("Invalid completed host setup");setup_done_=true;}
    (void)activate_foreign();
}
bool Domain::activate_foreign(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Foreign activation controller required");
    {
        std::lock_guard lock(gate_);if(control_||active_||closing_||poison_)throw std::logic_error("Foreign activation is not quiescent");
        if(!setup_done_)return false;ready_=false;control_=true;
    }
    bool proof;
    try{proof=ready_proof_();}catch(...){std::lock_guard lock(gate_);control_=false;throw;}
    std::lock_guard lock(gate_);control_=false;
    if(active_||closing_||poison_)throw std::logic_error("Foreign activation lost quiescence");
    ready_=proof;return proof;
}
bool Domain::pause(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Callback pause controller required");
    std::lock_guard lock(gate_);if(control_||poison_)throw std::logic_error("Cannot pause unknown callback outcome");
    closing_=true;ready_=false;return !active_;
}
void Domain::enter(native_recovered::Callback callback){
    Error error;
    {
        std::lock_guard lock(gate_);
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(control_)throw Rejected(::GetCurrentThreadId()==controller_?Fault::Reentry:Fault::Warming);
        if(active_)throw Rejected(thread_==::GetCurrentThreadId()?Fault::Reentry:Fault::Overlap);
        if(!ready_&&::GetCurrentThreadId()!=controller_)throw Rejected(Fault::Warming);
        active_=true;thread_=::GetCurrentThreadId();fiber_=::GetCurrentFiber();callback_=callback;
    }
    try{threads_->enter();if(!permits_current())throw std::logic_error("Initial callback PTD permission failed");}
    catch(...){std::lock_guard lock(gate_);poison_=threads_->poisoned()||threads_->active();if(!poison_)active_=false;throw;}
}
void Domain::leave(native_recovered::Callback callback) noexcept{
    Error error;
    {std::lock_guard lock(gate_);if(!active_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=callback)std::abort();}
    try{if(!threads_->leave())std::abort();}catch(...){std::abort();}
    std::lock_guard lock(gate_);active_=false;
}
Admission::Admission(std::shared_ptr<Domain> domain):domain_(std::move(domain)){if(!domain_)throw std::invalid_argument("Foreign callback domain required");}
void Admission::enter(native_recovered::Callback value){domain_->enter(value);}
void Admission::leave(native_recovered::Callback value)noexcept{domain_->leave(value);}
void Admission::host_setup_begin(){domain_->begin();}
void Admission::host_setup_complete(){domain_->complete();}
bool Admission::pause_before_cleanup(){return domain_->pause();}
bootstrap_backend::CallbackAttachmentFactory make_factory(std::shared_ptr<Domain> domain,
    native_context::CallbackSeed seed,callback_policy::Resources resources,native_recovered::Errors& errors,native_context::Quiescence proof){
    if(!domain||!resources.memory||resources.memory->domain_identity()!=domain->memory_domain())throw std::invalid_argument("Callback memory must use exact PTD domain");
    if(resources.callback_scope)throw std::invalid_argument("Existing callback permission cannot be silently replaced");
    resources.callback_scope=[domain]{return domain->permits_current();};
    resources.encoding=callback_identity::check_encoding(std::move(resources.encoding),resources.callback_scope);
    resources.owned_crt=callback_provider::check_crt(resources.crt,std::move(resources.owned_crt),resources.callback_scope);
    return native_context::make_factory(std::move(seed),callback_policy::make_policy_factory(std::move(resources)),errors,std::move(proof),std::make_unique<Admission>(domain));
}
static_assert(!std::is_abstract_v<Admission>);
} // namespace anadius::foreign_callback
