#include "flow.hpp"
#include "../anadius-socket-source/frame_read_test_fixture.hpp"
#include <deque>
#include <functional>
#include <iostream>
#include <limits>

namespace s=anadius::socket_recovered;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unexpected(){throw std::logic_error("Unexpected isolated session-flow dependency");}
struct Raised {s::Error error;std::uintptr_t context;};
struct Invalid {};
template<class E,class F>void rejects(F fn){bool caught=false;try{fn();}catch(const E&){caught=true;}check(caught,"required failure propagates without invented native tail cleanup");}
struct Control final:s::IoReference {
 unsigned disposes=0,deletes=0;std::function<void()> dispose,erase;
 void dispose_slot_00()override{++disposes;if(dispose)dispose();}
 void delete_slot_08()override{++deletes;if(erase)erase();}
};
struct Fixture;
struct Sockets final:s::Api {
 struct Step{int result;std::uint32_t bytes;int error;};
 Fixture& f;std::deque<Step> steps;std::vector<s::Socket> sockets;std::vector<std::uint32_t> lengths;std::vector<std::uintptr_t> pointers;std::string written;int last=0;unsigned waits=0;
 explicit Sockets(Fixture& f):f(f){}
 s::Category system_category_028620()override;
 std::int32_t get_last_error()override;
 s::SendCall wsa_send(s::Socket,std::span<s::Buffer>,std::uint32_t)override;
 std::int32_t select_write(s::Socket,std::int32_t)override;
 void set_last_error(std::int32_t)override{unexpected();}s::Conversion convert_acp(const char*,std::int32_t)override{unexpected();}s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unexpected();}
};
struct Logging final:s::WriteFinishApi {
 Fixture& f;s::WriteFinishStream first{0x10000,24},returned{0x20000,-24};
 std::vector<std::pair<std::uintptr_t,std::uint64_t>> frees;s::ReceiveBufferString* message=nullptr;std::string text;std::uint64_t character=0x12345678abcde57fULL;
 explicit Logging(Fixture& f):f(f){}
 void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;[[noreturn]]void invalid_parameter_05c974()override;
 void next_write_02a500(std::uintptr_t)override{unexpected();}
 s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override;
 s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override;
 std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override;
 void put_00fe90(s::WriteFinishStream&,std::uint8_t)override;void flush_00e1e0(s::WriteFinishStream&)override;
};
struct Category final:s::WriteFinishCategory {
 Fixture& f;std::string label;unsigned calls=0;int code=0;
 Category(Fixture& f,std::string label):f(f),label(std::move(label)){}
 void message_slot_10(s::WriteFinishString&,std::int32_t)override;
};
struct Flow final:s::FlowApi {
 Fixture& f;s::SessionPair* local_pair=nullptr;s::ReceiveBufferString* local_greeting=nullptr;bool different_return=false,empty_object=false;
 std::vector<std::uintptr_t> rearmed;std::function<void()> construct_hook,greeting_hook;
 explicit Flow(Fixture& f):f(f){}
 s::SessionPair& construct(s::SessionPair&,s::SocketObject&)override;
 s::ReceiveBufferString& greeting(s::SessionOwner&,s::ReceiveBufferString&)override;
 void start_accept(s::AsyncAcceptService&,s::AcceptorState&,s::AcceptPending&,s::AcceptEndpoint*,const std::uintptr_t&,std::uintptr_t)override;
 [[noreturn]]void raise_write(s::Error&,std::uintptr_t)override;
};
struct Fixture {
 frame_test::Fixture frame;buffer_test::Fixture strings;s::SessionOwner owner;Control control,other_control,third_control;
 s::AcceptorState listener,pending_state,second_listener,second_pending_state;s::SocketObject pending_object{pending_state},second_pending_object{second_pending_state};
 s::AcceptPending pending{pending_object,&frame.base.io.entry},second_pending{second_pending_object,&frame.base.io.entry};s::AsyncAcceptService service{&frame.base.io.entry,0x8765},second_service{&frame.base.io.entry,0x9876};
 s::FlowServer server{0x123400,service,listener,pending},second_server{0x234500,second_service,second_listener,second_pending};
 std::array<s::FlowServerBinding,2> servers{{{server.identity,&server},{second_server.identity,&second_server}}};
 Flow api{*this};Sockets sockets{*this};Logging logging{*this};Category category{*this,"message.a"},second_category{*this,"message.b"};
 std::array<s::WriteFinishCategoryBinding,2> categories{{{{0x1111},&category},{{0x2222},&second_category}}};
 s::FlowContext context{api,strings.context,frame.context,sockets,logging,servers,categories,{0x1111}};
 s::AcceptHandler handler{server.identity,{0,{0x1111}}};s::ReceiveBufferString returned;std::vector<std::string> events;std::function<void(std::string_view)> hook;
 s::SessionPair returned_pair;std::string greeting_bytes;
 Fixture(){
  owner.socket_state.socket=77;owner.socket_state.socket_state=0;owner.cipher.encrypted_c4=0x99;owner.cipher.unknown_c5=0x88;owner.cipher.native().mode_00=2;
  owner.weak_self={owner.identity(),&frame.owner,&frame.base.buffer.value};
  frame.owners[0]={owner.identity(),&owner.weak_self};frame.base.strings[0]={owner.identity()+0xa8,&frame.base.buffer.value};frame.base.streams[0]={owner.identity()+0x10,&frame.base.io.service,&frame.base.io.implementation};
  frame.base.io.implementation.state_08=0x10;frame.seed(std::string("\0",1));
  frame.base.io.completion.hook=[&]{event("read.post");};
  seed("hi\0",3);
 }
 void event(std::string_view name){events.emplace_back(name);if(hook)hook(name);}
 std::size_t count(std::string_view name)const{return std::count(events.begin(),events.end(),name);}
 std::size_t at(std::string_view name)const{auto p=std::find(events.begin(),events.end(),name);check(p!=events.end(),"required event observed");return static_cast<std::size_t>(p-events.begin());}
 void seed(const char* data,std::size_t size,std::uint64_t capacity=15){strings.seed(std::max<std::uint64_t>(capacity,size),size);if(size)std::copy_n(reinterpret_cast<const std::uint8_t*>(data),size,strings.memory.view(s::receive_buffer_address(strings.value),size).begin());greeting_bytes.assign(data,size);}
 std::string bytes(std::uintptr_t pointer,std::uint64_t size)const{
  const std::array<const s::ReceiveBufferString*,3> prefixes{{api.local_greeting,&returned,logging.message}};for(auto* prefix:prefixes)if(prefix){const auto base=reinterpret_cast<std::uintptr_t>(prefix->storage_00.data());if(pointer>=base&&pointer-base<=16&&size<=16-(pointer-base))return {reinterpret_cast<const char*>(pointer),static_cast<std::size_t>(size)};}
  auto view=strings.memory.view(pointer,size);return {reinterpret_cast<const char*>(view.data()),view.size()};
 }
 void call(){s::invoke_accept_handler_021a10(handler,context);}
};
s::SessionPair& Flow::construct(s::SessionPair& out,s::SocketObject& pending){check(&pending==&f.pending_object,"factory receives currently selected server pending socket");check(!out.object&&!out.control,"local session output is initialized plain pair");local_pair=&out;out={empty_object?nullptr:&f.owner,&f.control};f.event("construct");if(construct_hook)construct_hook();return out;}
s::ReceiveBufferString& Flow::greeting(s::SessionOwner& owner,s::ReceiveBufferString& out){check(&owner==&f.owner,"greeting uses factory-returned object");check(owner.cipher.encrypted_c4==0&&owner.cipher.unknown_c5==0&&owner.cipher.native().mode_00==0,"native198definedfield reset copied over existing cipher flags/mode");out=f.strings.value;local_greeting=&out;f.returned=out;f.event("greeting");if(greeting_hook)greeting_hook();return different_return?f.returned:out;}
void Flow::start_accept(s::AsyncAcceptService& service,s::AcceptorState& listener,s::AcceptPending& pending,s::AcceptEndpoint* endpoint,const std::uintptr_t& token,std::uintptr_t allocator){
 auto* expected=token==f.server.identity?&f.server:token==f.second_server.identity?&f.second_server:nullptr;check(expected&&&service==&expected->service&&&listener==&expected->listener&&&pending==&expected->pending,"rearm forwards selected live service/listener/pending references");check(!endpoint&&allocator==token+0x68&&&token!=&expected->identity,"rearm passes endpointNULL, local rawserver reference andidentity+68allocator");const auto cached=token;rearmed.push_back(token);f.event("rearm");check(token==cached,"local server token staysstable across rearm callback");}
void Flow::raise_write(s::Error& error,std::uintptr_t id){f.event("write.raise");throw Raised{error,id};}
s::Category Sockets::system_category_028620(){f.event("send.category");return {0x3333};}
std::int32_t Sockets::get_last_error(){f.event("send.error");return last;}
s::SendCall Sockets::wsa_send(s::Socket socket,std::span<s::Buffer> buffers,std::uint32_t flags){
 check(buffers.size()==1&&flags==0&&buffers[0].len>0&&buffers[0].len<=65536,"greeting singleWSABUF, flags0 and positive cap65536");sockets.push_back(socket);lengths.push_back(buffers[0].len);pointers.push_back(reinterpret_cast<std::uintptr_t>(buffers[0].buf));
 Step step{0,buffers[0].len,0};if(!steps.empty()){step=steps.front();steps.pop_front();}if(step.result==0&&step.bytes<=buffers[0].len)written+=f.bytes(pointers.back(),step.bytes);last=step.error;f.event("send");return {step.result,step.bytes};
}
std::int32_t Sockets::select_write(s::Socket,std::int32_t){++waits;f.event("send.wait");return 0;}
void Logging::free_sized_04b0f8(std::uintptr_t pointer,std::uint64_t count){frees.emplace_back(pointer,count);f.event("free");f.strings.api.free_sized_04b0f8(pointer,count);}
void Logging::invalid_parameter_05c974(){f.event("invalid");throw Invalid{};}
s::WriteFinishStream& Logging::begin_log_006880(std::uintptr_t global,std::uintptr_t prefix){check(global==0x1800a4130ULL&&prefix==0x1800845f8ULL,"accept error logger usesdistinctglobal/prefix");f.event("log");return first;}
s::WriteFinishStream& Logging::append_007990(s::WriteFinishStream& stream,std::uintptr_t pointer,std::uint64_t size){check(&stream==&first,"append startsfrombegin-log stream");text=f.bytes(pointer,size);f.event("append");return returned;}
std::uint64_t Logging::widen_01e510(std::uintptr_t adjusted,std::uint8_t ch){check(adjusted==returned.identity+static_cast<std::uintptr_t>(static_cast<std::intptr_t>(returned.virtual_base_offset_04))&&ch==10,"returnedstream live signed32vbaseoffset and newline10 widened");f.event("widen");return character;}
void Logging::put_00fe90(s::WriteFinishStream& stream,std::uint8_t character){check(&stream==&returned&&character==static_cast<std::uint8_t>(this->character),"put usesreturnedstream and low8ofwiden64result");f.event("put");}
void Logging::flush_00e1e0(s::WriteFinishStream& stream){check(&stream==&returned,"returnedstream isflushed");f.event("flush");}
void Category::message_slot_10(s::WriteFinishString& out,std::int32_t code){++calls;this->code=code;out=f.strings.value;f.logging.message=&out;f.event(label);}

void success(){
 for(std::size_t size:{std::size_t{0},std::size_t{1},std::size_t{15},std::size_t{16},std::size_t{65536},std::size_t{65537}}){Fixture f;std::string text(size,'a');if(size)text[size/2]='\0';f.seed(text.data(),size);const auto cap=f.strings.value.capacity_18;f.call();check(f.sockets.written==text,"binary greeting bytes include embeddedNUL and lengthdoesnotuseCstring");check(f.sockets.lengths.size()==(size?(size+65535)/65536:0),"greeting usesexact65536maximumchunks and emptyskipswrite");check(f.frame.base.io.completion.posts==1&&f.api.rearmed==std::vector<std::uintptr_t>{f.server.identity},"actual0280d0/025c40/021d10postsinitialread thenrearmsaccept");check(f.control.strong_08==0&&f.control.disposes==1&&f.control.deletes==1,"local construction control released onlyafterinitialread");check(f.at("read.post")<f.at("rearm"),"readpublicationprecedesrearm");if(cap>15)check(f.logging.frees.size()==1&&f.at("free")<f.at("read.post"),"localheapgreetingfreedbeforeactualread");else check(f.logging.frees.empty(),"localSSOgreetingrequiresnoregistryorfree");}
 {Fixture f;f.sockets.steps={{0,0,0},{0,1,0},{0,2,0}};f.call();check(f.sockets.lengths==std::vector<std::uint32_t>{3,3,2}&&f.sockets.written==std::string("hi\0",3),"zero-progresssendretainsnativefinite-loopbehaviorandlaterpartialprogress");}
 {Fixture f;f.sockets.steps={{-1,0,10035},{0,3,0}};f.call();check(f.sockets.waits==1&&f.sockets.written==std::string("hi\0",3),"actual02a200select0retryisusedbygreeting");}
 {Fixture f;f.sockets.steps={{0,1,0},{0,2,0}};f.hook=[&](auto event){if(event=="send"&&f.sockets.lengths.size()==1){f.owner.socket_state.socket=88;f.owner.socket_state.socket_state=1;}};f.call();check(f.sockets.sockets==std::vector<s::Socket>{77,88},"session socket/state areloadedliveforeachgreetingcall");}
 {Fixture f;f.sockets.steps={{0,1,0},{-1,0,10035}};f.hook=[&](auto event){if(event=="send"&&f.sockets.lengths.size()==1)f.owner.socket_state.socket_state=1;};bool caught=false;try{f.call();}catch(const Raised& error){caught=true;check(error.error.value==10035,"next02a200callusesnewnonblockingstateandpreserveswould-block");}check(caught&&!f.sockets.waits&&!f.count("read.post")&&f.api.rearmed.empty(),"statebit1loadedafterprevioussendpreventsselectratherthancachingoldstate");}
 {Fixture f;f.api.different_return=true;f.seed("LOCAL",5,32);std::string text(23,'r');auto& block=f.strings.add(64);std::copy(text.begin(),text.end(),block.bytes.begin());const auto original=f.strings.value;f.api.greeting_hook=[&]{f.returned.capacity_18=63;f.returned.size_10=text.size();buffer_test::word(f.returned.storage_00,block.raw);};f.sockets.steps={{0,1,0},{0,22,0}};f.hook=[&](auto event){if(event=="send"&&f.sockets.lengths.size()==1){f.returned.size_10=1;buffer_test::word(f.returned.storage_00,0xbad);}};f.call();check(f.sockets.written==text&&f.sockets.pointers==std::vector<std::uintptr_t>{block.raw,block.raw+1},"greeting data/size capturedfromRETURNEDprefixoncebeforecallbacks");check(f.logging.frees==std::vector<std::pair<std::uintptr_t,std::uint64_t>>{{s::receive_buffer_address(original),33}},"destructionfreesLOCALoutputratherthanreturnedprefix");}
}
void errors_and_control(){
 for(bool shim:{false,true}){Fixture f;f.handler.error={7,{0x1111}};s::Error direct=f.handler.error;f.hook=[&](auto event){if(event=="log"){f.handler.error={99,{0x2222}};direct={99,{0x2222}};f.handler.server_this=f.second_server.identity;}};
  if(shim)f.call();else s::accept_session_025720(f.handler,direct,f.context);
  check(f.category.calls==static_cast<unsigned>(shim)&&f.second_category.calls==static_cast<unsigned>(!shim),"shimcopieserrorbeforecallbacks;directcategoryreadsliveafterbeginlog");check((shim?f.category.code:f.second_category.code)==7,"nativeerrorcodecapturedonentrybeforeloggerchangesit");
  check(f.count("construct")==0&&f.count("greeting")==0&&f.count("send")==0&&f.count("read.post")==0&&!f.control.disposes,"failedacceptonlylogsandrearmswithoutfactory/cipher/greet/send/read/controlcleanup");check(f.api.rearmed==std::vector<std::uintptr_t>{f.second_server.identity},"shimstillpassesORIGINALhandlerreferenceandlivechangedserverrearms");check(f.logging.text==f.greeting_bytes&&f.at("log")<f.at(shim?"message.a":"message.b")&&f.at("append")<f.at("widen")&&f.at("put")<f.at("flush")&&f.at("flush")<f.at("rearm"),"categorymessage append-returnedstream widenlow8 putflushthenrearmorder");
 }
 for(std::int32_t adjustment:{-24,std::numeric_limits<std::int32_t>::min(),std::numeric_limits<std::int32_t>::max()}){Fixture f;f.handler.error.value=-7;f.logging.returned.virtual_base_offset_04=adjustment;f.call();check(f.category.code==-7&&f.logging.text==std::string("hi\0",3),"negativecodeandbinaryerrormessageflowthroughsigned-vbase logger");}
 for(unsigned strong:{0U,1U,2U,UINT32_MAX})for(unsigned weak:{0U,1U,2U}){Fixture f;f.other_control.strong_08=strong;f.other_control.weak_0c=weak;f.frame.base.io.completion.hook=[&]{f.event("read.post");check(f.api.local_pair!=nullptr,"actualreadcallbackseesfactoryLOCALpair");f.api.local_pair->control=&f.other_control;};f.call();check(f.control.strong_08==1&&!f.control.disposes,"localpaircontrolisloadedAFTERactualinitialreadcallback");check(f.other_control.strong_08==strong-1U&&f.other_control.disposes==static_cast<unsigned>(strong==1),"strongcounterwrapsanddisposesonlyold1");check(f.other_control.weak_0c==(strong==1?weak-1U:weak)&&f.other_control.deletes==static_cast<unsigned>(strong==1&&weak==1),"weakdecrementfollowslaststrong and deletesonlyold1");}
 {Fixture f;f.frame.base.io.completion.hook=[&]{f.event("read.post");f.api.local_pair->control=&f.other_control;};f.other_control.dispose=[&]{f.event("dispose");f.api.local_pair->control=&f.third_control;f.other_control.weak_0c=1;f.handler.server_this=f.second_server.identity;};f.other_control.erase=[&]{f.event("delete");};f.call();check(f.other_control.disposes==1&&f.other_control.deletes==1&&f.third_control.strong_08==1&&f.third_control.weak_0c==1,"disposemayreplacepairbutcachedcontrolweak/deleteoriginalisused");check(f.at("read.post")<f.at("dispose")&&f.at("delete")<f.at("rearm")&&f.api.rearmed.back()==f.second_server.identity,"nativecontrolcallbackscompletebeforeLIVEhandlerrearm");}
 for(bool invalid_socket:{false,true}){Fixture f;f.seed("abcdefghijklmnop",16,32);if(invalid_socket)f.owner.socket_state.socket=s::invalid_socket;else{f.owner.socket_state.socket_state=1;f.sockets.steps={{0,1,0},{-1,0,77}};}bool caught=false;try{f.call();}catch(const Raised& error){caught=true;check(error.error.value==(invalid_socket?10009:77)&&error.context==0x180084720ULL,"writeerrorpreservedandliteralwritecontextsenttonoreturnprovider");}check(caught&&f.logging.frees.empty()&&!f.count("read.post")&&f.api.rearmed.empty()&&f.control.strong_08==1,"writefailureprefixhasNOstringfree/read/controlrelease/rearm/EHguess");}
 {Fixture f;f.seed("abcdefghijklmnop",16,32);f.frame.context.read=nullptr;rejects<s::UnsupportedInput>([&]{f.call();});check(f.logging.frees.size()==1&&f.sockets.written==f.greeting_bytes&&!f.control.disposes&&f.api.rearmed.empty(),"requiredreadfailurepropagatesaftergreetingfreewithoutlocalpaircleanup/rearm");}
 {Fixture f;f.api.empty_object=true;rejects<s::UnsupportedInput>([&]{f.call();});check(f.count("construct")==1&&!f.count("greeting")&&!f.control.disposes&&f.api.rearmed.empty(),"invalidfactoryobjectdoesnotinventnormalcleanuporNativeEHrollback");}
 {Fixture f;f.handler.error.value=5;f.hook=[](auto event){if(event=="flush")throw std::runtime_error("flush");};rejects<std::runtime_error>([&]{f.call();});check(f.count("flush")==1&&f.api.rearmed.empty(),"throwingloggerproviderdoesnotfabricatefinalrearm");}
}
void string_lifetime(){
 for(std::uint64_t capacity:{16ULL,4094ULL,4095ULL,4096ULL,UINT64_MAX,UINT64_MAX-1}){
  Fixture f;auto& block=f.strings.add(128);s::ReceiveBufferString prefix;prefix.size_10=0;prefix.capacity_18=capacity;auto count=capacity+std::uint64_t{1};const bool header=count>=4096;const auto pointer=block.raw+(header?8:0);buffer_test::word(prefix.storage_00,pointer);if(header)buffer_test::word(f.strings.memory.view(pointer-8,8),block.raw);f.strings.value=prefix;f.call();check(f.logging.frees==std::vector<std::pair<std::uintptr_t,std::uint64_t>>{{block.raw,header?count+39:count}},"livecapacity+1 threshold4096/header/-8/rawbase/+39usesexactunsignedwrap");check(f.count("read.post")==1&&f.api.rearmed.size()==1,"validzero-sizedheapandcapacityoverflowdomainreachnormalread/rearm");
 }
 for(unsigned distance:{0U,31U,32U}){Fixture f;auto& block=f.strings.add(128);s::ReceiveBufferString prefix;prefix.capacity_18=4095;prefix.size_10=0;const auto pointer=block.raw+8+distance;buffer_test::word(prefix.storage_00,pointer);buffer_test::word(f.strings.memory.view(pointer-8,8),block.raw);f.strings.value=prefix;if(distance==32){rejects<Invalid>([&]{f.call();});check(f.logging.frees.empty()&&!f.count("read.post")&&!f.control.disposes&&f.api.rearmed.empty(),"distance32invalidparameterstopsallnative-tailcleanup");}else{f.call();check(f.logging.frees==std::vector<std::pair<std::uintptr_t,std::uint64_t>>{{block.raw,4135}},"unsigneddistance<=31headerbaseaccepted");}}
 {Fixture f;auto& block=f.strings.add(128);s::ReceiveBufferString prefix;prefix.capacity_18=4095;prefix.size_10=0;buffer_test::word(prefix.storage_00,block.raw+8);buffer_test::word(f.strings.memory.view(block.raw,8),block.raw+1);f.strings.value=prefix;rejects<Invalid>([&]{f.call();});check(f.logging.frees.empty()&&!f.count("read.post")&&f.api.rearmed.empty(),"headerdistanceunderflowusesUNSIGNEDcomparisonandinvokesinvalidparameter");}
 {Fixture f;s::ReceiveBufferString prefix;prefix.capacity_18=4095;prefix.size_10=0;buffer_test::word(prefix.storage_00,0xdeadbeef);f.strings.value=prefix;rejects<s::UnsupportedInput>([&]{f.call();});check(f.logging.frees.empty()&&!f.count("read.post")&&!f.control.disposes&&f.api.rearmed.empty(),"missingdata-minus8headermappingisexplicitunsupportedprefixnotinventednativefree");}
 {Fixture f;f.seed("abcdef",6,32);auto& other=f.strings.add(64);s::ReceiveBufferString replacement;replacement.capacity_18=63;replacement.size_10=0;buffer_test::word(replacement.storage_00,other.raw);f.hook=[&](auto event){if(event=="send")*f.api.local_greeting=replacement;};f.call();check(f.logging.frees==std::vector<std::pair<std::uintptr_t,std::uint64_t>>{{other.raw,64}},"greetingLOCALcapacity/pointerreloadedonlyafterlastsendcallback");}
 {Fixture f;f.seed("abcdef",6,32);f.hook=[&](auto event){if(event=="free")f.handler.server_this=f.second_server.identity;};f.call();check(f.api.rearmed.back()==f.second_server.identity&&f.at("free")<f.at("read.post")&&f.at("read.post")<f.at("rearm"),"successhandlerrearmreadsoriginalserverliveaftergreetingfreecallbackandread");}
 {Fixture f;f.handler.error.value=5;auto& block=f.strings.add(64);s::ReceiveBufferString replacement;replacement.capacity_18=63;replacement.size_10=0;buffer_test::word(replacement.storage_00,block.raw);f.hook=[&](auto event){if(event=="flush")*f.logging.message=replacement;if(event=="free")f.handler.server_this=f.second_server.identity;};f.call();check(f.logging.text==std::string("hi\0",3)&&f.logging.frees==std::vector<std::pair<std::uintptr_t,std::uint64_t>>{{block.raw,64}},"errorstringdata/sizeusedbeforecallbacksbutLOCALdestructorreadsflush-mutatedcapacity/pointer");check(f.api.rearmed.back()==f.second_server.identity&&f.at("flush")<f.at("free")&&f.at("free")<f.at("rearm"),"livehandlerreloadedaftererrorfreecallback");}
}
void cipher_and_rearm(){
 anadius::crypto_recovered::Key key{},other{},iv{};for(unsigned i=0;i<16;++i){key[i]=static_cast<std::uint8_t>(i+1);other[i]=static_cast<std::uint8_t>(i+33);iv[i]=static_cast<std::uint8_t>(i+65);}
 s::SessionCipher destination,expected;{
  s::SessionCipher temporary;temporary.set_key_and_iv_042760(key,iv);temporary.encrypted_c4=0xa1;temporary.unknown_c5=0xb2;s::copy_session_cipher_defined_fields(destination,temporary);expected.set_key_and_iv_042760(key,iv);check(&destination.native().schedule!=&temporary.native().schedule&&destination.native().mode_00==1&&destination.native().iv_b4==iv&&destination.encrypted_c4==0xa1&&destination.unknown_c5==0xb2,"198definedfieldscopiedwhileAESreferencebindstodestinationOWNschedule");temporary.set_key_042780(other);
 }
 std::string left="copy independence",right=left;check(destination.encrypt_042610(left)&&expected.encrypt_042610(right)&&left==right,"ciphercopyencryptsindependentlyaftertemporaryschedulelifetimeended");
 destination.set_key_042780(other);std::string changed="copy independence",original=changed;expected.set_key_and_iv_042760(key,iv);check(destination.encrypt_042610(changed)&&expected.encrypt_042610(original)&&changed!=original,"destinationkeymutationdoesnotaliasexpectedschedule");
 {Fixture f;s::rearm_accept_0270e0(f.server,f.context);check(f.api.rearmed==std::vector<std::uintptr_t>{f.server.identity}&&!f.count("construct")&&!f.count("greeting"),"0270e0isexactargumentadapterwithnolifecycleextras");}
 {Fixture f;const auto original=f.server.identity;f.hook=[&](auto event){if(event=="rearm")f.server.identity=0x999;};s::rearm_accept_0270e0(f.server,f.context);check(f.api.rearmed==std::vector<std::uintptr_t>{original},"0270e0useslocalserver-tokenreferenceinstead ofaliasingmutableidentitymember");}
}
}
int main(){try{success();errors_and_control();string_lifetime();cipher_and_rearm();std::cout<<"PASS: "<<checks<<" session flow core checks; actual025720/021a10/0270e0 and02a200/0280d0 source, synthetic bounded strings/loggers/controls/sockets\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(const Raised& e){std::cerr<<"FAIL: unexpectedwrite "<<e.error.value<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpectedterminal\n";return 1;}}
