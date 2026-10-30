#pragma once
#include "preloader_l/net/transport_message_core_bridge.hpp"
#include "tests/support/socket/response_containers_test_fixture.hpp"
#include "preloader_l/xml/reader.hpp"
#include <deque>

namespace core_test {
namespace s=preloader_l::socket_recovered;namespace m=preloader_l::messages_recovered;
namespace b=buffer_test;namespace r=receive_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure {int code;};
inline const std::string request_xml="<LSX><Request id=\"bridge\" recipient=\"fixture\"><GetConfig/></Request></LSX>";
inline std::string hex(std::string_view bytes){const char* digits="0123456789abcdef";std::string out;for(unsigned char c:bytes){out+=digits[c>>4];out+=digits[c&15];}return out;}
inline std::string unhex(std::string_view value){auto digit=[](char c){if(c>='0'&&c<='9')return c-'0';if(c>='a'&&c<='f')return c-'a'+10;throw std::runtime_error("oracle invalid hex");};check(value.size()%2==0,"oracle complete hex pairs");std::string out;for(std::size_t i=0;i<value.size();i+=2)out.push_back(static_cast<char>((digit(value[i])<<4)|digit(value[i+1])));return out;}
struct Xml:m::XmlBackend {
    preloader_l::xml_reader_bridge::Backend actual;std::deque<int> forced;std::vector<std::string> inputs;
    m::ParseResult parse_c_string(std::string_view input)override{inputs.emplace_back(input);if(!forced.empty()){const auto code=forced.front();forced.pop_front();auto result=actual.parse_c_string("<LSX>");result.returned_code=code;result.has_error=true;return result;}return actual.parse_c_string(input);}
    std::unique_ptr<m::XmlPrinter> printer()override{return actual.printer();}
};
struct Cipher:m::Cipher {
    unsigned decrypts=0,encrypts=0;bool fail_decrypt=false,fail_encrypt=false;std::function<void(std::string&)> decrypt_hook,encrypt_hook;
    std::vector<std::string> decrypted,encrypted;
    void construct_0423d0_in_place()override{r::unexpected();}void set_fixed_key_084c98()override{r::unexpected();}void set_key_042780(const m::Key&)override{r::unexpected();}
    bool decrypt_042480(std::string& value)override{++decrypts;decrypted.push_back(value);if(decrypt_hook)decrypt_hook(value);return !fail_decrypt;}
    bool encrypt_042610(std::string& value)override{++encrypts;encrypted.push_back(value);if(encrypt_hook)encrypt_hook(value);return !fail_encrypt;}
};
struct Factory:m::CryptoFactory {std::unique_ptr<m::Cipher> create_0423d0()override{r::unexpected();}};
struct Random:m::Random {m::Key challenge_bytes_034cf0()override{r::unexpected();}m::Key key_from_response_034c30(std::string_view)override{r::unexpected();}};
struct Events:m::EventQueue {
    std::deque<std::string> values;std::int32_t count=1;int lock_result=0;unsigned locks=0,pops=0,unlocks=0;bool empty_on_lock=false;
    bool nonempty_relaxed()const override{return !values.empty();}int lock_04a6ac()override{++locks;return lock_result;}
    std::int32_t& lock_count_4c()override{return count;}bool nonempty_locked()const override{return !empty_on_lock&&!values.empty();}
    std::string copy_front_locked()const override{return values.front();}void pop_front_locked()override{++pops;values.pop_front();}void unlock_04a6b4()override{++unlocks;}
};
struct Effects:m::Effects {
    std::string answer="alpha";std::vector<std::string> logs;std::vector<std::uint32_t> commands;bool accept=true;std::function<void()> handling;
    int atoi_05ee08(std::string_view)override{r::unexpected();}
    bool registered_handler(std::uint32_t address,const m::XmlNode&,m::XmlPrinter& printer)override{commands.push_back(address);if(handling)handling();printer.open("BridgeAnswer");printer.attribute("value",answer);printer.close();return accept;}
    [[noreturn]]void invalid_message_02bd30_04d228()override{throw Failure{66};}[[noreturn]]void thread_error_04a648(int code)override{throw Failure{code};}
    void log_received(std::string_view value)override{logs.push_back("received:"+std::string(value));}
    void log_response(std::string_view value)override{logs.push_back("response:"+std::string(value));}
    void log_event(std::string_view value)override{logs.push_back("event:"+std::string(value));}
    void log_bad_xml(std::string_view,std::string_view value)override{logs.push_back("bad:"+std::string(value));}
    void log_decryption_failure(std::string_view value)override{logs.push_back("decrypt-fail:"+std::string(value));}
};
struct Strings:s::BoundedMessageCoreStrings {
    container_test::ContainerFixture& f;explicit Strings(container_test::ContainerFixture& f):BoundedMessageCoreStrings(f.strings),f(f){}
    void construct_response(s::ReceiveBufferString& out,std::string_view bytes)override{f.f.m.seed(f.f.m.response,std::string(bytes));out=f.f.m.response.value;f.bytes.refresh();}
};
struct Spy:s::MessageCoreStrings {
    Strings delegate;unsigned reads=0,replaces=0,constructs=0;std::vector<std::string> trace;std::function<void(std::string)> hook;
    explicit Spy(container_test::ContainerFixture& f):delegate(f){}
    void event(std::string value){trace.push_back(value);if(hook)hook(value);}
    std::string read_request(const s::ReceiveBufferString& in)override{++reads;event("read");return delegate.read_request(in);}
    void replace_request(s::ReceiveBufferString& in,std::string_view value)override{++replaces;event("replace");delegate.replace_request(in,value);}
    void construct_response(s::ReceiveBufferString& out,std::string_view value)override{++constructs;event("construct");delegate.construct_response(out,value);}
};
struct Fixture {
    container_test::ContainerFixture containers;Xml xml;Cipher cipher;Factory factory;Random random;Events events;Effects effects,other_effects;
    std::uint8_t encrypted=0,show=0,other_encrypted=0,other_show=0;
    s::MessageCoreSession session{{encrypted,show,cipher,events},xml,factory,random,effects};
    s::MessageCoreSession other{{other_encrypted,other_show,cipher,events},xml,factory,random,other_effects};
    std::array<s::MessageCoreBinding,2> bindings{{{0x120f0,&session},{0x220f0,&other}}};Spy strings{containers};s::MessageCoreContext context{bindings,strings};
    s::ReceiveBufferString response;
    Fixture(){other_effects.answer="beta";
        // N5 also queues300KiB encrypted payloads followed by heap strings.
        // The older fixture's fixed128KiB stride must not overlap live blocks.
        auto& pool=containers.bytes.pool;pool.api.on_allocate=[&pool](){
            for(const auto& block:pool.blocks)if(block)pool.next_raw=std::max(pool.next_raw,block->raw+block->bytes.size()+0x1000);
        };
    }
    void seed(std::string input){containers.f.m.seed(containers.f.m.request,std::move(input));containers.bytes.refresh();}
    s::ReceiveBufferString& request(){return containers.f.m.request.value;}
    std::string input(){return containers.f.m.text(request());}
    std::string output(){return containers.f.m.text(response);}
    void run(std::uintptr_t id=0x120f0){s::process_message_core(id,response,request(),context);}
};
} // namespace core_test
