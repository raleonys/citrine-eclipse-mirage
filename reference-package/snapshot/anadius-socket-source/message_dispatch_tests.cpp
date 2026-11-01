#include "message_dispatch_test_fixture.hpp"
using namespace message_test;
namespace {
void normal(){
    for(auto request_cap:{15ULL,16ULL,4094ULL,4095ULL})for(auto response_cap:{15ULL,16ULL,4095ULL})for(auto text:{std::string{},std::string("reply"),std::string("a\0b",3)}){
        Fixture f;f.seed(f.request,"request",request_cap);f.seed(f.response,text,response_cap);f.dispatch();
        check(f.scheduled==text&&f.api.processes==1&&f.api.schedules==1&&f.copy_calls==2,"nonempty request dispatches exact full response including empty or embedded NUL");
        check(f.base.owner.strong_08==1&&!f.base.owner.disposes&&f.request.value.size_10==0&&f.request.value.capacity_18==15,"temporary strong acquisition balances and request is consumed");
        check(f.trace[0]=="process"&&f.trace[1]=="copy1"&&f.trace[2]=="copy2"&&f.trace[3]=="schedule"&&f.trace[4]=="destroy-b"&&f.trace[5]=="destroy-b","exact process/two copy/schedule/reverse environment destroy order");
        check(f.frees.size()==static_cast<unsigned>((request_cap>15)+(response_cap>15)),"only live residual task response and input heaps freed");
    }
    for(auto cap:{15ULL,16ULL,4095ULL}){Fixture f;f.seed(f.request,"",cap);f.context.owners={};f.context.copies={};f.base.projection.control_08=nullptr;f.dispatch();check(f.api.empty==1&&!f.api.processes&&!f.copy_calls&&!f.api.schedules&&f.request.value.capacity_18==15,"empty request only delegates00aa50; no owner/weak/executor dependency");}
}
void lifetimes(){
    Fixture f;f.copy_hook=[&](auto& to,auto& from,unsigned n){if(n==1){from.executor_28=&f.other_executor;to.schedule_20=0x999;}else{check(from.executor_28==&f.other_executor,"owner executor reloaded AFTER first copy");from.executor_28=&f.executor;to.operations_10=&f.c;}};
    f.hook=[&](auto event){if(event=="schedule"){check(f.api.second->executor_28==&f.executor&&f.api.second->schedule_20==0x999,"second executor copied AFTER second callback and live schedule retained");f.api.second->operations_10=&f.a;}if(event=="destroy-a")f.api.first->operations_10=&f.c;};f.dispatch();
    check(f.trace==std::vector<std::string>({"process","copy1","copy2","schedule","destroy-a","destroy-c"}),"destroy tables remain live through dispatch and prior destruction callback");
    for(auto strong:{0U,1U,2U,UINT32_MAX})for(auto weak:{0U,1U,2U,UINT32_MAX}){Fixture g;r::Ref residual,target;residual.strong_08=strong;residual.weak_0c=weak;
        g.hook=[&](auto event){if(event=="schedule")g.api.task->control_30=&residual;};residual.dispose=[&]{g.api.task->control_30=&target;};g.dispatch();
        check(residual.strong_08==strong-std::uint32_t{1}&&residual.disposes==(strong==1?1U:0U),"strong decrement and old1 dispose on captured residual");
        check(residual.weak_0c==(strong==1?weak-std::uint32_t{1}:weak)&&residual.deletes==0&&target.deletes==(strong==1&&weak==1?1U:0U),"cached residual WEAK decrement but live task DELETE target after dispose");
    }
    {Fixture g;g.seed(g.response,"owned",31);g.hook=[&](auto e){if(e=="schedule"){g.api.task->response_08.size_10=0;g.api.task->response_08.capacity_18=15;g.api.task->response_08.storage_00[0]=0;g.api.task->control_30=nullptr;}};g.dispatch();check(g.base.owner.strong_08==2&&g.frees.empty(),"callee can take task string/control; caller does not release moved state");}
    {Fixture g;auto owner=std::make_unique<s::MessageDispatchOwner>(g.owner);g.owners[0].owner=owner.get();g.hook=[&](auto e){if(e=="schedule")owner.reset();};g.dispatch();check(!owner&&g.base.owner.strong_08==1,"schedule can end owner projection lifetime; no later owner reads");}
}
void acquisition_and_failures(){
    for(auto strong:{1U,2U,0x7fffffffU,0x80000000U,UINT32_MAX}){Fixture f;f.base.owner.strong_08=strong;f.hook=[&](auto e){if(e=="schedule")check(f.base.owner.strong_08==strong+std::uint32_t{1},"real unsigned32 CAS includes UINTMAX-to-zero");};f.dispatch();check(f.base.owner.strong_08==strong&&!f.base.terminal.calls,"normal cleanup restores any nonzero initial count");}
    for(bool absent:{false,true}){Fixture f;if(absent)f.base.projection.control_08=nullptr;else f.base.owner.strong_08=0;rejects<frame_test::Failure>([&]{f.dispatch();});check(f.api.processes==1&&!f.copy_calls&&!f.api.schedules&&f.base.terminal.calls==1&&f.request.value.size_10==7,"bad weak failure occurs AFTER core response but before executor and input cleanup");}
    for(auto phase:{"process","copy1","copy2","schedule","destroy-b","free-string"}){Fixture f;f.seed(f.request,"input",31);f.seed(f.response,"output",31);f.hook=[&](auto e){if(e==phase)throw Failure{99};};rejects<Failure>([&]{f.dispatch();});check(f.request.value.size_10==5,"required provider failure propagates without invented request reset/unwind");}
    for(unsigned kind=0;kind<5;++kind){Fixture f;if(kind==0)f.context.owners={};if(kind==1)f.owners[1]=f.owners[0];if(kind==2)f.owner.environment_78=nullptr;if(kind==3)f.context.copies={};if(kind==4)f.copies[2]=f.copies[0];rejects<s::UnsupportedInput>([&]{f.dispatch();});check(f.api.processes==1&&!f.api.schedules,"bounded identity failures preserve core call prefix");}
}
void header_bounds(){
    for(auto distance:{0ULL,7ULL,8ULL,39ULL,40ULL,UINT64_MAX}){Fixture f;f.seed(f.response,"reply",4095);const auto address=s::receive_buffer_address(f.response.value);f.headers[f.header_count-1].allocation_base=address-distance;
        const bool valid=distance>=8&&distance<=39;f.hook=[&](auto e){if(e=="free-string")throw Failure{98};};
        try{f.dispatch();throw std::runtime_error("missing bounded failure");}catch(const Failure& e){check(valid&&e.phase==98,"valid data-base8..39 reaches free");}catch(const frame_test::Failure& e){check(!valid&&e.code==12,"unsigned header outside range reaches invalid parameter");}
        check(f.api.schedules==1&&f.request.value.size_10==7,"header failure happens after dispatch but before request cleanup");
    }
}
inline std::function<void(std::uintptr_t,std::uintptr_t,s::ReceiveBufferString&,s::MessageDispatchContext&)> invoke;
void capture(std::uintptr_t function,std::uintptr_t owner,s::ReceiveBufferString& input,s::MessageDispatchContext& context){invoke(function,owner,input,context);}
void trampoline(){
    for(auto cap:{15ULL,31ULL,4095ULL}){Fixture f;f.seed(f.request,"message",cap);f.memory.closure=std::make_unique<s::FrameMessageClosure>();auto* closure=f.memory.closure.get();closure->message_00=f.request.value;closure->owner_20=0xabc;closure->function_28=0xdef;const auto original=closure->message_00.storage_00;
        invoke=[&](auto function,auto owner,auto& request,auto&){check(function==0xdef&&owner==0xabc&&f.text(request)=="message","cached function/owner and moved full string passed to callee");check(closure->message_00.size_10==0&&closure->message_00.capacity_18==15&&closure->message_00.storage_00[0]==0&&std::equal(original.begin()+1,original.end(),closure->message_00.storage_00.begin()+1),"closure reset keeps remaining15 prefix bytes intact");f.free_string(request);f.event("invoke");};
        f.hook=[&](auto e){if(e=="exit-waits"){f.seed(f.extra,"late",31);closure->message_00=f.extra.value;}};
        check(!s::message_thread_entry_021770(*closure,f.context,&capture)&&!f.memory.closure&&f.api.cleanups==1,"normal thread result0, exit-wait cleanup and real owned closure deletion");
        check(f.trace.back()=="free-closure"&&f.frees.back().second==48,"closure residual string freed/reset after exit-waits then closure freed48");
    }
    for(auto phase:{"invoke","exit-waits","free-string"}){Fixture f;f.memory.closure=std::make_unique<s::FrameMessageClosure>();f.memory.closure->message_00=f.request.value;
        invoke=[&](auto,auto,auto& value,auto&){f.free_string(value);f.event("invoke");};f.hook=[&](auto e){if(e=="exit-waits"){f.seed(f.extra,"late",31);f.memory.closure->message_00=f.extra.value;}if(e==phase)throw Failure{99};};
        rejects<Failure>([&]{s::message_thread_entry_021770(*f.memory.closure,f.context,&capture);});check(bool(f.memory.closure),"trampoline exception keeps observed prefix, no invented closure destructor");
    }
    {Fixture f;f.memory.closure=std::make_unique<s::FrameMessageClosure>();f.memory.closure->message_00=f.request.value;f.memory.closure->function_28=0x123;
        rejects<s::UnsupportedInput>([&]{s::message_thread_entry_021770(*f.memory.closure,f.context);});check(f.memory.closure->message_00.size_10==0&&!f.api.cleanups,"unknown function token rejected at invocation after native move/reset prefix");}
    for(unsigned mode=0;mode<4;++mode){Fixture f;f.memory.closure=std::make_unique<s::FrameMessageClosure>();const auto ptr=reinterpret_cast<std::uintptr_t>(f.memory.closure.get());f.closures[0]={ptr,f.memory.closure.get()};if(mode==0)f.context.closures={};if(mode==1)f.closures[1]=f.closures[0];if(mode==2)f.closures[0].closure=nullptr;rejects<s::UnsupportedInput>([&]{s::route_message_thread_entry(mode==3?0x123:0x180021770ULL,ptr,f.context);});check(!f.api.processes&&!f.api.cleanups&&f.frees.empty(),"opaque router rejects missing/duplicate/wrong entry without dereferencing image VAs");}
}
}
int main(){try{normal();lifetimes();acquisition_and_failures();header_bounds();trampoline();std::cout<<"PASS: "<<checks<<" socket message dispatch core checks; actual021770/028d30 normal bodies, two executor copies, live delete target, precise string/control/closure ownership and bounded call-prefix tests\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
