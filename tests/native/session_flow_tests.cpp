// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/net/server.hpp"
#include "citrine_eclipse_mirage/net/session_policy.hpp"
#include "citrine_eclipse_mirage/net/session_handler.hpp"
#include "citrine_eclipse_mirage/xml/reader.hpp"
#include "citrine_eclipse_mirage/handlers/misc_handlers.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace srv=citrine_eclipse_mirage::server;
namespace net=citrine_eclipse_mirage::net;
using namespace std::chrono_literals;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Stats {unsigned committed=0,retried=0,destroyed=0;};
struct Receipt final:srv::DeliveryReceipt {
    Stats& stats;explicit Receipt(Stats& s):stats(s){}
    ~Receipt(){++stats.destroyed;}
    bool finish(bool success)noexcept override{if(success)++stats.committed;else ++stats.retried;return true;}
};
struct Handler final:srv::Handler {
    Stats stats;unsigned processed=0;std::string response=std::string("reply\0event\0",12);
    void reset_02bc70()override{}
    std::string initial_message_02d540()override{return std::string("hello\0",6);}
    std::string process_message_033d70(std::string_view)override{return response;}
    srv::PreparedResponse prepare_response(std::string_view)override{++processed;return {response,std::make_unique<Receipt>(stats)};}
};
struct Scheduler final:srv::Scheduling {
    std::deque<std::function<void()>> io,jobs;bool failed=false;
    std::size_t frame_limit=8,response_limit=16;
    void post_io(std::function<void()> fn)override{io.push_back(std::move(fn));}
    void launch_detached(std::function<void()> fn)override{jobs.push_back(std::move(fn));}
    void dispatch_write(std::function<void()> fn)override{post_io(std::move(fn));}
    bool ordered_handlers()const override{return true;}
    bool async_greeting()const override{return true;}
    std::size_t max_frame_bytes()const override{return frame_limit;}
    std::size_t max_response_bytes()const override{return response_limit;}
    void fail_session(srv::Error)override{failed=true;}
    void log_error(std::string_view,srv::Error)override{}
    [[noreturn]]void raise_write_error(srv::Error)override{throw std::runtime_error("unexpected sync greeting");}
    void drain(){while(!io.empty()){auto fn=std::move(io.front());io.pop_front();fn();}}
    void job(){auto fn=std::move(jobs.front());jobs.pop_front();fn();drain();}
};
struct Transport final:srv::Transport {
    struct Write{std::span<const char> bytes;srv::Completion done;};
    std::deque<Write> writes;std::function<void(srv::Error,std::string)> read;unsigned synchronous=0,sends=0;std::size_t max_request=8;std::string sent;
    srv::WriteResult synchronous_send(std::span<const char>)override{++synchronous;return {};}
    void async_receive(std::size_t count,std::function<void(srv::Error,std::string)> cb)override{require(count<=max_request,"receive allocation is frame-bounded");read=std::move(cb);}
    void async_send(std::span<const char> bytes,srv::Completion cb)override{++sends;writes.push_back({bytes,std::move(cb)});}
    void complete(std::size_t count,srv::Error error={}){auto op=std::move(writes.front());writes.pop_front();sent.append(op.bytes.data(),std::min(count,op.bytes.size()));op.done(error,count);}
    void deliver(std::string bytes){auto cb=std::move(read);read={};cb({},std::move(bytes));}
};
struct Fixture {
    Handler handler;Scheduler scheduling;Transport transport;srv::Session session{transport,scheduling,handler};
    Fixture(){session.start_025720();require(transport.synchronous==0&&!transport.read,"greeting starts asynchronously before receive");transport.complete(6);}
    void request(){transport.deliver(std::string("a\0",2));scheduling.drain();scheduling.job();}
};
void delivery_and_order() {
    Fixture f;f.transport.deliver(std::string("a\0b\0",4));f.scheduling.drain();f.scheduling.job();
    require(f.handler.processed==1&&f.scheduling.jobs.empty()&&!f.transport.read,"second frame waits while first response/event is pending");
    f.transport.complete(6);
    require(f.handler.stats.committed==0&&f.handler.processed==1,"response prefix alone cannot commit event or admit next handler");
    f.transport.complete(6);f.scheduling.drain();
    require(f.handler.stats.committed==1&&f.handler.stats.retried==0&&f.scheduling.jobs.size()==1,"full response/event commits before next frame admission");
    f.scheduling.job();f.transport.complete(12);require(f.handler.stats.committed==2,"next frame has its own delivery receipt");
}
void failure_and_buffer_lifetime() {
    {Fixture f;f.request();f.transport.complete(6);f.transport.complete(0,{10054,srv::ErrorCategory::system});
     require(f.handler.stats.committed==0&&f.handler.stats.retried==1&&f.scheduling.failed,"partial event failure retries rather than commits");}
    {Fixture f;f.request();f.session.cancel_output();
     require(f.handler.stats.retried==1&&f.handler.stats.destroyed==0,"cancel resolves receipt but keeps pending socket buffer owner alive");
     require(std::string_view(f.transport.writes.front().bytes.data(),12)==f.handler.response,"cancelled send buffer remains readable until callback");
     f.transport.complete(0,{995,srv::ErrorCategory::system});
     require(f.handler.stats.destroyed==1&&f.handler.stats.retried==1,"cancel completion releases buffer without duplicate retry");}
    {Fixture f;f.request();f.transport.complete(0);
     require(f.scheduling.failed&&f.handler.stats.committed==0&&f.handler.stats.retried==1,"zero progress is failure, never successful dequeue");}
    {Fixture f;f.request();f.transport.complete(13);
     require(f.scheduling.failed&&f.handler.stats.committed==0&&f.handler.stats.retried==1&&f.transport.writes.empty(),
             "over-reported send completion must not commit an event or retry an invalid buffer");}
    {Fixture f;f.request();f.transport.complete(6);f.transport.complete(7);
     require(f.scheduling.failed&&f.handler.stats.committed==0&&f.handler.stats.retried==1&&f.transport.writes.empty(),
             "over-reported final chunk must not commit after valid partial progress");}
    {Fixture f;f.request();f.transport.complete(12,{10054,srv::ErrorCategory::system});
     require(f.scheduling.failed&&f.handler.stats.committed==0&&f.handler.stats.retried==1,
             "full-length completion with a socket error must not commit a receipt");}
    {Fixture f;f.handler.response.assign(17,'x');bool failed=false;
     try{f.request();}catch(const std::length_error&){failed=true;}
     require(failed&&f.handler.stats.retried==1&&f.transport.writes.empty(),"oversize output rejects before socket send and cancels receipt");}
    {Fixture f;bool failed=false;try{f.transport.deliver("12345678");}catch(const std::length_error&){failed=true;}
     require(failed&&f.handler.processed==0,"unterminated full frame is rejected");}
    {Fixture f;f.transport.deliver(std::string("1234567\0",8));f.scheduling.drain();require(f.scheduling.jobs.size()==1,"exact-limit terminated frame is admitted");}
}
namespace msg=citrine_eclipse_mirage::messages;
struct IdentityCipher final:msg::Cipher {
    unsigned encryptions=0,decryptions=0;
    void construct_0423d0_in_place()override{}
    void set_fixed_key_084c98()override{}
    void set_key_042780(const msg::Key&)override{}
    bool decrypt_042480(std::string&)override{++decryptions;return true;}
    bool encrypt_042610(std::string&)override{++encryptions;return true;}
};
struct CipherFactory final:msg::CryptoFactory {
    std::unique_ptr<msg::Cipher> create_0423d0()override{return std::make_unique<IdentityCipher>();}
};
struct Random final:msg::Random {
    msg::Key challenge_bytes_034cf0()override{return {};}
    msg::Key key_from_response_034c30(std::string_view)override{return {};}
};
struct PendingEvent final:msg::EventQueue {
    bool present=true,reserved=false;unsigned inspections=0,commits=0;
    std::int32_t count=1;
    bool nonempty_relaxed()const override{return present;}
    int lock_04a6ac()override{++inspections;return 0;}
    std::int32_t& lock_count_4c()override{return count;}
    bool nonempty_locked()const override{return present;}
    std::string copy_front_locked()const override{return "queued-event";}
    void pop_front_locked()override{throw std::logic_error("ordered session must use a receipt");}
    void unlock_04a6b4()override{}
    std::uint64_t reserve_front_locked()override{require(!reserved&&present,"one live event reservation");reserved=true;return 1;}
    bool finish_reserved(std::uint64_t token,bool success)noexcept override{if(token!=1||!reserved)return false;reserved=false;if(success){present=false;++commits;}return true;}
};
struct ProtocolEffects final:msg::Effects,citrine_eclipse_mirage::misc_handlers::NoResponse {
    unsigned silent=0,valid=0,invalid=0;
    int atoi_05ee08(std::string_view)override{return 0;}
    bool registered_handler(std::uint32_t rva,const msg::XmlNode&,msg::XmlPrinter& printer)override{
        if(rva==0x030eb0){++silent;citrine_eclipse_mirage::misc_handlers::query_chunk_status_030eb0(*this);}
        require(rva==0x02f2d0,"follow-up uses registered connected-state command");++valid;
        printer.open("InternetConnectedState");printer.attribute("connected","1");printer.close();return true;
    }
    [[noreturn]]void throw_no_response_02bcd0_04d228()override{throw msg::NoResponseRequiredError{};}
    [[noreturn]]void invalid_message_02bd30_04d228()override{++invalid;throw std::invalid_argument("invalid session fixture request");}
    [[noreturn]]void thread_error_04a648(int)override{throw std::logic_error("unexpected event lock failure");}
    void log_received(std::string_view)override{}
    void log_response(std::string_view)override{}
    void log_event(std::string_view)override{}
    void log_bad_xml(std::string_view,std::string_view)override{throw std::logic_error("unexpected parse failure");}
    void log_decryption_failure(std::string_view)override{throw std::logic_error("unexpected decrypt failure");}
};
void no_response_same_session() {
    // Real ordered Session, MessageHandler, XML reader and protocol core with
    // a controlled cipher, command effects and in-memory transport. No socket,
    // original DLL, foreign provider or file backend is executed here.
    for(bool encrypted:{false,true})for(bool coalesced:{false,true}) {
        citrine_eclipse_mirage::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;PendingEvent events;ProtocolEffects effects;
        std::uint8_t encrypted_flag=0,unknown=0,show=0;const std::string version="fixture";
        citrine_eclipse_mirage::session_bridge::MessageHandler handler({encrypted_flag,unknown,show,version,cipher,events},xml,factory,random,effects);
        Scheduler scheduling;scheduling.frame_limit=scheduling.response_limit=4096;
        Transport transport;transport.max_request=4096;srv::Session session(transport,scheduling,handler);
        session.start_025720();transport.complete(transport.writes.front().bytes.size());transport.sent.clear();encrypted_flag=encrypted;
        const auto greeting_sends=transport.sends;
        const auto frame=[&](std::string_view command){std::string value="<LSX><Request id='next' recipient='EbisuSDK'><"+std::string(command)+"/></Request></LSX>";if(encrypted)msg::hex_encode_03d190(value);value.push_back('\0');return value;};
        const auto silent=frame("QueryChunkStatus"),valid=frame("GetInternetConnectedState");
        if(coalesced){transport.deliver(silent+silent+valid);scheduling.drain();}
        for(unsigned i=0;i<2;++i) {
            if(!coalesced){transport.deliver(silent);scheduling.drain();}
            require(scheduling.jobs.size()==1,"one ordered silent handler admitted");scheduling.job();
            require(effects.silent==i+1&&effects.valid==0&&!scheduling.failed,"silent command returns without a session failure");
            require(transport.sends==greeting_sends&&transport.writes.empty()&&transport.sent.empty(),"NoResponse emits neither bytes nor an empty socket send");
            require(events.present&&!events.reserved&&events.inspections==0&&cipher.encryptions==0,"NoResponse cannot reserve,consume or encrypt an event");
            require(encrypted_flag==encrypted,"NoResponse preserves the established cipher-mode flag");
            if(!coalesced)require(static_cast<bool>(transport.read),"same session rearms a pending read after NoResponse");
        }
        if(!coalesced){transport.deliver(valid);scheduling.drain();}
        require(scheduling.jobs.size()==1,"valid follow-up admitted on the same session");scheduling.job();
        require(effects.valid==1&&events.commits==0,"event cannot commit before follow-up delivery");
        while(!transport.writes.empty())transport.complete(transport.writes.front().bytes.size());
        scheduling.drain();
        std::string expected="<LSX>\n    <Response id=\"next\" sender=\"EbisuSDK\">\n        <InternetConnectedState connected=\"1\"/>\n    </Response>\n</LSX>\n";
        if(encrypted)msg::hex_encode_03d190(expected);expected.push_back('\0');
        if(encrypted){std::string event="queued-event";msg::hex_encode_03d190(event);expected+=event;expected.push_back('\0');}
        require(transport.sent==expected,"only the valid response and eligible event reach the wire");
        require(events.commits==(encrypted?1u:0u)&&events.present==!encrypted&&!events.reserved,"event delivered once under the existing encrypted/plaintext policy");
        require(transport.read&&!scheduling.failed&&session.queued_messages()==0&&session.buffered_input().empty(),"same session stays usable after silent commands and valid follow-up");
    }
}

void invalid_dispatch_stops_ordered_admission() {
    // The real Asio owner closes the peer after this propagated terminal.
    // Here verify the preceding Session/bridge/core boundary without pretending
    // this memory transport executes a socket close or a native worker.
    for(bool encrypted:{false,true})for(bool coalesced:{false,true}) {
        citrine_eclipse_mirage::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;PendingEvent events;ProtocolEffects effects;
        std::uint8_t mode=0,unknown=0,show=0;const std::string version="fixture";
        citrine_eclipse_mirage::session_bridge::MessageHandler handler({mode,unknown,show,version,cipher,events},xml,factory,random,effects);
        Scheduler scheduling;scheduling.frame_limit=scheduling.response_limit=4096;
        Transport transport;transport.max_request=4096;srv::Session session(transport,scheduling,handler);
        session.start_025720();transport.complete(transport.writes.front().bytes.size());transport.sent.clear();mode=encrypted;
        const auto greeting_sends=transport.sends;
        const auto frame=[&](std::string_view command){
            std::string value="<LSX><Request id='invalid' recipient='EbisuSDK'><"+std::string(command)+"/></Request></LSX>";
            if(encrypted)msg::hex_encode_03d190(value);value.push_back('\0');return value;
        };
        const auto invalid=frame("UnregisteredFixtureCommand"),valid=frame("GetInternetConnectedState");
        transport.deliver(coalesced?invalid+valid:invalid);scheduling.drain();
        require(scheduling.jobs.size()==1,"invalid dispatch has one actual ordered worker job");
        bool rejected=false;try{scheduling.job();}catch(const std::invalid_argument&){rejected=true;}
        require(rejected&&effects.invalid==1&&effects.valid==0&&effects.silent==0,"parsed unknown command propagates rather than becoming NoResponse");
        require(transport.sends==greeting_sends&&transport.writes.empty()&&transport.sent.empty(),"invalid dispatch publishes no response or empty socket send");
        require(!transport.read&&scheduling.io.empty()&&scheduling.jobs.empty(),"invalid dispatch cannot rearm receive or admit a buffered successor");
        require(session.buffered_input()==(coalesced?valid:std::string{})&&session.queued_messages()==0,"buffered successor remains unprocessed after dispatch failure");
        require(events.present&&!events.reserved&&!events.inspections&&!events.commits&&!cipher.encryptions,"invalid dispatch cannot reserve or consume another session's queued event");
    }
}

void deadline_policy() {
    net::SessionPolicy policy;policy.receive_idle=300ms;policy.receive_frame=100ms;policy.send_idle=50ms;
    net::SessionDeadlines d(policy);const net::SessionDeadlines::Time start{};
    d.begin_receive(start,false);require(!d.expired(start+299ms)&&d.expired(start+300ms),"receive idle boundary is monotonic and exact");
    d.receive_progress(start+200ms,1);d.receive_progress(start+280ms,1);
    require(!d.expired(start+299ms)&&d.expired(start+300ms),"fragments cannot extend the first-byte frame deadline");
    d.clear();d.begin_send(start);d.begin_send(start+40ms);d.send_progress(start+40ms,0);
    require(d.expired(start+50ms),"retry and zero-byte write do not reset send idle");
    d.send_progress(start+40ms,1);require(!d.expired(start+60ms)&&d.expired(start+90ms),"positive write progress extends send idle");
    d.clear();require(!d.next(),"stop clears every deadline");
}
}
int main()try{delivery_and_order();failure_and_buffer_lifetime();no_response_same_session();invalid_dispatch_stops_ordered_admission();deadline_policy();std::cout<<"PASS: "<<checks<<" delivery/FIFO/buffer ownership/deadline checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
