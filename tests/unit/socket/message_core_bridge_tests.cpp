#include "tests/support/socket/message_core_bridge_test_fixture.hpp"
using namespace core_test;
namespace {
void normal(){
    for(bool encrypted:{false,true})for(bool show:{false,true})for(bool second:{false,true}){
        Fixture f;const auto original=request_xml+std::string("\0ignored",8);f.encrypted=f.other_encrypted=encrypted;f.show=f.other_show=show;f.seed(encrypted?hex(original):original);f.run(second?0x220f0:0x120f0);
        auto result=f.output();check(!result.empty()&&result.back()==0,"semantic response includes explicit terminating NUL as payload");result.pop_back();if(encrypted)result=unhex(result);
        check(result.find(second?"beta":"alpha")!=std::string::npos&&result.find("BridgeAnswer")!=std::string::npos,"selected identity routes actual dispatcher/serializer to correct session");
        check(f.input()==original&&f.strings.reads==1&&f.strings.constructs==1&&f.strings.replaces==(encrypted?1U:0U),"normal return commits complete changed input including embedded NUL only when changed");
        auto& selected=second?f.other_effects:f.effects;check(selected.commands==std::vector<std::uint32_t>{0x02e5f0}&&selected.logs.size()==(show?2U:0U),"real registration dispatch and live logging state");
        check(f.xml.inputs==std::vector<std::string>{request_xml},"C-string XML boundary ends parsing at embedded NUL");
    }
    {Fixture f;f.seed(request_xml);f.strings.hook=[&](auto event){if(event=="read"){f.bindings[0].session=nullptr;f.show=1;}};f.run();check(f.effects.commands.size()==1&&f.effects.logs.size()==2,"session references selected before read provider; state cells remain live");}
    {Fixture f;f.encrypted=1;f.events.values={std::string("event\0tail",10),"next"};f.seed(hex(request_xml));f.run();auto output=f.output();const auto split=output.find('\0');check(split!=std::string::npos&&unhex(std::string_view(output).substr(0,split)).find("BridgeAnswer")!=std::string::npos,"encrypted response independently decodes");check(output.substr(split+1)==hex(std::string("event\0tail",10))+std::string("\0",1)&&f.events.pops==1&&f.events.values.front()=="next"&&f.events.unlocks==1,"exact one event concatenated with separate payload NUL and consumed");}
    {Fixture f;f.xml.forced={8};f.seed(hex(request_xml));f.run();check(f.encrypted==1&&f.cipher.decrypts==1&&f.input()==request_xml&&f.strings.replaces==1,"code8 retry flips live encryption state and commits decoded request on return");}
    {Fixture f;f.xml.forced={7};const auto input="<LSX version=\"\"><Request id=\"bridge\" recipient=\"fixture\"><GetConfig/></Request></LSX>";f.seed(input);f.run();check(f.input().find("version=\"\"")==std::string::npos&&f.xml.inputs.size()==2&&f.strings.replaces==1,"code7 removes10-byte version marker and retries then commits");}
    {Fixture f;f.encrypted=1;f.cipher.fail_decrypt=true;f.seed(hex(request_xml));f.run();check(f.output().empty()&&f.input()==request_xml&&f.effects.logs.size()==1&&f.strings.replaces==1,"normal decryption failure still commits decoded input and constructs empty response");}
    {Fixture f;f.encrypted=1;f.cipher.fail_encrypt=true;f.seed(hex(request_xml));f.run();check(f.output().empty()&&f.input()==request_xml&&f.strings.constructs==1,"response encryption failure returns owned empty response");}
    {Fixture f;f.seed("<LSX>");f.run();check(f.output().empty()&&f.effects.logs.size()==1&&!f.strings.replaces,"bad XML returns empty with original request retained");}
}
void failures(){
    {Fixture f;f.seed(request_xml);rejects<s::UnsupportedInput>([&]{f.run(0xbad);});check(!f.strings.reads&&!f.strings.constructs,"unknown identity rejected before reading input");}
    {Fixture f;f.seed(request_xml);f.bindings[1].handler_f0=0x120f0;rejects<s::UnsupportedInput>([&]{f.run();});check(!f.strings.reads,"duplicate matching identity rejected rather than first match");}
    {Fixture f;f.bindings[0].session=nullptr;rejects<s::UnsupportedInput>([&]{f.run();});rejects<s::UnsupportedInput>([&]{s::process_message_core(0x120f0,f.request(),f.request(),f.context);});check(!f.strings.reads,"null state and aliased prefixes rejected before effects");}
    {Fixture f;f.seed(request_xml);f.effects.accept=false;f.response.size_10=7;try{f.run();r::unexpected();}catch(const Failure& e){check(e.code==66&&!f.strings.constructs&&!f.strings.replaces&&f.response.size_10==7,"handler terminal propagates without fabricated response/commit");}}
    {Fixture f;f.xml.forced={8};const auto wire=hex(request_xml);f.seed(wire);f.cipher.decrypt_hook=[](auto& value){value="partial";throw Failure{77};};try{f.run();r::unexpected();}catch(const Failure& e){check(e.code==77&&f.encrypted==1&&f.input()==wire&&!f.strings.replaces&&!f.strings.constructs,"LIMIT: semantic exception keeps original request prefix while referenced state mutation survives");}}
    {Fixture f;f.encrypted=1;const auto wire=hex(request_xml);f.seed(wire);f.effects.handling=[&](){check(f.input()==wire,"LIMIT: native request commit deferred until semantic return, callback sees original prefix");};f.run();check(f.input()==request_xml,"normal-return mutation visible after bridge completes");}
    for(const auto& phase:{std::string("read"),std::string("replace"),std::string("construct")}){Fixture f;f.encrypted=1;const auto wire=hex(request_xml);f.seed(wire);f.response.size_10=9;f.strings.hook=[&](auto event){if(event==phase)throw Failure{88};};rejects<Failure>([&]{f.run();});check(f.response.size_10==9&&f.input()==(phase=="construct"?request_xml:wire),"provider failure preserves exact selected adapter prefix without extra cleanup");}
    {Fixture f;f.seed(request_xml);f.encrypted=1;f.events.values={"event"};f.events.lock_result=1;f.seed(hex(request_xml));try{f.run();r::unexpected();}catch(const Failure& e){check(e.code==5&&!f.events.unlocks&&!f.strings.replaces&&!f.strings.constructs,"semantic lock terminal no invented unlock or string exception commit");}}
}
void strings(){
    for(std::size_t size:{0U,1U,15U,16U,100U,4095U}){container_test::Fixture f;f.seed_string(7,15);
        struct Bounded final:s::BoundedMessageCoreStrings {using BoundedMessageCoreStrings::BoundedMessageCoreStrings;void construct_response(s::ReceiveBufferString&,std::string_view)override{r::unexpected();}} api(f.strings);
        std::string expected(size,'Q');if(size>2)expected[size/2]=0;api.replace_request(f.bytes.pool.value,expected);check(api.read_request(f.bytes.pool.value)==expected,"bounded compatibility writeback uses actual010040/008350 with exact bytes/NUL");check(f.bytes.memory.view(s::receive_buffer_address(f.bytes.pool.value)+size,1)[0]==0,"writeback maintains trailing storage NUL beyond payload");}
    {Fixture f;f.request().size_10=17;f.request().capacity_18=15;rejects<s::UnsupportedInput>([&]{f.run();});check(!f.strings.constructs,"out-of-prefix inline request is bounded failure");}
    {Fixture f;f.seed(request_xml);for(const auto& item:f.containers.bytes.regions)if(item.address&&item.address<=s::receive_buffer_address(f.request())&&s::receive_buffer_address(f.request())-item.address<item.bytes.size()){f.containers.bytes.regions[255]=item;break;}rejects<s::UnsupportedInput>([&]{f.run();});check(!f.strings.constructs,"ambiguous heap request mapping rejected");}
}
}
int main(){try{normal();failures();strings();std::cout<<"PASS: "<<checks<<" socket message core bridge checks; actual existing033d70/router/XML and010040/008350, normal-return request commit, identity/state binding and explicit exception/callback visibility limits;0 new native bodies\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unexpected terminal\n";return 1;}}
