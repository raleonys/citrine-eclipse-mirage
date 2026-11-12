// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/session_handler.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include "citrine_eclipse_mirage/runtime/event_queue.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>
namespace msg=citrine_eclipse_mirage::messages;
namespace bridge=citrine_eclipse_mirage::session_bridge;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected handler test dependency");}
struct Invalid{};
enum class DeliveryFault {copy,event_log,event_encrypt};
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
    unsigned encryptions=0,throw_at=0,fail_at=0;
    void construct_0423d0_in_place()override{++resets;}
    void set_fixed_key_084c98()override{unused();}
    void set_key_042780(const msg::Key&)override{unused();}
    bool decrypt_042480(std::string& input)override{decrypted=input;input="<LSX><Request id='7' recipient='EbisuSDK'><GetConfig/></Request></LSX>";return true;}
    bool encrypt_042610(std::string&)override{
        ++encryptions;
        if(encryptions==throw_at)throw DeliveryFault::event_encrypt;
        if(throw_encrypt)throw std::runtime_error("cipher");
        return encryptions!=fail_at;
    }
};
struct Factory final:msg::CryptoFactory {std::unique_ptr<msg::Cipher> create_0423d0()override{unused();}};
struct Random final:msg::Random {
    unsigned calls=0;
    msg::Key challenge_bytes_034cf0()override{++calls;msg::Key k{};for(unsigned i=0;i<16;++i)k[i]=static_cast<std::uint8_t>(i);return k;}
    msg::Key key_from_response_034c30(std::string_view)override{unused();}
};
struct Effects final:msg::Effects {
    unsigned handlers=0,received=0,responses=0,bad=0;bool throw_handler=false,throw_event=false,allow_event=false;
    int atoi_05ee08(std::string_view)override{unused();}
    bool registered_handler(std::uint32_t rva,const msg::XmlNode&,msg::XmlPrinter& p)override{require(rva==0x02e5f0,"exact GetConfig route");++handlers;if(throw_handler)throw std::runtime_error("handler");p.open("ok");p.close();return true;}
    [[noreturn]]void invalid_message_02bd30_04d228()override{throw Invalid{};}
    [[noreturn]]void thread_error_04a648(int)override{unused();}
    void log_received(std::string_view)override{++received;}
    void log_response(std::string_view)override{++responses;}
    void log_event(std::string_view)override{if(!allow_event)unused();if(throw_event)throw DeliveryFault::event_log;}
    void log_bad_xml(std::string_view,std::string_view)override{++bad;}
    void log_decryption_failure(std::string_view)override{unused();}
};
void contracts(){
    std::uint8_t encrypted=9,unknown=8,show=0;std::string version="one";
    Cipher cipher;Factory factory;Random random;Events events;Effects effects;citrine_eclipse_mirage::xml_reader_bridge::Backend xml;
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
struct ReservedEvents final:msg::EventQueue {
    mutable citrine_eclipse_mirage::event_queue::OwnedStorage storage;
    const std::string first=std::string("first\0payload",13)+std::string(512,'F'),next=std::string("next\0event",10);
    bool held=false,throw_copy=false;std::int32_t count=1;
    unsigned reservations=0,pops=0,finishes=0,positive=0,negative=0;
    ReservedEvents(){storage.construct_seed_copy(first);auto tail=next;storage.push_move_034660(tail);}
    bool nonempty_relaxed()const override{return storage.nonempty_relaxed();}
    int lock_04a6ac()override{require(!held,"prepared queue mutex is exclusive");held=true;return 0;}
    std::int32_t& lock_count_4c()override{return count;}
    bool nonempty_locked()const override{require(held,"prepared availability uses event lock");return storage.front_available_locked();}
    std::string copy_front_locked()const override{require(held,"prepared copy uses event lock");if(throw_copy)throw DeliveryFault::copy;return storage.copy_front_locked();}
    void pop_front_locked()override{require(held,"eager pop uses event lock");++pops;storage.pop_front_locked();}
    void unlock_04a6b4()override{require(held,"prepared unlock balances event lock");held=false;}
    std::uint64_t reserve_front_locked()override{require(held,"prepared reservation uses event lock");++reservations;return storage.reserve_front_locked();}
    bool finish_reserved(std::uint64_t token,bool success)noexcept override{++finishes;if(success)++positive;else ++negative;return storage.finish_reserved(token,success);}
};
struct PreparedFixture {
    ReservedEvents events;Cipher cipher;Factory factory;Random random;Effects effects;
    citrine_eclipse_mirage::xml_reader_bridge::Backend xml;std::uint8_t encrypted=1,unknown=0,show=1;std::string version="fixture";
    bridge::MessageHandler handler{{encrypted,unknown,show,version,cipher,events},xml,factory,random,effects};
    PreparedFixture(){effects.allow_event=true;}
    citrine_eclipse_mirage::server::PreparedResponse prepare(){cipher.encryptions=0;return handler.prepare_response("00");}
};
std::vector<std::string> plaintext_frames(std::string_view wire){
    std::vector<std::string> result;std::size_t at=0;
    while(at<wire.size()){
        const auto end=wire.find('\0',at);require(end!=wire.npos,"prepared wire frame terminates");
        auto frame=std::string(wire.substr(at,end-at));msg::hex_decode_03ce30(frame);result.push_back(std::move(frame));at=end+1;
    }
    return result;
}
std::string prepared_event(std::string_view wire){
    const auto frames=plaintext_frames(wire);
    require(frames.size()==2,"prepared output has exactly one response and one event frame");
    return frames[1];
}
void prepared_ownership(){
    PreparedFixture f;
    {
        auto response=f.prepare();
        const auto frames=plaintext_frames(response.bytes);
        require(response.receipt&&frames.size()==2&&frames[1]==f.events.first,"actual prepared bridge returns exact reserved binary event");
        require(f.events.reservations==1&&f.events.pops==0&&!f.events.held&&f.events.finishes==0,"preparing a response neither consumes nor settles its event");
        Cipher parallel_cipher;std::uint8_t parallel_encrypted=1,parallel_unknown=0;
        bridge::MessageHandler parallel{{parallel_encrypted,parallel_unknown,f.show,f.version,parallel_cipher,f.events},f.xml,f.factory,f.random,f.effects};
        auto other=parallel.prepare_response("00");
        require(plaintext_frames(other.bytes).size()==1&&f.events.reservations==1,"another actual handler cannot duplicate an outstanding prepared event");
    }
    require(f.events.negative==1&&f.events.positive==0,"dropping actual prepared response releases its reservation once");
    {
        auto retry=f.prepare();require(prepared_event(retry.bytes)==f.events.first,"dropped response retains exact original plaintext for retry");
        require(retry.finish(false)&&retry.finish(true)&&f.events.negative==2&&f.events.positive==0,"negative prepared receipt is idempotent and cannot later commit");
    }
    {
        auto commit=f.prepare();require(prepared_event(commit.bytes)==f.events.first,"explicit cancellation also retains original front");
        require(commit.finish(true)&&commit.finish(false)&&f.events.positive==1&&f.events.negative==2,"successful prepared receipt settles exactly once");
    }
    {
        auto next=f.prepare();require(prepared_event(next.bytes)==f.events.next,"commit advances one event without losing the following binary entry");
        require(next.finish(true),"following event commits independently");
    }
    auto empty=f.prepare();require(plaintext_frames(empty.bytes).size()==1&&!f.events.storage.nonempty_relaxed(),"only full delivery retires both queued events");
    require(f.events.finishes==4&&f.events.positive==2&&f.events.negative==2&&!f.events.held,"prepared destruction never adds duplicate settlements");
}
void prepared_exception_release(){
    for(const auto fault:{DeliveryFault::copy,DeliveryFault::event_log,DeliveryFault::event_encrypt}){
        PreparedFixture f;f.events.throw_copy=fault==DeliveryFault::copy;f.effects.throw_event=fault==DeliveryFault::event_log;f.cipher.throw_at=fault==DeliveryFault::event_encrypt?2u:0u;
        bool threw=false;try{(void)f.prepare();}catch(DeliveryFault observed){threw=observed==fault;}
        require(threw&&f.events.reservations==1&&f.events.negative==1&&f.events.positive==0&&!f.events.held,"post-reservation failure releases exact receipt and lock during actual bridge unwind");
        f.events.throw_copy=false;f.effects.throw_event=false;f.cipher.throw_at=0;
        auto retry=f.prepare();require(prepared_event(retry.bytes)==f.events.first,"post-reservation exception cannot consume or strand original binary event");
        require(retry.finish(true),"request after callback failure can commit the retained event");
    }
    for(const auto failed:{1u,2u}){
        PreparedFixture f;f.cipher.fail_at=failed;
        {
            auto response=f.prepare();require(plaintext_frames(response.bytes).size()==(failed==1?0u:1u),"failed encryption emits no unencrypted event frame");
            require(f.events.reservations==(failed==1?0u:1u)&&f.events.negative==(failed==1?0u:1u)&&!f.events.held,"response failure avoids reservation and event failure releases it before return");
        }
        f.cipher.fail_at=0;auto retry=f.prepare();require(prepared_event(retry.bytes)==f.events.first,"boolean encryption failure retains front for later actual handler preparation");
        require(retry.finish(true),"retry after encryption failure commits normally");
    }
}

}
int main(){try{contracts();prepared_ownership();prepared_exception_release();std::cout<<"PASS: "<<checks<<" session handler checks; actual bridge/core/queue with controlled cipher and mutex\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
