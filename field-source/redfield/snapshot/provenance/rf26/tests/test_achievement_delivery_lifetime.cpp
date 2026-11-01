// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/achievement_delivery.hpp"
#include "preloader_l/response_dispatch.hpp"
#include <array>
#include <atomic>
#include <iostream>
#include <thread>
#include <type_traits>
using namespace preloader_l;using D=PreparedAchievementDelivery;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
constexpr auto prepared=GrantDeliveryResult::prepared;
std::shared_ptr<const ConfigSnapshot> seed(){auto s=std::make_shared<ConfigSnapshot>();s->achievements.push_back({"A","Alpha&","","","","0000-00-00T00:00:00","0000-00-00T00:00:00",1,0,0});return s;}
bool request(xml::Request& r){std::string wire="<LSX><Request id='g'><GrantAchievement><AchievementId>A</AchievementId></GrantAchievement></Request></LSX>";const auto result=xml::parse(wire,r);wire.assign(wire.size(),'!');return result==xml::Result::ok;}
int main(){
    static_assert(!std::is_copy_constructible_v<D> && std::is_nothrow_move_constructible_v<D>);
    const auto initial=seed();AchievementSession a,b;CHECK(a.initialize(initial)==AchievementSession::InitializeResult::initialized && b.initialize(initial)==AchievementSession::InitializeResult::initialized);D one,two;
    {xml::Request r;CHECK(request(r) && prepare_achievement_delivery(a,r,"stamp",{},one)==prepared);}const auto pin=one.projected();const std::string response(one.response()),event(one.completion_event());CHECK(response.find("Alpha&amp;")!=std::string::npos && !event.empty());
    xml::Request r;CHECK(request(r) && prepare_achievement_delivery(b,r,"other",{},two)==prepared);one=std::move(two);CHECK(one.active() && !two.active() && two.response().empty() && two.completion_event().empty());CHECK(prepare_achievement_delivery(a,r,"retry",{},two)==prepared);two.rollback();D moved(std::move(one));CHECK(!one.active() && one.response().empty() && moved.active());CHECK(moved.commit() && b.snapshot()->achievements[0].count==1 && a.snapshot()==initial);moved.rollback();CHECK(pin->achievements[0].count==1);
    {D abandoned;CHECK(prepare_achievement_delivery(a,r,"abandon",{},abandoned)==prepared);}CHECK(prepare_achievement_delivery(a,r,"after-drop",{},one)==prepared);a.close();CHECK(!one.commit() && !one.active() && one.response().empty() && one.completion_event().empty());CHECK(response.find("stamp")!=std::string::npos && event.find("stamp")!=std::string::npos && pin->achievements[0].count==1);
    D orphan;{AchievementSession local;CHECK(local.initialize(initial)==AchievementSession::InitializeResult::initialized && prepare_achievement_delivery(local,r,"orphan",{},orphan)==prepared);}CHECK(!orphan.commit() && orphan.response().empty() && orphan.completion_event().empty());
    AchievementSession shared;CHECK(shared.initialize(initial)==AchievementSession::InitializeResult::initialized && prepare_achievement_delivery(shared,r,"held",{},one)==prepared);std::atomic<bool> bad=false;std::thread observer([&]{D blocked;if(shared.snapshot()!=initial || prepare_achievement_delivery(shared,r,"rival",{},blocked)!=GrantDeliveryResult::busy || blocked.active() || !blocked.response().empty() || !blocked.completion_event().empty())bad=true;});observer.join();CHECK(!bad && one.commit());const auto complete=shared.snapshot();CHECK(complete->achievements[0].count==1 && !one.completion_event().empty());one.rollback();CHECK(prepare_achievement_delivery(shared,r,"repeat",{},one)==prepared && one.completion_event().empty());one.rollback();
    std::array<char,512> out{};std::size_t n=9;CHECK(dispatch_response_with_config(r,*complete,out.data(),out.size(),&n)==ResponseResult::no_response && n==0);CHECK(xml::parse("<LSX><Request id='q'><QueryAchievements/></Request></LSX>",r)==xml::Result::ok);CHECK(prepare_achievement_delivery(shared,r,"",{},one)==GrantDeliveryResult::no_response && !one.active());
    std::cout<<"GRANT_OWNED_WIRE_DOM_DROP_MOVE_CANCEL_CLOSE_BUSY_UNBOUND_PASS\n";
}
