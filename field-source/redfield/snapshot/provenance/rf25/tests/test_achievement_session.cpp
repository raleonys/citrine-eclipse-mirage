// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/achievement_session.hpp"
#include "redfield/achievement_response.hpp"
#include <array>
#include <iostream>
using namespace redfield;
using Owner=AchievementSession;using Tx=Owner::PreparedGrant;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
constexpr auto prepared=Owner::PrepareResult::prepared;
int main(){
    ConfigStore config;CHECK(config.replace(R"("Config2" {"Game" {"Name" "Local"} "Emulator" {"Online" "true"} "Content" {"G" {"Name" "game"}} "Entitlements" {"E" {"Group" "group"}} "Achievements" {"AchievementNames" {"A1" "Alpha"} "AchievementTotals" {"A1" "3"}}})")==ConfigResult::published);
    Owner owner;Tx tx;CHECK(owner.prepare("A1",2,"stamp",tx)==Owner::PrepareResult::not_initialized);CHECK(!owner.snapshot());CHECK(owner.initialize(nullptr)==Owner::InitializeResult::invalid);
    const auto seed=config.snapshot();CHECK(owner.initialize(seed)==Owner::InitializeResult::initialized && owner.snapshot()==seed);CHECK(owner.initialize(seed)==Owner::InitializeResult::already_initialized);
    constexpr auto stamp="2026-10-20T00:00:00";
    CHECK(owner.prepare("A1",2,stamp,tx)==prepared && tx.active() && tx.mutated() && !tx.completed() && !tx.inserted());const auto partial=tx.projected();CHECK(partial->achievements[tx.index()].progress==2 && partial->achievements[0].grant_date==stamp);CHECK(owner.snapshot()==seed && seed->achievements[0].progress==0);
    Tx competing;CHECK(owner.prepare("A1",1,stamp,competing)==Owner::PrepareResult::busy && !competing.active());CHECK(owner.prepare("A1",1,stamp,tx)==Owner::PrepareResult::invalid && tx.projected()==partial);
    tx.rollback();tx.rollback();CHECK(!tx.active() && !tx.commit() && owner.snapshot()==seed && partial->achievements[0].progress==2);
    CHECK(owner.prepare("A1",2,stamp,tx)==prepared && tx.commit() && !tx.active());const auto committed=owner.snapshot();CHECK(committed!=seed && committed->achievements[0].progress==2 && committed->achievements[0].count==0 && committed->achievements[0].grant_date==stamp);
    CHECK(committed->online && committed->content[0].display_name=="game" && committed->entitlements[0].group=="group" && committed->leaves.size()==seed->leaves.size());
    {Tx abort;CHECK(owner.prepare("A1",1,"later",abort)==prepared && abort.completed() && abort.projected()->achievements[0].count==1);}CHECK(owner.snapshot()==committed);
    CHECK(owner.prepare("A1",1,"later",tx)==prepared && tx.completed() && tx.commit());const auto complete=owner.snapshot();CHECK(complete->achievements[0].progress==3 && complete->achievements[0].count==1 && complete->achievements[0].grant_date=="later");
    CHECK(owner.prepare("A1",9,"ignored",tx)==prepared && !tx.mutated() && !tx.completed() && !tx.inserted() && tx.commit() && owner.snapshot()==complete);
    // Existing serializer can read owner publications; production dispatcher/TCP
    // still uses RF17 initial configuration and does not route GrantAchievement.
    xml::Request query;CHECK(xml::parse("<LSX><Request id='q'><QueryAchievements/></Request></LSX>",query)==xml::Result::ok);std::array<char,2048> out{};std::size_t n=0;CHECK(dispatch_achievement_response(query,*complete,out.data(),out.size(),&n)==ResponseResult::response);CHECK(std::string_view(out.data(),n).find("Count=\"1\" Progress=\"3\"")!=std::string_view::npos);
    CHECK(owner.prepare("NEW",1,stamp,tx)==prepared && tx.inserted() && tx.mutated() && tx.completed() && tx.index()==1);const auto inserted=tx.projected();CHECK(inserted->achievements[1].total==1 && inserted->achievements[1].name.empty() && inserted->achievements[1].expiration=="0000-00-00T00:00:00");tx.rollback();CHECK(owner.snapshot()==complete && owner.snapshot()->achievements.size()==1);
    CHECK(owner.prepare("NEW",1,stamp,tx)==prepared && tx.commit() && owner.snapshot()->achievements.size()==2);
    CHECK(config.replace(R"("Config2" {})")==ConfigResult::published && owner.snapshot()->content[0].display_name=="game");
    Owner independent;CHECK(independent.initialize(seed)==Owner::InitializeResult::initialized && independent.snapshot()->achievements[0].progress==0);CHECK(owner.snapshot()->achievements[0].progress==3);
    CHECK(xml::parse("<LSX><Request id='g'><GrantAchievement><AchievementId>A1</AchievementId></GrantAchievement></Request></LSX>",query)==xml::Result::ok);n=9;CHECK(dispatch_response_with_config(query,*owner.snapshot(),out.data(),out.size(),&n)==ResponseResult::no_response && n==0);
    std::cout<<"ACHIEVEMENT_SESSION_PREPARE_ABORT_COMMIT_QUERY_ISOLATION_PASS\n";
}
