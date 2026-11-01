// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config.hpp"
#include <atomic>
#include <iostream>
#include <thread>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    ConfigStore store;CHECK(store.snapshot()->achievements.empty());
    std::string source=R"("Config2" {"Achievements" {"AchievementNames" {"B" "first" "B" "second" "only" "named" "" "skip" "Nested" {"deep" "nested-name"}} "AchievementTotals" {"A" "2" "B" "-3" "B" "+4" "bad" "2147483648" "only" " 8" "" "7" "Nested" {"deep" "9"}}} "achievements" {"AchievementNames" {"wrong" "case"}} "Other" {"Achievements" {"AchievementNames" {"wrong" "path"}}}})";
    CHECK(store.replace(source)==ConfigResult::published);source.assign(source.size(),'!');const auto held=store.snapshot();const auto& a=held->achievements;
    CHECK(a.size()==5 && a[0].id=="A" && a[0].name.empty() && a[0].total==2);
    CHECK(a[1].id=="B" && a[1].name=="first" && a[1].total==-3 && a[2].id=="B" && a[2].name=="first" && a[2].total==4);
    CHECK(a[3].id=="deep" && a[3].name=="nested-name" && a[3].total==9 && a[4].id=="only" && a[4].total==1);
    for(const auto& r:a)CHECK(r.count==0 && r.progress==0 && r.description.empty() && r.how_to.empty() && r.image_id.empty() && r.grant_date=="0000-00-00T00:00:00" && r.expiration==r.grant_date);
    CHECK(store.replace(R"("Config2" {"Achievements" {"AchievementTotals" {"lo" "-2147483648" "hi" "+2147483647" "zero" "-0" "leading" "00007" "bad1" "-2147483649" "bad2" "+" "bad3" "1 " "bad4" "1.0" "bad5" ""}}})")==ConfigResult::published);
    const auto limits=store.snapshot();CHECK(limits->achievements.size()==4 && limits->achievements[0].total==INT32_MIN && limits->achievements[1].total==INT32_MAX && limits->achievements[2].total==0 && limits->achievements[3].total==7);
    CHECK(store.replace(R"("Config2" {"Achievements" {"AchievementNames" {"raw&amp;" "A&amp;B" "q\"" "slash\\"}}})")==ConfigResult::published);const auto raw=store.snapshot();CHECK(raw->achievements[0].id=="raw&amp;" && raw->achievements[0].name=="A&amp;B" && raw->achievements[1].id=="q\\\"");
    CHECK(store.replace(R"("Config2" {"Achievements" {"AchievementNames" {"partial" "x"}})")==ConfigResult::invalid && store.snapshot()==raw);
    CHECK(store.replace(std::string(ConfigStore::max_bytes+1,' '))==ConfigResult::too_large && store.snapshot()==raw);
    std::string many="\"Config2\" {\"Achievements\" {\"AchievementNames\" {";for(unsigned i=0;i<4096;++i)many+='"'+std::to_string(i)+"\" \"name\"";
    CHECK(store.replace(many+"}}}")==ConfigResult::published);const auto max=store.snapshot();CHECK(max->achievements.size()==4096);
    CHECK(store.replace(many+"\"over\" \"name\"}}}")==ConfigResult::resource && store.snapshot()==max);
    CHECK(store.replace(R"("Config2" {"Achievements" {"AchievementNames" {} "AchievementTotals" {}}})")==ConfigResult::published && store.snapshot()->achievements.empty());CHECK(held->achievements[1].name=="first" && raw->achievements[0].name=="A&amp;B");
    const char* yes=R"("Config2" {"Emulator" {"Online" "true"} "Content" {"A" {"Name" "alpha"}} "Entitlements" {"A" {"Group" "gA"}} "Achievements" {"AchievementNames" {"A" "alpha"}}})";
    const char* no=R"("Config2" {"Emulator" {"Online" "false"} "Content" {"B" {"Name" "beta"}} "Entitlements" {"B" {"Group" "gB"}} "Achievements" {"AchievementNames" {"B" "beta"}}})";
    CHECK(store.replace(yes)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(unsigned i=0;i<200;++i)if(store.replace(i%2?yes:no)!=ConfigResult::published)bad=true;});
    for(unsigned i=0;i<500;++i){const auto s=store.snapshot();if(s->achievements.size()!=1 || s->content.size()!=1 || s->entitlements.size()!=1 || s->achievements[0].id!=(s->online?"A":"B") || s->content[0].content_id!=s->achievements[0].id || s->entitlements[0].item_id!=s->achievements[0].id || s->content[0].display_name!=s->achievements[0].name)bad=true;}
    writer.join();CHECK(!bad);std::shared_ptr<const ConfigSnapshot> escaped;{ConfigStore local;CHECK(local.replace(yes)==ConfigResult::published);escaped=local.snapshot();}CHECK(escaped->achievements[0].name=="alpha");
    std::cout<<"ACHIEVEMENT_TOTALS_FIRST_DUPLICATES_I32_ATOMIC_SNAPSHOT_LIFETIME_PASS\n";
}
