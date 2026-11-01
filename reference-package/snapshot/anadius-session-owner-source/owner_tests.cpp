#include "owner.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <exception>
#include <vector>
namespace {
namespace s=anadius::socket_recovered;unsigned checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
struct Fault{};
template<class E,class F>void rejects(F&& f,const char* text){bool ok=false;try{f();}catch(const E&){ok=true;}require(ok,text);}
void weak_release(s::IoReference* p){if(p&&p->weak_0c.fetch_sub(1)==1)p->delete_slot_08();}
void strong_release(s::IoReference* p){if(p&&p->strong_08.fetch_sub(1)==1){p->dispose_slot_00();weak_release(p);}}
struct Ref:s::IoReference {
    unsigned disposed=0,deleted=0;std::function<void()> on_delete;
    void dispose_slot_00()override{++disposed;}
    void delete_slot_08()override{++deleted;if(on_delete)on_delete();}
};
struct Gate:s::SocketGate {unsigned depth=0;void enter()override{++depth;}void leave()override{--depth;}};
struct Life:s::LifecycleEffects {
    s::SocketService& service;std::vector<s::Socket> closes;std::function<void(s::SocketObject&)> before_close;
    explicit Life(s::SocketService& v):service(v){}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService&,s::SocketObject& o)override{if(before_close)before_close(o);closes.push_back(o.state.socket);o.state.socket=s::invalid_socket;}
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}
};
struct Block:s::SessionControl {
    Life& life;s::ConstructionScratch& scratch;unsigned disposed=0,deleted=0,proxies=0;
    Block(Life& l,s::ConstructionScratch& c):life(l),scratch(c){}
    void dispose_slot_00()override{
        ++disposed;
        if(scratch.owner_socket_live){s::destroy_024db0(object.socket,life);scratch.owner_socket_live=false;}
        if(object.output.proxy_c8){++proxies;object.output.proxy_c8=nullptr;}
        auto* old=object.weak_self.control_08;object.weak_self.object_00=0;object.weak_self.control_08=nullptr;weak_release(old);
    }
    void delete_slot_08()override{++deleted;} // controlled pool retains inspectable sidecar
};
struct Alloc:s::SessionAllocator {
    Block& block;s::DequeProxy proxy;unsigned controls=0,proxies=0;bool null_control=false,null_proxy=false,throw_proxy=false;
    std::function<void()> proxy_hook;
    explicit Alloc(Block& b):block(b){}
    s::SessionControl* allocate_control_04b0bc(std::uint64_t size)override{++controls;if(size!=0x1c8)throw Fault{};return null_control?nullptr:&block;}
    s::DequeProxy* allocate_proxy_04b0bc(std::uint64_t size)override{++proxies;if(size!=16)throw Fault{};if(proxy_hook)proxy_hook();if(throw_proxy)throw Fault{};return null_proxy?nullptr:&proxy;}
};
struct Fixture {
    Gate gate;s::SocketService service{gate};Life life{service};s::ConstructionScratch scratch;
    Block block{life,scratch};Alloc alloc{block};s::AcceptorState pending_state;s::SocketObject pending{pending_state};
    s::SessionPair output;unsigned moves=0;std::function<void(unsigned)> move_hook;
    s::ConstructionContext context{alloc,{this,move},life,scratch};
    static void move(void* p,s::ExecutorFields& dst,s::ExecutorFields& src,std::uintptr_t table){
        auto& f=*static_cast<Fixture*>(p);if(table!=0x180084928ULL)throw Fault{};
        dst.context=src.context;dst.zero_74=src.zero_74;dst.self_reference_80=true;++f.moves;if(f.move_hook)f.move_hook(f.moves);
    }
    Fixture(){
        s::initialize_0211b0(pending,{0x51},life);pending_state.socket=0x123456789abcdef0ULL;pending_state.socket_state=0x41;
        block.strong_08=7;block.weak_0c=9;block.object.input.storage_00.fill(0xaa);block.object.input.size_10=3;block.object.input.capacity_18=31;
        block.object.weak_self.input_a8=&block.object.input;block.object.cipher.native().mode_00=99;block.object.cipher.encrypted_c4=0xbb;block.object.cipher.unknown_c5=0xcc;block.object.cipher.native().iv_b4.fill(0xdd);
        alloc.proxy.zero_08=0xbeef;
    }
    void close(){
        life.before_close={};
        if(scratch.temporary_live){s::destroy_024db0(scratch.temporary,life);scratch.temporary_live=false;}
        if(scratch.allocation&&block.strong_08>0){while(block.strong_08>0)strong_release(&block);}
        s::destroy_024db0(pending,life);
    }
};
void normal(){
    Fixture f;bool first=false,second=false,proxy=false,temp=false;Ref seed;
    f.move_hook=[&](unsigned n){
        if(n==1){first=f.block.strong_08==1&&f.block.weak_0c==1&&f.block.table_00==0x180084750ULL&&!f.scratch.published;f.block.object.weak_self.object_00=0x777;f.block.object.weak_self.control_08=&seed;}
        if(n==2)second=f.block.object.weak_self.object_00==0&&f.block.object.weak_self.control_08==nullptr&&f.block.object.weak_self.input_a8==&f.block.object.input&&f.block.object.cipher.encrypted_c4==0xbb;
    };
    f.alloc.proxy_hook=[&]{auto& o=f.block.object;proxy=o.input.size_10==0&&o.input.capacity_18==15&&std::all_of(o.input.storage_00.begin(),o.input.storage_00.end(),[](auto x){return x==0;})&&!o.output.proxy_c8&&o.output.fields.map_d0.empty()&&!o.output.fields.map_size_d8&&!o.output.fields.offset_e0&&!o.output.fields.count_e8&&o.cipher.native().mode_00==99;};
    f.life.before_close=[&](s::SocketObject& o){if(&o==&f.scratch.temporary){auto& v=f.block.object;temp=!f.output.object&&!f.output.control&&!f.scratch.published&&v.output.proxy_c8==&f.alloc.proxy&&f.alloc.proxy.owner_00==&v.output&&!f.alloc.proxy.zero_08&&v.cipher.native().mode_00==0&&!v.cipher.encrypted_c4&&!v.cipher.unknown_c5&&std::all_of(v.cipher.native().iv_b4.begin(),v.cipher.native().iv_b4.end(),[](auto x){return x==0;});}};
    require(&s::construct_session_owner_023000(f.output,f.pending,f.context)==&f.output,"normal constructor returns exact explicit output slot");
    require(first&&second&&seed.deleted==0,"control1/1/table prefix precedes first move; weak pair clears after first move without touching input linkage or resetting cipher early");
    require(proxy&&temp,"input/deque zero before proxy allocation; backlink/cipher ready before temporary destructor while output still unpublished");
    require(f.alloc.controls==1&&f.alloc.proxies==1&&f.moves==2,"exact1c8/16 requests and two real023d20 moves");
    require(f.output.object==&f.block.object&&f.output.control==&f.block&&f.block.strong_08==1&&f.block.weak_0c==2,"fresh output is one strong owner plus implicit and self weak references");
    require(f.block.object.weak_self.object_00==f.block.object.identity()&&f.block.object.weak_self.control_08==&f.block&&f.block.object.weak_self.input_a8==&f.block.object.input,"factory object, self-weak, input and output share one stable identity/control domain");
    require(f.service.head==&f.block.object.socket&&f.block.object.socket.next==&f.pending&&f.pending.previous==&f.block.object.socket&&!f.scratch.temporary.next&&!f.scratch.temporary.previous,"temporary unlinked before publication; owned and invalid pending nodes remain");
    require(f.scratch.complete&&f.scratch.published&&!f.scratch.temporary_live&&f.scratch.owner_socket_live&&f.scratch.proxy_live,"scratch records normal completed phases without becoming a native owner");
    f.close();require(f.block.disposed==1&&f.block.deleted==1&&f.block.proxies==1&&f.service.head==nullptr,"selected disposal releases actual owned socket/proxy/self weak before implicit weak delete");
    require(f.life.closes==std::vector<s::Socket>{s::invalid_socket,0x123456789abcdef0ULL,s::invalid_socket},"temporary, final active and pending cleanup close only one live socket");
}
void expired_weak_callback(bool throws){
    Fixture f;Ref old,alternate;old.strong_08=0;old.weak_0c=1;alternate.strong_08=5;
    f.move_hook=[&](unsigned n){if(n==2){f.block.object.weak_self.object_00=0x999;f.block.object.weak_self.control_08=&old;}};
    bool published=false;
    old.on_delete=[&]{published=f.output.object==&f.block.object&&f.output.control==&f.block&&f.scratch.published&&!f.scratch.temporary_live&&f.block.object.weak_self.control_08==&f.block&&f.block.strong_08==2&&f.block.weak_0c==2;f.output.control=&alternate;if(throws)throw Fault{};};
    bool caught=false;try{s::construct_session_owner_023000(f.output,f.pending,f.context);}catch(const Fault&){caught=true;}
    require(published&&old.deleted==1&&old.weak_0c==0,"expired old weak deletion observes published pair and already-replaced self weak");
    require(f.output.control==&alternate&&alternate.strong_08==5&&f.block.strong_08==(throws?2u:1u),"cached original output control releases after callback rather than reloading mutated output");
    require(caught==throws&&f.scratch.complete!=throws&&f.scratch.published,"callback exception preserves publication and unreleased local strong prefix without guessed EH rollback");
    rejects<s::UnsupportedInput>([&]{s::construct_session_owner_023000(f.output,f.pending,f.context);},"published or incomplete scratch cannot be blindly retried");
    f.close();require(f.block.disposed==1&&f.block.deleted==1,"explicit known controlled cleanup can drain retained source prefix after callback outcome");
}
void live_weak_is_not_replaced(){
    Fixture f;Ref old;old.weak_0c=2;
    f.move_hook=[&](unsigned n){if(n==2){f.block.object.weak_self.object_00=0x777;f.block.object.weak_self.control_08=&old;}};
    s::construct_session_owner_023000(f.output,f.pending,f.context);
    require(f.block.strong_08==1&&f.block.weak_0c==1&&old.weak_0c==2&&old.deleted==0&&f.block.object.weak_self.object_00==0x777&&f.block.object.weak_self.control_08==&old,"already-live self weak is preserved, without unnecessary fresh strong/weak increments");
    f.close();require(old.weak_0c==1&&f.block.deleted==1,"owner dispose releases retained foreign self weak, not its live implicit weak");strong_release(&old);require(old.disposed==1&&old.deleted==1,"external old owner remains separately releasable");
}
void allocation_failures(){
    for(unsigned mode=0;mode<3;++mode){
        Fixture f;f.alloc.null_control=mode==0;f.alloc.null_proxy=mode==1;f.alloc.throw_proxy=mode==2;Ref marker;f.output.control=&marker;
        bool caught=false;try{s::construct_session_owner_023000(f.output,f.pending,f.context);}catch(const std::bad_alloc&){caught=mode<2;}catch(const Fault&){caught=mode==2;}
        require(caught&&!f.output.object&&f.output.control==&marker&&!f.scratch.published&&!f.scratch.complete,"allocator failure/contract-null preserves previous plain output slot without normal null-owner success");
        require(f.scratch.begun&&f.moves==(mode==0?0u:2u)&&f.scratch.temporary_live==(mode!=0)&&f.scratch.owner_socket_live==(mode!=0)&&!f.scratch.proxy_live,"persistent scratch retains exact known move/allocation prefix when proxy cannot be constructed");
        if(mode){require(f.service.head==&f.block.object.socket&&f.block.object.socket.next==&f.scratch.temporary&&f.scratch.temporary.next==&f.pending,"throwing proxy allocation leaves live caller-owned temporary node, never a dangling function-local socket");f.close();require(f.block.disposed==1&&f.block.deleted==1,"separate known lower-provider cleanup drains retained partial construction");}
        else{s::destroy_024db0(f.pending,f.life);require(f.alloc.proxies==0&&f.pending_state.socket==s::invalid_socket,"control contract violation does not start moves/proxy work");}
    }
}
}
int main(){try{normal();expired_weak_callback(false);expired_weak_callback(true);live_weak_is_not_replaced();allocation_failures();std::cout<<"PASS: "<<checks<<" session-owner constructor core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"unexpected source test exception\n";return 1;}}
