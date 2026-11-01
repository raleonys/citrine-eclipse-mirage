// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/config.hpp"
#include <atomic>
#include <iostream>
#include <thread>
using namespace preloader_l;
#define CHECK(x) do { if (!(x)) { std::cerr << "line " << __LINE__ << ": " #x "\n"; return 1; } } while (false)
int main() {
    ConfigStore store;
    CHECK(!store.snapshot()->online);
    std::string text = R"("Config2" { "Emulator" { "Online" "true" } "Unknown" { "Nested" { "Key" "a\"b" } } })";
    CHECK(store.replace(text)==ConfigResult::published);
    auto old=store.snapshot(); CHECK(old->online && old->leaves.size()==2);
    CHECK(old->leaves[1].raw_value==R"(a\"b)");
    text.assign(text.size(),'!');
    CHECK(old->leaves[1].path==std::vector<std::string>({"Unknown","Nested"}));
    for (const auto* bad : {"", "\"Other\" {}", "\"Config2\" {", "\"Config2\" {} junk", "\"Config2\" {} /* unclosed "}) {
        CHECK(store.replace(bad)==ConfigResult::invalid);
        CHECK(store.snapshot()==old);
    }
    std::string nul="\"Config2\" {}"; nul.push_back('\0'); CHECK(store.replace(nul)==ConfigResult::invalid);
    CHECK(store.snapshot()==old);
    CHECK(store.replace(R"("Config2" { "Game" { "Online" "true" } "Emulator" { "Online" "1" } })")==ConfigResult::published);
    CHECK(!store.snapshot()->online && old->online);
    CHECK(store.replace(R"("Config2" { "Emulator" { "Online" "true" "Online" "0" } })")==ConfigResult::published);
    CHECK(store.snapshot()->online);
    CHECK(store.replace(R"("Config2" { "Emulator" { "Online" "true" "Online" "false" } })")==ConfigResult::published);
    CHECK(!store.snapshot()->online);
    CHECK(store.replace("\xef\xbb\xbf\"Config2\" { // comment\n \"Emulator\" { /* ok */ \"Online\" \"true\" } }")==ConfigResult::published);
    CHECK(store.snapshot()->online);
    auto valid=store.snapshot();
    std::string many="\"Config2\" {";
    for (std::size_t i=0;i<ConfigStore::max_leaves+1;++i) many+="\"Key\" \"Value\" ";
    many+="}"; CHECK(store.replace(many)==ConfigResult::resource && store.snapshot()==valid);
    std::string deep="\"Config2\" {";
    for (int i=0;i<40;++i) deep+="\"Nested\" {";
    deep+=std::string(41,'}'); CHECK(store.replace(deep)==ConfigResult::invalid && store.snapshot()==valid);
    CHECK(store.replace(std::string(ConfigStore::max_bytes+1,'x'))==ConfigResult::too_large);
    CHECK(store.snapshot()==valid);
    std::atomic<bool> bad{false}, done{false};
    std::thread reader([&] {
        while (!done.load()) {
            const auto snapshot=store.snapshot();
            if (snapshot->leaves.size()!=1 || snapshot->leaves[0].raw_value!=(snapshot->online?"true":"false")) bad=true;
        }
    });
    for (int i=0;i<1000;++i) {
        const auto source=i%2?R"("Config2" { "Emulator" { "Online" "true" } })":R"("Config2" { "Emulator" { "Online" "false" } })";
        if (store.replace(source)!=ConfigResult::published) bad=true;
    }
    done=true; reader.join(); CHECK(!bad);
    std::cout << "CONFIG_PASS\n";
}
