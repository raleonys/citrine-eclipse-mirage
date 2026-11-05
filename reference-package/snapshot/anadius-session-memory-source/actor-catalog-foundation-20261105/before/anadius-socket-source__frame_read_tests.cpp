#include "frame_read_test_fixture.hpp"
using namespace frame_test;
namespace {
void success(){
    for(const auto& text:{std::string{},std::string("abc"),std::string("a\0bc\0tail",9),std::string(31,'x'),std::string(4096,'y')})
    for(auto n:{0ULL,1ULL,2ULL,4ULL,100ULL,UINT64_MAX})for(bool null_alloc:{false,true}){
        Fixture f;f.seed(text,std::max<std::uint64_t>(15,text.size()));f.api.null_allocation=null_alloc;
        f.finish(n);const auto consumed=std::min<std::uint64_t>(n,text.size());
        const auto expected=n==0?text:text.substr(0,std::min<std::uint64_t>(n-1,text.size()));
        check(f.api.delivered==expected,"independent payload oracle preserves matched0 unsigned underflow and strips one delimiter");
        check(f.base.buffer.text()==text.substr(consumed)&&f.copy.count==text.size()-consumed+1,"consume clamps and copies trailing data plus NUL");
        check(f.api.restarts==1&&f.api.trace.back()=="restart"&&f.owner.strong_08==1,"success restarts once after detach and erase without touching handler ownership");
        if(!null_alloc){check(f.api.closure->owner_20==0x12000&&f.api.closure->function_28==0x180028d30ULL,"closure carries live owner and opaque application function");check(f.api.closure->message_00.size_10==expected.size()&&f.api.frees.empty(),"moved message ownership remains with closure");}
        else check(f.api.frees.size()==(expected.size()>15?1U:0U),"null allocation retains and destroys local copied message on normal return");
        auto bytes=f.base.buffer.memory.view(s::receive_buffer_address(f.base.buffer.value),f.base.buffer.value.size_10+1);check(bytes.back()==0,"erase retains terminating NUL");
    }
}
void failures(){
    for(unsigned mode=0;mode<5;++mode){Fixture f;f.seed("abc\0tail",15);const auto before=f.base.buffer.text();
        if(mode==0)f.api.handle=0;if(mode==1)f.api.id=0;if(mode==2)f.api.detached=1;if(mode==3)f.api.detached=0x100;if(mode==4)f.api.detached=INT32_MIN;
        try{f.finish(4);throw std::runtime_error("missing thread terminal");}catch(const Failure& e){check(e.code==(mode==0?6:1),"exact thread error index; detach checks full32 rather than low8");}
        check(f.base.buffer.text()==before&&!f.copy.calls&&!f.api.restarts&&f.api.closure,"failure before erase/restart preserves input and normal closure transfer");
        check((std::find(f.api.trace.begin(),f.api.trace.end(),"detach")!=f.api.trace.end())==(mode>=2),"no detach on null handle/id");
    }
    for(const auto& event:{"construct","allocate","begin","detach"}){Fixture f;f.seed("abc");f.api.hook=[&](auto e){if(e==event)throw Failure{99};};rejects<Failure>([&]{f.finish(2);});check(f.base.buffer.text()=="abc"&&!f.copy.calls&&!f.api.restarts,"provider exception preserves observed prefix without invented unwind");}
}
void live_owner(){
    Fixture f;f.seed("initial");b::Fixture other;other.seed(15,6);auto data=other.memory.view(s::receive_buffer_address(other.value),7);std::memcpy(data.data(),"ABtail",7);
    s::FrameReadOwner replacement{0x777,&f.owner,&other.value};f.owners[1]={0x13000,&replacement};
    // Byte regions remain one coherent projection across owner replacement.
    f.base.buffer.regions[31]=other.regions[0];
    f.api.hook=[&](auto event){if(event=="allocate")f.base.handler.words_88_98[2]=0x13000;};
    f.copy.after=[&]{other.value.size_10=123;f.base.handler.words_88_98[2]=0x14000;};
    restart_capture=[&](auto id){check(id==0x14000&&other.value.size_10==4,"size store targets saved string and restart reloads owner AFTER byte copy");++f.api.restarts;};
    f.finish(2);check(f.api.delivered=="i"&&f.api.closure->owner_20==0x13000&&other.text()=="tail"&&f.base.buffer.text()=="initial","copy snapshot, closure reload, postdetach input reload and postcopy owner reload are distinct");
    {Fixture g;g.seed("first");g.api.hook=[&](auto event){if(event=="begin")g.api.closure.reset();};g.finish(1);check(!g.api.closure&&g.api.restarts==1,"thread may destroy closure before begin returns; caller never rereads it");}
}
void logging(){
    for(auto code:{2,3,-1,INT32_MIN})for(bool same:{false,true})for(auto capacity:{15ULL,16ULL,4094ULL,4095ULL,8191ULL}){
        Fixture f;f.seed("input");f.copied_seed(capacity,7);f.base.incoming.value=code;f.category.identity_08=same?0x3333:0x2222;
        f.base.completion.hook=[&]{f.base.incoming.value=77;};f.finish(0);
        const bool suppressed=code==2&&same;check(f.base.completion.misc==1&&f.category.calls==(suppressed?0U:1U),"cached code and category name identity suppress only misc EOF2");
        check(f.category.code==(suppressed?0:code)&&!f.copy.calls&&!f.api.restarts&&f.base.buffer.text()=="input","error logs cached signed code and never consumes/rearms");
        if(!suppressed){check(f.api.trace==std::vector<std::string>({"log","message","append","widen","put","flush"})||(capacity>15&&f.api.trace==std::vector<std::string>({"log","message","append","widen","put","flush","free"})),"exact normal log order");check(f.api.frees.size()==(capacity>15?1U:0U),"inline message never freed; heap freed once");if(capacity>15)check(f.api.frees[0].second==capacity+1+(capacity>=4095?39:0),"4096-byte threshold adds39 to sized delete");}
    }
    {Fixture f;f.base.incoming.value=2;f.category.identity_08=0x3333;f.categories[0].category=nullptr;f.base.completion.hook=[&]{f.base.incoming.category={0x3333};};f.finish(0);check(!f.category.calls,"incoming category reloaded after misc provider");}
    for(auto distance:{0ULL,7ULL,8ULL,39ULL,40ULL,UINT64_MAX}){Fixture f;f.base.incoming.value=3;f.copied_seed(4095,2);const auto p=s::receive_buffer_address(f.copied.value);f.headers[0].allocation_base=p-distance;
        const bool valid=distance>=8&&distance<=39;f.api.hook=[&](auto e){if(e=="free")throw Failure{98};};
        try{f.finish(0);throw std::runtime_error("missing terminal");}catch(const Failure& e){check(e.code==(valid?98:12),"unsigned header range permits only data-base8..39");}
        check(!f.api.restarts&&!f.copy.calls,"invalid header never erases/rearms");
    }
}
void acquisition(){
    for(auto initial:{1U,2U,0x7fffffffU,0x80000000U,UINT32_MAX}){Fixture f;f.owner.strong_08=initial;
        start_capture=[&](auto& h,const auto& e,auto n,auto flag,auto& ctx,auto call){
            check(h.words_60_78==std::array<std::uint64_t,4>{0x12010,0x120a8,UINT64_MAX,UINT64_MAX}&&h.byte_80==0&&h.dword_84==0&&h.words_88_98==std::array<std::uint64_t,3>{0,0,0x12000},"native initial full64 state, NUL delimiter and literal members");
            check(h.object_a0==0xfeed&&h.control_a8==&f.owner&&f.owner.strong_08==initial+std::uint32_t{1},"real low32 CAS acquisition includes max-to-zero");
            check(e.value==0&&e.category.name_identity==0x1111&&!n&&flag==1&&&ctx==&f.base.context&&call==&s::start_async_receive_021d10,"initial error category binding, bytes0 flag1 and actual receive default");h.control_a8=nullptr;
        };f.start();check(!f.owner.disposes&&!f.terminal.calls,"moved reference is not decremented locally");
    }
    for(bool absent:{false,true}){Fixture f;if(absent)f.projection.control_08=nullptr;else f.owner.strong_08=0;rejects<Failure>([&]{f.start();});check(f.terminal.calls==1,"null or expired owner reaches026860");}
    for(auto strong:{0U,1U,2U,UINT32_MAX})for(auto weak:{0U,1U,2U,UINT32_MAX}){Fixture f;r::Ref residual,other;residual.strong_08=strong;residual.weak_0c=weak;s::ReceiveHandler* local=nullptr;
        start_capture=[&](auto& h,const auto&,auto,auto,auto&,auto){local=&h;h.control_a8=&residual;};residual.dispose=[&]{local->control_a8=&other;f.projection.control_08=&other;};f.start();
        check(residual.strong_08==strong-std::uint32_t{1}&&residual.disposes==(strong==1?1U:0U),"residual strong decrement only invokes dispose on old1");
        check(residual.weak_0c==(strong==1?weak-std::uint32_t{1}:weak)&&residual.deletes==(strong==1&&weak==1?1U:0U)&&other.strong_08==1,"weak/delete retain cached residual through disposal mutation");
    }
    {Fixture f;auto owner=std::make_unique<s::FrameReadOwner>(f.projection);auto residual=std::make_unique<r::Ref>();f.owners[0].owner=owner.get();residual->erase=[&]{residual.reset();};start_capture=[&](auto& h,const auto&,auto,auto,auto&,auto){h.control_a8=residual.get();owner.reset();};f.start();check(!owner&&!residual,"callee can destroy owner and final residual deletes itself before return");}
    for(unsigned mode=0;mode<4;++mode){Fixture f;if(mode==0)f.context.owners={};if(mode==1)f.owners[0].owner=nullptr;if(mode==2)f.owners[1]=f.owners[0];if(mode==3)f.context.read=nullptr;rejects<s::UnsupportedInput>([&]{f.start();});check(f.owner.strong_08==(mode==3?2U:1U),"projection failure keeps exact acquisition prefix");}
}
}
int main(){try{success();failures();live_owner();logging();acquisition();std::cout<<"PASS: "<<checks<<" socket frame read core checks; addressed025480/0280d0 normal flow, independent payload/error/ownership oracles and controlled provider/lifetime mutations\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
