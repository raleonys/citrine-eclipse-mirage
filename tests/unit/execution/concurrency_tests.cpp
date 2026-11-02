// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/execution.hpp"
#include <atomic>
#include <barrier>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace preloader_l::execution;
struct OS final:Platform {
    std::mutex gate;
    std::atomic<unsigned> next{1};
    std::barrier<>* cold_callers=nullptr;
    std::atomic<unsigned> frequencies{0};
    // Set only by the startup-payload regression. Ordinary clock tests keep
    // their exact fixed tick and frequency counts unchanged.
    bool advancing_clock=false;
    std::atomic<std::int64_t> advanced_counter{0};
    std::function<void()> sleep_hook;
    std::uint32_t current_thread_id()override{thread_local const auto id=next.fetch_add(1);return id;}
    void acquire_srw_exclusive(std::uintptr_t&)override{gate.lock();}
    void release_srw_exclusive(std::uintptr_t&)override{gate.unlock();}
    std::int64_t query_performance_frequency()override{
        frequencies.fetch_add(1);
        if(cold_callers)cold_callers->arrive_and_wait();
        return 10000000;
    }
    std::int64_t query_performance_counter()override{
        return advancing_clock?advanced_counter.fetch_add(1'000'000)+1'000'000:1;
    }
    void sleep_milliseconds(std::uint32_t)override{if(sleep_hook)sleep_hook();}
    [[noreturn]]void raise_thread_error_04a648(int)override{throw std::runtime_error("unexpected thread error");}
};
void cold_frequency_cache() {
    OS os;MutexState mutex{};initialize_mutex_04a698(mutex,2);
    std::uint8_t first=0,second=0,third=0;std::int64_t cache=0;
    Execution execution{mutex,{first,second,third},cache,os};
    std::barrier callers(2);os.cold_callers=&callers;
    std::int64_t samples[2]{};
    // Both platform queries start before either may publish. TSan must see
    // atomic publication even when the caller supplied a cold shared cache.
    std::thread a([&]{samples[0]=execution.now_nanoseconds_036740();});
    std::thread b([&]{samples[1]=execution.now_nanoseconds_036740();});
    a.join();b.join();os.cold_callers=nullptr;
    if(samples[0]!=100||samples[1]!=100||cache!=10000000||os.frequencies!=2)
        throw std::runtime_error("cold frequency publication changed the sample or cache");
    if(execution.now_nanoseconds_036740()!=100||os.frequencies!=2)
        throw std::runtime_error("published frequency was queried again");
}
void startup_flag_payload_publication() {
    // A11-02: this product exposes three native flag bytes, not the donor's
    // four-bit Milestones observer. A preseeded/duplicate first flag and an
    // unrelated publication cannot substitute for missing worker phases.
    OS os;os.advancing_clock=true;
    MutexState mutex{};initialize_mutex_04a698(mutex,2);
    std::uint8_t first=1,second=0,third=0,unrelated=0;
    std::int64_t cache=10'000'000;
    Execution execution{mutex,{first,second,third},cache,os};
    std::uint32_t profile_payload=0,library_payload=0; // Worker-written before mutex release.
    std::mutex gate;std::condition_variable changed;
    bool entered_sleep=false,release_sleep=false;
    os.sleep_hook=[&] {
        std::unique_lock lock(gate);
        entered_sleep=true;changed.notify_all();
        if(!changed.wait_for(lock,std::chrono::seconds(3),[&]{return release_sleep;}))
            throw std::runtime_error("startup waiter sleep gate timed out");
    };
    std::exception_ptr waiter_error;
    std::atomic<bool> waiter_finished{false};
    std::thread waiter([&] {
        try {
            execution.wait_steps_036880(-1);
            if(profile_payload!=0x1357u||library_payload!=0x2468u)
                throw std::runtime_error("worker prefix payload not visible after final flag");
            waiter_finished.store(true,std::memory_order_release);
        }catch(...){waiter_error=std::current_exception();}
    });
    bool saw_sleep=false;
    {
        std::unique_lock lock(gate);
        saw_sleep=changed.wait_for(lock,std::chrono::seconds(2),[&]{return entered_sleep;});
    }
    if(!saw_sleep) {
        {std::lock_guard lock(gate);release_sleep=true;}
        changed.notify_all();waiter.join();
        throw std::runtime_error("waiter never entered the observed sleep boundary");
    }
    execution.finish_flag_036820(first);
    execution.finish_flag_036820(first);
    execution.finish_flag_036820(unrelated);
    const bool prematurely_completed=waiter_finished.load(std::memory_order_acquire);
    std::thread online([&] {
        profile_payload=0x1357u;
        execution.finish_flag_036820(second);
    });
    online.join();
    const bool completed_without_third=waiter_finished.load(std::memory_order_acquire);
    std::thread libraries([&] {
        library_payload=0x2468u;
        execution.finish_flag_036820(third);
    });
    libraries.join();
    {std::lock_guard lock(gate);release_sleep=true;}
    changed.notify_all();
    waiter.join();
    if(waiter_error)std::rethrow_exception(waiter_error);
    if(prematurely_completed||completed_without_third||
       !waiter_finished.load(std::memory_order_acquire)||
       first!=1||second!=1||third!=1||unrelated!=1||
       profile_payload!=0x1357u||library_payload!=0x2468u||
       mutex.count_4c!=0||mutex.owner_48!=0xffffffff)
        throw std::runtime_error("three-phase worker publication or ownership contract changed");
}
int main()try {
    cold_frequency_cache();startup_flag_payload_publication();
    for(auto type:{1u,2u,0x102u}) {
        OS os;MutexState mutex{};initialize_mutex_04a698(mutex,type);
        unsigned value=0;std::vector<std::thread> threads;
        for(unsigned i=0;i<4;++i)threads.emplace_back([&]{
            for(unsigned j=0;j<10000;++j){
                if(lock_04a6ac(mutex,os)!=0||mutex.count_4c!=1)std::terminate();
                if(type!=2){
                    if(lock_04a6ac(mutex,os)!=0||mutex.count_4c!=2)std::terminate();
                    unlock_04a6b4(mutex,os);
                }
                ++value;unlock_04a6b4(mutex,os);
            }
        });
        for(auto& worker:threads)worker.join();
        if(value!=40000||mutex.count_4c||mutex.owner_48!=0xffffffff)throw std::runtime_error("mutex ownership/count mismatch");
    }
    std::cout<<"PASS: cold frequency publication and 120000 contended critical sections\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
