// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime.hpp"
#include <atomic>
#include <iostream>
#include <latch>
#include <thread>
#include <utility>
using namespace preloader_l;
using namespace std::chrono_literals;
#define CHECK(x) do { if (!(x)) { std::cerr << "line " << __LINE__ << ": " #x "\n"; return 1; } } while (false)
struct Service {
    int id;
    std::vector<int>* events;
    bool start_ok = true;
    bool stop_ok = true;
    static bool start(void* p) noexcept { auto& s = *static_cast<Service*>(p); s.events->push_back(s.id); return s.start_ok; }
    static bool stop(void* p) noexcept { auto& s = *static_cast<Service*>(p); s.events->push_back(-s.id); return s.stop_ok; }
    Resource resource() { return {this, start, stop}; }
};
int main() {
    std::vector<int> events; events.reserve(64);
    {
        Service a{1,&events}, b{2,&events}, c{3,&events};
        Resource resources[]{a.resource(), b.resource(), c.resource()};
        Runtime runtime(resources);
        CHECK(!runtime.acquire());
        CHECK(runtime.start() == StartResult::started);
        CHECK(runtime.start() == StartResult::already_running);
        auto lease = runtime.acquire(); CHECK(lease);
        auto moved = std::move(lease); CHECK(!lease && moved);
        CHECK(runtime.stop(0ms) == StopResult::timeout);
        CHECK(runtime.snapshot().state == State::draining && runtime.snapshot().owned == 3);
        CHECK(!runtime.acquire());
        CHECK(events == std::vector<int>({1,2,3}));
        moved = {};
        c.stop_ok = false;
        CHECK(runtime.stop(0ms) == StopResult::cleanup_pending);
        CHECK(runtime.snapshot().owned == 3 && runtime.snapshot().state == State::cleanup_pending);
        CHECK(runtime.start() == StartResult::invalid_state);
        c.stop_ok = true;
        CHECK(runtime.stop(0ms) == StopResult::stopped);
        CHECK(events == std::vector<int>({1,2,3,-3,-3,-2,-1}));
        CHECK(runtime.stop(0ms) == StopResult::stopped);
        CHECK(runtime.start() == StartResult::invalid_state);
    }
    events.clear();
    {
        Service a{1,&events}, b{2,&events,false,false}, c{3,&events};
        Resource resources[]{a.resource(),b.resource(),c.resource()};
        Runtime runtime(resources);
        CHECK(runtime.start() == StartResult::cleanup_pending);
        CHECK(events == std::vector<int>({1,2,-2}));
        CHECK(runtime.snapshot().owned == 2);
        b.stop_ok = true;
        CHECK(runtime.stop(0ms) == StopResult::stopped);
        CHECK(events == std::vector<int>({1,2,-2,-2,-1}));
    }
    events.clear();
    {
        Service a{1,&events}, b{2,&events,false,true};
        Resource resources[]{a.resource(),b.resource()}; Runtime runtime(resources);
        CHECK(runtime.start() == StartResult::start_failed);
        CHECK(runtime.snapshot().owned == 0 && runtime.snapshot().state == State::stopped);
        CHECK(events == std::vector<int>({1,2,-2,-1}));
    }
    {
        Runtime runtime({});
        CHECK(runtime.start() == StartResult::started);
        std::atomic<int> entered{0};
        std::vector<std::thread> threads;
        for (int i=0; i<8; ++i) threads.emplace_back([&] {
            for (int j=0; j<2000; ++j) { auto lease=runtime.acquire(); if (lease) ++entered; }
        });
        for (auto& thread : threads) thread.join();
        CHECK(entered == 16000 && runtime.snapshot().active == 0);
        CHECK(runtime.stop(1000ms) == StopResult::stopped);
    }
    {
        struct Blocked {
            std::latch started{1}, release{1};
            static bool start(void* p) noexcept {
                auto& b=*static_cast<Blocked*>(p); b.started.count_down(); b.release.wait(); return true;
            }
            static bool stop(void*) noexcept { return true; }
        } blocked;
        Resource resource{&blocked,Blocked::start,Blocked::stop}; Runtime runtime({&resource,1});
        StartResult result{};
        std::thread starter([&] { result=runtime.start(); });
        blocked.started.wait();
        const bool safe = runtime.stop(0ms)==StopResult::busy && runtime.start()==StartResult::busy && !runtime.acquire();
        blocked.release.count_down(); starter.join();
        CHECK(safe && result == StartResult::started);
        CHECK(runtime.stop(0ms) == StopResult::stopped);
    }
    {
        Runtime runtime({}); CHECK(runtime.start()==StartResult::started);
        std::latch entered{1}, release{1};
        std::thread caller([&] { auto lease=runtime.acquire(); entered.count_down(); release.wait(); });
        entered.wait(); CHECK(runtime.stop(0ms)==StopResult::timeout);
        release.count_down(); caller.join();
        CHECK(runtime.stop(1000ms)==StopResult::stopped);
    }
    std::cout << "RUNTIME_PASS\n";
}
