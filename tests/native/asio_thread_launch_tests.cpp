// SPDX-License-Identifier: GPL-3.0-only

#include <winsock2.h>
#include <windows.h>
#include <asio.hpp>
#include "laureondo/net/server.hpp"
#include "laureondo/net/session_policy.hpp"
#include "laureondo/platform/execution_windows.hpp"
#define private public
#include "laureondo/net/asio_service.hpp"
#undef private
#include <array>
#include <atomic>
#include <chrono>
#include <future>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <utility>

namespace asio_launch_fixture {
struct Failure final:std::system_error {
    unsigned attempt;
    explicit Failure(unsigned n):std::system_error(
        std::make_error_code(std::errc::resource_unavailable_try_again),
        "controlled Asio thread launch"),attempt(n){}
};
struct Probe {
    unsigned fail_on=0;
    std::atomic<unsigned> attempts{0},started{0},entered{0},finished{0},joins{0};
    void reset(unsigned failure) {
        fail_on=failure;attempts=0;started=0;entered=0;finished=0;joins=0;
    }
} probe;
class Thread {
    std::thread thread_;
public:
    Thread()noexcept=default;
    template<class Function> explicit Thread(Function&& function) {
        const auto n=++probe.attempts;
        if(n==probe.fail_on)throw Failure(n);
        thread_=std::thread([body=std::forward<Function>(function)]() mutable {
            struct Finish {~Finish(){++probe.finished;}} finish;
            ++probe.entered;body();
        });
        ++probe.started;
    }
    Thread(Thread&&)=default;
    Thread& operator=(Thread&&)=default;
    Thread(const Thread&)=delete;
    Thread& operator=(const Thread&)=delete;
    bool joinable()const noexcept{return thread_.joinable();}
    std::thread::id get_id()const noexcept{return thread_.get_id();}
    void join(){thread_.join();++probe.joins;}
};
}

// Only this executable selects the controlled launcher. It throws before the
// selected std::thread construction; earlier workers are real owned threads.
#define LAUREONDO_TEST_ASIO_THREAD ::asio_launch_fixture::Thread
#include "src/net/asio_service.cpp"
#undef LAUREONDO_TEST_ASIO_THREAD

namespace {
using namespace laureondo;
using namespace std::chrono_literals;
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Factory final:net::HandlerFactory {
    std::atomic<unsigned> creates{0},errors{0};
    std::unique_ptr<server::Handler> create()override {
        ++creates;throw std::logic_error("No session expected before runner publication");
    }
    void error(std::string_view,std::exception_ptr)noexcept override{++errors;}
};
struct Context {
    Factory factory;
    execution::MutexState mutex{};
    std::uint8_t ready=0,second=0,third=0;
    std::uint16_t port=0;
    execution::WindowsPlatform platform{+[](int){throw std::runtime_error("thread error");}};
    std::int64_t frequency=platform.query_performance_frequency();
    execution::Execution execution{mutex,{ready,second,third},frequency,platform};
    Context(){execution::initialize_mutex_04a698(mutex,2);}
};
struct PoolCleanup {
    net::AsioService::Impl& owner;
    ~PoolCleanup(){owner.stop_workers();}
};
bool any_worker(const net::AsioService::Impl& owner) {
    for(const auto& worker:owner.workers)if(worker.joinable())return true;
    return false;
}
void launch_failure(unsigned failure) {
    asio_launch_fixture::probe.reset(failure);
    Context context;
    net::AsioService::Impl owner(context.factory,context.execution,context.port,context.ready,{});
    PoolCleanup cleanup{owner};
    unsigned observed=0;
    try{owner.start();}
    catch(const asio_launch_fixture::Failure& error) {
        require(error.code()==std::make_error_code(std::errc::resource_unavailable_try_again),
                "original controlled launch error code propagates");
        observed=error.attempt;
    }
    // Capture before cleanup; a later stop must not make rollback checks pass.
    const bool no_workers=!any_worker(owner),no_runner=!owner.runner.joinable();
    const bool stopped=owner.jobs_stopping,had_guard=owner.guard.has_value();
    const auto attempts=asio_launch_fixture::probe.attempts.load();
    const auto started=asio_launch_fixture::probe.started.load();
    const auto entered=asio_launch_fixture::probe.entered.load();
    const auto finished=asio_launch_fixture::probe.finished.load();
    const auto joined=asio_launch_fixture::probe.joins.load();
    const bool unpublished=context.port==0&&context.ready==0&&
        context.factory.creates==0&&context.factory.errors==0;
    owner.stop();
    const auto after_stop=asio_launch_fixture::probe.joins.load();
    owner.stop();
    require(observed==failure&&attempts==failure,"exact selected launcher failed once");
    require(started==failure-1&&entered==started&&finished==started,
            "all earlier real threads have physically returned before failure escapes");
    require(joined==started&&no_workers&&no_runner&&stopped,
            "startup rollback joins every earlier thread before returning");
    if(failure>net::handler_policy::workers)
        require(!had_guard,"runner launch failure retires its unused IO work guard");
    require(after_stop==joined&&asio_launch_fixture::probe.joins.load()==joined,
            "explicit cleanup retries do not join any thread twice");
    require(!owner.guard&&owner.io.stopped()&&!any_worker(owner)&&!owner.runner.joinable(),
            "explicit stop retires remaining work guard and leaves no thread owner");
    require(unpublished,
            "launch failure publishes no listener, readiness or handler");
}
void fresh_pool() {
    asio_launch_fixture::probe.reset(0);
    Context context;
    net::AsioService::Impl owner(context.factory,context.execution,context.port,context.ready,{});
    std::atomic<unsigned> jobs{0};std::promise<void> completed;
    auto done=completed.get_future();
    PoolCleanup cleanup{owner};
    owner.start_workers();
    for(unsigned i=0;i<32;++i)owner.job([&]{if(++jobs==32)completed.set_value();});
    const bool finished=done.wait_for(5s)==std::future_status::ready;
    owner.stop_workers();
    require(finished&&jobs==32,"a fresh owner executes queued work after failed-owner cleanup");
    require(asio_launch_fixture::probe.attempts==net::handler_policy::workers&&
            asio_launch_fixture::probe.started==net::handler_policy::workers&&
            asio_launch_fixture::probe.finished==net::handler_policy::workers&&
            asio_launch_fixture::probe.joins==net::handler_policy::workers&&!any_worker(owner),
            "fresh pool owns and joins precisely its fixed worker count");
    require(context.factory.errors==0,"fresh pool has no hidden job exception");
}
}
int main()try{
    for(unsigned failure=1;failure<=laureondo::net::handler_policy::workers+1;++failure)
        launch_failure(failure);
    fresh_pool();
    std::cout<<"PASS: "<<checks<<" controlled Asio launcher checks; real threads, not OS launch-failure evidence\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
