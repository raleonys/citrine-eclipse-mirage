// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/xml/reader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {
namespace reader=preloader_l::xml_reader_bridge;
namespace messages=preloader_l::messages;
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
    std::size_t cases=0;
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
        if(actual.str()!=reference)
            throw std::runtime_error("XML baseline mismatch at corpus row "+std::to_string(cases));
    }
    if(!corpus.eof()||std::getline(expected,reference)||!expected.eof())
        throw std::runtime_error("Corpus IO error or extra expected rows");
    if(cases!=12087)throw std::runtime_error("Incomplete XML corpus");
    std::cout<<"PASS: "<<cases<<" pinned XML parse/ownership/error/tree corpus cases\n";
    return 0;
}catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
