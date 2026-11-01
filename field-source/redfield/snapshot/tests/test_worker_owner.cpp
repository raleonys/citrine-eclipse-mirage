// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/worker_owner.hpp"
#include <iostream>
#include <latch>
#include <thread>
#include <cstring>
using namespace preloader_l;
#define CHECK(x) do { if (!(x)) { std::cerr<<"line "<<__LINE__<<": " #x "\n"; return 1; } } while(false)
struct Backend {
    bool pin_ok=true,spawn_ok=true,cleanup_ok=true;
    unsigned close_failures=0,unpin_failures=0,pins=0,spawns=0,waits=0,closes=0,unpins=0,cleanups=0;
    WaitResult result=WaitResult::signaled;
    std::latch* waiting=nullptr;
    std::latch* release=nullptr;
    static bool pin(void* p,void*,void** out) noexcept { auto& b=*static_cast<Backend*>(p); ++b.pins; *out=b.pin_ok?&b:nullptr; return b.pin_ok; }
    static void* spawn(void* p,WorkerOwner*) noexcept { auto& b=*static_cast<Backend*>(p); ++b.spawns; return b.spawn_ok?&b:nullptr; }
    static WaitResult wait(void* p,void*,std::uint32_t) noexcept {
        auto& b=*static_cast<Backend*>(p);++b.waits;
        if(b.waiting){b.waiting->count_down();b.release->wait();}return b.result;
    }
    static bool close(void* p,void*) noexcept { auto& b=*static_cast<Backend*>(p);++b.closes;if(b.close_failures){--b.close_failures;return false;}return true; }
    static bool unpin(void* p,void*) noexcept { auto& b=*static_cast<Backend*>(p);++b.unpins;if(b.unpin_failures){--b.unpin_failures;return false;}return true; }
    static bool cleanup(void* p,std::uint32_t) noexcept { auto& b=*static_cast<Backend*>(p);++b.cleanups;return b.cleanup_ok; }
    WorkerOperations operations(){return {this,pin,spawn,wait,close,unpin,cleanup};}
};
struct OrderedBackend {
    char trace[64]{};
    unsigned used=0,cleanups=0,finalizers=0,closes=0,unpins=0;
    bool finalize_fails=false,close_fails=false,unpin_fails=false,reentry_busy=true;
    WorkerOwner* owner=nullptr;
    void note(char c) noexcept { trace[used++]=c;trace[used]=0; }
    static bool pin(void* p,void*,void** out) noexcept {
        auto& b=*static_cast<OrderedBackend*>(p);b.note('P');*out=p;return true;
    }
    static void* spawn(void* p,WorkerOwner* owner) noexcept {
        auto& b=*static_cast<OrderedBackend*>(p);b.note('S');b.owner=owner;return p;
    }
    static WaitResult wait(void* p,void*,std::uint32_t) noexcept {
        static_cast<OrderedBackend*>(p)->note('W');return WaitResult::signaled;
    }
    static bool cleanup(void* p,std::uint32_t) noexcept {
        auto& b=*static_cast<OrderedBackend*>(p);b.note('C');++b.cleanups;return true;
    }
    static bool finalize(void* p,std::uint32_t) noexcept {
        auto& b=*static_cast<OrderedBackend*>(p);b.note('F');++b.finalizers;
        b.reentry_busy=b.reentry_busy && b.owner->shutdown(0)==ShutdownResult::busy;
        return !b.finalize_fails;
    }
    static bool close(void* p,void*) noexcept {
        auto& b=*static_cast<OrderedBackend*>(p);b.note('H');++b.closes;return !b.close_fails;
    }
    static bool unpin(void* p,void*) noexcept {
        auto& b=*static_cast<OrderedBackend*>(p);b.note('U');++b.unpins;return !b.unpin_fails;
    }
    WorkerOperations operations() noexcept {return {this,pin,spawn,wait,close,unpin,cleanup,finalize};}
};
int main(){
    {
        OrderedBackend b;auto ops=b.operations();WorkerOwner owner;
        CHECK(owner.schedule(ops,&b));b.finalize_fails=true;
        CHECK(owner.shutdown(0)==ShutdownResult::service_cleanup_pending);
        CHECK(std::strcmp(b.trace,"PSWCF")==0 && b.closes==0 && b.unpins==0);
        b.finalize_fails=false;b.close_fails=true;
        CHECK(owner.shutdown(0)==ShutdownResult::handle_close_pending);
        b.close_fails=false;b.unpin_fails=true;
        CHECK(owner.shutdown(0)==ShutdownResult::pin_release_pending);
        b.unpin_fails=false;
        CHECK(owner.shutdown(0)==ShutdownResult::stopped);
        CHECK(std::strcmp(b.trace,"PSWCFWFHWFHUFU")==0);
        CHECK(b.cleanups==1 && b.finalizers==4 && b.closes==2 && b.unpins==2 && b.reentry_busy);
        CHECK(owner.shutdown(0)==ShutdownResult::stopped);
        CHECK(std::strcmp(b.trace,"PSWCFWFHWFHUFU")==0);
    }
    for(unsigned missing=0;missing<5;++missing) {
        OrderedBackend b;auto ops=b.operations();WorkerOwner owner;
        if(missing==0)ops.pin=nullptr;
        if(missing==1)ops.spawn=nullptr;
        if(missing==2)ops.wait=nullptr;
        if(missing==3)ops.close=nullptr;
        if(missing==4)ops.unpin=nullptr;
        CHECK(!owner.schedule(ops,&b) && owner.phase()==WorkerPhase::idle && b.used==0);
        ops=b.operations();CHECK(!owner.schedule(ops,nullptr) && b.used==0);
        CHECK(owner.schedule(ops,&b));owner.mark_worker_clean();
        CHECK(owner.shutdown(0)==ShutdownResult::stopped && b.cleanups==0 && b.finalizers==1);
    }
    {
        Backend b;auto ops=b.operations();WorkerOwner owner;
        CHECK(owner.schedule(ops,&b));CHECK(!owner.schedule(ops,&b));
        owner.mark_worker_clean(); // completed flag is never a signaled-thread proof
        b.result=WaitResult::timeout;
        CHECK(owner.shutdown(0)==ShutdownResult::timeout && owner.stop_requested());
        CHECK(b.closes==0 && b.unpins==0);
        b.result=WaitResult::failed;CHECK(owner.shutdown(1)==ShutdownResult::wait_failed);
        b.result=WaitResult::signaled;b.close_failures=1;b.unpin_failures=1;
        CHECK(owner.shutdown(10)==ShutdownResult::handle_close_pending && b.unpins==0);
        CHECK(owner.shutdown(10)==ShutdownResult::pin_release_pending && b.closes==2);
        CHECK(owner.shutdown(10)==ShutdownResult::stopped && b.closes==2 && b.unpins==2);
        CHECK(owner.shutdown(0)==ShutdownResult::stopped && b.unpins==2);
        CHECK(owner.phase()==WorkerPhase::stopped && !owner.schedule(ops,&b));
    }
    {
        Backend b;b.spawn_ok=false;auto ops=b.operations();WorkerOwner owner;
        CHECK(!owner.schedule(ops,&b));CHECK(b.pins==1 && b.unpins==0);
        CHECK(owner.phase()==WorkerPhase::failed);
        CHECK(owner.shutdown(0)==ShutdownResult::stopped && b.unpins==1 && b.waits==0);
    }
    {
        Backend b;b.pin_ok=false;auto ops=b.operations();WorkerOwner owner;
        CHECK(!owner.schedule(ops,&b));CHECK(b.spawns==0);
        CHECK(owner.shutdown(0)==ShutdownResult::stopped && b.unpins==0);
    }
    {
        Backend b;b.cleanup_ok=false;auto ops=b.operations();WorkerOwner owner;
        CHECK(owner.schedule(ops,&b));
        CHECK(owner.shutdown(10)==ShutdownResult::service_cleanup_pending);
        CHECK(b.closes==0 && b.unpins==0 && b.cleanups==1);
        b.cleanup_ok=true;CHECK(owner.shutdown(10)==ShutdownResult::stopped);
        CHECK(b.cleanups==2 && b.closes==1 && b.unpins==1);
    }
    {
        Backend b;std::latch waiting{1},release{1};b.waiting=&waiting;b.release=&release;
        auto ops=b.operations();WorkerOwner owner;CHECK(owner.schedule(ops,&b));owner.mark_worker_clean();
        ShutdownResult result{};std::thread closer([&]{result=owner.shutdown(1000);});
        waiting.wait();const auto competing=owner.shutdown(0);release.count_down();closer.join();
        CHECK(competing==ShutdownResult::busy && result==ShutdownResult::stopped);
        CHECK(b.closes==1 && b.unpins==1);
    }
    {
        Backend b;auto ops=b.operations();WorkerOwner owner;
        CHECK(owner.shutdown(0)==ShutdownResult::stopped);
        CHECK(!owner.schedule(ops,&b) && b.pins==0);
    }
    std::cout<<"WORKER_OWNER_PASS\n";
}
