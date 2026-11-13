// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "probe.hpp"
#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace citrine_eclipse_mirage::benchmark {
struct ApplicationRecord {
    std::uint64_t queue=0,handler=0,lock_wait=0,lock_hold=0;
    unsigned lock_calls=0;
    std::thread::id thread;
};
struct ApplicationMeasurements {
    std::vector<ApplicationRecord> records;
    std::atomic<unsigned> used{0},active{0},peak{0},errors{0};
    explicit ApplicationMeasurements(unsigned count):records(count){}
};
inline std::atomic<ApplicationMeasurements*> application_measurements{nullptr};
inline thread_local ApplicationRecord* application_record=nullptr;
inline void application_fault(std::string& bytes) {
    const auto* fault=std::getenv("RUNTIME_BENCHMARK_FAULT");
    if(application_measurements.load()&&fault&&std::string_view(fault)=="corrupt-response"&&!bytes.empty())bytes[0]='!';
}
// Instrument only prepare_response, the real Session delivery path. Handshake,
// warmup and final validation requests run with observation disabled.
class ApplicationRequest final {
    ApplicationMeasurements* measurements_=application_measurements.load();
    ApplicationRecord record_;
    Clock::time_point begin_=Clock::now();
    int exceptions_=std::uncaught_exceptions();
public:
    ApplicationRequest() {
        if(!measurements_)return;
        record_.queue=queue_wait_ns;record_.thread=std::this_thread::get_id();
        application_record=&record_;
        const auto count=++measurements_->active;
        auto peak=measurements_->peak.load();
        while(count>peak&&!measurements_->peak.compare_exchange_weak(peak,count)){}
    }
    ~ApplicationRequest() {
        if(!measurements_)return;
        application_record=nullptr;
        record_.handler=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-begin_).count();
        const auto slot=measurements_->used.fetch_add(1);
        if(slot<measurements_->records.size())measurements_->records[slot]=record_;
        else ++measurements_->errors;
        if(std::uncaught_exceptions()!=exceptions_)++measurements_->errors;
        --measurements_->active;
    }
    ApplicationRequest(const ApplicationRequest&)=delete;
    ApplicationRequest& operator=(const ApplicationRequest&)=delete;
};
class ApplicationLock final {
    ApplicationRecord* record_=application_record;
    Clock::time_point acquired_=Clock::now();
public:
    explicit ApplicationLock(Clock::time_point waiting) {
        if(record_){
            ++record_->lock_calls;
            record_->lock_wait+=std::chrono::duration_cast<std::chrono::nanoseconds>(acquired_-waiting).count();
        }
    }
    ~ApplicationLock() {
        if(record_)record_->lock_hold+=std::chrono::duration_cast<std::chrono::nanoseconds>(Clock::now()-acquired_).count();
    }
};
}
