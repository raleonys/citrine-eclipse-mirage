// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config.hpp"
#include <atomic>
#include <iostream>
#include <thread>
using namespace redfield;
#define CHECK(x) do{if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
int main(){
    ConfigStore store;CHECK(store.snapshot()->content.empty());
    std::string source=R"("Config2" {"Emulator" {"Online" "true"} "Content" {
        "empty" {} "nested" {"N" {"Name" "ignored"}} "unknown" {"x" "v"}
        "A" {"Name" "First" "Version" "1"} "B" {"Name" ""}
        "A" {"Name" "" "Version" "" "State" "REMOTE"} "A" {"Name" "Last"}
    } "content" {"bad" {"Name" "wrong case"}} "Other" {"Content" {"bad" {"Name" "wrong section"}}}})";
    CHECK(store.replace(source)==ConfigResult::published);source.assign(source.size(),'!');const auto held=store.snapshot();CHECK(held->online && held->content.size()==4);
    const auto& c=held->content;CHECK(c[0].content_id=="nested" && c[0].display_name=="GAME" && c[0].version=="0" && c[0].state=="INSTALLED");CHECK(c[1].content_id=="unknown" && c[1].display_name=="GAME");CHECK(c[2].content_id=="A" && c[2].display_name=="Last" && c[2].version=="1" && c[2].state=="REMOTE");CHECK(c[3].content_id=="B" && c[3].display_name=="GAME");
    CHECK(store.replace(R"("Config2" {"Content" {"" {"Name" "empty-id"} "case" {"Name" "lower"} "Case" {"Name" "upper"}}})")==ConfigResult::published);CHECK(store.snapshot()->content.size()==3 && store.snapshot()->content[0].content_id.empty());
    CHECK(store.replace(R"("Config2" {"Content" {"raw&amp;" {"Name" "A&amp;B" "Version" "a\"b"}}})")==ConfigResult::published);const auto raw=store.snapshot();CHECK(raw->content[0].content_id=="raw&amp;" && raw->content[0].display_name=="A&amp;B" && raw->content[0].version=="a\\\"b");
    CHECK(store.replace(R"("Config2" {"Content" {"x" {"Name" "partial"}})")==ConfigResult::invalid && store.snapshot()==raw);
    const std::string too_big(ConfigStore::max_bytes+1,' ');CHECK(store.replace(too_big)==ConfigResult::too_large && store.snapshot()==raw);
    std::string many="\"Config2\" {\"Content\" {";
    for(unsigned i=0;i<4096;++i)many+='"'+std::to_string(i)+"\" {\"Name\" \"n\"}";
    const auto full=many+"}}";CHECK(full.size()<ConfigStore::max_bytes && store.replace(full)==ConfigResult::published);const auto maximum=store.snapshot();CHECK(maximum->content.size()==4096);
    CHECK(store.replace(many+"\"over\" {\"Name\" \"n\"}}}")==ConfigResult::resource && store.snapshot()==maximum);
    CHECK(store.replace(R"("Config2" {"Content" {"empty" {}}})")==ConfigResult::published && store.snapshot()->content.empty());CHECK(held->content[2].display_name=="Last" && raw->content[0].display_name=="A&amp;B");
    const char* a=R"("Config2" {"Emulator" {"Online" "true"} "Content" {"A" {"Name" "alpha"}}})";const char* b=R"("Config2" {"Emulator" {"Online" "false"} "Content" {"B" {"Name" "beta"}}})";
    CHECK(store.replace(a)==ConfigResult::published);std::atomic<bool> bad=false;std::thread writer([&]{for(int i=0;i<200;++i)if(store.replace(i%2?a:b)!=ConfigResult::published)bad=true;});
    for(unsigned i=0;i<500;++i){const auto s=store.snapshot();if(s->content.size()!=1 || s->content[0].content_id!=(s->online?"A":"B") || s->content[0].display_name!=(s->online?"alpha":"beta"))bad=true;}
    writer.join();CHECK(!bad);
    std::shared_ptr<const ConfigSnapshot> escaped;
    {ConfigStore temporary;CHECK(temporary.replace(a)==ConfigResult::published);escaped=temporary.snapshot();}
    CHECK(escaped->content[0].display_name=="alpha");
    std::cout<<"CONTENT_TYPED_PROJECTION_DEFAULTS_ORDER_DUPLICATES_ATOMIC_LIMIT_LIFETIME_PASS\n";
}
