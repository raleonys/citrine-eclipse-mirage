#include "preloader_l/runtime/event_queue.hpp"
#include <iostream>
#include <limits>
#include <stdexcept>
#include <vector>
namespace eq=preloader_l::event_queue_recovered;
namespace {
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
template<class F>void rejects(F f,const char* why){bool hit=false;try{f();}catch(const std::exception&){hit=true;}require(hit,why);}
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
}
int main(){try{getters();failures();owned_fifo();std::cout<<"PASS: "<<checks<<" event queue checks; synthetic guards and chosen FIFO only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
