// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace xml_contract_fixture {
inline std::string profile_request(std::string_view index="0",std::string_view recipient="EbisuSDK",
                            std::string_view id="public-dll") {
    return "<LSX><Request id='"+std::string(id)+"' recipient='"+std::string(recipient)+
           "'><GetProfile index='"+std::string(index)+"'/></Request></LSX>";
}
inline std::vector<std::pair<std::string,std::string>> equivalent_forms() {
    const std::string canonical=profile_request();
    return {
        {"decimal request id",profile_request("0","EbisuSDK","public&#45;dll")},
        {"hexadecimal recipient",profile_request("0","Ebisu&#x53;DK")},
        {"numeric index",profile_request("&#48;")},
        {"declaration and newlines","<?xml version='1.0'?>\r\n"+canonical},
        {"BOM","\xef\xbb\xbf"+canonical},
        {"comments","<LSX><!--a--><Request id='public-dll' recipient='EbisuSDK'><!--b--><GetProfile index='0'/></Request></LSX>"},
        {"legacy whitespace","< LSX><Request id='public-dll' recipient='EbisuSDK'><GetProfile\vindex='0'/></Request></LSX>"},
        {"attribute order","<LSX><Request recipient='EbisuSDK' id='public-dll'><GetProfile index='0'/></Request></LSX>"}};
}
struct ValueCase {std::string literal,reference,decoded;};
inline std::vector<ValueCase> equivalent_values() {
    return {
        {"A&amp;&amp;unknown;","A&amp;&unknown;","A&&unknown;"},
        {"pre&amp;#0;suffix","pre&#0;suffix","pre&#0;suffix"},
        {"pre&amp;#x100000000;suffix","pre&#x100000000;suffix","pre&#x100000000;suffix"},
        {"caf\xc3\xa9 \xf0\x9f\x98\x80","caf&#233; &#x1F600;","caf\xc3\xa9 \xf0\x9f\x98\x80"},
        {"A","&#"+std::string(20000,'0')+"65;","A"}};
}
inline std::vector<std::string> malformed_frames() {
    return {
        "<LSX><Request id='must-not-reply' recipient='EbisuSDK'><GetProfile index='0' index='1'/></Request></LSX>",
        "<LSX><Request id='must-not-reply' recipient='EbisuSDK'><GetProfile index='0'/></Wrong></LSX>",
        profile_request("0","EbisuSDK","must-not-reply")+"junk"};
}
}
