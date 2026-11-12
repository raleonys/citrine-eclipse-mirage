// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/protocol/messages.hpp"
#include "citrine_eclipse_mirage/codec/crypto.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>
using namespace citrine_eclipse_mirage::messages;
static unsigned checks;
static void check(bool v){++checks;if(!v)throw std::runtime_error("check "+std::to_string(checks));}
struct Invalid {};
struct ConstructionFailure {};
struct ThreadError {int code;};
struct Node final:XmlNode {
    std::string label;
    std::map<std::string,std::string,std::less<>> attributes;
    const Node* first=nullptr;
    const Node* next=nullptr;
    explicit Node(std::string value):label(std::move(value)){}
    std::string_view name()const override{return label;}
    const XmlNode* first_element()const override{return first;}
    const XmlNode* next_element()const override{return next;}
    std::optional<std::string_view> attribute(std::string_view name)const override{
        auto found=attributes.find(name);
        return found==attributes.end()?std::nullopt:std::optional<std::string_view>{found->second};
    }
};
struct Document final:XmlDocument {
    Node root{"LSX"},request{"Request"},command{"GetConfig"};
    bool has_root=true;
    Document(){root.first=&request;request.first=&command;request.attributes={{"id","synthetic-id"},{"recipient","synthetic-service"}};}
    const XmlNode* first_element()const override{return has_root?&root:nullptr;}
    std::string_view error_name()const override{return "synthetic parse error";}
};
struct PrintTrace {std::vector<std::string> operations;std::string buffer=std::string("R\0",2);};
struct Printer final:XmlPrinter {
    std::shared_ptr<PrintTrace> trace;
    explicit Printer(std::shared_ptr<PrintTrace> value):trace(std::move(value)){}
    void open(std::string_view name)override{trace->operations.push_back("open:"+std::string(name));}
    void attribute(std::string_view name,std::string_view value)override{trace->operations.push_back("attr:"+std::string(name)+"="+std::string(value));}
    void close()override{trace->operations.push_back("close");}
    std::string buffer_including_terminator()const override{return trace->buffer;}
};
struct Xml final:XmlBackend {
    struct Plan{int code=0;bool has_error=false;};
    std::deque<Plan> plans;
    std::vector<std::string> parsed;
    std::vector<std::shared_ptr<PrintTrace>> printed;
    std::function<void(Document&)> configure;
    ParseResult parse_c_string(std::string_view value)override{
        parsed.emplace_back(value);auto doc=std::make_unique<Document>();if(configure)configure(*doc);
        Plan plan;if(!plans.empty()){plan=plans.front();plans.pop_front();}
        return {std::move(doc),plan.code,plan.has_error};
    }
    std::unique_ptr<XmlPrinter> printer()override{
        auto trace=std::make_shared<PrintTrace>();printed.push_back(trace);return std::make_unique<Printer>(trace);
    }
};
struct CipherTrace {
    std::vector<std::string> encrypted,decrypted;
    unsigned fixed=0,key=0,constructed=0;
    bool fail_construction=false;
    Key last_key{};
    std::deque<bool> encrypt_results,decrypt_results;
    std::deque<std::string> decrypt_outputs;
    std::string encrypt_output="C";
};
struct FakeCipher final:Cipher {
    std::shared_ptr<CipherTrace> trace;
    explicit FakeCipher(std::shared_ptr<CipherTrace> value=std::make_shared<CipherTrace>()):trace(std::move(value)){}
    void construct_0423d0_in_place()override{++trace->constructed;if(trace->fail_construction)throw ConstructionFailure{};}
    void set_fixed_key_084c98()override{++trace->fixed;}
    void set_key_042780(const Key& key)override{++trace->key;trace->last_key=key;}
    bool decrypt_042480(std::string& value)override{
        trace->decrypted.push_back(value);bool result=true;
        if(!trace->decrypt_results.empty()){result=trace->decrypt_results.front();trace->decrypt_results.pop_front();}
        if(!trace->decrypt_outputs.empty()){value=trace->decrypt_outputs.front();trace->decrypt_outputs.pop_front();}
        return result;
    }
    bool encrypt_042610(std::string& value)override{
        trace->encrypted.push_back(value);value=trace->encrypt_output;bool result=true;
        if(!trace->encrypt_results.empty()){result=trace->encrypt_results.front();trace->encrypt_results.pop_front();}
        return result;
    }
};
struct Factory final:CryptoFactory {
    std::shared_ptr<CipherTrace> trace=std::make_shared<CipherTrace>();
    unsigned creates=0;
    std::unique_ptr<Cipher> create_0423d0()override{++creates;return std::make_unique<FakeCipher>(trace);}
};
struct FakeRandom final:Random {
    unsigned challenges=0,derivations=0;
    std::string received;
    Key challenge_bytes_034cf0()override{++challenges;Key k{};for(unsigned i=0;i<16;++i)k[i]=static_cast<std::uint8_t>(i);return k;}
    Key key_from_response_034c30(std::string_view response)override{++derivations;received=response;Key k{};k.fill(42);return k;}
};
struct FakeEffects final:Effects {
    unsigned invalids=0,atoi_calls=0;
    bool handler_result=true,no_response=false,unexpected_failure=false;
    std::vector<std::uint32_t> handlers;
    std::vector<std::string> logs;
    int atoi_05ee08(std::string_view value)override{++atoi_calls;return std::atoi(std::string(value).c_str());}
    bool registered_handler(std::uint32_t rva,const XmlNode& node,XmlPrinter& printer)override{
        handlers.push_back(rva);
        if(no_response)throw NoResponseRequiredError{};
        if(unexpected_failure)throw std::runtime_error("controlled handler failure");
        printer.open(node.name());printer.close();return handler_result;
    }
    [[noreturn]]void invalid_message_02bd30_04d228()override{++invalids;throw Invalid{};}
    [[noreturn]]void thread_error_04a648(int code)override{throw ThreadError{code};}
    void log_received(std::string_view value)override{logs.push_back("received:"+std::string(value));}
    void log_response(std::string_view value)override{logs.push_back("response:"+std::string(value));}
    void log_event(std::string_view value)override{logs.push_back("event:"+std::string(value));}
    void log_bad_xml(std::string_view error,std::string_view value)override{logs.push_back("bad:"+std::string(error)+":"+std::string(value));}
    void log_decryption_failure(std::string_view value)override{logs.push_back("decrypt-fail:"+std::string(value));}
};
struct Queue final:EventQueue {
    std::deque<std::string> values;
    mutable std::vector<std::string> trace;
    int lock_result=0;
    std::int32_t count=1;
    bool empty_on_lock=false;
    bool nonempty_relaxed()const override{trace.push_back("relaxed");return !values.empty();}
    int lock_04a6ac()override{trace.push_back("lock");return lock_result;}
    std::int32_t& lock_count_4c()override{trace.push_back("count");return count;}
    bool nonempty_locked()const override{trace.push_back("locked");return !empty_on_lock&&!values.empty();}
    std::string copy_front_locked()const override{trace.push_back("copy");return values.front();}
    void pop_front_locked()override{trace.push_back("pop");values.pop_front();}
    void unlock_04a6b4()override{trace.push_back("unlock");}
};
struct Fixture {
    Xml xml;Factory factory;FakeRandom random;FakeEffects effects;FakeCipher cipher;Queue queue;
    std::uint8_t encrypted=0,show=0;
    State state(){return {encrypted,show,cipher,queue};}
    std::string process(std::string& input){return process_message_033d70(state(),input,xml,factory,random,effects);}
    std::string dispatch(const Document& doc){return dispatch_032d00(doc,cipher,xml,factory,random,effects);}
};

static void codec_tests(){
    for(unsigned i=0;i<256;++i){
        std::string value(1,static_cast<char>(i));hex_encode_03d190(value);
        check(value.size()==2);hex_decode_03ce30(value);check(value==std::string(1,static_cast<char>(i)));
    }
    std::string mixed="aF00Z!9";hex_decode_03ce30(mixed);check(mixed==std::string("\xaf\0\0",3));
    std::string empty;hex_encode_03d190(empty);hex_decode_03ce30(empty);check(empty.empty());
}
static void reset_tests(){
    FakeCipher cipher;std::uint8_t encrypted=5,unknown=7;
    check(&reset_02bc70(cipher,encrypted,unknown)==&cipher);
    check(cipher.trace->constructed==1&&encrypted==0&&unknown==0);
    cipher.trace->fail_construction=true;encrypted=8;unknown=9;bool threw=false;
    try{(void)reset_02bc70(cipher,encrypted,unknown);}catch(ConstructionFailure&){threw=true;}
    check(threw);check(encrypted==8&&unknown==9);check(cipher.trace->constructed==2);
}
static void dispatch_tests(){
    check(registrations_032d00().size()==39);
    for(const auto& entry:registrations_032d00()){
        Fixture f;Document doc;doc.command.label=entry.name;
        check(f.dispatch(doc)=="R");check(f.effects.handlers==std::vector<std::uint32_t>{entry.rva});
        check(f.xml.printed[0]->operations[0]=="open:LSX");
        check(f.xml.printed[0]->operations[2]=="attr:id=synthetic-id");
        check(f.xml.printed[0]->operations[3]=="attr:sender=synthetic-service");
    }
    for(int invalid=0;invalid<10;++invalid){
        Fixture f;Document doc;
        switch(invalid){
            case 0:doc.has_root=false;break;case 1:doc.root.label="LSXextra";break;
            case 2:doc.root.first=nullptr;break;case 3:doc.request.label="request";break;
            case 4:doc.request.next=&doc.command;break;case 5:doc.request.attributes.erase("id");break;
            case 6:doc.request.attributes.erase("recipient");break;case 7:doc.request.first=nullptr;break;
            case 8:doc.command.label="unknown";break;case 9:f.effects.handler_result=false;break;
        }
        bool threw=false;try{(void)f.dispatch(doc);}catch(Invalid&){threw=true;}
        check(threw && f.effects.invalids==1);
    }
    Fixture f;Document doc;doc.request.attributes={{"id",""},{"recipient",""}};
    doc.root.next=&doc.command;doc.command.next=&doc.root;
    check(f.dispatch(doc)=="R"); // empty attributes and additional command siblings accepted
}
static void multi_command_request_contract_tests(){
    // C21-RISK-001: retain observed first-child-only dispatch. This tests the
    // real pugixml-backed document tree without inventing a Consumer acceptance rule.
    citrine_eclipse_mirage::xml_reader_bridge::Backend reader;
    struct Accepted {const char* xml;std::string_view first_name;std::uint32_t rva;};
    const Accepted accepted[]={
        {R"XML(<LSX><Request id="multi" recipient="service"><GetConfig/><GetProfile/></Request></LSX>)XML",
            "GetConfig",0x02e5f0},
        {R"XML(<LSX><Request id="multi" recipient="service"><GetProfile/><UnknownCommand/></Request></LSX>)XML",
            "GetProfile",0x02f530},
        {R"XML(<LSX><Request id="multi" recipient="service"><GetConfig/><ChallengeResponse key="unused"/></Request></LSX>)XML",
            "GetConfig",0x02e5f0},
        {R"XML(<LSX><Request id="multi" recipient="service"><GetAuthCode/><GetConfig/><GetProfile/></Request></LSX>)XML",
            "GetAuthCode",0x02e3b0}
    };
    for(const auto& sample:accepted){
        auto parsed=reader.parse_c_string(sample.xml);
        check(!parsed.has_error&&parsed.returned_code==0&&parsed.document!=nullptr);
        Fixture f;
        check(dispatch_032d00(*parsed.document,f.cipher,f.xml,f.factory,f.random,f.effects)=="R");
        check(f.effects.handlers==std::vector<std::uint32_t>{sample.rva});
        check(f.effects.invalids==0&&f.xml.printed.size()==1);
        const auto& operations=f.xml.printed.front()->operations;
        check(std::count(operations.begin(),operations.end(),
                         "open:"+std::string(sample.first_name))==1);
        check(operations[2]=="attr:id=multi"&&operations[3]=="attr:sender=service");
        // A trailing ChallengeResponse cannot mutate cipher/key state.
        check(f.factory.creates==0&&f.cipher.trace->key==0&&f.cipher.trace->fixed==0);
    }
    for(const auto* input:{
        R"XML(<LSX><Request id="multi" recipient="service"><UnknownCommand/><GetConfig/></Request></LSX>)XML",
        R"XML(<LSX><Request id="multi" recipient="service"/></LSX>)XML",
        R"XML(<LSX><Request id="multi" recipient="service"><GetConfig/></Request><Request id="second" recipient="service"><GetProfile/></Request></LSX>)XML",
        R"XML(<LSX><Request id="multi" recipient="service"><GetConfig/></Request><Other/></LSX>)XML"
    }){
        auto parsed=reader.parse_c_string(input);
        check(!parsed.has_error&&parsed.returned_code==0&&parsed.document!=nullptr);
        Fixture f;
        bool rejected=false;
        try{(void)dispatch_032d00(*parsed.document,f.cipher,f.xml,f.factory,f.random,f.effects);}
        catch(const Invalid&){rejected=true;}
        check(rejected&&f.effects.invalids==1&&f.effects.handlers.empty());
        check(f.xml.printed.empty()&&f.factory.creates==0);
    }
}
static void challenge_tests(){
    Fixture initial;
    check(initial_message_02d540(initial.xml,initial.random,"synthetic-version")==std::string("R\0",2));
    check(initial.random.challenges==1);
    check(initial.xml.printed[0]->operations==std::vector<std::string>{"open:LSX","open:Event","attr:sender=EALS","open:Challenge","attr:version=synthetic-version","attr:build=release","attr:key=000102030405060708090a0b0c0d0e0f","close","close","close"});
    for(const auto& version:std::vector<std::optional<std::string>>{std::nullopt,"3","2","0","-1","bad",""}){
        Fixture f;Document doc;doc.command.label="ChallengeResponse";
        doc.command.attributes["key"]="synthetic-challenge";
        if(version)doc.command.attributes["version"]=*version;
        f.factory.trace->encrypt_results={false}; // ignored by original challenge handler
        check(f.dispatch(doc)=="R");check(f.factory.creates==1&&f.factory.trace->fixed==1);
        check(f.factory.trace->encrypted==std::vector<std::string>{"synthetic-challenge"});
        check(std::find(f.xml.printed[0]->operations.begin(),f.xml.printed[0]->operations.end(),"attr:response=43")!=f.xml.printed[0]->operations.end());
        const bool derived=!version||*version=="3";
        check(f.random.derivations==(derived?1u:0u));
        check(f.cipher.trace->key==(derived?1u:0u)&&f.cipher.trace->fixed==(derived?0u:1u));
        check(f.effects.atoi_calls==(version?1u:0u));
        if(derived){check(f.random.received=="43");check(f.cipher.trace->last_key[0]==42);}
    }
    Fixture f;Document doc;doc.command.label="ChallengeResponse";
    bool threw=false;try{(void)f.dispatch(doc);}catch(Invalid&){threw=true;}
    check(threw&&f.factory.creates==0);
}
static void process_tests(){
    {
        Fixture f;f.show=1;f.queue.values={"event"};std::string input("valid\0ignored",13);
        check(f.process(input)==std::string("R\0",2));check(f.xml.parsed==std::vector<std::string>{"valid"});
        check(f.queue.trace.empty()&&f.queue.values.size()==1);check(f.effects.logs.size()==2);
    }
    for(bool event_success:{false,true}){
        Fixture f;f.encrypted=true;f.show=1;f.queue.values={"first-event","second-event"};
        f.cipher.trace->decrypt_outputs={"parsed"};f.cipher.trace->encrypt_results={true,event_success};
        std::string input="41";auto out=f.process(input);
        check(out==(event_success?std::string("43\0" "43\0",6):std::string("43\0",3)));
        check(input=="parsed"&&f.cipher.trace->decrypted==std::vector<std::string>{"A"});
        check(f.cipher.trace->encrypted==std::vector<std::string>{"R","first-event"});
        check(f.queue.values==std::deque<std::string>{"second-event"});
        check(f.queue.trace==std::vector<std::string>{"relaxed","lock","count","locked","copy","pop","unlock"});
        check(f.effects.logs==std::vector<std::string>{"received:parsed","response:R","event:first-event"});
    }
    {
        Fixture f;f.encrypted=true;f.cipher.trace->encrypt_results={false};f.queue.values={"event"};
        std::string input="41";check(f.process(input).empty());check(f.queue.trace.empty()&&f.queue.values.size()==1);
    }
    {
        Fixture f;f.encrypted=true;f.queue.values={"event"};f.queue.empty_on_lock=true;
        std::string input="41";check(f.process(input)==std::string("43\0",3));
        check(f.queue.trace==std::vector<std::string>{"relaxed","lock","count","locked","unlock"});
    }
    for(int error:{5,6}){
        Fixture f;f.encrypted=true;f.queue.values={"event"};
        if(error==5)f.queue.lock_result=1;else f.queue.count=std::numeric_limits<std::int32_t>::max();
        std::string input="41";bool threw=false;
        try{(void)f.process(input);}catch(const ThreadError& e){threw=true;check(e.code==error);}
        check(threw);check(std::find(f.queue.trace.begin(),f.queue.trace.end(),"unlock")==f.queue.trace.end());
        if(error==6)check(f.queue.count==std::numeric_limits<std::int32_t>::max()-1);
    }
    {
        Fixture f;f.encrypted=true;f.cipher.trace->decrypt_results={false};std::string input="41";
        check(f.process(input).empty());check(f.xml.parsed.empty());
        check(f.effects.logs==std::vector<std::string>{"decrypt-fail:A"});
    }
    {
        Fixture f;f.xml.plans={{8,true},{0,false}};f.cipher.trace->decrypt_outputs={"decoded"};std::string input="41";
        check(f.process(input)==std::string("43\0",3));check(f.encrypted);
        check(f.xml.parsed==std::vector<std::string>{"41","decoded"});
    }
    {
        Fixture f;f.xml.plans={{8,true}};std::string input="<LSX>invalid";
        check(f.process(input).empty());check(!f.encrypted&&f.xml.parsed.size()==1&&f.effects.logs.size()==1);
    }
    for(bool contains_version:{false,true}){
        Fixture f;f.xml.plans={{7,true},{7,true}};
        std::string input=contains_version?"version=\"\" version=\"\"":"invalid";
        check(f.process(input).empty());check(f.xml.parsed.size()==2);
        check(input==(contains_version?" version=\"\"":"invalid"));check(f.effects.logs.size()==1);
    }
    {
        Fixture f;f.encrypted=true;f.xml.plans={{7,true},{0,false}};
        f.cipher.trace->decrypt_outputs={"version=\"\"","second"};std::string input="41";
        check(f.process(input)==std::string("43\0",3));check(f.cipher.trace->decrypted==std::vector<std::string>{"A",""});
        check(f.xml.parsed==std::vector<std::string>{"version=\"\"","second"});
    }
    {
        Fixture f;f.xml.plans={{8,false}};std::string input="synthetic";
        check(f.process(input)==std::string("R\0",2));check(!f.encrypted); // error field controls success
    }
}
// A11-09: a syntactically parsed but invalid LSX Request is rejected by
// the full message core before any response encryption or event dequeue.
// This pins CURRENT product behavior, not donor exception compatibility.
static void process_invalid_dispatch_boundaries(){
    for(const bool encrypted:{false,true}){
        for(unsigned invalid=0;invalid<4;++invalid){
            Fixture f;
            f.encrypted=encrypted;
            f.show=1;
            f.queue.values={"retained-event"};
            switch(invalid){
                case 0:f.xml.configure=[](Document& d){d.root.label="OtherRoot";};break;
                case 1:f.xml.configure=[](Document& d){d.request.attributes.erase("id");};break;
                case 2:f.xml.configure=[](Document& d){d.command.label="UnregisteredCommand";};break;
                case 3:f.effects.handler_result=false;break;
            }
            std::string input=encrypted?"41":"validly-parsed-fixture";
            if(encrypted)f.cipher.trace->decrypt_outputs={"decrypted-request"};
            bool rejected=false;
            try{(void)f.process(input);}catch(const Invalid&){rejected=true;}
            check(rejected&&f.effects.invalids==1);
            check(f.xml.parsed.size()==1&&f.effects.logs.size()==1&&
                  f.effects.logs[0].starts_with("received:"));
            check(f.effects.handlers.size()==(invalid==3?1u:0u));
            check(f.cipher.trace->encrypted.empty()&&f.queue.trace.empty());
            check(f.queue.values==std::deque<std::string>{"retained-event"});
            // An independently valid next request must still use the same
            // cipher and queue, with no replayed error or prematurely lost event.
            f.xml.configure={};
            f.effects.handler_result=true;
            std::string followup=encrypted?"41":"followup-valid";
            const auto result=f.process(followup);
            if(encrypted) {
                check(result==std::string("43\0" "43\0",6));
                check(f.queue.values.empty());
                check(f.queue.trace==std::vector<std::string>{"relaxed","lock","count","locked","copy","pop","unlock"});
            } else {
                check(result==std::string("R\0",2));
                check(f.queue.values==std::deque<std::string>{"retained-event"}&&f.queue.trace.empty());
            }
            check(f.effects.invalids==1&&f.effects.handlers.size()==(invalid==3?2u:1u));
            check(f.effects.logs.size()==(encrypted?4u:3u));
        }
    }
}
static void encrypted_rekey_boundary(){
    namespace crypto=citrine_eclipse_mirage::crypto;
    struct NoClock final:crypto::TimeSource {
        std::int64_t unix_seconds_05eeb8()override{throw std::runtime_error("rekey must not query time");}
    } time;
    crypto::Cipher server,client;server.set_fixed_key_084c98();client.set_fixed_key_084c98();
    crypto::CryptoFactory factory;crypto::Random random{time};
    citrine_eclipse_mirage::xml_reader_bridge::Backend xml;
    Queue queue;FakeEffects effects;std::uint8_t encrypted=1,show=0;
    auto decode=[&](std::string wire,crypto::Cipher& cipher){
        std::vector<std::string> frames;std::size_t begin=0;
        while(begin<wire.size()){
            const auto end=wire.find('\0',begin);check(end!=std::string::npos);
            auto frame=wire.substr(begin,end-begin);hex_decode_03ce30(frame);
            check(cipher.decrypt_042480(frame));frames.push_back(std::move(frame));begin=end+1;
        }
        return frames;
    };
    auto exchange=[&](std::string command){
        check(client.encrypt_042610(command));hex_encode_03d190(command);
        return process_message_033d70({encrypted,show,server,queue},command,xml,factory,random,effects);
    };
    for(const int stage:{0,1,2}){
        const int version=stage==2?2:3;
        const auto id="live-rekey-"+std::to_string(stage);
        const auto key=stage==0?"live-rekey-first":stage==1?"live-rekey-second":"live-rekey-fixed";
        std::string answer(key);crypto::Cipher fixed;fixed.set_fixed_key_084c98();
        check(fixed.encrypt_042610(answer));hex_encode_03d190(answer);
        crypto::Cipher next;if(version>2)next.set_key_042780(random.key_from_response_034c30(answer));else next.set_fixed_key_084c98();
        check(next.round_keys()!=client.round_keys()); // Different text alone is not a key oracle.
        queue.values={"event-after-"+id};
        const auto wire=exchange("<LSX><Request id='"+id+"' recipient='EALS'><ChallengeResponse key='"+key+"' version='"+std::to_string(version)+"'/></Request></LSX>");
        client=next; // Request used the old key; its response/event use the new key.
        const auto frames=decode(wire,client);check(frames.size()==2);
        auto parsed=xml.parse_c_string(frames[0]);check(!parsed.has_error);
        const auto* root=parsed.document->first_element();check(root&&root->name()=="LSX");
        const auto* response=root->first_element();
        check(response&&response->name()=="Response"&&response->attribute("id")==id);
        const auto* accepted=response->first_element();
        check(accepted&&accepted->name()=="ChallengeAccepted"&&accepted->attribute("response")==answer);
        check(frames[1]=="event-after-"+id&&queue.values.empty()&&encrypted==1);
        const auto followup=decode(exchange("<LSX><Request id='after-"+id+"' recipient='EALS'><GetConfig/></Request></LSX>"),client);
        check(followup.size()==1&&followup[0].find("id=\"after-"+id+"\"")!=std::string::npos);
        check(server.round_keys()==client.round_keys()&&effects.invalids==0);
    }
}
static void no_response_continuation_boundary(){
    for(const bool encrypted:{false,true}) {
        Fixture f;f.encrypted=encrypted;f.show=1;f.queue.values={"retained-event"};
        f.effects.no_response=true;
        std::string input=encrypted?"41":"recognized-no-response";
        if(encrypted)f.cipher.trace->decrypt_outputs={"recognized-no-response"};
        EventDelivery receipt;auto state=f.state();state.delivery=&receipt;
        check(process_message_033d70(state,input,f.xml,f.factory,f.random,f.effects).empty());
        check(f.encrypted==encrypted && f.effects.handlers.size()==1 && f.effects.invalids==0);
        check(f.cipher.trace->encrypted.empty() && f.queue.trace.empty() && !receipt.pending());
        check(f.queue.values==std::deque<std::string>{"retained-event"});
        check(f.effects.logs==std::vector<std::string>{"received:recognized-no-response"});
        // Same state remains usable; only a later successful encrypted response
        // can select the event. Plaintext retains the existing queue policy.
        f.effects.no_response=false;std::string next=encrypted?"41":"followup";
        const auto reply=f.process(next);
        check(reply==(encrypted?std::string("43\0" "43\0",6):std::string("R\0",2)));
        check(f.queue.values.size()==(encrypted?0u:1u));
        check(f.effects.handlers.size()==2 && f.effects.invalids==0);
    }
    Fixture f;f.effects.unexpected_failure=true;f.queue.values={"retained-event"};
    std::string input="unexpected";bool propagated=false;
    try{(void)f.process(input);}catch(const std::runtime_error& e){propagated=std::string_view(e.what())=="controlled handler failure";}
    check(propagated && f.queue.trace.empty() && f.queue.values.size()==1);
}

int main(){codec_tests();reset_tests();dispatch_tests();multi_command_request_contract_tests();challenge_tests();process_tests();process_invalid_dispatch_boundaries();encrypted_rekey_boundary();no_response_continuation_boundary();std::printf("PASS: %u checks; real and synthetic XML; actual AES rekey and controlled crypto/queue; no sockets or target execution\n",checks);}
