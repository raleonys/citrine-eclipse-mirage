#include "write_finish.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <vector>
namespace s=anadius::socket_recovered;
namespace {
unsigned checks=0;void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Stop{};struct Terminal{};
[[noreturn]]void unexpected(){throw std::logic_error("Unexpected write finish dependency");}
struct Completion:s::IoCompletionEffects{
 std::function<s::Category()> misc;
 s::Category misc_category_0285b0()override{return misc();}
 void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unexpected();}
 void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unexpected();}
 void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unexpected();}
 void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unexpected();}
};
struct Category:s::WriteFinishCategory{std::function<void(s::WriteFinishString&,std::int32_t)> message;void message_slot_10(s::WriteFinishString& v,std::int32_t code)override{message(v,code);}};
struct Api:s::WriteFinishApi{
 std::function<void(std::uintptr_t,std::uint64_t)> free;
 std::function<void(std::uintptr_t)> next;
 std::function<s::WriteFinishStream&(std::uintptr_t,std::uintptr_t)> begin;
 std::function<s::WriteFinishStream&(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)> append;
 std::function<std::uint64_t(std::uintptr_t,std::uint8_t)> widen;
 std::function<void(s::WriteFinishStream&,std::uint8_t)> put;
 std::function<void(s::WriteFinishStream&)> flush;
 std::function<void()> invalid;
 void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{free(p,n);}
 [[noreturn]]void invalid_parameter_05c974()override{invalid();throw Terminal{};}
 void next_write_02a500(std::uintptr_t p)override{next(p);}
 s::WriteFinishStream& begin_log_006880(std::uintptr_t p,std::uintptr_t n)override{return begin(p,n);}
 s::WriteFinishStream& append_007990(s::WriteFinishStream& o,std::uintptr_t p,std::uint64_t n)override{return append(o,p,n);}
 std::uint64_t widen_01e510(std::uintptr_t p,std::uint8_t c)override{return widen(p,c);}
 void put_00fe90(s::WriteFinishStream& o,std::uint8_t c)override{put(o,c);}
 void flush_00e1e0(s::WriteFinishStream& o)override{flush(o);}
};
void heap(s::WriteFinishString& v,std::uintptr_t p){for(unsigned i=0;i<8;++i)v.storage_00[i]=static_cast<std::uint8_t>(p>>(8*i));}
struct Fixture{
 Completion completion;Api api;Category cat,other;s::SendHandler handler;s::Error error{0,{0x101}};
 s::WriteFinishString item;std::array<s::WriteFinishString*,8> map{};s::WriteFinishOwner owner{map,8,3,2};
 std::array<s::WriteFinishOwnerBinding,1> owners{{{0x456,&owner}}};
 std::array<s::WriteFinishCategoryBinding,2> cats{{{{0x101},&cat},{{0x102},&other}}};
 std::array<s::WriteFinishAllocationHeader,1> headers{{{0x10000-8,0x10000-8}}};
 s::WriteFinishContext context{completion,api,owners,cats,headers};
 s::WriteFinishStream initial{0x500,7},returned{0x800,-12};s::WriteFinishString* message=nullptr;
 std::vector<std::string> events;std::string throw_at;std::uint64_t message_capacity=15;
 Fixture(){handler.word_88=0x456;map.fill(&item);item.storage_00.fill(0xa5);item.size_10=7;cat.identity_08=0x201;other.identity_08=0x202;
  completion.misc=[&]{event("misc");return s::Category{0x201};};
  api.free=[&](auto p,auto n){event("free");check(p==0x10000&&n==17,"default sized free matches heap cap16");};
  api.next=[&](auto p){event("next");check(p==0x456,"next uses current handler owner token");};
  api.invalid=[&]{event("invalid");};
  api.begin=[&](auto global,auto prefix)->s::WriteFinishStream&{event("begin");check(global==0x1800a4130ULL&&prefix==0x1800845d8ULL,"log uses exact opaque global and prefix");return initial;};
  cat.message=[&](auto& v,auto code){event("message");check(code==error.value,"message gets retained code");message=&v;v.storage_00.fill(0x6a);v.size_10=9;v.capacity_18=message_capacity;if(message_capacity>15)heap(v,0x10000);};
  other.message=cat.message;
  api.append=[&](auto& stream,auto p,auto n)->s::WriteFinishStream&{event("append");check(&stream==&initial&&n==9,"append gets initial stream and message length");check(p==(message_capacity>15?0x10000:reinterpret_cast<std::uintptr_t>(message->storage_00.data())),"append chooses live inline or heap representation");return returned;};
  api.widen=[&](auto p,auto c){event("widen");check(p==0x7f4&&c==10,"widen uses returned stream signed vbase adjustment and LF");return 0x1234ULL;};
  api.put=[&](auto& stream,auto c){event("put");check(&stream==&returned&&c==0x34,"put uses low8 widening and returned stream");};
  api.flush=[&](auto& stream){event("flush");check(&stream==&returned,"flush returned append stream");};
 }
 void event(const std::string& v){events.push_back(v);if(throw_at==v)throw Stop{};}
 void run(){s::finish_write_025a70(handler,error,context);}
};
void success_matrix(){for(auto capacity:{0ULL,15ULL,16ULL,4094ULL,4095ULL,4096ULL,std::numeric_limits<unsigned long long>::max()})for(auto count:{0ULL,1ULL,2ULL,std::numeric_limits<unsigned long long>::max()})for(auto offset:{0ULL,3ULL,std::numeric_limits<unsigned long long>::max()}){
 Fixture f;f.item.capacity_18=capacity;f.owner.count_e8=count;f.owner.offset_e0=offset;if(capacity>15)heap(f.item,0x10000);const auto before=f.item.storage_00;
 f.api.free=[&](auto p,auto n){f.event("free");const auto raw=static_cast<std::uint64_t>(capacity)+1;check(p==(raw>=4096?0xfff8:0x10000),"correct data or aligned base passed to free");check(n==(raw>=4096?raw+39:raw),"sized free cap+1 and large +39 wrap");check(f.item.size_10==7&&f.item.capacity_18==capacity&&f.owner.count_e8==count,"free precedes string reset and queue mutation");};
 f.api.next=[&](auto p){f.event("next");check(p==0x456&&f.item.size_10==0&&f.item.capacity_18==15&&f.item.storage_00[0]==0,"next follows string reset");check(f.owner.count_e8==static_cast<std::uint64_t>(count)-1,"count decrements modulo64");check(f.owner.offset_e0==(count==1?0:static_cast<std::uint64_t>(offset)+1),"offset zero only after empty else modulo increment");};
 f.run();check(f.events==(capacity>15?std::vector<std::string>{"free","next"}:std::vector<std::string>{"next"}),"success has no category or logging callbacks");for(unsigned i=1;i<16;++i)check(f.item.storage_00[i]==before[i],"only first inline byte reset");
}}
void live_success(){
 {Fixture f;s::WriteFinishString selected,wrong;selected.capacity_18=16;heap(selected,0x10000);wrong.size_10=99;f.map.fill(&wrong);f.map[1]=&selected;f.owner.map_size_d8=6;f.owner.offset_e0=3;f.api.free=[&](auto,auto){f.event("free");f.map[1]=&wrong;f.owner.map_size_d8=8;f.owner.offset_e0=7;f.owner.count_e8=4;f.handler.word_88=0x999;};f.api.next=[&](auto p){f.event("next");check(p==0x999&&f.owner.count_e8==3&&f.owner.offset_e0==8,"free mutations are live for saved owner fields and next token");};f.run();check(selected.size_10==0&&selected.capacity_18==15&&wrong.size_10==99,"nonpower mask selects one saved string across free mutation");}
 {Fixture f;auto handler=std::make_unique<s::SendHandler>();handler->word_88=0x456;auto owner=std::make_unique<s::WriteFinishOwner>(f.owner);f.owners[0].owner=owner.get();f.api.next=[&](auto){f.event("next");handler.reset();owner.reset();};s::finish_write_025a70(*handler,f.error,f.context);check(!handler&&!owner&&f.events==std::vector<std::string>{"next"},"next may end handler and owner lifetimes");}
}
void alignment(){for(auto displacement:{0ULL,1ULL,31ULL,32ULL,~0ULL})for(bool message:{false,true}){Fixture f;f.headers[0].allocation_base=0x10000-8-displacement;f.item.capacity_18=4095;heap(f.item,0x10000);f.error.value=message?5:0;f.message_capacity=4095;f.api.free=[&](auto p,auto n){f.event("free");check(p==f.headers[0].allocation_base&&n==4135,"large string uses header base and exact sized free");};bool terminal=false;try{f.run();}catch(const Terminal&){terminal=true;}check(terminal==(displacement>31),"unsigned header displacement boundary validated");if(terminal)check(f.events.back()=="invalid"&&std::find(f.events.begin(),f.events.end(),"free")==f.events.end(),"invalid header terminates before free or success queue changes");}}
void logging(){for(auto code:{-2147483647,-1,1,2,3,2147483647})for(bool same:{false,true})for(auto capacity:{15ULL,16ULL,4094ULL,4095ULL,~0ULL}){
 Fixture f;f.error.value=code;f.cat.identity_08=same?0x201:0x202;f.message_capacity=capacity;f.api.free=[&](auto p,auto n){f.event("free");const auto raw=static_cast<std::uint64_t>(capacity)+1;check(p==(raw>=4096?0xfff8:0x10000)&&n==(raw>=4096?raw+39:raw),"message destruction after flush uses exact allocation fields");};
 f.run();if(code==2&&same)check(f.events==std::vector<std::string>{"misc"},"only exact code2 category identity match suppresses output");else{auto expected=std::vector<std::string>{"misc","begin","message","append","widen","put","flush"};if(capacity>15)expected.push_back("free");check(f.events==expected,"error logging preserves native sequence");}check(f.owner.count_e8==2&&f.item.size_10==7,"errors never pop successful queue");
}}
void live_logging(){
 {Fixture f;f.error.value=5;f.completion.misc=[&]{f.event("misc");f.error.value=2;f.error.category={0x102};return s::Category{0x201};};f.api.begin=[&](auto,auto)->s::WriteFinishStream&{f.event("begin");f.error.category={0x101};f.other.message=[&](auto& v,auto code){f.event("late_message");check(code==5,"code cached before misc despite mutation");f.message=&v;v.size_10=9;};return f.initial;};f.run();check(std::find(f.events.begin(),f.events.end(),"late_message")!=f.events.end(),"category projected after misc and cached across begin with live message method");}
 for(auto offset:{std::numeric_limits<std::int32_t>::min(),-1,0,1,std::numeric_limits<std::int32_t>::max()}){Fixture f;f.error.value=5;f.returned.identity=~std::uintptr_t{0}-7;f.returned.virtual_base_offset_04=offset;f.api.widen=[&](auto p,auto c){f.event("widen");check(p==f.returned.identity+static_cast<std::uintptr_t>(static_cast<std::intptr_t>(offset))&&c==10,"signed32 virtual offset sign extends then pointer wraps");return 0x1234ULL;};f.api.flush=[&](auto&){f.event("flush");f.message->capacity_18=16;heap(*f.message,0x10000);};f.run();check(f.events.back()=="free","message allocation fields reloaded after flush");}
}
template<class F>void unsupported(F action){bool caught=false;try{action();}catch(const s::UnsupportedInput&){caught=true;}check(caught,"invalid bounded projection rejected explicitly");}
void registries(){
 for(unsigned kind=0;kind<5;++kind){Fixture f;std::array<s::WriteFinishOwnerBinding,2> duplicate{{f.owners[0],f.owners[0]}};if(kind==0)f.context.owners={};if(kind==1)f.owners[0].owner=nullptr;if(kind==2)f.context.owners=duplicate;if(kind==3)f.owner.map_d0={};if(kind==4)f.map[3]=nullptr;unsupported([&]{f.run();});check(f.events.empty(),"invalid owner/map rejected before effects");}
 for(unsigned kind=0;kind<4;++kind){Fixture f;f.error.value=5;std::array<s::WriteFinishCategoryBinding,2> duplicate{{f.cats[0],f.cats[0]}};if(kind==0)f.context.categories={};if(kind==1)f.cats[0].category=nullptr;if(kind==2)f.context.categories=duplicate;if(kind==3)f.cats[0].key.logical=anadius::server_recovered::ErrorCategory::other;unsupported([&]{f.run();});check(f.events==std::vector<std::string>{"misc"},"category projection validates after misc before log");}
 for(bool duplicate:{false,true}){Fixture f;f.item.capacity_18=4095;heap(f.item,0x10000);std::array<s::WriteFinishAllocationHeader,2> d{{f.headers[0],f.headers[0]}};if(duplicate)f.context.allocation_headers=d;else f.context.allocation_headers={};unsupported([&]{f.run();});check(f.events.empty(),"header projection rejects before free");}
}
struct HeaderOwner final:s::AllocationHeaderResolver {
 std::uintptr_t base=0xffe8;s::MemoryLookup result=s::MemoryLookup::Found;unsigned calls=0;
 s::MemoryLookup lookup_header(std::uintptr_t address,std::uintptr_t& output)override{
  ++calls;check(address==0xfff8,"write resolves exact data-8 header");
  if(result==s::MemoryLookup::Found)output=base;return result;
 }
};
void authoritative_headers(){
 for(bool message:{false,true}){
  Fixture f;HeaderOwner owner;f.context.allocation_header_resolver=&owner;
  f.error.value=message?5:0;f.message_capacity=4095;f.item.capacity_18=4095;heap(f.item,0x10000);
  f.api.free=[&](auto p,auto n){f.event("free");check(p==owner.base&&p!=f.headers[0].allocation_base&&n==4135,"actual write/log destruction uses current owner header over stale catalog");};
  f.run();check(owner.calls==1,"one exact header read per actual destruction");
 }
 for(auto result:{s::MemoryLookup::Miss,s::MemoryLookup::Ambiguous}){
  Fixture f;HeaderOwner owner;owner.result=result;f.context.allocation_header_resolver=&owner;
  f.item.capacity_18=4095;heap(f.item,0x10000);unsupported([&]{f.run();});
  check(owner.calls==1&&f.events.empty()&&f.owner.count_e8==2&&f.item.size_10==7,"owner miss/ambiguity rejects before free/reset even when stale header is valid");
 }
 Fixture f;HeaderOwner owner;owner.base=0x10000-40;f.context.allocation_header_resolver=&owner;
 f.item.capacity_18=4095;heap(f.item,0x10000);bool terminal=false;try{f.run();}catch(const Terminal&){terminal=true;}
 check(terminal&&f.events==std::vector<std::string>{"invalid"}&&f.item.size_10==7&&f.owner.count_e8==2,"authoritative invalid gap preserves original terminal before free and queue mutation");
}
void exceptions(){const auto sequence=std::vector<std::string>{"misc","begin","message","append","widen","put","flush","free"};for(std::size_t i=0;i<sequence.size();++i){Fixture f;f.error.value=5;f.message_capacity=16;f.throw_at=sequence[i];bool caught=false;try{f.run();}catch(const Stop&){caught=true;}check(caught&&f.events==std::vector<std::string>(sequence.begin(),sequence.begin()+i+1),"required logging exception preserves only native normal prefix without invented cleanup");}for(auto effect:{"free","next"}){Fixture f;f.item.capacity_18=16;heap(f.item,0x10000);f.throw_at=effect;bool caught=false;try{f.run();}catch(const Stop&){caught=true;}check(caught,"success required provider exception propagates");check(f.item.capacity_18==(std::string(effect)=="free"?16U:15U),"success mutation sequence stops exactly at throwing callback");}}
}
int main(){try{success_matrix();live_success();alignment();logging();live_logging();registries();authoritative_headers();exceptions();std::cout<<"PASS: "<<checks<<" socket write finish core checks; actual025a70 mask/pop, saved strings/owner and live callback mutations, size/alignment boundaries and terminal, exact category/code suppression and log stream order, wrapped signed virtual offset, heap lifetime ends, bounded projections and callback exception prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}

