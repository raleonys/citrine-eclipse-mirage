#include "profile.hpp"
#include "../anadius-native-profile-source/native.hpp"
#include "../anadius-owned-deque-source/deque.hpp"
#include "../anadius-owned-environment-source/environment.hpp"
#include "../anadius-owned-jobs-source/jobs.hpp"
#include "../anadius-session-policy-source/policy.hpp"
#include <algorithm>
#include <exception>
#include <limits>
#include <mutex>
#include <type_traits>

namespace anadius::session_profile {
namespace {
struct PreserveError {DWORD saved=::GetLastError();~PreserveError(){::SetLastError(saved);}};
void add_reference(std::atomic<std::uint32_t>& counter,bool require_live){
    auto value=counter.load();
    for(;;){if((require_live&&!value)||value==std::numeric_limits<std::uint32_t>::max())throw std::overflow_error("Session reference cannot be acquired");if(counter.compare_exchange_weak(value,value+1))return;}
}
void drop_strong(s::IoReference* p){if(p&&p->strong_08.fetch_sub(1)==1){p->dispose_slot_00();if(p->weak_0c.fetch_sub(1)==1)p->delete_slot_08();}}
struct DispatchState {std::mutex gate;Application* target=nullptr;std::size_t active=0;};
}
struct Profile::State {
    mutable std::mutex gate;Phase phase=Phase::Fresh;
    std::size_t live_sessions=0,live_jobs=0,active_calls=0;
    bool mutation=false,profile_call=false,application_live=false;
    DWORD thread=0;void* fiber=nullptr;
    o::OwnedServerBackend* backend=nullptr;s::SessionMemoryDomain* memory=nullptr;s::OwnedIoMemory* io=nullptr;
    s::SocketService* service=nullptr;s::SocketObject* acceptor=nullptr;s::SocketObject* pending_socket=nullptr;s::IocpSchedulerEntry* scheduler=nullptr;
    std::uintptr_t context=0,server=0;Application* application=nullptr;
    std::unique_ptr<Application> pending_application;
    std::shared_ptr<owned_environment::Environment> owned_environment;
    std::shared_ptr<owned_jobs::InlineJobs> owned_jobs;
    Dependencies deps;
    std::vector<s::IoEnvironmentSourceBinding> sources;
    s::IoEnvironmentConstructorContext constructor;
    explicit State(Dependencies d,std::shared_ptr<anadius::owned_environment::Environment> env,std::shared_ptr<anadius::owned_jobs::InlineJobs> jobs):owned_environment(std::move(env)),owned_jobs(std::move(jobs)),deps(std::move(d)),sources(deps.sessions),
        constructor{*deps.environment.types,deps.environment.queries,deps.environment.factories}{}
    Snapshot snapshot()const{PreserveError error;std::lock_guard lock(gate);return {phase,live_sessions,live_jobs,active_calls};}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Retiring)phase=Phase::Poisoned;}
    void match(const o::ServerView& v)const{
        if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Profile requires its bound worker thread/fiber");
        if(backend!=&v.backend||memory!=&v.memory||io!=&v.io||context!=v.identity.value||
           service!=&v.service||acceptor!=&v.acceptor||pending_socket!=&v.pending||scheduler!=&v.scheduler)
            throw std::invalid_argument("Profile belongs to a different owned server context");
    }
};
struct Application::Impl {
    enum class SlotPhase {Empty,Binding,Ready,Retiring};
    struct Slot {
        SlotPhase phase=SlotPhase::Empty;std::uint64_t generation=0;
        s::SessionOwner* owner=nullptr;s::IoReference* observer=nullptr;std::size_t jobs=0;bool processing=false;
        s::ReceiveSocketImplementation receive;s::SendSocketImplementation send;
        s::IoHandlerEnvironment environment;s::QueuedWriteOwner queued;s::MessageDispatchOwner message;
        std::unique_ptr<SessionLogic> logic;
    };
    enum class JobPhase {Empty,Allocated,Submitting,Pending,Running,Done,Unknown};
    struct Job {
        JobPhase phase=JobPhase::Empty;std::uint64_t generation=0;std::unique_ptr<s::FrameMessageClosure> closure;
        SessionToken session;s::IoReference* strong=nullptr;s::FrameThreadHandle handle;bool detached=false,start_returned=false,detaching=false;
    };
    Application* owner=nullptr;const std::shared_ptr<Profile::State> state;const rt::Ports ports;
    std::vector<Slot> slots;std::vector<Job> jobs;
    std::vector<s::FrameReadOwnerBinding> frames;std::vector<s::ReceiveStringBinding> string_bindings;
    std::vector<s::ReceiveStreamBinding> reads;std::vector<s::SendStreamBinding> writes;
    std::vector<s::WriteFinishOwnerBinding> finish_owners;std::vector<s::QueuedWriteOwnerBinding> queued_owners;
    std::vector<s::MessageDispatchOwnerBinding> message_owners;std::vector<s::MessageClosureBinding> closures;
    Strings strings;std::unique_ptr<owned_deque::Deques> deques;
    std::shared_ptr<DispatchState> dispatch=std::make_shared<DispatchState>();
    s::AsyncReceiveService receive_service;s::AsyncSendService send_service;s::OwnedNoopControlFactory controls;
    template<class F>std::invoke_result_t<F> effect(F&& fn,bool cleanup=false){
        check(cleanup);{PreserveError error;std::lock_guard lock(state->gate);++state->active_calls;}
        struct Leave {Profile::State& s;~Leave(){PreserveError error;std::lock_guard lock(s.gate);--s.active_calls;}} leave{*state};
        try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){std::forward<F>(fn)();check(cleanup);}
            else{decltype(auto) result=std::forward<F>(fn)();check(cleanup);return result;}}
        catch(...){state->poison();throw;}
    }
    struct Mutation {
        Impl& v;explicit Mutation(Impl& v,bool cleanup=false):v(v){v.check(cleanup);PreserveError error;std::lock_guard lock(v.state->gate);
            if(v.state->mutation||v.state->active_calls){v.state->phase=Phase::Poisoned;throw std::logic_error("Reentrant session registry mutation");}v.state->mutation=true;}
        ~Mutation(){PreserveError error;std::lock_guard lock(v.state->gate);v.state->mutation=false;}
    };
    void metadata(bool cleanup=false)const{
        PreserveError error;
        {std::lock_guard lock(state->gate);
            if(state->phase!=Phase::Open&&!(cleanup&&state->phase==Phase::Closing))throw std::logic_error("Session application is closed or poisoned");
            if(state->thread!=::GetCurrentThreadId()||state->fiber!=::GetCurrentFiber())throw std::logic_error("Session profile requires its bound worker thread/fiber");}
    }
    void check(bool cleanup=false)const{
        PreserveError error;metadata(cleanup);(void)ports.view.memory.buffer_context();
        const auto backend=ports.view.backend.snapshot().phase;
        if((backend!=o::Phase::Ready&&backend!=o::Phase::Running&&!(cleanup&&(backend==o::Phase::Returned||backend==o::Phase::Quiescing)))||
           ports.view.io.snapshot().phase!=s::IoMemoryPhase::Open||ports.accept.snapshot().poisoned)
            throw std::logic_error("Session application source admission was poisoned");
    }
    struct Category final:s::WriteFinishCategory {
        Impl& v;s::WriteFinishCategory& lower;Category(Impl& v,s::WriteFinishCategory& c):v(v),lower(c){identity_08=c.identity_08;}
        void message_slot_10(s::WriteFinishString& out,std::int32_t code)override{v.effect([&]{lower.message_slot_10(out,code);v.strings.track(out);});}
    };
    std::vector<std::unique_ptr<Category>> category_objects;std::vector<s::WriteFinishCategoryBinding> categories;
    struct Logging final:s::RecoveredQueuedWriteApi {
        Impl& v;Logging(Impl& v,s::QueuedWriteContext& c):RecoveredQueuedWriteApi(c),v(v){}
        void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{v.effect([&]{v.strings.free_sized(p,n);},true);}
        [[noreturn]]void invalid_parameter_05c974()override{v.check();v.state->poison();v.state->deps.policy->logging().invalid_parameter_05c974();std::terminate();}
        s::WriteFinishStream& begin_log_006880(std::uintptr_t p,std::uintptr_t q)override{return v.effect([&]() -> s::WriteFinishStream&{return v.state->deps.policy->logging().begin_log_006880(p,q);});}
        s::WriteFinishStream& append_007990(s::WriteFinishStream& s,std::uintptr_t p,std::uint64_t n)override{return v.effect([&]() -> s::WriteFinishStream&{return v.state->deps.policy->logging().append_007990(s,p,n);});}
        std::uint64_t widen_01e510(std::uintptr_t p,std::uint8_t n)override{return v.effect([&]{return v.state->deps.policy->logging().widen_01e510(p,n);});}
        void put_00fe90(s::WriteFinishStream& s,std::uint8_t n)override{v.effect([&]{v.state->deps.policy->logging().put_00fe90(s,n);});}
        void flush_00e1e0(s::WriteFinishStream& s)override{v.effect([&]{v.state->deps.policy->logging().flush_00e1e0(s);});}
    };
    struct Receive final:s::AsyncReceiveApi {Impl& v;explicit Receive(Impl& v):v(v){}
        s::AsyncReceiveResult receive(s::Socket p,s::Buffer& b,std::uint32_t n,OVERLAPPED& o)override{return v.effect([&]{return v.state->deps.receive->receive(p,b,n,o);});}
    } raw_receive{*this};
    struct Send final:s::AsyncSendApi {Impl& v;explicit Send(Impl& v):v(v){}
        s::AsyncSendResult send(s::Socket p,s::Buffer& b,std::uint32_t n,OVERLAPPED& o)override{return v.effect([&]{return v.state->deps.send->send(p,b,n,o);});}
    } raw_send{*this};
    struct Worker final:rt::WorkerControl {Impl& v;explicit Worker(Impl& v):v(v){}
        std::uint32_t exchange_stop(s::IocpSchedulerEntry& s,std::uint32_t n)override{return v.effect([&]{return v.state->deps.policy->exchange_stop(s,n);});}
        std::uint32_t exchange_wake(s::IocpSchedulerEntry& s,std::uint32_t n)override{return v.effect([&]{return v.state->deps.policy->exchange_wake(s,n);});}
        void initialize(std::array<std::uintptr_t,2>& s)override{v.effect([&]{v.state->deps.policy->initialize(s);});}
        void finish(std::array<std::uintptr_t,2>& s)override{v.effect([&]{v.state->deps.policy->finish(s);});}
        [[noreturn]]void raise(s::Error e,std::uintptr_t p)override{v.check();v.state->poison();v.state->deps.policy->raise(e,p);std::terminate();}
    } worker{*this};
    struct Guard final:s::DispatcherEffects {Impl& v;explicit Guard(Impl& v):v(v){}
        s::DispatcherEffects& lower(){return v.state->deps.policy->guards();}
        std::uintptr_t capture_tls_slot()override{return v.effect([&]{return lower().capture_tls_slot();});}
        std::int32_t thread_epoch(std::uintptr_t p)override{return v.effect([&]{return lower().thread_epoch(p);});}
        void guard_header_04b838(std::int32_t& n)override{v.effect([&]{lower().guard_header_04b838(n);});}
        void register_atexit_04b770(std::uintptr_t p)override{v.effect([&]{lower().register_atexit_04b770(p);});}
        void guard_footer_04b7cc(std::int32_t& n)override{v.effect([&]{lower().guard_footer_04b7cc(n);});}
        std::uint32_t read_stop_34(s::IocpSchedulerEntry& e)override{return v.effect([&]{return lower().read_stop_34(e);});}
        std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>& a,const std::array<std::uintptr_t,2>& b)override{return v.effect([&]() -> std::array<std::uintptr_t,2>&{return lower().copy_exception_024100(a,b);});}
        [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>& a)override{v.check();v.state->poison();lower().rethrow_029880(a);std::terminate();}
        void complete(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override{throw std::logic_error("Runtime owns actual coherent routing");}
    } guard{*this};
    struct Terminal final:s::FrameReadTerminal,s::QueuedWriteTerminal {Impl& v;explicit Terminal(Impl& v):v(v){}
        [[noreturn]]void bad_weak_ptr_026860()override{v.check();v.state->poison();v.state->deps.policy->bad_weak_ptr_026860();std::terminate();}
    } terminal{*this};
    s::QueuedWriteContext queued;Logging logging;
    s::PinnedAsyncReceiveApi pinned_receive;s::PinnedAsyncSendApi pinned_send;
    struct Effects final:s::RecoveredComposedReadEffects {
        Impl& v;
        explicit Effects(Impl& v):RecoveredComposedReadEffects(v.ports.environment,v.ports.worker,v.ports.memory,
            v.state->deps.environment.handlers,v.pinned_send,v.ports.completion,*v.state->deps.errors,v.writes,
            v.state->constructor,v.state->sources,v.logging,v.finish_owners,v.categories,v.strings.bindings()),v(v){}
        s::AsyncSendOperation& project_async_send(s::OperationStorage& s)override{return v.effect([&]() -> s::AsyncSendOperation&{return v.ports.memory.project_async_send(s);});}
        s::Category misc_category_0285b0()override{return v.state->deps.misc;}
    } effects;
    struct Frame final:s::FrameReadApi {
        Impl& v;explicit Frame(Impl& v):v(v){}
        void construct_message_006b30(s::ReceiveBufferString& out,std::uintptr_t p,std::uint64_t n)override{v.effect([&]{const auto b=v.ports.view.memory.buffer_context().memory.view(p,n);v.strings.assign(out,{reinterpret_cast<const char*>(b.data()),b.size()});});}
        s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t n)override;
        std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,s::FrameMessageClosure*,std::uint32_t,std::uint32_t&)override;
        std::int32_t detach_04a458(const s::FrameThreadHandle&)override;
        [[noreturn]]void thread_error_04a648(int n)override{v.check();v.state->poison();v.state->deps.policy->thread_error(n);std::terminate();}
        void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override;
        [[noreturn]]void invalid_parameter_05c974()override{v.logging.invalid_parameter_05c974();std::terminate();}
        void next_write_02a500(std::uintptr_t p)override{v.logging.next_write_02a500(p);}
        s::WriteFinishStream& begin_log_006880(std::uintptr_t p,std::uintptr_t q)override{return v.logging.begin_log_006880(p,q);}
        s::WriteFinishStream& append_007990(s::WriteFinishStream& s,std::uintptr_t p,std::uint64_t n)override{return v.logging.append_007990(s,p,n);}
        std::uint64_t widen_01e510(std::uintptr_t p,std::uint8_t n)override{return v.logging.widen_01e510(p,n);}
        void put_00fe90(s::WriteFinishStream& s,std::uint8_t n)override{v.logging.put_00fe90(s,n);}
        void flush_00e1e0(s::WriteFinishStream& s)override{v.logging.flush_00e1e0(s);}
    } frame_api{*this};
    s::FrameReadContext frame;
    struct Read final:s::RecoveredFrameReadApi {Impl& v;Read(Impl& v,s::FrameReadContext& f):RecoveredFrameReadApi(f),v(v){}
        void construct_length_error_024390(s::ReadLengthError& e,std::uintptr_t p)override{v.effect([&]{v.state->deps.policy->construct_length_error(e,p);});}
        [[noreturn]]void throw_length_error_023930(s::ReadLengthError& e)override{v.check();v.state->poison();v.state->deps.policy->throw_length_error(e);std::terminate();}
    } read_api;
    s::AsyncReceiveContext receive_context;s::ComposedReadContext read;
    s::OwnedResponseContext owned_response;
    struct Response final:s::RecoveredOwnedResponseApi {Impl& v;Response(Impl& v,s::OwnedResponseContext& c):RecoveredOwnedResponseApi(c),v(v){}
        s::ReceiveBufferString& copy_response_009eb0(s::ReceiveBufferString& out,s::ReceiveBufferString& from)override{return v.effect([&]() -> s::ReceiveBufferString&{return v.strings.copy(out,from);});}
    } response_api;
    struct Queue final:s::ResponseQueueApi {Impl& v;explicit Queue(Impl& v):v(v){}
        void grow_deque_026670(s::WriteFinishOwner& o,std::uint64_t n)override{v.effect([&]{v.deques->grow(o,n,[&v=v]{v.check();});});}
        s::WriteFinishString* allocate_string_04b0bc(std::uint64_t n)override{return v.effect([&]{if(n!=32)throw s::UnsupportedInput("Output string projection requires32bytes");return v.ports.view.memory.allocate_output_block();});}
    } queue_api{*this};
    s::ResponseQueueContext queue;s::ResponseDispatchContext responses;
    struct Message final:s::RecoveredResponseDispatchApi {Impl& v;Message(Impl& v,s::ResponseDispatchContext& c):RecoveredResponseDispatchApi(c),v(v){}
        void process_message_033d70(std::uintptr_t,s::ReceiveBufferString&,s::ReceiveBufferString&)override;
        void destroy_empty_message_00aa50(s::ReceiveBufferString& s)override{v.effect([&]{v.strings.release(s);});}
        void cleanup_exit_waits_04a47c()override{v.effect([&]{v.state->deps.policy->cleanup_exit_waits();});}
    } message_api;
    s::MessageDispatchContext message;
    Impl(std::shared_ptr<Profile::State> s,rt::Ports p):state(std::move(s)),ports(p),slots(state->deps.sessions),jobs(state->deps.jobs_capacity),
        frames(slots.size()),string_bindings(slots.size()),reads(slots.size()),writes(slots.size()),finish_owners(slots.size()),queued_owners(slots.size()),message_owners(slots.size()),closures(jobs.size()),
        strings(ports.view.memory,state->deps.headers),receive_service{&ports.view.scheduler},send_service{&ports.view.scheduler},
        queued{queued_owners,terminal,nullptr},logging(*this,queued),pinned_receive(ports.view.io,raw_receive),pinned_send(ports.view.io,raw_send),
        effects(*this),frame{ports.view.memory.buffer_context(),effects,frame_api,frames,categories,strings.bindings(),terminal,state->deps.success,nullptr},
        read_api(*this,frame),receive_context{ports.environment,ports.worker,ports.memory.posted(),pinned_receive,ports.completion,*state->deps.errors,ports.memory},
        read{ports.view.memory.buffer_context(),receive_context,effects,read_api,string_bindings,reads},
        owned_response{ports.environment.storage,ports.worker,ports.memory,frame,nullptr},response_api(*this,owned_response),
        queue{frame,queue_api,finish_owners,&queued},responses{frame,ports.environment.sentinels,state->deps.environment.response_executors,state->deps.environment.responses,response_api,&queue},
        message_api(*this,responses),message{frame,message_api,message_owners,state->deps.environment.copies,closures}{
        // All containers finish growing before any source publication.
        category_objects.reserve(state->deps.categories.size());categories.reserve(state->deps.categories.size());
        for(const auto& c:state->deps.categories){category_objects.push_back(std::make_unique<Category>(*this,*c.object));categories.push_back({c.key,category_objects.back().get()});}
        frame.categories=categories;effects.finish_context().categories=categories;
        frame.read=&read;effects.bind_read_context(read);queued.write=&effects.write_context();owned_response.response=&responses;
        // Last potentially throwing construction: no mandatory-open provider
        // is unwound by a later context/category allocation failure.
        deques=std::make_unique<owned_deque::Deques>(ports.view.memory);
    }
    Slot& slot(SessionToken t){if(t.slot>=slots.size()||slots[t.slot].generation!=t.generation||slots[t.slot].phase!=SlotPhase::Ready)throw std::invalid_argument("Stale session token");return slots[t.slot];}
    std::size_t slot_for(std::uintptr_t id){for(std::size_t i=0;i<slots.size();++i)if(slots[i].phase==SlotPhase::Ready&&slots[i].owner->identity()==id)return i;throw std::invalid_argument("Session owner is not registered");}
    Job& job(JobToken t){if(t.slot>=jobs.size()||jobs[t.slot].generation!=t.generation||jobs[t.slot].phase==JobPhase::Empty)throw std::invalid_argument("Stale frame-job token");return jobs[t.slot];}
    void retire_job(Job& j){
        if(j.phase!=JobPhase::Done||!j.detached||!j.start_returned||j.detaching||j.strong||j.closure)return;
        j.phase=JobPhase::Empty;j.handle={};j.session={};j.detached=false;j.start_returned=false;j.detaching=false;
        PreserveError error;std::lock_guard lock(state->gate);--state->live_jobs;
    }
    void seal_dispatch(){std::lock_guard lock(dispatch->gate);if(dispatch->active)throw std::logic_error("Frame runner invocation is still active");dispatch->target=nullptr;}
    void reap(){
        for(std::size_t i=0;i<slots.size();++i){auto& s=slots[i];if(s.phase!=SlotPhase::Ready||s.jobs||s.processing||s.observer->strong_08.load()!=0)continue;
            auto* const control=s.observer;s.phase=SlotPhase::Retiring;
            frames[i]={};string_bindings[i]={};reads[i]={};writes[i]={};finish_owners[i]={};queued_owners[i]={};message_owners[i]={};state->sources[i]={};
            s.receive={};s.send={};s.queued={};s.message={};
            if(state->owned_environment)state->owned_environment->release(s.environment);
            s.environment={};s.owner=nullptr;
            s.logic.reset();check(true); // unknown destructor reentry retains observer
            s.observer=nullptr;
            {PreserveError error;std::lock_guard lock(state->gate);--state->live_sessions;}
            if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();
            // Never read owner/control after the potentially final weak drop.
            check(true);s.phase=SlotPhase::Empty;
        }
        strings.prune();
    }
};
s::FrameMessageClosure* Application::Impl::Frame::allocate_frame_04b0bc(std::uint64_t n){return v.effect([&]{
    if(n!=48)throw s::UnsupportedInput("Frame closure projection requires48bytes");
    for(std::size_t i=0;i<v.jobs.size();++i){auto& j=v.jobs[i];if(j.phase!=JobPhase::Empty)continue;
        if(j.generation==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Job generation exhausted");
        j.closure=std::make_unique<s::FrameMessageClosure>();++j.generation;j.phase=JobPhase::Allocated;
        v.closures[i]={reinterpret_cast<std::uintptr_t>(j.closure.get()),j.closure.get()};
        {PreserveError error;std::lock_guard lock(v.state->gate);++v.state->live_jobs;}return j.closure.get();}
    throw std::length_error("Fixed frame-job capacity exhausted");
});}
std::uintptr_t Application::Impl::Frame::begin_frame_thread_05e8e0(std::uintptr_t security,std::uint32_t stack,std::uintptr_t entry,s::FrameMessageClosure* closure,std::uint32_t flags,std::uint32_t& id){return v.effect([&]{
    if(security||stack||flags||entry!=0x180021770ULL)throw s::UnsupportedInput("Unsupported frame job entry arguments");
    std::size_t i=0;for(;i<v.jobs.size();++i)if(v.jobs[i].phase==JobPhase::Allocated&&v.jobs[i].closure.get()==closure)break;
    if(i==v.jobs.size())throw std::invalid_argument("Frame closure is not an allocated owned job");
    auto& j=v.jobs[i];const auto index=v.slot_for(closure->owner_20);auto& session=v.slots[index];
    j.session={index,session.generation};add_reference(session.observer->strong_08,true);j.strong=session.observer;++session.jobs;j.phase=JobPhase::Submitting;
    const JobToken token{i,j.generation};const auto dispatch=v.dispatch;
    auto runner=[dispatch,token]{
        Application* target=nullptr;{std::lock_guard lock(dispatch->gate);if(!dispatch->target)throw std::logic_error("Closed frame runner tombstone");target=dispatch->target;++dispatch->active;}
        struct Leave {std::shared_ptr<DispatchState> state;~Leave(){std::lock_guard lock(state->gate);--state->active;}} leave{dispatch};
        target->run_job(token);
    };
    const auto handle=v.state->deps.jobs->start(token,std::move(runner));v.check();
    if(!handle.handle||!handle.id)throw s::UnsupportedInput("Frame backend returned no valid handle/id");
    for(const auto& other:v.jobs)if(&other!=&j&&other.phase!=JobPhase::Empty&&other.start_returned&&other.handle.handle==handle.handle&&other.handle.id==handle.id)
        throw std::invalid_argument("Frame backend reused an outstanding handle/id");
    j.handle=handle;j.start_returned=true;if(j.phase==JobPhase::Submitting)j.phase=JobPhase::Pending;
    id=handle.id;return handle.handle;
});}
std::int32_t Application::Impl::Frame::detach_04a458(const s::FrameThreadHandle& handle){return v.effect([&]{
    Job* found=nullptr;for(auto& j:v.jobs)if(j.phase!=JobPhase::Empty&&j.start_returned&&j.handle.handle==handle.handle&&j.handle.id==handle.id){if(found)throw std::invalid_argument("Ambiguous frame detach handle");found=&j;}
    if(!found||found->detached||found->detaching)throw std::invalid_argument("Frame detach handle is stale, active or already detached");
    found->detaching=true;
    const auto result=v.state->deps.jobs->detach(handle);v.check();found->detaching=false;
    if(!result){found->detached=true;v.retire_job(*found);}return result;
});}
void Application::Impl::Frame::free_sized_04b0f8(std::uintptr_t p,std::uint64_t n){v.effect([&]{
    for(std::size_t i=0;i<v.jobs.size();++i){auto& j=v.jobs[i];if(j.closure&&reinterpret_cast<std::uintptr_t>(j.closure.get())==p){
        if(n!=48||j.phase!=JobPhase::Running)throw s::UnsupportedInput("Closure free requires its running job");
        v.closures[i]={};j.closure.reset();return;}}
    v.strings.free_sized(p,n);
},true);}
void Application::Impl::Message::process_message_033d70(std::uintptr_t handler,s::ReceiveBufferString& response,s::ReceiveBufferString& request){v.effect([&]{
    if(handler<0xf0||&response==&request)throw s::UnsupportedInput("Invalid message handler/prefix identity");
    const auto slot_id=v.slot_for(handler-0xf0);auto& slot=v.slots[slot_id];if(slot.processing)throw std::logic_error("Session message logic reentry");slot.processing=true;
    if(auto* native=dynamic_cast<native_profile::Logic*>(slot.logic.get())){
        if(!native->matches({{slot_id,slot.generation},*slot.owner}))throw std::invalid_argument("Native message logic belongs to a different session generation");
        native->process_native(v.ports.view.memory,v.strings,response,request,[&v=v]{v.check();});slot.processing=false;return;
    }
    const auto original=v.strings.read(request);auto working=original;const auto result=slot.logic->process(working);v.check();
    if(working!=original)v.strings.assign(request,working);v.strings.assign(response,result);slot.processing=false;
});}
Application::Application(std::unique_ptr<Impl> p):impl_(std::move(p)){impl_->owner=this;impl_->dispatch->target=this;}
Application::~Application()noexcept{
    auto& v=*impl_;{PreserveError error;std::lock_guard lock(v.state->gate);if(v.state->phase!=Phase::Closed||v.state->active_calls||v.state->live_sessions||v.state->live_jobs)std::terminate();}
    try{v.seal_dispatch();}catch(...){std::terminate();}
    {PreserveError error;std::lock_guard lock(v.state->gate);v.state->application=nullptr;v.state->application_live=false;}
}
rt::Bindings Application::bindings(){auto& v=*impl_;return v.effect([&]{return rt::Bindings{v.frame,v.logging,v.effects,v.guard,v.worker,v.categories,v.state->deps.environment.accepts,&v.effects.write_context()};});}
void Application::construct_accept_environment(s::IoHandlerEnvironment& out,std::uintptr_t allocator){auto& v=*impl_;v.effect([&]{if(allocator!=v.ports.server_identity+0x68)throw std::invalid_argument("Foreign server allocator token");if(v.state->owned_environment)v.state->owned_environment->accept(out,allocator);else v.state->deps.policy->accept_environment(out,allocator);});}
void Application::reset_pending_owner(s::AcceptPending& pending){auto& v=*impl_;v.effect([&]{if(&pending.object!=&v.ports.view.pending)throw std::invalid_argument("Foreign pending socket");s::replace_owner_0237f0(pending.object.cancellation,0,v.controls);});}
s::ReceiveBufferString& Application::greeting(s::SessionOwner& owner,s::ReceiveBufferString& out){
    auto& v=*impl_;Impl::Mutation mutation(v);
    return v.effect([&]() -> s::ReceiveBufferString&{
        const auto id=owner.identity();auto* const control=owner.weak_self.control_08;
        if(!control||!v.ports.view.memory.owns_control(control)||!control->strong_08.load()||owner.weak_self.object_00!=id)
            throw std::invalid_argument("Only a live factory session from this domain may register");
        if(&static_cast<s::SessionControl*>(control)->object!=&owner)
            throw std::invalid_argument("Session owner is not the registered control's embedded object");
        for(const auto& s:v.slots)if(s.phase!=Impl::SlotPhase::Empty&&s.observer==control)throw std::invalid_argument("Session is already registered");
        std::size_t i=0;for(;i<v.slots.size();++i)if(v.slots[i].phase==Impl::SlotPhase::Empty)break;
        if(i==v.slots.size())throw std::length_error("Fixed session capacity exhausted after native factory prefix");
        auto& slot=v.slots[i];if(slot.generation==std::numeric_limits<std::uint64_t>::max())throw std::overflow_error("Session generation exhausted");
        add_reference(control->weak_0c,false);slot.phase=Impl::SlotPhase::Binding;++slot.generation;slot.owner=&owner;slot.observer=control;
        {PreserveError error;std::lock_guard lock(v.state->gate);++v.state->live_sessions;}
        const SessionView session{{i,slot.generation},owner};
        if(v.state->owned_environment)v.state->owned_environment->session(slot.environment,session);
        else slot.environment=v.state->deps.policy->session_environment(session);
        v.check();
        slot.receive={owner.socket_state.socket,owner.socket_state.socket_state,id,control};slot.send={owner.socket_state.socket,owner.socket_state.socket_state,id,control};
        v.frames[i]={id,&owner.weak_self};v.string_bindings[i]={id+0xa8,&owner.input};
        v.reads[i]={id+0x10,&v.receive_service,&slot.receive};v.writes[i]={id+0x10,&v.send_service,&slot.send,id+0x78};
        v.state->sources[i]={id+0x78,&slot.environment};
        if(owner.output.fields.map_d0.data()||owner.output.fields.map_size_d8||owner.output.fields.count_e8)throw std::invalid_argument("Fresh session output queue required");
        auto map=v.ports.view.memory.allocate_output_map(v.state->deps.initial_output_slots);owner.output.fields={map,map.size(),0,0};
        v.finish_owners[i]={id,&owner.output.fields};slot.queued={id,control,&owner.output.fields,&v.writes[i]};v.queued_owners[i]={id,&slot.queued};
        slot.message={&owner.weak_self,&slot.environment};v.message_owners[i]={id,&slot.message};
        slot.logic=v.state->deps.logic(session);v.check();if(!slot.logic)throw std::invalid_argument("Session logic factory returned null");
        if(auto* native=dynamic_cast<native_profile::Logic*>(slot.logic.get());native&&!native->matches(session))throw std::invalid_argument("Native greeting logic belongs to a different session generation");
        const auto text=dynamic_cast<native_profile::Logic*>(slot.logic.get())?
            static_cast<native_profile::Logic*>(slot.logic.get())->greeting_native(v.ports.view.memory,v.strings,[&v=v]{v.check();}):slot.logic->greeting();
        v.check();v.strings.assign(out,text);slot.phase=Impl::SlotPhase::Ready;return out;
    });
}
void Application::raise_write(s::Error& e,std::uintptr_t p){impl_->check();impl_->state->poison();impl_->state->deps.policy->raise_write(e,p);std::terminate();}
s::IocpSchedulerEntry* Application::completion_owner(s::IocpSchedulerEntry* owner,s::CancelOperation& op,const s::Error& e,std::uint32_t n){auto& v=*impl_;return v.effect([&]{return v.state->deps.policy->completion_owner(*this,owner,op,e,n);});}
void Application::reap(){auto& v=*impl_;Impl::Mutation mutation(v,true);v.effect([&]{v.reap();},true);}
void Application::after_completion(){reap();auto& v=*impl_;v.effect([&]{v.state->deps.policy->after_completion(*this);});reap();}
void Application::worker_returned(std::uint64_t n,const s::WorkerRunResult& r){
    reap();auto& v=*impl_;v.effect([&]{
        {PreserveError error;std::lock_guard lock(v.state->gate);if(v.state->live_jobs)throw std::logic_error("Deferred source jobs must finish before native worker return");}
        v.state->deps.policy->worker_returned(*this,n,r);
    });reap();
}
Snapshot Application::snapshot()const{return impl_->state->snapshot();}
RegistryView Application::registry()const{auto& v=*impl_;v.metadata(true);return {v.frames,v.string_bindings,v.reads,v.writes,v.finish_owners,v.queued_owners,v.message_owners,v.state->sources};}
rt::Ports Application::ports()const{impl_->metadata(true);return impl_->ports;}
SessionView Application::session(SessionToken token)const{auto& v=*impl_;v.check(true);return {token,*v.slot(token).owner};}
void Application::run_job(JobToken token){auto& v=*impl_;v.effect([&]{
    auto& job=v.job(token);if(job.phase!=Impl::JobPhase::Submitting&&job.phase!=Impl::JobPhase::Pending)throw std::logic_error("Frame job cannot execute twice or resume an unknown prefix");
    auto& slot=v.slot(job.session);if(!job.strong||!job.closure)throw std::logic_error("Frame job has no retained owner/closure");
    job.phase=Impl::JobPhase::Running;
    try{if(s::message_thread_entry_021770(*job.closure,v.message)!=0)throw std::logic_error("Unexpected message entry result");}
    catch(...){job.phase=Impl::JobPhase::Unknown;throw;}
    if(job.closure)throw std::logic_error("Normal message entry did not release its closure");
    auto* const strong=job.strong;job.strong=nullptr;--slot.jobs;job.phase=Impl::JobPhase::Done;
    drop_strong(strong);v.check();v.retire_job(job);
});}
Profile::Profile(Dependencies d){
    const auto& e=d.environment;const auto& sentinels=e.sentinels;
    const bool empty=!e.types&&!sentinels.empty_operations_846c8&&!sentinels.empty_schedule_846e8&&!sentinels.empty_executor_84770&&!sentinels.guard_destroy_017a30&&
        e.queries.empty()&&e.factories.empty()&&e.handlers.empty()&&e.response_executors.empty()&&e.responses.empty()&&e.copies.empty()&&e.accepts.empty();
    std::shared_ptr<anadius::owned_environment::Environment> owned;
    if(empty){owned=std::make_shared<anadius::owned_environment::Environment>(d.environment_capacity);d.environment=owned->configuration();}
    if(!d.receive){if(!d.receive_imports.receive||!d.receive_imports.get_error)throw std::invalid_argument("Complete default receive SDK imports required");d.receive=std::make_shared<s::NativeAsyncReceiveApi>(d.receive_imports);}
    if(!d.send){if(!d.send_imports.send||!d.send_imports.get_error)throw std::invalid_argument("Complete default send SDK imports required");d.send=std::make_shared<s::NativeAsyncSendApi>(d.send_imports);}
    std::shared_ptr<anadius::owned_jobs::InlineJobs> jobs;
    if(!d.jobs){jobs=std::make_shared<anadius::owned_jobs::InlineJobs>(d.jobs_capacity);d.jobs=jobs;}
    if(!d.errors||!d.receive||!d.send||!d.policy||!d.jobs||!d.logic||!d.environment.types||!d.environment.sentinels.empty_operations_846c8||
       !d.sessions||d.sessions>65536||!d.jobs_capacity||d.jobs_capacity>65536||!d.headers||d.headers>1048576||
       d.initial_output_slots<8||(d.initial_output_slots&(d.initial_output_slots-1))||d.initial_output_slots>1048576)
        throw std::invalid_argument("Complete session profile providers and bounded capacities required");
    for(std::size_t i=0;i<d.categories.size();++i){if(!d.categories[i].object)throw std::invalid_argument("Category provider is null");
        for(std::size_t j=0;j<i;++j)if(d.categories[i].key.name_identity==d.categories[j].key.name_identity&&d.categories[i].key.logical==d.categories[j].key.logical)throw std::invalid_argument("Category binding is ambiguous");}
    state_=std::make_shared<State>(std::move(d),std::move(owned),std::move(jobs));
}
Profile::~Profile()noexcept{
    auto& s=*state_;{PreserveError error;std::lock_guard lock(s.gate);if(s.application_live||s.active_calls||s.profile_call)std::terminate();s.phase=Phase::Retiring;}
    {Dependencies retiring=std::move(s.deps);s.deps.logic={};} // clear moved-from SBO target while metadata is alive, outside mutex
}
rt::Environment Profile::environment(o::ServerView view,std::uintptr_t server){
    auto& s=*state_;PreserveError error;
    {std::lock_guard lock(s.gate);if(s.profile_call||s.active_calls||s.mutation){if(s.phase!=Phase::Retiring)s.phase=Phase::Poisoned;throw std::logic_error("Reentrant profile environment binding");}if(s.phase!=Phase::Fresh)throw std::logic_error("Profile environment binds once");}
    (void)view.memory.buffer_context();if(!server)throw std::invalid_argument("Stable server identity required");
    const auto backend=view.backend.snapshot().phase;
    if((backend!=o::Phase::Ready&&backend!=o::Phase::Running)||view.io.snapshot().phase!=s::IoMemoryPhase::Open)throw std::logic_error("Profile requires an admissible owned backend");
    {std::lock_guard lock(s.gate);s.backend=&view.backend;s.memory=&view.memory;s.io=&view.io;s.service=&view.service;s.acceptor=&view.acceptor;s.pending_socket=&view.pending;s.scheduler=&view.scheduler;s.context=view.identity.value;s.server=server;s.thread=::GetCurrentThreadId();s.fiber=::GetCurrentFiber();s.phase=Phase::Bound;}
    return {s.deps.environment.sentinels,s.constructor,s.sources};
}
std::unique_ptr<rt::Application> Profile::create(rt::Ports ports){
    auto& s=*state_;{PreserveError error;std::lock_guard lock(s.gate);if(s.profile_call||s.active_calls||s.mutation){if(s.phase!=Phase::Retiring)s.phase=Phase::Poisoned;throw std::logic_error("Reentrant profile application binding");}if(s.phase!=Phase::Bound||s.application_live)throw std::logic_error("Profile application binds once");}
    s.match(ports.view);(void)ports.view.memory.buffer_context();if(ports.server_identity!=s.server)throw std::invalid_argument("Foreign profile server identity");
    const auto storage=ports.accept.storage_bindings();const auto& sentinels=s.deps.environment.sentinels;
    if(ports.environment.storage.storage.data()!=storage.data()||ports.environment.storage.storage.size()!=storage.size()||
       ports.environment.sentinels.empty_operations_846c8!=sentinels.empty_operations_846c8||ports.environment.sentinels.empty_schedule_846e8!=sentinels.empty_schedule_846e8||
       ports.environment.sentinels.empty_executor_84770!=sentinels.empty_executor_84770||ports.environment.sentinels.guard_destroy_017a30!=sentinels.guard_destroy_017a30)
        throw std::invalid_argument("Profile requires the exact owned storage and sentinel projections");
    {PreserveError error;std::lock_guard lock(s.gate);s.profile_call=true;s.application_live=true;}
    struct Leave {State& s;~Leave(){PreserveError error;std::lock_guard lock(s.gate);s.profile_call=false;}} leave{s};
    try{s.pending_application=std::unique_ptr<Application>(new Application(std::make_unique<Application::Impl>(state_,ports)));
        // The completed pending Application owns its mandatory-close providers
        // before a potentially throwing environment bind. Unknown bind prefixes
        // retain this application instead of unwinding its graph.
        if(s.owned_environment){auto* impl=s.pending_application->impl_.get();s.owned_environment->bind(ports,[impl]{impl->check(true);});}
        if(s.owned_jobs){auto* impl=s.pending_application->impl_.get();s.owned_jobs->bind(ports,[impl]{impl->check(true);});}
        if(auto* policy=dynamic_cast<session_policy::Policy*>(s.deps.policy.get())){auto* impl=s.pending_application->impl_.get();policy->bind(ports,[impl]{impl->check(true);});}
        {PreserveError error;std::lock_guard lock(s.gate);if(s.phase!=Phase::Bound)throw std::logic_error("Profile binding was poisoned");s.application=s.pending_application.get();s.phase=Phase::Open;}
        return std::move(s.pending_application);
    }catch(...){if(!s.pending_application){PreserveError error;std::lock_guard lock(s.gate);s.application_live=false;}s.poison();throw;}
}
bool Profile::external_quiescent(o::ServerView view,rt::Application& application){
    auto& s=*state_;{PreserveError error;std::lock_guard lock(s.gate);
        if((s.phase!=Phase::Open&&s.phase!=Phase::Closing)||s.application!=&application||s.profile_call||s.active_calls||s.mutation){if(s.phase!=Phase::Retiring)s.phase=Phase::Poisoned;throw std::logic_error("Invalid/reentrant profile quiescence");}
        s.profile_call=true;s.phase=Phase::Closing;}
    struct Leave {State& s;~Leave(){PreserveError error;std::lock_guard lock(s.gate);s.profile_call=false;}} leave{s};
    try{s.match(view);auto& app=static_cast<Application&>(application);
        const auto initial_backend=view.backend.snapshot().phase;
        if((initial_backend!=o::Phase::Returned&&initial_backend!=o::Phase::Quiescing)||view.memory.snapshot().phase!=s::SessionMemoryPhase::Open||view.io.snapshot().phase!=s::IoMemoryPhase::Open)
            throw std::logic_error("External proof requires returned/closing source admission");
        {auto probe=view.memory.enter();} // succeeds only outside any existing/mutating parent Operation
        const auto external=s.deps.policy->external_quiescent(view,app);
        {PreserveError error;std::lock_guard lock(s.gate);if(s.phase!=Phase::Closing)throw std::logic_error("Profile quiescence was poisoned");}
        const auto backend=view.backend.snapshot().phase;
        if((backend!=o::Phase::Returned&&backend!=o::Phase::Quiescing)||view.memory.snapshot().phase!=s::SessionMemoryPhase::Open||view.io.snapshot().phase!=s::IoMemoryPhase::Open)
            throw std::logic_error("External quiescence poisoned source admission");
        if(!external)return false;
        {auto operation=view.memory.enter();app.reap();}
        {PreserveError error;std::lock_guard lock(s.gate);if(s.live_sessions||s.live_jobs||s.active_calls)return false;}
        app.impl_->seal_dispatch();
        if(s.owned_jobs){auto operation=view.memory.enter();if(!s.owned_jobs->close(true))return false;}
        if(s.owned_environment){auto operation=view.memory.enter();if(!s.owned_environment->close(true))return false;}
        {auto operation=view.memory.enter();app.impl_->deques->close();}
        if(auto* policy=dynamic_cast<session_policy::Policy*>(s.deps.policy.get())){auto operation=view.memory.enter();if(!policy->close(true))return false;}
        {PreserveError error;std::lock_guard lock(s.gate);if(s.phase!=Phase::Closing)throw std::logic_error("Profile retirement was poisoned");s.phase=Phase::Closed;}
        return true;
    }catch(...){s.poison();throw;}
}
Snapshot Profile::snapshot()const{return state_->snapshot();}
} // namespace anadius::session_profile
