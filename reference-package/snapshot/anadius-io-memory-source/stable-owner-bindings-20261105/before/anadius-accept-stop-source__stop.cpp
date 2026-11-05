#include "stop.hpp"
#include <bit>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <mutex>
#include <optional>
#include <stdexcept>

namespace anadius::accept_stop {
namespace {struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};}
struct Stop::Impl {
    enum class Family {Accept,Receive,Send,Posted};
    struct Loan {
        Family family;s::IoMemoryHandle handle;std::uint64_t submission;s::AcceptSubmission state;
        s::Socket listener;OVERLAPPED* overlapped;s::CancelOperation* operation;bool pending,consumed;
        s::IoReference* source_control=nullptr;std::uintptr_t buffer=0;std::uint64_t bytes=0;
    };
    struct Entry {Loan loan;bool cancelled=false,observed=false,ack=false;std::uint64_t serial=0;};
    struct Packet {OVERLAPPED* raw;std::uint32_t bytes,status;std::uintptr_t key;std::int32_t received;};
    struct Route {std::uint64_t serial;bool drop;};
    Options options;const DWORD controller=::GetCurrentThreadId();void* const controller_fiber=::GetCurrentFiber();
    mutable std::mutex gate;std::condition_variable posts_done;Phase phase=Phase::Fresh;
    bool control_busy=false,working=false;DWORD worker=0,slot=FLS_OUT_OF_INDEXES;void* fiber=nullptr;void* ptd=nullptr;
    std::optional<o::ServerView> view;s::OwnedAcceptMemory* accept=nullptr;
    std::vector<Entry> entries;std::optional<Packet> packet;
    std::optional<Route> route;std::uint64_t route_epoch=0;
    std::size_t cancel_calls=0,not_found=0,kernel_packets=0,software_packets=0,acknowledged=0,active_posts=0;
    std::uint64_t serial=0;
    explicit Impl(Options o):options(std::move(o)){entries.reserve(options.capacity);}
    ~Impl(){if(phase!=Phase::Retiring)std::terminate();}
    std::uintptr_t wake()const noexcept{return reinterpret_cast<std::uintptr_t>(this);}
    void poison()noexcept{std::lock_guard lock(gate);if(phase!=Phase::Closed&&phase!=Phase::Retiring)phase=Phase::Poisoned;posts_done.notify_all();}
    void identity(){
        if(worker!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Accept stop source calls belong to the bound worker/fiber");
        if(::FlsGetValue(slot)!=ptd){poison();throw std::logic_error("Accept stop Domain PTD changed");}
    }
    struct Call {
        Impl& v;explicit Call(Impl& v):v(v){
            v.identity();
            {std::lock_guard lock(v.gate);if(v.phase==Phase::Closed||v.phase==Phase::Closing||v.phase==Phase::Retiring||v.phase==Phase::Poisoned)throw std::logic_error("Accept stop source admission is closed or unknown");}
            (void)v.view->memory.buffer_context();
            std::lock_guard lock(v.gate);if(v.working){v.phase=Phase::Poisoned;throw std::logic_error("Accept stop source reentry");}v.working=true;
        }
        ~Call(){std::lock_guard lock(v.gate);v.working=false;}
    };
    bool stopping(){std::lock_guard lock(gate);return phase==Phase::Requested||phase==Phase::Draining;}
    bool session_io()const noexcept{return options.scope==Scope::SessionIo||options.scope==Scope::SessionIoPrepared;}
    static Loan convert(const s::AcceptLoan& l){return {Family::Accept,{l.handle.slot,l.handle.generation},l.submission,l.state,l.listener,l.overlapped,l.operation,l.pending,l.consumed};}
    static Loan convert(const s::IoLoan& l){
        const auto family=l.kind==s::IoMemoryKind::Receive?Family::Receive:l.kind==s::IoMemoryKind::Send?Family::Send:Family::Posted;
        if(l.kind!=s::IoMemoryKind::Receive&&l.kind!=s::IoMemoryKind::Send&&l.kind!=s::IoMemoryKind::Posted)throw std::logic_error("SessionIo packet is not owned receive/send/posted work");
        auto state=s::AcceptSubmission::Unknown;
        switch(l.state){
            case s::IoSubmission::NotSubmitted:state=s::AcceptSubmission::NotSubmitted;break;
            case s::IoSubmission::Issuing:state=s::AcceptSubmission::Issuing;break;
            case s::IoSubmission::Kernel:state=s::AcceptSubmission::Kernel;break;
            case s::IoSubmission::Software:state=s::AcceptSubmission::Software;break;
            case s::IoSubmission::Unclassified:case s::IoSubmission::Unknown:break;
        }
        return {family,l.handle,l.submission,state,l.socket,l.overlapped,l.operation,l.pending,l.completed,l.source_control,l.buffer,l.bytes};
    }
    std::vector<Loan> pending(){
        std::vector<Loan> out;out.reserve(options.capacity);
        for(const auto& l:accept->pending_loans())out.push_back(convert(l));
        if(session_io())for(const auto& l:view->io.pending_loans())out.push_back(convert(l));return out;
    }
    bool active_session_io(){
        bool active=false;
        for(const auto& loan:view->io.active_loans()){
            if(loan.binding_valid){active=true;continue;}
            if(loan.kind!=s::IoMemoryKind::Posted||!accept->contains(loan.binding_operation))throw std::logic_error("Unowned IO alias during prepared session drain");
            const auto handle=accept->handle_for(loan.binding_overlapped);const auto actual=accept->loan(handle);
            if(handle.slot!=loan.handle.slot||actual.operation!=loan.binding_operation||actual.overlapped!=loan.binding_overlapped)throw std::logic_error("Prepared drain accept alias differs from actual owner");
        }
        return active;
    }
    Loan current(const Loan& l){
        if(l.family==Family::Accept)return convert(accept->loan({l.handle.slot,l.handle.generation}));
        return convert(view->io.loan(l.handle));
    }
    void checkpoint(){
        identity();(void)view->memory.buffer_context();std::lock_guard lock(gate);
        if(phase==Phase::Poisoned||phase==Phase::Closed||phase==Phase::Closing||phase==Phase::Retiring)throw std::logic_error("Accept stop was poisoned during a lower effect");
    }
    Entry& entry(const Loan& loan){
        std::lock_guard lock(gate);
        for(auto& e:entries)if(e.loan.family==loan.family&&e.loan.handle==loan.handle&&e.loan.submission==loan.submission){
            if(e.loan.overlapped!=loan.overlapped||e.loan.listener!=loan.listener||e.loan.state!=loan.state||e.loan.source_control!=loan.source_control||e.loan.buffer!=loan.buffer||e.loan.bytes!=loan.bytes)throw std::logic_error("Tracked submission identity changed");return e;
        }
        if(entries.size()==options.capacity)throw std::length_error("Accept stop receipt capacity exhausted");
        entries.push_back({loan});return entries.back();
    }
};
Stop::Stop(std::unique_ptr<Impl> p)noexcept:impl_(std::move(p)){}
std::shared_ptr<Stop> Stop::create(Options options){
    PreserveError error;const auto& i=options.imports;
    if(!options.lifetime||!options.other_work_drained||!i.cancel||!i.post||!i.error||!options.capacity||options.capacity>65536||!options.post_drain_timeout_ms||options.post_drain_timeout_ms==INFINITE||(options.scope!=Scope::AcceptOnly&&options.scope!=Scope::SessionIo&&options.scope!=Scope::SessionIoPrepared))
        throw std::invalid_argument("Owned other-work proof, native cancel/post imports, bounded capacity and finite timeout required");
    auto result=std::shared_ptr<Stop>(new Stop(nullptr));result->impl_=std::make_unique<Impl>(std::move(options));return result;
}
Stop::~Stop()noexcept{
    PreserveError error;if(!impl_)return;auto& v=*impl_;
    {std::lock_guard lock(v.gate);if((v.phase!=Phase::Fresh&&v.phase!=Phase::Closed)||v.working||v.control_busy||v.active_posts)std::terminate();v.phase=Phase::Retiring;}
    {Options retiring=std::move(v.options);v.options.other_work_drained={};v.options.lifetime.reset();}
}
bool Stop::request(){
    PreserveError error;auto& v=*impl_;std::uint64_t epoch=0;
    {
        std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return true;if(v.phase==Phase::Retiring)return false;
        if(v.controller!=::GetCurrentThreadId()||v.controller_fiber!=::GetCurrentFiber())throw std::logic_error("Accept stop request belongs to its controller/fiber");
        if(v.control_busy){v.phase=Phase::Poisoned;throw std::logic_error("Accept stop controller reentry");}
        if(v.phase==Phase::Requested||v.phase==Phase::Draining)return true;
        if(v.phase!=Phase::Running||v.route)return false;epoch=v.route_epoch;v.control_busy=true;
    }
    std::shared_ptr<Stop> self; // destroyed AFTER the Busy reservation clears
    struct Busy {Impl& v;~Busy(){std::lock_guard lock(v.gate);v.control_busy=false;}} busy{v};
    self=shared_from_this(); // Retiring callbacks must not try to resurrect it
    HANDLE port=nullptr;
    try{
        const bool other=v.options.other_work_drained();
        {std::lock_guard lock(v.gate);
            if(v.phase!=Phase::Running)throw std::logic_error("Other-work proof changed stop admission");
            if(v.route||v.route_epoch!=epoch)return false; // retry a proof overlapped by source routing
            if(!other)return false;
            v.phase=Phase::Requested;++v.active_posts;port=reinterpret_cast<HANDLE>(v.view->scheduler.completion.port);
        }
        BOOL posted=FALSE;
        try{posted=v.options.imports.post(port,0,v.wake(),nullptr);}
        catch(...){std::lock_guard lock(v.gate);--v.active_posts;v.phase=Phase::Poisoned;v.posts_done.notify_all();throw;}
        {std::lock_guard lock(v.gate);--v.active_posts;if(!posted)v.phase=Phase::Poisoned;v.posts_done.notify_all();return posted&&v.phase!=Phase::Poisoned;}
    }catch(...){v.poison();throw;}
}
Snapshot Stop::snapshot()const{
    PreserveError error;auto& v=*impl_;std::lock_guard lock(v.gate);std::size_t receipts=0;for(const auto& e:v.entries)receipts+=e.observed&&!e.ack;
    if(v.route&&!v.route->drop)++receipts;
    return {v.phase,v.cancel_calls,v.not_found,v.kernel_packets,v.software_packets,v.acknowledged,receipts,v.active_posts};
}
Scope Stop::scope()const noexcept{return impl_->options.scope;}
void Stop::bind(o::ServerView view,s::OwnedAcceptMemory& accept,bool prepared_jobs_managed){
    PreserveError error;auto& v=*impl_;
    {std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh)throw std::logic_error("Stop binding admission is closed");}
    if(v.options.scope==Scope::SessionIoPrepared&&!prepared_jobs_managed)throw std::invalid_argument("Prepared SessionIo requires exact Runtime prepared-job management");
    (void)view.memory.buffer_context();
    if(!view.scheduler.completion.port||accept.storage_bindings().size()>v.options.capacity)throw std::invalid_argument("Accept stop requires matching live port and adequate receipt capacity");
    std::lock_guard lock(v.gate);if(v.phase!=Phase::Fresh)throw std::logic_error("Accept stop binds once");
    v.worker=::GetCurrentThreadId();v.fiber=::GetCurrentFiber();v.slot=view.memory.memory_domain_identity()->native_fls_index();v.ptd=::FlsGetValue(v.slot);
    if(!v.ptd||v.ptd==crt_ptd_recovered::construction_sentinel())throw std::logic_error("Accept stop requires an admitted Domain PTD");
    v.view.emplace(view);v.accept=&accept;v.phase=Phase::Bound;
}
void Stop::running(){PreserveError error;auto& v=*impl_;Impl::Call call(v);std::lock_guard lock(v.gate);if(v.phase!=Phase::Bound)throw std::logic_error("Accept stop starts once");v.phase=Phase::Running;}
void Stop::pump(){
    PreserveError error;auto& v=*impl_;Impl::Call call(v);
    try{
        if(!v.stopping())return;
        {std::lock_guard lock(v.gate);if(v.phase!=Phase::Requested&&v.phase!=Phase::Draining)throw std::logic_error("Stop admission was poisoned before cancellation");v.phase=Phase::Draining;}
        for(const auto& loan:v.pending()){
            if(loan.state==s::AcceptSubmission::Issuing||loan.state==s::AcceptSubmission::Unknown)throw std::logic_error("Cannot cancel an unresolved SDK submission");
            if(loan.state!=s::AcceptSubmission::Kernel)continue;
            if(loan.listener==s::invalid_socket||!loan.overlapped)throw std::logic_error("Kernel accept loan lacks exact SDK operands");
            if(loan.family!=Impl::Family::Accept&&!loan.source_control)throw std::logic_error("Kernel session IO has no captured source owner");
            auto& e=v.entry(loan);if(e.cancelled)continue;
            if(loan.family==Impl::Family::Accept)v.accept->note_cancel_requested({loan.handle.slot,loan.handle.generation});
            else v.view->io.note_cancel_requested(loan.handle);e.cancelled=true;
            v.checkpoint();
            {std::lock_guard lock(v.gate);++v.cancel_calls;}
            const auto result=v.options.imports.cancel(reinterpret_cast<HANDLE>(loan.listener),loan.overlapped);
            const auto status=result?0:v.options.imports.error(); // immediate native failure only
            v.checkpoint();const auto after=v.current(loan);
            if(!after.pending||after.submission!=loan.submission||after.overlapped!=loan.overlapped)throw std::logic_error("Cancel SDK changed the owned loan");
            if(!result){if(status!=ERROR_NOT_FOUND)throw std::runtime_error("CancelIoEx failed with an unresolved accept loan");std::lock_guard lock(v.gate);++v.not_found;}
        }
    }catch(...){v.poison();throw;}
}
bool Stop::physical(std::int32_t received,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED* raw,std::uint32_t status){
    PreserveError error;auto& v=*impl_;Impl::Call call(v);
    try{
        if(key==v.wake()){
            if(raw||bytes||!received||status||!v.stopping())throw std::logic_error("Malformed accept stop control wake");
            return true; // runtime pumps then waits again; never a kernel receipt
        }
        v.packet=Impl::Packet{raw,bytes,status,key,received};return false;
    }catch(...){v.poison();throw;}
}
Receipt Stop::before(s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& error,std::uint32_t bytes){
    PreserveError saved;pump();auto& v=*impl_;Impl::Call call(v);
    try{
        auto physical=v.packet;if(physical&&physical->raw==&op.overlapped)v.packet.reset();else physical.reset();
        const bool is_accept=v.accept->contains(&op);
        if(!is_accept&&!v.session_io())return {};
        const auto loan=is_accept?Impl::convert(v.accept->loan(v.accept->handle_for(&op.overlapped))):Impl::convert(v.view->io.loan(v.view->io.handle_for(&op)));
        if(loan.consumed||loan.operation!=&op)throw std::logic_error("Observed accept has stale or consumed ownership");
        bool drop=false;std::uint64_t token=0;
        {std::lock_guard lock(v.gate);
            if(v.phase==Phase::Poisoned||v.route)throw std::logic_error("Accept route is unknown or already active");
            if(v.serial==UINT64_MAX||v.route_epoch==UINT64_MAX)throw std::overflow_error("Accept route sequence exhausted");
            drop=v.phase==Phase::Requested||v.phase==Phase::Draining;
            if(drop)v.phase=Phase::Draining;
            token=++v.serial;++v.route_epoch;v.route=Impl::Route{token,drop};
        }
        if(!drop)return {token,false}; // lease spans normal user/source/after callbacks too
        if(loan.state==s::AcceptSubmission::Kernel){
            if(!loan.pending||!physical||physical->key!=0||physical->status!=std::bit_cast<std::uint32_t>(error.value)||physical->bytes!=bytes||owner!=&v.view->scheduler)
                throw std::logic_error("Kernel accept requires its exact observed physical packet, not a source/control wake");
        }else if(loan.state==s::AcceptSubmission::Issuing||loan.state==s::AcceptSubmission::Unknown)throw std::logic_error("Observed accept submission is unknown");
        auto& e=v.entry(loan);if(e.observed||e.ack)throw std::logic_error("Duplicate accept receipt");
        {std::lock_guard lock(v.gate);
            e.observed=true;e.serial=token;if(loan.state==s::AcceptSubmission::Kernel)++v.kernel_packets;else ++v.software_packets;
        }
        return {e.serial,true}; // observed success also drops accepted socket
    }catch(...){v.poison();throw;}
}
void Stop::after(Receipt receipt){
    PreserveError error;if(!receipt.serial)return;auto& v=*impl_;Impl::Call call(v);
    try{
        {std::lock_guard lock(v.gate);if(!v.route||v.route->serial!=receipt.serial||v.route->drop!=receipt.drop_owner)throw std::logic_error("Unknown or duplicate accept route lease");}
        if(!receipt.drop_owner){
            std::lock_guard lock(v.gate);if(v.route_epoch==UINT64_MAX)throw std::overflow_error("Accept route epoch exhausted");
            ++v.route_epoch;v.route.reset();return;
        }
        Impl::Entry* found=nullptr;for(auto& e:v.entries)if(e.serial==receipt.serial){found=&e;break;}
        if(!found||!found->observed||found->ack||!receipt.drop_owner)throw std::logic_error("Unknown or duplicate source acknowledgement");
        for(const auto& loan:v.pending())if(loan.family==found->loan.family&&loan.handle==found->loan.handle&&loan.submission==found->loan.submission)
            throw std::logic_error("Source completion did not end the observed kernel loan");
        if(found->loan.family!=Impl::Family::Accept)for(const auto& active:v.view->io.active_loans()){
            if(!active.binding_valid){
                // B36 uses an inert base Posted record for actual accept
                // storage. Only its concrete accept owner can authorize this
                // alias; arbitrary public table mutations never become loans.
                if(active.kind!=s::IoMemoryKind::Posted||!v.accept->contains(active.binding_operation))throw std::logic_error("Unowned IO packet alias after source completion");
                const auto h=v.accept->handle_for(active.binding_overlapped);const auto a=v.accept->loan(h);
                if(h.slot!=active.handle.slot||a.operation!=active.binding_operation||a.overlapped!=active.binding_overlapped)throw std::logic_error("Accept reservation alias differs from its actual owner");
                continue;
            }
            const auto loan=Impl::convert(active);
            if(loan.family==found->loan.family&&loan.handle==found->loan.handle&&loan.submission==found->loan.submission)
                throw std::logic_error("Source completion did not retire the observed IO/Posted packet binding");
        }
        std::lock_guard lock(v.gate);if(v.route_epoch==UINT64_MAX)throw std::overflow_error("Accept route epoch exhausted");
        found->ack=true;++v.acknowledged;++v.route_epoch;v.route.reset();
    }catch(...){v.poison();throw;}
}
bool Stop::session_io_pending(){
    PreserveError error;auto& v=*impl_;Impl::Call call(v);
    try{
        if(v.options.scope!=Scope::SessionIoPrepared)throw std::logic_error("Session drain query requires explicit prepared scope");
        if(!v.stopping())return true; // independent stop proof has not been accepted
        const bool loans=!v.pending().empty(),active=v.active_session_io();
        std::lock_guard lock(v.gate);if(v.route)return true;
        for(const auto& entry:v.entries)if(!entry.observed||!entry.ack)return true;
        // Controller posts are drained separately by close_worker outside
        // Parent; waiting for their metadata here could lose the only wake.
        return loans||active;
    }catch(...){v.poison();throw;}
}
bool Stop::source_quiescent(o::ServerView view){
    PreserveError error;auto& v=*impl_;
    // Identity errors by an unrelated actor cannot access or poison this
    // worker's borrowed source graph. Same actor calls remain serialized.
    v.identity();
    try{
        {
            std::lock_guard lock(v.gate);
            if(v.phase==Phase::Poisoned||v.phase==Phase::Closed||v.phase==Phase::Closing||v.phase==Phase::Retiring)
                throw std::logic_error("Source drain requires a live known Stop binding");
            if(!v.view)throw std::logic_error("Source drain requires the bound server");
            const auto& own=*v.view;
            if(view.identity.value!=own.identity.value||&view.backend!=&own.backend||&view.memory!=&own.memory||
               &view.io!=&own.io||&view.scheduler!=&own.scheduler||&view.service!=&own.service||
               &view.acceptor!=&own.acceptor||&view.pending!=&own.pending)
                throw std::invalid_argument("Source drain proof belongs to another server");
            if((v.phase!=Phase::Requested&&v.phase!=Phase::Draining)||v.working||v.control_busy||v.route||v.active_posts)return false;
        }
        // Enter only after releasing Stop metadata; this also rejects a caller
        // holding Parent. No SDK cancellation, waits or opaque calls occur.
        {auto operation=view.memory.enter();
            if(!v.pending().empty()||(v.options.scope==Scope::SessionIoPrepared&&v.active_session_io()))return false;
        }
        std::lock_guard lock(v.gate);
        if((v.phase!=Phase::Requested&&v.phase!=Phase::Draining)||v.working||v.control_busy||v.route||v.active_posts)return false;
        for(const auto& entry:v.entries)if(!entry.observed||!entry.ack)return false;
        return true;
    }catch(...){v.poison();throw;}
}
bool Stop::close_worker(){
    PreserveError error;auto& v=*impl_;
    {std::lock_guard lock(v.gate);if(v.phase==Phase::Closed)return true;if((v.phase!=Phase::Draining&&v.phase!=Phase::Requested)||v.working||v.route)return false;}
    v.identity();
    {auto operation=v.view->memory.enter();if(!v.pending().empty()||(v.options.scope==Scope::SessionIoPrepared&&v.active_session_io()))return false;}
    std::unique_lock lock(v.gate);if((v.phase!=Phase::Draining&&v.phase!=Phase::Requested)||v.working||v.route)return false;
    for(const auto& e:v.entries)if(!e.observed||!e.ack)return false;
    v.phase=Phase::Closing;
    if(!v.posts_done.wait_for(lock,std::chrono::milliseconds(v.options.post_drain_timeout_ms),[&]{return !v.active_posts;})){v.phase=Phase::Poisoned;return false;}
    if(v.phase!=Phase::Closing)return false;
    v.packet.reset();v.accept=nullptr;v.view.reset();v.phase=Phase::Closed;return true;
}
} // namespace anadius::accept_stop
