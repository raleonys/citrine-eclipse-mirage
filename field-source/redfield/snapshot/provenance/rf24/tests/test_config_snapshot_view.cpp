// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config_snapshot_view.hpp"
#include <iostream>
#include <type_traits>
using namespace redfield;
#define CHECK(x) do {if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
static_assert(!std::is_constructible_v<ConfigSnapshotView,ConfigSnapshot&&>);
static_assert(!std::is_constructible_v<ConfigSnapshotView,const ConfigSnapshot&&>);
std::string_view text(config::ValueView value) {return {value.data,value.size};}
int main() {
    ConfigStore store;
    CHECK(store.replace(R"("Config2" {
      "Game" { "Name" "first" "GameName" "second" "Name" ""
        "Language" "raw&amp;" "Nested" { "Language" "nested" } }
      "Other" { "Language" "wrong-section" }
      "game" { "Name" "wrong-case" }
      "Emulator" { "Online" "true" "Online" "1" }
    })")==ConfigResult::published);
    const auto held=store.snapshot();const ConfigSnapshotView view{*held};
    bool fallback=true;
    auto value=view.or_default("Game",{"Name","GameName"},"fallback",&fallback);
    CHECK(value.present && value.size==0 && !fallback);
    CHECK(text(view.find_any("Game",{"GameName","Name"}))=="");
    CHECK(text(view.find("Game","GameName"))=="second");
    value=view.find("Game","Language");CHECK(text(value)=="raw&amp;");
    const auto* raw=value.data;
    CHECK(raw==held->leaves[3].raw_value.data());
    CHECK(!view.find("Game","missing").present);
    CHECK(!view.find_any("Game",{}).present);
    value=view.or_default("Game","missing","fallback",&fallback);
    CHECK(text(value)=="fallback" && fallback);
    CHECK(!view.find("GAME","Language").present && !view.find("Game","language").present);
    CHECK(held->online); // Raw numeric spelling must not alter normalized Online.
    CHECK(text(view.find("Emulator","Online"))=="1");
    CHECK(store.replace(R"("Config2" {"Game" {"Language" "new"}})")==ConfigResult::published);
    CHECK(text(view.find("Game","Language"))=="raw&amp;" && view.find("Game","Language").data==raw);
    CHECK(text(ConfigSnapshotView{*store.snapshot()}.find("Game","Language"))=="new");
    const auto before=store.snapshot();
    CHECK(store.replace(R"("Config2" {"Game" {"Name" "partial"})")==ConfigResult::invalid);
    CHECK(store.snapshot()==before && text(view.find("Game","Language"))=="raw&amp;");
    // Independent forward last-write oracle checks aliases across interleaved
    // sections/depths without relying on lookup's reverse traversal.
    ConfigSnapshot manual;
    std::string expected;
    for(unsigned i=0;i<500;++i) {
        const bool direct=i%3!=0;const bool section=i%5!=0;const bool alias=i%2!=0;
        ConfigLeaf leaf{{section?"Game":"Other"},alias?"Name":"GameName",std::to_string(i)};
        if(!direct)leaf.path.push_back("Nested");
        if(direct && section)expected=leaf.raw_value;
        manual.leaves.push_back(std::move(leaf));
        const auto found=ConfigSnapshotView{manual}.find_any("Game",{"GameName","Name"});
        CHECK(expected.empty()?!found.present:text(found)==expected);
    }
    std::cout<<"CONFIG_SNAPSHOT_BORROWED_DIRECT_LAST_ALIAS_EMPTY_RAW_PINNED_GENERATION_PASS\n";
}
