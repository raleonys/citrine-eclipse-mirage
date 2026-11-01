// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/pending_session_cleanup.hpp"
#include "preloader_l/worker_owner.hpp"
#include <asio/ip/tcp.hpp>
#include <filesystem>
#include <iostream>
#include <limits>
#include <thread>
using namespace preloader_l;
using namespace std::chrono_literals;
#define CHECK(x) do { if (!(x)) { std::cerr<<__LINE__<<": " #x "\n"; return 1; } } while(false)

// Model only hook preflight ownership. Transport, budget, WorkerOwner and the
// production cleanup helper below are real; this is not a Win32 detour test.
struct HookProbe {
    bool blocked=true;
    std::uint32_t supplied=0;
    bool stop(std::uint32_t timeout) noexcept { supplied=timeout;return !blocked; }
};
struct Backend {
    HookProbe hooks;
    std::atomic<LocalService*> pending{nullptr};
    unsigned closes=0,unpins=0;
    bool consume_wait=false,cleanup_seen=false;
    std::uint32_t cleanup_budget=0;
    static bool pin(void* p,void*,void** out) noexcept {*out=p;return true;}
    static void* spawn(void* p,WorkerOwner*) noexcept {return p;}
    static WaitResult wait(void* p,void*,std::uint32_t) noexcept {
        if(static_cast<Backend*>(p)->consume_wait)std::this_thread::sleep_for(30ms);
        return WaitResult::signaled;
    }
    static bool close(void* p,void*) noexcept {++static_cast<Backend*>(p)->closes;return true;}
    static bool unpin(void* p,void*) noexcept {++static_cast<Backend*>(p)->unpins;return true;}
    static bool cleanup(void* p,std::uint32_t remaining) noexcept {
        auto& b=*static_cast<Backend*>(p);b.cleanup_seen=true;b.cleanup_budget=remaining;
        return cleanup_pending_session(b.hooks,b.pending,remaining);
    }
    WorkerOperations operations() noexcept {return {this,pin,spawn,wait,close,unpin,cleanup};}
};
int main() {
    const auto start=ShutdownBudget::Clock::now();
    CHECK(ShutdownBudget(0,start).remaining(start)==0);
    ShutdownBudget finite(100,start);
    CHECK(finite.remaining(start)==100);
    CHECK(finite.remaining(start+25ms)==75);
    CHECK(finite.remaining(start+999us)==99);
    CHECK(finite.remaining(start+100ms)==0 && finite.remaining(start+200ms)==0);
    const auto infinite=std::numeric_limits<std::uint32_t>::max();
    CHECK(ShutdownBudget(infinite,start).remaining(start+24h)==infinite);
    {
        Backend b;b.consume_wait=true;auto ops=b.operations();WorkerOwner owner;
        CHECK(owner.schedule(ops,&b));
        CHECK(owner.shutdown(10)==ShutdownResult::service_cleanup_pending);
        CHECK(b.cleanup_seen && b.cleanup_budget==0 && b.hooks.supplied==0);
        CHECK(b.closes==0 && b.unpins==0);
        b.consume_wait=false;b.hooks.blocked=false;
        CHECK(owner.shutdown(infinite)==ShutdownResult::stopped);
        CHECK(b.cleanup_budget==infinite && b.hooks.supplied==infinite);
    }
    for(unsigned mode=0;mode<2;++mode) {
        Backend b;auto ops=b.operations();WorkerOwner owner;
        auto* service=new LocalService(std::filesystem::temp_directory_path()/
            "PRELOADER_L_RF21_CLEANUP_REGRESSION"/"preloader_l.dll");
        b.pending.store(service);
        CHECK(service->start());
        asio::io_context io;asio::ip::tcp::socket client(io);
        client.connect({asio::ip::address_v4::loopback(),service->snapshot().port});
        const auto until=ShutdownBudget::Clock::now()+3s;
        while(service->snapshot().active!=1 && ShutdownBudget::Clock::now()<until)service->poll_for(1ms);
        CHECK(service->snapshot().active==1);
        CHECK(owner.schedule(ops,&b));
        CHECK(owner.shutdown(mode?0:1000)==ShutdownResult::service_cleanup_pending);
        CHECK(b.pending.load()==service && b.closes==0 && b.unpins==0);
        CHECK(mode?b.hooks.supplied==0:b.hooks.supplied>0);
        b.hooks.blocked=false;
        if(mode) {
            // A zero-budget poll may make progress, but must never destroy an
            // undrained service or drop its retained module pin.
            const auto zero=owner.shutdown(0);
            CHECK(b.cleanup_budget==0 && b.hooks.supplied==0);
            CHECK(zero==ShutdownResult::stopped || zero==ShutdownResult::service_cleanup_pending);
            if(zero==ShutdownResult::service_cleanup_pending)
                CHECK(b.pending.load()==service && b.closes==0 && b.unpins==0);
        }
        // One finite retry must drain the actual outstanding accept/read/cancel
        // completions. The old exact DLL callback fails this check with code10.
        CHECK(owner.shutdown(1000)==ShutdownResult::stopped);
        CHECK(b.pending.load()==nullptr && b.closes==1 && b.unpins==1);
        CHECK(owner.shutdown(0)==ShutdownResult::stopped && b.unpins==1);
    }
    std::cout<<"PENDING_SESSION_REAL_TCP_BLOCKED_OWNER_FINITE_RETRY_ZERO_EXHAUSTED_INFINITE_BUDGET_PASS\n";
}
