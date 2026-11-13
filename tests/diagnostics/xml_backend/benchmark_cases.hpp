// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "citrine_eclipse_mirage/router/router.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace xml_comparison {
inline std::uint64_t consume_xml(const citrine_eclipse_mirage::messages::XmlNode* root,
                                 citrine_eclipse_mirage::dispatch::NodeAccess& access) {
    std::uint64_t result=1469598103934665603ULL;
    auto add=[&](std::string_view value){for(unsigned char c:value){result^=c;result*=1099511628211ULL;}result^=255;result*=1099511628211ULL;};
    for(auto node=root;node;node=node->next_element()) {
        add(node->name());
        for(const auto& [key,value]:access.attributes_in_order(*node)){add(key);add(value);}
        if(auto text=access.text_003860(*node))add(text);
        result^=consume_xml(node->first_element(),access);result*=1099511628211ULL;
    }
    return result;
}
struct BenchmarkCase {std::string name,input;};
inline std::vector<BenchmarkCase> benchmark_cases() {
    std::vector<BenchmarkCase> cases={
        {"small-request","<LSX><Request id='1' recipient='EALS'><GetProfile index='0'/></Request></LSX>"},
        {"large-text","<R>"+std::string(65536,'x')+"</R>"},
        {"one-MiB-text","<R>"+std::string(1048576,'x')+"</R>"}};
    std::string wide="<R>";
    for(unsigned i=0;i<256;++i)wide+="<C v='1'/>";
    cases.push_back({"many-elements",wide+"</R>"});
    cases.push_back({"reference-heavy","<R>"+std::string(4096,'&')+"&#65;&amp;&#x1F600;&unknown;</R>"});
    cases.push_back({"declaration-request","<?xml version='1.0'?><LSX><Request id='1' recipient='EALS'><GetProfile index='0'/></Request></LSX>"});
    cases.push_back({"legacy-accepted","< R a='A&amp;'><!odd><C v='1'/></R>"});
    cases.push_back({"unicode-reference","<R a='caf&#233; &#x1F600;'>caf&#233; &#x1F600;</R>"});
    std::string attributes="<R";
    for(unsigned i=0;i<64;++i)attributes+=" a"+std::to_string(i)+"='prefix&amp;&#65;&unknown;suffix'";
    cases.push_back({"many-attributes",attributes+"/>"});
    std::string deep;
    // Both profiles must accept the same work. Rejection is measured in the
    // explicit safety matrix, not timed against successful tree traversal.
    for(unsigned i=0;i<80;++i)deep+="<R v='1'>";
    for(unsigned i=0;i<80;++i)deep+="</R>";
    cases.push_back({"deep-elements",deep});
    return cases;
}
}
