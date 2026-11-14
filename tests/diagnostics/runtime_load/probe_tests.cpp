// SPDX-License-Identifier: GPL-3.0-only
#include "application_probe.hpp"
#include <atomic>
#include <future>
#include <iostream>
#include <latch>
#include <memory>
#include <stdexcept>
using laureondo::benchmark::PerMessageJobs;
namespace {
void require(bool value,const char* why){if(!value)throw std::runtime_error(why);}
void ordinary_jobs(){
    PerMessageJobs jobs;std::atomic<unsigned> calls{0},errors{0};
    const auto before=laureondo::benchmark::created_message_threads.load();
    for(unsigned i=0;i<64;++i)jobs.submit([&]{++calls;},[&](std::exception_ptr)noexcept{++errors;});
    jobs.stop();jobs.stop();
    require(calls==64&&errors==0,"control lost work or reported spurious failures");
    require(laureondo::benchmark::created_message_threads-before==64,"one thread per admitted job required");
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
void application_observation(){
    using namespace laureondo::benchmark;
    {ApplicationRequest ignored;ApplicationLock lock(Clock::now());}
    require(application_record==nullptr,"disabled observation must not retain a TLS record");
    ApplicationMeasurements measured(64);application_measurements=&measured;
    PerMessageJobs jobs;std::mutex state;
    for(unsigned i=0;i<64;++i)jobs.submit([&]{
        ApplicationRequest request;
        const auto waiting=Clock::now();std::lock_guard lock(state);ApplicationLock timing(waiting);
    },[&](std::exception_ptr)noexcept{++measured.errors;});
    jobs.stop();application_measurements=nullptr;
    require(measured.used==64&&measured.active==0&&measured.errors==0,"Application observations must drain and count every request");
    for(const auto& record:measured.records)require(record.lock_calls==1&&record.lock_wait+record.lock_hold<=record.handler&&record.thread!=std::thread::id{},"lock samples must belong to their measured handler");
    ApplicationMeasurements failed(1);application_measurements=&failed;
    try{ApplicationRequest request;throw std::runtime_error("controlled Application failure");}catch(const std::runtime_error&){}
    {ApplicationRequest overflow;}
    application_measurements=nullptr;
    require(failed.used==2&&failed.errors==2&&failed.active==0&&application_record==nullptr,"exceptions and sample overflow must invalidate measurements without retaining TLS ownership");
}
}
int main()try{ordinary_jobs();failed_job_lifetime();application_observation();std::cout<<"PASS: scheduler ownership/drain and Application measurement contracts\n";}
catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
