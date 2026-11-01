#pragma once
#include "publication.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <condition_variable>
#include <chrono>

namespace b19_test {
using namespace anadius;namespace sp=startup_publication;namespace ex=execution_recovered;
inline unsigned checks=0;
inline void require(bool value,const char* text){++checks;if(!value){std::cerr<<text<<'\n';throw std::runtime_error(text);}}
template<class E,class F>void rejects(F&& f,const char* text){bool seen=false;try{f();}catch(const E&){seen=true;}require(seen,text);}
struct Platform final:ex::Platform {
    SRWLOCK native=SRWLOCK_INIT;std::atomic<bool> held=false,throw_release=false;
    std::atomic<unsigned> frequencies=0,counters=0,sleeps=0;std::atomic<std::int64_t> ticks=0;
    std::function<void()> sleep_hook;
    std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}
    void acquire_srw_exclusive(std::uintptr_t&)override{::AcquireSRWLockExclusive(&native);held=true;}
    void release_srw_exclusive(std::uintptr_t&)override{held=false;::ReleaseSRWLockExclusive(&native);if(throw_release)throw std::runtime_error("own post-release exception");}
    std::int64_t query_performance_frequency()override{++frequencies;return 10000000;}
    std::int64_t query_performance_counter()override{if(held)throw std::logic_error("clock while native flag lock held");++counters;return ticks.fetch_add(100000);}
    void sleep_milliseconds(std::uint32_t)override{if(held)throw std::logic_error("sleep with native lock");++sleeps;if(sleep_hook)sleep_hook();}
    [[noreturn]] void raise_thread_error_04a648(int index)override{throw index;}
};
struct State {
    ex::MutexState mutex{};std::uint8_t server=0,online=0,libraries=0,other=0;std::int64_t cache=0;
    State(){ex::initialize_mutex_04a698(mutex,2);}
    ex::FlagReferences flags(){return {server,online,libraries};}
};
} // namespace b19_test
