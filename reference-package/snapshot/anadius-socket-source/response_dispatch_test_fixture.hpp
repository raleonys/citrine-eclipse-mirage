#pragma once
#include "response_dispatch.hpp"
#include "message_dispatch_test_fixture.hpp"

namespace response_test {
namespace s=anadius::socket_recovered;namespace b=buffer_test;namespace r=receive_test;
inline unsigned checks=0;inline void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Failure {int phase;};
template<class E,class F>void rejects(F f){bool caught=false;try{f();}catch(const E&){caught=true;}check(caught,"required response failure propagates");}
struct Fixture;inline Fixture* active=nullptr;
void factory20(s::IoHandlerEnvironment&,std::uintptr_t,std::uintptr_t);
void factory88(s::IoHandlerEnvironment&,std::uintptr_t,s::ResponseExecutorScratch&);
void publish(s::IoHandlerEnvironment&,s::PostedFunctor*&);
void borrowed(s::IoHandlerEnvironment&,s::BorrowedResponse&);
void next(std::uintptr_t,s::QueuedWriteContext&,s::QueuedWriteSend);
struct Table:s::IoHandlerVtable {
    Fixture& f;bool empty;Table(Fixture& f,bool empty):IoHandlerVtable(empty?0x180017a30ULL:0x777),f(f),empty(empty){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override;
    void move_slot_10(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&)override;
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override;
};
struct Functor final:s::PostedFunctor {
    Fixture& f;s::MessageWriteTask task;explicit Functor(Fixture& f):f(f){}
    void call_slot_00(std::uint8_t)override;
};
struct Api:s::ResponseDispatchApi {
    Fixture& f;explicit Api(Fixture& f):f(f){}
    void make_owned_response_020cb0(s::PostedFunctor*&,s::MessageWriteTask&)override;
    s::ReceiveBufferString& copy_response_009eb0(s::ReceiveBufferString&,s::ReceiveBufferString&)override;
};
struct QueueApi:s::ResponseQueueApi {
    Fixture& f;explicit QueueApi(Fixture& f):f(f){}
    void grow_deque_026670(s::WriteFinishOwner&,std::uint64_t)override;
    s::WriteFinishString* allocate_string_04b0bc(std::uint64_t)override;
};
struct Weak:s::QueuedWriteTerminal {[[noreturn]]void bad_weak_ptr_026860()override{throw Failure{26};}};
struct Fixture {
    message_test::Fixture m;Api api{*this};QueueApi queue_api{*this};Weak weak;Table live{*this,false},empty{*this,true};
    s::IoHandlerExecutor first_executor{},second_executor{},third_executor{};
    s::IoHandlerEnvironment source{{1,2},&live,9,0x456,&first_executor};
    std::array<s::ResponseExecutorBinding,3> executors{{{&first_executor,&factory20,nullptr},{&second_executor,nullptr,&factory88},{&third_executor,nullptr,&factory88}}};
    s::ResponseSchedule schedule{&publish,&borrowed};std::array<s::ResponseScheduleBinding,2> schedules{{{0x567,&schedule},{}}};
    s::IoHandlerSentinels sentinels{&empty,0x846e8,&third_executor,0x180017a30ULL};
    std::array<s::WriteFinishString*,8> map{};std::array<s::WriteFinishString,8> cells{};
    s::WriteFinishOwner queue{map,8,0,0};std::array<s::WriteFinishOwnerBinding,2> owners{{{0x12000,&queue},{}}};
    s::QueuedWriteContext queued{{},weak,nullptr};s::ResponseQueueContext queue_context{m.frame,queue_api,owners,&queued};
    s::ResponseDispatchContext context{m.frame,sentinels,executors,schedules,api,&queue_context};
    s::MessageWriteTask task;std::unique_ptr<Functor> functor;std::vector<std::unique_ptr<Functor>> retained;
    s::MessageWriteTask* local=nullptr;s::IoHandlerEnvironment *first=nullptr,*final=nullptr;
    std::vector<std::string> trace;std::function<void(std::string)> hook;
    std::function<void(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&,unsigned)> move_hook;
    unsigned moves=0,queries=0,makes=0,allocations=0,grows=0,nexts=0,calls=0;bool invoke_borrowed=false,take_owned=false,invoke_owned=false;
    std::function<void(s::WriteFinishOwner&)> grow_hook;std::function<void()> allocation_hook,next_hook;
    Fixture(){active=this;task={0x180028270ULL,m.response.value,0x12000,&m.base.owner};m.base.owner.strong_08=2;}
    void event(std::string text){trace.push_back(text);if(hook)hook(text);}
    void run(){active=this;s::dispatch_response_021420(source,task,context);}
    void enqueue(s::ReceiveBufferString& value){active=this;s::enqueue_response_028270(0x12000,value,queue_context,&next);}
};
inline void Table::destroy_slot_00(s::IoHandlerEnvironment&){f.event(empty?"destroy-empty":"destroy-live");}
inline void Table::move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from){
    check(from.operations_10==&f.empty&&from.schedule_20==0x846e8&&to.operations_10==this,"actual023cb0 detaches table/schedule before move callback");
    to.payload_00=from.payload_00;to.state_18=from.state_18;++f.moves;if(f.moves==1)f.first=&to;if(f.moves==3)f.final=&to;
    f.event("move"+std::to_string(f.moves));if(f.move_hook)f.move_hook(to,from,f.moves);
}
inline std::uintptr_t Table::query_slot_18(s::IoHandlerEnvironment&){++f.queries;f.event("query"+std::to_string(f.queries));return f.queries==1?0x123:0x234;}
inline void factory20(s::IoHandlerEnvironment& out,std::uintptr_t query,std::uintptr_t literal){auto& f=*active;check(query==0x123&&literal==0x180084425ULL,"factory20 exact query and property literal");out={{20,21},&f.live,1,0x567,&f.second_executor};f.event("factory20");}
inline void factory88(s::IoHandlerEnvironment& out,std::uintptr_t query,s::ResponseExecutorScratch&){auto& f=*active;check(query==0x234,"factory88 receives first moved environment query");out={{88,89},&f.live,2,0x567,&f.third_executor};f.event("factory88");}
inline void publish(s::IoHandlerEnvironment& env,s::PostedFunctor*& slot){auto& f=*active;check(&env==f.final,"owned schedule receives final third moved environment");f.event("publish");if(f.take_owned){check(slot==f.functor.get(),"owned mutable slot names actual functor");slot=nullptr;if(f.invoke_owned)f.functor->call_slot_00(1);else f.retained.push_back(std::move(f.functor));}}
inline void borrowed(s::IoHandlerEnvironment& env,s::BorrowedResponse& descriptor){auto& f=*active;check(&env==f.final&&descriptor.invoke_identity==0x180022240ULL,"borrowed descriptor receives actual callback token");f.local=descriptor.task;f.event("borrowed");if(f.invoke_borrowed)descriptor.invoke(f.context);}
inline void next(std::uintptr_t identity,s::QueuedWriteContext& context,s::QueuedWriteSend call){auto& f=*active;check(identity==0x12000&&&context==&f.queued&&call==&s::start_async_send_021fd0,"queue start literal owner and actual021fd0 default");++f.nexts;f.event("next");if(f.next_hook)f.next_hook();}
inline void Api::make_owned_response_020cb0(s::PostedFunctor*& slot,s::MessageWriteTask& source){
    ++f.makes;f.functor=std::make_unique<Functor>(f);f.functor->task=source;source.response_08.size_10=0;source.response_08.capacity_18=15;source.response_08.storage_00[0]=0;source.object_28=0;source.control_30=nullptr;slot=f.functor.get();f.event("make");
}
inline s::ReceiveBufferString& Api::copy_response_009eb0(s::ReceiveBufferString& out,s::ReceiveBufferString& in){
    f.m.seed(f.m.extra,f.m.text(in));out=f.m.extra.value;f.event("string-copy");return out;
}
inline void QueueApi::grow_deque_026670(s::WriteFinishOwner& q,std::uint64_t n){check(&q==&f.queue&&n==1,"native deque growth called with extra1");++f.grows;f.event("grow");if(f.grow_hook)f.grow_hook(q);else q.map_size_d8=8;}
inline s::WriteFinishString* QueueApi::allocate_string_04b0bc(std::uint64_t n){check(n==32,"queue allocates exactly32-byte string block");++f.allocations;f.event("allocate");if(f.allocation_hook)f.allocation_hook();if(f.allocations>f.cells.size())r::unexpected();return &f.cells[f.cells.size()-f.allocations];}
inline void Functor::call_slot_00(std::uint8_t flag){
    ++f.calls;f.event(flag?"functor-call":"functor-discard");
    if(flag)s::invoke_borrowed_response_022240(task,f.context);
    f.m.free_string(task.response_08);auto* c=task.control_30;
    if(c&&c->strong_08.fetch_sub(1)==1){c->dispose_slot_00();if(c->weak_0c.fetch_sub(1)==1)c->delete_slot_08();}
    task.control_30=nullptr;f.functor.reset(); // actual host lifetime end; no later this access
}
} // namespace response_test
