// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/config/hash_table.hpp"
#include "laureondo/config/order.hpp"
#include <algorithm>
#include <iostream>
#include <stdexcept>

using namespace laureondo;
namespace {
std::size_t checks=0;
void require(bool value,const char* message) {++checks;if(!value)throw std::runtime_error(message);}
void old_bound_differential() {
    std::vector<std::string> names={"a","b","c","i","j","q"};
    do {
        parser::Node logical;
        for(const auto& name:names){parser::Node child;child.name=name;logical.children.push_back(std::move(child));}
        const auto parsed=config_hash::from_parser(logical);
        require(parsed.table.traversal==config_order::parsed_order(logical),"old bounded parse differential");
        const auto selected=config_hash::selected_copy(logical);
        require(selected.table.traversal==config_order::SelectedCopyOrder().order_for(logical,{}),"old bounded firstcopy differential");
    }while(std::next_permutation(names.begin(),names.end()));
}
}
int main() {
    try {old_bound_differential();std::cout<<"PASS: "<<checks<<" bounded Config2 model differential checks\n";}
    catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
