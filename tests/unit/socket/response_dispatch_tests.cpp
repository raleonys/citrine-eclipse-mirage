#include "tests/support/socket/response_dispatch_test_fixture.hpp"
using namespace response_test;
namespace {
void dispatch_paths(){
    for(bool owned:{false,true})for(bool take:{false,true})for(auto cap:{15ULL,31ULL,4095ULL}){
        Fixture f;f.m.seed(f.m.response,"payload",cap);f.task.response_08=f.m.response.value;f.schedule.borrowed_slot_18=owned?nullptr:&borrowed;f.take_owned=take;const auto prefix=f.task.response_08.storage_00;f.run();
        check(f.moves==3&&f.queries==2&&f.task.response_08.size_10==0&&f.task.response_08.capacity_18==15&&f.task.response_08.storage_00[0]==0,"021420 consumes incoming task before three actual023cb0 moves");
        check(std::equal(prefix.begin()+1,prefix.end(),f.task.response_08.storage_00.begin()+1)&&!f.task.control_30&&!f.task.object_28,"task move keeps prefix bytes and clears only owned pair");
        check(f.trace[0]=="query1"&&f.trace[1]=="factory20"&&f.trace[2]=="move1"&&f.trace[3]=="query2"&&f.trace[4]=="factory88"&&f.trace[5]=="move2"&&f.trace[6]=="move3"&&f.trace[7]=="destroy-empty","exact factory/query/move/unconditional intermediate destruction order");
        if(owned&&take){check(f.retained.size()==1&&f.m.base.owner.strong_08==2&&f.m.frees.empty(),"owned publish takes mutable slot and retains payload/reference");}
        else check(f.m.base.owner.strong_08==1&&f.m.frees.size()==(cap>15?1U:0U),"borrowed residual or owned discard cleans exact single string/reference");
        check(f.trace[f.trace.size()-1]=="destroy-live"||!owned,"final and first environments destroyed after scheduling/discard");
    }
}
void callbacks(){
    {Fixture f;f.hook=[&](auto e){if(e=="query1")f.executors[0].factory_20=nullptr;if(e=="query2")f.executors[1].factory_88=nullptr;};f.run();check(f.moves==3,"factory20/88 FUNCTIONS cached before respective query callbacks mutate their bindings");}
    {Fixture f;f.move_hook=[&](auto&,auto& from,unsigned n){if(n==1)from.operations_10=&f.live;};f.hook=[&](auto e){if(e=="destroy-live"&&f.moves==1){f.first->executor_28=&f.third_executor;f.executors[1].factory_88=nullptr;}};f.run();check(f.moves==3&&std::find(f.trace.begin(),f.trace.end(),"destroy-live")<std::find(f.trace.begin(),f.trace.end(),"query2"),"conditional first cleanup precedes LIVE first.executor factory88 selection");}
    {Fixture f;f.schedule.borrowed_slot_18=nullptr;f.hook=[&](auto e){if(e=="make")f.schedule.owned_slot_10=nullptr;};f.run();check(f.makes==1&&f.calls==1,"owned publish FUNCTION cached before020cb0 provider callback");}
    for(auto strong:{0U,1U,2U,UINT32_MAX})for(auto weak:{0U,1U,2U,UINT32_MAX}){Fixture f;r::Ref residual,other;residual.strong_08=strong;residual.weak_0c=weak;
        f.hook=[&](auto e){if(e=="borrowed")f.local->control_30=&residual;};residual.dispose=[&]{f.local->control_30=&other;};f.run();
        check(residual.strong_08==strong-std::uint32_t{1}&&residual.disposes==(strong==1?1U:0U),"021420 residual strong decrement old1 only");
        check(residual.weak_0c==(strong==1?weak-std::uint32_t{1}:weak)&&residual.deletes==(strong==1&&weak==1?1U:0U)&&!other.deletes,"021420 weak AND delete use same cached control despite task mutation");
    }
    for(auto phase:{"query1","factory20","move1","query2","factory88","move2","move3","borrowed"}){Fixture f;f.hook=[&](auto e){if(e==phase)throw Failure{99};};rejects<Failure>([&]{f.run();});check(!f.task.control_30&&f.task.response_08.size_10==0&&f.m.base.owner.strong_08==2,"required provider exception keeps consumed-task normal prefix without invented cleanup/unwind");}
    for(unsigned kind=0;kind<5;++kind){Fixture f;if(kind==0)f.context.executors={};if(kind==1)f.executors[2]=f.executors[0];if(kind==2)f.context.schedules={};if(kind==3)f.schedules[1]=f.schedules[0];if(kind==4){f.schedule.borrowed_slot_18=nullptr;f.schedule.owned_slot_10=nullptr;}rejects<s::UnsupportedInput>([&]{f.run();});check(!f.task.control_30&&f.task.response_08.size_10==0,"bounded missing/duplicate factories/schedules preserve observed initial transfer");}
}
void queue_matrix(){
    for(auto count:{0ULL,1ULL,UINT64_MAX})for(auto offset:{0ULL,3ULL,UINT64_MAX})for(auto map_size:{1ULL,2ULL,6ULL,8ULL})for(bool allocate:{false,true}){
        Fixture f;f.queue.count_e8=count;f.queue.offset_e0=offset;f.queue.map_size_d8=map_size;for(auto& slot:f.map)slot=allocate?nullptr:&f.cells[0];
        auto expected_size=map_size;const bool grown=map_size<=count+std::uint64_t{1};if(grown)expected_size=8;
        const auto normalized=(expected_size-1)&offset,index=(expected_size-1)&(count+normalized);const auto input=f.task.response_08;
        f.enqueue(f.task.response_08);
        check(f.queue.count_e8==count+std::uint64_t{1}&&f.queue.offset_e0==normalized&&f.grows==(grown?1U:0U),"independent unsigned count wrap and bitmask offset oracle; growth only native predicate");
        check(f.map[index]->storage_00==input.storage_00&&f.map[index]->size_10==input.size_10&&f.map[index]->capacity_18==input.capacity_18,"queue receives exact moved four-qword string prefix");
        check(f.task.response_08.size_10==0&&f.task.response_08.capacity_18==15&&f.allocations==(allocate?1U:0U)&&f.nexts==(count==0?1U:0U),"only missing slot allocates and only new count1 starts queued write");
    }
}
void queue_live(){
    {Fixture f;std::array<s::WriteFinishString*,8> replacement{};f.queue.count_e8=2;f.queue.offset_e0=1;
        // Initial index3 is cached for store, current size2 recomputes index1
        // for destination from cached absolute3. Both maps stay bounded.
        replacement[1]=&f.cells[1];f.allocation_hook=[&]{f.queue.map_d0=replacement;f.queue.map_size_d8=2;f.queue.count_e8=77;f.queue.offset_e0=99;};f.enqueue(f.task.response_08);
        check(replacement[3]==&f.cells[7]&&replacement[1]->size_10==5&&f.queue.count_e8==78&&f.queue.offset_e0==99,"allocation reloads map for cached-slot store and recomputes destination from live map size plus cached absolute index");}
    {Fixture f;f.next_hook=[&]{f.m.seed(f.m.extra,"late",4095);f.task.response_08=f.m.extra.value;};f.enqueue(f.task.response_08);check(f.m.frees.size()==1&&f.task.response_08.size_10==0,"response residual cleanup reloads capacity/heap/header AFTER queued-write callback");}
    {Fixture f;auto queue=std::make_unique<s::WriteFinishOwner>(f.queue);f.owners[0].owner=queue.get();f.next_hook=[&]{queue.reset();};f.enqueue(f.task.response_08);check(!queue&&f.task.response_08.capacity_18==15,"queued-write may destroy queue/owner projection; caller touches only response afterward");}
    {Fixture f;f.queue.count_e8=1;f.queue_context.queued=nullptr;f.map.fill(&f.cells[0]);f.enqueue(f.task.response_08);check(!f.nexts&&f.queue.count_e8==2,"nonfirst queued item never requires queued-write context");}
    for(unsigned mode=0;mode<3;++mode){Fixture f;if(mode==0)f.queue_context.owners={};if(mode==1)f.owners[1]=f.owners[0];if(mode==2)f.queue.map_d0={};rejects<s::UnsupportedInput>([&]{f.enqueue(f.task.response_08);});check(f.task.response_08.size_10==5,"bounded queue identity/map errors preserve input until actual move");}
    {Fixture f;f.queue_context.queued=nullptr;rejects<s::UnsupportedInput>([&]{f.enqueue(f.task.response_08);});check(f.queue.count_e8==1&&f.task.response_08.size_10==0,"unbound first-start rejects AFTER committed insertion/move without rollback");}
}
inline std::function<void(std::uintptr_t,std::uintptr_t,s::ReceiveBufferString&,s::ResponseDispatchContext&)> capture;
void write(std::uintptr_t function,std::uintptr_t owner,s::ReceiveBufferString& value,s::ResponseDispatchContext& context){capture(function,owner,value,context);}
void borrowed_entry(){Fixture f;f.task.function_00=0xabc;f.task.object_28=0xdef;f.m.seed(f.m.response,"owned",31);f.task.response_08=f.m.response.value;f.hook=[&](auto e){if(e=="string-copy"){f.task.function_00=0x999;f.task.object_28=0x888;}};
    capture=[&](auto function,auto owner,auto& value,auto& context){check(function==0xabc&&owner==0xdef&&&context==&f.context&&f.m.text(value)=="owned"&&s::receive_buffer_address(value)!=s::receive_buffer_address(f.task.response_08),"022240 caches function/owner before009eb0 and forwards independent returned copy");f.m.free_string(value);};
    s::invoke_borrowed_response_022240(f.task,f.context,&write);check(f.m.text(f.task.response_08)=="owned"&&f.task.response_08.capacity_18==31&&f.task.control_30==&f.m.base.owner&&f.m.base.owner.strong_08==2,"borrowed invoke preserves source response and reference for outer cleanup");f.m.free_string(f.task.response_08);
}
}
int main(){try{dispatch_paths();callbacks();queue_matrix();queue_live();borrowed_entry();std::cout<<"PASS: "<<checks<<" socket response dispatch core checks; actual021420/028270/022240 bodies, factory caching/three environment moves, borrowed/owned ownership, deque wrap/live allocation reload and response residual cleanup\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
