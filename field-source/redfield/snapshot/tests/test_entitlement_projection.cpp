// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/config.hpp"
#include <atomic>
#include <iostream>
#include <thread>
using namespace preloader_l;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    ConfigStore store;CHECK(store.snapshot()->entitlements.empty());
    std::string source=R"("Config2" {"Entitlements" {"empty" {} "nested" {"N" {"Group" "ignored"}} "unknown" {"Other" "x"} "A" {"Group" "g" "Version" "1" "Type" "T" "EntitlementTag" "tag"} "B" {"Other" ""} "A" {"Group" "" "Version" "" "Type" "" "EntitlementTag" ""}} "entitlements" {"bad" {"Type" "bad"}} "Other" {"Entitlements" {"bad" {"Type" "bad"}}}})";
    CHECK(store.replace(source)==ConfigResult::published);source.assign(source.size(),'!');const auto held=store.snapshot();CHECK(held->entitlements.size()==4 && held->content.empty());const auto& e=held->entitlements;
    CHECK(e[0].item_id=="nested" && e[0].group.empty() && e[0].version=="0" && e[0].type=="DEFAULT" && e[0].entitlement_tag=="ONLINE_ACCESS");CHECK(e[1].item_id=="unknown" && e[2].item_id=="A" && e[3].item_id=="B");CHECK(e[2].group.empty() && e[2].version.empty() && e[2].type.empty() && e[2].entitlement_tag.empty());
    CHECK(store.replace(R"("Config2" {"Entitlements" {"" {"Group" "empty-id"} "x" {"Type" "one"} "X" {"Type" "two"}}})")==ConfigResult::published);CHECK(store.snapshot()->entitlements.size()==3 && store.snapshot()->entitlements[0].item_id.empty());
    CHECK(store.replace(R"("Config2" {"Entitlements" {"raw&amp;" {"Group" "A&amp;B" "Version" "a\"b"}}})")==ConfigResult::published);const auto raw=store.snapshot();CHECK(raw->entitlements[0].item_id=="raw&amp;" && raw->entitlements[0].group=="A&amp;B" && raw->entitlements[0].version=="a\\\"b");
    CHECK(store.replace(R"("Config2" {"Entitlements" {"A" {"Type" "partial"}})")==ConfigResult::invalid && store.snapshot()==raw);
    CHECK(store.replace(std::string(ConfigStore::max_bytes+1,' '))==ConfigResult::too_large && store.snapshot()==raw);
    std::string many="\"Config2\" {\"Entitlements\" {";for(unsigned i=0;i<4096;++i)many+='"'+std::to_string(i)+"\" {\"Group\" \"g\"}";
    CHECK(store.replace(many+"}}") ==ConfigResult::published);const auto maximum=store.snapshot();CHECK(maximum->entitlements.size()==4096);CHECK(store.replace(many+"\"over\" {\"Type\" \"t\"}}}")==ConfigResult::resource && store.snapshot()==maximum);
    CHECK(store.replace(R"("Config2" {"Entitlements" {"empty" {}}})")==ConfigResult::published && store.snapshot()->entitlements.empty());CHECK(held->entitlements[0].entitlement_tag=="ONLINE_ACCESS" && raw->entitlements[0].group=="A&amp;B");
    const char* yes=R"("Config2" {"Emulator" {"Online" "true"} "Content" {"A" {"Name" "alpha"}} "Entitlements" {"A" {"Group" "gA"}}})";const char* no=R"("Config2" {"Emulator" {"Online" "false"} "Content" {"B" {"Name" "beta"}} "Entitlements" {"B" {"Group" "gB"}}})";
    CHECK(store.replace(yes)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(unsigned i=0;i<200;++i)if(store.replace(i%2?yes:no)!=ConfigResult::published)bad=true;});
    for(unsigned i=0;i<500;++i){const auto s=store.snapshot();if(s->entitlements.size()!=1 || s->content.size()!=1 || s->entitlements[0].item_id!=s->content[0].content_id || s->entitlements[0].item_id!=(s->online?"A":"B") || s->entitlements[0].group!=(s->online?"gA":"gB"))bad=true;}
    writer.join();CHECK(!bad);
    std::shared_ptr<const ConfigSnapshot> escaped;{ConfigStore temporary;CHECK(temporary.replace(yes)==ConfigResult::published);escaped=temporary.snapshot();}CHECK(escaped->entitlements[0].group=="gA" && escaped->content[0].display_name=="alpha");
    std::cout<<"ENTITLEMENT_PROJECTION_DEFAULTS_EMPTY_REPLACE_ORDER_ATOMIC_CONTENT_LIFETIME_PASS\n";
}
