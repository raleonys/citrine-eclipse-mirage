#include "graph.hpp"
#include "../anadius-messages-source/control.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include "../anadius-event-queue-source/queue.hpp"
#include "../anadius-config-iteration-source/thread_error.hpp"
#include "../anadius-socket-source/worker_run_native.hpp"
#include "../anadius-information-effects-source/effects.hpp"
#include "../anadius-content-effects-source/effects.hpp"
#include "../anadius-licensing-effects-source/effects.hpp"
#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <exception>
#include <limits>
#include <list>
#include <mutex>
#include <type_traits>
#include <type_traits>

namespace anadius::application_graph {
namespace m=messages_recovered;namespace ch=content_handlers_recovered;
namespace {
struct PreserveError {DWORD saved=::GetLastError();~PreserveError(){::SetLastError(saved);}};
[[noreturn]]void reject(std::unique_lock<std::mutex>& lock,const char* text){lock.unlock();throw std::logic_error(text);}
struct Backing {std::shared_ptr<Graph> graph;std::shared_ptr<void> prior;};
}
struct Graph::Impl:std::enable_shared_from_this<Impl> {
    Options owned_options;Options& options;DWORD thread=::GetCurrentThreadId();void* fiber=::GetCurrentFiber();
    mutable std::mutex metadata;Phase phase=Phase::Fresh;bool active=false,router_active=false,server_active=false,in_opaque=false;
    std::weak_ptr<Graph> facade;std::weak_ptr<sp::Profile> profile;std::weak_ptr<session_policy::Policy> policy;
    o::OwnedServerBackend* backend=nullptr;std::int32_t epoch=0;
    s::SessionMemoryDomain* parent=nullptr;sp::Strings* strings=nullptr;std::shared_ptr<void> lease;
    struct Row {sp::SessionToken token;np::Logic* logic=nullptr;};std::vector<Row> rows;std::size_t live=0;
    struct MessageCall;MessageCall* message=nullptr;
    struct Admission {Admission* next;DWORD thread;void* fiber;s::ReceiveBufferString* output; s::ReceiveBufferString* request;np::Logic* logic;sp::SessionToken token;};
    Admission* admissions=nullptr;std::list<std::shared_ptr<MessageCall>> calls;
    std::shared_ptr<information_effects::Information::CallView> message_information;
    std::shared_ptr<content_effects::Content::CallView> message_content;
    std::shared_ptr<licensing_effects::Handlers::CallView> message_licensing;
    std::shared_ptr<xml_reader_bridge::Backend> xml=std::make_shared<xml_reader_bridge::Backend>();
    std::shared_ptr<s::SessionCipherFactory> cipher=std::make_shared<s::SessionCipherFactory>();
    std::shared_ptr<crypto_recovered::TimeSource> clock;
    struct Clock final:crypto_recovered::TimeSource {Impl& v;explicit Clock(Impl& v):v(v){}std::int64_t unix_seconds_05eeb8()override;};
    std::shared_ptr<Clock> guarded_clock;
    std::shared_ptr<crypto_recovered::Random> random;
    struct Server;std::unique_ptr<Server> pending_server;
    std::vector<std::unique_ptr<ch::DocumentWriter>> writers;
    std::vector<s::ReceiveBufferString> owned_event_values;std::vector<s::ReceiveBufferString>& event_values;
    std::vector<s::ReceiveBufferString*> owned_event_map;std::vector<s::ReceiveBufferString*>& event_map;
    s::NativeMessageEventQueue owned_queue;s::NativeMessageEventQueue& queue;
    s::NativeMessageMutex owned_native_mutex;s::NativeMessageMutex& native_mutex;
    std::mutex owned_event_gate,owned_achievement_gate;std::mutex& event_gate;std::mutex& achievement_gate;
    bool event_locked=false,achievement_locked=false,event_waiting=false,achievement_waiting=false;
    std::atomic<bool> owned_initialized{false};std::atomic<bool>& initialized;
    std::atomic<bool> owned_event_observed{false};std::atomic<bool>& event_observed;
    execution_recovered::MutexState* shared_achievement_mutex=nullptr;
    execution_recovered::Platform* shared_achievement_platform=nullptr;
    enum class LockMode {None,Native,Semantic};LockMode lock_mode=LockMode::None;
    std::uint64_t old_offset=0,old_count=0;std::size_t native_head=0;std::int32_t owned_achievement_count=0;std::int32_t& achievement_count;
    explicit Impl(Options o):owned_options(std::move(o)),options(owned_options),rows(options.profile.sessions),
        clock(options.time?options.time:std::make_shared<crypto_recovered::WindowsTimeSource>()),
        guarded_clock(std::make_shared<Clock>(*this)),random(std::make_shared<crypto_recovered::Random>(*guarded_clock)),
        owned_event_values(options.event_capacity),event_values(owned_event_values),owned_event_map(event_values.size()),event_map(owned_event_map),
        queue(owned_queue),native_mutex(owned_native_mutex),event_gate(owned_event_gate),achievement_gate(owned_achievement_gate),
        initialized(owned_initialized),event_observed(owned_event_observed),achievement_count(owned_achievement_count){
        for(std::size_t i=0;i<event_map.size();++i)event_map[i]=&event_values[i];queue={event_map,event_map.size(),0,0};
    }

    Impl(Impl& root,MessageCall& call):options(root.options),facade(root.facade),profile(root.profile),policy(root.policy),
        backend(root.backend),parent(root.parent),strings(root.strings),lease(root.lease),message(&call),
        xml(root.xml),cipher(root.cipher),clock(root.clock),guarded_clock(std::make_shared<Clock>(*this)),
        random(std::make_shared<crypto_recovered::Random>(*guarded_clock)),
        event_values(root.event_values),event_map(root.event_map),queue(root.queue),native_mutex(root.native_mutex),
        event_gate(root.event_gate),achievement_gate(root.achievement_gate),initialized(root.initialized),event_observed(root.event_observed),
        shared_achievement_mutex(root.shared_achievement_mutex),shared_achievement_platform(root.shared_achievement_platform),
        achievement_count(root.achievement_count){phase=Phase::Bound;}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Application graph requires its original controller/worker fiber");}

    struct MessageCall {
        Impl& root;std::shared_ptr<void> lease;
        s::SessionMemoryDomain& parent;sp::Strings& strings;np::Logic& logic;
        const sp::SessionView view;const s::SessionMemoryDomain::JobPermit permit;s::IoReference* const control;
        s::ReceiveBufferString& output;s::ReceiveBufferString& request;
        const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
        bool open=true,checking=false,retiring=false,failed=false;std::function<void()> checkpoint;
        std::shared_ptr<Impl> execution;
        MessageCall(Impl& owner,std::shared_ptr<void> backing,s::SessionMemoryDomain& memory,sp::Strings& owned,np::Logic& selected,sp::SessionView session,
                    const s::SessionMemoryDomain::JobPermit& proof,s::ReceiveBufferString& out,s::ReceiveBufferString& req)
            :root(owner),lease(std::move(backing)),parent(memory),strings(owned),logic(selected),view(session),permit(proof),
             control(session.owner.weak_self.control_08),output(out),request(req){}
        bool actor()const noexcept{return thread==::GetCurrentThreadId()&&fiber==::GetCurrentFiber();}
        void poison()noexcept{PreserveError error;std::lock_guard lock(root.metadata);failed=true;if(root.phase!=Phase::Closed)root.phase=Phase::Poisoned;}
        void metadata_valid(){
            PreserveError error;const auto current_thread=::GetCurrentThreadId();auto* current_fiber=::GetCurrentFiber();
            std::unique_lock lock(root.metadata);bool found=false;for(const auto& c:root.calls)if(c.get()==this){found=true;break;}
            if(!found||!open||failed||root.phase!=Phase::Bound||thread!=current_thread||fiber!=current_fiber||root.parent!=&parent||root.strings!=&strings||
               view.token.slot>=root.rows.size()||root.rows[view.token.slot].token!=view.token||root.rows[view.token.slot].logic!=&logic||!logic.matches(view))
                reject(lock,"Graph exact message/session admission ended or was poisoned");
        }
        void validate(){
            if(!actor())throw std::logic_error("Graph message belongs to another actor");
            permit.check(parent);metadata_valid();
            if(!parent.owns_control(control)||!control->strong_08.load())throw std::logic_error("Graph child session is not live");
            const auto& owner=static_cast<s::SessionControl*>(control)->object;
            if(&owner!=&view.owner||owner.weak_self.control_08!=control||owner.weak_self.object_00!=owner.identity())
                throw std::logic_error("Graph child owner/control identity changed");
            root.source_state();permit.check(parent);metadata_valid();
        }
        void admit(){
            validate();
            {PreserveError error;std::unique_lock lock(root.metadata);if(checking){failed=true;root.phase=Phase::Poisoned;reject(lock,"Graph message checkpoint cannot reenter");}checking=true;}
            try{checkpoint();validate();}
            catch(...){PreserveError error;std::lock_guard lock(root.metadata);checking=false;failed=true;if(root.phase!=Phase::Closed)root.phase=Phase::Poisoned;throw;}
            {PreserveError error;std::unique_lock lock(root.metadata);checking=false;if(root.phase!=Phase::Bound||failed)reject(lock,"Graph message checkpoint was poisoned");}
        }
    };

    void provider_actor(){
        if(message){
            message->metadata_valid();PreserveError error;std::unique_lock lock(message->root.metadata);
            if(message->checking||message->retiring){message->failed=true;message->root.phase=Phase::Poisoned;reject(lock,"Graph provider reentered its message checkpoint");}
            return;
        }
        affinity();
    }
    void poison(){if(message){message->poison();return;}PreserveError error;std::lock_guard lock(metadata);if(phase!=Phase::Closed&&phase!=Phase::Retiring)phase=Phase::Poisoned;}
    void valid(){
        if(message)message->metadata_valid();
        PreserveError error;std::unique_lock lock(metadata);if(phase==Phase::Poisoned||phase==Phase::Retiring||phase==Phase::Closed||!active)reject(lock,"Application graph admission was poisoned");
    }
    void source_state(bool cleanup=false){
        auto& root=message?message->root:*this;
        s::SessionMemoryDomain* memory;sp::Strings* owned;o::OwnedServerBackend* server;std::shared_ptr<sp::Profile> application;
        {PreserveError error;std::lock_guard lock(root.metadata);memory=root.parent;owned=root.strings;server=root.backend;application=root.profile.lock();}
        if(!memory||!owned||!server||!application)throw std::logic_error("Application graph has no bound live source");
        (void)memory->buffer_context();const auto ps=application->snapshot().phase;const auto bs=server->snapshot().phase;
        if((ps!=sp::Phase::Open&&!(cleanup&&ps==sp::Phase::Closing))||(bs!=o::Phase::Ready&&bs!=o::Phase::Running&&!(cleanup&&(bs==o::Phase::Returned||bs==o::Phase::Quiescing))))
            throw std::logic_error("Application graph source is closed or poisoned");
    }
    void source(bool cleanup=false){if(message){message->admit();return;}affinity();source_state(cleanup);}
    void admit(bool setup=false){valid();if(parent)source();else if(!setup)throw std::logic_error("Application graph is not bound");valid();}
    void begin(bool setup=false){
        provider_actor();PreserveError error;std::unique_lock lock(metadata);
        if(active){lock.unlock();poison();throw std::logic_error("Application graph execution cannot reenter");}
        if(phase!=Phase::Bound&&!(setup&&(phase==Phase::Fresh||phase==Phase::Installed)))reject(lock,"Application graph is not in a supported phase");
        active=true;
    }
    void end(){valid();PreserveError error;std::lock_guard lock(metadata);active=false;}
    void failed(){poison();PreserveError error;std::lock_guard lock(metadata);active=false;router_active=false;in_opaque=false;}
    template<class F>std::invoke_result_t<F> call(F&& f,bool setup=false,bool control=false,bool unknown=false){begin(setup);try{admit(setup);if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();admit(setup);end();}else{std::invoke_result_t<F> result=f();admit(setup);end();return result;}}catch(const m::NoResponseRequiredError&){if(!control){failed();throw;}try{admit(setup);end();}catch(...){failed();throw;}throw;}
    catch(const m::UnknownMessageError&){if(!unknown){failed();throw;}try{admit(setup);end();}catch(...){failed();throw;}throw;}catch(...){failed();throw;}}
    template<class F>std::invoke_result_t<F> internal(F&& f){
        provider_actor();if(!active||!router_active||in_opaque)return call(std::forward<F>(f));
        try{admit();if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();admit();}else{std::invoke_result_t<F> result=f();admit();return result;}}catch(...){poison();throw;}
    }
    template<class F>std::invoke_result_t<F> opaque(F&& f,bool control=false){
        provider_actor();if(!active)return call([&]() -> std::invoke_result_t<F>{return opaque(std::forward<F>(f));});
        admit();if(in_opaque){poison();throw std::logic_error("Opaque graph providers cannot reenter a source operation");}in_opaque=true;
        try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();in_opaque=false;admit();}else{std::invoke_result_t<F> result=f();in_opaque=false;admit();return result;}}
        catch(const m::NoResponseRequiredError&){in_opaque=false;if(!control){poison();throw;}try{admit();}catch(...){poison();throw;}throw;}
        catch(...){in_opaque=false;poison();throw;}
    }
    void structure(){if(!event_locked)throw std::logic_error("Event structure requires its actual lock");if(event_values.size()!=event_map.size()||event_map.empty()||(event_map.size()&(event_map.size()-1))||queue.map_08.data()!=event_map.data()||queue.map_08.size()!=event_map.size()||queue.map_size_10!=event_map.size()||queue.count_20>event_map.size())throw s::UnsupportedInput("Event ring metadata leaves its owned projection");
        for(std::size_t i=0;i<event_map.size();++i)if(event_map[i]!=&event_values[i])throw s::UnsupportedInput("Event ring slot identity was replaced");}
    void grow_event_map(){
        structure();if(!event_locked||lock_mode!=LockMode::Semantic)throw std::logic_error("Event growth requires the owned semantic lock");
        const auto old_capacity=event_values.size();
        const auto maximum=std::min(event_values.max_size(),event_map.max_size());
        if(old_capacity>maximum/2)throw std::length_error("Owned event map growth exceeds source container size");
        const auto capacity=std::max<std::size_t>(8,old_capacity*2);
        if(capacity>maximum)throw std::length_error("Owned event map growth exceeds source container size");
        // Prepare both allocations and their pointers before moving ownership.
        // Compiler allocation callbacks may have poisoned source admission.
        std::vector<s::ReceiveBufferString> values(capacity);
        std::vector<s::ReceiveBufferString*> map(capacity);
        for(std::size_t i=0;i<capacity;++i)map[i]=&values[i];
        admit();
        const auto head=static_cast<std::size_t>(queue.offset_18&(old_capacity-1));
        const auto count=static_cast<std::size_t>(queue.count_20);
        // Only the trivial prefix moves: inline bytes travel with it; owned
        // heap payload/header addresses (and Strings tracking) remain unchanged.
        static_assert(std::is_trivially_copyable_v<s::ReceiveBufferString> &&
                      std::is_nothrow_copy_assignable_v<s::ReceiveBufferString>);
        for(std::size_t i=0;i<count;++i){
            auto& source=event_values[(head+i)&(old_capacity-1)];
            values[head+i]=source;source={};
        }
        event_values.swap(values);event_map.swap(map);
        queue.map_08=event_map;queue.map_size_10=capacity;queue.offset_18=head;
        admit();
    }
    void push_raw(std::string_view text){
        structure();if(!event_locked||lock_mode!=LockMode::Semantic)throw std::logic_error("Event insertion requires the owned semantic lock");
        //034660 grows BEFORE offset normalization, retaining one spare slot.
        if(queue.count_20>=queue.map_size_10-1)grow_event_map();
        queue.offset_18&=queue.map_size_10-1;
        auto& slot=event_values[(queue.offset_18+queue.count_20)&(queue.map_size_10-1)];
        if(slot.size_10||slot.capacity_18!=15)throw s::UnsupportedInput("Event ring insertion slot is not reset");
        strings->assign(slot,text);admit();
        // Producer last use: the shared queue owns this payload before count
        // publication. Drop only this actor's whole-ticket temporary view;
        // a later consumer must not wait for the producer Operation to end.
        strings->relinquish(slot);admit();++queue.count_20;
    }

    void ensure_queue(){
        if(initialized.load(std::memory_order_acquire))return;
        if(event_locked){
            if(lock_mode!=LockMode::Semantic)throw std::logic_error("Native event lock cannot initialize the queue");
            if(!initialized.load(std::memory_order_relaxed)){push_raw(event_queue_recovered::initial_down_event);initialized.store(true,std::memory_order_release);}
            return;
        }
        lock_event(LockMode::Semantic);
        if(!initialized.load(std::memory_order_relaxed)){push_raw(event_queue_recovered::initial_down_event);initialized.store(true,std::memory_order_release);}
        unlock_event(LockMode::Semantic);
    }
    void lock_event(LockMode mode){
        if(event_locked||event_waiting||(achievement_locked&&mode==LockMode::Native))throw std::logic_error("Owned event lock is nonrecursive on this invocation");
        event_waiting=true;event_gate.lock();event_waiting=false;event_locked=true;lock_mode=mode;event_observed.store(true,std::memory_order_release);
        structure();native_mutex.lock_count_4c=1;
        if(mode==LockMode::Native){old_offset=queue.offset_18;old_count=queue.count_20;if(old_count)native_head=static_cast<std::size_t>(old_offset&(queue.map_size_10-1));}
    }
    void unlock_event(LockMode mode){
        if(!event_locked||lock_mode!=mode||native_mutex.lock_count_4c!=1)throw std::logic_error("Owned event unlock has no matching source lock");
        structure();
        if(mode==LockMode::Native){
            if(old_count){const auto expected=old_count-1;const auto offset=expected?old_offset+1:0;const auto& retired=event_values[native_head];
                if(queue.count_20!=expected||queue.offset_18!=offset||retired.size_10||retired.capacity_18!=15)throw s::UnsupportedInput("Native event pop did not retire exactly its observed head");}
            else if(queue.count_20!=0||queue.offset_18!=old_offset)throw s::UnsupportedInput("Empty-after-lock native path changed its queue");
        }
        native_mutex.lock_count_4c=0;lock_mode=LockMode::None;event_locked=false;event_observed.store(false,std::memory_order_release);event_gate.unlock();
    }
    struct EventMutex final:ch::Mutex {Impl& v;explicit EventMutex(Impl& v):v(v){}
        int lock_04a6ac()override{return v.internal([&]{v.lock_event(LockMode::Semantic);return 0;});}
        std::int32_t& count_4c()override{return v.internal([&]() -> std::int32_t&{if(!v.event_locked||v.lock_mode!=LockMode::Semantic)throw std::logic_error("Event mutex count requires its lock");return v.native_mutex.lock_count_4c;});}
        void unlock_04a6b4()override{v.internal([&]{v.unlock_event(LockMode::Semantic);});}
        [[noreturn]]void thread_error_04a648(int n)override{v.poison();bootstrap_policies::throw_thread_error_04a648(n);}
    } event_mutex{*this};
    struct AchievementPlatform final:execution_recovered::Platform {
        Impl& v;explicit AchievementPlatform(Impl& v):v(v){}
        std::uint32_t current_thread_id()override{return v.opaque([&]{return v.shared_achievement_platform->current_thread_id();});}
        void acquire_srw_exclusive(std::uintptr_t& lock)override{v.opaque([&]{v.shared_achievement_platform->acquire_srw_exclusive(lock);});}
        void release_srw_exclusive(std::uintptr_t& lock)override{v.opaque([&]{v.shared_achievement_platform->release_srw_exclusive(lock);});}
        std::int64_t query_performance_frequency()override{return v.opaque([&]{return v.shared_achievement_platform->query_performance_frequency();});}
        std::int64_t query_performance_counter()override{return v.opaque([&]{return v.shared_achievement_platform->query_performance_counter();});}
        void sleep_milliseconds(std::uint32_t ms)override{v.opaque([&]{v.shared_achievement_platform->sleep_milliseconds(ms);});}
        [[noreturn]]void raise_thread_error_04a648(int index)override{v.opaque([&]{v.shared_achievement_platform->raise_thread_error_04a648(index);});std::terminate();}
    } achievement_sdk{*this};
    struct AchievementMutex final:ch::Mutex {Impl& v;explicit AchievementMutex(Impl& v):v(v){}
        int lock_04a6ac()override{return v.internal([&]{
            if(v.achievement_locked||v.achievement_waiting)throw std::logic_error("Achievement mutex is nonrecursive on this invocation");
            v.achievement_waiting=true;
            if(v.shared_achievement_mutex){
                const auto result=execution_recovered::lock_04a6ac(*v.shared_achievement_mutex,v.achievement_sdk);
                if(result){v.achievement_waiting=false;return result;}
            }else{v.achievement_gate.lock();v.achievement_count=1;}
            v.achievement_waiting=false;v.achievement_locked=true;return 0;
        });}
        std::int32_t& count_4c()override{return v.internal([&]() -> std::int32_t&{
            if(!v.achievement_locked)throw std::logic_error("Achievement count requires its lock");
            // C++ permits aliasing through the corresponding signed type.
            static_assert(std::is_same_v<std::int32_t,std::make_signed_t<std::uint32_t>>);
            if(v.shared_achievement_mutex)return reinterpret_cast<std::int32_t&>(v.shared_achievement_mutex->count_4c);
            return v.achievement_count;
        });}
        void unlock_04a6b4()override{v.internal([&]{
            const auto count=v.shared_achievement_mutex?v.shared_achievement_mutex->count_4c:static_cast<std::uint32_t>(v.achievement_count);
            if(!v.achievement_locked||count!=1)throw std::logic_error("Achievement unlock has no matching lock");
            if(v.shared_achievement_mutex)execution_recovered::unlock_04a6b4(*v.shared_achievement_mutex,v.achievement_sdk);
            else{v.achievement_count=0;v.achievement_gate.unlock();}
            v.achievement_locked=false;
        });}
        [[noreturn]]void thread_error_04a648(int n)override{v.poison();bootstrap_policies::throw_thread_error_04a648(n);}
    } achievement_mutex{*this};
    struct Events final:np::Events,ch::EventQueue,misc_handlers_recovered::IgoEffects {Impl& v;explicit Events(Impl& v):v(v){}
        s::NativeMessageEventQueue& queue()override{return v.call([&]() -> s::NativeMessageEventQueue&{v.ensure_queue();return v.queue;});}
        s::NativeMessageMutex& mutex()override{return v.call([&]() -> s::NativeMessageMutex&{return v.native_mutex;});}
        int lock(s::NativeMessageMutex& m)override{return v.call([&]{if(&m!=&v.native_mutex)throw std::invalid_argument("Foreign native event mutex");v.lock_event(LockMode::Native);return 0;});}
        void unlock(s::NativeMessageMutex& m)override{v.call([&]{if(&m!=&v.native_mutex)throw std::invalid_argument("Foreign native event mutex");v.unlock_event(LockMode::Native);});}
        ch::Mutex& mutex_02c620()override{return v.internal([&]() -> ch::Mutex&{return v.event_mutex;});}
        bool initialized_locked()const override{return v.internal([&]{if(!v.event_locked||v.lock_mode!=LockMode::Semantic)throw std::logic_error("Event initialization query requires its lock");return v.initialized.load(std::memory_order_acquire);});}
        void initialize_locked(std::string seed)override{v.internal([&]{if(seed!=event_queue_recovered::initial_down_event||v.initialized)throw std::invalid_argument("Unknown or repeated native DOWN seed");v.push_raw(seed);v.initialized=true;});}
        void push_locked_034660(std::string text)override{v.internal([&]{if(!v.initialized)throw std::logic_error("Event queue is not initialized");v.push_raw(text);});}
        int lock_02c620_04a6ac()override{return v.event_mutex.lock_04a6ac();}
        std::int32_t& lock_count_4c()override{return v.event_mutex.count_4c();}
        void acquire_event_queue_02c400()override{v.internal([&]{v.ensure_queue();});}
        void enqueue_034660(std::string&& text)override{v.internal([&]{if(!v.initialized)throw std::logic_error("Event queue is not initialized");v.push_raw(text);text.clear();});}
        void unlock_04a6b4()override{v.event_mutex.unlock_04a6b4();}
        [[noreturn]]void thread_error_04a648(int n)override{v.poison();bootstrap_policies::throw_thread_error_04a648(n);}
    } events{*this};
    struct NoResponse final:misc_handlers_recovered::NoResponse {[[noreturn]]void throw_no_response_02bcd0_04d228()override{throw m::NoResponseRequiredError{};}} no_response;
    struct EffectScope {
        ch::CheckedCalls* checked;
        EffectScope(Impl& owner,ch::CheckedCalls* provider):checked(provider){if(checked)checked->bind_content_checkpoint({&owner,[](void* p){static_cast<Impl*>(p)->admit();}});}
        ~EffectScope(){if(checked)checked->clear_content_checkpoint();}
    };

    struct Information final:handlers_recovered::Effects {Impl& v;explicit Information(Impl& v):v(v){}
        void log_profile_name_084f48(std::string_view n)override{v.opaque([&]{
            if(v.message){if(!v.message_information)throw std::logic_error("Missing private Information view");v.message_information->log_profile_name_084f48(n);return;}
            EffectScope scope(v,dynamic_cast<ch::CheckedCalls*>(v.options.information.get()));v.options.information->log_profile_name_084f48(n);
        });}
        std::string current_time_041b60()override{return v.opaque([&]{
            if(v.message){if(!v.message_information)throw std::logic_error("Missing private Information view");return v.message_information->current_time_041b60();}
            EffectScope scope(v,dynamic_cast<ch::CheckedCalls*>(v.options.information.get()));return v.options.information->current_time_041b60();
        });}
    } information{*this};
    struct Writer final:ch::DocumentWriter {
        std::shared_ptr<Impl> v;ch::DocumentWriter& raw;
        struct Xml final:ch::Xml {Writer& w;ch::Xml* raw=nullptr;explicit Xml(Writer& w):w(w){}
            ch::Xml& source(){if(!raw)throw std::logic_error("Owned document XML proxy is unbound");return *raw;}
            const char* attribute_002cc0(handlers_recovered::Request r,const char* k)override{return w.v->opaque([&]{return source().attribute_002cc0(r,k);});}
            void open_004080(const char* e,bool c)override{w.v->opaque([&]{source().open_004080(e,c);});}
            void attribute_005560(const char* k,const char* val)override{w.v->opaque([&]{source().attribute_005560(k,val);});}
            void close_virtual_48(bool c)override{w.v->opaque([&]{source().close_virtual_48(c);});}
        } projected{*this};
        Writer(std::shared_ptr<Impl> v,ch::DocumentWriter& r):v(std::move(v)),raw(r){}
        ch::Xml& xml()override{projected.raw=&v->opaque([&]() -> ch::Xml&{return raw.xml();});return projected;}
        std::string c_string_without_nul()const override{return v->opaque([&]{return raw.c_string_without_nul();});}
    };
    struct Content final:ch::Effects,ch::AvatarRequest {Impl& v;explicit Content(Impl& v):v(v){}

        ch::Effects& source(){if(v.message){if(!v.message_content)throw std::logic_error("Missing private Content view");return *v.message_content;}return *v.options.content;}
        template<class F>std::invoke_result_t<F> invoke(F&& f){return v.opaque([&]() -> std::invoke_result_t<F>{
            if(v.message)return f();
            EffectScope scope(v,dynamic_cast<ch::CheckedCalls*>(v.options.content.get()));return f();
        });}
        std::int32_t atoi_05ee08(std::string_view s)override{return invoke([&]{return source().atoi_05ee08(s);});}
        void unknown_entitlement_field(std::string_view s)override{invoke([&]{source().unknown_entitlement_field(s);});}
        std::string utc_time_041b60(std::uint64_t& n)override{return invoke([&]{return source().utc_time_041b60(n);});}
        std::unique_ptr<ch::DocumentWriter> printer_0019b0()override{
            v.admit();if(v.writers.size()==v.writers.max_size())throw std::length_error("Graph writer inventory exhausted");v.writers.reserve(v.writers.size()+1);
            auto* raw=invoke([&]{auto value=source().printer_0019b0();if(!value)throw s::UnsupportedInput("Missing content document writer");auto* result=value.get();v.writers.push_back(std::move(value));return result;});
            return std::make_unique<Writer>(v.shared_from_this(),*raw);
        }
        void write_xml_03c1d0(const ch::Path& p,std::string_view s)override{invoke([&]{source().write_xml_03c1d0(p,s);});}
        void save_steam_01f9a0(const ch::AchievementMap& a)override{invoke([&]{source().save_steam_01f9a0(a);});}
        ch::Path filesystem_widen_006d70(std::string_view s)override{return invoke([&]{return source().filesystem_widen_006d70(s);});}
        ch::Path path_append_016b50(const ch::Path& p,const ch::Path& q)override{return invoke([&]{return source().path_append_016b50(p,q);});}
        bool online_avatar_virtual_28(const ch::Path& p)override{return invoke([&]{return source().online_avatar_virtual_28(p);});}
        bool request_online_avatar(const ch::Path& p,const char* owner,std::int32_t size)override{return invoke([&]{
            if(auto* full=dynamic_cast<ch::AvatarRequest*>(&source()))return full->request_online_avatar(p,owner,size);
            return source().online_avatar_virtual_28(p);
        });}
        bool copy_overwrite_048d20(const ch::Path& p,const ch::Path& q)override{return invoke([&]{return source().copy_overwrite_048d20(p,q);});}
        std::string path_to_utf8_031ca0(const ch::Path& p)override{return invoke([&]{return source().path_to_utf8_031ca0(p);});}
        void log_saved_size_085530(std::int32_t n)override{invoke([&]{source().log_saved_size_085530(n);});}
    } content{*this};
    struct License final:dispatch_recovered::LicenseHandlers {Impl& v;explicit License(Impl& v):v(v){}

        dispatch_recovered::LicenseHandlers& source(){if(v.message){if(!v.message_licensing)throw std::logic_error("Missing private Licensing view");return *v.message_licensing;}return *v.options.licensing;}
        template<class F>std::invoke_result_t<F> invoke(F&& f,bool control=true){return v.opaque([&]() -> std::invoke_result_t<F>{
            if(v.message)return f();
            EffectScope scope(v,dynamic_cast<ch::CheckedCalls*>(v.options.licensing.get()));return f();
        },control);}
        struct Xml final:handlers_recovered::Xml {Impl& v;handlers_recovered::Xml& raw;Xml(Impl& v,handlers_recovered::Xml& x):v(v),raw(x){}
            template<class F>std::invoke_result_t<F> checked(F&& f){v.admit();try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();v.admit();}else{std::invoke_result_t<F> result=f();v.admit();return result;}}catch(...){v.poison();throw;}}
            const char* attribute_002cc0(handlers_recovered::Request r,const char* k)override{return checked([&]{return raw.attribute_002cc0(r,k);});}
            void open_004080(const char* s,bool c)override{checked([&]{raw.open_004080(s,c);});}
            void attribute_005560(const char* k,const char* s)override{checked([&]{raw.attribute_005560(k,s);});}
            void close_virtual_48(bool c)override{checked([&]{raw.close_virtual_48(c);});}
        };
        bool auth_code_02e3b0(const m::XmlNode& n,handlers_recovered::Xml& x)override{Xml guarded(v,x);return invoke([&]{return source().auth_code_02e3b0(n,guarded);});}
        bool request_license_0320c0(const m::XmlNode& n,handlers_recovered::Xml& x)override{Xml guarded(v,x);return invoke([&]{return source().request_license_0320c0(n,guarded);});}
        bool invalidate_license_030c50(const m::XmlNode& n,handlers_recovered::Xml& x)override{Xml guarded(v,x);return invoke([&]{return source().invalidate_license_030c50(n,guarded);});}
    } licensing{*this};
    void retire_writers(){for(auto& writer:writers)if(writer)opaque([&]{writer.reset();});writers.clear();}
    bool dispatch(std::uint32_t rva,const m::XmlNode& node,m::XmlPrinter& printer){
        if(event_locked||achievement_locked)throw std::logic_error("Router cannot enter while a prior event/storage lock is held");
        auto& c=*options.state.configuration;const auto& st=options.state;
        // Recreate spans/references for EACH source dispatch; vector relocation
        // by an explicitly serialized update never leaves a persistent Router.
        handlers_recovered::Profile profile_value{c.username,c.persona_id,c.user_id,c.country,c.currency,c.subscriber_level,c.is_subscriber,c.is_under_age,c.is_steam_subscriber};
        dispatch_recovered::Router router{{c.game_attributes,c.emulator_attributes,profile_value,*st.online,c.pretend_connected},
            {c.content,c.entitlements,c.game_attributes,c.achievements,c.achievements_set,c.content_id,c.user_id,*st.achievement_file,c.avatar,c.default_avatar,c.user_directory,c.last_avatar_path,*st.online,*st.saved_avatar_size,achievement_mutex,events},
            {*xml,information,events,no_response,content,licensing}};
        router_active=true;try{const auto result=router.registered_handler(rva,node,printer);router_active=false;retire_writers();return result;}catch(...){router_active=false;throw;}
    }
    void write_log(sp::SessionView view,Log kind,std::string_view text,std::string_view error={}){
        const char* prefix=kind==Log::Received?"Received: ":kind==Log::Response?"Response: ":kind==Log::Event?"Event: ":kind==Log::BadXml?"Bad XML: ":"Decryption failure: ";
        if(text.size()>static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max())||error.size()>static_cast<std::size_t>(std::numeric_limits<std::streamsize>::max()))throw std::length_error("Message log exceeds compiler stream bounds");
        auto& out=*options.policy.stream;
        opaque([&]{const auto lock=shared_log::acquire(out);out<<prefix;});
        if(!error.empty()){opaque([&]{const auto lock=shared_log::acquire(out);out.write(error.data(),static_cast<std::streamsize>(error.size()));});opaque([&]{const auto lock=shared_log::acquire(out);out<<": ";});}
        opaque([&]{const auto lock=shared_log::acquire(out);out.write(text.data(),static_cast<std::streamsize>(text.size()));});
        const auto newline=opaque([&]{const auto lock=shared_log::acquire(out);return out.widen('\n');});
        opaque([&]{const auto lock=shared_log::acquire(out);out.put(newline);});opaque([&]{const auto lock=shared_log::acquire(out);out.flush();});
        if(options.observe_log)opaque([&]{options.observe_log(view,kind,text,error);});
    }
    struct Effects final:m::Effects,m::UnknownMessageEffects {
        std::shared_ptr<Impl> v;sp::SessionView view;Effects(std::shared_ptr<Impl> v,sp::SessionView s):v(std::move(v)),view(s){}
        template<class F>std::invoke_result_t<F> invoke(F&& f,bool control=false,bool unknown=false){return v->call([&]() -> std::invoke_result_t<F>{v->for_session(view);return std::forward<F>(f)();},false,control,unknown);}
        int atoi_05ee08(std::string_view text)override{return invoke([&]{return std::atoi(std::string(text).c_str());});}
        bool registered_handler(std::uint32_t rva,const m::XmlNode& node,m::XmlPrinter& printer)override{return invoke([&]{return v->dispatch(rva,node,printer);},true);}
        [[noreturn]]void invalid_message_02bd30_04d228()override{invoke([]{throw m::UnknownMessageError{};},false,true);std::terminate();}
        void unknown_message_07bd70(const m::UnknownMessageInput& input)override{invoke([&]{
            auto* diagnostic=v->message?static_cast<m::UnknownMessageEffects*>(v->message_licensing.get()):dynamic_cast<m::UnknownMessageEffects*>(v->options.licensing.get());if(!diagnostic)throw s::UnsupportedInput("Unknown-message diagnostic provider required");
            v->licensing.invoke([&]{diagnostic->unknown_message_07bd70(input);},false);
        });}
        [[noreturn]]void thread_error_04a648(int n)override{invoke([&]{v->poison();bootstrap_policies::throw_thread_error_04a648(n);});std::terminate();}
        void log_received(std::string_view text)override{invoke([&]{v->write_log(view,Log::Received,text);});}
        void log_response(std::string_view text)override{invoke([&]{v->write_log(view,Log::Response,text);});}
        void log_event(std::string_view text)override{invoke([&]{v->write_log(view,Log::Event,text);});}
        void log_bad_xml(std::string_view error,std::string_view text)override{invoke([&]{v->write_log(view,Log::BadXml,text,error);});}
        void log_decryption_failure(std::string_view text)override{invoke([&]{v->write_log(view,Log::DecryptionFailure,text);});}
    };

    void for_session(sp::SessionView view){
        if(!message)return;message->metadata_valid();
        if(message->view.token!=view.token||&message->view.owner!=&view.owner){poison();throw std::logic_error("Graph message cannot use another session's Effects");}
    }
    void process_job(np::Logic& logic,sp::SessionView view,s::SessionMemoryDomain& memory,sp::Strings& owned,s::ReceiveBufferString& output,s::ReceiveBufferString& request,
                     const s::SessionMemoryDomain::JobPermit& permit,std::function<void()>& checkpoint){
        auto pin=shared_from_this();permit.check(memory);
        if(!checkpoint||&output==&request||owned.domain_identity()!=&memory)throw std::invalid_argument("Exact child string domain, distinct prefixes and admission required");
        Admission admission{nullptr,::GetCurrentThreadId(),::GetCurrentFiber(),&output,&request,&logic,view.token};std::shared_ptr<void> backing;
        {
            PreserveError error;std::unique_lock lock(metadata);
            if(phase!=Phase::Bound||parent!=&memory||strings!=&owned||view.token.slot>=rows.size()||rows[view.token.slot].token!=view.token||rows[view.token.slot].logic!=&logic||!logic.matches(view))
                reject(lock,"Child message belongs to another Graph/session/Parent");
            if(thread==admission.thread)reject(lock,"Graph message requires a distinct child worker");
            if(logic.failed_prefix())reject(lock,"Graph native Logic retains a failed prefix");
            const auto aliases=[&](s::ReceiveBufferString* out,s::ReceiveBufferString* req){return &output==out||&output==req||&request==out||&request==req;};
            for(auto* a=admissions;a;a=a->next)if((a->thread==admission.thread&&a->fiber==admission.fiber)||aliases(a->output,a->request)){
                phase=Phase::Poisoned;reject(lock,"Graph message preparation cannot reenter or alias");}
            for(const auto& a:calls)if((a->thread==admission.thread&&a->fiber==admission.fiber)||aliases(&a->output,&a->request)){
                phase=Phase::Poisoned;reject(lock,"Graph message actor or prefixes are already reserved");}
            backing=lease;admission.next=admissions;admissions=&admission;
        }
        struct Pending {Impl& v;Admission& admission;bool linked=true;
            void remove()noexcept{auto** p=&v.admissions;while(*p&&*p!=&admission)p=&(*p)->next;if(!*p)std::terminate();*p=admission.next;linked=false;}
            ~Pending(){PreserveError error;if(linked){std::lock_guard lock(v.metadata);remove();}}
        } pending{*this,admission};
        std::list<std::shared_ptr<MessageCall>> prepared,retired;std::shared_ptr<MessageCall> selected;std::shared_ptr<Impl> execution;
        try{
            {PreserveError error;prepared.emplace_back();selected=std::make_shared<MessageCall>(*this,std::move(backing),memory,owned,logic,view,permit,output,request);
                prepared.front()=selected;std::unique_lock lock(metadata);calls.splice(calls.end(),prepared);pending.remove();if(phase!=Phase::Bound)reject(lock,"Graph message preparation was poisoned");}
            selected->checkpoint.swap(checkpoint);selected->admit();
            {PreserveError error;selected->execution=std::make_shared<Impl>(*this,*selected);}execution=selected->execution;selected->admit();
            application_io::CallContext context(memory,permit,{selected.get(),[](void* p){static_cast<MessageCall*>(p)->admit();}});
            auto information=std::dynamic_pointer_cast<information_effects::Information>(options.information);
            auto content=std::dynamic_pointer_cast<content_effects::Content>(options.content);
            auto licensing=std::dynamic_pointer_cast<licensing_effects::Handlers>(options.licensing);
            if(!information||!content||!licensing)throw s::UnsupportedInput("Graph message requires the selected concrete call-view providers");
            execution->message_information=information->make_call_view(context);selected->admit();
            execution->message_content=content->make_call_view(context);selected->admit();
            execution->message_licensing=licensing->make_call_view(context);selected->admit();
            {
                np::Dependencies d;d.lifetime=selected;d.xml=execution->xml;d.factory=execution->cipher;d.random=execution->random;
                d.effects=std::make_shared<Effects>(execution,view);d.events={execution,&execution->events};d.version=options.version;d.show_lsx=options.show_lsx;
                selected->admit();logic.process_native_call(memory,owned,output,request,d,[raw=selected.get()]{raw->admit();});selected->admit();
                execution->retire_writers();selected->admit();
                {PreserveError error;std::unique_lock lock(metadata);if(phase!=Phase::Bound||selected->failed)reject(lock,"Graph message return was poisoned");selected->retiring=true;}
                execution->message_licensing->close();selected->admit();execution->message_content->close();selected->admit();execution->message_information->close();selected->admit();
                // d and its adapter targets retire while the call remains registered.
            }
            selected->admit();
            {PreserveError error;execution->message_licensing.reset();execution->message_content.reset();execution->message_information.reset();}
            selected->admit();
            if(execution->active||execution->router_active||execution->in_opaque||execution->event_locked||execution->achievement_locked||execution->event_waiting||execution->achievement_waiting||!execution->writers.empty()||logic.failed_prefix())
                throw std::logic_error("Graph message retains an unfinished execution prefix");
            {PreserveError error;execution.reset();selected->execution.reset();}
            selected->admit();
            {std::function<void()> retiring;retiring.swap(selected->checkpoint);}selected->validate();
            {PreserveError error;std::unique_lock lock(metadata);if(phase!=Phase::Bound||selected->failed||selected->checking)reject(lock,"Graph message target retirement was poisoned");
                auto at=calls.begin();while(at!=calls.end()&&at->get()!=selected.get())++at;if(at==calls.end())std::terminate();selected->open=false;retired.splice(retired.end(),calls,at);}
            {PreserveError error;retired.clear();selected.reset();}
        }catch(...){if(selected)selected->poison();else poison();throw;}
    }
    void bind(s::SessionMemoryDomain& memory,sp::Strings& owned){
        affinity();bool bound=false;
        {PreserveError error;std::unique_lock lock(metadata);
            if(parent){if(parent!=&memory||strings!=&owned)reject(lock,"Application graph was bound to another Parent/string domain");bound=true;}
            else{if(phase!=Phase::Installed||active||owned.domain_identity()!=&memory)reject(lock,"Application graph cannot bind this source domain");active=true;}
        }
        if(bound){source();return;}
        try{
            (void)memory.buffer_context();auto prepared=memory.acquire_provider_lease_from_operation();
            PreserveError error;std::unique_lock lock(metadata);
            // Adopt the known returned lease even if an allocator callback
            // poisoned preparation; no speculative provider-prefix rollback.
            parent=&memory;strings=&owned;lease=std::move(prepared);
            if(phase!=Phase::Installed||!active)reject(lock,"Graph source binding was poisoned during lease allocation");
            phase=Phase::Bound;active=false;
        }catch(...){failed();throw;}
    }
    struct Logic final:np::Logic,np::JobLogic {
        std::shared_ptr<Impl> v;sp::SessionView view;bool registered=false;
        Logic(std::shared_ptr<Impl> v,sp::SessionView view,np::Dependencies d):np::Logic(view,std::move(d)),v(std::move(v)),view(view){}
        ~Logic()noexcept override{if(!registered)return;v->affinity();std::lock_guard lock(v->metadata);for(auto* pending=v->admissions;pending;pending=pending->next)if(pending->logic==this)std::terminate();for(const auto& call:v->calls)if(&call->logic==this)std::terminate();auto& row=v->rows[view.token.slot];if(row.logic!=this||retained_prefix()||!v->live)std::terminate();row.logic=nullptr;registered=false;--v->live;}
        std::string greeting_native(s::SessionMemoryDomain& memory,sp::Strings& owned,std::function<void()> checkpoint)override{
            try{v->bind(memory,owned);v->call([&]{if(v->options.prepare_session)v->options.prepare_session(view);});return np::Logic::greeting_native(memory,owned,std::move(checkpoint));}catch(...){v->poison();throw;}
        }
        void process_native_job(s::SessionMemoryDomain& memory,sp::Strings& owned,s::ReceiveBufferString& output,s::ReceiveBufferString& request,const s::SessionMemoryDomain::JobPermit& permit,std::function<void()> checkpoint)override{
            v->process_job(*this,view,memory,owned,output,request,permit,checkpoint);
        }
    };
    void ensure_logic_row(std::size_t slot){
        PreserveError error;std::size_t previous_size;const Row* previous_data;
        {
            std::lock_guard lock(metadata);if(slot<rows.size())return;
            previous_size=rows.size();previous_data=rows.data();
        }
        const auto maximum=rows.max_size();
        if(slot>=maximum)throw std::length_error("Graph session registry exceeds source container size");
        auto capacity=std::max<std::size_t>(previous_size,8);
        if(capacity>maximum)capacity=maximum;
        while(capacity<=slot){
            if(capacity>maximum/2){capacity=maximum;break;}
            capacity*=2;
        }
        // Allocate without retaining any Row reference or source iterator.
        // The old rows remain authoritative until admission is rechecked.
        std::vector<Row> prepared(capacity);admit(true);
        {
            std::lock_guard lock(metadata);
            if(rows.size()!=previous_size||rows.data()!=previous_data)
                throw std::logic_error("Graph session registry changed during growth allocation");
            static_assert(std::is_nothrow_copy_assignable_v<Row>);
            std::copy(rows.begin(),rows.end(),prepared.begin());rows.swap(prepared);
        }
        admit(true);
    }
    std::unique_ptr<sp::SessionLogic> make_logic(sp::SessionView view){return call([&]() -> std::unique_ptr<sp::SessionLogic>{
        if(!view.token.generation)throw std::invalid_argument("Session token has no graph generation");
        ensure_logic_row(view.token.slot);
        {
            PreserveError error;std::lock_guard lock(metadata);const auto& row=rows[view.token.slot];
            if(row.logic||view.token.generation<=row.token.generation)throw std::invalid_argument("Session graph slot is live or stale");
        }
        auto self=shared_from_this();np::Dependencies d;d.lifetime=self;d.xml=xml;d.factory=cipher;d.random=random;d.effects=std::make_shared<Effects>(self,view);d.events={self,&events};d.version=options.version;d.show_lsx=options.show_lsx;
        admit(true);auto result=std::make_unique<Logic>(self,view,std::move(d));admit(true);
        {
            PreserveError error;std::lock_guard lock(metadata);auto& row=rows[view.token.slot];
            if(row.logic||view.token.generation<=row.token.generation)throw std::invalid_argument("Session graph slot changed during logic construction");
            row.token=view.token;row.logic=result.get();result->registered=true;++live;
        }
        return result;
    },true);}
    bool drain(o::ServerView view,sp::Application& app){
        affinity();{PreserveError error;std::lock_guard lock(metadata);if(admissions||!calls.empty())return false;if(active||event_locked||achievement_locked){phase=Phase::Poisoned;throw std::logic_error("Application graph cannot retire an active prefix");}if(phase==Phase::Drained)return true;if(phase!=Phase::Bound&&phase!=Phase::Installed)throw std::logic_error("Application graph drain is unknown");active=true;}
        try{
            const auto app_ports=app.ports();if(&view.backend!=backend||&app_ports.view.backend!=backend||(parent&&parent!=&view.memory))throw std::invalid_argument("Application graph drain belongs to another server");
            const auto bs=backend->snapshot().phase;if(bs!=o::Phase::Returned&&bs!=o::Phase::Quiescing)throw std::logic_error("Graph retirement requires observed worker return");
            auto operation=view.memory.enter();app.reap();valid();const auto snapshot=app.snapshot();
            if(snapshot.live_sessions||snapshot.live_jobs||live||!writers.empty()){end();return false;}
            if(parent){source(true);lock_event(LockMode::Semantic);structure();
                while(queue.count_20){auto& head=event_values[queue.offset_18&(queue.map_size_10-1)];strings->release(head);valid();source(true);--queue.count_20;if(!queue.count_20)queue.offset_18=0;else ++queue.offset_18;}
                unlock_event(LockMode::Semantic);strings->prune();valid();source(true);
                std::shared_ptr<void> retiring;
                {PreserveError error;std::lock_guard lock(metadata);retiring.swap(lease);parent=nullptr;strings=nullptr;}
                retiring.reset();
            }
            {PreserveError error;std::lock_guard lock(metadata);if(phase==Phase::Poisoned)throw std::logic_error("Graph retirement was poisoned");phase=Phase::Drained;active=false;}return true;
        }catch(...){failed();throw;}
    }
    bool external(o::ServerView view,sp::Application& app,const std::function<bool(o::ServerView,sp::Application&)>& proof){
        affinity();{PreserveError error;std::lock_guard lock(metadata);if(admissions||!calls.empty())return false;if(active||event_locked||achievement_locked){phase=Phase::Poisoned;throw std::logic_error("External graph proof cannot reenter an active prefix");}if(phase!=Phase::Bound&&phase!=Phase::Installed&&phase!=Phase::Drained)throw std::logic_error("External graph proof is not admissible");active=true;}
        try{if(&view.backend!=backend||(parent&&parent!=&view.memory))throw std::invalid_argument("External graph proof belongs to another server");
            const auto result=proof(view,app);valid();end();if(!result)return false;return drain(view,app);
        }catch(...){failed();throw;}
    }
    Snapshot snapshot()const{affinity();PreserveError error;std::lock_guard lock(metadata);return {phase,live,static_cast<std::size_t>(queue.count_20),active||admissions||!calls.empty(),event_observed.load(std::memory_order_acquire),initialized.load(std::memory_order_acquire),parent!=nullptr};}
    struct Server final:server_recovered::ServerBackend {
        std::shared_ptr<Graph> owner;std::unique_ptr<o::OwnedServerBackend> inner;
        explicit Server(std::shared_ptr<Graph> p):owner(std::move(p)){}
        ~Server()noexcept override{auto& v=*owner->impl_;v.affinity();{PreserveError error;std::lock_guard lock(v.metadata);v.backend=nullptr;}if(inner&&inner->snapshot().phase!=o::Phase::Closed)std::terminate();inner.reset();}
        template<class F>std::invoke_result_t<F> invoke(F&& f,bool allow_deferral=false){auto& v=*owner->impl_;v.affinity();
            {PreserveError error;std::lock_guard lock(v.metadata);if(v.server_active||v.phase==Phase::Poisoned||v.phase==Phase::Retiring||!inner){if(v.phase!=Phase::Closed)v.phase=Phase::Poisoned;throw std::logic_error("Application server entry is closed, reentrant or foreign");}v.server_active=true;}
            try{if constexpr(std::is_void_v<std::invoke_result_t<F>>){f();check(v);}else{std::invoke_result_t<F> result=f();check(v);return result;}}
            catch(const o::QuiescenceDeferred&){
                const auto state=inner->snapshot();
                if(!allow_deferral||state.phase!=o::Phase::Quiescing||state.quiescence_attested||!state.acceptor_initialized||!state.pending_initialized)v.poison();
                check(v);throw;
            }catch(...){v.poison();PreserveError error;std::lock_guard lock(v.metadata);v.server_active=false;throw;}
        }
        void check(Impl& v){PreserveError error;std::lock_guard lock(v.metadata);v.server_active=false;if(v.phase==Phase::Poisoned)throw std::logic_error("Application server source was poisoned");}
        void initialize_context_0241b0()override{invoke([&]{inner->initialize_context_0241b0();});}
        server_recovered::ParsedAddress parse_address_028690(std::uint16_t f,std::string_view s)override{return invoke([&]{return inner->parse_address_028690(f,s);});}
        void construct_acceptor_020e20(const server_recovered::Endpoint& e,bool r,int n)override{invoke([&]{inner->construct_acceptor_020e20(e,r,n);});}
        void initialize_pending_socket_0211b0()override{invoke([&]{inner->initialize_pending_socket_0211b0();});}
        server_recovered::LocalEndpoint local_endpoint()override{return invoke([&]{return inner->local_endpoint();});}
        bool set_environment_variable(std::string_view k,std::string_view s)override{return invoke([&]{return inner->set_environment_variable(k,s);});}
        void async_accept_021b00()override{invoke([&]{inner->async_accept_021b00();});}
        server_recovered::Error run_context_0298b0()override{return invoke([&]{return inner->run_context_0298b0();});}
        void destroy_pending_socket_024db0()override{invoke([&]{inner->destroy_pending_socket_024db0();},true);}
        void destroy_acceptor_024db0()override{invoke([&]{inner->destroy_acceptor_024db0();});}
        void destroy_context()override{invoke([&]{inner->destroy_context();auto& v=*owner->impl_;PreserveError error;std::unique_lock lock(v.metadata);if(v.phase!=Phase::Drained&&v.phase!=Phase::Installed)reject(lock,"Application server has no known drained/unstarted graph");v.phase=Phase::Closed;});}
        [[noreturn]]void raise_error(server_recovered::Error e,std::string_view text)override{invoke([&]{inner->raise_error(e,text);});std::terminate();}
    };
};
std::int64_t Graph::Impl::Clock::unix_seconds_05eeb8(){return v.opaque([&]{return v.clock->unix_seconds_05eeb8();});}
std::shared_ptr<Graph> Graph::create(Options o){const auto& s=o.state;const auto& p=o.profile;const auto& l=o.policy.lifecycle;
    if(!s.lifetime||!s.configuration||!s.online||!s.saved_avatar_size||!s.achievement_file||!o.policy.stream||o.policy.guard_epoch||!session_policy::lifecycle_configuration_valid(l)||!o.information||!o.content||!o.licensing||!p.sessions||p.sessions>65536||!p.jobs_capacity||p.jobs_capacity>65536||!p.headers||p.headers>1048576||(!p.environment_owner&&(!p.environment_capacity||p.environment_capacity>1048576))||p.initial_output_slots<8||p.initial_output_slots>1048576||(p.initial_output_slots&(p.initial_output_slots-1))||!p.receive.receive||!p.receive.get_error||!p.send.send||!p.send.get_error||!o.event_capacity||o.event_capacity>65536||(o.event_capacity&(o.event_capacity-1)))throw std::invalid_argument("Complete exact application state, effects, lifecycle and bounded capacities required");
    const auto& i=o.policy.imports;if(!i.set_last_error||!i.get_last_error||!i.convert||!i.parse||!i.send||!i.select_write||!o.policy.maximum_message_bytes||o.policy.maximum_message_bytes>1048576)throw std::invalid_argument("Complete bounded policy SDK configuration required");
    for(const auto* prefix:{&o.policy.read_prefix,&o.policy.write_prefix,&o.policy.accept_prefix})if(prefix->size()>o.policy.maximum_message_bytes||prefix->find('\0')!=std::string::npos)throw std::invalid_argument("Policy prefixes require bounded C strings");
    auto result=std::shared_ptr<Graph>(new Graph(std::move(o)));result->impl_->facade=result;return result;
}
Graph::Graph(Options o):impl_(std::make_shared<Impl>(std::move(o))){}
void Graph::bind_achievement_mutex(execution_recovered::MutexState& state,execution_recovered::Platform& platform){
    auto& v=*impl_;v.affinity();PreserveError error;std::lock_guard lock(v.metadata);
    if(v.phase!=Phase::Fresh||v.active||v.shared_achievement_mutex)
        throw std::logic_error("Graph achievement mutex binds once before installation");
    if(state.type!=2||state.owner_48!=0xffffffffU||state.count_4c||state.srw_10||state.storage_08)
        throw std::invalid_argument("Graph achievement mutex requires initialized idle type2 state");
    v.shared_achievement_mutex=&state;v.shared_achievement_platform=&platform;
}
Graph::~Graph()noexcept{auto& v=*impl_;v.affinity();{PreserveError error;std::lock_guard lock(v.metadata);if(v.admissions||!v.calls.empty()||v.active||v.server_active||v.live||v.event_locked||v.achievement_locked||v.lease||v.backend||v.pending_server||!v.writers.empty()||(v.phase!=Phase::Fresh&&v.phase!=Phase::Closed))std::terminate();v.phase=Phase::Retiring;}
    v.random.reset();v.guarded_clock.reset();v.clock.reset();v.xml.reset();v.cipher.reset();
    {Options retiring=std::move(v.options);v.options.prepare_session={};v.options.observe_log={};v.options.policy.lifecycle={};v.options.policy.format_override={};}
}
void Graph::install_server(bootstrap_backend::Resources& resources,std::shared_ptr<thread_domain::DomainStorage> domain,o::Dependencies server,rt::Dependencies runtime){
    install_server_impl(resources,std::move(domain),nullptr,std::move(server),std::move(runtime));
}
void Graph::install_server_prepared(bootstrap_backend::Resources& resources,std::shared_ptr<thread_domain::DomainStorage> domain,worker_handoff::PreparedMemory::Borrow&& borrow,o::Dependencies server,rt::Dependencies runtime){
    if(!domain||borrow.domain_identity()!=domain.get())throw std::invalid_argument("Graph requires matching dedicated worker memory");
    (void)borrow.memory();
    install_server_impl(resources,std::move(domain),&borrow,std::move(server),std::move(runtime));
}
void Graph::install_server_impl(bootstrap_backend::Resources& resources,std::shared_ptr<thread_domain::DomainStorage> domain,worker_handoff::PreparedMemory::Borrow* borrow,o::Dependencies server,rt::Dependencies runtime){auto& v=*impl_;v.affinity();
    const auto& k=server.kernel;
    if(resources.server||!domain||domain->phase()!=thread_domain::Phase::Ready||server.errors||server.runtime||server.misc_category.name_identity||runtime.errors||runtime.profile||runtime.guard_epoch||runtime.generic_category.name_identity||runtime.system_category.name_identity||runtime.misc_category.name_identity||!runtime.tls_index||*runtime.tls_index==TLS_OUT_OF_INDEXES||!server.sockets||!server.iocp||!server.gates||!server.dispatcher||!server.executor||!k.startup||!k.cleanup||!k.local_name||!k.environment||!k.delete_gate||!runtime.accept||!runtime.factory||!runtime.cancel||!runtime.executor.move||!runtime.capacity||runtime.capacity>65536||(runtime.sockets&&runtime.sockets!=server.sockets)||(runtime.dispatcher&&runtime.dispatcher!=server.dispatcher))throw std::invalid_argument("Graph installation refuses incomplete/prebound/mixed application resources");
    {PreserveError error;std::lock_guard lock(v.metadata);if(v.phase!=Phase::Fresh||v.active)throw std::logic_error("Application graph installs once");v.active=true;v.phase=Phase::Installing;}
    try{auto self=shared_from_this();auto core=impl_;v.pending_server=std::make_unique<Impl::Server>(self);
        auto selected=v.options.policy;selected.guard_epoch=&v.epoch;
        struct Lifetime {std::shared_ptr<Impl> core;std::shared_ptr<void> prior;};selected.lifecycle.lifetime=std::make_shared<Lifetime>(Lifetime{core,selected.lifecycle.lifetime});
        session_policy::Policy::GraphRetirement retirement;
        if(selected.lifecycle.mode==session_policy::LifecycleMode::OriginalApplication){
            // Preserve the empty external-proof slot required by this profile.
            // Still retire this exact Graph's source queue/strings/Parent lease.
            retirement=[core](o::ServerView view,sp::Application& app){return core->drain(view,app);};
        }else{
            const auto external=selected.lifecycle.external_drained;
            selected.lifecycle.external_drained=[core,external](o::ServerView view,sp::Application& app){return core->external(view,app,external);};
        }
        const bool original_default_environment=selected.lifecycle.mode==session_policy::LifecycleMode::OriginalApplication;
        auto policy=session_policy::Policy::create(std::move(selected),std::move(retirement));sp::Dependencies d;policy->install(d);d.state_lifetime=self;
        // Native initial socket type selects023dd0's empty IO environment;
        // Slot context/copy/response ownership remains in the owned provider.
        d.original_default_environment=original_default_environment;
        const auto& p=v.options.profile;d.sessions=p.sessions;d.jobs_capacity=p.jobs_capacity;d.headers=p.headers;d.initial_output_slots=p.initial_output_slots;d.environment_capacity=p.environment_capacity;d.receive_imports=p.receive;d.send_imports=p.send;d.jobs=p.jobs;d.environment_owner=p.environment_owner;
        d.logic=[weak=std::weak_ptr<Graph>(self)](sp::SessionView view){auto graph=weak.lock();if(!graph)throw std::logic_error("Application graph factory backing expired");return graph->impl_->make_logic(view);};
        auto profile=std::make_shared<sp::Profile>(std::move(d));{PreserveError error;std::lock_guard lock(v.metadata);v.policy=policy;v.profile=profile;}
        server.errors=policy->errors();server.misc_category=policy->misc_category();runtime.errors=server.errors;
        if(!runtime.sockets)runtime.sockets=server.sockets;if(!runtime.dispatcher)runtime.dispatcher=server.dispatcher;
        if(!runtime.worker)runtime.worker=std::make_shared<s::NativeWorkerRunApi>();
        runtime.profile=profile;runtime.guard_epoch=&v.epoch;runtime.generic_category=policy->generic_category();runtime.system_category=policy->system_category();runtime.misc_category=policy->misc_category();
        server.state_lifetime=std::make_shared<Backing>(Backing{self,server.state_lifetime});runtime.state_lifetime=std::make_shared<Backing>(Backing{self,runtime.state_lifetime});
        const auto capacity=runtime.capacity;
        v.pending_server->inner=borrow?rt::make_server_prepared(std::move(domain),std::move(*borrow),std::move(server),std::move(runtime),capacity):rt::make_server(std::move(domain),std::move(server),std::move(runtime),capacity);
        {PreserveError error;std::lock_guard lock(v.metadata);v.backend=v.pending_server->inner.get();}v.valid();resources.server=std::move(v.pending_server);
        {PreserveError error;std::lock_guard lock(v.metadata);v.phase=Phase::Installed;v.active=false;}
    }catch(...){v.failed();throw;}
}
void Graph::close_unstarted(){auto& v=*impl_;v.begin(true);try{
    if(v.phase!=Phase::Installed||!v.backend||v.backend->snapshot().phase!=o::Phase::Constructed||v.parent||v.live||v.lease)throw std::logic_error("Only a known never-started application graph can be cancelled");v.backend->destroy_context();v.valid();{PreserveError error;std::lock_guard lock(v.metadata);v.phase=Phase::Closed;v.active=false;}
}catch(...){v.failed();throw;}}
void Graph::enqueue(std::string_view text){auto& v=*impl_;v.affinity();v.call([&]{if(v.event_locked||v.achievement_locked)throw std::logic_error("Cannot enqueue into a live source callback lock");v.lock_event(Impl::LockMode::Semantic);v.ensure_queue();v.push_raw(text);v.unlock_event(Impl::LockMode::Semantic);});}
void Graph::update(const std::function<void(StateView)>& update){if(!update)throw std::invalid_argument("Explicit state update required");auto& v=*impl_;v.affinity();{PreserveError error;std::unique_lock lock(v.metadata);if(v.admissions||!v.calls.empty())reject(lock,"Graph configuration update overlaps a live message");}v.call([&]{if(v.event_locked||v.achievement_locked)throw std::logic_error("Cannot mutate state under a live source lock");update(v.options.state);},true);}
Snapshot Graph::snapshot()const{return impl_->snapshot();}
bool Graph::retained(sp::SessionToken t)const{auto& v=*impl_;v.affinity();PreserveError error;std::lock_guard lock(v.metadata);if(t.slot>=v.rows.size()||v.rows[t.slot].token!=t)throw std::invalid_argument("Unknown graph session token");const auto* logic=v.rows[t.slot].logic;if(!logic)return false;for(auto* pending=v.admissions;pending;pending=pending->next)if(pending->logic==logic&&pending->token==t)return true;for(const auto& call:v.calls)if(&call->logic==logic)return true;return logic->retained_prefix();}
std::shared_ptr<sp::Profile> Graph::profile()const{impl_->affinity();PreserveError error;std::lock_guard lock(impl_->metadata);return impl_->profile.lock();}
std::shared_ptr<session_policy::Policy> Graph::policy()const{impl_->affinity();PreserveError error;std::lock_guard lock(impl_->metadata);return impl_->policy.lock();}
o::OwnedServerBackend& Graph::backend()const{impl_->affinity();PreserveError error;std::lock_guard lock(impl_->metadata);if(!impl_->backend)throw std::logic_error("Application server no longer exists");return *impl_->backend;}
} // namespace anadius::application_graph
