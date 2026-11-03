// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/execution.hpp"
#include <atomic>
#include <barrier>
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
    std::uint32_t current_thread_id()override{thread_local const auto id=next.fetch_add(1);return id;}
    void acquire_srw_exclusive(std::uintptr_t&)override{gate.lock();}
    void release_srw_exclusive(std::uintptr_t&)override{gate.unlock();}
    std::int64_t query_performance_frequency()override{
        frequencies.fetch_add(1);
        if(cold_callers)cold_callers->arrive_and_wait();
        return 10000000;
    }
    std::int64_t query_performance_counter()override{return 1;}
    void sleep_milliseconds(std::uint32_t)override{}
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
int main()try {
    cold_frequency_cache();
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
