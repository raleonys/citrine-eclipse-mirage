#include "preloader_l/net/transport_lifecycle.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <vector>

namespace sock=preloader_l::socket_recovered;
namespace srv=preloader_l::server_recovered;
namespace {
unsigned checks=0;
void require(bool yes,const char* why){++checks;if(!yes)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected socket lifetime dependency");}
struct Owner final:sock::Control {
    unsigned destroys=0,deletes=0;std::function<void()> on_destroy,on_delete;
    explicit Owner(int strong=1,int weak=1){this->strong=strong;this->weak=weak;}
    void destroy_resource()override{++destroys;if(on_destroy)on_destroy();}
    void delete_control()override{++deletes;if(on_delete)on_delete();}
};
struct Factory final:sock::ControlFactory {
    sock::Control* returned=nullptr;unsigned allocations=0;std::function<void()> hook;
    sock::Control* allocate_04b0bc(std::uint32_t request)override{
        ++allocations;require(request==24,"original shared owner allocation request is24");
        if(hook)hook();return returned;
    }
};
struct Gate final:sock::SocketGate {
    std::vector<std::string>& trace;std::function<void(std::string_view)> hook;int depth=0;
    explicit Gate(std::vector<std::string>& trace):trace(trace){}
    void enter()override{trace.push_back("enter");require(depth==0,"fixture gate not recursively entered");++depth;if(hook)hook("enter");}
    void leave()override{trace.push_back("leave");require(depth==1,"fixture gate leave follows enter");if(hook)hook("leave");--depth;}
};
struct Effects final:sock::LifecycleEffects {
    std::vector<std::string>& trace;sock::SocketService& service;
    std::function<void()> resolve_hook;std::function<void(sock::SocketService&,sock::SocketObject&)> close_hook;
    std::function<void(sock::SocketObject&,std::uintptr_t)> cleanup_hook;
    sock::ContextIdentity resolved{};sock::SocketService* closed_service=nullptr;
    explicit Effects(std::vector<std::string>& trace,sock::SocketService& service):trace(trace),service(service){}
    sock::SocketService& resolve_service_0239c0(sock::ContextIdentity identity)override{trace.push_back("resolve");resolved=identity;if(resolve_hook)resolve_hook();return service;}
    void close_and_cancel_026fb0(sock::SocketService& selected,sock::SocketObject& object)override{trace.push_back("close_provider");closed_service=&selected;if(close_hook)close_hook(selected,object);}
    void executor_cleanup(sock::SocketObject& object,std::uintptr_t identity)override{trace.push_back("executor");if(cleanup_hook)cleanup_hook(object,identity);}
};
void check_initialized(const sock::SocketObject& object,const sock::SocketService& service,std::uintptr_t context){
    require(object.service==&service && object.state.service==&service,"logical object and acceptor share resolved service");
    require(object.state.socket==sock::invalid_socket && object.state.socket_state==0,"constructor starts invalid socket/state0");
    require(object.state.protocol_40==2 && object.state.byte_44==0,"constructor protocol family2 and byte44 zero");
    require(object.state.slot_48==2 && object.state.slot_50==0 && object.state.slot_58==0 && object.state.slot_60==0,"constructor fixed protocol seed and zero trailing fields");
    require(object.cancellation.resource==0 && object.cancellation.control==nullptr,"constructor clears cancellation pair");
    require(object.executor.context.value==context && object.executor.zero_74==0,"executor preserves context identity and clears marker");
    require(object.executor.table_78==0x180084928ULL && object.executor.table_88==0x180084908ULL && object.executor.table_90==0x180084860ULL,"executor metadata uses observed opaque table identities");
    require(object.executor.self_reference_80,"executor has logical self-reference");
}
void constructor(){
    std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);Effects effects(trace,service);
    sock::AcceptorState a,b,c;sock::SocketObject first(a),second(b),third(c);Owner ignored(9,8);
    first.cancellation={123,&ignored};first.executor.zero_74=77;first.executor.table_78=99;
    effects.resolve_hook=[&]{require(first.cancellation.control==&ignored,"service resolver precedes constructor pair reset");};
    gate.hook=[&](std::string_view phase){if(phase=="enter")check_initialized(first,service,0x1122334455667788ULL);};
    auto& result=sock::initialize_0211b0(first,{0x1122334455667788ULL},effects);
    require(&result==&first && service.head==&first && !first.next && !first.previous,"first insertion returns object and establishes sole head");
    require(trace==std::vector<std::string>{"resolve","enter","leave"},"constructor calls resolver before selected gate pair");
    require(ignored.strong==9 && ignored.weak==8 && !ignored.destroys && !ignored.deletes,"initializer overwrites owner before reload; no preexisting owner release");
    effects.resolve_hook={};gate.hook={};
    sock::initialize_0211b0(second,{2},effects);sock::initialize_0211b0(third,{3},effects);
    require(service.head==&third && third.next==&second && second.next==&first && first.next==nullptr,"insertions prepend in reverse constructor order");
    require(third.previous==nullptr && second.previous==&third && first.previous==&second,"constructor patches old head backward link");
    check_initialized(second,service,2);check_initialized(third,service,3);
    require(effects.resolved.value==3 && gate.depth==0,"resolver sees live context and gate leaves balanced");
}
void replacement(){
    for(const int strong:{2,1,0,-1,std::numeric_limits<int>::min()})for(const int weak:{3,1,0}){
        Owner old(strong,weak),fresh(77,88);old.resource=111;fresh.resource=999;
        sock::ResourcePair pair{111,&old};Factory factory;factory.returned=&fresh;
        factory.hook=[&]{require(pair.resource==111 && pair.control==&old,"allocator runs before pair writes");};
        old.on_destroy=[&]{require(pair.resource==222 && pair.control==&fresh,"new pair published before old resource callback");require(fresh.strong==1 && fresh.weak==1 && fresh.resource==222,"fresh control initialized before callback publication");require(old.strong==0 && old.weak==weak,"destroy callback observes decremented strong and still-live original weak count");};
        old.on_delete=[&]{require(old.strong==0 && old.weak==0 && old.destroys==1,"delete callback follows resource destruction and final weak decrement");};
        sock::replace_owner_0237f0(pair,222,factory);
        require(factory.allocations==1 && pair.resource==222 && pair.control==&fresh,"replacement publishes requested resource and new owner");
        const int decremented=strong==std::numeric_limits<int>::min()?std::numeric_limits<int>::max():strong-1;
        require(old.strong==decremented && old.destroys==static_cast<unsigned>(strong==1),"strong atomic decrement precedes sole last-owner destruction");
        require(old.weak==(strong==1?weak-1:weak) && old.deletes==static_cast<unsigned>(strong==1 && weak==1),"weak decrement only after strong old1 and delete only weak old1");
        require(fresh.strong==1 && fresh.weak==1 && fresh.resource==222,"fresh owner has independent1/1 counts");
    }
    {Owner old;sock::ResourcePair pair{9,&old};Factory factory;sock::replace_owner_0237f0(pair,8,factory);require(pair.resource==8 && !pair.control && old.destroys==1 && old.deletes==1,"nullable allocation still replaces resource and releases old owner");}
    {Owner fresh;sock::ResourcePair pair;Factory factory;factory.returned=&fresh;sock::replace_owner_0237f0(pair,0x123456789abcdef0ULL,factory);require(pair.resource==0x123456789abcdef0ULL && fresh.resource==pair.resource && pair.control==&fresh,"null old owner skips release and preserves full resource bits");}
    {Owner original(7,8),reentered,new_owner;sock::ResourcePair pair{10,&original};Factory factory;factory.returned=&new_owner;factory.hook=[&]{pair={20,&reentered};};sock::replace_owner_0237f0(pair,30,factory);require(original.strong==7 && !original.destroys && reentered.destroys==1 && reentered.deletes==1,"allocator reentry changes which live old owner gets released");require(pair.control==&new_owner && pair.resource==30,"allocator reentry does not replace requested final resource");}
    {Owner old(1,1),fresh,reentrant(5,7);sock::ResourcePair pair{1,&old};Factory factory;factory.returned=&fresh;old.on_destroy=[&]{pair.control=&reentrant;old.weak=4;old.strong=9;};sock::replace_owner_0237f0(pair,2,factory);require(old.destroys==1 && old.weak==3 && !old.deletes && old.strong==9,"weak read occurs after destroy callback; strong resurrection does not skip weak decrement");require(pair.control==&reentrant && reentrant.strong==5,"release caches old control while callback changes pair owner");}
    {Owner old;sock::ResourcePair pair{44,&old};Factory factory;factory.hook=[]{throw std::runtime_error("allocation effect");};bool caught=false;try{sock::replace_owner_0237f0(pair,55,factory);}catch(const std::runtime_error&){caught=true;}require(caught && pair.resource==44 && pair.control==&old && old.strong==1,"throwing allocator effect prevents subsequent pair stores/releases");}
}
void destruction(){
    for(unsigned removed:{0U,1U,2U}){
        std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);Effects effects(trace,service);
        sock::AcceptorState states[3];sock::SocketObject a(states[0]),b(states[1]),c(states[2]);
        sock::SocketObject* objects[]={&a,&b,&c};for(auto* object:objects)sock::initialize_0211b0(*object,{3},effects);
        auto& victim=*objects[removed];Owner owner;victim.cancellation={42,&owner};trace.clear();
        effects.close_hook=[&](sock::SocketService& selected,sock::SocketObject& object){require(&selected==&service && &object==&victim,"destructor passes cached service and current object to required close provider");require(gate.depth==0,"close provider runs before list gate");};
        effects.cleanup_hook=[&](sock::SocketObject& object,std::uintptr_t table){require(gate.depth==0 && !object.next && !object.previous,"executor callback runs after unlink and leave");require(table==0x180084928ULL,"executor cleanup receives current table identity");require(owner.strong==1,"owner read/release follows executor callback");};
        sock::destroy_024db0(victim,effects);
        require(trace==std::vector<std::string>{"close_provider","enter","leave","executor"},"destructor close/unlink/executor call order");
        require(owner.destroys==1 && owner.deletes==1 && victim.cancellation.control==&owner && victim.cancellation.resource==42,"destructor final release does not clear pair slot or resource");
        std::vector<sock::SocketObject*> survivors;for(int i=2;i>=0;--i)if(static_cast<unsigned>(i)!=removed)survivors.push_back(objects[i]);
        require(service.head==survivors[0] && survivors[0]->next==survivors[1] && survivors[1]->next==nullptr,"removal preserves forward head/middle/tail list");
        require(!survivors[0]->previous && survivors[1]->previous==survivors[0],"removal patches backward links for every position");
    }
    {std::vector<std::string> trace;Gate first_gate(trace),other_gate(trace);sock::SocketService first(first_gate),other(other_gate);Effects effects(trace,first);sock::AcceptorState state;sock::SocketObject object(state);sock::initialize_0211b0(object,{7},effects);Owner original(4,5),late;object.cancellation={4,&original};trace.clear();effects.close_hook=[&](sock::SocketService&,sock::SocketObject& target){target.service=&other;target.state.service=&other;target.executor.table_78=123;};effects.cleanup_hook=[&](sock::SocketObject& target,std::uintptr_t table){require(table==123,"executor table read after close provider mutation");target.cancellation.control=&late;};sock::destroy_024db0(object,effects);require(effects.closed_service==&first && first.head==nullptr && other.head==nullptr && first_gate.depth==0 && other_gate.depth==0,"cached service remains unlink target after object service changes");require(original.strong==4 && !original.destroys && late.destroys==1 && late.deletes==1 && object.cancellation.control==&late,"destructor reads owner only after executor callback mutation");}
    {std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);Effects effects(trace,service);sock::AcceptorState state;sock::SocketObject object(state);sock::initialize_0211b0(object,{1},effects);Owner owner;object.cancellation={55,&owner};effects.close_hook=[&](sock::SocketService&,sock::SocketObject& target){Factory null; sock::replace_owner_0237f0(target.cancellation,0,null);};sock::destroy_024db0(object,effects);require(owner.destroys==1 && owner.deletes==1 && !object.cancellation.control,"close provider clearing pair prevents a second final release");}
    {std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);Effects effects(trace,service);sock::AcceptorState state;sock::SocketObject object(state);sock::initialize_0211b0(object,{1},effects);Owner owner;object.cancellation.control=&owner;owner.on_destroy=[&]{owner.weak=3;};sock::destroy_024db0(object,effects);require(owner.destroys==1 && owner.weak==2 && !owner.deletes,"destructor weak decrement observes callback's live weak count");}
    {std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);Effects effects(trace,service);sock::AcceptorState state;sock::SocketObject object(state);sock::initialize_0211b0(object,{1},effects);Owner retained(3,4);object.cancellation.control=&retained;effects.cleanup_hook=[](sock::SocketObject& target,std::uintptr_t){target.cancellation.control=nullptr;};sock::destroy_024db0(object,effects);require(retained.strong==3 && retained.weak==4 && !retained.destroys,"executor clearing owner slot suppresses late final release");}
    {std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);Effects effects(trace,service);sock::AcceptorState state;sock::SocketObject object(state);bool caught=false;try{sock::destroy_024db0(object,effects);}catch(const sock::UnsupportedInput&){caught=true;}require(caught && trace.empty(),"null service projection is explicit unsupported domain before effects");}
}
struct Api final:sock::Api,sock::AcceptorApi {
    std::vector<std::string>& trace;explicit Api(std::vector<std::string>& trace):trace(trace){}
    void set_last_error(std::int32_t)override{unused();}sock::Category system_category_028620()override{unused();}std::int32_t get_last_error()override{unused();}
    sock::Conversion convert_acp(const char*,std::int32_t)override{unused();}sock::AddressCall string_to_address(sock::Conversion&,std::int32_t,std::int32_t)override{unused();}
    sock::SendCall wsa_send(sock::Socket,std::span<sock::Buffer>,std::uint32_t)override{unused();}std::int32_t select_write(sock::Socket,std::int32_t)override{unused();}
    sock::Socket create_socket(std::int32_t family)override{require(family==2,"integration selects IPv4");trace.push_back("create");return 13;}
    std::uintptr_t attach_completion_port(sock::Socket socket,std::uintptr_t port)override{require(socket==13 && port==99,"integration uses recovered service completion port");trace.push_back("attach");return 23;}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(sock::Socket,int,int,int)override{unused();}
    std::int32_t bind_socket(sock::Socket socket,std::span<const std::byte>,std::int32_t size)override{require(socket==13 && size==16,"integration bind follows constructor and transfer");trace.push_back("bind");return 0;}
    std::int32_t listen_socket(sock::Socket socket,std::int32_t backlog)override{require(socket==13 && backlog==std::numeric_limits<int>::max(),"integration listen uses recovered socket/backlog");trace.push_back("listen");return 0;}
    std::int32_t close_socket(sock::Socket socket)override{require(socket==13,"selected close provider invokes recovered close helper on live socket");trace.push_back("close_api");return 0;}
    std::int32_t set_nonblocking(sock::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Runtime final:sock::LifecycleRuntime {
    using LifecycleRuntime::LifecycleRuntime;
    sock::Category miscellaneous_category_0285b0()override{unused();}
    [[noreturn]]void raise_028240(sock::Error,std::string_view)override{unused();}
};
void composition(){
    std::vector<std::string> trace;Gate gate(trace);sock::SocketService service(gate);service.completion_port=99;Effects effects(trace,service);
    sock::AcceptorState state;sock::SocketObject object(state);Owner allocated;Factory factory;factory.returned=&allocated;
    factory.hook=[&]{require(state.socket==13 && state.socket_state==0x10,"acceptor transfer precedes actual recovered owner replacement");trace.push_back("allocate");};
    Runtime runtime(object,{0x1234},effects,factory);Api api(trace);alignas(8) std::array<std::byte,128> address{};address[0]=std::byte{2};
    sock::acceptor_020e20(state,address,false,api,api,runtime);
    require(trace==std::vector<std::string>{"resolve","enter","leave","create","attach","allocate","bind","listen"},"actual acceptor composes actual initializer and owner body before bind/listen");
    require(service.head==&object && object.cancellation.control==&allocated && allocated.strong==1 && allocated.weak==1 && allocated.resource==0,"acceptor owns new cancellation control while registered in service");
    effects.close_hook=[&](sock::SocketService&,sock::SocketObject& target){sock::close_025310(target.state.socket,api,api);target.state.socket=sock::invalid_socket;Factory null;sock::replace_owner_0237f0(target.cancellation,0,null);};
    trace.clear();runtime.destroy_socket_024db0();
    require(trace==std::vector<std::string>{"close_provider","close_api","enter","leave","executor"},"selected close boundary composes recovered close then destructor unlink and executor cleanup");
    require(!service.head && allocated.destroys==1 && allocated.deletes==1 && !object.cancellation.control,"integration final owner released once; list empty");
    sock::AcceptorState wrong;bool init_caught=false,replace_caught=false;
    try{runtime.initialize_0211b0(wrong);}catch(const sock::UnsupportedInput&){init_caught=true;}
    try{runtime.reset_cancellation_0237f0(wrong);}catch(const sock::UnsupportedInput&){replace_caught=true;}
    require(init_caught && replace_caught,"runtime refuses unrelated acceptor state projection");
}
void owned_factory(){
    sock::OwnedNoopControlFactory factory;int resource=55;sock::ResourcePair pair;
    sock::replace_owner_0237f0(pair,reinterpret_cast<std::uintptr_t>(&resource),factory);
    require(pair.control && pair.control->strong==1 && pair.control->weak==1 && pair.control->resource==reinterpret_cast<std::uintptr_t>(&resource),"selected host factory creates own typed live noop control");
    Factory null;sock::replace_owner_0237f0(pair,0,null);
    require(!pair.control && pair.resource==0 && resource==55,"selected noop owner releases own control without freeing resource");
}
}
int main(){try{constructor();replacement();destruction();composition();owned_factory();std::cout<<"PASS: "<<checks<<" socket lifecycle checks; controlled service/list/owner providers only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
