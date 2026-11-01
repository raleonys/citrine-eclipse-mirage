#pragma once
#include "src/configuration/parser.hpp"
#include <stdexcept>
#include <string>
#include <string_view>

namespace preloader_l::tests::parser_snapshot {
inline void field(std::string& out,std::string_view text) {
    out+=std::to_string(text.size());out+=':';out.append(text);
}
inline void node(std::string& out,const parser::Node& n) {
    field(out,n.name);field(out,n.value);
    out+=std::to_string(n.completed_assignments.size());out+=';';
    for(const auto& key:n.completed_assignments)field(out,key);
    out+=std::to_string(n.children.size());out+=';';
    for(const auto& child:n.children)node(out,child);
}
// Length-prefixed bytes, not locale-sensitive XML/JSON. Includes completed and
// partial action state, duplicate chronology and all three error coordinates.
inline std::string evaluate(std::string_view bytes) {
    parser::State s;std::string out;
    try {out=parser::parse(bytes,s)?"ok;":"false;";}
    catch(const parser::ParseError& e) {
        out="error;"+std::to_string(static_cast<int>(e.rule))+';'+
            std::to_string(e.position.byte)+';'+std::to_string(e.position.line)+';'+
            std::to_string(e.position.column)+';';
    }
    out+=s.removed_utf8_bom?"bom;":"raw;";node(out,s.root);
    out+=std::to_string(s.open_objects.size());out+=';';
    for(const auto& frame:s.open_objects)node(out,frame);
    field(out,s.accumulator);field(out,s.key);field(out,s.value);
    return out;
}
inline std::string unhex(std::string_view text) {
    if(text.size()%2)throw std::runtime_error("odd hex fixture");
    auto nibble=[](char c)->unsigned {
        if(c>='0' && c<='9')return static_cast<unsigned>(c-'0');
        if(c>='a' && c<='f')return static_cast<unsigned>(c-'a'+10);
        throw std::runtime_error("invalid hex fixture");
    };
    std::string out;out.reserve(text.size()/2);
    for(std::size_t i=0;i<text.size();i+=2)
        out.push_back(static_cast<char>((nibble(text[i])<<4)|nibble(text[i+1])));
    return out;
}
inline std::string hex(std::string_view bytes) {
    const char* digits="0123456789abcdef";std::string out;
    for(unsigned char c:bytes){out+=digits[c>>4];out+=digits[c&15];}
    return out;
}
} // namespace preloader_l::tests::parser_snapshot
