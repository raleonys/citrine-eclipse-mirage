// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/hook_configuration.hpp"
#include <cassert>
#include <string_view>
int main() {
    using namespace redfield;
    assert(!hook_setting("ClientKey").present);
    assert(!bind_hook_configuration(nullptr));
    ConfigStore store;
    assert(store.replace(R"("Config2" {
      "Emulator" { "ClientPath" "wrong-section" }
      "Game" { "ClientPath" "first" "ClientPath" "second"
        "ClientKey" "SOFTWARE\RF21" "Language" "tr_TR" "ClientValue" "" }
      "Ignored" { "ClientPath" "wrong-last" }
    })")==ConfigResult::published);
    auto first=store.snapshot();
    assert(bind_hook_configuration(first));
    auto value=hook_setting("ClientPath");
    assert(value.present && std::string_view(value.data,value.size)=="second");
    value=hook_setting("ClientKey");
    assert(value.present && std::string_view(value.data,value.size)==R"(SOFTWARE\RF21)");
    assert(hook_setting("ClientValue").present && hook_setting("ClientValue").size==0);
    assert(!hook_setting("ClientKey2").present && !hook_setting(nullptr).present);
    assert(!bind_hook_configuration(first));
    assert(store.replace(R"("Config2" { "Game" { "ClientPath" "new" } })")==ConfigResult::published);
    first.reset();
    value=hook_setting("ClientPath");
    assert(std::string_view(value.data,value.size)=="second");
    release_hook_configuration();
    assert(!hook_setting("ClientPath").present);
    assert(bind_hook_configuration(store.snapshot()));
    value=hook_setting("ClientPath");
    assert(std::string_view(value.data,value.size)=="new");
    release_hook_configuration();
}
