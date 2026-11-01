// SPDX-License-Identifier: GPL-3.0-only
#include "native/worker_observation.hpp"
#include <cassert>
#include <thread>
#include <vector>

namespace { constinit preloader_l::testing::WorkerObservation observation; }
int main() {
    using namespace preloader_l;
    assert(observation.status(WorkerPhase::idle,HookPhase::idle,0)==(255u<<8));
    assert(observation.transport()==0 && observation.allow_close() && observation.allow_unpin());
    observation.configuration_result(FileConfigResult::loaded);
    observation.service_ready(true,65535);
    const auto status=observation.status(WorkerPhase::ready,HookPhase::committed,0x1ffff);
    assert((status&255u)==static_cast<unsigned>(WorkerPhase::ready));
    assert(((status>>8)&255u)==static_cast<unsigned>(FileConfigResult::loaded));
    assert(((status>>16)&1u)==1 && ((status>>17)&0xffffu)==65535);
    assert(((status>>33)&7u)==static_cast<unsigned>(HookPhase::committed));
    assert(((status>>40)&0xffffu)==65535);
    observation.transport_progress(0x1ffff,0x10001,0x100000003ull);
    assert(observation.transport()==0xffff000100000003ull);
    observation.service_stopped();
    assert(((observation.status(WorkerPhase::finished,HookPhase::stopped,0)>>17)&0xffffu)==0);
    assert(observation.transport()==0xffff000100000003ull); // lifetime totals are retained
    observation.set_faults(2,1);
    assert(!observation.allow_close() && !observation.allow_close() && observation.allow_close());
    assert(!observation.allow_unpin() && observation.allow_unpin());
    observation.set_faults(1000,700);std::atomic<unsigned> close{0},unpin{0};
    std::vector<std::thread> threads;
    for(unsigned i=0;i<8;++i)threads.emplace_back([&]{for(unsigned n=0;n<1100;++n){
        if(!observation.allow_close())++close;
        if(!observation.allow_unpin())++unpin;
    }});
    for(auto& thread:threads)thread.join();
    assert(close==1000 && unpin==700 && observation.allow_close() && observation.allow_unpin());
    observation.set_faults(3,4);observation.set_faults(0,0);
    assert(observation.allow_close() && observation.allow_unpin());
    observation.service_ready(false,1234);
    assert(((observation.status(WorkerPhase::ready,HookPhase::committed,0)>>16)&1u)==0);
}
