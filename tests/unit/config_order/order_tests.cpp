// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/config/order.hpp"
#include <algorithm>
#include <array>
#include <iostream>

using namespace preloader_l;
namespace {
unsigned checks=0;
void require(bool ok,const char* message) {
    ++checks;if(!ok)throw std::runtime_error(message);
}
template<class E,class F> void throws(F f,const char* label) {
    bool hit=false;try {f();} catch(const E&) {hit=true;}
    require(hit,label);
}
parser::Node keys(const std::vector<std::string>& names) {
    parser::Node result;
    for(const auto& name:names) {parser::Node child;child.name=name;result.children.push_back(child);}
    return result;
}
void fixed_cases() {
    require(config_order::fnv1a64("")==0xcbf29ce484222325ULL,"FNV empty seed");
    require(config_order::fnv1a64("a")==0xaf63dc4c8601ec8cULL,"FNV a");
    require(config_order::fnv1a64("foobar")==0x85944171f73967e8ULL,"FNV foobar");
    const std::string binary("a\0i",3);
    require(config_order::fnv1a64(binary)!=config_order::fnv1a64("a"),"FNV embedded NUL uses string length");
    const std::string high(1,static_cast<char>(0xff));
    require(config_order::fnv1a64(high)==((0xcbf29ce484222325ULL^255ULL)*0x100000001b3ULL),"FNV bytes unsigned");
    const auto node=keys({"a","b","i","c","q","j","k","z"});
    const std::vector<std::string> parsed={"q","i","a","z","j","b","k","c"};
    const std::vector<std::string> copied={"a","i","q","b","j","z","c","k"};
    require(config_order::parsed_order(node)==parsed,"fresh bucket collision groups reverse insertion");
    require(config_order::copy_order(parsed)==copied,"copy rehash reverses collision groups again");
    require(config_order::SelectedCopyOrder().order_for(node,{})==copied,"selected root first copy");
    require(config_order::SelectedCopyOrder().order_for(node,{"nested","child"})==copied,"descendant one copy");
    require(config_order::parsed_order(keys({})).empty(),"empty parser container");
    require(config_order::copy_order({}).empty(),"empty copied container");
    throws<config_order::UnsupportedOrder>([]{(void)config_order::parsed_order(keys({"0","1","2","3","4","5","6","7","8"}));},"ninth unique requires unsupported growth");
    throws<config_order::UnsupportedOrder>([]{(void)config_order::copy_order({"0","1","2","3","4","5","6","7","8"});},"copy above bound rejected");
    throws<std::invalid_argument>([]{(void)config_order::parsed_order(keys({"same","same"}));},"duplicate logical slots rejected");
    throws<std::invalid_argument>([]{(void)config_order::copy_order({"same","same"});},"duplicate native list input rejected");
}
void permutations() {
    // Independent bucket-group oracle. Group order is first bucket appearance;
    // parse order within each group is reverse, selected first copy is forward.
    std::vector<std::string> names={"a","b","c","i","j","q"};
    do {
        std::array<std::vector<std::string>,8> groups;
        std::vector<std::size_t> first_seen;
        for(const auto& key:names) {
            const auto bucket=static_cast<std::size_t>(config_order::fnv1a64(key)&7);
            if(groups[bucket].empty())first_seen.push_back(bucket);
            groups[bucket].push_back(key);
        }
        std::vector<std::string> parsed,copied;
        for(const auto bucket:first_seen) {
            const auto& group=groups[bucket];
            parsed.insert(parsed.end(),group.rbegin(),group.rend());
            copied.insert(copied.end(),group.begin(),group.end());
        }
        const auto actual=config_order::parsed_order(keys(names));
        require(actual==parsed,"permuted parse collision groups");
        require(config_order::copy_order(actual)==copied,"permuted copy collision groups");
        require(config_order::copy_order(copied)==parsed,"second fresh copy collision reversal");
    } while(std::next_permutation(names.begin(),names.end()));
}
void parser_copy() {
    parser::State state;
    require(parser::parse(R"("Config2" { "a" "first" "b" "middle" "i" "other" "a" { "q" "new" "a" "next" } })",state),"real parser synthetic bytes");
    const auto* config=state.root.find("Config2");
    require(config!=nullptr,"selected Config2 exists");
    require(config->completed_assignments==std::vector<std::string>{"a","b","i","a"},"duplicate assignment history retained");
    require(config_order::parsed_order(*config)==std::vector<std::string>{"i","a","b"},"duplicate keeps original parent slot");
    config_order::SelectedCopyOrder order;
    const auto document=parser::to_configuration_node(*config,order);
    require(document.iteration_order==std::vector<std::string>{"a","i","b"},"selected subtree copy order");
    require(document.child("a").value.empty() && document.child("a").text("q")=="new","duplicate replaces entire scalar with subtree");
    require(document.child("a").iteration_order==std::vector<std::string>{"q","a"},"nested copied collision container");
    state={};
    require(document.child("a").text("a")=="next","owned copied result survives parser destruction");
    parser::State too_large;
    require(parser::parse(R"("Config2" { "small" { "0" "x" "1" "x" "2" "x" "3" "x" "4" "x" "5" "x" "6" "x" "7" "x" "8" "x" } })",too_large),"large grammar still valid");
    throws<config_order::UnsupportedOrder>([&]{(void)parser::to_configuration_node(*too_large.root.find("Config2"),order);},"descendant above order bound rejected");
}
}
int main() {
    try {fixed_cases();permutations();parser_copy();std::cout<<"PASS: "<<checks<<" Config2 hash/list first-copy checks\n";return 0;}
    catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
