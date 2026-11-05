#include "environment.hpp"
#include "../anadius-application-io-source/call.hpp"
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
std::uintptr_t tracked_type(){return 0x1800a2690ULL;}
std::uintptr_t next_capability(){
    static std::atomic<std::uintptr_t> next{1};auto value=next.load();
    for(;;){if(value==std::numeric_limits<std::uintptr_t>::max())throw std::overflow_error("Environment capabilities exhausted");if(next.compare_exchange_weak(value,value+1))return value;}
}
bool same(s::IoHandlerSentinels a,s::IoHandlerSentinels b){return a.empty_operations_846c8==b.empty_operations_846c8&&a.empty_schedule_846e8==b.empty_schedule_846e8&&a.empty_executor_84770==b.empty_executor_84770&&a.guard_destroy_017a30==b.guard_destroy_017a30;}
struct Origin {
    sp::SessionToken session;std::uintptr_t owner=0;s::IoReference* control=nullptr;
    bool operator==(const Origin&)const=default;
};
}
struct Environment::JobState {
    Control& control;std::shared_ptr<void> lease;s::SessionMemoryDomain* const parent;
    std::optional<s::SessionMemoryDomain::JobPermit> permit;const Origin origin;
    const s::IoHandlerEnvironment* const source;const std::uintptr_t source_cap,epoch;
    const DWORD thread=::GetCurrentThreadId();void* const fiber=::GetCurrentFiber();
    bool open=false,preparing=true,closing=false,checking=false,failed=false,closed=false;
    // Address ownership only: sentinel results have no native row/capability,
    // work pin, environments/ref count, or replacement business object.
    std::list<s::IoHandlerEnvironment*> empty_results;
    std::function<void()> checkpoint;std::optional<application_io::CallContext> context;
    struct Operations final:s::IoEnvironmentCallOperations {
        JobState& state;explicit Operations(JobState& s):state(s){}
        std::int32_t compare_type_names_04d2d0(std::uintptr_t,std::uintptr_t)override;
        std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override;
        void destroy_slot_00(s::IoHandlerEnvironment&)override;
        void construct_slot_58(s::IoEnvironmentFactory,s::IoHandlerEnvironment&,std::uintptr_t,s::IoEnvironmentConstructorScratch&)override;
        s::IoHandlerEnvironment& move_023cb0(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&,s::IoHandlerSentinels)override;
    } operations{*this};
    JobState(Control& c,std::shared_ptr<void> l,s::SessionMemoryDomain& p,const s::SessionMemoryDomain::JobPermit& proof,Origin o,const s::IoHandlerEnvironment& from,std::uintptr_t cap,std::uintptr_t id)
        :control(c),lease(std::move(l)),parent(&p),permit(proof),origin(o),source(&from),source_cap(cap),epoch(id){}
    static void invoke_checkpoint(void* raw){static_cast<JobState*>(raw)->checkpoint();}
};
struct Environment::Control:std::enable_shared_from_this<Control> {
    enum class Family {Host,Initial,Tracked};enum class Pin {None,Owned,ReleaseEntered};
    enum class Transform {Copy,Prepare,Factory,Response20,Response88};
    struct Row {
        s::IoHandlerEnvironment* address=nullptr;std::uintptr_t cap=0,state=0;Origin origin;std::uintptr_t job_epoch=0;
        Family family=Family::Host;std::uint32_t flags=0;Pin pin=Pin::None;bool release_consumed=false;
    };
    enum class Shape {None,Full,Partial,Fresh,MoveDestination,FullOrEmpty};
    struct Claim {s::IoHandlerEnvironment* address=nullptr;bool write=false;};
    struct Seed {s::IoHandlerEnvironment* address=nullptr;bool write=false;Shape shape=Shape::None;};
    // Fixed bound of the measured023dd0 call, not a registry/peer/job bound.
    // Source, output and its at most two native temporary addresses use four.
    static constexpr std::size_t algorithm_claims=8;
    struct Admission {Admission* next=nullptr;JobState* scope=nullptr;DWORD thread=0;void* fiber=nullptr;std::array<Claim,algorithm_claims> claims{};std::size_t count=0;bool partial_slot=false;};
    struct ScopeAdmission {ScopeAdmission* next=nullptr;DWORD thread=0;void* fiber=nullptr;const s::IoHandlerEnvironment* source=nullptr;std::uintptr_t cap=0;};
    struct Effect {
        Control& control;JobState* const scope;const DWORD thread;void* const fiber;
        std::array<Claim,algorithm_claims> claims{};std::size_t count=0;
        bool open=true,checking=false,failed=false,cleanup=false,allow_nested=false;const bool partial_slot;
        bool release_entered=false,consumed=false;Row release;
        s::IoHandlerEnvironment* copy_output=nullptr; // raw slot08 caller still owns its live executor reload
        Effect(Control& c,const Admission& a,bool clean):control(c),scope(a.scope),thread(a.thread),fiber(a.fiber),claims(a.claims),count(a.count),cleanup(clean),partial_slot(a.partial_slot){}
    };
    mutable std::mutex gate;Phase phase=Phase::Fresh;bool binding=false,concurrent=false,close_checking=false;
    const ClassifierProfile classifier;const std::uintptr_t token=next_capability(),tracked_token;
    std::list<Row> rows;std::list<std::shared_ptr<JobState>> scopes;std::list<std::shared_ptr<Effect>> effects;
    Admission* admissions=nullptr;ScopeAdmission* scope_admissions=nullptr;
    std::size_t refs=0;std::uint64_t created=0,moved=0,copied=0,destroyed=0,handlers=0,responses=0;
    DWORD thread=0;void* fiber=nullptr;std::optional<rt::Ports> ports;std::shared_ptr<void> lease;std::function<void()> checkpoint;
    template<class Error=std::logic_error>[[noreturn]] static void reject(std::unique_lock<std::mutex>& lock,const char* text){lock.unlock();throw Error(text);}
    template<class Error=std::logic_error>[[noreturn]] void invalid(std::unique_lock<std::mutex>& lock,const char* text){if(phase!=Phase::Closed)phase=Phase::Poisoned;reject<Error>(lock,text);}
    struct Registry {std::mutex gate;std::list<std::pair<std::uintptr_t,std::weak_ptr<Control>>> entries;};
    static Registry& registry(){static Registry value;return value;}
    static void register_owner(const std::shared_ptr<Control>& c){auto& r=registry();decltype(r.entries) prepared;prepared.emplace_back(c->token,c);std::lock_guard lock(r.gate);r.entries.splice(r.entries.end(),prepared);}
    static void unregister_owner(std::uintptr_t token){auto& r=registry();decltype(r.entries) retired;{std::lock_guard lock(r.gate);for(auto at=r.entries.begin();at!=r.entries.end();++at)if(at->first==token){retired.splice(retired.end(),r.entries,at);break;}}}
    static std::shared_ptr<Control> owner(std::uintptr_t token){
        auto& r=registry();std::weak_ptr<Control> selected;
        {std::lock_guard lock(r.gate);for(const auto& item:r.entries)if(item.first==token){selected=item.second;break;}}
        auto c=selected.lock();if(!c)throw s::UnsupportedInput("Stale or foreign environment owner token");return c;
    }
    static std::shared_ptr<Control> query_owner(std::uintptr_t cap){
        // Retain no nested registry gate while scanning an owner's metadata;
        // weak/shared target destruction and failures occur outside both gates.
        auto& r=registry();std::uintptr_t cursor=0;
        for(;;){std::weak_ptr<Control> selected;std::uintptr_t next=0;
            {std::lock_guard lock(r.gate);for(const auto& item:r.entries)if(item.first>cursor&&(!next||item.first<next)){next=item.first;selected=item.second;}}
            if(!next)break;cursor=next;auto c=selected.lock();if(!c)continue;
            {std::lock_guard lock(c->gate);if(c->phase==Phase::Bound)for(const auto& row:c->rows)if(row.address&&row.cap==cap)return c;}
        }
        throw s::UnsupportedInput("Stale or foreign environment query capability");
    }
    explicit Control(std::size_t capacity,ClassifierProfile profile):classifier(profile),tracked_token(profile==ClassifierProfile::OriginalDefault?next_capability():0),rows(capacity){}
    void poison(){PreserveError error;std::lock_guard lock(gate);if(phase!=Phase::Closed)phase=Phase::Poisoned;}
    void affinity()const{if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())throw std::logic_error("Environment requires its bound worker/fiber");}
    Row* at(const s::IoHandlerEnvironment* value)noexcept{for(auto& row:rows)if(row.address==value)return &row;return nullptr;}
    bool scope_member(const JobState* state)const noexcept{for(const auto& item:scopes)if(item.get()==state)return true;return false;}
    bool effect_member(const Effect* effect)const noexcept{for(const auto& item:effects)if(item.get()==effect)return true;return false;}
    bool protected_source(const s::IoHandlerEnvironment* value)const noexcept{
        for(auto* a=scope_admissions;a;a=a->next)if(a->source==value)return true;
        for(const auto& s:scopes)if(s->source==value)return true;return false;
    }
    JobState* empty_owner(const s::IoHandlerEnvironment* value)const noexcept{for(const auto& scope:scopes)for(auto* address:scope->empty_results)if(address==value)return scope.get();return nullptr;}
    static bool canonical_empty(const s::IoHandlerEnvironment& value)noexcept{return value.operations_10==&empty()&&value.payload_00[0]==0&&value.payload_00[1]==0&&!value.state_18&&!value.schedule_20&&value.executor_28==&empty_executor();}
    std::uintptr_t schedule(const Row& r)const noexcept{return r.family==Family::Tracked?tracked_token:token;}
    s::IoHandlerExecutor* execution(const Row& r)noexcept{return r.family==Family::Tracked?&tracked_executor:&executor;}
    bool full(const Row& r)const noexcept{const auto& v=*r.address;return v.payload_00[0]==token&&v.payload_00[1]==r.cap&&v.state_18==r.state&&v.operations_10==&table&&v.schedule_20==schedule(r)&&v.executor_28==(r.family==Family::Tracked?&tracked_executor:&executor);}
    bool partial(const Row& r)const noexcept{const auto& v=*r.address;return v.payload_00[0]==token&&v.payload_00[1]==r.cap&&v.state_18==r.state&&v.operations_10==&empty()&&!v.schedule_20&&v.executor_28==(r.family==Family::Tracked?&tracked_executor:&executor);}
    bool copy_prefix(const Row& r)const noexcept{const auto& v=*r.address;return v.payload_00[0]==token&&v.payload_00[1]==r.cap&&v.state_18==r.state&&v.operations_10==&table&&v.schedule_20==schedule(r);}
    void authority(std::unique_lock<std::mutex>& lock,JobState* scope,const Row& r,bool write){
        if(scope){if(r.origin!=scope->origin||(r.cap!=scope->source_cap&&r.job_epoch!=scope->epoch))reject<s::UnsupportedInput>(lock,"Environment row is outside exact scope lineage");if(write&&r.cap==scope->source_cap)reject<s::UnsupportedInput>(lock,"A scope cannot mutate its protected source");}
        else if(r.job_epoch)reject<s::UnsupportedInput>(lock,"Root operation cannot access scoped temporaries");
        if(write&&protected_source(r.address))reject<s::UnsupportedInput>(lock,"Live scope protects this environment source");
        if(r.pin==Pin::ReleaseEntered)reject<s::UnsupportedInput>(lock,"Environment work release already entered");
    }
    static bool conflicts(const Claim& a,const Claim& b)noexcept{return a.address&&a.address==b.address&&(a.write||b.write);}
    void claim(std::unique_lock<std::mutex>& lock,JobState* scope,std::array<Claim,algorithm_claims>& claims,std::size_t& count,s::IoHandlerEnvironment& value,bool write,const Effect* self=nullptr,const Admission* pending=nullptr){
        Claim wanted{&value,write};
        if(auto* owner=empty_owner(&value);owner&&owner!=scope)reject<s::UnsupportedInput>(lock,"Environment sentinel result belongs to another scope");
        for(auto* p=admissions;p;p=p->next)if(p!=pending)for(std::size_t i=0;i<p->count;++i)if(conflicts(wanted,p->claims[i]))reject(lock,"Environment address has another pending effect");
        for(const auto& p:effects)if(p.get()!=self)for(std::size_t i=0;i<p->count;++i)if(conflicts(wanted,p->claims[i]))reject(lock,"Environment address has another live effect");
        if(auto* row=at(&value))authority(lock,scope,*row,write);
        else if(empty_owner(&value)&&!canonical_empty(value))invalid<s::UnsupportedInput>(lock,"Scoped sentinel result fields changed");
        for(std::size_t i=0;i<count;++i)if(claims[i].address==&value){claims[i].write|=write;return;}
        if(count==claims.size())invalid<s::UnsupportedInput>(lock,"Measured constructor effect exceeded address budget");claims[count++]=wanted;
    }
    void shape(std::unique_lock<std::mutex>& lock,const Seed& seed){
        auto* r=at(seed.address);auto& value=*seed.address;
        if(seed.shape==Shape::Full||seed.shape==Shape::Partial){if(!r)reject<s::UnsupportedInput>(lock,"Environment input address is not registered");if(!(seed.shape==Shape::Full?full(*r):partial(*r)))invalid<s::UnsupportedInput>(lock,"Environment projection fields changed");}
        else if(seed.shape==Shape::FullOrEmpty){if(r){if(!full(*r))invalid<s::UnsupportedInput>(lock,"Environment projection fields changed");}else if(!canonical_empty(value))reject<s::UnsupportedInput>(lock,"Scoped input is neither owned projection nor canonical empty");}
        else if(seed.shape==Shape::Fresh||seed.shape==Shape::MoveDestination){if(r||empty_owner(seed.address))reject<s::UnsupportedInput>(lock,"Environment destination already owns a projection");if(seed.shape==Shape::Fresh&&(value.payload_00[0]||(value.operations_10&&value.operations_10!=&empty())))invalid<s::UnsupportedInput>(lock,"Environment factory output is not fresh");}
    }
    bool actor_busy(DWORD actor,void* actor_fiber)const noexcept{
        for(auto* a=admissions;a;a=a->next)if(a->thread==actor&&a->fiber==actor_fiber)return true;
        for(auto* a=scope_admissions;a;a=a->next)if(a->thread==actor&&a->fiber==actor_fiber)return true;
        for(const auto& e:effects)if(e->thread==actor&&e->fiber==actor_fiber&&(!e->allow_nested||e->checking))return true;
        for(const auto& s:scopes)if(s->thread==actor&&s->fiber==actor_fiber&&(s->preparing||s->closing||s->checking))return true;return false;
    }
    void scope_metadata(std::unique_lock<std::mutex>& lock,JobState& scope,bool preparing=false,bool closing=false){
        if(scope.thread!=::GetCurrentThreadId()||scope.fiber!=::GetCurrentFiber())reject(lock,"Environment scope belongs to another actor");
        if(phase==Phase::Bound&&scope_member(&scope)&&!scope.failed&&!scope.closed&&((!preparing&&scope.preparing)||(!closing&&scope.closing)))invalid(lock,"Environment scope actor cannot reenter preparation/retirement");
        if(phase!=Phase::Bound||binding||!scope_member(&scope)||scope.failed||scope.closed||(!preparing&&scope.preparing)||(!closing&&scope.closing)||(!preparing&&!closing&&!scope.open))reject(lock,"Environment scope admission ended or was poisoned");
        auto* row=at(scope.source);if(!row||row->cap!=scope.source_cap||row->origin!=scope.origin||!full(*row))invalid(lock,"Protected environment source changed");
    }
    void source(bool cleanup){auto& p=*ports;(void)p.view.memory.buffer_context();const auto b=p.view.backend.snapshot().phase;if((b!=sp::o::Phase::Ready&&b!=sp::o::Phase::Running&&!(cleanup&&(b==sp::o::Phase::Returned||b==sp::o::Phase::Quiescing)))||p.view.io.snapshot().phase!=s::IoMemoryPhase::Open||p.accept.snapshot().poisoned)throw std::logic_error("Environment source graph is unavailable");if(classifier==ClassifierProfile::OriginalDefault&&(!p.environment_work_stop||!p.environment_work_stop->matches(p.view.memory,p.view.scheduler)))throw std::logic_error("Environment work capability changed");}
    void scope_validate(JobState& scope,bool preparing=false,bool closing=false){
        {std::unique_lock lock(gate);scope_metadata(lock,scope,preparing,closing);}
        if(!scope.context)throw std::logic_error("Environment scope has no immutable call context");scope.context->validate();
        if(!scope.parent->owns_control(scope.origin.control)||!scope.origin.control->strong_08.load())throw std::logic_error("Environment scope session is no longer live");
        const auto& owner=static_cast<s::SessionControl*>(scope.origin.control)->object;if(owner.identity()!=scope.origin.owner||owner.weak_self.control_08!=scope.origin.control||owner.weak_self.object_00!=scope.origin.owner)throw std::logic_error("Environment scope owner/control changed");
        source(true);scope.context->validate();
        {std::unique_lock lock(gate);scope_metadata(lock,scope,preparing,closing);}
    }
    void metadata(Effect& effect){std::unique_lock lock(gate);if(phase!=Phase::Bound||binding||!effect_member(&effect)||!effect.open||effect.failed||effect.thread!=::GetCurrentThreadId()||effect.fiber!=::GetCurrentFiber())invalid(lock,"Environment effect admission ended or was poisoned");if(effect.scope)scope_metadata(lock,*effect.scope);else if(thread!=effect.thread||fiber!=effect.fiber)invalid(lock,"Raw environment callback requires original actor");}
    void validate_claims(Effect& e){std::unique_lock lock(gate);if(phase!=Phase::Bound||e.failed)invalid(lock,"Environment claimed source was poisoned");if(!e.partial_slot)for(std::size_t i=0;i<e.count;++i){if(auto* r=at(e.claims[i].address);r&&!(e.copy_output==r->address?copy_prefix(*r):full(*r)))invalid(lock,"Environment claimed projection changed across callback");if(empty_owner(e.claims[i].address)&&!canonical_empty(*e.claims[i].address))invalid(lock,"Scoped sentinel result changed across callback");}}
    void admit(Effect& effect){
        PreserveError error;metadata(effect);validate_claims(effect);if(effect.scope)scope_validate(*effect.scope);else{affinity();(void)ports->view.memory.buffer_context();}
        {std::unique_lock lock(gate);if(effect.checking)invalid(lock,"Environment checkpoint cannot reenter");effect.checking=true;}
        try{if(effect.scope)effect.scope->context->check();else checkpoint();source(effect.cleanup);metadata(effect);validate_claims(effect);}
        catch(...){std::lock_guard lock(gate);effect.checking=false;effect.failed=true;if(phase!=Phase::Closed)phase=Phase::Poisoned;throw;}
        {std::unique_lock lock(gate);effect.checking=false;if(phase!=Phase::Bound||effect.failed)invalid(lock,"Environment checkpoint return was poisoned");}
    }
    template<class F>decltype(auto) run(JobState* scope,std::initializer_list<Seed> seeds,bool cleanup,F&& fn){
        PreserveError error;auto pin=shared_from_this();Admission pending;pending.scope=scope;pending.thread=::GetCurrentThreadId();pending.fiber=::GetCurrentFiber();
        {std::unique_lock lock(gate);if(phase!=Phase::Bound||binding){if((binding||phase==Phase::Closing)&&thread==pending.thread&&fiber==pending.fiber)invalid(lock,"Environment actor cannot reenter binding/retirement");reject(lock,"Environment is unbound/closed/poisoned");}if(scope)scope_metadata(lock,*scope);else if(thread!=pending.thread||fiber!=pending.fiber)reject(lock,"Raw environment callback requires original actor");if(actor_busy(pending.thread,pending.fiber))invalid(lock,"Environment actor cannot reenter its pending effect");
            if(!concurrent){for(const auto& e:effects)if(e->thread!=pending.thread||e->fiber!=pending.fiber)reject(lock,"Serialized environment has another actor");if(scope_admissions)reject(lock,"Serialized environment scope preparation overlaps");}
            for(const auto& seed:seeds){claim(lock,scope,pending.claims,pending.count,*seed.address,seed.write);shape(lock,seed);pending.partial_slot|=seed.shape==Shape::Partial;}pending.next=admissions;admissions=&pending;}
        struct Pending {Control& c;Admission& a;bool linked=true;void remove()noexcept{auto** at=&c.admissions;while(*at&&*at!=&a)at=&(*at)->next;if(!*at)std::terminate();*at=a.next;linked=false;}~Pending(){if(linked){std::lock_guard lock(c.gate);remove();}}} admission{*this,pending};
        std::list<std::shared_ptr<Effect>> prepared,retired;std::shared_ptr<Effect> effect;
        try{prepared.emplace_back();effect=std::make_shared<Effect>(*this,pending,cleanup);prepared.front()=effect;
            {std::unique_lock lock(gate);effects.splice(effects.end(),prepared);admission.remove();if(phase!=Phase::Bound)invalid(lock,"Environment effect allocation was poisoned");}
            admit(*effect);
            const auto finish=[&]{admit(*effect);{std::unique_lock lock(gate);if(phase!=Phase::Bound||effect->failed||effect->checking)invalid(lock,"Environment effect retirement was poisoned");auto at=effects.begin();while(at!=effects.end()&&at->get()!=effect.get())++at;if(at==effects.end())std::terminate();effect->open=false;retired.splice(retired.end(),effects,at);}retired.clear();effect.reset();};
            if constexpr(std::is_void_v<std::invoke_result_t<F,Effect&>>){fn(*effect);finish();}
            else{decltype(auto) result=fn(*effect);finish();return result;}
        }catch(...){std::lock_guard lock(gate);if(effect)effect->failed=true;if(scope)scope->failed=true;if(phase!=Phase::Closed)phase=Phase::Poisoned;throw;}
    }
    void use(Effect& e,s::IoHandlerEnvironment& value,bool write){std::unique_lock lock(gate);claim(lock,e.scope,e.claims,e.count,value,write,&e);}
    Row row(Effect& e,s::IoHandlerEnvironment& value,bool write=false,bool half=false){std::unique_lock lock(gate);claim(lock,e.scope,e.claims,e.count,value,write,&e);auto* found=at(&value);if(!found||!(half?partial(*found):full(*found)))invalid<s::UnsupportedInput>(lock,"Environment row lost its exact fields");return *found;}
    Row query_row(Effect& e,std::uintptr_t cap){std::unique_lock lock(gate);Row* found=nullptr;for(auto& r:rows)if(r.address&&r.cap==cap){found=&r;break;}if(!found)invalid<s::UnsupportedInput>(lock,"Expired environment query generation");claim(lock,e.scope,e.claims,e.count,*found->address,false,&e);if(!full(*found))invalid<s::UnsupportedInput>(lock,"Queried environment source changed");return *found;}
    s::IoHandlerEnvironment* source_by_cap(JobState* scope,std::uintptr_t cap){
        std::unique_lock lock(gate);if(phase!=Phase::Bound||binding)reject(lock,"Environment source is unavailable");if(scope)scope_metadata(lock,*scope);else if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())reject(lock,"Raw factory requires original actor");if(actor_busy(::GetCurrentThreadId(),::GetCurrentFiber()))invalid(lock,"Environment factory actor cannot reenter");
        Row* found=nullptr;for(auto& r:rows)if(r.address&&r.cap==cap){found=&r;break;}if(!found)reject<s::UnsupportedInput>(lock,"Expired environment query generation");std::array<Claim,algorithm_claims> claims{};std::size_t count=0;claim(lock,scope,claims,count,*found->address,false);if(!full(*found))invalid<s::UnsupportedInput>(lock,"Factory source fields changed");return found->address;
    }
    Row inspect_source(JobState* scope,s::IoHandlerEnvironment& value){std::unique_lock lock(gate);if(phase!=Phase::Bound||binding)reject(lock,"Environment source is unavailable");if(scope)scope_metadata(lock,*scope);else if(thread!=::GetCurrentThreadId()||fiber!=::GetCurrentFiber())reject(lock,"Environment source requires original actor");if(actor_busy(::GetCurrentThreadId(),::GetCurrentFiber()))invalid(lock,"Environment constructor actor cannot reenter");std::array<Claim,algorithm_claims> claims{};std::size_t count=0;claim(lock,scope,claims,count,value,false);auto* r=at(&value);if(!r)reject<s::UnsupportedInput>(lock,"Constructor input address is not registered");if(!full(*r))invalid<s::UnsupportedInput>(lock,"Constructor source fields changed");return *r;}
    bool matches(const Row& expected)noexcept{auto* r=at(expected.address);return r&&r->cap==expected.cap&&r->state==expected.state&&r->origin==expected.origin&&r->job_epoch==expected.job_epoch&&r->family==expected.family&&r->flags==expected.flags&&r->pin==expected.pin&&r->release_consumed==expected.release_consumed&&full(*r);}
    void make(Effect& e,s::IoHandlerEnvironment& out,std::uintptr_t state,Origin origin,const Row* from=nullptr,bool partial_output=false,Transform transform=Transform::Copy){
        use(e,out,true);const auto cap=next_capability();auto family=classifier==ClassifierProfile::Host?Family::Host:(from?from->family:Family::Initial);auto flags=from?from->flags:0U;
        if(family!=Family::Host){if(transform==Transform::Prepare)flags&=~1U;else if(transform==Transform::Factory)family=Family::Tracked;else if(transform==Transform::Response20)flags|=1U;else if(transform==Transform::Response88)flags&=~2U;}
        std::list<Row> prepared;bool need=false;
        {std::unique_lock lock(gate);if(from&&!matches(*from))invalid(lock,"Environment source changed before allocation");need=std::none_of(rows.begin(),rows.end(),[](const Row& r){return !r.address;});}
        if(need){prepared.emplace_back();admit(e);}
        bool origin_live=true;
        if(origin.control){auto& memory=ports->view.memory;origin_live=memory.owns_control(origin.control);if(origin_live){origin_live=origin.control->strong_08.load()!=0;if(origin_live){const auto& owner=static_cast<s::SessionControl*>(origin.control)->object;origin_live=owner.identity()==origin.owner&&owner.weak_self.control_08==origin.control&&owner.weak_self.object_00==origin.owner;}}}
        // Allocation/new-handler/source callbacks may invalidate Root's owner;
        // prove actual Parent ownership before dereferencing its control again.
        if(!origin_live)throw std::logic_error("Environment allocation lost live session origin");
        {std::unique_lock lock(gate);if(phase!=Phase::Bound||e.failed||(from&&!matches(*from)))invalid(lock,"Environment row allocation lost its source");if(at(&out))invalid<s::UnsupportedInput>(lock,"Environment destination acquired another row");
            if(partial_output){if(out.operations_10!=&table||!from||out.schedule_20!=schedule(*from))invalid<s::UnsupportedInput>(lock,"Environment copy prefix changed");}
            else if(out.payload_00[0]||(out.operations_10&&out.operations_10!=&empty()))invalid<s::UnsupportedInput>(lock,"Environment factory output is not fresh");
            if(e.scope&&origin!=e.scope->origin)invalid(lock,"Environment factory lost scope origin");Row* target=nullptr;
            // If allocation was needed, commit that exact node even if a sibling
            // retired another row meanwhile. No unused-node deleter may run in
            // the native copy's published-payload/live-executor tail window.
            if(!prepared.empty()){target=&prepared.front();rows.splice(rows.end(),prepared);}else{for(auto& r:rows)if(!r.address){target=&r;break;}if(!target)invalid(lock,"Environment prepared row missing");}
            if(family==Family::Tracked)ports->view.scheduler.work_count_30.fetch_add(1,std::memory_order_seq_cst);*target={&out,cap,state,origin,e.scope?e.scope->epoch:0,family,flags,family==Family::Tracked?Pin::Owned:Pin::None,false};++refs;
            if(partial_output){++copied;out.payload_00={token,cap};out.state_18=state;}else{++created;out={{token,cap},&table,state,schedule(*target),execution(*target)};}}
    }
    void retire(Row& r,s::IoHandlerEnvironment& value)noexcept{r={};--refs;++destroyed;value={};value.operations_10=&empty();value.executor_28=&empty_executor();}
    void destroy(Effect& e,s::IoHandlerEnvironment& value){
        use(e,value,true);
        std::list<s::IoHandlerEnvironment*> retired_empty;
        {std::unique_lock lock(gate);if(value.operations_10==&empty()){if(at(&value))invalid(lock,"Empty environment still owns a row");if(auto* owner=empty_owner(&value)){if(owner!=e.scope)reject<s::UnsupportedInput>(lock,"Sentinel result belongs to another scope");auto at=owner->empty_results.begin();while(at!=owner->empty_results.end()&&*at!=&value)++at;retired_empty.splice(retired_empty.end(),owner->empty_results,at);}value={};value.operations_10=&empty();value.executor_28=&empty_executor();return;}}
        auto expected=row(e,value,true);
        {std::unique_lock lock(gate);auto* r=at(&value);if(!r||!matches(expected))invalid(lock,"Environment destroy source changed");if(r->pin==Pin::None){retire(*r,value);return;}r->pin=Pin::ReleaseEntered;e.release_entered=true;e.release=*r;}
        const std::function<void()> proof=[owned=&e]{auto& c=owned->control;c.admit(*owned);std::unique_lock lock(c.gate);if(!owned->release_entered||!c.matches(owned->release))c.invalid(lock,"Environment release lost its exact prefix");};
        try{auto& p=*ports;if(e.scope)p.environment_work_stop->finish_job(p.view.scheduler,*e.scope->permit,proof,e.consumed);else p.environment_work_stop->finish_original(p.view.scheduler,proof,e.consumed);
            std::unique_lock lock(gate);if(!e.consumed||phase!=Phase::Bound||!matches(e.release))invalid(lock,"Environment work did not finish exact consumption");auto* r=at(&value);retire(*r,value);
        }catch(...){std::lock_guard lock(gate);if(auto* r=at(&value);r&&r->cap==e.release.cap)r->release_consumed=e.consumed;if(phase!=Phase::Closed)phase=Phase::Poisoned;throw;}
    }
    struct MoveSlot final:s::IoEnvironmentMoveSlot {Control& c;Effect& e;Row expected;bool empty_source=false;MoveSlot(Control& c,Effect& e,Row r,bool zero):c(c),e(e),expected(r),empty_source(zero){}void move_slot_10(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from)override{
        std::unique_lock lock(c.gate);if(c.phase!=Phase::Bound||e.failed)c.invalid(lock,"Environment move slot admission changed");if(empty_source){out.payload_00={};out.state_18=0;from.payload_00={};from.state_18=0;return;}
        auto* r=c.at(&from);if(!r||r->cap!=expected.cap||r->origin!=expected.origin||!c.partial(*r)||out.operations_10!=&c.table||out.schedule_20!=c.schedule(*r))c.invalid<s::UnsupportedInput>(lock,"Environment move native prefix changed");out.payload_00=from.payload_00;out.state_18=from.state_18;r->address=&out;++c.moved;from.payload_00={};from.state_18=0;
    }};
    s::IoHandlerEnvironment& move(Effect& e,s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from,s::IoHandlerSentinels selected){
        if(!same(selected,sentinels()))throw s::UnsupportedInput("Environment move sentinels changed");use(e,from,true);use(e,out,true);Row expected;bool zero=false;JobState* empty_source_owner=nullptr;std::list<s::IoHandlerEnvironment*> prepared_empty;
        {std::unique_lock lock(gate);if(at(&out)||empty_owner(&out))reject<s::UnsupportedInput>(lock,"Move destination already owns a projection");auto* r=at(&from);zero=!r&&from.operations_10==&empty();if(r){if(!full(*r))invalid<s::UnsupportedInput>(lock,"Move source fields changed");expected=*r;}else if(!zero)invalid<s::UnsupportedInput>(lock,"Move source is not a live environment");empty_source_owner=empty_owner(&from);}
        if(zero&&e.scope&&!empty_source_owner){prepared_empty.push_back(&out);admit(e);}
        MoveSlot slot(*this,e,expected,zero);s::move_io_environment_023cb0(out,from,selected,&slot);
        {std::unique_lock lock(gate);if(phase!=Phase::Bound||e.failed)invalid(lock,"Environment move completion was poisoned");if(!zero){auto* r=at(&out);if(!r||r->cap!=expected.cap||!full(*r))invalid(lock,"Moved environment lost its exact destination");}else if(e.scope){if(empty_source_owner){for(auto*& address:empty_source_owner->empty_results)if(address==&from){address=&out;break;}}else e.scope->empty_results.splice(e.scope->empty_results.end(),prepared_empty);}}
        return out;
    }
    std::int32_t compare(std::uintptr_t first,std::uintptr_t second){if(second!=0x1800a25c8ULL)throw s::UnsupportedInput("Unknown environment type comparison");if(first==empty_type()+8)return 0;if(first==active_type()+8||(classifier==ClassifierProfile::OriginalDefault&&first==tracked_type()+8))return -1;throw s::UnsupportedInput("Unknown environment type token");}
    std::uintptr_t query(Effect& e,s::IoHandlerEnvironment& value){return row(e,value).cap;}
    void construct(Effect& e,s::IoHandlerEnvironment& out,std::uintptr_t cap,Transform transform){const auto from=query_row(e,cap);make(e,out,from.state,from.origin,&from,false,transform);}
    void copy(Effect& e,s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from,bool prefix_ready){const auto expected=row(e,from);use(e,out,true);if(prefix_ready){std::lock_guard lock(gate);e.copy_output=&out;}else{out.operations_10=from.operations_10;out.schedule_20=from.schedule_20;}make(e,out,expected.state,expected.origin,&expected,true);if(!prefix_ready)out.executor_28=from.executor_28;}
    struct EffectOperations final:s::IoEnvironmentCallOperations {
        Control& c;Effect& e;EffectOperations(Control& c,Effect& e):c(c),e(e){}
        std::int32_t compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b)override{c.admit(e);const auto value=c.compare(a,b);c.admit(e);return value;}
        std::uintptr_t query_slot_18(s::IoHandlerEnvironment& value)override{c.admit(e);const auto cap=c.query(e,value);c.admit(e);return cap;}
        void destroy_slot_00(s::IoHandlerEnvironment& value)override{c.admit(e);c.destroy(e,value);c.admit(e);}
        void construct_slot_58(s::IoEnvironmentFactory cached,s::IoHandlerEnvironment& out,std::uintptr_t cap,s::IoEnvironmentConstructorScratch&)override{c.admit(e);if(cached!=&Control::factory)throw s::UnsupportedInput("Scoped factory function is foreign");c.construct(e,out,cap,Transform::Factory);c.admit(e);}
        s::IoHandlerEnvironment& move_023cb0(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from,s::IoHandlerSentinels sentinels)override{c.admit(e);auto& result=c.move(e,out,from,sentinels);c.admit(e);return result;}
    };
    struct Table final:s::IoHandlerVtable {
        Control& c;explicit Table(Control& value):IoHandlerVtable(1),c(value){}
        void destroy_slot_00(s::IoHandlerEnvironment& value)override{c.run(nullptr,{{&value,true,Shape::Full}},true,[&](Effect& e){c.destroy(e,value);});}
        void move_slot_10(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from)override{c.run(nullptr,{{&from,true,Shape::Partial},{&out,true,Shape::MoveDestination}},false,[&](Effect& e){auto r=c.row(e,from,true,true);MoveSlot slot(c,e,r,false);slot.move_slot_10(out,from);});}
        std::uintptr_t query_slot_18(s::IoHandlerEnvironment& value)override{return c.run(nullptr,{{&value,false,Shape::Full}},false,[&](Effect& e){return c.query(e,value);});}
        bool try_move_environment_023cb0(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from,const s::IoHandlerSentinels& selected)override{if(!same(selected,sentinels()))throw s::UnsupportedInput("Owned move requires exact sentinels");c.run(nullptr,{{&from,true,Shape::Full},{&out,true,Shape::MoveDestination}},false,[&](Effect& e){c.move(e,out,from,selected);});return true;}
    } table{*this};
    struct Types final:s::IoEnvironmentTypeApi {std::weak_ptr<Control> control;explicit Types(std::weak_ptr<Control> c):control(std::move(c)){}std::int32_t compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b)override{auto c=control.lock();if(!c)throw s::UnsupportedInput("Retired environment type provider");return c->run(nullptr,{},false,[&](Effect&){return c->compare(a,b);});}};
    static void prepare(s::IoHandlerEnvironment& out,std::uintptr_t cap,s::IoPrepareScratch&){construct_root(out,cap,Transform::Prepare);}
    static void factory(s::IoHandlerEnvironment& out,std::uintptr_t cap,s::IoEnvironmentConstructorScratch&){construct_root(out,cap,Transform::Factory);}
    static void response20(s::IoHandlerEnvironment& out,std::uintptr_t cap,std::uintptr_t literal){if(literal!=0x180084425ULL)throw s::UnsupportedInput("Response factory literal mismatch");construct_root(out,cap,Transform::Response20);}
    static void response88(s::IoHandlerEnvironment& out,std::uintptr_t cap,s::ResponseExecutorScratch&){construct_root(out,cap,Transform::Response88);}
    static void construct_root(s::IoHandlerEnvironment& out,std::uintptr_t cap,Transform transform){auto c=query_owner(cap);auto* from=c->source_by_cap(nullptr,cap);c->run(nullptr,{{from,false,Shape::Full},{&out,true,Shape::Fresh}},false,[&](Effect& e){c->construct(e,out,cap,transform);});}
    static void copy_root(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from){auto c=owner(from.payload_00[0]);c->run(nullptr,{{&from,false,Shape::Full},{&out,true,Shape::MoveDestination}},false,[&](Effect& e){c->copy(e,out,from,true);});}
    void handoff(Effect& e){std::unique_lock lock(gate);if(phase!=Phase::Bound||e.failed)invalid(lock,"Environment scheduling handoff was poisoned");e.count=0;e.allow_nested=true;}
    static void handler(s::IoHandlerEnvironment& env,s::OwnedHandlerTask*& slot){auto c=owner(env.payload_00[0]);c->run(nullptr,{{&env,false,Shape::Full}},false,[&](Effect& e){const auto r=c->row(e,env);{std::lock_guard lock(c->gate);++c->handlers;}if(!slot)throw s::UnsupportedInput("Owned handler slot is empty");c->handoff(e);
        if(r.family!=Family::Host){auto& p=*c->ports;s::PostedExecutor selected{p.view.identity.value,&p.view.scheduler,r.flags};s::dispatch_posted_022e40(selected,slot,p.environment.storage,p.worker,p.completion,p.memory.posted());return;}auto* const task=slot;slot=nullptr;task->call_slot_00(1); // no task/env read after ownership handoff
    });}
    static void response(s::IoHandlerEnvironment& env,s::PostedFunctor*& slot){auto c=owner(env.payload_00[0]);c->run(nullptr,{{&env,false,Shape::Full}},false,[&](Effect& e){const auto r=c->row(e,env);{std::lock_guard lock(c->gate);++c->responses;}if(!slot)throw s::UnsupportedInput("Owned response slot is empty");c->handoff(e);auto& p=*c->ports;s::PostedExecutor selected{p.view.identity.value,&p.view.scheduler,r.family==Family::Host?1U:r.flags};s::dispatch_posted_022e40(selected,slot,p.environment.storage,p.worker,p.completion,p.memory.posted());});}
    s::IoHandlerExecutor executor{prepare},tracked_executor{prepare};s::HandlerSchedule handlers_table{handler,nullptr};s::ResponseSchedule responses_table{response,nullptr};s::AcceptSchedule accepts_table{handler,nullptr};
    Snapshot snapshot()const{PreserveError error;std::lock_guard lock(gate);std::size_t active=effects.size();for(auto* a=admissions;a;a=a->next)++active;for(auto* a=scope_admissions;a;a=a->next)++active;for(const auto& s:scopes)active+=s->preparing||s->closing||s->checking;return {phase,refs,active,created,moved,copied,destroyed,handlers,responses,token};}
};

Environment::Environment(std::size_t capacity,ClassifierProfile profile){if(!capacity||capacity>1048576||(profile!=ClassifierProfile::Host&&profile!=ClassifierProfile::OriginalDefault))throw std::invalid_argument("Bounded environment capacity and known classifier profile required");control_=std::make_shared<Control>(capacity,profile);Control::register_owner(control_);}
Environment::~Environment()noexcept{auto c=control_;{PreserveError error;std::lock_guard lock(c->gate);if(c->refs||c->binding||c->admissions||c->scope_admissions||!c->effects.empty()||!c->scopes.empty()||(c->phase!=Phase::Closed&&c->phase!=Phase::Fresh))std::terminate();c->phase=Phase::Closed;}Control::unregister_owner(c->token);}
sp::Environment Environment::configuration()const{
    auto c=control_;{PreserveError error;std::unique_lock lock(c->gate);if(c->phase!=Phase::Fresh||c->binding)Control::reject(lock,"Environment configuration exports before binding only");}
    sp::Environment out;out.sentinels=sentinels();out.types=std::make_shared<Control::Types>(c);
    out.queries={{c->token,c->classifier==ClassifierProfile::OriginalDefault?empty_type:active_type},{0,empty_type}};
    out.factories={{&c->executor,Control::factory}};out.handlers={{c->token,&c->handlers_table}};out.response_executors={{&c->executor,Control::response20,Control::response88}};
    out.responses={{c->token,&c->responses_table}};out.copies={{&c->table,Control::copy_root}};out.accepts={{c->token,&c->accepts_table}};
    if(c->classifier==ClassifierProfile::OriginalDefault){out.queries.push_back({c->tracked_token,tracked_type});out.factories.push_back({&c->tracked_executor,Control::factory});out.handlers.push_back({c->tracked_token,&c->handlers_table});out.response_executors.push_back({&c->tracked_executor,Control::response20,Control::response88});out.responses.push_back({c->tracked_token,&c->responses_table});out.accepts.push_back({c->tracked_token,&c->accepts_table});}return out;
}
void Environment::bind(rt::Ports ports,std::function<void()> checkpoint){
    auto c=control_;{PreserveError error;std::unique_lock lock(c->gate);if(c->phase!=Phase::Fresh||c->binding)c->invalid(lock,"Environment binds once");c->binding=true;c->thread=::GetCurrentThreadId();c->fiber=::GetCurrentFiber();}
    try{if(!checkpoint||!same(ports.environment.sentinels,sentinels())||!ports.server_identity||!ports.view.identity.value)throw std::invalid_argument("Exact owned sentinels, stable Ports and admission required");if(c->classifier==ClassifierProfile::OriginalDefault&&(!ports.environment_work_stop||!ports.environment_work_stop->matches(ports.view.memory,ports.view.scheduler)))throw std::invalid_argument("Native environment requires exact work-stop capability");if(!ports.accept.matches_storage(ports.environment.storage))throw std::invalid_argument("Environment storage belongs to another graph");
        (void)ports.view.memory.buffer_context();const auto phase=ports.view.backend.snapshot().phase;if((phase!=sp::o::Phase::Ready&&phase!=sp::o::Phase::Running)||ports.view.io.snapshot().phase!=s::IoMemoryPhase::Open)throw std::logic_error("Environment requires an open source graph");
        c->ports.emplace(ports);c->concurrent=ports.view.memory.operation_policy()==s::OperationPolicy::OriginalConcurrentActors;c->checkpoint.swap(checkpoint);checkpoint={};
        {std::unique_lock lock(c->gate);if(c->phase!=Phase::Fresh||!c->binding)c->invalid(lock,"Environment bind target retirement was poisoned");}
        c->lease=ports.view.memory.acquire_provider_lease_from_operation();
        {std::unique_lock lock(c->gate);if(c->phase!=Phase::Fresh||!c->binding)c->invalid(lock,"Environment bind was poisoned");c->thread=::GetCurrentThreadId();c->fiber=::GetCurrentFiber();c->binding=false;c->phase=Phase::Bound;}
    }catch(...){c->poison();{std::lock_guard lock(c->gate);c->binding=false;}throw;}
}
void Environment::session(s::IoHandlerEnvironment& out,sp::SessionView view){auto c=control_;c->run(nullptr,{{&out,true,Control::Shape::Fresh}},false,[&](Control::Effect& e){auto& p=*c->ports;auto* control=view.owner.weak_self.control_08;if(!p.view.memory.owns_control(control)||!control->strong_08.load()||&static_cast<s::SessionControl*>(control)->object!=&view.owner||view.owner.weak_self.object_00!=view.owner.identity())throw std::invalid_argument("Environment session must be exact live Parent owner");c->make(e,out,1,{view.token,view.owner.identity(),control});});}
void Environment::accept(s::IoHandlerEnvironment& out,std::uintptr_t allocator){auto c=control_;c->run(nullptr,{{&out,true,Control::Shape::Fresh}},false,[&](Control::Effect& e){if(allocator!=c->ports->server_identity+0x68)throw std::invalid_argument("Foreign accept allocator token");c->make(e,out,0,{});});}
void Environment::release(s::IoHandlerEnvironment& value){
    auto c=control_;s::IoHandlerVtable* operations=nullptr;
    {std::unique_lock lock(c->gate);
        // Address-only ownership/conflict rejection precedes native field reads.
        // A bad Root caller cannot race a scoped move's unlocked prefix writes.
        if(auto* r=c->at(&value);r&&r->job_epoch)Control::reject<s::UnsupportedInput>(lock,"Root cannot release a scoped temporary");
        if(c->empty_owner(&value))Control::reject<s::UnsupportedInput>(lock,"Root cannot release a scoped sentinel result");
        if(c->protected_source(&value))Control::reject<s::UnsupportedInput>(lock,"Scope protects this environment source");
        if(c->at(&value)&&c->thread==::GetCurrentThreadId()&&c->fiber==::GetCurrentFiber()&&c->actor_busy(c->thread,c->fiber))c->invalid(lock,"Environment release cannot reenter admission");
        std::array<Control::Claim,Control::algorithm_claims> claims{};std::size_t count=0;c->claim(lock,nullptr,claims,count,value,true);operations=value.operations_10;
    }
    if(operations==&empty()){empty().destroy_slot_00(value);return;}if(operations!=&c->table)throw s::UnsupportedInput("Foreign environment table");c->table.destroy_slot_00(value);
}
Environment::JobScope::JobScope(std::shared_ptr<Control> control,std::shared_ptr<JobState> state)noexcept:control_(std::move(control)),state_(std::move(state)){}
Environment::JobScope::~JobScope()noexcept{PreserveError error;if(!state_||state_->closed)return;try{close();}catch(...){std::lock_guard lock(control_->gate);state_->failed=true;state_->open=false;if(control_->phase!=Phase::Closed)control_->phase=Phase::Poisoned;}}
Environment::JobScope Environment::enter_job(const s::IoHandlerEnvironment& source,sp::SessionView view,const s::SessionMemoryDomain::JobPermit& permit,std::function<void()> checkpoint){
    PreserveError error;auto c=control_;Origin origin;std::uintptr_t cap=0;s::SessionMemoryDomain* parent=nullptr;std::shared_ptr<void> lease;
    if(!checkpoint)throw std::invalid_argument("Explicit scope checkpoint required");
    Control::ScopeAdmission pending;pending.thread=::GetCurrentThreadId();pending.fiber=::GetCurrentFiber();pending.source=&source;
    const auto capture=[&](std::unique_lock<std::mutex>& lock,bool repeat){
        if(c->phase!=Phase::Bound||c->binding||!c->ports)Control::reject(lock,"Environment scope requires bound provider");if(c->thread==pending.thread)Control::reject(lock,"Environment scope requires distinct child worker");if(c->actor_busy(pending.thread,pending.fiber))c->invalid(lock,"Environment scope preparation cannot reenter");if(!c->concurrent&&(c->admissions||!c->effects.empty()||c->scope_admissions||!c->scopes.empty()))Control::reject(lock,"Serialized environment scope requires idle provider");
        std::array<Control::Claim,Control::algorithm_claims> claims{};std::size_t count=0;c->claim(lock,nullptr,claims,count,const_cast<s::IoHandlerEnvironment&>(source),false);auto* row=c->at(&source);
        if(repeat&&(!row||row->cap!=cap||row->origin!=origin))Control::reject(lock,"Scope source changed before admission");
        if(!row)Control::reject<s::UnsupportedInput>(lock,"Scope input address is not registered");
        if(!c->full(*row))c->invalid<s::UnsupportedInput>(lock,"Scope protected source fields changed");
        if(row->state!=1||!row->origin.control||row->origin.session!=view.token||row->origin.owner!=view.owner.identity())Control::reject<std::invalid_argument>(lock,"Exact session environment origin required");
        if(!repeat){origin=row->origin;cap=row->cap;parent=&c->ports->view.memory;lease=c->lease;}
        else if(parent!=&c->ports->view.memory)Control::reject(lock,"Scope Parent binding changed");
    };
    {std::unique_lock lock(c->gate);capture(lock,false);}
    // Bad/expired/foreign caller proof is a known preflight rejection. No
    // admission, callback, native prefix or sticky failure has been published.
    permit.check(*parent);if(!parent->owns_control(origin.control)||!origin.control->strong_08.load()||&static_cast<s::SessionControl*>(origin.control)->object!=&view.owner||view.owner.weak_self.control_08!=origin.control||view.owner.weak_self.object_00!=origin.owner)throw std::invalid_argument("Scope requires exact live Parent owner/control");
    {std::unique_lock lock(c->gate);capture(lock,true);pending.cap=cap;pending.next=c->scope_admissions;c->scope_admissions=&pending;}
    struct Pending {Control& c;Control::ScopeAdmission& a;bool linked=true;void remove()noexcept{auto** at=&c.scope_admissions;while(*at&&*at!=&a)at=&(*at)->next;if(!*at)std::terminate();*at=a.next;linked=false;}~Pending(){if(linked){std::lock_guard lock(c.gate);remove();}}} admission{*c,pending};
    std::list<std::shared_ptr<JobState>> prepared;std::shared_ptr<JobState> state;
    try{const auto epoch=next_capability();prepared.emplace_back();state=std::make_shared<JobState>(*c,std::move(lease),*parent,permit,origin,source,cap,epoch);prepared.front()=state;
        {std::unique_lock lock(c->gate);c->scopes.splice(c->scopes.end(),prepared);admission.remove();if(c->phase!=Phase::Bound)c->invalid(lock,"Scope allocation was poisoned");}
        state->checkpoint.swap(checkpoint);checkpoint={};state->context.emplace(*parent,*state->permit,content_handlers_recovered::CallCheckpoint{state.get(),&JobState::invoke_checkpoint});
        c->scope_validate(*state,true);{std::unique_lock lock(c->gate);state->checking=true;}
        state->context->check();c->source(false);c->scope_validate(*state,true);
        {std::unique_lock lock(c->gate);state->checking=false;if(c->phase!=Phase::Bound||state->failed)c->invalid(lock,"Scope preparation callback was poisoned");state->preparing=false;state->open=true;}
        return JobScope(std::move(c),std::move(state));
    }catch(...){std::lock_guard lock(c->gate);if(state){state->checking=false;state->failed=true;state->open=false;}if(c->phase!=Phase::Closed)c->phase=Phase::Poisoned;throw;}
}
s::IoEnvironmentCallOperations& Environment::JobScope::operations(){if(!state_)throw std::logic_error("Environment scope is closed");{std::unique_lock lock(control_->gate);control_->scope_metadata(lock,*state_);}return state_->operations;}
s::IoHandlerEnvironment& Environment::JobScope::construct_023dd0(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& source,s::IoHandlerSentinels selected,s::IoEnvironmentConstructorContext& context){
    if(!state_)throw std::logic_error("Environment scope is closed");auto c=control_;auto state=state_;
    // Known foreign contexts/callbacks are rejected before allocating an effect,
    // checkpoint invocation, native raw type query or destination prefix writes.
    const auto from=c->inspect_source(state.get(),source);
    if(!same(selected,sentinels())||context.calls)throw s::UnsupportedInput("Scoped constructor requires canonical unbound context");
    auto* types=dynamic_cast<Control::Types*>(&context.types);auto typed_owner=types?types->control.lock():std::shared_ptr<Control>{};if(typed_owner.get()!=c.get())throw s::UnsupportedInput("Scoped constructor has foreign type provider");
    const auto expected_type=from.family==Control::Family::Tracked?tracked_type:(c->classifier==ClassifierProfile::OriginalDefault?empty_type:active_type);
    unsigned queries=0,factories=0;for(const auto& q:context.type_queries)if(q.schedule_identity==c->schedule(from)){++queries;if(q.query_slot_00!=expected_type)throw s::UnsupportedInput("Scoped constructor type query is foreign");}
    if(expected_type!=empty_type)for(const auto& f:context.factories)if(f.executor_identity==c->execution(from)){++factories;if(f.construct_slot_58!=&Control::factory)throw s::UnsupportedInput("Scoped constructor factory binding is foreign");}
    if(queries!=1||(expected_type!=empty_type&&factories!=1))throw s::UnsupportedInput("Scoped constructor requires unique selected canonical bindings");
    return c->run(state.get(),{{&source,false,Control::Shape::Full},{&out,true,Control::Shape::MoveDestination}},false,[&](Control::Effect& e)->s::IoHandlerEnvironment&{
        const auto current=c->row(e,source);if(current.cap!=from.cap)throw s::UnsupportedInput("Scoped constructor source generation changed");
        Control::EffectOperations calls(*c,e);auto private_context=context;private_context.calls=&calls;
        s::construct_io_environment_023dd0(out,source,selected,private_context);
        // Interior native temporaries have ended their C++ lifetimes now. Only
        // inspect their retained address metadata, never expired stack payloads.
        {std::unique_lock lock(c->gate);for(std::size_t i=2;i<e.count;++i)if(c->at(e.claims[i].address)||c->empty_owner(e.claims[i].address))c->invalid(lock,"Constructor returned with an owned internal temporary");}
        return out;
    });
}
void Environment::JobScope::copy(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& source){if(!state_)throw std::logic_error("Environment scope is closed");auto c=control_;c->run(state_.get(),{{&source,false,Control::Shape::Full},{&out,true,Control::Shape::MoveDestination}},false,[&](Control::Effect& e){c->copy(e,out,source,false);});}
std::int32_t Environment::JobState::Operations::compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b){auto& c=state.control;return c.run(&state,{},false,[&](Control::Effect&){return c.compare(a,b);});}
std::uintptr_t Environment::JobState::Operations::query_slot_18(s::IoHandlerEnvironment& value){auto& c=state.control;return c.run(&state,{{&value,false,Control::Shape::Full}},false,[&](Control::Effect& e){return c.query(e,value);});}
void Environment::JobState::Operations::destroy_slot_00(s::IoHandlerEnvironment& value){auto& c=state.control;c.run(&state,{{&value,true,Control::Shape::FullOrEmpty}},true,[&](Control::Effect& e){c.destroy(e,value);});}
void Environment::JobState::Operations::construct_slot_58(s::IoEnvironmentFactory cached,s::IoHandlerEnvironment& out,std::uintptr_t cap,s::IoEnvironmentConstructorScratch&){auto& c=state.control;if(cached!=&Control::factory)throw s::UnsupportedInput("Scoped cached factory is foreign");auto* from=c.source_by_cap(&state,cap);c.run(&state,{{from,false,Control::Shape::Full},{&out,true,Control::Shape::Fresh}},false,[&](Control::Effect& e){c.construct(e,out,cap,Control::Transform::Factory);});}
s::IoHandlerEnvironment& Environment::JobState::Operations::move_023cb0(s::IoHandlerEnvironment& out,s::IoHandlerEnvironment& from,s::IoHandlerSentinels selected){auto& c=state.control;if(!same(selected,sentinels()))throw s::UnsupportedInput("Scoped move requires exact sentinels");return c.run(&state,{{&from,true,Control::Shape::FullOrEmpty},{&out,true,Control::Shape::MoveDestination}},false,[&](Control::Effect& e)->s::IoHandlerEnvironment&{return c.move(e,out,from,selected);});}
void Environment::JobScope::close(){
    PreserveError error;if(!state_)return;auto state=state_;auto c=control_;std::list<std::shared_ptr<JobState>> retired;
    {std::unique_lock lock(c->gate);if(state->thread!=::GetCurrentThreadId()||state->fiber!=::GetCurrentFiber())Control::reject(lock,"Scope close belongs to another actor");if(state->closed)return;}
    try{{std::unique_lock lock(c->gate);c->scope_metadata(lock,*state);if(c->actor_busy(state->thread,state->fiber))c->invalid(lock,"Scope cannot retire active call/checkpoint");for(const auto& e:c->effects)if(e->scope==state.get())c->invalid(lock,"Scope still owns effect records");for(const auto& r:c->rows)if(r.address&&r.job_epoch==state->epoch)c->invalid(lock,"Scope still owns temporary projections");if(!state->empty_results.empty())c->invalid(lock,"Scope still owns sentinel results");state->closing=true;}
        c->scope_validate(*state,false,true);{std::function<void()> retiring;retiring.swap(state->checkpoint);} // target dies while stable scope membership remains published
        c->scope_validate(*state,false,true);
        {std::unique_lock lock(c->gate);c->scope_metadata(lock,*state,false,true);for(const auto& e:c->effects)if(e->scope==state.get())c->invalid(lock,"Scope retirement acquired a call");for(const auto& r:c->rows)if(r.address&&r.job_epoch==state->epoch)c->invalid(lock,"Scope retirement acquired a temporary");if(!state->empty_results.empty())c->invalid(lock,"Scope retirement acquired a sentinel result");auto at=c->scopes.begin();while(at!=c->scopes.end()&&at->get()!=state.get())++at;if(at==c->scopes.end())std::terminate();state->open=false;state->closing=false;state->closed=true;retired.splice(retired.end(),c->scopes,at);}
        state->context.reset();state->permit.reset();state->lease.reset();retired.clear(); // closed wrapper retains metadata only, no source/provider lease
    }catch(...){std::lock_guard lock(c->gate);state->failed=true;state->open=false;if(c->phase!=Phase::Closed)c->phase=Phase::Poisoned;throw;}
}
bool Environment::close(bool attested){
    PreserveError error;auto c=control_;
    {std::unique_lock lock(c->gate);if(c->phase==Phase::Closed)return true;if(c->thread!=::GetCurrentThreadId()||c->fiber!=::GetCurrentFiber())Control::reject(lock,"Environment close requires original actor");
        const bool pending=c->admissions||c->scope_admissions||!c->effects.empty()||!c->scopes.empty();if(c->phase==Phase::Poisoned&&pending)return false;bool own_effect=false;for(const auto& e:c->effects)own_effect|=e->thread==c->thread&&e->fiber==c->fiber;if(own_effect||c->actor_busy(c->thread,c->fiber)||c->binding||c->phase==Phase::Closing)c->invalid(lock,"Environment lifecycle reentry");if(c->phase!=Phase::Bound)Control::reject(lock,"Environment cannot close unknown/unbound state");if(pending||!attested||c->refs)return false;c->phase=Phase::Closing;c->close_checking=true;}
    try{c->checkpoint();{std::unique_lock lock(c->gate);c->close_checking=false;if(c->phase!=Phase::Closing)c->invalid(lock,"Environment close probe was poisoned");}
        auto& p=*c->ports;(void)p.view.memory.buffer_context();const auto backend=p.view.backend.snapshot().phase;const auto io=p.view.io.snapshot();const auto memory=p.view.memory.snapshot();const auto accept=p.accept.snapshot();
        const bool drained=(backend==sp::o::Phase::Returned||backend==sp::o::Phase::Quiescing)&&io.phase==s::IoMemoryPhase::Open&&!io.allocations&&!io.pending&&!io.active_calls&&memory.phase==s::SessionMemoryPhase::Open&&!memory.allocations&&!memory.controls&&!memory.proxies&&!memory.blocks&&!memory.maps&&!memory.buffers&&!memory.reserved&&!memory.pending_frees&&!memory.partial_constructions&&!memory.buffer_borrows&&!accept.poisoned&&!accept.projections&&!accept.pending&&!accept.active_calls&&!p.view.scheduler.work_count_30.load()&&!p.view.scheduler.completion.head&&!p.view.scheduler.completion.tail;
        {std::unique_lock lock(c->gate);if(c->phase!=Phase::Closing)c->invalid(lock,"Environment close was poisoned");if(!drained||c->admissions||c->scope_admissions||!c->effects.empty()||!c->scopes.empty()){c->phase=Phase::Bound;return false;}}
        Control::unregister_owner(c->token);{std::function<void()> retiring;retiring.swap(c->checkpoint);}
        {std::unique_lock lock(c->gate);if(c->phase!=Phase::Closing)c->invalid(lock,"Environment checkpoint retirement was poisoned");}
        c->lease.reset();c->ports.reset();{std::unique_lock lock(c->gate);if(c->phase!=Phase::Closing)c->invalid(lock,"Environment provider retirement was poisoned");c->phase=Phase::Closed;}return true;
    }catch(...){c->poison();throw;}
}
Snapshot Environment::snapshot()const{return control_->snapshot();}
} // namespace anadius::owned_environment
