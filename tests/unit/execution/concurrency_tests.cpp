#include "preloader_l/runtime/execution.hpp"
#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using namespace preloader_l::execution_recovered;
struct OS final:Platform {
    std::mutex gate;
    std::atomic<unsigned> next{1};
    std::uint32_t current_thread_id()override{thread_local const auto id=next.fetch_add(1);return id;}
    void acquire_srw_exclusive(std::uintptr_t&)override{gate.lock();}
    void release_srw_exclusive(std::uintptr_t&)override{gate.unlock();}
    std::int64_t query_performance_frequency()override{return 10000000;}
    std::int64_t query_performance_counter()override{return 1;}
    void sleep_milliseconds(std::uint32_t)override{}
    [[noreturn]]void raise_thread_error_04a648(int)override{throw std::runtime_error("unexpected thread error");}
};
int main()try {
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
    std::cout<<"PASS: 120000 contended critical sections across nonrecursive/recursive mutex types\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
