#include "handler.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace msg=anadius::messages_recovered;
namespace bridge=anadius::session_bridge;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected handler test dependency");}
struct Invalid{};
struct Events final:msg::EventQueue {
    bool nonempty_relaxed()const override{return false;}
    int lock_04a6ac()override{unused();}
    std::int32_t& lock_count_4c()override{unused();}
    bool nonempty_locked()const override{unused();}
    std::string copy_front_locked()const override{unused();}
    void pop_front_locked()override{unused();}
    void unlock_04a6b4()override{unused();}
};
struct Cipher final:msg::Cipher {
    unsigned resets=0;bool throw_encrypt=false;std::string decrypted;
    void construct_0423d0_in_place()override{++resets;}
    void set_fixed_key_084c98()override{unused();}
    void set_key_042780(const msg::Key&)override{unused();}
    bool decrypt_042480(std::string& input)override{decrypted=input;input="<LSX><Request id='7' recipient='EbisuSDK'><GetConfig/></Request></LSX>";return true;}
    bool encrypt_042610(std::string&)override{if(throw_encrypt)throw std::runtime_error("cipher");return true;}
};
struct Factory final:msg::CryptoFactory {std::unique_ptr<msg::Cipher> create_0423d0()override{unused();}};
struct Random final:msg::Random {
    unsigned calls=0;
    msg::Key challenge_bytes_034cf0()override{++calls;msg::Key k{};for(unsigned i=0;i<16;++i)k[i]=static_cast<std::uint8_t>(i);return k;}
    msg::Key key_from_response_034c30(std::string_view)override{unused();}
};
struct Effects final:msg::Effects {
    unsigned handlers=0,received=0,responses=0,bad=0;bool throw_handler=false;
    int atoi_05ee08(std::string_view)override{unused();}
    bool registered_handler(std::uint32_t rva,const msg::XmlNode&,msg::XmlPrinter& p)override{require(rva==0x02e5f0,"exact GetConfig route");++handlers;if(throw_handler)throw std::runtime_error("handler");p.open("ok");p.close();return true;}
    [[noreturn]]void invalid_message_02bd30_04d228()override{throw Invalid{};}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{++received;}
    void log_response(std::string_view)override{++responses;}
    void log_event(std::string_view)override{unused();}
    void log_bad_xml(std::string_view,std::string_view)override{++bad;}
    void log_decryption_failure(std::string_view)override{unused();}
};
void contracts(){
    std::uint8_t encrypted=9,unknown=8,show=0;std::string version="one";
    Cipher cipher;Factory factory;Random random;Events events;Effects effects;anadius::xml_reader_bridge::Backend xml;
    bridge::MessageHandler handler({encrypted,unknown,show,version,cipher,events},xml,factory,random,effects);
    require(cipher.resets==0 && encrypted==9 && unknown==8,"bridge construction adds no reset");
    handler.reset_02bc70();require(cipher.resets==1 && encrypted==0 && unknown==0,"reset changes shared flags and actual cipher");
    auto greeting=handler.initial_message_02d540();
    require(greeting.back()=='\0' && greeting.find('\0')==greeting.size()-1,"greeting retains exactly terminating NUL");
    require(greeting.find("version=\"one\"")!=std::string::npos && greeting.find("key=\"000102030405060708090a0b0c0d0e0f\"")!=std::string::npos,"actual greeting serialization and challenge hex");
    version="two & more";greeting=handler.initial_message_02d540();require(greeting.find("version=\"two &amp; more\"")!=std::string::npos && random.calls==2,"version cell remains live on subsequent greeting");
    const std::string plain="<LSX><Request id='7' recipient='EbisuSDK'><GetConfig/></Request></LSX>";
    auto response=handler.process_message_033d70(plain);require(response.back()=='\0' && response.find("<ok/>")!=std::string::npos,"actual plain message path");
    require(effects.handlers==1 && effects.received==0,"show flag initially off");
    show=1;const std::string nul_input=plain+std::string("\0ignored",8);response=handler.process_message_033d70(nul_input);
    require(response.find("<ok/>")!=std::string::npos && nul_input.size()==plain.size()+8,"borrowed input and embedded NUL preserved, parser CStr semantics retained");
    require(effects.received==1 && effects.responses==1,"live show flag controls actual core logging");
    encrypted=1;const std::string wire="410042";response=handler.process_message_033d70(wire);
    require(cipher.decrypted==std::string("A\0B",3) && wire=="410042","hex decode mutates owned copy retaining explicit binary length");
    require(response.back()=='\0' && encrypted==1,"encrypted response follows shared mode");
    encrypted=0;require(handler.process_message_033d70("<broken").empty() && effects.bad==1,"parse error follows core empty response and logging");
    effects.throw_handler=true;bool threw=false;try{(void)handler.process_message_033d70(plain);}catch(const std::runtime_error&){threw=true;}require(threw,"handler exception propagates through composition");
    effects.throw_handler=false;encrypted=1;cipher.throw_encrypt=true;threw=false;try{(void)handler.process_message_033d70(wire);}catch(const std::runtime_error&){threw=true;}require(threw,"cipher exception propagates");
}
}
int main(){try{contracts();std::cout<<"PASS: "<<checks<<" session handler checks; actual message core and controlled cipher only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
