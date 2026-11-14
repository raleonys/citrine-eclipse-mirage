// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/server.hpp"
#include "laureondo/net/session_policy.hpp"
#include "laureondo/net/session_handler.hpp"
#include "laureondo/xml/reader.hpp"
#include "laureondo/handlers/misc_handlers.hpp"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <new>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace allocation_fault {
thread_local bool next=false;
unsigned injected=0;
}
void* operator new(std::size_t size){
    // A shared holder must fit its PreparedResponse. Permit smaller checked-STL
    // bookkeeping allocations, which may occur in noexcept string moves.
    if(allocation_fault::next&&size>=sizeof(laureondo::server::PreparedResponse)){
        allocation_fault::next=false;++allocation_fault::injected;throw std::bad_alloc();
    }
    if(auto* value=std::malloc(size?size:1))return value;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size){return ::operator new(size);}
void operator delete(void* value)noexcept{std::free(value);}
void operator delete[](void* value)noexcept{std::free(value);}
void operator delete(void* value,std::size_t)noexcept{std::free(value);}
void operator delete[](void* value,std::size_t)noexcept{std::free(value);}

namespace srv=laureondo::server;
namespace net=laureondo::net;
using namespace std::chrono_literals;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Stats {unsigned committed=0,retried=0,destroyed=0,commit_attempts=0;bool accept_commit=true;};
struct Receipt final:srv::DeliveryReceipt {
    Stats& stats;explicit Receipt(Stats& s):stats(s){}
    ~Receipt(){++stats.destroyed;}
    bool finish(bool success)noexcept override{
        if(success){++stats.commit_attempts;if(!stats.accept_commit)return false;++stats.committed;}
        else ++stats.retried;
        return true;
    }
};
struct Handler final:srv::Handler {
    Stats stats;unsigned processed=0;std::string response=std::string("reply\0event\0",12);
    bool fail_owner_allocation=false;
    void reset_02bc70()override{}
    std::string initial_message_02d540()override{return std::string("hello\0",6);}
    std::string process_message_033d70(std::string_view)override{return response;}
    srv::PreparedResponse prepare_response(std::string_view)override{
        ++processed;srv::PreparedResponse prepared{response,std::make_unique<Receipt>(stats)};
        // The wire bytes and receipt already exist. The caller's shared owner
        // allocation is the next object-sized allocation, before dispatch.
        if(fail_owner_allocation)allocation_fault::next=true;
        return prepared;
    }
};
struct DispatchRefused {};
struct Scheduler final:srv::Scheduling {
    std::deque<std::function<void()>> io,jobs;bool failed=false,refuse_dispatch=false;unsigned dispatches=0,failures=0;srv::Error failure{};
    std::size_t frame_limit=8,response_limit=16;
    void post_io(std::function<void()> fn)override{io.push_back(std::move(fn));}
    void launch_detached(std::function<void()> fn)override{jobs.push_back(std::move(fn));}
    void dispatch_write(std::function<void()> fn)override{++dispatches;if(refuse_dispatch)throw DispatchRefused{};post_io(std::move(fn));}
    bool ordered_handlers()const override{return true;}
    bool async_greeting()const override{return true;}
    std::size_t max_frame_bytes()const override{return frame_limit;}
    std::size_t max_response_bytes()const override{return response_limit;}
    void fail_session(srv::Error error)override{failed=true;++failures;failure=error;}
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
void prepared_owner_allocation_failure() {
    for(const bool coalesced:{false,true}) {
        Fixture f;
        struct Disarm {~Disarm(){allocation_fault::next=false;}} disarm;
        f.handler.fail_owner_allocation=true;
        const auto before=allocation_fault::injected;
        const auto sends=f.transport.sends;
        f.transport.deliver(coalesced?std::string("a\0b\0",4):std::string("a\0",2));
        f.scheduling.drain();
        bool threw=false;
        try{f.scheduling.job();}catch(const std::bad_alloc&){threw=true;}
        allocation_fault::next=false;
        require(threw&&allocation_fault::injected==before+1,
                "one prepared shared-owner allocation failure propagates before dispatch");
        require(f.handler.processed==1&&f.handler.stats.committed==0&&
                f.handler.stats.retried==1&&f.handler.stats.destroyed==1,
                "unpublished prepared receipt cancels and is destroyed exactly once");
        require(f.session.queued_messages()==0&&f.transport.writes.empty()&&f.transport.sends==sends,
                "failed shared-owner construction cannot enqueue output or start a send");
        require(f.scheduling.io.empty()&&f.scheduling.jobs.empty()&&!f.transport.read,
                "failed preparation ownership cannot dispatch or resume ordered input");
        require(f.session.buffered_input()==(coalesced?std::string_view("b\0",2):std::string_view{}),
                "shared-owner failure preserves any unadmitted successor frame");
        f.session.cancel_output();
        require(f.handler.stats.committed==0&&f.handler.stats.retried==1&&f.handler.stats.destroyed==1,
                "later output cancellation cannot settle the unpublished receipt again");
    }
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
// Socket completion and event settlement are separate outcomes. Rejecting a
// receipt after every byte was accepted must still stop ordered admission.
void rejected_delivery_stops_admission() {
    for(const auto size:{std::size_t{0},std::size_t{12},std::size_t{0x10002}})
    for(const bool fragmented:{false,true}) {
        Fixture f;
        f.handler.stats.accept_commit=false;
        f.handler.response.assign(size,'X');
        if(size){f.handler.response.front()='R';f.handler.response[size/2]='\0';f.handler.response.back()='\0';}
        f.scheduling.response_limit=std::max(size,std::size_t{16});
        f.transport.sent.clear();
        const auto greeting_sends=f.transport.sends;
        f.transport.deliver(std::string("a\0b\0",4));
        f.scheduling.drain();f.scheduling.job();
        require(f.handler.processed==1&&f.session.buffered_input()==std::string_view("b\0",2),
            "only the first coalesced request reaches its handler");
        while(!f.transport.writes.empty()) {
            require(f.handler.stats.commit_attempts==0&&f.handler.stats.retried==0&&
                    !f.scheduling.failed&&f.scheduling.jobs.empty()&&!f.transport.read,
                "partial delivery cannot settle the receipt or admit the next request");
            const auto offered=f.transport.writes.front().bytes.size();
            require(offered>0&&offered<=0x10000,"response writes stay within the existing send quantum");
            f.transport.complete(fragmented?std::max(std::size_t{1},offered/2):offered);
        }
        f.scheduling.drain();
        require(f.transport.sent==f.handler.response,"receipt refusal occurs after exact counted wire delivery including NUL bytes");
        require(size?f.transport.sends>greeting_sends:f.transport.sends==greeting_sends,
            "empty response settles asynchronously without a zero-length send");
        require(f.scheduling.failed&&f.scheduling.failures==1&&f.scheduling.failure.value==29&&
                f.scheduling.failure.category==srv::ErrorCategory::system,
            "rejected delivery receipt fails the session once with its current error");
        require(f.handler.stats.commit_attempts==1&&f.handler.stats.committed==0&&
                f.handler.stats.retried==1&&f.handler.stats.destroyed==1,
            "refused commit is cancelled and its receipt destroyed exactly once");
        require(f.handler.processed==1&&f.scheduling.jobs.empty()&&f.scheduling.io.empty()&&
                !f.transport.read&&f.session.queued_messages()==0&&
                f.session.buffered_input()==std::string_view("b\0",2),
            "failed settlement cancels output without admitting buffered or new input");
        f.session.cancel_output();f.scheduling.drain();
        require(f.handler.stats.commit_attempts==1&&f.handler.stats.retried==1&&
                f.handler.stats.destroyed==1&&f.scheduling.failures==1,
            "repeated cancellation cannot settle, destroy or fail the retired owner again");
    }
}
namespace msg=laureondo::messages;
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
    bool present=true,reserved=false;unsigned inspections=0,commits=0,cancellations=0;
    std::int32_t count=1;
    bool nonempty_relaxed()const override{return present;}
    int lock_04a6ac()override{++inspections;return 0;}
    std::int32_t& lock_count_4c()override{return count;}
    bool nonempty_locked()const override{return present;}
    std::string copy_front_locked()const override{return "queued-event";}
    void pop_front_locked()override{throw std::logic_error("ordered session must use a receipt");}
    void unlock_04a6b4()override{}
    std::uint64_t reserve_front_locked()override{require(!reserved&&present,"one live event reservation");reserved=true;return 1;}
    bool finish_reserved(std::uint64_t token,bool success)noexcept override{if(token!=1||!reserved)return false;reserved=false;if(success){present=false;++commits;}else ++cancellations;return true;}
};
struct ProtocolEffects final:msg::Effects,laureondo::misc_handlers::NoResponse,msg::UnknownMessageEffects {
    unsigned silent=0,valid=0,invalid=0,diagnostics=0;bool reference_invalid=false;std::string unknown_request;
    int atoi_05ee08(std::string_view)override{return 0;}
    bool registered_handler(std::uint32_t rva,const msg::XmlNode&,msg::XmlPrinter& printer)override{
        if(rva==0x030eb0){++silent;laureondo::misc_handlers::query_chunk_status_030eb0(*this);}
        require(rva==0x02f2d0,"follow-up uses registered connected-state command");++valid;
        printer.open("InternetConnectedState");printer.attribute("connected","1");printer.close();return true;
    }
    [[noreturn]]void throw_no_response_02bcd0_04d228()override{throw msg::NoResponseRequiredError{};}
    [[noreturn]]void invalid_message_02bd30_04d228()override{++invalid;if(reference_invalid)throw msg::UnknownMessageError{};throw std::invalid_argument("invalid session fixture request");}
    void unknown_message_07bd70(const msg::UnknownMessageInput& input)override{++diagnostics;unknown_request=input.view();}
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
        laureondo::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;PendingEvent events;ProtocolEffects effects;
        std::uint8_t encrypted_flag=0,unknown=0,show=0;const std::string version="fixture";
        laureondo::session_bridge::MessageHandler handler({encrypted_flag,unknown,show,version,cipher,events},xml,factory,random,effects);
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


void unknown_message_same_session(){
    const std::vector<std::string> invalid={"<Other/>","<LSX/>","<LSX><Other/></LSX>",
        "<LSX><Request id='bad' recipient='EbisuSDK'><Unknown/></Request><Extra/></LSX>",
        "<LSX><Request recipient='EbisuSDK'><Unknown/></Request></LSX>",
        "<LSX><Request id='bad'><Unknown/></Request></LSX>",
        "<LSX><Request id='bad' recipient='EbisuSDK'/></LSX>",
        "<LSX><Request id='bad' recipient='EbisuSDK'><Unknown/></Request></LSX>",
        "<LSX><Request id='bad' recipient='EbisuSDK'><ChallengeResponse/></Request></LSX>"};
    for(bool encrypted:{false,true})for(bool coalesced:{false,true})for(const auto& bad:invalid){
        laureondo::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;PendingEvent events;ProtocolEffects effects;effects.reference_invalid=true;
        std::uint8_t encrypted_flag=0,unknown=0,show=0;const std::string version="fixture";
        laureondo::session_bridge::MessageHandler handler({encrypted_flag,unknown,show,version,cipher,events},xml,factory,random,effects);
        Scheduler scheduling;scheduling.frame_limit=scheduling.response_limit=4096;Transport transport;transport.max_request=4096;srv::Session session(transport,scheduling,handler);
        session.start_025720();transport.complete(transport.writes.front().bytes.size());transport.sent.clear();encrypted_flag=encrypted;const auto sends=transport.sends;
        auto frame=[&](std::string value){if(encrypted)msg::hex_encode_03d190(value);value.push_back('\0');return value;};
        const auto good="<LSX><Request id='next' recipient='EbisuSDK'><GetInternetConnectedState/></Request></LSX>";
        transport.deliver(frame(bad)+(coalesced?frame(good):std::string{}));scheduling.drain();scheduling.job();
        require(effects.diagnostics==1&&effects.unknown_request==bad&&effects.valid==0,"each invalid dispatch reaches its counted diagnostic before a later command");
        require(transport.sends==sends&&transport.writes.empty()&&transport.sent.empty()&&!scheduling.failed,"unknown diagnostic returns without socket bytes or failure");
        require(events.present&&!events.reserved&&events.inspections==0&&cipher.encryptions==0&&encrypted_flag==encrypted,"unknown path preserves event reservation and session cipher mode");
        if(!coalesced){require(static_cast<bool>(transport.read),"unknown isolated frame rearms same session");transport.deliver(frame(good));scheduling.drain();}
        require(scheduling.jobs.size()==1,"coalesced or isolated successor admitted after diagnostic return");scheduling.job();
        require(effects.valid==1&&events.commits==0,"later valid response cannot commit before its send");
        while(!transport.writes.empty())transport.complete(transport.writes.front().bytes.size());scheduling.drain();
        std::string expected="<LSX>\n    <Response id=\"next\" sender=\"EbisuSDK\">\n        <InternetConnectedState connected=\"1\"/>\n    </Response>\n</LSX>\n";
        if(encrypted)msg::hex_encode_03d190(expected);expected.push_back('\0');
        if(encrypted){std::string event="queued-event";msg::hex_encode_03d190(event);expected+=event;expected.push_back('\0');}
        require(transport.sent==expected,"only the correlated valid reply and eligible event are transmitted after UnknownMessage");
        require(events.commits==(encrypted?1u:0u)&&events.present==!encrypted&&!events.reserved,"only the subsequent eligible successful response settles the event");
        require(transport.read&&!scheduling.failed&&session.queued_messages()==0&&session.buffered_input().empty(),"same connection continues after each invalid dispatch form");
    }
}

void invalid_dispatch_stops_ordered_admission() {
    // This custom Effects deliberately throws ordinary invalid_argument. It is
    // distinct from Application's selected UnknownMessage diagnostic/continuation.
    // Observe the Session/bridge/core boundary, without native sockets/workers.
    for(bool encrypted:{false,true})for(bool coalesced:{false,true}) {
        laureondo::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;PendingEvent events;ProtocolEffects effects;
        std::uint8_t mode=0,unknown=0,show=0;const std::string version="fixture";
        laureondo::session_bridge::MessageHandler handler({mode,unknown,show,version,cipher,events},xml,factory,random,effects);
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
        require(rejected&&effects.invalid==1&&effects.valid==0&&effects.silent==0&&effects.diagnostics==0,"custom invalid_argument propagates without NoResponse or UnknownMessage diagnostics");
        require(transport.sends==greeting_sends&&transport.writes.empty()&&transport.sent.empty(),"invalid dispatch publishes no response or empty socket send");
        require(!transport.read&&scheduling.io.empty()&&scheduling.jobs.empty(),"invalid dispatch cannot rearm receive or admit a buffered successor");
        require(session.buffered_input()==(coalesced?valid:std::string{})&&session.queued_messages()==0,"buffered successor remains unprocessed after dispatch failure");
        require(events.present&&!events.reserved&&!events.inspections&&!events.commits&&!cipher.encryptions,"invalid dispatch cannot reserve or consume another session's queued event");
    }
}

void response_dispatch_refusal() {
    // A worker has finished preparation, but the IO executor refuses ownership.
    // Exercise real Session/bridge/core receipt unwind with a controlled
    // pre-enqueue exception. Native post allocation and peer close are not run.
    for(bool encrypted:{false,true})for(bool coalesced:{false,true}) {
        PendingEvent events;
        const auto request=[](std::string_view id,bool encoded){
            std::string value="<LSX><Request id='"+std::string(id)+"' recipient='EbisuSDK'><GetInternetConnectedState/></Request></LSX>";
            if(encoded)msg::hex_encode_03d190(value);value.push_back('\0');return value;
        };
        {
            laureondo::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;ProtocolEffects effects;
            std::uint8_t mode=0,unknown=0,show=0;const std::string version="fixture";
            laureondo::session_bridge::MessageHandler handler({mode,unknown,show,version,cipher,events},xml,factory,random,effects);
            Scheduler scheduling;scheduling.frame_limit=scheduling.response_limit=4096;
            Transport transport;transport.max_request=4096;srv::Session session(transport,scheduling,handler);
            session.start_025720();transport.complete(transport.writes.front().bytes.size());transport.sent.clear();mode=encrypted;
            const auto greeting_sends=transport.sends;
            const auto first=request("first",encrypted),next=request("blocked",encrypted);
            transport.deliver(coalesced?first+next:first);scheduling.drain();
            require(scheduling.jobs.size()==1,"dispatch refusal starts with one ordered worker");
            scheduling.refuse_dispatch=true;
            bool refused=false;try{scheduling.job();}catch(const DispatchRefused&){refused=true;}
            require(refused&&scheduling.dispatches==1&&effects.valid==1,"prepared response reaches the selected pre-enqueue refusal exactly once");
            require(events.present&&!events.reserved&&events.commits==0&&events.cancellations==(encrypted?1u:0u),
                    "refused response handoff releases its reservation without acknowledging the event");
            require(events.inspections==(encrypted?1u:0u)&&cipher.encryptions==(encrypted?2u:0u),
                    "encrypted response and event preparation occur before the refusal; plaintext does not select an event");
            require(transport.sends==greeting_sends&&transport.writes.empty()&&transport.sent.empty(),"dispatch refusal exposes no response buffer to transport");
            require(!transport.read&&scheduling.io.empty()&&scheduling.jobs.empty()&&session.queued_messages()==0&&
                    session.buffered_input()==(coalesced?next:std::string{}),"dispatch refusal neither rearms receive nor executes the buffered successor");
            session.cancel_output();
            require(events.cancellations==(encrypted?1u:0u)&&events.commits==0,"later output cancellation cannot settle the refused owner again");
        }
        // The failed peer does not resume. A separate Session sharing the queue
        // retries the released event and acknowledges it only after full output.
        laureondo::xml_reader_bridge::Backend xml;IdentityCipher cipher;CipherFactory factory;Random random;ProtocolEffects effects;
        std::uint8_t mode=0,unknown=0,show=0;const std::string version="fixture";
        laureondo::session_bridge::MessageHandler handler({mode,unknown,show,version,cipher,events},xml,factory,random,effects);
        Scheduler scheduling;scheduling.frame_limit=scheduling.response_limit=4096;
        Transport transport;transport.max_request=4096;srv::Session retry(transport,scheduling,handler);
        retry.start_025720();transport.complete(transport.writes.front().bytes.size());transport.sent.clear();mode=1;
        transport.deliver(request("retry",true));scheduling.drain();scheduling.job();
        require(events.present&&events.reserved&&events.commits==0&&transport.writes.size()==1,"another Session reserves the same event before delivery without consuming it");
        require(transport.writes.front().bytes.size()>1,"retry contains response and event bytes");
        transport.complete(1);
        require(events.present&&events.reserved&&events.commits==0&&!transport.read,"partial retry output cannot acknowledge the event or resume input");
        while(!transport.writes.empty())transport.complete(transport.writes.front().bytes.size());
        std::string expected="<LSX>\n    <Response id=\"retry\" sender=\"EbisuSDK\">\n        <InternetConnectedState connected=\"1\"/>\n    </Response>\n</LSX>\n";
        msg::hex_encode_03d190(expected);expected.push_back('\0');
        std::string event="queued-event";msg::hex_encode_03d190(event);expected+=event;expected.push_back('\0');
        require(transport.sent==expected,"retry emits one exact response and the retained event in order");
        require(!events.present&&!events.reserved&&events.commits==1&&events.cancellations==(encrypted?1u:0u),"full retry delivery acknowledges exactly once after the earlier refusal");
        require(transport.read&&retry.buffered_input().empty()&&retry.queued_messages()==0&&!scheduling.failed,"successful retry Session rearms ordinary input");
    }
}

void deadline_policy() {
    static_assert(std::is_constructible_v<net::SessionDeadlines,net::SessionPolicy&>);
    static_assert(std::is_constructible_v<net::SessionDeadlines,const net::SessionPolicy&>);
    static_assert(!std::is_constructible_v<net::SessionDeadlines,net::SessionPolicy&&>);
    static_assert(!std::is_constructible_v<net::SessionDeadlines,const net::SessionPolicy&&>);
    net::SessionPolicy policy;policy.receive_idle=300ms;policy.receive_frame=100ms;policy.send_idle=50ms;
    net::SessionDeadlines d(policy);const net::SessionDeadlines::Time start{};
    d.begin_receive(start,false);require(!d.expired(start+299ms)&&d.expired(start+300ms),"receive idle boundary is monotonic and exact");
    d.receive_progress(start+200ms,1);d.receive_progress(start+280ms,1);
    require(!d.expired(start+299ms)&&d.expired(start+300ms),"fragments cannot extend the first-byte frame deadline");
    d.clear();d.begin_send(start);d.begin_send(start+40ms);d.send_progress(start+40ms,0);
    require(d.expired(start+50ms),"retry and zero-byte write do not reset send idle");
    d.send_progress(start+40ms,1);require(!d.expired(start+60ms)&&d.expired(start+90ms),"positive write progress extends send idle");
    d.clear();require(!d.next(),"stop clears every deadline");
    policy.send_idle=75ms;d.begin_send(start);
    require(!d.expired(start+74ms)&&d.expired(start+75ms),
            "lvalue policy remains borrowed rather than snapshotted");
}
}
int main()try{delivery_and_order();prepared_owner_allocation_failure();failure_and_buffer_lifetime();rejected_delivery_stops_admission();no_response_same_session();unknown_message_same_session();invalid_dispatch_stops_ordered_admission();response_dispatch_refusal();deadline_policy();std::cout<<"PASS: "<<checks<<" delivery/FIFO/buffer ownership/deadline checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
