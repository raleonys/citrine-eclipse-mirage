#include "owned_response_test_fixture.hpp"
using namespace owned_test;
namespace {
void factory(){
    for(bool tls:{false,true})for(unsigned cached:{0U,4U,5U}){
        OwnedFixture o;if(tls)o.use_tls();if(cached){auto& p=o.memory.allocate_04b0bc(73);p.bytes[0]=std::byte{23};o.state.cache_00[cached]=p.identity;}
        const auto n=o.memory.allocations.size();const auto before=o.f.task.response_08;auto* control=o.f.task.control_30;o.make();
        auto& p=static_cast<s::OwnedResponseTask*>(o.output)->payload;
        check(p.function_00==0x180028270ULL&&p.object_28==0x12000&&p.control_30==control,"factory transfers original task fields and reference");
        check(p.response_08.storage_00==before.storage_00&&p.response_08.size_10==before.size_10&&p.response_08.capacity_18==before.capacity_18,"all four string qwords transferred");
        check(!o.f.task.object_28&&!o.f.task.control_30&&!o.f.task.response_08.size_10&&o.f.task.response_08.capacity_18==15&&!o.f.task.response_08.storage_00[0],"source ownership and string reset before output return");
        check(o.f.task.function_00==0x180028270ULL&&o.f.task.response_08.storage_00[1]==before.storage_00[1],"reset leaves function and remaining string storage intact");
        auto& block=o.memory.at(o.allocated_id);check(block.storage.bytes.size()==73&&block.bytes[72]==(tls&&cached?std::byte{23}:std::byte{18}),"native021900 request48 produces49-byte allocation and correct marker");
        check(o.memory.allocations.size()==n+(tls&&cached?0U:1U)&&block.bytes[64]==std::byte{0xa5}&&block.bytes[71]==std::byte{0xa5},"cache hit avoids allocation; unknown padding is not initialized");
        check(o.worker.indices==std::vector<std::uint32_t>{17}&&o.f.m.base.owner.strong_08==2,"factory one TLS lookup and no invented shared retain/release");
        o.output->call_slot_00(0);
    }
    {OwnedFixture o;o.worker.get=[&]()->void*{o.f.task.function_00=91;o.f.task.object_28=92;return nullptr;};o.make();auto& p=static_cast<s::OwnedResponseTask*>(o.output)->payload;check(p.function_00==91&&p.object_28==92,"source fields read after TLS provider");o.output->call_slot_00(0);}
    {OwnedFixture o;o.memory.allocating=[](){throw response_test::Failure{1};};auto before=o.f.task;o.output=reinterpret_cast<s::PostedFunctor*>(1);rejects<response_test::Failure>([&]{o.make();});check(o.output==reinterpret_cast<s::PostedFunctor*>(1)&&o.f.task.control_30==before.control_30&&o.f.task.response_08.storage_00==before.response_08.storage_00,"allocation exception leaves output and source ownership untouched");}
    {OwnedFixture o;o.use_tls();auto& p=o.memory.allocate_04b0bc(72);p.bytes[0]=std::byte{18};o.state.cache_00[4]=p.identity;rejects<s::UnsupportedInput>([&]{o.make();});check(!o.state.cache_00[4]&&o.memory.projected==0,"bounded marker failure after cache detach but before projection");}
}
void invoke(){
    for(unsigned flag:{0U,1U,2U,255U,256U})for(unsigned occupancy:{0U,1U,2U})for(bool tls:{false,true}){
        OwnedFixture o;const auto text=std::string("reply\0tail",10);o.f.m.seed(o.f.m.response,text);o.f.task.response_08=o.f.m.response.value;o.make();
        if(tls)o.use_tls();o.state.cache_00[0]=0xbeef;if(occupancy)o.state.cache_00[4]=0x111;if(occupancy==2)o.state.cache_00[5]=0x222;
        o.index=29;const auto id=o.allocated_id;const bool freed=!tls||occupancy==2;
        o.api.copying=[&](auto& source){check(o.memory.live(id)!=freed,"allocation physically destroyed or cached before callback COPY");check(o.worker.indices==std::vector<std::uint32_t>({17,29}),"fresh live TLS index, no redundant tail lookup");check(o.f.m.text(source)==text,"owned local string survives allocation release");if(!freed)check(o.memory.at(id).bytes[0]==std::byte{18}&&o.state.cache_00[4+occupancy]==id,"marker copied before caching callback");};
        s::call_owned_response_022280(*static_cast<s::OwnedResponseTask*>(o.output),static_cast<std::uint8_t>(flag),o.owned,&s::invoke_owned_response_0219d0,&capture);
        const bool called=static_cast<std::uint8_t>(flag)!=0;check(o.writes==(called?1U:0U)&&o.api.copies==(called?1U:0U),"only nonzero LOW BYTE invokes0219d0");
        check(!called||o.result==text,"independent copy preserves embedded NUL");check(o.f.m.base.owner.strong_08==1&&o.state.cache_00[0]==0xbeef,"exact one residual release, unrelated cache unchanged");
        check(o.memory.frees.size()==(freed?1U:0U)&&o.worker.indices.size()==2,"one allocation release even discard; no late phantom cleanup");
    }
    for(std::uint32_t strong:{0U,1U,2U,UINT32_MAX})for(std::uint32_t weak:{0U,1U,2U}){
        OwnedFixture o;Ref ref;ref.strong_08=strong;ref.weak_0c=weak;o.f.task.control_30=&ref;o.make();o.output->call_slot_00(0);
        check(ref.strong_08==strong-1U&&ref.disposes==(strong==1?1U:0U),"strong unsigned32 decrement/dispose old1");
        check(ref.weak_0c==(strong==1?weak-1U:weak)&&ref.deletes==(strong==1&&weak==1?1U:0U),"weak decrement/delete cached control old1");
    }
    {OwnedFixture o;auto ref=std::make_unique<Ref>();o.f.task.control_30=ref.get();ref->deleting=[&](){ref.reset();};o.make();o.output->call_slot_00(0);check(!ref&&!o.memory.live(o.allocated_id),"actual control and closure lifetimes may both end");}
}
s::MessageWriteTask* local=nullptr;Ref* replacement=nullptr;
void replace_control(s::MessageWriteTask& task,s::ResponseDispatchContext&,s::ResponseWriteCall){local=&task;task.control_30=replacement;}
void saving_invoke(s::MessageWriteTask& task,s::ResponseDispatchContext& c,s::ResponseWriteCall call){local=&task;s::invoke_owned_response_0219d0(task,c,call);}
void lifetime(){
    {OwnedFixture o;Ref original,live,other;o.f.task.control_30=&original;replacement=&live;live.disposing=[&](){local->control_30=&other;};o.make();s::call_owned_response_022280(*static_cast<s::OwnedResponseTask*>(o.output),1,o.owned,&replace_control);
        check(!original.disposes&&live.disposes==1&&live.deletes==1&&!other.disposes,"post-invoke control live reload, dispose/delete use same cached target");}
    {OwnedFixture o;o.make();o.api.copying=[&](auto&){local->function_00=1;local->object_28=2;};s::call_owned_response_022280(*static_cast<s::OwnedResponseTask*>(o.output),1,o.owned,&saving_invoke,&capture);check(o.writes==1,"0219d0 function/owner cached across COPY mutation");}
    for(std::size_t size:{16U,4094U,4095U,8192U}){
        OwnedFixture o;const auto text=std::string(size,'H');o.f.m.seed(o.f.m.response,text);o.f.task.response_08=o.f.m.response.value;o.make();
        o.api.copying=[&](auto& source){check(o.f.m.text(source)==text&&o.f.m.frees.empty(),"independent COPY reads still-owned heap source after closure deletion");};
        s::call_owned_response_022280(*static_cast<s::OwnedResponseTask*>(o.output),1,o.owned,&s::invoke_owned_response_0219d0,&capture);
        check(o.result==text&&o.f.m.frees.size()==2,"copy and original heap payload each freed once");
        check(o.f.m.frees.back().second==size+1+(size+1>=4096?39U:0U),"original sizedfree switches at4096 bytes including NUL");
    }
    {OwnedFixture o;o.use_tls();o.make();const auto id=o.allocated_id;auto* first=o.output;
        o.api.copying=[&](auto&){check(o.state.cache_00[4]==id,"callback sees released allocation in cache");s::MessageWriteTask next;next.response_08.storage_00[0]='N';s::PostedFunctor* second=nullptr;o.api.make_owned_response_020cb0(second,next);check(static_cast<s::OwnedResponseTask*>(second)->storage.identity==id&&second!=nullptr,"reentrant factory reuses same native allocation");o.held.push_back(second);};
        s::call_owned_response_022280(*static_cast<s::OwnedResponseTask*>(first),1,o.owned,&s::invoke_owned_response_0219d0,&capture);check(o.memory.allocations.size()==1&&o.memory.projected==2&&o.writes==1,"old call continues solely from local after projection replaced");o.held.back()->call_slot_00(0);
    }
    {OwnedFixture o;o.make();o.api.copying=[](auto&){throw response_test::Failure{9};};rejects<response_test::Failure>([&]{o.output->call_slot_00(1);});check(!o.memory.live(o.allocated_id)&&o.f.m.base.owner.strong_08==2,"COPY exception propagates after release without invented normal-tail cleanup");}
    {OwnedFixture o;o.make();o.owned.response=nullptr;rejects<s::UnsupportedInput>([&]{o.output->call_slot_00(1);});check(!o.memory.live(o.allocated_id),"unbound invoke fails at callback boundary after release");}
    {OwnedFixture o;o.make();o.owned.response=nullptr;o.output->call_slot_00(0);check(o.f.m.base.owner.strong_08==1,"discard does not require invocation context");}
}
void boundaries(){
    for(std::uint64_t distance:{0ULL,7ULL,8ULL,39ULL,40ULL,UINT64_MAX}){
        OwnedFixture o;o.f.m.seed(o.f.m.response,std::string(4095,'B'));o.f.task.response_08=o.f.m.response.value;
        const auto data=s::receive_buffer_address(o.f.task.response_08);o.f.m.headers[0].allocation_base=data-distance;
        o.f.m.hook=[](auto name){if(name=="free-string")throw response_test::Failure{98};};o.make();bool observed=false;
        try{o.output->call_slot_00(0);}catch(const frame_test::Failure& e){observed=true;check(e.code==12&&(distance<8||distance>39),"invalid header uses unsigned data-base-8 range");}
        catch(const response_test::Failure& e){observed=true;check(e.phase==98&&distance>=8&&distance<=39,"inclusive base displacement8..39 accepted before sizedfree");}
        check(observed&&!o.memory.live(o.allocated_id)&&o.f.m.base.owner.strong_08==1,"header terminal occurs after allocation release and local control cleanup");
    }
    for(bool duplicate:{false,true}){
        OwnedFixture o;o.f.m.seed(o.f.m.response,std::string(4095,'B'));o.f.task.response_08=o.f.m.response.value;
        if(duplicate)o.f.m.headers[1]=o.f.m.headers[0];else o.f.m.headers[0]={};o.make();
        rejects<s::UnsupportedInput>([&]{o.output->call_slot_00(0);});check(o.f.m.frees.empty()&&!o.memory.live(o.allocated_id),"missing/ambiguous allocation header rejected without fabricated free");
    }
    {OwnedFixture o;o.make();s::WorkerTlsContext top{nullptr,nullptr,&o.tls};o.worker.get=[&](){return &top;};o.output->call_slot_00(0);check(!o.state.cache_00[4]&&!o.memory.live(o.allocated_id),"null TLS state does not follow previous TLS chain");}
    {OwnedFixture o;o.make();o.use_tls();auto& p=o.memory.at(o.allocated_id);p.storage.bytes=std::span<std::byte>(p.bytes.data(),72);
        rejects<s::UnsupportedInput>([&]{o.output->call_slot_00(1);});check(!o.state.cache_00[4]&&p.task->payload.control_30==nullptr&&!p.task->payload.response_08.size_10&&o.f.m.base.owner.strong_08==2,"marker bounds failure after payload move; no rollback or callback tail invented");}
}
}
int main(){try{factory();invoke();lifetime();boundaries();std::cout<<"PASS: "<<checks<<" socket owned response checks; actual020cb0/021900/022280/0219d0, early destruction, cache reuse, byte flags, independent copy and residual cleanup\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
