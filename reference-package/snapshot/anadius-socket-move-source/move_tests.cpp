#include "move.hpp"
#include <iostream>
#include <exception>
#include <functional>
#include <vector>
namespace {
namespace s=anadius::socket_recovered;
unsigned checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
template<class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const s::UnsupportedInput&){caught=true;}require(caught,text);}
struct Gate:s::SocketGate {
    unsigned enters=0,leaves=0,depth=0;std::function<void(bool)> observe;
    void enter()override{++enters;++depth;if(observe)observe(true);}
    void leave()override{if(observe)observe(false);--depth;++leaves;}
};
struct Ref:s::Control {
    unsigned destroyed=0,deleted=0;std::function<void()> dispose;
    Ref(){strong=1;weak=1;}
    void destroy_resource()override{++destroyed;if(dispose)dispose();}
    void delete_control()override{++deleted;}
};
struct Lifecycle:s::LifecycleEffects {
    s::SocketService& service;std::vector<s::Socket> closes;unsigned cleanups=0;
    explicit Lifecycle(s::SocketService& value):service(value){}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService&,s::SocketObject& o)override{closes.push_back(o.state.socket);o.state.socket=s::invalid_socket;o.state.socket_state=0;}
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{++cleanups;}
};
struct Executor {
    unsigned calls=0;std::vector<std::uintptr_t> tables;bool throwing=false;
    std::exception_ptr failure=std::make_exception_ptr(19);
    static void call(void* context,s::ExecutorFields& dst,s::ExecutorFields& src,std::uintptr_t table){
        auto& e=*static_cast<Executor*>(context);++e.calls;e.tables.push_back(table);
        dst.context=src.context;dst.zero_74=src.zero_74;dst.self_reference_80=src.self_reference_80;
        src.table_90+=1; // Proves the caller's post-slot live reload.
        if(e.throwing)std::rethrow_exception(e.failure);
    }
    s::ExecutorMove effect(){return {this,call};}
};
struct Fixture {
    Gate gate;s::SocketService service{gate};Lifecycle lifecycle{service};
    s::AcceptorState p,t,o;s::SocketObject pending{p},temporary{t},owned{o};Ref reference;Executor executor;
    Fixture(){s::initialize_0211b0(pending,{0x1234},lifecycle);p.socket=0xfedcba9876543210ULL;p.socket_state=0x9a;p.protocol_40=23;p.byte_44=0xe1;p.slot_48=0x1122334455667788ULL;p.slot_50=0x8877665544332211ULL;p.slot_58=0xabcdef0987654321ULL;p.slot_60=0xdeadbeef;pending.cancellation={0xcafe,&reference};}
};
void selected_executor(){
    s::ExecutorFields source,destination;source.context={0x1234};source.zero_74=0xabcdef12;
    source.table_78=s::RecoveredSocketExecutor::table_identity;source.table_88=21;source.table_90=22;source.self_reference_80=false;
    destination.context={0x9999};destination.zero_74=7;destination.table_78=source.table_78;destination.table_88=31;destination.table_90=32;
    const auto selected=s::RecoveredSocketExecutor::move();selected.move(selected.context,destination,source,source.table_78);
    require(destination.context.value==0x1234&&destination.zero_74==0xabcdef12&&destination.self_reference_80,"Actual0228c0 copies payload context/dword and installs destination self reference");
    require(destination.table_88==31&&destination.table_90==32&&source.context.value==0x1234&&source.zero_74==0xabcdef12&&!source.self_reference_80&&source.table_90==22,"Selected executor move changes neither source nor unrelated destination tables");
    rejects([&]{selected.move(selected.context,destination,source,9);},"Unobserved executor table cannot select0228c0");
    s::AcceptorState state;s::SocketObject object(state);object.executor=source;s::RecoveredSocketExecutor cleanup;
    cleanup.executor_cleanup(object,source.table_78);
    require(object.executor.context.value==source.context.value&&object.executor.table_90==22&&!object.executor.self_reference_80,"Observed017a30 cleanup RET0 changes no fields");
    rejects([&]{cleanup.executor_cleanup(object,9);},"Cleanup is not a universal no-op for foreign tables");
    object.executor.table_78=9;rejects([&]{cleanup.executor_cleanup(object,source.table_78);},"Mismatching live table rejects before cleanup");
    Fixture f;f.pending.executor.self_reference_80=false;
    s::move_socket_023d20(f.temporary,f.pending,selected);
    require(f.temporary.executor.self_reference_80&&!f.pending.executor.self_reference_80&&f.temporary.executor.context.value==0x1234,"Actual socket constructor invokes selected0228c0 rather than copying source self-reference state");
    s::destroy_024db0(f.temporary,f.lifecycle);s::destroy_024db0(f.pending,f.lifecycle);
}
void two_moves_and_cleanup(){
    Fixture f;const auto original=f.p;const auto initial_gate=f.gate.enters;
    bool prefix=true;f.gate.observe=[&](bool){prefix=prefix&&f.gate.depth==1&&f.reference.strong==1;};
    require(&s::move_socket_023d20(f.temporary,f.pending,f.executor.effect())==&f.temporary,"constructor move returns exact destination reference");
    require(f.t.socket==original.socket&&f.p.socket==s::invalid_socket&&f.t.socket_state==original.socket_state&&f.p.socket_state==0,"full64-bit socket/state move invalidates source before later phases");
    require(f.temporary.cancellation.control==&f.reference&&f.temporary.cancellation.resource==0xcafe&&!f.pending.cancellation.control&&!f.pending.cancellation.resource&&f.reference.strong==1,"cancellation pair transfers one strong owner without destruction");
    require(f.service.head==&f.temporary&&f.temporary.next==&f.pending&&f.pending.previous==&f.temporary&&f.pending.next==nullptr,"first move inserts fresh head and leaves invalid pending node linked");
    require(f.t.protocol_40==23&&f.t.byte_44==0xe1&&f.t.slot_48==original.slot_48&&f.t.slot_50==original.slot_50&&f.t.slot_58==original.slot_58&&f.t.slot_60==original.slot_60,"protocol and all resource fields preserve exact byte/32/64-bit values");
    require(f.p.protocol_40==2&&f.p.byte_44==0&&f.p.slot_48==2&&f.p.slot_50==0&&f.p.slot_58==0&&f.p.slot_60==0,"source protocol/resource fields reset only after head insertion gate");
    require(f.temporary.executor.table_78==0x180084928ULL&&f.temporary.executor.table_88==f.pending.executor.table_88&&f.temporary.executor.table_90==f.pending.executor.table_90&&f.temporary.executor.context.value==0x1234,"exact executor slot identity is forwarded and table90 is reloaded after lower mutation");
    s::move_socket_023d20(f.owned,f.temporary,f.executor.effect());
    require(f.service.head==&f.owned&&f.owned.next==&f.temporary&&f.temporary.previous==&f.owned&&f.temporary.next==&f.pending&&f.pending.previous==&f.temporary,"second move produces owned-temporary-pending chain rather than replacing source node");
    require(f.o.socket==original.socket&&f.t.socket==s::invalid_socket&&f.reference.strong==1&&f.reference.destroyed==0&&f.reference.deleted==0,"two moves preserve exactly one cancellation owner and one live socket");
    require(f.executor.calls==2&&f.executor.tables==std::vector<std::uintptr_t>{0x180084928ULL,0x180084928ULL}&&prefix&&f.gate.enters==initial_gate+2&&f.gate.leaves==f.gate.enters,"each move invokes selected slot once and gates only list insertion");
    f.gate.observe={};s::destroy_024db0(f.temporary,f.lifecycle);
    require(f.service.head==&f.owned&&f.owned.next==&f.pending&&f.pending.previous==&f.owned&&!f.temporary.next&&!f.temporary.previous,"actual024db0 removes only the moved temporary and repairs adjacent links");
    require(f.lifecycle.closes==std::vector<s::Socket>{s::invalid_socket}&&f.reference.destroyed==0,"temporary destruction never closes transferred socket or releases its owner");
    s::destroy_024db0(f.pending,f.lifecycle);s::destroy_024db0(f.owned,f.lifecycle);
    require(f.service.head==nullptr&&f.reference.destroyed==1&&f.reference.deleted==1&&f.reference.strong==0&&f.reference.weak==0,"final cleanup empties service and releases owned resource/control exactly once");
    require(f.lifecycle.closes==std::vector<s::Socket>{s::invalid_socket,s::invalid_socket,original.socket}&&f.lifecycle.cleanups==3,"three registered nodes each receive one matching existing destructor");
}
void invalid_inputs_and_slot_throw(){
    Fixture f;const auto gate=f.gate.enters;
    rejects([&]{s::move_socket_023d20(f.pending,f.pending,f.executor.effect());},"same object rejected before effects");
    s::SocketObject aliased{f.p};rejects([&]{s::move_socket_023d20(aliased,f.pending,f.executor.effect());},"distinct objects sharing one state rejected before source invalidation");
    rejects([&]{s::transfer_socket_028950(f.service,aliased,f.pending);},"low-level transfer also rejects shared state alias");
    rejects([&]{s::move_socket_023d20(f.temporary,f.pending,{});},"missing selected executor effect rejected");
    f.temporary.service=&f.service;rejects([&]{s::move_socket_023d20(f.temporary,f.pending,f.executor.effect());},"existing destination ownership rejected as non-constructor storage");f.temporary.service=nullptr;
    require(f.executor.calls==0&&f.gate.enters==gate&&f.p.socket==0xfedcba9876543210ULL&&f.reference.strong==1,"invalid metadata inputs leave native state and lower effects untouched");
    f.executor.throwing=true;bool same=false;try{s::move_socket_023d20(f.temporary,f.pending,f.executor.effect());}catch(...){same=std::current_exception()==f.executor.failure;}
    require(same&&f.executor.calls==1&&f.gate.enters==gate&&f.service.head==&f.pending,"executor throw preserves exception identity and stops before control/list transfer");
    require(f.temporary.service==&f.service&&f.temporary.executor.table_78==f.pending.executor.table_78&&f.temporary.executor.table_90==0&&f.p.socket==0xfedcba9876543210ULL&&f.temporary.cancellation.control==nullptr,"throw retains constructor prefix but does not invent post-callback table90/socket writes");
    rejects([&]{s::move_socket_023d20(f.temporary,f.pending,f.executor.effect());},"partly constructed destination is not blindly retried");
    s::destroy_024db0(f.pending,f.lifecycle);require(f.reference.destroyed==1,"separate known pending owner remains explicitly destructible after pre-transfer throw");
}
void release_reloads_and_gate_order(){
    Fixture f;Ref old,replacement;Gate other_gate;s::SocketService other{other_gate};
    f.owned.service=&f.service;f.owned.state.service=&f.service;f.owned.cancellation={0x1111,&old};
    bool prefix=false,gate_prefix=false;
    old.dispose=[&]{
        prefix=f.owned.cancellation.control==&f.reference&&f.reference.strong==2&&f.p.socket==s::invalid_socket&&f.p.socket_state==0&&f.gate.depth==0;
        old.weak=3; // Native must reload this AFTER destroy_resource.
        f.reference.strong.fetch_sub(1); // legitimate replacement releases source ownership
        f.pending.cancellation={0xbeef,&replacement};f.owned.service=&other;
    };
    f.gate.observe=[&](bool entering){gate_prefix=gate_prefix||(entering&&f.o.protocol_40==2&&f.p.protocol_40==23&&f.pending.cancellation.control==nullptr&&replacement.destroyed==1);};
    s::transfer_socket_028950(f.service,f.owned,f.pending);
    require(prefix&&old.destroyed==1&&old.deleted==0&&old.weak==2,"old destination disposal sees incremented source and native reloads weak after callback");
    require(replacement.destroyed==1&&replacement.deleted==1&&!f.pending.cancellation.control&&f.owned.cancellation.control==&f.reference&&f.reference.strong==1,"source cancellation is reloaded and cleared after old destructor mutation");
    require(gate_prefix&&f.service.head==&f.owned&&other.head==nullptr&&other_gate.enters==0,"entry service remains authoritative even if control callback changes object.service");
    require(f.o.protocol_40==23&&f.p.protocol_40==2&&f.o.slot_60==0xdeadbeef&&f.gate.depth==0,"protocol transfer occurs after balanced list gate and preserves exact32-bit tail");
    f.gate.observe={};f.owned.service=&f.service;s::destroy_024db0(f.pending,f.lifecycle);s::destroy_024db0(f.owned,f.lifecycle);
    require(f.reference.destroyed==1&&f.reference.deleted==1,"remaining original owner releases exactly once after controlled reentrant replacement");
}
}
int main(){try{selected_executor();two_moves_and_cleanup();invalid_inputs_and_slot_throw();release_reloads_and_gate_order();std::cout<<"PASS: "<<checks<<" socket constructor/implementation move core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
