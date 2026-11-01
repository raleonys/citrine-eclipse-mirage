#include "environment.hpp"
#include <algorithm>
#include <atomic>
#include <exception>
#include <limits>
#include <list>
#include <mutex>
#include <optional>
#include <type_traits>

namespace anadius::owned_environment {
namespace {
struct PreserveError {DWORD value=::GetLastError();~PreserveError(){::SetLastError(value);}};
s::IoHandlerExecutor& empty_executor(){static s::IoHandlerExecutor value{};return value;}
struct Empty final:s::IoHandlerVtable {
    Empty():IoHandlerVtable(0x180017a30ULL){}
    void destroy_slot_00(s::IoHandlerEnvironment& out)override{out={};out.operations_10=this;out.executor_28=&empty_executor();}
    void move_slot_10(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from)override{
        out.payload_00={};out.state_18=0;from.payload_00={};from.state_18=0;
    }
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{throw s::UnsupportedInput("Empty owned environment has no query capability");}
};
Empty& empty(){static Empty value;return value;}
s::IoHandlerSentinels sentinels(){return {&empty(),0,&empty_executor(),empty().destroy_identity};}
// Opaque selected-policy tokens, never dereferenced RTTI/code addresses.
std::uintptr_t active_type(){return 1;}
std::uintptr_t empty_type(){return 0x1800a25c0ULL;}
std::uintptr_t next_capability(){
    static std::atomic<std::uintptr_t> next{1};auto value=next.load();
    for(;;){if(value==std::numeric_limits<std::uintptr_t>::max())throw std::overflow_error("Environment capabilities exhausted");if(next.compare_exchange_weak(value,value+1))return value;}
}
bool same(s::IoHandlerSentinels a,s::IoHandlerSentinels b){return a.empty_operations_846c8==b.empty_operations_846c8&&a.empty_schedule_846e8==b.empty_schedule_846e8&&a.empty_executor_84770==b.empty_executor_84770&&a.guard_destroy_017a30==b.guard_destroy_017a30;}
}
struct Environment::Control:std::enable_shared_from_this<Control> {
    struct Row {s::IoHandlerEnvironment* address=nullptr;std::uintptr_t cap=0,state=0;};
    mutable std::mutex gate;Phase phase=Phase::Fresh;bool binding=false,checking=false;
    const std::uintptr_t token=next_capability();std::vector<Row> rows;
    std::size_t refs=0,active=0;std::uint64_t created=0,moved=0,copied=0,destroyed=0,handlers=0,responses=0;
    DWORD thread=0;void* fiber=nullptr;std::optional<rt::Ports> ports;
    std::shared_ptr<void> lease;std::function<void()> checkpoint;
    struct Registry {std::mutex gate;std::list<std::pair<std::uintptr_t,std::weak_ptr<Control>>> entries;};
    static Registry& registry(){static Registry value;return value;}
    static void register_owner(const std::shared_ptr<Control>& c){
        auto& r=registry();decltype(r.entries) prepared;prepared.emplace_back(c->token,c);
        std::lock_guard lock(r.gate);r.entries.splice(r.entries.end(),prepared);
    }
    static void unregister_owner(std::uintptr_t token){
        auto& r=registry();decltype(r.entries) retired;
        {std::lock_guard lock(r.gate);for(auto at=r.entries.begin();at!=r.entries.end();++at)if(at->first==token){retired.splice(retired.end(),r.entries,at);break;}}
        // Weak control/node retirement and allocator effects occur outside gate.
    }
    static std::shared_ptr<Control> owner(std::uintptr_t token){
        auto& r=registry();std::lock_guard lock(r.gate);auto at=std::find_if(r.entries.begin(),r.entries.end(),[=](const auto& item){return item.first==token;});
        if(at==r.entries.end())throw s::UnsupportedInput("Stale or foreign environment owner token");
        auto c=at->second.lock();if(!c)throw s::UnsupportedInput("Retired environment owner");return c;
    }
    static std::shared_ptr<Control> query_owner(std::uintptr_t cap){
        auto& r=registry();std::lock_guard lock(r.gate);
        for(const auto& entry:r.entries)if(auto c=entry.second.lock()){
            std::lock_guard own(c->gate);if(c->phase!=Phase::Bound)continue;
            for(const auto& row:c->rows)if(row.address&&row.cap==cap)return c;
        }
        throw s::UnsupportedInput("Stale or foreign environment query capability");
    }
    explicit Control(std::size_t capacity):rows(capacity){}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Closed)phase=Phase::Poisoned;}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Environment requires its bound worker/fiber");}
    void probe(Phase expected){
        {PreserveError error;std::lock_guard lock(gate);if(checking||phase!=expected){if(phase!=Phase::Closed)phase=Phase::Poisoned;throw std::logic_error("Environment admission probe reentry");}checking=true;}
        try{checkpoint();}catch(...){PreserveError error;std::lock_guard lock(gate);checking=false;if(phase!=Phase::Closed)phase=Phase::Poisoned;throw;}
        {PreserveError error;std::lock_guard lock(gate);checking=false;if(phase!=expected)throw std::logic_error("Environment admission probe was poisoned");}
    }
    void admission(bool cleanup){
        {PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound)throw std::logic_error("Environment source is closed or poisoned");affinity();}
        probe(Phase::Bound);
        {PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Bound)throw std::logic_error("Environment admission was poisoned");}
        (void)ports->view.memory.buffer_context();const auto backend=ports->view.backend.snapshot().phase;
        if((backend!=sp::o::Phase::Ready&&backend!=sp::o::Phase::Running&&!(cleanup&&(backend==sp::o::Phase::Returned||backend==sp::o::Phase::Quiescing)))||
           ports->view.io.snapshot().phase!=s::IoMemoryPhase::Open||ports->accept.snapshot().poisoned)
            throw std::logic_error("Environment backend/IO admission is unavailable");
    }
    template<class F>std::invoke_result_t<F> perform(F&& fn,bool cleanup=false){
        auto pin=shared_from_this();
        {PreserveError error;std::lock_guard lock(gate);if(checking){phase=Phase::Poisoned;throw std::logic_error("Environment source cannot reenter its admission probe");}if(phase!=Phase::Bound){if(phase==Phase::Closing||binding)phase=Phase::Poisoned;throw std::logic_error("Environment is unbound or closed");}affinity();++active;}
        struct Leave {Control& c;~Leave(){PreserveError error;std::lock_guard lock(c.gate);--c.active;}} leave{*this};
        try{admission(cleanup);if constexpr(std::is_void_v<std::invoke_result_t<F>>){std::forward<F>(fn)();admission(cleanup);}
            else{decltype(auto) result=std::forward<F>(fn)();admission(cleanup);return result;}}
        catch(...){poison();throw;}
    }
    Row& row(s::IoHandlerEnvironment& value,bool full){
        for(auto& r:rows)if(r.address==&value){
            if(value.payload_00[0]!=token||value.payload_00[1]!=r.cap||value.state_18!=r.state||
               (full&&(value.operations_10!=&table||value.schedule_20!=token||value.executor_28!=&executor))||
               (!full&&(value.operations_10!=&empty()||value.schedule_20!=0||value.executor_28!=&executor)))
                throw s::UnsupportedInput("Environment projection/capability fields changed");
            return r;
        }
        throw s::UnsupportedInput("Environment projection is not registered at this address");
    }
    std::uintptr_t query_state(std::uintptr_t cap){for(const auto& r:rows)if(r.address&&r.cap==cap)return r.state;throw s::UnsupportedInput("Expired environment query generation");}
    void fresh(s::IoHandlerEnvironment& out,bool partial){
        for(const auto& r:rows)if(r.address==&out)throw s::UnsupportedInput("Environment output already owns a projection");
        if(partial){if(out.operations_10!=&table||out.schedule_20!=token)throw s::UnsupportedInput("Environment output has a foreign native copy/move prefix");}
        else if(out.payload_00[0]||(out.operations_10&&out.operations_10!=&empty()))throw s::UnsupportedInput("Environment output is not fresh");
    }
    void make(s::IoHandlerEnvironment& out,std::uintptr_t state){
        std::lock_guard lock(gate);fresh(out,false);Row* target=nullptr;for(auto& r:rows)if(!r.address){target=&r;break;}
        if(!target)throw std::length_error("Owned environment reference capacity exhausted");
        const auto cap=next_capability();*target={&out,cap,state};++refs;++created;out={{token,cap},&table,state,token,&executor};
    }
    struct Table final:s::IoHandlerVtable {
        Control& c;explicit Table(Control& value):IoHandlerVtable(1),c(value){}
        void destroy_slot_00(s::IoHandlerEnvironment& value)override{c.perform([&]{std::lock_guard lock(c.gate);auto& r=c.row(value,true);r={};--c.refs;++c.destroyed;value={};value.operations_10=&empty();value.executor_28=&empty_executor();},true);}
        void move_slot_10(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from)override{c.perform([&]{
            std::lock_guard lock(c.gate);c.fresh(out,true);auto& r=c.row(from,false);
            out.payload_00=from.payload_00;out.state_18=from.state_18;r.address=&out;++c.moved;from.payload_00={};from.state_18=0;
        });}
        std::uintptr_t query_slot_18(s::IoHandlerEnvironment& value)override{return c.perform([&]{std::lock_guard lock(c.gate);return c.row(value,true).cap;});}
    } table{*this};
    struct Types final:s::IoEnvironmentTypeApi {
        std::weak_ptr<Control> control;explicit Types(std::weak_ptr<Control> c):control(std::move(c)){}
        std::int32_t compare_type_names_04d2d0(std::uintptr_t first,std::uintptr_t second)override{
            auto c=control.lock();if(!c)throw s::UnsupportedInput("Retired environment type provider");
            return c->perform([&]{if(second!=0x1800a25c8ULL)throw s::UnsupportedInput("Unknown environment type comparison");if(first==empty_type()+8)return 0;if(first==active_type()+8)return -1;throw s::UnsupportedInput("Unknown environment type token");});
        }
    };
    static void prepare(s::IoHandlerEnvironment& out,std::uintptr_t query,s::IoPrepareScratch&){construct(out,query);}
    static void factory(s::IoHandlerEnvironment& out,std::uintptr_t query,s::IoEnvironmentConstructorScratch&){construct(out,query);}
    static void response20(s::IoHandlerEnvironment& out,std::uintptr_t query,std::uintptr_t literal){if(literal!=0x180084425ULL)throw s::UnsupportedInput("Response factory literal mismatch");construct(out,query);}
    static void response88(s::IoHandlerEnvironment& out,std::uintptr_t query,s::ResponseExecutorScratch&){construct(out,query);}
    static void construct(s::IoHandlerEnvironment& out,std::uintptr_t query){auto c=query_owner(query);c->perform([&]{std::uintptr_t state;{std::lock_guard lock(c->gate);state=c->query_state(query);}c->make(out,state);});}
    static void copy(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from){auto c=owner(from.payload_00[0]);c->perform([&]{
        std::lock_guard lock(c->gate);c->fresh(out,true);const auto state=c->row(from,true).state;Row* target=nullptr;for(auto& r:c->rows)if(!r.address){target=&r;break;}
        if(!target)throw std::length_error("Owned environment copy capacity exhausted");const auto cap=next_capability();*target={&out,cap,state};++c->refs;++c->copied;
        out.payload_00={c->token,cap};out.state_18=state;
    });}
    static void handler(s::IoHandlerEnvironment& env,s::OwnedHandlerTask*& slot){auto c=owner(env.payload_00[0]);c->perform([&]{
        {std::lock_guard lock(c->gate);(void)c->row(env,true);++c->handlers;}
        auto* const task=slot;if(!task)throw s::UnsupportedInput("Owned handler slot is empty");slot=nullptr;task->call_slot_00(1);
        // Neither task nor environment projection may be inspected after call.
    });}
    static void response(s::IoHandlerEnvironment& env,s::PostedFunctor*& slot){auto c=owner(env.payload_00[0]);c->perform([&]{
        {std::lock_guard lock(c->gate);(void)c->row(env,true);++c->responses;}
        if(!slot)throw s::UnsupportedInput("Owned response slot is empty");auto& p=*c->ports;
        s::PostedExecutor selected{p.view.identity.value,&p.view.scheduler,1};
        s::dispatch_posted_022e40(selected,slot,p.environment.storage,p.worker,p.completion,p.memory.posted());
    });}
    s::IoHandlerExecutor executor{prepare};s::HandlerSchedule handlers_table{handler,nullptr};s::ResponseSchedule responses_table{response,nullptr};s::AcceptSchedule accepts_table{handler,nullptr};
    Snapshot snapshot()const{PreserveError error;std::lock_guard lock(gate);return {phase,refs,active,created,moved,copied,destroyed,handlers,responses,token};}
};
Environment::Environment(std::size_t capacity){if(!capacity||capacity>1048576)throw std::invalid_argument("Bounded environment capacity required");control_=std::make_shared<Control>(capacity);Control::register_owner(control_);}
Environment::~Environment()noexcept{
    auto c=control_;{PreserveError error;std::lock_guard lock(c->gate);if(c->active||c->refs||c->binding||(c->phase!=Phase::Closed&&c->phase!=Phase::Fresh))std::terminate();c->phase=Phase::Closed;}
    Control::unregister_owner(c->token);
}
sp::Environment Environment::configuration()const{
    auto c=control_;{PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Fresh||c->binding)throw std::logic_error("Environment configuration exports before binding only");}
    sp::Environment out;out.sentinels=sentinels();out.types=std::make_shared<Control::Types>(c);out.queries={{c->token,active_type},{0,empty_type}};
    out.factories={{&c->executor,Control::factory}};out.handlers={{c->token,&c->handlers_table}};out.response_executors={{&c->executor,Control::response20,Control::response88}};
    out.responses={{c->token,&c->responses_table}};out.copies={{&c->table,Control::copy}};out.accepts={{c->token,&c->accepts_table}};return out;
}
void Environment::bind(rt::Ports ports,std::function<void()> checkpoint){
    auto c=control_;{PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Fresh||c->binding){if(c->binding||c->active||c->phase==Phase::Closing)c->phase=Phase::Poisoned;throw std::logic_error("Environment binds once");}c->binding=true;}
    try{
        if(!checkpoint||!same(ports.environment.sentinels,sentinels())||!ports.server_identity||!ports.view.identity.value)throw std::invalid_argument("Exact owned sentinels, stable Ports and source admission required");
        const auto bindings=ports.accept.storage_bindings();if(bindings.data()!=ports.environment.storage.storage.data()||bindings.size()!=ports.environment.storage.storage.size())throw std::invalid_argument("Environment storage belongs to another source graph");
        (void)ports.view.memory.buffer_context();const auto phase=ports.view.backend.snapshot().phase;
        if((phase!=sp::o::Phase::Ready&&phase!=sp::o::Phase::Running)||ports.view.io.snapshot().phase!=s::IoMemoryPhase::Open)throw std::logic_error("Environment requires an open owned source graph");
        c->ports.emplace(ports);c->checkpoint=std::move(checkpoint);checkpoint={};
        {PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Fresh||!c->binding)throw std::logic_error("Environment bind target retirement was poisoned");}
        c->lease=ports.view.memory.acquire_provider_lease_from_operation();
        {PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Fresh||!c->binding)throw std::logic_error("Environment bind was poisoned");c->thread=::GetCurrentThreadId();c->fiber=::GetCurrentFiber();c->binding=false;c->phase=Phase::Bound;}
    }catch(...){c->poison();{PreserveError error;std::lock_guard lock(c->gate);c->binding=false;}throw;}
}
void Environment::session(s::IoHandlerEnvironment& out,sp::SessionView view){auto c=control_;c->perform([&]{auto& p=*c->ports;auto* control=view.owner.weak_self.control_08;
    if(!p.view.memory.owns_control(control)||!control->strong_08.load()||&static_cast<s::SessionControl*>(control)->object!=&view.owner||view.owner.weak_self.object_00!=view.owner.identity())throw std::invalid_argument("Environment session must be an exact live Parent owner");c->make(out,1);
});}
void Environment::accept(s::IoHandlerEnvironment& out,std::uintptr_t allocator){auto c=control_;c->perform([&]{if(allocator!=c->ports->server_identity+0x68)throw std::invalid_argument("Foreign accept allocator token");c->make(out,0);});}
void Environment::release(s::IoHandlerEnvironment& value){if(value.operations_10==&empty()){empty().destroy_slot_00(value);return;}auto c=control_;if(value.operations_10!=&c->table)throw s::UnsupportedInput("Foreign environment table");c->table.destroy_slot_00(value);}
bool Environment::close(bool attested){
    auto c=control_;{PreserveError error;std::lock_guard lock(c->gate);if(c->phase==Phase::Closed)return true;if(c->active||c->binding||c->phase==Phase::Closing){c->phase=Phase::Poisoned;throw std::logic_error("Environment lifecycle reentry");}if(c->phase!=Phase::Bound)throw std::logic_error("Environment cannot close an unknown/unbound state");c->affinity();if(!attested||c->refs)return false;c->phase=Phase::Closing;}
    try{
        c->probe(Phase::Closing);auto& p=*c->ports;(void)p.view.memory.buffer_context();const auto phase=p.view.backend.snapshot().phase;
        const auto io=p.view.io.snapshot();const auto memory=p.view.memory.snapshot();const auto accept=p.accept.snapshot();
        const bool drained=(phase==sp::o::Phase::Returned||phase==sp::o::Phase::Quiescing)&&io.phase==s::IoMemoryPhase::Open&&!io.allocations&&!io.pending&&!io.active_calls&&
            memory.phase==s::SessionMemoryPhase::Open&&!memory.allocations&&!memory.controls&&!memory.proxies&&!memory.blocks&&!memory.maps&&!memory.buffers&&!memory.reserved&&!memory.pending_frees&&!memory.partial_constructions&&!memory.buffer_borrows&&
            !accept.poisoned&&!accept.projections&&!accept.pending&&!accept.active_calls&&!p.view.scheduler.work_count_30.load()&&!p.view.scheduler.completion.head&&!p.view.scheduler.completion.tail;
        {PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Closing)throw std::logic_error("Environment close was poisoned");if(!drained){c->phase=Phase::Bound;return false;}}
        Control::unregister_owner(c->token);
        {auto retiring=std::move(c->checkpoint);c->checkpoint={};}
        {PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Closing)throw std::logic_error("Environment checkpoint retirement was poisoned");}
        c->lease.reset();c->ports.reset();
        {PreserveError error;std::lock_guard lock(c->gate);if(c->phase!=Phase::Closing)throw std::logic_error("Environment provider retirement was poisoned");c->phase=Phase::Closed;}return true;
    }catch(...){c->poison();throw;}
}
Snapshot Environment::snapshot()const{return control_->snapshot();}
} // namespace anadius::owned_environment
