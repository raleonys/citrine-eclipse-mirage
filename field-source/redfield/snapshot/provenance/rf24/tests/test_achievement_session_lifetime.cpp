// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/achievement_session.hpp"
#include <atomic>
#include <barrier>
#include <iostream>
#include <thread>
#include <type_traits>
using namespace redfield;using Owner=AchievementSession;using Tx=Owner::PreparedGrant;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
constexpr auto prepared=Owner::PrepareResult::prepared;
std::shared_ptr<const ConfigSnapshot> seed(){auto s=std::make_shared<ConfigSnapshot>();s->achievements.push_back({"A","Alpha","","","","0000-00-00T00:00:00","0000-00-00T00:00:00",1000,0,0});return s;}
int main(){
    static_assert(!std::is_copy_constructible_v<Tx> && std::is_nothrow_move_constructible_v<Tx> && !std::is_move_constructible_v<Owner>);
    Owner a,b;const auto initial=seed();CHECK(a.initialize(initial)==Owner::InitializeResult::initialized && b.initialize(initial)==Owner::InitializeResult::initialized);Tx one,two;
    CHECK(a.prepare("A",1,"one",one)==prepared && b.prepare("A",2,"two",two)==prepared);const auto pin=one.projected();one=std::move(two);CHECK(one.active() && !two.active() && one.projected()->achievements[0].progress==2);CHECK(a.prepare("A",3,"three",two)==prepared);two.rollback();Tx* self=&one;one=std::move(*self);CHECK(one.active() && one.commit() && b.snapshot()->achievements[0].progress==2 && a.snapshot()==initial);
    CHECK(a.prepare("A",4,"close",one)==prepared);const auto projected=one.projected();a.close();a.close();CHECK(!a.snapshot() && one.active() && !one.commit() && !one.active());CHECK(a.prepare("A",1,"",two)==Owner::PrepareResult::closed && a.initialize(initial)==Owner::InitializeResult::closed);CHECK(projected->achievements[0].progress==4 && pin->achievements[0].progress==1 && initial->achievements[0].progress==0);
    Tx orphan;std::shared_ptr<const ConfigSnapshot> escaped;{Owner local;CHECK(local.initialize(initial)==Owner::InitializeResult::initialized && local.prepare("A",5,"orphan",orphan)==prepared);escaped=orphan.projected();}CHECK(!orphan.commit() && escaped->achievements[0].progress==5);
    // No mutex is held across a reservation: other threads observe committed
    // state and get busy instead of blocking behind a simulated delivery wait.
    Owner shared;CHECK(shared.initialize(initial)==Owner::InitializeResult::initialized);Tx held;CHECK(shared.prepare("A",1,"pending",held)==prepared);std::atomic<bool> bad=false;std::thread observer([&]{Tx blocked;if(shared.snapshot()!=initial || shared.prepare("A",2,"",blocked)!=Owner::PrepareResult::busy)bad=true;});observer.join();CHECK(!bad && held.commit());
    // Exactly one winner at a simultaneous reservation boundary, with loser
    // retry after commit and immutable snapshots read during publication.
    std::barrier ready(3),attempted(3);std::atomic<unsigned> winners=0,busy=0;
    auto race=[&]{Tx tx;ready.arrive_and_wait();const auto r=shared.prepare("A",1,"race",tx);if(r==prepared)++winners;else if(r==Owner::PrepareResult::busy)++busy;else bad=true;attempted.arrive_and_wait();if(r==prepared && !tx.commit())bad=true;};
    std::thread t1(race),t2(race);ready.arrive_and_wait();attempted.arrive_and_wait();t1.join();t2.join();CHECK(!bad && winners==1 && busy==1 && shared.snapshot()->achievements[0].progress==2);
    std::thread writer([&]{for(unsigned i=0;i<100;++i){Tx tx;if(shared.prepare("A",1,"loop",tx)!=prepared || !tx.commit())bad=true;}});int32_t previous=0;
    for(unsigned i=0;i<500;++i){const auto s=shared.snapshot();const auto& record=s->achievements[0];if(record.progress<previous || record.progress>102 || record.count!=0 || record.name!="Alpha")bad=true;previous=record.progress;}
    writer.join();CHECK(!bad && shared.snapshot()->achievements[0].progress==102);
    Tx finishing;CHECK(shared.prepare("A",1,"race-close",finishing)==prepared);std::barrier close_start(3);std::atomic<bool> committed=false;std::thread commit_thread([&]{close_start.arrive_and_wait();committed=finishing.commit();});std::thread close_thread([&]{close_start.arrive_and_wait();shared.close();});close_start.arrive_and_wait();commit_thread.join();close_thread.join();CHECK(!shared.snapshot() && !finishing.active());(void)committed;
    std::cout<<"ACHIEVEMENT_RESERVATION_MOVE_CLOSE_ORPHAN_THREAD_PUBLICATION_PASS\n";
}
