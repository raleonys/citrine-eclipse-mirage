// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/xml/reader.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
namespace reader=citrine_eclipse_mirage::xml_reader_bridge;
namespace messages=citrine_eclipse_mirage::messages;
// Keep the historical fixture unchanged. Each reviewed exception checks both
// its frozen row and the exact repaired value; no row or error check is skipped.
// 1792/10495 preserve invalid control references (#6/#5) literally. All other
// differences preserve an unknown ampersand after an earlier valid conversion.
struct ValueCorrection {std::size_t row;const char* frozen;const char* corrected;};
constexpr ValueCorrection corrections[]={
    {529,"0|0||[52:61=266136353b26207831b1663630303b;NULL]","0|0||[52:61=262636353b26207831b1663630303b;NULL]"},
    {550,"0|0||[52:61=9726086d5636353b562409237831663630303b;NULL]","0|0||[52:61=972608265636353b262409237831663630303b;NULL]"},
    {730,"0|0||[52:61=26262328353b23933166363021303b;NULL]","0|0||[52:61=26262328353b26933166363021303b;NULL]"},
    {1675,"0|0||[52:61=262623ca353b23fe78316636303056;NULL]","0|0||[52:61=262623ca353b26fe78316636303056;NULL]"},
    {1735,"0|0||[52:61=26416d7831663630303b;NULL]","0|0||[52:61=2641267831663630303b;NULL]"},
    {1792,"0|0||[52:61=2661d16d3b062623783166863630303b3c;NULL]","0|0||[52:61=2661d16d3b2623363b2623783166863630303b3c;NULL]"},
    {2182,"0|0||[52:61=266136353bf09f988051;NULL]","0|0||[52:61=262636353bf09f988051;NULL]"},
    {3529,"0|0||[a7:61=2661b82336a53b23cecd7831663630303b;NULL]","0|0||[a7:61=2626b82336a53b26cecd7831663630303b;NULL]"},
    {4759,"0|0||[8f:61=26416d785a6636eb30303b;NULL]","0|0||[8f:61=264126785a6636eb30303b;NULL]"},
    {6679,"0|0||[52:7361=26416d7831663630303b;NULL]","0|0||[52:7361=2641267831663630303b;NULL]"},
    {7246,"0|0||[52:61=266175363526239b78d0a331fe3630303b;NULL]","0|0||[52:61=262675363526239b78d0a331fe3630303b;NULL]"},
    {7675,"0|0||[52:61=266136353b26237831873630303bd7;NULL]","0|0||[52:61=262636353b26237831873630303bd7;NULL]"},
    {8086,"0|0||[a0:61=26616d704123857831663630303b;NULL]","0|0||[a0:61=26616d704126857831663630303b;NULL]"},
    {8569,"0|0||[ed52:6e61=2661596d703b412378e86636303b;NULL]","0|0||[ed52:6e61=2661596d703b412678e86636303b;NULL]"},
    {9295,"0|0||[52:61=26617736353bf09f9880;NULL]","0|0||[52:61=26267736353bf09f9880;NULL]"},
    {9331,"0|0||[52:61=2610616d703b4123b8237831663630303b;NULL]","0|0||[52:61=2610616d703b4126b8237831663630303b;NULL]"},
    {9526,"0|0||[52:61=26beb16d703b4123d3237831663630303b;NULL]","0|0||[52:61=26beb16d703b4126d3237831663630303b;NULL]"},
    {10495,"0|0||[52:61=2605262378315b3630303b;NULL]","0|0||[52:61=262623353b262378315b3630303b;NULL]"},
    {11221,"0|0||[52:61=2626233635893b367831663630303b;NULL]","0|0||[52:61=2626233635893b267831663630303b;NULL]"},
    {11407,"0|0||[52:61=26615b36353bf09f9880;NULL]","0|0||[52:61=26265b36353bf09f9880;NULL]"},
};
std::string hex(std::string_view value) {
    constexpr char digits[]="0123456789abcdef";
    std::string result;
    result.reserve(value.size()*2);
    for(unsigned char c:value){result+=digits[c>>4];result+=digits[c&15];}
    return result;
}
std::string unhex(std::string_view value) {
    const auto nibble=[](char c)->unsigned {
        if(c>='0'&&c<='9')return c-'0';
        if(c>='a'&&c<='f')return c-'a'+10;
        throw std::runtime_error("Invalid corpus hex digit");
    };
    if(value.size()%2)throw std::runtime_error("Odd corpus hex length");
    std::string result;
    result.reserve(value.size()/2);
    for(std::size_t i=0;i<value.size();i+=2)
        result.push_back(static_cast<char>((nibble(value[i])<<4)|nibble(value[i+1])));
    return result;
}
void dump(std::ostream& out,const messages::XmlNode* node,reader::Backend& backend) {
    for(;node;node=node->next_element()) {
        out<<'['<<hex(node->name());
        for(const auto& [key,value]:backend.attributes_in_order(*node))
            out<<':'<<hex(key)<<'='<<hex(value);
        const auto* text=backend.text_003860(*node);
        out<<';'<<(text?hex(text):"NULL");
        dump(out,node->first_element(),backend);
        out<<']';
    }
}
}
int main(int argc,char** argv)try {
    if(argc!=3)throw std::runtime_error("Expected corpus and baseline-output paths");
    std::ifstream corpus(argv[1]),expected(argv[2]);
    if(!corpus||!expected)throw std::runtime_error("Cannot open pinned XML corpus");
    reader::Backend backend;
    std::size_t cases=0,corrected=0;
    std::string line,reference;
    while(std::getline(corpus,line)) {
        ++cases;
        if(!std::getline(expected,reference))throw std::runtime_error("Missing expected corpus row");
        const auto split=line.find(' ');
        if(split==std::string::npos)throw std::runtime_error("Invalid corpus count separator");
        const auto count=std::stoull(line.substr(0,split));
        auto input=unhex(std::string_view(line).substr(split+1));
        if(count>input.size())throw std::runtime_error("Corpus count exceeds actual input");
        auto parsed=backend.parse_c_string({input.data(),static_cast<std::size_t>(count)});
        input.assign(input.size(),'X'); // Only the parsed document owns live text.
        std::ostringstream actual;
        actual<<parsed.returned_code<<'|'<<parsed.has_error<<'|'<<hex(parsed.document->error_name())<<'|';
        dump(actual,parsed.document->first_element(),backend);
        for(const auto& fix:corrections)if(fix.row==cases) {
            if(reference!=fix.frozen)throw std::runtime_error("Reviewed frozen XML row changed");
            reference=fix.corrected;++corrected;break;
        }
        if(actual.str()!=reference)
            throw std::runtime_error("XML baseline mismatch at corpus row "+std::to_string(cases));
    }
    if(!corpus.eof()||std::getline(expected,reference)||!expected.eof())
        throw std::runtime_error("Corpus IO error or extra expected rows");
    if(cases!=12087)throw std::runtime_error("Incomplete XML corpus");
    if(corrected!=std::size(corrections))throw std::runtime_error("Incomplete reviewed XML value corrections");
    std::cout<<"PASS: "<<cases<<" pinned XML parse/ownership/error/tree corpus cases; "<<corrected<<" reviewed malformed-value corrections\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
