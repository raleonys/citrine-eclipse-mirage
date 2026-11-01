// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/achievement_session.hpp"
#include <iostream>
using namespace preloader_l;using Owner=AchievementSession;using Tx=Owner::PreparedGrant;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
constexpr auto prepared=Owner::PrepareResult::prepared;
std::shared_ptr<const ConfigSnapshot> seed(int32_t total){auto s=std::make_shared<ConfigSnapshot>();s->achievements.push_back({"A","name","","","","0000-00-00T00:00:00","0000-00-00T00:00:00",total,0,0});return s;}
int main(){
    Owner owner;CHECK(owner.initialize(seed(3))==Owner::InitializeResult::initialized);Tx tx;
    CHECK(owner.prepare("A",-1,"negative",tx)==prepared && tx.mutated() && !tx.completed() && tx.projected()->achievements[0].progress==-1 && tx.commit());CHECK(owner.prepare("A",4,"completed",tx)==prepared && tx.completed() && tx.projected()->achievements[0].progress==3 && tx.commit());
    for(int32_t increment:{0,-5}){Owner fresh;CHECK(fresh.initialize(std::make_shared<ConfigSnapshot>())==Owner::InitializeResult::initialized);CHECK(fresh.prepare("NEW",increment,"ignored",tx)==prepared && tx.inserted() && !tx.mutated() && !tx.completed());const auto p=tx.projected();CHECK(p->achievements[0].total==increment && p->achievements[0].count==0 && p->achievements[0].progress==0 && p->achievements[0].grant_date=="0000-00-00T00:00:00");CHECK(tx.commit() && fresh.snapshot()->achievements.size()==1);}
    Owner overflow;CHECK(overflow.initialize(seed(INT32_MAX))==Owner::InitializeResult::initialized);CHECK(overflow.prepare("A",INT32_MAX-1,"first",tx)==prepared && tx.commit());CHECK(overflow.prepare("A",10,"wrapped",tx)==prepared && tx.mutated() && !tx.completed() && tx.projected()->achievements[0].progress==INT32_MIN+8 && tx.commit());
    // Duplicates are retained; original find-active policy updates first match.
    auto duplicates=std::make_shared<ConfigSnapshot>(*seed(3));duplicates->achievements.push_back(duplicates->achievements[0]);Owner dup;CHECK(dup.initialize(duplicates)==Owner::InitializeResult::initialized);CHECK(dup.prepare("A",2,"first-only",tx)==prepared && tx.index()==0 && tx.commit());CHECK(dup.snapshot()->achievements[0].progress==2 && dup.snapshot()->achievements[1].progress==0);
    const auto before=dup.snapshot();CHECK(dup.prepare("",1,"",tx)==Owner::PrepareResult::invalid && !tx.active() && dup.snapshot()==before);CHECK(dup.prepare(std::string(Owner::max_text_bytes+1,'x'),1,"",tx)==Owner::PrepareResult::limit && !tx.active() && dup.snapshot()==before);
    // This passes the individual input bound, but fails aggregate staged text.
    CHECK(dup.prepare("A",1,std::string(Owner::max_text_bytes,'x'),tx)==Owner::PrepareResult::limit && !tx.active() && dup.snapshot()==before);CHECK(dup.prepare("A",1,"retry",tx)==prepared && tx.commit());
    auto maximum=std::make_shared<ConfigSnapshot>();maximum->achievements.resize(Owner::max_records);for(std::size_t i=0;i<maximum->achievements.size();++i)maximum->achievements[i].id=std::to_string(i);Owner capped;CHECK(capped.initialize(maximum)==Owner::InitializeResult::initialized);const auto cap=capped.snapshot();CHECK(capped.prepare("over",0,"",tx)==Owner::PrepareResult::limit && capped.snapshot()==cap);CHECK(capped.prepare("0",1,"",tx)==prepared && tx.commit());
    auto oversized=std::make_shared<ConfigSnapshot>(*maximum);oversized->achievements.emplace_back();Owner rejected;CHECK(rejected.initialize(oversized)==Owner::InitializeResult::limit && !rejected.snapshot());auto huge=std::make_shared<ConfigSnapshot>(*seed(1));huge->achievements[0].name.assign(Owner::max_text_bytes,'x');CHECK(rejected.initialize(huge)==Owner::InitializeResult::limit && !rejected.snapshot());CHECK(rejected.initialize(seed(1))==Owner::InitializeResult::initialized);
    std::cout<<"GRANT_POLICY_WRAP_NEGATIVE_DUPLICATE_FIRST_AGGREGATE_BUDGET_PASS\n";
}
