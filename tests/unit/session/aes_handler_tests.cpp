// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/session_handler.hpp"
#include "laureondo/codec/crypto.hpp"
#include "laureondo/runtime/event_queue.hpp"
#include "laureondo/xml/reader.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {
using namespace laureondo;
unsigned checks=0;
void require(bool value,const char* label){++checks;if(!value)throw std::runtime_error(label);}
struct Queue final:messages::EventQueue {
    mutable event_queue::OwnedStorage storage;
    const std::string first=std::string("first\0payload",13)+std::string(512,'F'),second=std::string("next\0event",10);
    bool held=false;std::int32_t count=1;
    unsigned reservations=0,pops=0,finishes=0,positive=0,negative=0;
    Queue(){storage.construct_seed_copy(first);auto tail=second;storage.push_move_034660(tail);}
    bool nonempty_relaxed()const override{return storage.nonempty_relaxed();}
    int lock_04a6ac()override{require(!held,"shared queue lock is exclusive");held=true;return 0;}
    std::int32_t& lock_count_4c()override{return count;}
    bool nonempty_locked()const override{require(held,"availability uses the event lock");return storage.front_available_locked();}
    std::string copy_front_locked()const override{require(held,"event copy uses the event lock");return storage.copy_front_locked();}
    void pop_front_locked()override{require(held,"eager pop requires lock");++pops;storage.pop_front_locked();}
    void unlock_04a6b4()override{require(held,"event lock releases once");held=false;}
    std::uint64_t reserve_front_locked()override{require(held,"reservation uses the lock");++reservations;return storage.reserve_front_locked();}
    bool finish_reserved(std::uint64_t token,bool success)noexcept override{++finishes;if(success)++positive;else ++negative;return storage.finish_reserved(token,success);}
};
struct Time final:crypto::TimeSource {
    std::int64_t unix_seconds_05eeb8()override{throw std::logic_error("pre-negotiated fixture cannot query a clock");}
};
struct Effects final:messages::Effects {
    int atoi_05ee08(std::string_view)override{throw std::logic_error("no handshake in prepared delivery fixture");}
    bool registered_handler(std::uint32_t rva,const messages::XmlNode&,messages::XmlPrinter& printer)override{
        require(rva==0x02e5f0,"actual GetConfig dispatch");printer.open("ok");printer.close();return true;
    }
    [[noreturn]]void invalid_message_02bd30_04d228()override{throw std::logic_error("unexpected invalid message");}
    [[noreturn]]void thread_error_04a648(int)override{throw std::logic_error("unexpected thread error");}
    void log_received(std::string_view)override{}
    void log_response(std::string_view)override{}
    void log_event(std::string_view)override{}
    void log_bad_xml(std::string_view,std::string_view)override{throw std::logic_error("unexpected XML failure");}
    void log_decryption_failure(std::string_view)override{throw std::logic_error("unexpected decryption failure");}
};
std::string request(crypto::Cipher& client,std::string_view id){
    auto bytes="<LSX><Request id='"+std::string(id)+"' recipient='EbisuSDK'><GetConfig/></Request></LSX>";
    require(client.encrypt_042610(bytes),"client request encrypts");messages::hex_encode_03d190(bytes);return bytes;
}
std::vector<std::string> decode(std::string_view wire,crypto::Cipher& client){
    std::vector<std::string> frames;std::size_t at=0;
    while(at<wire.size()){
        const auto end=wire.find('\0',at);require(end!=wire.npos,"each encrypted frame has a counted terminator");
        std::string frame(wire.substr(at,end-at));
        require(!frame.empty()&&frame.size()%2==0&&std::all_of(frame.begin(),frame.end(),[](unsigned char c){return(c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F');}),"prepared frames contain hex ciphertext only");
        messages::hex_decode_03ce30(frame);require(client.decrypt_042480(frame),"frame decrypts using its recipient's real AES key");
        frames.push_back(std::move(frame));at=end+1;
    }
    return frames;
}
void response_id(xml_reader_bridge::Backend& xml,const std::vector<std::string>& frames,std::string_view id){
    require(!frames.empty(),"prepared response frame exists");auto parsed=xml.parse_c_string(frames[0]);
    require(!parsed.has_error,"decrypted response parses");
    const auto* root=parsed.document->first_element();const auto* response=root?root->first_element():nullptr;
    require(root&&root->name()=="LSX"&&response&&response->name()=="Response"&&response->attribute("id")==id,"decrypted response belongs to this request");
}
void shared_receipt_real_keys(bool drop){
    Queue events;Time clock;crypto::Random random{clock};crypto::CryptoFactory factory;
    crypto::Cipher left,right,left_client,right_client;left.set_fixed_key_084c98();left_client.set_fixed_key_084c98();
    messages::Key key{};std::copy_n(left.round_keys().begin(),key.size(),key.begin());key[0]=static_cast<std::uint8_t>(key[0]^0x80U);
    right.set_key_042780(key);right_client.set_key_042780(key);
    const auto left_key=left.round_keys(),right_key=right.round_keys();require(left_key!=right_key,"two genuine distinct AES schedules");
    xml_reader_bridge::Backend xml;Effects effects;
    std::uint8_t left_encrypted=1,right_encrypted=1,left_unknown=0,right_unknown=0,show=1;std::string version="fixture";
    session_bridge::MessageHandler a{{left_encrypted,left_unknown,show,version,left,events},xml,factory,random,effects};
    session_bridge::MessageHandler b{{right_encrypted,right_unknown,show,version,right,events},xml,factory,random,effects};
    const auto prepare=[&](session_bridge::MessageHandler& handler,crypto::Cipher& client,std::string_view id){auto bytes=request(client,id);const auto original=bytes;auto result=handler.prepare_response(bytes);require(bytes==original,"borrowed encrypted request stays unchanged");return result;};
    {
        auto first=prepare(a,left_client,"left-first");auto frames=decode(first.bytes,left_client);response_id(xml,frames,"left-first");
        require(frames.size()==2&&frames[1]==events.first,"first key receives exact counted binary front");
        auto waiting=prepare(b,right_client,"right-wait");auto other=decode(waiting.bytes,right_client);response_id(xml,other,"right-wait");
        require(other.size()==1&&events.reservations==1&&events.finishes==0,"other key cannot duplicate a pending event");
        require(waiting.finish(true),"eventless response completes independently");
        if(!drop)require(first.finish(false)&&first.finish(true),"explicit cancellation is final and idempotent");
    }
    require(events.negative==1&&events.positive==0&&!events.held,"dropped or cancelled response releases only its reservation");
    {
        auto retry=prepare(b,right_client,"right-retry");auto frames=decode(retry.bytes,right_client);response_id(xml,frames,"right-retry");
        require(frames.size()==2&&frames[1]==events.first,"other key re-encrypts original binary plaintext after cancellation");
        auto waiting=prepare(a,left_client,"left-wait");auto other=decode(waiting.bytes,left_client);response_id(xml,other,"left-wait");
        require(other.size()==1&&events.reservations==2,"old key cannot duplicate the new pending claim");
        require(waiting.finish(true)&&retry.finish(true)&&retry.finish(false),"only new successful receipt commits once");
    }
    {
        auto next=prepare(a,left_client,"left-next");auto frames=decode(next.bytes,left_client);response_id(xml,frames,"left-next");
        require(frames.size()==2&&frames[1]==events.second,"next binary event remains FIFO and uses old key independently");require(next.finish(true),"following receipt completes");
    }
    auto empty=prepare(b,right_client,"right-empty");auto frames=decode(empty.bytes,right_client);response_id(xml,frames,"right-empty");
    require(frames.size()==1&&!events.storage.nonempty_relaxed(),"only full receipt completion drains both events");
    require(events.reservations==3&&events.finishes==3&&events.negative==1&&events.positive==2&&events.pops==0&&!events.held,"exact generations and no eager dequeue or duplicate finish");
    require(left.round_keys()==left_key&&right.round_keys()==right_key&&left_encrypted==1&&right_encrypted==1,"shared delivery never mixes per-handler keys or mode cells");
}
}
int main(){try{shared_receipt_real_keys(true);shared_receipt_real_keys(false);std::cout<<"PASS: "<<checks<<" real-AES shared prepared-event checks; controlled effects, no native socket or original DLL\n";return 0;}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
