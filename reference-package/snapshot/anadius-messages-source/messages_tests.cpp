#include "messages.hpp"
#include "control.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <functional>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>
using namespace anadius::messages_recovered;
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
struct FakeEffects final:Effects,UnknownMessageEffects {
    unsigned invalids=0,atoi_calls=0;
    bool handler_result=true,suppress=false,unknown=false,diagnostic_failure=false;
    std::vector<std::string> diagnostics;
    void unknown_message_07bd70(const UnknownMessageInput& input)override{diagnostics.emplace_back(input.view());if(diagnostic_failure)throw UnknownMessageError{};}
    std::vector<std::uint32_t> handlers;
    std::vector<std::string> logs;
    int atoi_05ee08(std::string_view value)override{++atoi_calls;return std::atoi(std::string(value).c_str());}
    bool registered_handler(std::uint32_t rva,const XmlNode& node,XmlPrinter& printer)override{
        handlers.push_back(rva);if(suppress)throw NoResponseRequiredError{};printer.open(node.name());printer.close();return handler_result;
    }
    [[noreturn]]void invalid_message_02bd30_04d228()override{++invalids;if(unknown)throw UnknownMessageError{};throw Invalid{};}
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
static void no_response_tests(){
    for(bool encrypted:{false,true}){
        Fixture f;f.encrypted=encrypted;f.show=1;f.effects.suppress=true;f.queue.values={"queued"};std::string input=encrypted?"41":"valid";
        check(f.process(input).empty());check(f.cipher.trace->encrypted.empty()&&f.queue.trace.empty()&&f.queue.values.size()==1);check(f.effects.logs.size()==1&&f.effects.logs[0].starts_with("received:"));
        f.effects.suppress=false;f.queue.values.clear();input=encrypted?"41":"again";check(f.process(input)==(encrypted?std::string("43\0",3):std::string("R\0",2)));check(f.encrypted==encrypted);
    }
}
static void unknown_tests(){
    for(bool encrypted:{false,true})for(bool show:{false,true}){
        Fixture f;f.encrypted=encrypted;f.show=show;f.effects.unknown=true;f.xml.configure=[](Document& d){d.command.label="Unknown";};f.queue.values={"queued"};
        const std::string plain("original\0tail",13);f.cipher.trace->decrypt_outputs={plain};std::string input=encrypted?"41":plain;
        check(f.process(input).empty());check(f.effects.diagnostics==std::vector<std::string>{plain});
        check(f.cipher.trace->encrypted.empty()&&f.queue.trace.empty()&&f.queue.values.size()==1);check(f.effects.logs.size()==unsigned(show));
        f.xml.configure={};f.queue.values.clear();input=encrypted?"41":"next";check(!f.process(input).empty());
    }
    Fixture f;f.effects.unknown=true;f.effects.diagnostic_failure=true;f.xml.configure=[](Document& d){d.command.label="Unknown";};std::string input="original";
    bool caught=false;try{(void)f.process(input);}catch(const UnknownMessageError&){caught=true;}
    check(caught&&f.effects.diagnostics.size()==1&&f.cipher.trace->encrypted.empty());
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
int main(){codec_tests();reset_tests();dispatch_tests();challenge_tests();process_tests();no_response_tests();unknown_tests();std::printf("PASS: %u checks; synthetic XML/crypto/queue only; no sockets or target execution\n",checks);}
