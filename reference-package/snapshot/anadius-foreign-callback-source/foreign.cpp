#include "foreign.hpp"
#include "../anadius-callback-identity-source/identity.hpp"
#include "../anadius-callback-provider-source/provider.hpp"
#include "../anadius-shared-log-source/stream.hpp"
#include <cstdlib>
#include <type_traits>

namespace anadius::foreign_callback {
namespace {
struct Error {DWORD saved=::GetLastError();~Error(){::SetLastError(saved);}};
// Trivial metadata only: no private CRT/PTD allocation or destructor. The
// native Context lease retains all actual code/providers across the SDK call.
enum class OriginalConnectPhase {Checking,Waiting,Redirect,Original};
struct OriginalConnectLane {
    const Domain* owner=nullptr;void* fiber=nullptr;bool admitted=false;
    OriginalConnectPhase phase=OriginalConnectPhase::Checking;
    std::uint16_t requested=0;
};
thread_local OriginalConnectLane original_connect_lane;
thread_local native_recovered::OriginalForwardFrame* original_forward_top=nullptr;
thread_local native_recovered::Callback original_forward_callback{};
thread_local native_recovered::RegistrySdkFrame* registry_sdk_top=nullptr;
thread_local native_recovered::RegistryNestedFrame* registry_nested_top=nullptr;
bool registry_callback(native_recovered::Callback callback) noexcept {
    using C=native_recovered::Callback;
    switch(callback){
    case C::reg_open_a_00f800:case C::reg_open_w_00f960:
    case C::reg_query_a_00fac0:case C::reg_query_w_00fb60:
    case C::reg_close_00f5b0:return true;
    default:return false;
    }
}
void clear_registry_sdk(native_recovered::RegistrySdkFrame& frame) noexcept {
    frame.owner=nullptr;frame.parent=nullptr;frame.body=nullptr;
    frame.thread=0;frame.fiber=nullptr;frame.callback={};frame.admitted=false;
}
void clear_registry_nested(native_recovered::RegistryNestedFrame& frame) noexcept {
    frame.owner=nullptr;frame.parent=nullptr;frame.sdk=nullptr;
    frame.thread=0;frame.fiber=nullptr;frame.callback={};frame.parent_callback={};frame.admitted=false;
}
bool forward_callback(native_recovered::Callback callback) noexcept {
    using C=native_recovered::Callback;
    switch(callback){
    case C::reg_open_a_00f800:case C::reg_open_w_00f960:
    case C::create_process_a_00f3f0:case C::create_process_w_00f4d0:
    case C::folder_a_00fc00:case C::folder_w_00fc80:return true;
    default:return false;
    }
}
void clear_forward_frame(native_recovered::OriginalForwardFrame& frame) noexcept {
    frame.owner=nullptr;frame.parent=nullptr;frame.thread=0;frame.fiber=nullptr;
    frame.callback={};frame.admitted=false;
}
void check_unsuspended_connect(){
    if(original_forward_top)throw std::logic_error("Pure original frame suspends outer connect access");
}
}
Rejected::Rejected(Fault value):std::logic_error(value==Fault::Warming?"Foreign callback publication is not ready":
    value==Fault::Overlap?"Overlapping callback is outside the single-active domain":value==Fault::Reentry?"Callback reentry is unsupported":"Callback domain is closing"),fault(value){}
Domain::Domain(std::shared_ptr<thread_domain::DomainStorage> domain,std::function<bool()> proof,std::function<bool()> original,std::function<bool(std::uint16_t)> web,std::function<bool(std::uint16_t)> other,std::function<std::optional<std::uint16_t>()> redirect,HookPublicationProof hook)
    :domain_(std::move(domain)),ready_proof_(std::move(proof)),connect_original_proof_(std::move(original)),web_no_redirect_proof_(std::move(web)),other_no_redirect_proof_(std::move(other)),redirect_port_proof_(std::move(redirect)),hook_publication_proof_(std::move(hook)),controller_(::GetCurrentThreadId()){
    if(!domain_||(!ready_proof_&&!hook_publication_proof_)||(ready_proof_&&hook_publication_proof_))throw std::invalid_argument("Owned PTD domain and one selected callback publication profile required");
    if(hook_publication_proof_&&(connect_original_proof_||web_no_redirect_proof_||other_no_redirect_proof_||redirect_port_proof_))throw std::invalid_argument("Original hook publication cannot mix staged cooperative routes or port snapshots");
    if((web_no_redirect_proof_||other_no_redirect_proof_||redirect_port_proof_)&&!connect_original_proof_&&!hook_publication_proof_)throw std::invalid_argument("Port routes require the same original publication proof");
    threads_=domain_->make_callback_threads();
}
Domain::~Domain() noexcept{std::lock_guard lock(gate_);if(active_||original_connect_calls_||original_forward_calls_||poison_||control_)std::abort();}
const thread_domain::DomainStorage* Domain::memory_domain() const noexcept{return domain_.get();}
void Domain::bind_connect_server_port(std::uint16_t& port){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Original port binding controller required");
    std::lock_guard lock(gate_);
    if(!hook_publication_proof_||connect_server_port_||active_||original_connect_calls_||original_forward_calls_||setup_done_||closing_||control_||poison_)
        throw std::logic_error("Original port binding requires an unbound passive callback domain");
    connect_server_port_=&port;
}
bool Domain::active() const{Error error;std::lock_guard lock(gate_);return active_||original_connect_calls_||original_forward_calls_;}
bool Domain::ready() const{Error error;std::lock_guard lock(gate_);return ready_&&!closing_&&!poison_;}
bool Domain::permits_current() const{return scope(false,false);}
bool Domain::permits_original_current() const{return scope(false,true);}
bool Domain::permits_connect_wait_current() const{return scope(false,false,true);}
bool Domain::permits_connect_body_current() const{return scope(false,false,false,true);}
bool Domain::requires_managed_log()const noexcept{return bool(redirect_port_proof_)||bool(hook_publication_proof_);}
bool Domain::scope(bool probe_allowed,bool original_allowed,bool wait_allowed,bool body_allowed) const{
    Error error;
    if(original_forward_top){
        std::lock_guard lock(gate_);const auto& frame=*original_forward_top;
        return original_allowed&&frame.owner==this&&frame.admitted&&original_forward_calls_&&!poison_&&
            frame.callback==original_forward_callback&&forward_callback(frame.callback)&&
            frame.thread==::GetCurrentThreadId()&&frame.fiber==::GetCurrentFiber();
    }
    if(original_connect_lane.owner==this){
        std::lock_guard lock(gate_);
        return original_connect_lane.admitted&&
            ((original_allowed&&original_connect_lane.phase==OriginalConnectPhase::Original)||
             (wait_allowed&&original_connect_lane.phase==OriginalConnectPhase::Waiting)||
             (body_allowed&&original_connect_lane.phase==OriginalConnectPhase::Redirect))&&
            original_connect_lane.fiber==::GetCurrentFiber()&&original_connect_calls_&&!poison_;
    }
    const auto mode_allowed=[&]{return mode_==Mode::Full||(original_allowed&&mode_==Mode::OriginalOnly)||(probe_allowed&&mode_==Mode::ConnectProbe)||(wait_allowed&&(mode_==Mode::Waiting||mode_==Mode::RedirectWaiting))||(body_allowed&&mode_==Mode::RedirectBody);};
    {std::lock_guard lock(gate_);if(!active_||poison_||!mode_allowed()||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber())return false;}
    bool permitted,poison;
    try{permitted=threads_->permits_current();poison=threads_->poisoned();}
    catch(...){std::lock_guard lock(gate_);poison_=true;ready_=false;throw;}
    std::lock_guard lock(gate_);
    if(poison){poison_=true;ready_=false;}
    return permitted&&active_&&!poison_&&mode_allowed()&&thread_==::GetCurrentThreadId()&&fiber_==::GetCurrentFiber();
}
bool Domain::connect_probe() const{
    Error error;std::lock_guard lock(gate_);
    if(original_forward_top)return false;
    if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=native_recovered::Callback::connect_00fd00)
        throw std::logic_error("Connect route query requires its admitted thread/fiber");
    return mode_==Mode::ConnectProbe;
}
void Domain::connect_classified(bool untouched){
    Error error;check_unsuspended_connect();
    if(!untouched){
        std::lock_guard lock(gate_);
        if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=native_recovered::Callback::connect_00fd00||mode_!=Mode::ConnectProbe)
            throw std::logic_error("Connect classification requires its reserved probe");
        if(closing_)throw Rejected(Fault::Stopped);
        throw Rejected(Fault::Warming);
    }
    if(!original_route(nullptr))throw Rejected(Fault::Warming);
}
bool Domain::connect_web_passthrough(std::uint16_t port){return original_route(&port);}
bool Domain::connect_wait_begin(std::uint16_t port){return original_route(&port,true);}
bool Domain::original_route(const std::uint16_t* requested,bool waiting){
    Error error;check_unsuspended_connect();
    {
        std::lock_guard lock(gate_);
        if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=native_recovered::Callback::connect_00fd00||mode_!=Mode::ConnectProbe)
            throw std::logic_error("Connect classification requires its reserved probe");
        if(closing_)throw Rejected(Fault::Stopped);
        if(waiting){
            if(!requested||::GetCurrentThreadId()==controller_||*requested==80||*requested==443||*requested==3216||!other_no_redirect_proof_)return false;
        }else if(requested&&((*requested!=80&&*requested!=443)||!web_no_redirect_proof_))return false;
        mode_=Mode::Checking;
    }
    bool published;
    try{published=connect_original_proof_()&&(!requested||(waiting?other_no_redirect_proof_(*requested):web_no_redirect_proof_(*requested)));}
    catch(...){std::lock_guard lock(gate_);poison_=true;ready_=false;throw;}
    std::lock_guard lock(gate_);
    if(poison_||mode_!=Mode::Checking){poison_=true;ready_=false;throw std::logic_error("Connect original proof lost its reservation");}
    mode_=Mode::ConnectProbe;
    if(closing_)throw Rejected(Fault::Stopped);
    if(!published)return false;
    if(waiting){waiting_port_=*requested;mode_=Mode::Waiting;}
    else mode_=Mode::OriginalOnly;
    return true;
}
void Domain::connect_wait_complete(){
    Error error;check_unsuspended_connect();std::uint16_t port;
    {
        std::lock_guard lock(gate_);
        if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=native_recovered::Callback::connect_00fd00||mode_!=Mode::Waiting)
            throw std::logic_error("Connect wait completion requires its admitted thread/fiber");
        port=waiting_port_;mode_=Mode::Checking;
    }
    bool published;
    try{published=connect_original_proof_()&&other_no_redirect_proof_(port);}
    catch(...){std::lock_guard lock(gate_);poison_=true;ready_=false;throw;}
    std::lock_guard lock(gate_);
    if(poison_||mode_!=Mode::Checking){poison_=true;ready_=false;throw std::logic_error("Connect wait lost its original reservation");}
    mode_=Mode::ConnectProbe;waiting_port_=0;
    if(!published)throw Rejected(closing_?Fault::Stopped:Fault::Warming);
    // Already admitted before pause: finish without admitting a new callback.
    // Pause cannot turn an unfinished native wait into successful completion.
    mode_=Mode::OriginalOnly;
}
bool Domain::connect_redirect_begin(std::uint16_t requested){
    Error error;check_unsuspended_connect();
    {
        std::lock_guard lock(gate_);
        if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=native_recovered::Callback::connect_00fd00||mode_!=Mode::ConnectProbe)
            throw std::logic_error("Redirect requires its reserved connect probe");
        if(closing_)throw Rejected(Fault::Stopped);
        if(::GetCurrentThreadId()==controller_||!redirect_port_proof_)return false;
        mode_=Mode::Checking;
    }
    std::optional<std::uint16_t> port;
    try{if(connect_original_proof_())port=redirect_port_proof_();}
    catch(...){std::lock_guard lock(gate_);poison_=true;ready_=false;throw;}
    std::lock_guard lock(gate_);
    if(poison_||mode_!=Mode::Checking){poison_=true;ready_=false;throw std::logic_error("Redirect probe lost its reservation");}
    mode_=Mode::ConnectProbe;
    if(closing_)throw Rejected(Fault::Stopped);
    if(!port||(requested!=3216&&requested!=*port))return false;
    waiting_port_=requested;redirect_server_port_=*port;
    mode_=(requested==80||requested==443)?Mode::RedirectPending:Mode::RedirectWaiting;return true;
}
std::uint16_t Domain::redirect_checkpoint(bool completed){
    Error error;check_unsuspended_connect();std::uint16_t requested,expected;
    {
        std::lock_guard lock(gate_);
        const bool phase=completed?mode_==Mode::RedirectBody:(mode_==Mode::RedirectPending||mode_==Mode::RedirectWaiting);
        if(!active_||poison_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=native_recovered::Callback::connect_00fd00||!phase)
            throw std::logic_error("Redirect checkpoint requires its admitted thread/fiber and phase");
        requested=waiting_port_;expected=redirect_server_port_;mode_=Mode::Checking;
    }
    std::optional<std::uint16_t> port;
    try{if(connect_original_proof_())port=redirect_port_proof_();}
    catch(...){std::lock_guard lock(gate_);poison_=true;ready_=false;throw;}
    std::lock_guard lock(gate_);
    if(poison_||mode_!=Mode::Checking){poison_=true;ready_=false;throw std::logic_error("Redirect checkpoint lost its reservation");}
    mode_=Mode::ConnectProbe;
    if(!port||*port!=expected||(requested!=3216&&requested!=*port))throw Rejected(closing_?Fault::Stopped:Fault::Warming);
    mode_=completed?Mode::OriginalOnly:Mode::RedirectBody;
    return *port;
}
void Domain::begin(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Host setup controller required");
    std::lock_guard lock(gate_);if(active_||original_connect_calls_||original_forward_calls_||closing_||control_||poison_)throw std::logic_error("Host setup cannot mutate an active/closing callback domain");
    setup_done_=false;ready_=false;
}
void Domain::complete(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Host setup controller required");
    {std::lock_guard lock(gate_);
        if(closing_||poison_||control_||(active_&&mode_==Mode::Full&&!hook_publication_proof_))throw std::logic_error("Invalid completed host setup");
        setup_done_=true;
        if(hook_publication_proof_){ready_=true;return;}
        // A restricted probe/original/wait entry does not touch host body
        // state. Do not wait for it or attempt a full proof while it is active.
        if(active_)return;
    }
    (void)activate_foreign();
}
bool Domain::activate_foreign(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Foreign activation controller required");
    {
        std::lock_guard lock(gate_);if(control_||active_||original_connect_calls_||original_forward_calls_||closing_||poison_)throw std::logic_error("Foreign activation is not quiescent");
        if(!setup_done_)return false;if(hook_publication_proof_){ready_=true;return true;}ready_=false;control_=true;
    }
    bool proof;
    try{proof=ready_proof_();}catch(...){std::lock_guard lock(gate_);control_=false;throw;}
    std::lock_guard lock(gate_);control_=false;
    if(active_||original_connect_calls_||original_forward_calls_||closing_||poison_)throw std::logic_error("Foreign activation lost quiescence");
    ready_=proof;return proof;
}
bool Domain::pause(){
    Error error;if(::GetCurrentThreadId()!=controller_)throw std::logic_error("Callback pause controller required");
    std::lock_guard lock(gate_);if(control_||poison_)throw std::logic_error("Cannot pause unknown callback outcome");
    closing_=true;ready_=false;return !active_&&!original_connect_calls_&&!original_forward_calls_;
}
bool Domain::try_enter_original_forward(native_recovered::Callback callback,native_recovered::OriginalForwardFrame& frame){
    Error error;
    if(!hook_publication_proof_||!forward_callback(callback))return false;
    auto* const parent=original_forward_top;
    const auto parent_callback=original_forward_callback;
    const auto thread=::GetCurrentThreadId();auto* const fiber=::GetCurrentFiber();
    {
        std::lock_guard lock(gate_);
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(control_)throw Rejected(thread==controller_?Fault::Reentry:Fault::Warming);
        if(frame.owner||frame.parent||frame.thread||frame.fiber||frame.admitted||frame.callback!=native_recovered::Callback{})
            throw std::logic_error("Pure original entry requires a clean automatic frame");
        if(parent&&(!parent->owner||parent->thread!=thread||parent->fiber!=fiber||
           parent->callback!=parent_callback||!forward_callback(parent_callback)))throw Rejected(Fault::Reentry);
        if((original_connect_lane.owner&&original_connect_lane.fiber!=fiber)||
           (active_&&thread_==thread&&fiber_!=fiber))throw Rejected(Fault::Reentry);
        // Install checking metadata before the opaque publication proof. Even
        // a nested proof callback must not inherit any outer body permission.
        frame.owner=this;frame.parent=parent;frame.thread=thread;frame.fiber=fiber;
        frame.callback=callback;frame.admitted=false;
        ++original_forward_calls_;original_forward_top=&frame;original_forward_callback=callback;
    }
    bool observing=true;
    try{
        const bool published=hook_publication_proof_(callback);observing=false;
        std::lock_guard lock(gate_);
        if(original_forward_top!=&frame||frame.owner!=this||frame.parent!=parent||frame.thread!=thread||
           frame.fiber!=fiber||frame.callback!=callback||original_forward_callback!=callback||frame.admitted||
           thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber()){
            poison_=true;ready_=false;throw std::logic_error("Pure original proof changed its actor/frame reservation");
        }
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(!published)throw Rejected(Fault::Warming);
        frame.admitted=true;return true;
    }catch(...){
        std::lock_guard lock(gate_);
        if(observing){poison_=true;ready_=false;}
        if(original_forward_top!=&frame||!original_forward_calls_)std::abort();
        original_forward_top=parent;original_forward_callback=parent_callback;
        --original_forward_calls_;clear_forward_frame(frame);throw;
    }
}
void Domain::leave_original_forward(native_recovered::OriginalForwardFrame& frame) noexcept{
    Error error;std::lock_guard lock(gate_);
    if(original_forward_top!=&frame||frame.owner!=this||!frame.admitted||!original_forward_calls_||
       frame.thread!=::GetCurrentThreadId()||frame.fiber!=::GetCurrentFiber()||
       frame.callback!=original_forward_callback||!forward_callback(frame.callback))std::abort();
    auto* const parent=frame.parent;
    if(parent&&(!parent->owner||parent->thread!=frame.thread||parent->fiber!=frame.fiber||
       !forward_callback(parent->callback)))std::abort();
    original_forward_top=parent;original_forward_callback=parent?parent->callback:native_recovered::Callback{};
    --original_forward_calls_;clear_forward_frame(frame);
}
bool Domain::try_enter_connect_original(std::uint16_t& exact_server_port,std::optional<std::uint16_t> port,bool wait_for_steps){
    Error error;check_unsuspended_connect();
    if(!hook_publication_proof_)return false;
    if(wait_for_steps){if(!port||*port==80||*port==443)return false;}
    else if(port&&*port!=80&&*port!=443)return false;
    {
        std::lock_guard lock(gate_);
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(connect_server_port_!=&exact_server_port)throw std::logic_error("Original connect requires its exact before-publish port binding");
        if(original_connect_lane.owner||(active_&&thread_==::GetCurrentThreadId()))throw Rejected(Fault::Reentry);
        if(control_)throw Rejected(::GetCurrentThreadId()==controller_?Fault::Reentry:Fault::Warming);
        ++original_connect_calls_;
        original_connect_lane={this,::GetCurrentFiber(),false};
    }
    bool observing=true;
    try{
        const bool published=hook_publication_proof_(native_recovered::Callback::connect_00fd00);
        // Native00fd73 compares the live word for web calls without a Server
        // milestone. Non-web waits first, so do not read this cell here.
        const bool redirect=published&&port&&!wait_for_steps&&
            *port==network_hook_recovered::load_server_port_0a3c90(*connect_server_port_);
        observing=false;
        std::lock_guard lock(gate_);
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(!published)throw Rejected(Fault::Warming);
        original_connect_lane.phase=wait_for_steps?OriginalConnectPhase::Waiting:
            redirect?OriginalConnectPhase::Redirect:OriginalConnectPhase::Original;
        original_connect_lane.requested=port.value_or(0);
        original_connect_lane.admitted=true;
        return true;
    }catch(...){
        std::lock_guard lock(gate_);
        if(observing){poison_=true;ready_=false;}
        original_connect_lane={};--original_connect_calls_;
        throw;
    }
}
bool Domain::connect_original_wait_pending()const{
    Error error;std::lock_guard lock(gate_);
    if(original_forward_top)return false;
    return original_connect_lane.owner==this&&original_connect_lane.admitted&&original_connect_lane.phase==OriginalConnectPhase::Waiting&&
        original_connect_lane.fiber==::GetCurrentFiber()&&!poison_;
}
void Domain::complete_connect_original_wait(){
    Error error;check_unsuspended_connect();std::uint16_t requested;
    {
        std::lock_guard lock(gate_);
        if(original_connect_lane.owner!=this||!original_connect_lane.admitted||original_connect_lane.phase!=OriginalConnectPhase::Waiting||
           original_connect_lane.fiber!=::GetCurrentFiber()||poison_)
            throw std::logic_error("Concurrent original wait completion requires its exact waiting actor");
        requested=original_connect_lane.requested;
        original_connect_lane.phase=OriginalConnectPhase::Checking;
    }
    bool observing=true;
    try{
        const bool published=hook_publication_proof_(native_recovered::Callback::connect_00fd00);
        //3216 branches straight to the body; other non-web ports compare only
        // after the actual native wait has returned (00fd6b..00fd73).
        const bool redirect=published&&(requested==3216||
            requested==network_hook_recovered::load_server_port_0a3c90(*connect_server_port_));observing=false;
        std::lock_guard lock(gate_);
        if(poison_||!published)throw Rejected(closing_?Fault::Stopped:Fault::Warming);
        original_connect_lane.phase=redirect?OriginalConnectPhase::Redirect:OriginalConnectPhase::Original;
    }catch(...){
        std::lock_guard lock(gate_);original_connect_lane.phase=OriginalConnectPhase::Checking;
        if(observing){poison_=true;ready_=false;}throw;
    }
}
bool Domain::connect_original_redirect_pending()const{
    Error error;std::lock_guard lock(gate_);
    if(original_forward_top)return false;
    if(original_connect_lane.owner!=this||!original_connect_lane.admitted||original_connect_lane.fiber!=::GetCurrentFiber()||poison_||
       (original_connect_lane.phase!=OriginalConnectPhase::Redirect&&original_connect_lane.phase!=OriginalConnectPhase::Original))
        throw std::logic_error("Original redirect query requires its completed route decision");
    return original_connect_lane.phase==OriginalConnectPhase::Redirect;
}
void Domain::complete_connect_original_redirect(){
    Error error;check_unsuspended_connect();
    {
        std::lock_guard lock(gate_);
        if(original_connect_lane.owner!=this||!original_connect_lane.admitted||original_connect_lane.phase!=OriginalConnectPhase::Redirect||
           original_connect_lane.fiber!=::GetCurrentFiber()||poison_)
            throw std::logic_error("Original redirect completion requires its exact body actor");
        original_connect_lane.phase=OriginalConnectPhase::Checking;
    }
    bool observing=true;
    try{
        const bool published=hook_publication_proof_(native_recovered::Callback::connect_00fd00);
        observing=false;
        std::lock_guard lock(gate_);
        // Native route selection is already complete. A changed scalar cannot
        // undo it; the body's log and final store perform their own live reads.
        if(poison_||!published)throw Rejected(closing_?Fault::Stopped:Fault::Warming);
        original_connect_lane.phase=OriginalConnectPhase::Original;
    }catch(...){
        std::lock_guard lock(gate_);original_connect_lane.phase=OriginalConnectPhase::Checking;
        if(observing){poison_=true;ready_=false;}throw;
    }
}
bool Domain::begin_registry_sdk(native_recovered::Callback callback,native_recovered::RegistrySdkFrame& frame){
    Error error;
    if(!hook_publication_proof_||!registry_callback(callback))return false;
    if(frame.owner||frame.parent||frame.body||frame.thread||frame.fiber||frame.callback!=native_recovered::Callback{}||frame.admitted)
        throw std::logic_error("Registry SDK frame must be unused");
    if(original_forward_top||original_connect_lane.owner)return false;
    auto* const parent=registry_sdk_top;auto* const body=registry_nested_top;
    const auto thread=::GetCurrentThreadId();auto* const fiber=::GetCurrentFiber();
    {
        std::lock_guard lock(gate_);
        if(!active_||mode_!=Mode::Full||thread_!=thread||fiber_!=fiber||callback_!=callback)return false;
        if(poison_)throw Rejected(Fault::Stopped);
        if(control_||bool(body)!=bool(parent)||(body&&(body->owner!=this||!body->admitted||body->callback!=callback||body->thread!=thread||body->fiber!=fiber||body->sdk!=parent))||
           (parent&&(parent->owner!=this||!parent->admitted||parent->thread!=thread||parent->fiber!=fiber||parent->body==body)))
            throw Rejected(Fault::Reentry);
    }
    // The existing root PTD stays active. Do not reject only because pause
    // closed new admission while this already-owned SDK call was progressing.
    if(!scope(false,false))throw std::logic_error("Registry SDK phase requires its root PTD");
    std::lock_guard lock(gate_);
    if(!active_||poison_||mode_!=Mode::Full||thread_!=thread||fiber_!=fiber||callback_!=callback||
       registry_sdk_top!=parent||registry_nested_top!=body||original_forward_top||original_connect_lane.owner){
        poison_=true;ready_=false;throw std::logic_error("Registry SDK phase lost its actor");
    }
    frame.owner=this;frame.parent=parent;frame.body=body;frame.thread=thread;frame.fiber=fiber;
    frame.callback=callback;frame.admitted=true;registry_sdk_top=&frame;return true;
}
void Domain::end_registry_sdk(native_recovered::RegistrySdkFrame& frame) noexcept {
    Error error;std::lock_guard lock(gate_);
    if(registry_sdk_top!=&frame||frame.owner!=this||!frame.admitted||frame.body!=registry_nested_top||bool(frame.body)!=bool(frame.parent)||
       (frame.body&&(frame.body->owner!=this||!frame.body->admitted||frame.body->callback!=frame.callback||frame.body->sdk!=frame.parent))||
       frame.thread!=::GetCurrentThreadId()||frame.fiber!=::GetCurrentFiber()||!active_||mode_!=Mode::Full||
       thread_!=frame.thread||fiber_!=frame.fiber||callback_!=frame.callback||original_forward_top||original_connect_lane.owner)
        std::abort();
    registry_sdk_top=frame.parent;clear_registry_sdk(frame);
}
bool Domain::try_enter_registry_nested(native_recovered::Callback callback,native_recovered::RegistryNestedFrame& frame){
    Error error;
    if(!hook_publication_proof_||!registry_callback(callback))return false;
    if(frame.owner||frame.parent||frame.sdk||frame.thread||frame.fiber||frame.callback!=native_recovered::Callback{}||frame.parent_callback!=native_recovered::Callback{}||frame.admitted)
        throw std::logic_error("Nested registry frame must be unused");
    if(original_forward_top||original_connect_lane.owner)return false;
    auto* const sdk=registry_sdk_top;auto* const parent=registry_nested_top;
    if(!sdk||sdk->owner!=this)return false;
    const auto thread=::GetCurrentThreadId();auto* const fiber=::GetCurrentFiber();
    const auto parent_callback=sdk->callback;
    {
        std::lock_guard lock(gate_);
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(control_||!active_||mode_!=Mode::Full||thread_!=thread||fiber_!=fiber||!registry_callback(callback_)||
           !sdk->admitted||sdk->thread!=thread||sdk->fiber!=fiber||sdk->callback!=callback_||sdk->body!=parent||
           sdk->parent!=(parent?parent->sdk:nullptr)||
           (parent&&(parent->owner!=this||!parent->admitted||parent->callback!=callback_||parent->thread!=thread||parent->fiber!=fiber)))
            throw Rejected(Fault::Reentry);
        frame.owner=this;frame.parent=parent;frame.sdk=sdk;frame.thread=thread;frame.fiber=fiber;
        frame.callback=callback;frame.parent_callback=parent_callback;registry_nested_top=&frame;
        callback_=callback;mode_=Mode::Checking;
    }
    bool observing=false;
    const auto reserved=[&]{return registry_nested_top==&frame&&registry_sdk_top==sdk&&frame.owner==this&&
        frame.parent==parent&&frame.sdk==sdk&&frame.thread==thread&&frame.fiber==fiber&&frame.callback==callback&&frame.parent_callback==parent_callback&&
        sdk->owner==this&&sdk->admitted&&sdk->body==parent&&sdk->thread==thread&&sdk->fiber==fiber&&sdk->callback==parent_callback&&
        !frame.admitted&&active_&&thread_==thread&&fiber_==fiber&&callback_==callback&&mode_==Mode::Checking&&
        !original_forward_top&&!original_connect_lane.owner;};
    try{
        observing=true;const bool published=hook_publication_proof_(callback);observing=false;
        {
            std::lock_guard lock(gate_);
            if(!reserved()){poison_=true;ready_=false;throw std::logic_error("Nested registry publication lost reservation");}
            if(closing_||poison_)throw Rejected(Fault::Stopped);
            if(!published)throw Rejected(Fault::Warming);
        }
        observing=true;const bool permitted=threads_->permits_current();const bool ptd_poison=threads_->poisoned();observing=false;
        std::lock_guard lock(gate_);
        if(!reserved()){poison_=true;ready_=false;throw std::logic_error("Nested registry PTD check lost reservation");}
        if(!permitted||ptd_poison){poison_=true;ready_=false;throw std::logic_error("Nested registry body lost its root PTD");}
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        frame.admitted=true;mode_=Mode::Full;return true;
    }catch(...){
        std::lock_guard lock(gate_);
        if(!reserved())std::abort();
        if(observing){poison_=true;ready_=false;}
        registry_nested_top=parent;callback_=frame.parent_callback;mode_=Mode::Full;clear_registry_nested(frame);throw;
    }
}
void Domain::leave_registry_nested(native_recovered::RegistryNestedFrame& frame) noexcept {
    Error error;std::lock_guard lock(gate_);
    if(registry_nested_top!=&frame||frame.owner!=this||!frame.admitted||registry_sdk_top!=frame.sdk||!frame.sdk||
       frame.sdk->owner!=this||!frame.sdk->admitted||frame.sdk->body!=frame.parent||frame.sdk->callback!=frame.parent_callback||
       frame.sdk->thread!=frame.thread||frame.sdk->fiber!=frame.fiber||frame.sdk->parent!=(frame.parent?frame.parent->sdk:nullptr)||
       frame.thread!=::GetCurrentThreadId()||frame.fiber!=::GetCurrentFiber()||!active_||mode_!=Mode::Full||
       thread_!=frame.thread||fiber_!=frame.fiber||callback_!=frame.callback||original_forward_top||original_connect_lane.owner)
        std::abort();
    registry_nested_top=frame.parent;callback_=frame.parent_callback;clear_registry_nested(frame);
}
void Domain::enter(native_recovered::Callback callback){
    Error error;
    {
        std::lock_guard lock(gate_);
        if(closing_||poison_)throw Rejected(Fault::Stopped);
        if(original_forward_top||original_connect_lane.owner||registry_sdk_top||registry_nested_top)throw Rejected(Fault::Reentry);
        if(control_)throw Rejected(::GetCurrentThreadId()==controller_?Fault::Reentry:Fault::Warming);
        if(active_)throw Rejected(thread_==::GetCurrentThreadId()?Fault::Reentry:Fault::Overlap);
        mode_=Mode::Full;
        if(hook_publication_proof_)mode_=Mode::Checking;
        else if(!ready_&&connect_original_proof_&&callback==native_recovered::Callback::connect_00fd00)mode_=Mode::ConnectProbe;
        else if(!ready_&&::GetCurrentThreadId()!=controller_)throw Rejected(Fault::Warming);
        active_=true;thread_=::GetCurrentThreadId();fiber_=::GetCurrentFiber();callback_=callback;
    }
    bool observing=false;
    try{
        if(hook_publication_proof_){
            observing=true;const bool published=hook_publication_proof_(callback);observing=false;
            std::lock_guard lock(gate_);if(closing_||poison_)throw Rejected(Fault::Stopped);
            if(!published)throw Rejected(Fault::Warming);mode_=Mode::Full;
        }
        threads_->enter();if(!scope(true,false))throw std::logic_error("Initial callback PTD permission failed");
    }catch(...){std::lock_guard lock(gate_);poison_=observing||threads_->poisoned()||threads_->active();if(!poison_)active_=false;throw;}
}
void Domain::leave(native_recovered::Callback callback) noexcept{
    Error error;
    if(original_forward_top||registry_sdk_top||registry_nested_top)std::abort();
    if(original_connect_lane.owner==this){
        std::lock_guard lock(gate_);
        if(callback!=native_recovered::Callback::connect_00fd00||!original_connect_lane.admitted||original_connect_lane.fiber!=::GetCurrentFiber()||!original_connect_calls_)std::abort();
        original_connect_lane={};--original_connect_calls_;return;
    }
    {std::lock_guard lock(gate_);if(!active_||thread_!=::GetCurrentThreadId()||fiber_!=::GetCurrentFiber()||callback_!=callback)std::abort();}
    try{if(!threads_->leave())std::abort();}catch(...){std::abort();}
    std::lock_guard lock(gate_);active_=false;mode_=Mode::Full;waiting_port_=redirect_server_port_=0;
}
Admission::Admission(std::shared_ptr<Domain> domain):domain_(std::move(domain)){if(!domain_)throw std::invalid_argument("Foreign callback domain required");}
void Admission::enter(native_recovered::Callback value){domain_->enter(value);}
void Admission::leave(native_recovered::Callback value)noexcept{domain_->leave(value);}
bool Admission::try_enter_original_forward(native_recovered::Callback value,native_recovered::OriginalForwardFrame& frame){return domain_->try_enter_original_forward(value,frame);}
void Admission::leave_original_forward(native_recovered::OriginalForwardFrame& frame)noexcept{domain_->leave_original_forward(frame);}
bool Admission::begin_registry_sdk(native_recovered::Callback value,native_recovered::RegistrySdkFrame& frame){return domain_->begin_registry_sdk(value,frame);}
void Admission::end_registry_sdk(native_recovered::RegistrySdkFrame& frame)noexcept{domain_->end_registry_sdk(frame);}
bool Admission::try_enter_registry_nested(native_recovered::Callback value,native_recovered::RegistryNestedFrame& frame){return domain_->try_enter_registry_nested(value,frame);}
void Admission::leave_registry_nested(native_recovered::RegistryNestedFrame& frame)noexcept{domain_->leave_registry_nested(frame);}
bool Admission::has_connect_original_lane()const noexcept{return bool(domain_->hook_publication_proof_);}
bool Admission::try_enter_connect_original(std::uint16_t& cell,std::optional<std::uint16_t> port,bool wait){return domain_->try_enter_connect_original(cell,port,wait);}
bool Admission::connect_original_wait_pending()const{return domain_->connect_original_wait_pending();}
void Admission::complete_connect_original_wait(){domain_->complete_connect_original_wait();}
bool Admission::connect_original_redirect_pending()const{return domain_->connect_original_redirect_pending();}
void Admission::complete_connect_original_redirect(){domain_->complete_connect_original_redirect();}
void Admission::host_setup_begin(){domain_->begin();}
void Admission::host_setup_complete(){domain_->complete();}
bool Admission::pause_before_cleanup(){return domain_->pause();}
bool Admission::connect_probe()const{return domain_->connect_probe();}
void Admission::connect_classified(bool untouched){domain_->connect_classified(untouched);}
bool Admission::connect_web_passthrough(std::uint16_t port){return domain_->connect_web_passthrough(port);}
bool Admission::connect_wait_begin(std::uint16_t port){return domain_->connect_wait_begin(port);}
void Admission::connect_wait_complete(){domain_->connect_wait_complete();}
bool Admission::connect_redirect_begin(std::uint16_t port){return domain_->connect_redirect_begin(port);}
std::uint16_t Admission::connect_redirect_ready(){return domain_->redirect_checkpoint(false);}
void Admission::connect_redirect_complete(){(void)domain_->redirect_checkpoint(true);}
bootstrap_backend::CallbackAttachmentFactory make_factory(std::shared_ptr<Domain> domain,
    native_context::CallbackSeed seed,callback_policy::Resources resources,native_recovered::Errors& errors,native_context::Quiescence proof,native_context::ClosureProfile profile){
    if(!domain||!resources.memory||resources.memory->domain_identity()!=domain->memory_domain())throw std::invalid_argument("Callback memory must use exact PTD domain");
    if(domain->requires_managed_log()&&!std::dynamic_pointer_cast<shared_log::Stream>(resources.stream))throw std::invalid_argument("Early redirect requires the owned managed shared log stream");
    if(resources.callback_scope||resources.original_scope||resources.connect_wait_scope||resources.connect_body_scope)throw std::invalid_argument("Existing callback/original/wait/body permission cannot be silently replaced");
    resources.callback_scope=[domain]{return domain->permits_current();};
    resources.original_scope=[domain]{return domain->permits_original_current();};
    resources.connect_wait_scope=[domain]{return domain->permits_connect_wait_current();};
    resources.connect_body_scope=[domain]{return domain->permits_connect_body_current();};
    resources.encoding=callback_identity::check_encoding(std::move(resources.encoding),resources.callback_scope);
    resources.owned_crt=callback_provider::check_crt(resources.crt,std::move(resources.owned_crt),resources.callback_scope);
    auto attachment=native_context::make_factory(std::move(seed),callback_policy::make_policy_factory(std::move(resources)),errors,std::move(proof),std::make_unique<Admission>(domain),profile);
    return [domain,attachment=std::move(attachment),profile](bootstrap_backend::CallbackStateView view){
        // Binding is passive and precedes Owner construction/publication. The
        // exact same view cell becomes native Context.network.server_port.
        if(profile==native_context::ClosureProfile::OriginalCrt)domain->bind_connect_server_port(view.configuration.server_port);
        return attachment(view);
    };
}
static_assert(!std::is_abstract_v<Admission>);
} // namespace anadius::foreign_callback
