#include "dispose.hpp"
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
namespace s=anadius::socket_recovered;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
struct Injected{};struct Invalid{};
using FreeRecord=std::pair<std::uintptr_t,std::uint64_t>;
struct Free:s::SessionFreeEffects {
    std::vector<FreeRecord> freed;std::function<void(std::uintptr_t,std::uint64_t)> call;
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{freed.emplace_back(p,n);if(call)call(p,n);}
    [[noreturn]]void invalid_parameter_05c974()override{throw Invalid{};}
};
struct Gate:s::SocketGate {bool held=false;void enter()override{if(held)throw Injected{};held=true;}void leave()override{if(!held)throw Injected{};held=false;}};
struct Life:s::LifecycleEffects {
    Gate gate;s::SocketService service{gate};unsigned closes=0,cleanups=0;std::function<void(s::SocketObject&)> close;
    s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
    void close_and_cancel_026fb0(s::SocketService& selected,s::SocketObject& object)override{
        if(&selected!=&service)throw Injected{};++closes;if(close)close(object);object.state.socket=s::invalid_socket;
    }
    void executor_cleanup(s::SocketObject&,std::uintptr_t)override{++cleanups;}
};
struct Control:s::SessionControl {
    unsigned deletes=0;std::function<void()> deleted;
    void dispose_slot_00()override{throw Injected{};}
    void delete_slot_08()override{++deletes;if(deleted)deleted();}
};
struct Fixture {
    Life life;Free free;std::array<std::uint8_t,8> header{};std::array<s::ReceiveBufferRegion,1> regions{};
    s::ReceiveBufferMemory memory{{}};s::SessionDisposalContext context{memory,free,life};Control control;
    Fixture(){s::initialize_0211b0(control.object.socket,{1},life);control.object.socket_state.socket=77;}
    void base(std::uintptr_t data,std::uintptr_t allocation){
        for(std::size_t i=0;i<8;++i)header[i]=static_cast<std::uint8_t>(allocation>>(i*8));
        regions[0]={data-8,header};memory.regions=regions;
    }
};
void heap(s::ReceiveBufferString& value,std::uintptr_t address,std::uint64_t capacity){
    value.storage_00.fill(0xa7);for(std::size_t i=0;i<8;++i)value.storage_00[i]=static_cast<std::uint8_t>(address>>(i*8));
    value.size_10=9;value.capacity_18=capacity;
}
std::uintptr_t id(const void* p){return reinterpret_cast<std::uintptr_t>(p);}
void ordered_dispose(){
    Fixture f;auto& owner=f.control.object;s::DequeProxy proxy;
    std::array<s::WriteFinishString,3> strings;
    heap(strings[0],0x5000,31);strings[1].size_10=5;strings[2].size_10=7;
    std::array<s::WriteFinishString*,4> map{&strings[0],nullptr,&strings[1],&strings[2]};
    owner.output.proxy_c8=&proxy;owner.output.fields={map,4,3,2};
    heap(owner.input,0x6020,4095);f.base(0x6020,0x6000);
    Control self;self.weak_0c=1;owner.weak_self.object_00=0x1234;owner.weak_self.control_08=&self;
    std::vector<std::string> events;
    f.free.call=[&](auto address,auto bytes){
        if(address==id(&proxy)){
            events.push_back("proxy");require(bytes==16&&owner.output.proxy_c8==nullptr,"proxy cleared before free16");
            require(owner.input.capacity_18==4095&&f.life.closes==0&&self.weak_0c==1,"proxy precedes input/socket/weak");
        }
        if(address==0x6000){events.push_back("input");require(bytes==4135&&owner.input.capacity_18==4095,"aligned input native bytes4096+39 before reset");}
    };
    f.life.close=[&](auto&){events.push_back("socket");require(owner.input.size_10==0&&owner.input.capacity_18==15&&owner.input.storage_00[0]==0,"input reset before socket");require(owner.input.storage_00[8]==0xa7,"input tail retained");};
    self.deleted=[&]{events.push_back("weak");require(f.life.cleanups==1&&!f.life.gate.held,"weak delete follows complete socket teardown");};
    s::dispose_session_control_026570(f.control,f.context);
    const std::vector<FreeRecord> expected{{0x5000,32},{id(&strings[2]),32},{id(&strings[1]),32},{id(&strings[0]),32},{id(map.data()),32},{id(&proxy),16},{0x6000,4135}};
    require(f.free.freed==expected,"logical reverse strings, reverse blocks, map, proxy, input sizedfree order");
    require(events==std::vector<std::string>{"proxy","input","socket","weak"},"resource dispose phase order");
    require(owner.output.fields.map_d0.data()==nullptr&&owner.output.fields.map_size_d8==0,"map cleared after frees");
    require(owner.output.fields.count_e8==0&&owner.output.fields.offset_e0==0,"nonempty deque drained");
    require(strings[0].size_10==0&&strings[0].capacity_18==15&&strings[0].storage_00[8]==0xa7,"cached heap element reset only prefix");
    require(strings[1].size_10==5,"unused block payload is not logically destroyed");
    require(owner.weak_self.object_00==0x1234&&owner.weak_self.control_08==&self,"native weak-self slots retained");
    require(self.deletes==1&&f.life.service.head==nullptr,"last weak callback and socket unlink occurred");
}
void empty_and_thresholds(){
    {Fixture f;auto& o=f.control.object;o.output.fields.offset_e0=77;o.input.storage_00.fill(0x62);
     s::dispose_session_control_026570(f.control,f.context);
     require(f.free.freed==std::vector<FreeRecord>{{0,16}},"empty owner still calls null proxy free16");
     require(o.output.fields.offset_e0==77,"initially empty deque retains offset");
     require(o.input.storage_00[0]==0&&o.input.storage_00[1]==0x62,"inline reset keeps remaining15 bytes");}
    {Fixture f;std::array<s::WriteFinishString*,1> storage{};auto& q=f.control.object.output;
     q.fields.map_d0=std::span<s::WriteFinishString*>(storage.data(),0);q.fields.offset_e0=22;
     s::clear_session_output_026880(q,f.context);
     require(f.free.freed==std::vector<FreeRecord>{{id(storage.data()),0}},"nonnull zero-length map still freed");
     require(q.fields.offset_e0==22,"zero count nonnull map preserves offset");}
    {Fixture f;std::array<s::WriteFinishString*,1> storage{};auto& q=f.control.object.output;
     q.fields.map_d0=std::span<s::WriteFinishString*>(storage.data(),0);q.fields.map_size_d8=std::uint64_t{1}<<63;
     s::clear_session_output_026880(q,f.context);
     require(f.free.freed==std::vector<FreeRecord>{{id(storage.data()),0}},"signed negative map bound skips block loop; bytes shift wraps");}
    for(auto capacity: {std::uint64_t{4094},std::numeric_limits<std::uint64_t>::max()}){
        Fixture f;heap(f.control.object.input,0x7000,capacity);s::dispose_session_control_026570(f.control,f.context);
        require(f.free.freed==std::vector<FreeRecord>{{0,16},{0x7000,capacity+1}},"input cap+1 uses unsigned threshold/wrap");
    }
    {Fixture f;auto& q=f.control.object.output;std::array<s::WriteFinishString*,1> storage{};
     q.fields={std::span<s::WriteFinishString*>(storage.data(),0),512,0,0};f.base(id(storage.data()),id(storage.data())-8);
     // All null blocks are projected; full512-entry map is required by the loop.
     std::array<s::WriteFinishString*,512> big{};q.fields.map_d0=big;f.base(id(big.data()),id(big.data())-8);
     s::clear_session_output_026880(q,f.context);
     require(f.free.freed==std::vector<FreeRecord>{{id(big.data())-8,4135}},"512-slot map sizedfree4096+39 with bounded header");}
}
void live_reloads(){
    {Fixture f;auto& q=f.control.object.output;std::array<s::WriteFinishString,4> strings;std::array<s::WriteFinishString*,4> map;
     for(std::size_t i=0;i<4;++i){heap(strings[i],0x8000+i*256,31);map[i]=&strings[i];}
     q.fields={map,4,0,3};f.free.call=[&](auto p,auto){if(p==0x8200)q.fields.count_e8=2;};
     s::clear_session_output_026880(q,f.context);
     require(f.free.freed[0]==FreeRecord{0x8200,32}&&f.free.freed[1]==FreeRecord{0x8000,32},"logical count reloaded after string free");
     require(strings[1].capacity_18==31&&q.fields.count_e8==0,"live count mutation skips prior middle element");}
    {Fixture f;auto& q=f.control.object.output;std::array<s::WriteFinishString,4> strings;
     std::array<s::WriteFinishString*,2> first{&strings[0],&strings[1]},second{&strings[2],&strings[3]};
     q.fields={first,2,19,0};f.free.call=[&](auto p,auto){if(p==id(&strings[1])){q.fields.map_d0=second;q.fields.map_size_d8=1;}};
     s::clear_session_output_026880(q,f.context);
     require(f.free.freed==std::vector<FreeRecord>{{id(&strings[1]),32},{id(&strings[2]),32},{id(second.data()),8}},"cached loop bound, live map and live final size");
     require(q.fields.offset_e0==19,"empty-map path does not reset offset on callback");}
    {Fixture f;auto& o=f.control.object;s::DequeProxy initial,replaced;Control old,intermediate,last;
     old.weak_0c=4;intermediate.weak_0c=5;last.weak_0c=1;o.weak_self.control_08=&old;
     std::array<s::WriteFinishString*,1> map{};o.output.fields={map,1,0,0};o.output.proxy_c8=&initial;
     f.free.call=[&](auto p,auto n){
        if(p==id(map.data()))o.output.proxy_c8=&replaced;
        if(p==id(&replaced)){require(n==16&&o.output.proxy_c8==nullptr,"reloaded proxy cleared before free");heap(o.input,0x9000,31);o.weak_self.control_08=&intermediate;}
     };
     f.life.close=[&](auto&){o.weak_self.control_08=&last;};s::dispose_session_control_026570(f.control,f.context);
     require(f.free.freed==std::vector<FreeRecord>{{id(map.data()),8},{id(&replaced),16},{0x9000,32}},"proxy after deque and input after proxy are live loads");
     require(old.weak_0c==4&&intermediate.weak_0c==5&&last.weak_0c==0&&last.deletes==1,"self weak loaded only after actual024db0");
     require(o.weak_self.control_08==&last,"live replaced weak slot is retained");}
}
void failure_prefixes(){
    {Fixture f;auto& o=f.control.object;s::DequeProxy proxy;o.output.proxy_c8=&proxy;heap(o.input,0xa028,4095);f.base(0xa028,0xa000);
     bool invalid=false;try{s::dispose_session_control_026570(f.control,f.context);}catch(const Invalid&){invalid=true;}
     require(invalid&&f.free.freed==std::vector<FreeRecord>{{id(&proxy),16}},"padding32 terminates after proxy, before input free");
     require(o.output.proxy_c8==nullptr&&o.input.capacity_18==4095&&f.life.closes==0,"invalid input retains unreset prefix and live socket");}
    {Fixture f;heap(f.control.object.input,0xb000,4095);f.base(0xb000,0xb008);bool invalid=false;
     try{s::dispose_session_control_026570(f.control,f.context);}catch(const Invalid&){invalid=true;}
     require(invalid&&f.life.closes==0,"unsigned backwards allocation header rejected");}
    {Fixture f;auto& o=f.control.object;s::DequeProxy proxy;o.output.proxy_c8=&proxy;
     f.free.call=[&](auto,auto){throw Injected{};};bool threw=false;try{s::dispose_session_control_026570(f.control,f.context);}catch(const Injected&){threw=true;}
     require(threw&&o.output.proxy_c8==nullptr&&f.life.closes==0,"proxy free throw preserves prior null store");}
    {Fixture f;auto& o=f.control.object;heap(o.input,0xc000,31);
     f.free.call=[&](auto p,auto){if(p==0xc000)throw Injected{};};bool threw=false;
     try{s::dispose_session_control_026570(f.control,f.context);}catch(const Injected&){threw=true;}
     require(threw&&o.input.size_10==9&&o.input.capacity_18==31&&f.life.closes==0,"input free throw does not fabricate reset/socket cleanup");}
    {Fixture f;auto& o=f.control.object;Control self;self.weak_0c=2;o.weak_self.control_08=&self;f.life.close=[](auto&){throw Injected{};};bool threw=false;
     try{s::dispose_session_control_026570(f.control,f.context);}catch(const Injected&){threw=true;}
     require(threw&&o.input.capacity_18==15&&self.weak_0c==2&&f.life.cleanups==0,"socket lower throw occurs after input reset before weak release");}
    {Fixture f;auto& q=f.control.object.output;std::array<s::WriteFinishString*,1> map{};q.fields={map,1,7,0};
     f.free.call=[](auto,auto){throw Injected{};};bool threw=false;try{s::clear_session_output_026880(q,f.context);}catch(const Injected&){threw=true;}
     require(threw&&q.fields.map_d0.data()==map.data()&&q.fields.map_size_d8==1&&q.fields.offset_e0==7,"map free throw leaves map metadata unreset");}
    {Fixture f;auto& q=f.control.object.output;s::WriteFinishString string;heap(string,0xd028,4095);std::array<s::WriteFinishString*,1> map{&string};q.fields={map,1,0,1};f.base(0xd028,0xd000);
     bool invalid=false;try{s::clear_session_output_026880(q,f.context);}catch(const Invalid&){invalid=true;}
     require(invalid&&f.free.freed.empty()&&q.fields.count_e8==1&&string.capacity_18==4095,"invalid deque string fails before element reset/pop/block free");}
    {Fixture f;auto& o=f.control.object;Control self;self.weak_0c=1;o.weak_self.control_08=&self;self.deleted=[](){throw Injected{};};bool threw=false;
     try{s::dispose_session_control_026570(f.control,f.context);}catch(const Injected&){threw=true;}
     require(threw&&self.weak_0c==0&&o.weak_self.control_08==&self&&f.life.cleanups==1,"last weak callback throw retains completed prefix without slot clearing");}
}
void delete_dispatch(){
    struct Other:s::SessionDeleteTarget {unsigned calls=0;std::uint32_t flags=0;std::uintptr_t delete_flags_slot_10(std::uint32_t value)override{++calls;flags=value;return 99;}} other;
    s::delete_session_control_017420(nullptr);require(other.calls==0,"nullable delete dispatch no-op");
    s::delete_session_control_017420(&other);require(other.calls==1&&other.flags==1,"generic virtualslot10(flags1), not direct selected destructor");
    for(auto flags:{0u,2u,0xfffffffeu,1u,3u,0xffffffffu}){
        Fixture f;f.control.table_00=7;bool restored=false;f.free.call=[&](auto p,auto n){restored=f.control.table_00==0x180084750ULL&&p==id(&f.control)&&n==0x1c8;f.control.table_00=99;};
        auto result=s::delete_session_control_026290(f.control,flags,f.free);
        require(result==id(&f.control),"deleting destructor returns cached control identity");
        if(flags&1)require(restored&&f.free.freed.size()==1&&f.control.table_00==99,"table restored before conditional free1c8, no post-free restore");
        else require(f.free.freed.empty()&&f.control.table_00==0x180084750ULL,"flags lowbit0 restores table without deallocation");
    }
}
}
int main(){try{ordered_dispose();empty_and_thresholds();live_reloads();failure_prefixes();delete_dispatch();std::cout<<"PASS: "<<checks<<" session disposal controls\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"unexpected terminal\n";return 1;}}
