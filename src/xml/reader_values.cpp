// SPDX-License-Identifier: GPL-3.0-only

#include "reader_policy.hpp"
#include <cstdint>
#include <utility>

namespace preloader_l::xml_reader_bridge::detail {
bool xml_character(std::uint32_t value) {
    return value==9 || value==10 || value==13 ||
        (value>=0x20 && value<=0xd7ff) || (value>=0xe000 && value<=0xfffd) ||
        (value>=0x10000 && value<=0x10ffff);
}
// This handles references in values already parsed by the syntax reader, not XML syntax.
// Invalid/unknown references stay literal; only original input is inspected, so
// an escaped ampersand cannot introduce a second round of entity decoding.
std::string decode_references(std::string_view input) {
    std::string output;
    std::size_t copied=0;
    for(auto at=input.find('&');at!=input.npos;at=input.find('&',at+1)) {
        const auto tail=input.substr(at+1);
        std::uint32_t value=0;
        std::size_t length=0;
        constexpr std::pair<std::string_view,char> named[]={
            {"amp;",'&'},{"lt;",'<'},{"gt;",'>'},{"quot;",'"'},{"apos;",'\''}
        };
        for(const auto& [name,character]:named)if(tail.starts_with(name)) {
            value=static_cast<unsigned char>(character);length=1+name.size();break;
        }
        if(!length && tail.starts_with('#')) {
            std::size_t cursor=1;
            const unsigned radix=cursor<tail.size()&&tail[cursor]=='x'?16:10;
            if(radix==16)++cursor;
            const auto first=cursor;
            for(;cursor<tail.size();++cursor) {
                const auto c=tail[cursor];unsigned digit;
                if(c>='0'&&c<='9')digit=c-'0';
                else if(radix==16&&c>='a'&&c<='f')digit=c-'a'+10;
                else if(radix==16&&c>='A'&&c<='F')digit=c-'A'+10;
                else break;
                // Check before multiplying, including arbitrarily many zeroes.
                if(value>(0x10ffffU-digit)/radix)break;
                value=value*radix+digit;
            }
            if(cursor>first && cursor<tail.size() && tail[cursor]==';' && xml_character(value))
                length=cursor+2;
        }
        if(!length)continue;
        output.append(input.substr(copied,at-copied));
        if(value<=0x7f)output.push_back(static_cast<char>(value));
        else {
            if(value<=0x7ff)output.push_back(static_cast<char>(0xc0|(value>>6)));
            else {
                if(value<=0xffff)output.push_back(static_cast<char>(0xe0|(value>>12)));
                else {
                    output.push_back(static_cast<char>(0xf0|(value>>18)));
                    output.push_back(static_cast<char>(0x80|((value>>12)&0x3f)));
                }
                output.push_back(static_cast<char>(0x80|((value>>6)&0x3f)));
            }
            output.push_back(static_cast<char>(0x80|(value&0x3f)));
        }
        copied=at+length;at=copied-1;
    }
    // Empty means no replacement: every supported character emits a byte.
    if(copied)output.append(input.substr(copied));
    return output;
}
}
