// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/transport_environment_constructor.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
namespace s=citrine_eclipse_mirage::socket;
namespace {
unsigned checks=0;
void check(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
struct Stop{};
struct Table:s::IoHandlerVtable{
 std::function<void(s::IoHandlerEnvironment&)> destroy;
 std::function<void(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&)> move;
 std::function<std::uintptr_t(s::IoHandlerEnvironment&)> query;
 explicit Table(std::uintptr_t id):IoHandlerVtable(id){}
 void destroy_slot_00(s::IoHandlerEnvironment& e)override{if(destroy)destroy(e);}
 void move_slot_10(s::IoHandlerEnvironment& a,s::IoHandlerEnvironment& b)override{if(move)move(a,b);}
 std::uintptr_t query_slot_18(s::IoHandlerEnvironment& e)override{if(query)return query(e);return 0xabc;}
};
struct Fixture;
Fixture* active=nullptr;
std::uintptr_t query_type();
void construct(s::IoHandlerEnvironment&,std::uintptr_t,s::IoEnvironmentConstructorScratch&);
struct Types:s::IoEnvironmentTypeApi{
 std::int32_t compare_type_names_04d2d0(std::uintptr_t,std::uintptr_t)override;
};
struct Fixture{
 Table empty{0x17a30},original{0x111},late{0x222},guard{0x17a30};
 s::IoHandlerExecutor executor{},replacement{},empty_executor{};
 std::unique_ptr<s::IoHandlerExecutor> heap_executor;
 s::IoHandlerSentinels sentinels{&empty,0x846e8,&empty_executor,0x17a30};
 s::IoHandlerEnvironment source{{11,12},&original,9,0x345,&executor},destination;
 Types types;
 std::array<s::IoEnvironmentTypeBinding,1> queries{{{0x345,&query_type}}};
 std::array<s::IoEnvironmentFactoryBinding,1> factories{{{&executor,&construct}}};
 s::IoEnvironmentConstructorContext context{types,queries,factories};
 std::vector<std::string> events;
 std::uintptr_t type=0x1000;
 std::int32_t result=1;
 std::string throw_at;
 unsigned moves=0;
 bool guard_first=false,guard_final=false,delete_executor=false;
 Fixture(){
  active=this;
  original.query=[this](s::IoHandlerEnvironment& e){event("query");check(&e==&source,"slot18 receives original source");if(delete_executor){heap_executor.reset();source.executor_28=&replacement;factories[0].construct_slot_58=nullptr;}return 0xabcdef;};
  original.move=[this](auto& to,auto& from){
   ++moves;event(moves==1?"move1":"move2");
   check(to.operations_10==&original&&to.schedule_20==0x456,"destination fields copied before move callback");
   check(from.operations_10==&empty&&from.schedule_20==0x846e8,"source ops and schedule reset before move callback");
   check(from.state_18==(moves==1?7U:19U),"source state remains live during move callback");
   to.payload_00=from.payload_00;to.state_18=moves==1?19:23;
   from.state_18=99;from.executor_28=&replacement;
   from.operations_10=(moves==1&&guard_first)||(moves==2&&guard_final)?&guard:&late;
  };
  late.destroy=[this](auto& value){event(moves==1?"destroy1":"destroy2");check(value.state_18==0&&value.executor_28==&empty_executor,"cleanup sees state and executor reset after move");};
  guard.destroy=[this](auto& value){event("guarddestroy");check(moves==2&&value.state_18==0,"guard final cleanup is unconditional");};
 }
 void event(const std::string& name){events.push_back(name);if(throw_at==name)throw Stop{};}
 void run(){check(&s::construct_io_environment_023dd0(destination,source,sentinels,context)==&destination,"constructor returns same destination");}
};
std::uintptr_t query_type(){active->event("type");return active->type;}
std::int32_t Types::compare_type_names_04d2d0(std::uintptr_t first,std::uintptr_t second){auto& f=*active;f.event("compare");check(first==f.type+std::uintptr_t{8}&&second==0x1800a25c8ULL,"type pointer plus8 and exact expected name identity");return f.result;}
void construct(s::IoHandlerEnvironment& value,std::uintptr_t query,s::IoEnvironmentConstructorScratch&){auto& f=*active;f.event("factory");check(query==0xabcdef,"cached factory gets live query result without executor this");value={{31,32},&f.original,7,0x456,&f.executor};}
void defaults(){for(bool change_guard:{false,true})for(std::uintptr_t type:{std::uintptr_t{0},std::uintptr_t{0x1000},std::numeric_limits<std::uintptr_t>::max()-3}){
 Fixture f;f.result=0;f.type=type;
 f.empty.move=[&](auto& to,auto& from){f.event("empty_move");check(to.operations_10==&f.empty&&to.schedule_20==f.sentinels.empty_schedule_846e8,"default initialized sentinel ops and schedule");check(from.operations_10==&f.empty&&from.schedule_20==f.sentinels.empty_schedule_846e8&&from.state_18==0&&from.executor_28==&f.empty_executor,"empty temporary setup and reset precede provider move");to.state_18=55;from.executor_28=&f.replacement;if(change_guard)from.operations_10=&f.late;};
 f.late.destroy=[&](auto& e){f.event("empty_destroy");check(e.state_18==0&&e.executor_28==&f.empty_executor,"live temporary cleanup after sentinel move");};
 f.run();check(f.destination.state_18==55&&f.destination.executor_28==&f.replacement,"default destination keeps callback state and reloads executor");
 check(f.events==(change_guard?std::vector<std::string>{"type","compare","empty_move","empty_destroy"}:std::vector<std::string>{"type","compare","empty_move"}),"default guard uses live operations identity");check(f.source.payload_00==std::array<std::uintptr_t,2>{11,12}&&f.source.state_18==9,"default branch leaves allocator source payload intact");
}}
void nonmatching(){for(auto result:{-2147483647,-1,1,2147483647})for(bool first_guard:{false,true})for(bool final_guard:{false,true})for(bool deletion:{false,true}){
 Fixture f;f.result=result;f.guard_first=first_guard;f.guard_final=final_guard;f.delete_executor=deletion;
 if(deletion){f.heap_executor=std::make_unique<s::IoHandlerExecutor>();f.source.executor_28=f.heap_executor.get();f.factories[0].executor_identity=f.heap_executor.get();}
 f.run();std::vector<std::string> expected{"type","compare","query","factory","move1"};if(!first_guard)expected.push_back("destroy1");expected.push_back("move2");expected.push_back(final_guard?"guarddestroy":"destroy2");
 check(f.events==expected,"nonzero compare performs cached factory, two ordered moves and exact guard cleanup");check(f.destination.payload_00==std::array<std::uintptr_t,2>{31,32}&&f.destination.state_18==23&&f.destination.executor_28==&f.replacement,"two move callbacks preserve defined payload and live executor reload");
}}
void exceptions(){const std::vector<std::string> sequence{"type","compare","query","factory","move1","destroy1","move2","destroy2"};for(std::size_t index=0;index<sequence.size();++index){Fixture f;f.throw_at=sequence[index];bool caught=false;try{f.run();}catch(const Stop&){caught=true;}check(caught,"required callback exception propagates");check(f.events==std::vector<std::string>(sequence.begin(),sequence.begin()+index+1),"exception stops normal sequence without invented cleanup");}}
template<class F>void unsupported(F action){bool caught=false;try{action();}catch(const s::UnsupportedInput&){caught=true;}check(caught,"unsupported projection fails boundedly");}
void registries(){
 {Fixture f;f.context.type_queries={};unsupported([&]{f.run();});check(f.events.empty(),"missing type query never calls comparator");}
 {Fixture f;f.queries[0].query_slot_00=nullptr;unsupported([&]{f.run();});}
 {Fixture f;std::array<s::IoEnvironmentTypeBinding,2> values{{f.queries[0],f.queries[0]}};f.context.type_queries=values;unsupported([&]{f.run();});check(f.events.empty(),"ambiguous query rejects before callback");}
 for(unsigned kind=0;kind<4;++kind){Fixture f;std::array<s::IoEnvironmentFactoryBinding,2> values{{f.factories[0],f.factories[0]}};if(kind==0)f.context.factories={};if(kind==1)f.factories[0].construct_slot_58=nullptr;if(kind==2)f.source.executor_28=nullptr;if(kind==3)f.context.factories=values;unsupported([&]{f.run();});check(f.events==std::vector<std::string>{"type","compare"},"invalid factory rejected before source query");}
 {Fixture f;f.source.operations_10=nullptr;unsupported([&]{f.run();});check(f.events==std::vector<std::string>{"type","compare"},"missing live operations bounded before query");}
 {Fixture f;f.empty.move=[&](auto&,auto& from){from.operations_10=nullptr;};f.result=0;unsupported([&]{f.run();});}
}
}
int main(){try{defaults();nonmatching();exceptions();registries();std::cout<<"PASS: "<<checks<<" socket environment constructor core checks; actual023dd0, zero and signed nonzero comparator, wrapped type identity, cached factory despite executor deletion, live moves and guard cleanup, exact exception prefixes and bounded registries\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
