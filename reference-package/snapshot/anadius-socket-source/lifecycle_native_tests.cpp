#include "lifecycle_native.hpp"
#include <functional>
#include <iostream>
#include <vector>

namespace sock=anadius::socket_recovered;
namespace {
unsigned checks=0;
void require(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
struct Fixture {
    CRITICAL_SECTION section{};int depth=0;unsigned enters=0,leaves=0;
    std::vector<std::string> trace;std::function<void(std::string_view)> hook;
};
Fixture* active=nullptr;
void WINAPI enter(CRITICAL_SECTION* section){
    auto& fixture=*active;fixture.trace.push_back("enter");++fixture.enters;
    require(section==&fixture.section,"SDK EnterCriticalSection receives exact borrowed typed object");
    require(fixture.depth==0,"selected fixture begins enter outside lock");++fixture.depth;
    if(fixture.hook)fixture.hook("enter");
}
void WINAPI leave(CRITICAL_SECTION* section){
    auto& fixture=*active;fixture.trace.push_back("leave");++fixture.leaves;
    require(section==&fixture.section,"SDK LeaveCriticalSection receives identical borrowed typed object");
    require(fixture.depth==1,"selected fixture leaves after enter");
    if(fixture.hook)fixture.hook("leave");--fixture.depth;
}
sock::GateImports imports(){sock::GateImports value;value.enter=enter;value.leave=leave;return value;}
struct Effects final:sock::LifecycleEffects {
    Fixture& fixture;sock::SocketService& service;unsigned closes=0,cleanups=0;
    explicit Effects(Fixture& fixture,sock::SocketService& service):fixture(fixture),service(service){}
    sock::SocketService& resolve_service_0239c0(sock::ContextIdentity context)override{fixture.trace.push_back("resolve");require(context.value==123,"composed resolver receives supplied context identity");return service;}
    void close_and_cancel_026fb0(sock::SocketService& selected,sock::SocketObject&)override{fixture.trace.push_back("close_provider");++closes;require(&selected==&service && fixture.depth==0,"close boundary uses service before native gate entry");}
    void executor_cleanup(sock::SocketObject& object,std::uintptr_t table)override{fixture.trace.push_back("executor");++cleanups;require(fixture.depth==0 && !object.next && !object.previous,"executor cleanup follows native gate leave and unlink");require(table==0x180084928ULL,"composed executor uses logical metadata without executing token");}
};
void forwarding(){
    for(unsigned pairs:{0U,1U,3U,17U}){
        Fixture fixture;active=&fixture;
        {sock::NativeSocketGate defaults(fixture.section);require(!fixture.enters && !fixture.leaves && fixture.trace.empty(),"default SDK import construction invokes no operating-system call");}
        sock::NativeSocketGate gate(fixture.section,imports());
        for(unsigned i=0;i<pairs;++i){gate.enter();require(fixture.depth==1,"injected typed enter updates controlled fixture");gate.leave();require(fixture.depth==0,"injected typed leave balances controlled fixture");}
        require(fixture.enters==pairs && fixture.leaves==pairs,"wrapper forwards each requested SDK operation exactly once");
        require(fixture.trace.size()==2*pairs,"no initialization/deletion/error/API calls inserted by borrowed gate");
    }
    active=nullptr;
}
void composition(){
    Fixture fixture;active=&fixture;sock::NativeSocketGate gate(fixture.section,imports());sock::SocketService service(gate);Effects effects(fixture,service);
    sock::AcceptorState state;sock::SocketObject object(state);
    fixture.hook=[&](std::string_view phase){if(phase=="enter"){require(state.socket==sock::invalid_socket && state.socket_state==0,"real constructor initializes socket before native gate call");require(object.executor.context.value==123 && object.executor.self_reference_80,"real constructor initializes executor before native gate call");require(service.head==nullptr,"first list insertion has not happened at gate entry");}else{require(service.head==&object && !object.previous && !object.next,"real constructor completes insertion before native gate leave");}};
    sock::initialize_0211b0(object,{123},effects);
    require(fixture.trace==std::vector<std::string>{"resolve","enter","leave"},"actual initializer uses only required resolver and injected SDK gate pair");
    fixture.trace.clear();fixture.hook=[&](std::string_view phase){if(phase=="enter")require(service.head==&object && effects.closes==1,"real destructor closes before gate while list still registered");else require(service.head==nullptr && !object.next && !object.previous,"real destructor clears links before SDK gate leave");};
    sock::destroy_024db0(object,effects);
    require(fixture.trace==std::vector<std::string>{"close_provider","enter","leave","executor"},"actual destructor composes required close boundary and injected SDK pair before executor");
    require(effects.closes==1 && effects.cleanups==1 && fixture.enters==2 && fixture.leaves==2 && fixture.depth==0,"composed typed gate balances both constructor/destructor operations");
    active=nullptr;
}
void live_head(){
    Fixture fixture;active=&fixture;sock::NativeSocketGate gate(fixture.section,imports());sock::SocketService service(gate);Effects effects(fixture,service);
    sock::AcceptorState first_state,second_state;sock::SocketObject first(first_state),second(second_state);
    fixture.hook=[&](std::string_view phase){if(phase=="enter")service.head=&first;};
    sock::initialize_0211b0(second,{123},effects);
    require(service.head==&second && second.next==&first && first.previous==&second,"constructor reads live head after injected native gate entry");
    fixture.hook={};active=nullptr;
}
}
int main(){try{forwarding();composition();live_head();std::cout<<"PASS: "<<checks<<" socket lifecycle native checks; injected SDK gate imports only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
