#include "native_message_test_fixture.hpp"
#include "../anadius-messages-source/control.hpp"
using namespace native_message_test;
namespace {
void shared_event_count_contract(){
    std::array<s::ReceiveBufferString*,8> slots{};
    s::NativeMessageEventQueue queue{slots,8,0,2};
    const auto sampled=queue;
    check(queue.count_20==2&&--queue.count_20==1&&sampled.count_20==2,"Queue aggregate/copy preserves a sampled count while the one shared source cell changes");
    s::NativeMessageEventQueue assigned;assigned=queue;
    check(++queue.count_20==2&&assigned.count_20==1,"Copied queue metadata does not alias the live count cell or become another queue owner");
    queue={slots,8,0,0};
    check(!queue.count_20&&sampled.count_20==2&&assigned.count_20==1,"Aggregate reassignment resets the selected live count without rewriting older value snapshots");
    Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.event("committed");
    f.api.hook=[](std::string name){if(name=="unlock")throw Failure{83};};
    rejects<Failure>([&]{f.run();});
    check(!f.api.queue.count_20&&!f.api.queue.offset_18&&!f.events[0].size_10&&f.events[0].capacity_18==15&&!f.api.destroys,"Actual typed event pop commits shared count/head retirement BEFORE an unknown unlock outcome, without invented document cleanup");
}
void normal(){
    std::vector<unsigned> lengths;for(unsigned n=0;n<35;++n)lengths.push_back(n);for(auto n:{128U,2047U,2048U,4095U,4096U,8192U,150000U})lengths.push_back(n);
    for(bool encrypted:{false,true})for(bool show:{false,true})for(auto n:lengths){
        Fixture f;f.core.encrypted=encrypted;f.core.show=show;const auto request=core_test::request_xml+std::string("\0tail",5);f.seed(encrypted?core_test::hex(request):request);
        std::string payload;for(unsigned i=0;i<n;++i)payload.push_back(static_cast<char>((i*17+31)&255));
        f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,payload);return out;};f.run();
        check(f.output()==framed(payload,encrypted),"typed033d70 full binary response oracle, inline/heap/large boundaries and explicit payload NUL");
        check(f.input()==request&&f.api.decrypts==unsigned(encrypted)&&f.api.encrypts==unsigned(encrypted),"request decoded in actual prefix, cipher lower calls only encrypted branch");
        check(f.api.constructs==1&&f.api.parses==1&&f.api.dispatches==1&&f.api.destroys==1,"single reusable document and native dispatch construction/destruction order");
        check(!f.locals.response.size_10&&f.locals.response.capacity_18==15&&!f.locals.response.storage_00[0]&&!f.core.strings.reads&&!f.core.strings.replaces&&!f.core.strings.constructs,"output move resets local; N5 deferred-writeback adapter is absent");
        check(f.core.effects.logs.size()==(show?2U:0U)&&!f.api.locks&&!f.api.unlocks,"live logging flag and no event lock for empty queue");
    }
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.api.hook=[&](std::string name){if(name=="decrypt")check(f.input()==core_test::request_xml,"decrypt callback sees actual decoded prefix before any semantic writeback");if(name=="dispatch")check(f.input()==core_test::request_xml,"dispatch callback observes same live request");};f.run();check(f.output()==framed(response("alpha"),true),"actual XML reader/032d00 semantic lower provider plus typed033d70/03d190 composition");}
    {Fixture f;f.seed(core_test::request_xml);f.api.codes={8};f.api.errors={0};f.run();check(f.api.dispatches==1&&!f.core.encrypted,"document error cell0, not returned parser code8, selects success");}
    {Fixture f;f.seed(core_test::request_xml);f.core.effects.handling=[&](){f.core.encrypted=1;f.core.show=1;};f.run();check(f.api.encrypts==1&&f.core.effects.logs.size()==1&&f.output()==framed(response("alpha"),true),"live handler mutation controls post-dispatch encryption and response log independently");}
}
void retries(){
    const std::string version="<LSX version=\"\"><Request id=\"bridge\" recipient=\"fixture\"><GetConfig/></Request></LSX>";
    {Fixture f;f.seed(version);f.api.codes={7};f.api.errors={1};f.api.hook=[&](std::string name){if(name=="dispatch")check(f.input()=="<LSX ><Request id=\"bridge\" recipient=\"fixture\"><GetConfig/></Request></LSX>","native034730 erase committed before dispatch callback");};f.run();check(f.api.parses==2&&f.api.constructs==1&&f.api.destroys==1&&!f.core.strings.replaces,"code7 retry reuses document and directly mutates original prefix");}
    {Fixture f;f.seed(core_test::hex(core_test::request_xml));f.run();check(f.core.encrypted==1&&f.api.parses==2&&f.api.decrypts==1&&f.output()==framed(response("alpha"),true),"code8 absent LSX switches live flag and reenters decode/decrypt head");}
    for(const auto& input:{std::string("<LSX>broken"),std::string("bad\0<LSX>",9)}){Fixture f;f.seed(input);f.api.codes={8};f.api.errors={1};f.run();check(!f.core.encrypted&&f.api.parses==1&&f.core.effects.logs.size()==1&&!f.output().size(),"code8 LSX search uses full native size including suffix after embedded NUL");}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(version));f.api.codes={7};f.api.errors={1};f.api.decrypt=[&](auto& in){if(f.api.decrypts==2)f.api.assign(in,core_test::request_xml);return true;};f.run();check(f.api.decrypts==2&&f.api.parses==2,"code7 encrypted retry reenters hex decode and decrypt, not only XML parse");}
    {Fixture f;f.seed("bad XML without version");f.api.codes={7,7};f.api.errors={1,1};f.run();check(f.api.parses==2&&!f.core.encrypted&&!f.output().size()&&f.core.effects.logs.size()==1,"missing version still retries exactly once; repeated7 logs bad XML");}
    {Fixture f;f.seed(std::string("bad\0version=\"\"tail",18));f.api.codes={7,4};f.api.errors={1,1};f.run();check(f.input()==std::string("bad\0tail",8)&&f.api.parses==2,"code7 full-size search erases version after embedded NUL");}
}
void events(){
    for(bool accepted:{false,true})for(bool large:{false,true}){Fixture f;f.core.encrypted=1;f.core.show=1;f.seed(core_test::hex(core_test::request_xml));const auto event=large?std::string(5000,'E'):std::string("event\0tail",10);f.event(event);f.event("second",1);f.api.encrypt=[&](auto& value){return &value==&f.locals.response||accepted;};f.run();
        auto expected=framed(response("alpha"),true);if(accepted)expected+=framed(event,true);
        check(f.output()==expected&&f.api.locks==1&&f.api.unlocks==1&&f.api.queue.count_20==1&&f.api.queue.offset_18==1,"encrypted response appends at most one event with separate payload NULs; failed event still consumed");
        check(!f.events[0].size_10&&f.events[0].capacity_18==15&&!f.events[0].storage_00[0]&&f.api.text(f.events[1])=="second","live front reset and second event retained");
        check(f.core.effects.logs.size()==3&&f.core.effects.logs.back()=="event:"+event,"event log before cipher and full embedded-NUL byte view");
    }
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.event("event");f.api.empty_on_lock=true;f.run();check(f.api.locks==1&&f.api.unlocks==1&&f.api.encrypts==1&&f.events[0].size_10==5,"empty-after-lock recheck skips event copy/encrypt/pop and still unlocks");}
    for(int code:{5,6}){Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.event("event");if(code==5)f.api.lock_result=1;else f.api.mutex.lock_count_4c=INT32_MAX;try{f.run();throw std::runtime_error("expected thread terminal");}catch(const Failure& e){check(e.code==code&&!f.api.unlocks&&!f.api.destroys&&f.api.queue.count_20==1,"lock failure or max-count terminal preserves no invented unlock/pop/document cleanup");if(code==6)check(f.api.mutex.lock_count_4c==INT32_MAX-1,"max mutex count decremented before terminal6");}}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.event("first");f.event("second",1);f.api.encrypt=[&](auto& value){if(&value==&f.locals.temporary){f.api.queue.offset_18=1;return false;}return true;};f.run();check(f.api.text(f.events[0])=="first"&&!f.events[1].size_10&&f.api.queue.count_20==1&&f.api.queue.offset_18==2,"event removal reloads live queue slot after cipher callback rather than cached copied source");}
}
void no_response_control(){
    using Control=anadius::messages_recovered::NoResponseRequiredError;
    for(bool encrypted:{false,true})for(bool show:{false,true}){
        Fixture f;f.core.encrypted=encrypted;f.core.show=show;f.seed(encrypted?core_test::hex(core_test::request_xml):core_test::request_xml);f.event("queued");
        f.api.dispatch=[](auto&)->s::ReceiveBufferString&{throw Control{};};f.run();
        check(f.output().empty()&&f.api.destroys==1&&f.api.dispatches==1&&!f.api.encrypts&&!f.api.locks&&f.api.queue.count_20==1,"known native catch returns unframed empty output, destroys DOM and skips encryption/events");
        check(f.core.effects.logs.size()==unsigned(show)&&(!show||f.core.effects.logs[0].starts_with("received:")),"suppression has received log only, no response/event log");
        check(f.input()==core_test::request_xml&&!f.locals.response.size_10&&f.locals.response.capacity_18==15&&!f.locals.response.storage_00[0],"catch preserves decoded request and resets transferred local response");
    }
    {Fixture f;f.core.encrypted=true;f.seed(core_test::hex(core_test::request_xml));const std::string current(5000,'P');f.api.dispatch=[&](auto&)->s::ReceiveBufferString&{f.api.assign(f.locals.response,current);throw Control{};};f.run();check(f.output()==current&&!f.api.encrypts&&!f.api.locks&&f.api.destroys==1&&!f.locals.response.size_10,"funclet transfers CURRENT response including owned heap bytes with no framing/encryption");}
    {Fixture f;f.core.show=true;f.seed(core_test::request_xml);f.api.hook=[](std::string name){if(name=="log-response")throw Control{};};rejects<Control>([&]{f.run();});check(!f.api.destroys&&f.output().empty(),"same marker from response logging outside native protected interval is not suppressed");}
}
void unknown_control(){
    using Control=m::UnknownMessageError;
    struct Reporter final:Api,m::UnknownMessageEffects {
        std::vector<std::string> inputs;std::function<void(const m::UnknownMessageInput&)> diagnostic;
        Reporter(Fixture& f):Api(f.core,f.locals,f.strings){}
        void unknown_message_07bd70(const m::UnknownMessageInput& input)override{event("unknown-diagnostic");inputs.emplace_back(input.view());if(diagnostic)diagnostic(input);}
    };
    for(bool encrypted:{false,true})for(bool show:{false,true})for(bool large:{false,true}){
        Fixture f;f.core.encrypted=encrypted;f.core.show=show;const auto plain=core_test::request_xml+std::string("\0tail",5);f.seed(encrypted?core_test::hex(plain):plain);f.event("queued");
        Reporter r(f);r.queue=f.api.queue;const std::string current=large?std::string(5000,'P'):std::string{};
        r.dispatch=[&](auto&)->s::ReceiveBufferString&{r.assign(f.locals.response,current);throw Control{};};
        r.diagnostic=[&](const auto&){check(!r.destroys&&f.output().empty()&&r.text(f.locals.response)==current,"Unknown diagnostic precedes current-response transfer and DOM destruction");};
        s::NativeMessageContext context{f.bindings,f.core.show,f.strings,f.mutation,f.locals,r};s::process_native_message_033d70(0x120f0,f.core.response,f.core.request(),context);
        check(r.inputs==std::vector<std::string>{plain}&&f.output()==current&&r.destroys==1&&!f.locals.response.size_10,"Unknown catch borrows original full counted decoded request and transfers current response");
        check(!r.encrypts&&!r.locks&&r.queue.count_20==1&&f.core.effects.logs.size()==unsigned(show),"Unknown diagnostic independent of show flag skips response framing, cipher and event consumption");
    }
    {Fixture f;f.seed(core_test::request_xml);f.api.dispatch=[](auto&)->s::ReceiveBufferString&{throw Control{};};rejects<Control>([&]{f.run();});check(!f.api.destroys&&f.output().empty(),"Missing optional diagnostic provider rethrows without output publication or guessed DOM cleanup");}
    {Fixture f;f.seed(core_test::request_xml);Reporter r(f);r.dispatch=[](auto&)->s::ReceiveBufferString&{throw Control{};};r.diagnostic=[](const auto&){throw Control{};};s::NativeMessageContext c{f.bindings,f.core.show,f.strings,f.mutation,f.locals,r};rejects<Control>([&]{s::process_native_message_033d70(0x120f0,f.core.response,f.core.request(),c);});check(r.inputs.size()==1&&!r.destroys&&f.output().empty(),"Diagnostic same-type failure escapes once without recursively entering catch or cleaning DOM");}
    {Fixture f;f.core.show=true;f.seed(core_test::request_xml);f.api.hook=[](std::string name){if(name=="log-response")throw Control{};};rejects<Control>([&]{f.run();});check(!f.api.destroys&&f.output().empty(),"Unknown marker outside native protected interval remains a failure");}
}
void failures(){
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.api.decrypt=[](auto&){throw Failure{77};return false;};rejects<Failure>([&]{f.run();});check(f.input()==core_test::request_xml&&!f.api.parses&&!f.api.destroys,"decrypt exception retains actual decoded input; no deferred-writeback gap or invented cleanup");}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.api.decrypt=[](auto&){return false;};f.run();check(f.input()==core_test::request_xml&&f.output().empty()&&f.api.destroys==1&&f.core.effects.logs.size()==1,"decrypt false logs decoded bytes and returns exact empty result");}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.api.encrypt=[](auto&){return false;};f.run();check(f.output().empty()&&f.core.response.storage_00==std::array<std::uint8_t,16>{}&&!f.api.locks&&f.api.destroys==1&&f.core.containers.f.m.response.live_blocks()==0,"response cipher false zeroes output16, frees local, skips codec/events and destroys document");}
    {Fixture f;f.seed("version=\"\"x");f.api.codes={7};f.api.errors={1};f.api.hook=[&](std::string name){if(name=="parse"&&f.api.parses==2)throw Failure{78};};rejects<Failure>([&]{f.run();});check(f.input()=="x"&&!f.api.destroys,"parser exception after code7 sees committed native erase");}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.event("event");f.api.hook=[](std::string name){if(name=="encrypt-event")throw Failure{79};};rejects<Failure>([&]{f.run();});check(f.api.queue.count_20==1&&!f.api.unlocks&&!f.api.destroys&&f.api.text(f.events[0])=="event","event callback exception does not pop/unlock/document cleanup on normal-only projection");}
    {Fixture f;f.seed(core_test::request_xml);f.api.dispatch=[&](auto&)->s::ReceiveBufferString&{f.api.assign(f.locals.response,"alias");return f.locals.response;};f.run();check(f.output()==std::string("alias\0",6),"dispatch returned identity equal to response skips move/free branch");}
    {Fixture f;f.seed(core_test::request_xml);f.context.handlers={};rejects<s::UnsupportedInput>([&]{f.run();});check(!f.api.constructs,"unbound projection rejected before target effects");}
    {Fixture f;f.seed(core_test::request_xml);f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,std::string(15,'Q'));return out;};f.api.hook=[](std::string name){if(name=="grow-nul-provider")throw Failure{80};};rejects<Failure>([&]{f.run();});check(f.locals.response.size_10==15&&f.api.nul_grows==1&&!f.api.destroys&&f.output().empty(),"012350 exception retains pre-append local and unpublished output");}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,std::string(5000,'Q'));return out;};f.api.encrypt=[&](auto& value){auto data=s::receive_buffer_address(value);buffer_test::word(f.strings.memory.view(data-8,8),data-40);return false;};rejects<Failure>([&]{f.run();});check(f.output().empty()&&f.core.response.storage_00==std::array<std::uint8_t,16>{}&&f.locals.response.size_10==5000&&!f.api.destroys,"cipher false zeroes output before invalid aligned header terminal; no invented document cleanup");}
    {Fixture f;f.core.encrypted=1;f.seed(core_test::hex(core_test::request_xml));f.event("event");f.api.dispatch=[&](auto& out)->s::ReceiveBufferString&{f.api.assign(out,std::string(1000,'x'));s::resize_receive_buffer_010040(out,1,0,f.strings);f.core.containers.bytes.hook=[](std::string name){if(name=="copy")throw Failure{81};};return out;};rejects<Failure>([&]{f.run();});check(f.locals.response.size_10==14&&f.strings.memory.view(s::receive_buffer_address(f.locals.response)+14,1)[0]=='x'&&f.api.queue.count_20==1&&!f.api.unlocks&&!f.api.destroys,"spare append publishes new size before COPY, final NUL/pop/unlock absent on failure");}
    {Fixture f;f.seed(core_test::request_xml);f.api.hook=[](std::string name){if(name=="destroy-document")throw Failure{82};};rejects<Failure>([&]{f.run();});check(f.output()==framed(response("alpha"),false)&&!f.locals.response.size_10&&f.locals.response.capacity_18==15,"document destructor exception occurs after output move and local reset");}
}
}
int main(){try{shared_event_count_contract();normal();retries();events();no_response_control();unknown_control();failures();std::cout<<"PASS: "<<checks<<" socket native message checks; typed033d70 request/cipher/parser retries, output codec/move, live event front and normal failure prefixes; lower providers/ABI/EH remain open\n";}catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}

