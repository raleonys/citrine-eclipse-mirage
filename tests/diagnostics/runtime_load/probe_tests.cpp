// SPDX-License-Identifier: GPL-3.0-only
#include "probe.hpp"
#include <atomic>
#include <future>
#include <iostream>
#include <latch>
#include <memory>
#include <stdexcept>
using citrine_eclipse_mirage::benchmark::PerMessageJobs;
namespace {
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
void ordinary_jobs(){
    PerMessageJobs jobs;std::atomic<unsigned> calls{0},errors{0};
    const auto before=citrine_eclipse_mirage::benchmark::created_message_threads.load();
    for(unsigned i=0;i<64;++i)jobs.submit([&]{++calls;},[&](std::exception_ptr)noexcept{++errors;});
    jobs.stop();jobs.stop();
    require(calls==64&&errors==0,"control lost work or reported spurious failures");
    require(citrine_eclipse_mirage::benchmark::created_message_threads-before==64,"one thread per admitted job required");
    bool rejected=false;try{jobs.submit([]{},[](std::exception_ptr)noexcept{});}catch(const std::runtime_error&){rejected=true;}
    require(rejected,"control admitted work after stop");
}
void failed_job_lifetime(){
    std::atomic<unsigned> retired{0},errors{0};
    std::latch entered{1},release{1};
    auto owner=std::shared_ptr<int>(new int(7),[&](int* p){delete p;++retired;});
    PerMessageJobs jobs;
    jobs.submit([owner,&entered,&release]{entered.count_down();release.wait();throw std::runtime_error("injected handler failure");},
                [&](std::exception_ptr error)noexcept{try{std::rethrow_exception(error);}catch(const std::runtime_error&){++errors;}catch(...){}});
    owner.reset();entered.wait();
    auto stopped=std::async(std::launch::async,[&]{jobs.stop();return retired.load();});
    const bool held=stopped.wait_for(std::chrono::milliseconds(20))==std::future_status::timeout;
    release.count_down(); // Always unblock before inspecting any assertion.
    const auto released_before_stop_return=stopped.get();
    require(held&&released_before_stop_return==1&&retired==1&&errors==1,"stop must drain failed job and release its capture exactly once");
}
}
int main()try{ordinary_jobs();failed_job_lifetime();std::cout<<"PASS: per-message control completion, exception, capture lifetime and idempotent stop checks\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
