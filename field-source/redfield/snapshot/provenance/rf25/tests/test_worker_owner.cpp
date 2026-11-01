// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/worker_owner.hpp"
#include <iostream>
#include <latch>
#include <thread>
using namespace redfield;
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
int main(){
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
