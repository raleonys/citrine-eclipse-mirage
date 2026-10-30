#include "preloader_l/net/server.hpp"
#include "preloader_l/net/session_policy.hpp"
#include <iostream>
#include <stdexcept>
#include <vector>

namespace srv=preloader_l::server_recovered;
namespace net=preloader_l::net;
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
    void post_io(std::function<void()> fn)override{io.push_back(std::move(fn));}
    void launch_detached(std::function<void()> fn)override{jobs.push_back(std::move(fn));}
    void dispatch_write(std::function<void()> fn)override{post_io(std::move(fn));}
    bool ordered_handlers()const override{return true;}
    bool async_greeting()const override{return true;}
    std::size_t max_frame_bytes()const override{return 8;}
    std::size_t max_response_bytes()const override{return 16;}
    void fail_session(srv::Error)override{failed=true;}
    void log_error(std::string_view,srv::Error)override{}
    [[noreturn]]void raise_write_error(srv::Error)override{throw std::runtime_error("unexpected sync greeting");}
    void drain(){while(!io.empty()){auto fn=std::move(io.front());io.pop_front();fn();}}
    void job(){auto fn=std::move(jobs.front());jobs.pop_front();fn();drain();}
};
struct Transport final:srv::Transport {
    struct Write{std::span<const char> bytes;srv::Completion done;};
    std::deque<Write> writes;std::function<void(srv::Error,std::string)> read;unsigned synchronous=0;
    srv::WriteResult synchronous_send(std::span<const char>)override{++synchronous;return {};}
    void async_receive(std::size_t count,std::function<void(srv::Error,std::string)> cb)override{require(count<=8,"receive allocation is frame-bounded");read=std::move(cb);}
    void async_send(std::span<const char> bytes,srv::Completion cb)override{writes.push_back({bytes,std::move(cb)});}
    void complete(std::size_t count,srv::Error error={}){auto op=std::move(writes.front());writes.pop_front();op.done(error,count);}
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
    {Fixture f;f.handler.response.assign(17,'x');bool failed=false;
     try{f.request();}catch(const std::length_error&){failed=true;}
     require(failed&&f.handler.stats.retried==1&&f.transport.writes.empty(),"oversize output rejects before socket send and cancels receipt");}
    {Fixture f;bool failed=false;try{f.transport.deliver("12345678");}catch(const std::length_error&){failed=true;}
     require(failed&&f.handler.processed==0,"unterminated full frame is rejected");}
    {Fixture f;f.transport.deliver(std::string("1234567\0",8));f.scheduling.drain();require(f.scheduling.jobs.size()==1,"exact-limit terminated frame is admitted");}
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
int main()try{delivery_and_order();failure_and_buffer_lifetime();deadline_policy();std::cout<<"PASS: "<<checks<<" delivery/FIFO/buffer ownership/deadline checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
