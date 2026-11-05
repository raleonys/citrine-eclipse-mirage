#include "native.hpp"
#include "../anadius-messages-source/control.hpp"
#include "../anadius-thread-domain-source/domain_test_fixture.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include <deque>
#include <iostream>
#include <cstdlib>
#include <new>

std::atomic<bool> fail_next_metadata_new=false;
void* operator new(std::size_t bytes){if(fail_next_metadata_new.exchange(false))throw std::bad_alloc();if(void* raw=std::malloc(bytes?bytes:1))return raw;throw std::bad_alloc();}
void* operator new[](std::size_t bytes){return ::operator new(bytes);}
void operator delete(void* raw)noexcept{std::free(raw);}void operator delete[](void* raw)noexcept{std::free(raw);}
void operator delete(void* raw,std::size_t)noexcept{std::free(raw);}void operator delete[](void* raw,std::size_t)noexcept{std::free(raw);}

namespace s=anadius::socket_recovered;namespace sp=anadius::session_profile;namespace n=anadius::native_profile;namespace m=anadius::messages_recovered;namespace b=b6_core_test;
namespace {
unsigned checks=0;
void check(bool value,const char* message){++checks;if(!value){std::cerr<<"FAIL: "<<message<<'\n';throw std::runtime_error(message);}}
template<class E,class F>void rejects(F fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}check(caught,text);}
[[noreturn]]void unused(){throw std::logic_error("Unselected native-profile test dependency");}
void* WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T bytes){check(heap==::GetProcessHeap()&&(flags==0||flags==HEAP_ZERO_MEMORY),"controlled parent heap uses exact borrowed heap/flags");++b::observed->allocations;auto* raw=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(raw)b::observed->blocks.emplace(raw,bytes);::SetLastError(error);return raw;}
struct Heap {
 b::Observation observed;b::RuntimeState runtime;std::shared_ptr<anadius::thread_domain::DomainStorage> root;
 Heap(){check(!b::observed&&!b::runtime_state,"native-profile fixtures do not overlap");b::observed=&observed;b::runtime_state=&runtime;auto api=b::apis();api.heap.heap_alloc=allocate;root=anadius::thread_domain::DomainStorage::create(b::seed(),api);check(root->initialize(),"real own heap/FLS domain initialized");}
 ~Heap(){if(!root->close())std::terminate();root.reset();if(!observed.blocks.empty())std::terminate();b::observed=nullptr;b::runtime_state=nullptr;}
};
struct Gate:s::SocketGate {void enter()override{}void leave()override{}};
struct Life:s::LifecycleEffects {Gate gate;s::SocketService service{gate};s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}void close_and_cancel_026fb0(s::SocketService&,s::SocketObject& object)override{object.state.socket=s::invalid_socket;}void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}};
void move_executor(void*,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t){to=from;}
void strong_release(s::IoReference* control){if(control->strong_08.fetch_sub(1)==1){control->dispose_slot_00();if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();}}
struct Hooks {std::vector<std::string> trace,inputs,logs;std::function<void(std::string_view)> callback;unsigned parses=0,printers=0,dispatches=0,printers_destroyed=0;bool throw_answer=false;void event(std::string_view name){trace.emplace_back(name);if(callback)callback(name);}};
struct Xml final:m::XmlBackend {
 struct Printer final:m::XmlPrinter {std::unique_ptr<m::XmlPrinter> actual;Hooks& hooks;std::optional<std::string> forced;Printer(std::unique_ptr<m::XmlPrinter> p,Hooks& h,std::optional<std::string> f):actual(std::move(p)),hooks(h),forced(std::move(f)){}~Printer()override{++hooks.printers_destroyed;hooks.event("printer-destroy");}void open(std::string_view tag)override{hooks.event("print-open");if(tag=="Answer"&&hooks.throw_answer)throw std::runtime_error("controlled nested printer unknown outcome");actual->open(tag);}void attribute(std::string_view name,std::string_view value)override{hooks.event("print-attribute");actual->attribute(name,value);}void close()override{hooks.event("print-close");actual->close();}std::string buffer_including_terminator()const override{hooks.event("print-buffer");if(forced)return *forced+std::string(1,0);return actual->buffer_including_terminator();}};
 anadius::xml_reader_bridge::Backend actual;Hooks& hooks;std::deque<std::pair<int,bool>> forced;std::optional<std::string> forced_response;bool fail_proxy_allocation=false,warm_child=false,raw_printer=false;
 explicit Xml(Hooks& h):hooks(h){}
 m::ParseResult parse_c_string(std::string_view input)override{++hooks.parses;hooks.inputs.emplace_back(input);hooks.event("parse");auto result=actual.parse_c_string(input);if(warm_child){const auto* child=result.document->first_element()->first_element()->first_element()->first_element();check(child&&child->name()=="Child","raw XML backend caches child before proxy metadata failure injection");}if(!forced.empty()){result.returned_code=forced.front().first;result.has_error=forced.front().second;forced.pop_front();}return result;}
 std::unique_ptr<m::XmlPrinter> printer()override{++hooks.printers;hooks.event("printer");if(raw_printer)return actual.printer();auto result=std::make_unique<Printer>(actual.printer(),hooks,forced_response);if(fail_proxy_allocation)fail_next_metadata_new=true;return result;}
};
struct Random final:m::Random {Hooks& hooks;explicit Random(Hooks& h):hooks(h){}m::Key challenge_bytes_034cf0()override{hooks.event("random-challenge");return {};}m::Key key_from_response_034c30(std::string_view)override{hooks.event("random-key");m::Key key;key.fill(0x47);return key;}};
struct Effects final:m::Effects,m::UnknownMessageEffects {
 Hooks& hooks;std::string answer="alpha";bool catch_printer=false,catch_node_allocation=false,unknown_control=false;std::vector<std::string> diagnostic_inputs;std::function<void(const m::UnknownMessageInput&)> diagnostic;
 void unknown_message_07bd70(const m::UnknownMessageInput& input)override{hooks.event("unknown-diagnostic");diagnostic_inputs.emplace_back(input.view());if(diagnostic)diagnostic(input);}
std::function<bool(const m::XmlNode&,m::XmlPrinter&)> selected;explicit Effects(Hooks& h):hooks(h){}
 int atoi_05ee08(std::string_view input)override{hooks.event("atoi");check(input=="2"||input=="3","selected ChallengeResponse version arguments");return input=="2"?2:3;}
 bool registered_handler(std::uint32_t rva,const m::XmlNode& node,m::XmlPrinter& printer)override{check(rva==0x02e5f0&&node.name()=="GetConfig","actual032d00 selects exact GetConfig registration and real XML node");++hooks.dispatches;hooks.event("dispatch");if(selected)return selected(node,printer);if(catch_node_allocation){fail_next_metadata_new=true;try{(void)node.first_element();}catch(const std::bad_alloc&){hooks.event("caught-node");return true;}throw std::runtime_error("expected fresh node proxy allocation failure");}if(catch_printer){try{printer.open("Answer");}catch(const std::runtime_error&){hooks.event("caught-printer");return true;}}else printer.open("Answer");printer.attribute("value",answer);printer.close();return true;}
 [[noreturn]]void invalid_message_02bd30_04d228()override{hooks.event("invalid");if(unknown_control)throw m::UnknownMessageError{};unused();}[[noreturn]]void thread_error_04a648(int code)override{hooks.event("thread-error"+std::to_string(code));throw std::runtime_error("selected thread terminal");}
 void log_received(std::string_view value)override{hooks.logs.push_back("received:"+std::string(value));hooks.event("received");}void log_response(std::string_view value)override{hooks.logs.push_back("response:"+std::string(value));hooks.event("response");}void log_event(std::string_view value)override{hooks.logs.push_back("event:"+std::string(value));hooks.event("event");}void log_bad_xml(std::string_view,std::string_view value)override{hooks.logs.push_back("bad:"+std::string(value));hooks.event("bad");}void log_decryption_failure(std::string_view value)override{hooks.logs.push_back("decrypt-fail:"+std::string(value));hooks.event("decrypt-fail");}
};
struct Events final:n::Events {
 Hooks& hooks;std::array<s::ReceiveBufferString,8> values{};std::array<s::ReceiveBufferString*,8> slots{};s::NativeMessageEventQueue data{slots,8,0,0};s::NativeMessageMutex gate{1};unsigned locks=0,unlocks=0;int lock_result=0;std::function<void()> lock_hook;
 explicit Events(Hooks& h):hooks(h){}
 s::NativeMessageEventQueue& queue()override{hooks.event("queue");return data;}s::NativeMessageMutex& mutex()override{hooks.event("mutex");return gate;}
 int lock(s::NativeMessageMutex& value)override{check(&value==&gate,"native event mutex identity cached across lock");++locks;hooks.event("lock");if(lock_hook)lock_hook();return lock_result;}void unlock(s::NativeMessageMutex& value)override{check(&value==&gate,"native unlock uses same typed mutex");++unlocks;hooks.event("unlock");}
};
const std::string request_xml="<LSX><Request id=\"native\" recipient=\"core\"><GetConfig/></Request></LSX>";
std::string hex(std::string_view text){const char* digits="0123456789abcdef";std::string out;for(const unsigned char c:text){out.push_back(digits[c>>4]);out.push_back(digits[c&15]);}return out;}
std::string unhex(std::string_view text){auto digit=[](char c)->unsigned{if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;throw std::runtime_error("invalid oracle hex");};check(text.size()%2==0,"oracle receives complete hex pairs");std::string out;for(std::size_t i=0;i<text.size();i+=2)out.push_back(static_cast<char>((digit(text[i])<<4)|digit(text[i+1])));return out;}
struct Fixture {
 Heap heap;std::shared_ptr<Life> life=std::make_shared<Life>();std::shared_ptr<s::SessionMemoryDomain> memory;std::unique_ptr<sp::Strings> strings;s::AcceptorState pending_state;s::SocketObject pending{pending_state};s::SessionPair pair;Hooks hooks;std::shared_ptr<Xml> xml=std::make_shared<Xml>(hooks);std::shared_ptr<Random> random=std::make_shared<Random>(hooks);std::shared_ptr<Effects> effects=std::make_shared<Effects>(hooks);std::shared_ptr<Events> events=std::make_shared<Events>(hooks);std::shared_ptr<s::SessionCipherFactory> factory=std::make_shared<s::SessionCipherFactory>();std::unique_ptr<n::Logic> logic;s::ReceiveBufferString request,output;unsigned checkpoints=0;
 Fixture(){memory=std::make_shared<s::SessionMemoryDomain>(heap.root->make_memory(),life);strings=std::make_unique<sp::Strings>(*memory,64);auto operation=memory->enter();s::initialize_0211b0(pending,{77},*life);pair=memory->construct_session(pending,{nullptr,move_executor});}
 n::Dependencies dependencies(bool show=false){return {{},xml,factory,random,effects,events,static_cast<std::uint8_t>(show),"test-version"};}
 void create(bool show=false){logic=std::make_unique<n::Logic>(sp::SessionView{{0,1},*pair.object},dependencies(show));}
 template<class F>void operation(F fn){auto scope=memory->enter();fn();}
 void seed(std::string_view text){operation([&]{strings->assign(request,text);});}
 void run(std::function<void()> checkpoint={}){operation([&]{logic->process_native(*memory,*strings,output,request,[&]{++checkpoints;if(checkpoint)checkpoint();});});}
 std::string input(){std::string out;operation([&]{out=strings->read(request);});return out;}std::string result(){std::string out;operation([&]{out=strings->read(output);});return out;}
 void event(std::string_view value,std::size_t slot){operation([&]{strings->assign(events->values[slot],value);events->slots[slot]=&events->values[slot];++events->data.count_20;});}
 void finish(){logic.reset();operation([&]{strings->release(request);strings->release(output);for(auto& event:events->values)strings->release(event);strong_release(pair.control);s::destroy_024db0(pending,*life);strings->prune();if(strings->live_headers()||memory->snapshot().allocations){const auto state=memory->snapshot();std::cerr<<"remaining allocations="<<state.allocations<<" buffers="<<state.buffers<<" controls="<<state.controls<<" headers="<<strings->live_headers()<<" request="<<request.size_10<<" output="<<output.size_10<<"\n";}check(!strings->live_headers()&&!memory->snapshot().allocations,"normal invocation and explicit source disposal drain all parent strings/controls");});strings.reset();check(memory->close(),"owned parent retires after complete source disposal");memory.reset();}
};
void normal_and_boundaries(){
 for(const auto length:{std::size_t{0},std::size_t{15},std::size_t{16},std::size_t{4095},std::size_t{5000}}){
   auto* retained=new Fixture;auto& f=*retained;f.create(true);f.seed(request_xml);const std::string payload(length,'Q');f.xml->forced_response=payload;
   f.hooks.callback=[&](std::string_view event){if(event=="dispatch")check(f.strings->read(f.request)==request_xml,"selected032d00 handler sees same live original request prefix");};
   f.run();check(f.result()==payload+std::string(1,0)&&f.input()==request_xml,"typed033d70 owns exact binary output and explicit framing NUL across SSO/large boundaries");check(!f.logic->retained_prefix()&&f.hooks.parses==1&&f.hooks.dispatches==1&&f.checkpoints>5,"normal typed invocation releases scratch after one real XML/dispatch path");check(f.hooks.logs.size()==2&&f.hooks.logs[0]=="received:"+request_xml&&f.hooks.logs[1]=="response:"+payload,"received and pre-framing response logs preserve actual intermediate values");f.finish();delete retained;
 }
}
void no_response_same_session(){
 for(bool encrypted:{false,true}){
  auto* retained=new Fixture;auto& f=*retained;f.create(true);s::SessionCipher client;client.set_fixed_key_084c98();
  if(encrypted){f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;}
  auto seed=[&]{std::string text=request_xml;if(encrypted){check(client.encrypt_042610(text),"client encrypts same-session control request");text=hex(text);}f.seed(text);};
  f.event("queued",0);f.effects->selected=[](const auto&,auto&)->bool{throw m::NoResponseRequiredError{};};seed();f.run();
  check(f.result().empty()&&!f.logic->retained_prefix()&&f.hooks.printers_destroyed==1&&f.hooks.logs.size()==1&&f.events->data.count_20==1&&!f.events->locks,"known dispatch control retires DOM/printer normally, skips response/event and leaves invocation reusable");
  check(f.input()==request_xml&&f.pair.object->cipher.encrypted_c4==encrypted,"suppression preserves decoded request and exact same owner cipher mode");
  f.effects->selected={};seed();f.run();auto response=f.result();if(encrypted){const auto end=response.find('\0');check(end!=std::string::npos,"next same-session response remains framed");response=unhex(response.substr(0,end));check(client.decrypt_042480(response),"same client decrypts subsequent same-owner output");}
  check(response.find("Answer")!=std::string::npos&&!f.logic->retained_prefix()&&f.hooks.printers_destroyed==2&&f.events->data.count_20==unsigned(!encrypted),"same session processes next normal request and consumes event only on that encrypted normal response");
  f.finish();delete retained;
 }
}
void unknown_same_session(){
 for(bool encrypted:{false,true})for(bool show:{false,true}){
  auto* retained=new Fixture;auto& f=*retained;f.create(show);f.effects->unknown_control=true;s::SessionCipher client;client.set_fixed_key_084c98();
  if(encrypted){f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;}
  const std::string unknown="<LSX><Request id=\"unknown\" recipient=\"core\"><NotRegistered/></Request></LSX>"+std::string("\0tail",5);
  auto seed=[&](std::string text){if(encrypted){check(client.encrypt_042610(text),"Client encrypts full counted unknown request");text=hex(text);}f.seed(text);};
  f.event("queued",0);
  f.effects->diagnostic=[&](const m::UnknownMessageInput& input){
    f.strings->assign(f.request,std::string(5000,'R'));check(input.view()==std::string(5000,'R'),"Borrowed request proxy refreshes native mapping after callback reallocates original request");
  };
  seed(unknown);f.run();
  check(f.effects->diagnostic_inputs==std::vector<std::string>{unknown}&&f.result().empty()&&!f.logic->retained_prefix(),"Actual native XML invalid producer reports full original counted request and completes normally");
  check(f.hooks.logs.size()==unsigned(show)&&!f.events->locks&&f.events->data.count_20==1,"Unknown path obeys only received flag and leaves event/cipher framing for next response");
  f.effects->diagnostic={};seed(request_xml);f.run();auto response=f.result();if(encrypted){response=unhex(response.substr(0,response.find('\0')));check(client.decrypt_042480(response),"Same client decrypts normal response after unknown catch");}
  check(response.find("Answer")!=std::string::npos&&!f.logic->retained_prefix()&&f.events->data.count_20==unsigned(!encrypted),"Same session continues normal dispatch and consumes queued event only on encrypted normal path");
  f.finish();delete retained;
 }
}
void retries_and_same_cipher(){
 {
   auto* retained=new Fixture;auto& f=*retained;f.create();const std::string old="<LSX version=\"\"><Request id=\"native\" recipient=\"core\"><GetConfig/></Request></LSX>";const std::string repaired="<LSX ><Request id=\"native\" recipient=\"core\"><GetConfig/></Request></LSX>";f.seed(old);f.xml->forced.push_back({7,true});f.hooks.callback=[&](std::string_view event){if(event=="dispatch")check(f.strings->read(f.request)==repaired,"native code7 erase commits original prefix before handler dispatch");};f.run();check(f.input()==repaired&&f.hooks.parses==2&&f.hooks.inputs[1]==repaired&&!f.pair.object->cipher.encrypted_c4,"version retry reuses invocation and leaves live unencrypted flag");check(f.result().find("<Answer value=\"alpha\"/>")!=std::string::npos,"real XMLReader/printer plus recovered032d00 builds selected response");f.finish();delete retained;
 }
 {
   auto* retained=new Fixture;auto& f=*retained;f.create(true);f.pair.object->cipher.set_fixed_key_084c98();s::SessionCipher client;client.set_fixed_key_084c98();std::string ciphertext=request_xml;check(client.encrypt_042610(ciphertext),"independent client produces actual AES ciphertext");f.seed(hex(ciphertext));f.run();check(f.pair.object->cipher.encrypted_c4==1&&f.hooks.parses==2&&f.input()==request_xml,"code8 auto-detect switches same owner flag then actual hex/prefix AES decrypt mutates original request");auto output=f.result();check(!output.empty()&&!output.back(),"encrypted response includes distinct framing NUL");output.pop_back();output=unhex(output);check(client.decrypt_042480(output)&&output.find("<Answer value=\"alpha\"/>")!=std::string::npos,"independent client decrypts typed prefix response using owner schedule state");f.finish();delete retained;
 }
 {
   auto* retained=new Fixture;auto& f=*retained;f.create();f.seed(request_xml);m::Key key{};key.fill(0x6b);f.pair.object->cipher.unknown_c5=37;f.hooks.callback=[&](std::string_view event){if(event=="dispatch"){f.pair.object->cipher.set_key_042780(key);f.pair.object->cipher.encrypted_c4=1;}};f.run();auto result=f.result();result.pop_back();result=unhex(result);s::SessionCipher client;client.set_key_042780(key);check(client.decrypt_042480(result)&&result.find("Answer")!=std::string::npos&&f.pair.object->cipher.unknown_c5==37,"semantic handler key/flag mutation controls later actual prefix AES through same coherent SessionCipher");f.finish();delete retained;
 }
}
void encrypted_events(){
 for(const auto size:{std::size_t{15},std::size_t{16},std::size_t{5000}}){auto* retained=new Fixture;auto& f=*retained;f.create(true);f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;s::SessionCipher client;client.set_fixed_key_084c98();std::string encoded=request_xml;check(client.encrypt_042610(encoded),"client encrypts event-case request");f.seed(hex(encoded));std::string event(size,'E');if(size>2)event[2]=0;f.event(event,0);f.event("second",1);f.run();auto output=f.result();const auto separator=output.find('\0');check(separator!=std::string::npos&&separator+1<output.size()&&output.back()==0,"encrypted response and one event have separate binary framing NULs");auto response=unhex(std::string_view(output).substr(0,separator));auto event_output=unhex(std::string_view(output).substr(separator+1,output.size()-separator-2));check(client.decrypt_042480(response)&&client.decrypt_042480(event_output)&&event_output==event,"typed event copy/AES/hex preserves full binary SSO/heap/aligned event");check(f.events->locks==1&&f.events->unlocks==1&&f.events->data.count_20==1&&f.events->data.offset_18==1,"typed native front consumes at most one event under matching lock/unlock");f.operation([&]{check(f.events->values[0].size_10==0&&f.events->values[0].capacity_18==15&&f.strings->read(f.events->values[1])=="second","consumed event resets original front while second event retains bytes");});check(f.hooks.logs.size()==3&&f.hooks.logs.back()=="event:"+event,"event log precedes cipher and preserves embedded NUL bytes");f.finish();delete retained;}
 {
   auto* retained=new Fixture;auto& f=*retained;f.create(true);f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;s::SessionCipher client;client.set_fixed_key_084c98();std::string request=request_xml;check(client.encrypt_042610(request),"client encrypts reload-case request");f.seed(hex(request));f.event("first",0);f.event("second",1);f.hooks.callback=[&](std::string_view name){if(name=="event")f.events->data.offset_18=1;};f.run();f.operation([&]{check(f.strings->read(f.events->values[0])=="first"&&!f.events->values[1].size_10&&f.events->data.count_20==1&&f.events->data.offset_18==2,"native event removal reloads live queue front after event-log callback mutation");});check(f.events->unlocks==1,"live event-front mutation still unlocks same mutex once");f.finish();delete retained;
 }
 {
   auto* retained=new Fixture;auto& f=*retained;f.create(true);f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;s::SessionCipher client;client.set_fixed_key_084c98();std::string request=request_xml;check(client.encrypt_042610(request),"client encrypts empty-after-lock request");f.seed(hex(request));f.event("retained",0);f.events->lock_hook=[&]{f.events->data.count_20=0;};f.run();check(f.events->locks==1&&f.events->unlocks==1&&f.hooks.logs.size()==2&&f.events->values[0].size_10==8,"empty-after-lock recheck skips event/log/pop while preserving original front");f.finish();delete retained;
 }
}
void validation_and_normal_failures(){
 auto* retained=new Fixture;auto& f=*retained;
 auto invalid=[&](auto change){auto d=f.dependencies();change(d);rejects<std::invalid_argument>([&]{n::Logic value({{0,1},*f.pair.object},std::move(d));},"native Logic requires complete selected opaque providers");};invalid([](auto& d){d.xml.reset();});invalid([](auto& d){d.factory.reset();});invalid([](auto& d){d.random.reset();});invalid([](auto& d){d.effects.reset();});invalid([](auto& d){d.events.reset();});f.create();
 f.operation([&]{check(f.logic->matches({{0,1},*f.pair.object})&&!f.logic->matches({{0,2},*f.pair.object})&&!f.logic->matches({{1,1},*f.pair.object}),"Native Logic matches exact owner pointer plus slot and generation");rejects<std::invalid_argument>([&]{f.logic->process_native(*f.memory,*f.strings,f.request,f.request,[]{});},"native request/output alias rejected before invocation publication");rejects<std::invalid_argument>([&]{f.logic->process_native(*f.memory,*f.strings,f.output,f.request,{});},"missing admission checkpoint rejected before invocation publication");check(!f.logic->retained_prefix()&&!f.hooks.parses,"invalid call has no retained invocation or parser effects");});
 auto other_life=std::make_shared<Life>();auto other=std::make_shared<s::SessionMemoryDomain>(f.heap.root->make_memory(),other_life);s::AcceptorState state;s::SocketObject pending(state);{auto operation=other->enter();s::initialize_0211b0(pending,{91},*other_life);const auto pair=other->construct_session(pending,{nullptr,move_executor});check(!f.logic->matches({{0,1},*pair.object}),"same token from independent real parent-owned session cannot match native owner");sp::Strings foreign(*other,4);unsigned admissions=0;rejects<std::invalid_argument>([&]{f.logic->process_native(*f.memory,foreign,f.output,f.request,[&]{++admissions;});},"typed process rejects Strings bound to different live parent before checkpoint");rejects<std::invalid_argument>([&]{(void)f.logic->greeting_native(*f.memory,foreign,[&]{++admissions;});},"typed greeting rejects foreign string domain before checkpoint");check(!admissions&&!f.logic->retained_prefix(),"foreign domain rejection performs no admission or invocation publication");strong_release(pair.control);s::destroy_024db0(pending,*other_life);}check(other->close(),"independent real parent retires after identity negative test");other.reset();
 std::string old="unchanged";rejects<s::UnsupportedInput>([&]{(void)f.logic->process(old);},"semantic stdstring route cannot silently substitute native prefix semantics");check(old=="unchanged"&&!f.logic->retained_prefix(),"semantic route rejection preserves caller string and no prefix");
 f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;f.seed(hex("not-a-block"));f.run();check(f.input()=="not-a-block"&&f.result().empty()&&!f.hooks.parses&&f.hooks.logs.size()==1&&f.hooks.logs[0]=="decrypt-fail:not-a-block"&&!f.logic->retained_prefix(),"actual hex decode commits before normal failed prefix AES, with decoded failure log and no XML dispatch");f.finish();delete retained;
}
struct Lifetime {std::function<void()> callback;~Lifetime(){if(callback)callback();}};
void explicit_call_dependencies(){
 auto* retained=new Fixture;auto& f=*retained;f.create();f.seed(request_xml);
 check(!f.logic->failed_prefix()&&!f.logic->active_calls()&&!f.logic->retained_prefix(),"Idle native Logic distinguishes healthy state from active or unknown prefixes");
 auto invalid=f.dependencies();invalid.xml.reset();unsigned denied_checkpoints=0;
 f.operation([&]{rejects<std::invalid_argument>([&]{f.logic->process_native_call(*f.memory,*f.strings,f.output,f.request,invalid,[&]{++denied_checkpoints;});},"Incomplete explicit call providers reject before an opaque source call");});
 check(!denied_checkpoints&&!f.hooks.parses&&!f.logic->active_calls()&&!f.logic->failed_prefix()&&!f.logic->retained_prefix(),"Invalid explicit dependencies leave no published invocation or poisoned prefix");
 Hooks selected_hooks;auto xml=std::make_shared<Xml>(selected_hooks);auto random=std::make_shared<Random>(selected_hooks);
 auto effects=std::make_shared<Effects>(selected_hooks);auto events=std::make_shared<Events>(selected_hooks);
 const std::string answer(5000,'V');effects->answer=answer;
 auto selected=f.dependencies(true);selected.xml=xml;selected.random=random;selected.effects=effects;selected.events=events;selected.version="explicit-call-version";
 unsigned provider_observations=0,admissions=0;
 selected_hooks.callback=[&](std::string_view){check(f.logic->retained_prefix()&&!f.logic->failed_prefix()&&f.logic->active_calls()==1,"Healthy active call retains HOST observation without becoming a failed prefix");++provider_observations;};
 f.operation([&]{f.logic->process_native_call(*f.memory,*f.strings,f.output,f.request,selected,[&]{++admissions;});});
 check(provider_observations&&admissions&&selected_hooks.dispatches==1&&selected_hooks.parses==1&&selected_hooks.logs.size()==2,"Explicit call uses its own XML/effects/show-log dependencies throughout the recovered body");
 check(!f.hooks.parses&&!f.hooks.dispatches&&f.hooks.logs.empty()&&f.result().find(answer)!=std::string::npos,"Large explicit-call response is published without consulting default parser or effects");
 check(!f.logic->active_calls()&&!f.logic->failed_prefix()&&!f.logic->retained_prefix(),"Known-normal explicit invocation and retirement leave no active or failed row");
 f.operation([&]{f.strings->release(f.output);}); // each native message has a fresh output owner
 f.run();
 check(f.hooks.parses==1&&f.hooks.dispatches==1&&f.result().find("value=\"alpha\"")!=std::string::npos&&selected_hooks.dispatches==1,"Subsequent legacy call still uses the immutable default dependencies without an ambient swap");
 check(!f.logic->active_calls()&&!f.logic->failed_prefix(),"Default and explicit routes share normal completion ownership");
 selected_hooks.callback={};f.finish();delete retained;
}
void dependency_retirement(){
 auto* retained=new Fixture;auto& f=*retained;auto lifetime=std::make_shared<Lifetime>();auto d=f.dependencies();d.lifetime=lifetime;auto logic=std::make_unique<n::Logic>(sp::SessionView{{0,1},*f.pair.object},std::move(d));auto* identity=logic.get();unsigned observed=0,admissions=0;
 lifetime->callback=[&]{check(!identity->retained_prefix(),"normal retiring Logic exposes no retained invocation metadata");rejects<std::logic_error>([&]{(void)identity->greeting();},"final dependency destructor cannot call unguarded greeting during retirement");rejects<std::logic_error>([&]{(void)identity->greeting_native(*f.memory,*f.strings,[&]{++admissions;});},"final dependency destructor cannot start greeting after providers moved");rejects<std::logic_error>([&]{identity->process_native(*f.memory,*f.strings,f.output,f.request,[&]{++admissions;});},"final dependency destructor cannot start typed invocation after providers moved");check(!admissions&&!f.hooks.parses&&!f.hooks.printers,"retirement seal rejects before checkpoint, parent access or opaque provider use");++observed;};lifetime.reset();logic.reset();check(observed==1,"last-owned lifetime retires exactly once while Logic seal remains alive");f.finish();delete retained;
}
void guarded_rich_xml(){
 auto* retained=new Fixture;auto& f=*retained;f.create();f.xml->raw_printer=true;
 f.seed("<LSX><Request id=\"native\" recipient=\"core\"><GetConfig z=\"last\" a=\"&lt;&amp;\">text &lt;&amp;<Child><![CDATA[raw<&>]]></Child></GetConfig></Request></LSX>");
 f.effects->selected=[&](const m::XmlNode& node,m::XmlPrinter& printer){namespace d=anadius::dispatch_recovered;auto* guarded_node=dynamic_cast<const d::GuardedNodeAccess*>(&node);auto* guarded_printer=dynamic_cast<d::GuardedPrinterAccess*>(&printer);check(guarded_node&&guarded_printer,"native wrappers retain explicit guarded node and rich printer interfaces");const auto attributes=guarded_node->attributes_with(f.xml->actual);check(attributes==std::vector<std::pair<std::string,std::string>>{{"z","last"},{"a","<&"}},"guarded NodeAccess forwards raw selected node and preserves attribute order/entity decoding");const auto* text=guarded_node->text_with(f.xml->actual);check(text&&std::string_view(text)=="text <&","guarded text read uses actual XMLReader concrete node contract");const auto* child=node.first_element();auto* guarded_child=dynamic_cast<const d::GuardedNodeAccess*>(child);check(guarded_child&&std::string_view(guarded_child->text_with(f.xml->actual))=="raw<&>","new child proxy retains same backend text/CDATA access");guarded_printer->open_element("Compact",true);printer.attribute("a","\"<&");guarded_printer->push_text("text <&>",false);guarded_printer->close_element(true);guarded_printer->open_element("Cdata",false);guarded_printer->push_text("raw<&>",true);guarded_printer->close_element(false);return true;};
 anadius::xml_recovered::MemoryPrinter expected;expected.open("LSX");expected.open("Response");expected.attribute("id","native");expected.attribute("sender","core");expected.open_element("Compact",true);expected.attribute("a","\"<&");expected.push_text("text <&>",false);expected.close_element(true);expected.open_element("Cdata",false);expected.push_text("raw<&>",true);expected.close_element(false);expected.close();expected.close();f.run();check(f.result()==expected.buffer_including_terminator(),"guarded compact/text/CDATA serialization equals selected direct MemoryPrinter byte-for-byte including framing");check(!f.logic->retained_prefix()&&f.hooks.dispatches==1,"rich interface path normal return retires guarded invocation");f.finish();delete retained;
}
void greeting_and_challenge(){
 auto* retained=new Fixture;auto& f=*retained;f.create();std::string greeting;f.operation([&]{greeting=f.logic->greeting_native(*f.memory,*f.strings,[&]{++f.checkpoints;});});check(greeting.find("version=\"test-version\"")!=std::string::npos&&greeting.find("key=\"00000000000000000000000000000000\"")!=std::string::npos&&greeting.back()==0&&!f.logic->retained_prefix(),"typed greeting invokes guarded real XML printer/random and preserves returned terminator");rejects<s::UnsupportedInput>([&]{(void)f.logic->greeting();},"unguarded no-argument greeting cannot bypass typed admission");
 const std::string challenge="<LSX><Request id=\"n\" recipient=\"core\"><ChallengeResponse key=\"challenge\" version=\"3\"/></Request></LSX>";f.seed(challenge);f.run();check(f.result().find("ChallengeAccepted")!=std::string::npos&&!f.hooks.dispatches,"recovered032d00 selects real ChallengeResponse factory/cipher/random branch");
 m::Key key;key.fill(0x47);s::SessionCipher client;client.set_key_042780(key);std::string encrypted=request_xml;check(client.encrypt_042610(encrypted),"independent client uses key established by ChallengeResponse");f.pair.object->cipher.encrypted_c4=1;f.seed(hex(encrypted));f.operation([&]{f.strings->release(f.output);});f.run();auto response=f.result();response.pop_back();response=unhex(response);check(client.decrypt_042480(response)&&response.find("Answer")!=std::string::npos,"actual native prefix transform shares owner schedule established through semantic ChallengeResponse proxy");f.finish();delete retained;
}
}
[[noreturn]]void retained_child(std::string_view mode){
 auto* retained=new Fixture;auto& f=*retained;f.create(true);f.seed(request_xml);
 if(mode=="unknownproducer"||mode=="unknowndiag"||mode=="unknownview"||mode=="unknownhandler"){
  bool admitted=true,swallowed=false;auto guard=[&]{if(!admitted)throw std::logic_error("selected unknown admission denial");};f.effects->unknown_control=true;
  f.seed("<LSX><Request id=\"unknown\" recipient=\"core\"><NotRegistered/></Request></LSX>");
  if(mode=="unknownproducer")f.hooks.callback=[&](std::string_view event){if(event=="invalid")admitted=false;};
  if(mode=="unknowndiag")f.effects->diagnostic=[](const auto&){throw m::UnknownMessageError{};};
  if(mode=="unknownview")f.effects->diagnostic=[&](const auto& input){admitted=false;try{(void)input.view();}catch(const std::logic_error&){swallowed=true;}};
  if(mode=="unknownhandler"){f.seed(request_xml);f.effects->selected=[](const auto&,auto&)->bool{throw m::UnknownMessageError{};};}
  if(mode=="unknowndiag")rejects<m::UnknownMessageError>([&]{f.run(guard);},"Same-type diagnostic failure escapes without recursive catch");else rejects<std::logic_error>([&]{f.run(guard);},"Denied producer/view or unrelated handler marker cannot become known diagnostic success");
  check(f.logic->retained_prefix()&&!f.hooks.printers_destroyed&&!f.output.size_10&&f.effects->diagnostic_inputs.size()==unsigned(mode=="unknowndiag"||mode=="unknownview"),"Unknown failure retains raw invocation and runs diagnostic at most once before output publication");
  check(swallowed==(mode=="unknownview"),"Swallowed borrowed-view denial remains sticky through reporter return");const auto before=f.checkpoints;rejects<std::logic_error>([&]{f.run();},"Retained unknown prefix cannot be reused");check(f.checkpoints==before,"Poisoned unknown invocation rejects before another opaque checkpoint");
  ::ExitProcess(mode=="unknownproducer"?242:mode=="unknowndiag"?243:mode=="unknownview"?244:245);
 }
 if(mode=="controlguard"){
  bool admitted=true;auto guard=[&]{if(!admitted)throw std::logic_error("selected failed admission");};f.effects->selected=[&](const auto&,auto&)->bool{admitted=false;throw m::NoResponseRequiredError{};};
  rejects<std::logic_error>([&]{f.run(guard);},"known marker cannot erase existing failed source admission");check(f.logic->retained_prefix()&&!f.hooks.printers_destroyed&&!f.output.size_10&&f.hooks.dispatches==1,"denied known-control postcheck retains original invocation/printer without normal cleanup");::ExitProcess(240);
 }
 if(mode=="controlprinter"){
  f.hooks.callback=[](std::string_view event){if(event=="print-open")throw m::NoResponseRequiredError{};};rejects<std::logic_error>([&]{f.run();},"same marker from unrelated opaque printer remains unknown");check(f.logic->retained_prefix()&&!f.hooks.printers_destroyed&&!f.output.size_10&&!f.hooks.dispatches,"unselected printer marker poisons before dispatch and cannot become suppressed success");::ExitProcess(241);
 }
 if(mode=="nodemetadata"){
   f.xml->warm_child=true;f.effects->catch_node_allocation=true;f.seed("<LSX><Request id=\"native\" recipient=\"core\"><GetConfig><Child/></GetConfig></Request></LSX>");rejects<std::logic_error>([&]{f.run();},"caught fresh Node proxy allocation failure outside opaque lower remains sticky");check(!fail_next_metadata_new&&f.logic->retained_prefix()&&f.hooks.trace.back()=="caught-node"&&f.hooks.dispatches==1&&!f.hooks.printers_destroyed&&!f.output.size_10,"node metadata failure stops before any later printer callback and retains raw providers");::ExitProcess(239);
 }
 if(mode=="metadata"){
   f.xml->fail_proxy_allocation=true;rejects<std::bad_alloc>([&]{f.run();},"metadata failure after opaque printer return propagates without losing adopted object");check(!fail_next_metadata_new&&f.logic->retained_prefix()&&f.hooks.printers==1&&!f.hooks.printers_destroyed&&!f.hooks.dispatches,"raw printer remains owned by retained invocation when proxy allocation fails");::ExitProcess(235);
 }
 if(mode=="nested"){
   f.hooks.throw_answer=true;f.effects->catch_printer=true;rejects<std::logic_error>([&]{f.run();},"raw handler swallowing nested proxy printer throw cannot resume outer source");check(f.logic->retained_prefix()&&f.hooks.trace.back()=="caught-printer"&&!f.hooks.printers_destroyed&&f.output.size_10==0,"nested unknown failure poisons before later printer/response callbacks and retains raw printer");::ExitProcess(234);
 }
 if(mode=="dispatchguard"||mode=="greetguard"){
   bool admitted=true,swallowed=false;auto guard=[&]{if(!admitted)throw std::logic_error("controlled caller admission poisoned");};
   f.hooks.callback=[&](std::string_view event){if(event==(mode=="dispatchguard"?"dispatch":"random-challenge")){admitted=false;try{guard();}catch(const std::logic_error&){swallowed=true;}}};
   if(mode=="dispatchguard")rejects<std::logic_error>([&]{f.run(guard);},"swallowed opaque handler admission poison stops nested printer boundary");
   else f.operation([&]{rejects<std::logic_error>([&]{(void)f.logic->greeting_native(*f.memory,*f.strings,guard);},"guarded greeting stops after swallowed random-provider admission poison");});
   check(swallowed&&f.logic->retained_prefix()&&f.hooks.trace.back()==(mode=="dispatchguard"?"dispatch":"random-challenge")&&!f.hooks.printers_destroyed,"admission checkpoint prevents every subsequent opaque printer callback and raw-object retirement");::ExitProcess(mode=="dispatchguard"?233:238);
 }
 if(mode=="self"){
   bool denied=false;f.hooks.callback=[&](std::string_view event){if(event=="parse"){try{f.logic->process_native(*f.memory,*f.strings,f.output,f.request,[]{});}catch(const std::logic_error&){denied=true;}}};rejects<std::logic_error>([&]{f.run();},"caught direct typed Logic reentry poisons active outer invocation");check(denied&&f.logic->retained_prefix()&&f.hooks.parses==1&&!f.hooks.dispatches&&!f.hooks.printers,"sticky self-reentry stops before second parser, dispatch or printer");::ExitProcess(232);
 }
 if(mode=="unknown"){
   f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;s::SessionCipher client;client.set_fixed_key_084c98();std::string wire=request_xml;check(client.encrypt_042610(wire),"client builds retained decrypt-prefix case");f.seed(hex(wire));f.hooks.callback=[](std::string_view event){if(event=="parse")throw std::runtime_error("controlled unknown parser result");};rejects<std::runtime_error>([&]{f.run();},"unknown parser throw escapes actual typed prefix chain");check(f.input()==request_xml&&f.logic->retained_prefix()&&!f.hooks.dispatches&&!f.output.size_10,"unknown parser failure retains already decoded/decrypted original request without guessed rollback");check(f.logic->failed_prefix()&&!f.logic->active_calls(),"Failed invocation remains retained without being reported as a healthy active call");const auto before=f.checkpoints;rejects<std::logic_error>([&]{f.run();},"retained invocation rejects reuse before admission or source effects");check(f.checkpoints==before&&f.logic->failed_prefix()&&!f.logic->active_calls(),"retained failed invocation cannot execute checkpoint or parser a second time");::ExitProcess(231);
 }
 f.pair.object->cipher.set_fixed_key_084c98();f.pair.object->cipher.encrypted_c4=1;s::SessionCipher client;client.set_fixed_key_084c98();std::string wire=request_xml;check(client.encrypt_042610(wire),"client builds retained event case");f.seed(hex(wire));f.event("retained event",0);
 if(mode=="unlock"){f.hooks.callback=[](std::string_view event){if(event=="unlock")throw std::runtime_error("controlled unknown unlock result");};rejects<std::runtime_error>([&]{f.run();},"unknown unlock outcome escapes after typed event pop");check(f.logic->retained_prefix()&&!f.events->data.count_20&&!f.events->values[0].size_10&&f.events->unlocks==1&&!f.output.size_10,"unlock failure preserves committed event consumption and unpublished response without invented cleanup");::ExitProcess(236);}
 f.events->lock_result=1;rejects<std::runtime_error>([&]{f.run();},"native lock failure reaches explicit thread terminal");check(f.logic->retained_prefix()&&f.events->data.count_20==1&&f.events->locks==1&&!f.events->unlocks&&!f.output.size_10,"thread terminal retains original event/lock prefix and performs no invented unlock or output move");::ExitProcess(237);
}
void isolated(std::wstring mode,DWORD expected){std::array<wchar_t,32768> path{};const auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));check(n&&n<path.size(),"own native-prefix fixture executable located");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" --"+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};check(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"isolated fail-closed prefix fixture launched");const auto wait=::WaitForSingleObject(process.hProcess,30000);DWORD code=0;check(wait==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==expected,"isolated native-prefix fixture reached exact retention assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
int main(int argc,char** argv){try{if(argc==2)retained_child(std::string_view(argv[1]).substr(2));normal_and_boundaries();no_response_same_session();unknown_same_session();retries_and_same_cipher();encrypted_events();validation_and_normal_failures();greeting_and_challenge();explicit_call_dependencies();dependency_retirement();guarded_rich_xml();isolated(L"unknown",231);isolated(L"self",232);isolated(L"dispatchguard",233);isolated(L"nested",234);isolated(L"metadata",235);isolated(L"unlock",236);isolated(L"lock",237);isolated(L"greetguard",238);isolated(L"nodemetadata",239);isolated(L"controlguard",240);isolated(L"controlprinter",241);isolated(L"unknownproducer",242);isolated(L"unknowndiag",243);isolated(L"unknownview",244);isolated(L"unknownhandler",245);std::cout<<"PASS: "<<checks<<" native-profile core checks; actual typed033d70/AES/XML, guarded provider graph and fifteen isolated retained prefixes\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}


