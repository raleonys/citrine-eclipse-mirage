// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/event_queue.hpp"
#include <array>
#include <chrono>
#include <exception>
#include <future>
#include <iostream>
#include <latch>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>
namespace eq=citrine_eclipse_mirage::event_queue;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>void rejects(F f,const char* why){bool hit=false;try{f();}catch(const std::exception&){hit=true;}require(hit,why);}
struct LatchRelease {
    std::latch& gate;bool released=false;
    void release()noexcept{if(!released){released=true;gate.count_down();}}
    ~LatchRelease(){release();}
};
struct Mutex final:eq::Mutex {
    std::int32_t count=1;
    int lock_04a6ac()override{return 0;}
    std::int32_t& count_4c()override{return count;}
    void unlock_04a6b4()override{}
    [[noreturn]]void thread_error_04a648(int)override{throw std::runtime_error("thread");}
};
struct Runtime final:eq::Initialization {
    std::vector<std::string> trace;std::int32_t tls=std::numeric_limits<std::int32_t>::min(),epoch=tls;
    bool winner=false;std::string failure;int atexit_result=0;eq::Mutex* expected=nullptr;
    void step(std::string name){trace.push_back(name);if(failure==name)throw std::runtime_error(name);}
    std::int32_t thread_epoch()override{step("epoch");return tls;}
    void header_04b838(std::int32_t& guard)override{step("header");guard=winner?-20:-1;}
    void footer(std::int32_t& guard)override{step("footer");guard=++epoch;tls=epoch;}
    void construct_mutex_04a698(eq::Mutex& mutex,std::uint32_t flags)override{require(&mutex==expected && flags==2,"same mutex and exact constructor flags2");step("construct-mutex");}
    int register_atexit(eq::Cleanup id)override{step(id==eq::Cleanup::queue_07e000?"atexit-queue":"atexit-mutex");return atexit_result;}
};
struct Storage final:eq::Storage {
    Runtime& runtime;std::string seed;bool throw_seed=false;
    explicit Storage(Runtime& r):runtime(r){}
    void construct_seed_copy(std::string_view value)override{runtime.step("seed");seed=value;if(throw_seed)throw std::runtime_error("seed");}
    bool nonempty_relaxed()const override{return false;}
    bool nonempty_locked()const override{return false;}
    std::string copy_front_locked()const override{return {};}
    void push_move_034660(std::string&)override{}
    void pop_front_locked()override{}
};
void getters(){
    Mutex mutex;Runtime runtime;Storage storage(runtime);eq::Guards guards{0,0};eq::Context context{guards,runtime,mutex,storage};runtime.expected=&mutex;
    runtime.atexit_result=-1;
    require(&eq::mutex_02c620(context)==&mutex,"getter returns same mutex identity");
    require(runtime.trace==std::vector<std::string>{"epoch","header","construct-mutex","atexit-mutex","footer"},"cold mutex normal call order even atexit failure");
    require(guards.mutex_0a3970<0 && guards.queue_0a39a0==0,"independent guard cells");
    runtime.trace.clear();require(&eq::mutex_02c620(context)==&mutex && runtime.trace==std::vector<std::string>{"epoch"},"hot mutex skips header and construction");
    runtime.trace.clear();require(&eq::queue_02c400(context)==&storage,"same queue storage identity");
    require(runtime.trace==std::vector<std::string>{"epoch","header","seed","atexit-queue","footer"},"cold queue seed copy then cleanup registration then footer");
    require(storage.seed==eq::initial_down_event && storage.seed.size()==68 && storage.seed.find('\0')==std::string::npos,"exact68byte DOWN copied without terminator");
    runtime.trace.clear();(void)eq::queue_02c400(context);require(runtime.trace==std::vector<std::string>{"epoch"},"hot queue not reseeded");
    for(bool queue:{false,true}){
        runtime.trace.clear();runtime.winner=true;runtime.tls=-100;auto& guard=queue?guards.queue_0a39a0:guards.mutex_0a3970;guard=0;
        if(queue)(void)eq::queue_02c400(context);else (void)eq::mutex_02c620(context);
        require(runtime.trace==std::vector<std::string>{"epoch","header"} && guard==-20,"header winner recheck skips constructor registration footer");
        runtime.trace.clear();runtime.tls=5;guard=-20;
        if(queue)(void)eq::queue_02c400(context);else (void)eq::mutex_02c620(context);
        require(runtime.trace==std::vector<std::string>{"epoch"},"signed comparison positive TLS vs negative guard is hot");
    }
}
void failures(){
    for(bool queue:{false,true})for(const auto* stage:{"header","construct","register","footer"}){
        Mutex mutex;Runtime runtime;Storage storage(runtime);eq::Guards guards{0,0};eq::Context context{guards,runtime,mutex,storage};runtime.expected=&mutex;
        runtime.failure=std::string(stage)=="construct"?(queue?"seed":"construct-mutex"):std::string(stage)=="register"?(queue?"atexit-queue":"atexit-mutex"):stage;
        rejects([&]{if(queue)(void)eq::queue_02c400(context);else(void)eq::mutex_02c620(context);},"initializer exception propagates");
        require(runtime.trace.back()==runtime.failure,"failure stops following initializer effects");
        const auto guard=queue?guards.queue_0a39a0:guards.mutex_0a3970;
        require(guard==(std::string(stage)=="header"?0:-1),"normal source adds no unproven rollback or abort");
    }
}
void owned_fifo(){
    eq::OwnedStorage storage;require(!storage.nonempty_relaxed(),"unconstructed relaxed publication empty");
    rejects([&]{(void)storage.nonempty_locked();},"locked inspection requires constructed store");
    rejects([&]{(void)storage.copy_front_locked();},"front before construction rejected");
    rejects([&]{storage.pop_front_locked();},"pop before construction rejected");
    std::string value="before";rejects([&]{storage.push_move_034660(value);},"push before construction rejected");require(value=="before","precondition failure does not move input");
    storage.construct_seed_copy(eq::initial_down_event);require(storage.nonempty_relaxed() && storage.nonempty_locked(),"seed published");
    require(storage.copy_front_locked()==eq::initial_down_event && storage.copy_front_locked()==eq::initial_down_event,"front copy does not consume");
    rejects([&]{storage.construct_seed_copy("again");},"second construction rejected");storage.pop_front_locked();
    require(!storage.nonempty_relaxed() && !storage.nonempty_locked(),"last pop publishes empty");
    for(unsigned i=0;i<1000;++i){std::string item=std::to_string(i)+std::string("\0tail",5);const auto expected=item;storage.push_move_034660(item);require(item.empty(),"move source logically empty");if(i==0)require(storage.copy_front_locked()==expected,"embeddedNUL retained in first queued item");}
    for(unsigned i=0;i<1000;++i){require(storage.copy_front_locked()==std::to_string(i)+std::string("\0tail",5),"chosen FIFO backend preserves order and explicit bytes");storage.pop_front_locked();}
    require(!storage.nonempty_relaxed(),"all items consumed");rejects([&]{storage.pop_front_locked();},"empty pop rejected");rejects([&]{(void)storage.copy_front_locked();},"empty front rejected");
}
void stale_delivery_receipts(){
    eq::OwnedStorage storage;storage.construct_seed_copy("first");
    std::string second="second",third("third\0tail",10);
    storage.push_move_034660(second);
    const auto cancelled=storage.reserve_front_locked();
    require(cancelled!=0&&!storage.front_available_locked(),"one reservation excludes another consumer");
    require(storage.reserve_front_locked()==0,"second reservation cannot steal pending front");
    require(!storage.finish_reserved(0,true),"zero receipt cannot complete a pending event");
    rejects([&]{storage.pop_front_locked();},"ordinary pop cannot consume a reserved event");
    require(storage.finish_reserved(cancelled,false),"failed delivery resolves its reservation");
    require(!storage.finish_reserved(cancelled,true),"late success cannot reverse cancellation");
    const auto retried=storage.reserve_front_locked();
    require(retried!=0&&retried!=cancelled&&storage.copy_front_locked()=="first","retry uses a distinct receipt for the retained front");
    require(!storage.finish_reserved(cancelled,true)&&!storage.finish_reserved(cancelled,false),"stale completion cannot settle a newer retry");
    require(!storage.front_available_locked(),"retry remains exclusive after stale callbacks");
    storage.push_move_034660(third);
    require(storage.finish_reserved(retried,true),"current retry can commit delivery");
    require(!storage.finish_reserved(retried,false)&&!storage.finish_reserved(retried,true),"duplicate callbacks cannot reverse or repeat commit");
    require(storage.copy_front_locked()=="first","completion defers physical unlink until locked settlement");
    require(storage.front_available_locked()&&storage.copy_front_locked()=="second","settlement removes exactly the delivered front");
    const auto next=storage.reserve_front_locked();
    require(next!=0&&next!=retried&&!storage.finish_reserved(retried,true),"previous successful receipt cannot consume the following event");
    require(storage.finish_reserved(next,true)&&storage.front_available_locked(),"next delivery settles independently");
    require(storage.copy_front_locked()==std::string("third\0tail",10),"append during delivery preserves FIFO and explicit bytes");
    const auto last=storage.reserve_front_locked();
    require(storage.finish_reserved(last,true)&&!storage.front_available_locked(),"last committed event settles to empty");
    require(!storage.nonempty_relaxed()&&storage.reserve_front_locked()==0,"settled empty queue publishes no phantom event");
}
void competing_delivery_completions(){
    for(unsigned round=0;round<64;++round){
        eq::OwnedStorage storage;storage.construct_seed_copy("first");
        std::string second="second";storage.push_move_034660(second);
        const auto token=storage.reserve_front_locked();
        std::latch start{1};std::array<bool,2> accepted{};
        // Only finish_reserved is concurrent; deque access stays on this thread.
        std::jthread delivered([&]{start.wait();accepted[0]=storage.finish_reserved(token,true);});
        LatchRelease release_start{start};
        std::jthread cancelled([&]{start.wait();accepted[1]=storage.finish_reserved(token,false);});
        release_start.release();delivered.join();cancelled.join();
        require(accepted[0]!=accepted[1],"exactly one competing delivery outcome is accepted");
        require(!storage.finish_reserved(token,true)&&!storage.finish_reserved(token,false),"resolved receipt rejects both duplicate outcomes");
        require(storage.front_available_locked(),"winner leaves a retriable front or the next queued event");
        require(storage.copy_front_locked()==(accepted[0]?"second":"first"),"only successful winner removes the original front");
        const auto retry=storage.reserve_front_locked();
        require(retry!=0&&retry!=token&&!storage.finish_reserved(token,true),"late previous winner cannot settle the next reservation");
        require(storage.finish_reserved(retry,true),"new receipt remains usable after the race");
        if(accepted[0])require(!storage.front_available_locked(),"success followed by success consumes exactly two events");
        else {
            require(storage.front_available_locked()&&storage.copy_front_locked()=="second","cancel followed by retry consumes exactly one event");
            const auto tail=storage.reserve_front_locked();
            require(storage.finish_reserved(tail,true)&&!storage.front_available_locked(),"remaining tail can be independently delivered");
        }
        require(!storage.nonempty_relaxed(),"race and retry leave no lost or duplicated queue entry");
    }
}
void completion_during_persistence(){
    using namespace std::chrono_literals;
    for(const bool delivered:{false,true})for(const bool disk_failure:{false,true}){
        eq::OwnedStorage storage;std::mutex queue_mutex;
        storage.construct_seed_copy("first");const auto token=storage.reserve_front_locked();
        std::latch release_persistence{1};std::promise<void> entered;
        auto entered_future=entered.get_future();std::exception_ptr producer_error;
        std::string tail="persisted-tail";
        std::jthread producer([&]{
            bool announced=false;
            try {
                std::scoped_lock lock(queue_mutex);
                storage.push_persisted_locked(tail,[&]{
                    entered.set_value();announced=true;release_persistence.wait();
                    if(disk_failure)throw std::runtime_error("injected persistence failure");
                });
            }catch(...){producer_error=std::current_exception();if(!announced)entered.set_exception(producer_error);}
        });
        LatchRelease release_held_producer{release_persistence};
        const bool producer_entered=entered_future.wait_for(5s)==std::future_status::ready;
        if(!producer_entered){release_held_producer.release();producer.join();require(false,"producer reached the held persistence callback");}
        std::promise<bool> completed;auto completed_future=completed.get_future();
        std::jthread finisher([&]{completed.set_value(storage.finish_reserved(token,delivered));});
        const bool prompt=completed_future.wait_for(5s)==std::future_status::ready;
        // Release and join before asserting, including on a failed progress oracle.
        release_held_producer.release();producer.join();finisher.join();
        require(prompt,"completion returns while persistence still owns the queue mutex");
        require(completed_future.get(),"pending front resolves during another event's disk transaction");
        require(static_cast<bool>(producer_error)==disk_failure,"persistence outcome remains independent of delivery outcome");
        if(producer_error){
            try{std::rethrow_exception(producer_error);}
            catch(const std::runtime_error& error){require(std::string(error.what())=="injected persistence failure","only the injected disk failure is accepted");}
        }
        std::scoped_lock lock(queue_mutex);
        require(!storage.finish_reserved(token,!delivered),"opposite late outcome cannot change the completed receipt");
        if(!delivered){
            require(storage.front_available_locked()&&storage.copy_front_locked()=="first","cancelled front remains ahead of any persisted tail");
            const auto retry=storage.reserve_front_locked();
            require(retry!=token&&storage.finish_reserved(retry,true),"cancelled front can be retried after persistence finishes");
        }
        require(storage.front_available_locked()==!disk_failure,"only a successfully persisted tail survives front settlement");
        if(!disk_failure){
            require(storage.copy_front_locked()=="persisted-tail","disk commit preserves the exact tail after concurrent completion");
            const auto receipt=storage.reserve_front_locked();
            require(storage.finish_reserved(receipt,true)&&!storage.front_available_locked(),"persisted tail retires exactly once");
        }
        require(!storage.nonempty_relaxed(),"rollback or final delivery leaves the queue empty");
    }
}
}
int main(){try{getters();failures();owned_fifo();stale_delivery_receipts();competing_delivery_completions();completion_during_persistence();std::cout<<"PASS: "<<checks<<" event queue checks; synthetic guards, real completion threads and owned FIFO only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
