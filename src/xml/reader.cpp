// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/xml/reader.hpp"
#include <tinyxml2.h>
#include <cstdint>
#include <stdexcept>
#include <unordered_map>
#include <utility>

namespace preloader_l::xml_reader_bridge {
namespace {
namespace msg=messages;
bool xml_character(std::uint32_t value) {
    return value==9 || value==10 || value==13 ||
        (value>=0x20 && value<=0xd7ff) || (value>=0xe000 && value<=0xfffd) ||
        (value>=0x10000 && value<=0x10ffff);
}
// This handles references in values already parsed by TinyXML2, not XML syntax.
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
class Document;
class Element final:public msg::XmlNode {
public:
    Element(const tinyxml2::XMLElement& native,const Document& owner):native_(native),owner_(owner){}
    std::string_view name()const override{return native_.Name();}
    const msg::XmlNode* first_element()const override;
    const msg::XmlNode* next_element()const override;
    std::optional<std::string_view> attribute(std::string_view name)const override;
    const char* decoded(const char*)const;
    const tinyxml2::XMLElement& native()const{return native_;}
private:
    const tinyxml2::XMLElement& native_;
    const Document& owner_;
};
class Document final:public msg::XmlDocument {
public:
    // Keep the pinned syntax parser, depth and newline policy, but bypass its
    // numeric overflow and unknown-entity compaction paths via its public option.
    Document():document_(false,tinyxml2::PRESERVE_WHITESPACE){}
    int parse(std::string_view value){
        // Preserve C-string semantics within the borrowed view. TinyXML2 makes
        // its own terminated parse-buffer copy; no staging string is needed.
        const auto end=value.find('\0');
        const auto size=end==std::string_view::npos?value.size():end;
        return document_.Parse(value.empty()?"":value.data(),size);
    }
    bool has_error()const{return document_.Error();}
    const msg::XmlNode* first_element()const override{return wrap(document_.FirstChildElement());}
    // Historical interface name: original 033fd7 calls ErrorStr (0032d0),
    // the full diagnostic, rather than tinyxml2::ErrorName.
    std::string_view error_name()const override{return document_.ErrorStr();}
    const Element* wrap(const tinyxml2::XMLElement* native)const {
        if(!native)return nullptr;
        // Node addresses in unordered_map remain stable through rehash. Store
        // the wrapper in its map node instead of allocating it separately.
        return &elements_.try_emplace(native,*native,*this).first->second;
    }
    const char* decoded(const char* value)const {
        const std::string_view input(value);
        if(input.find('&')==input.npos)return value;
        if(const auto found=decoded_.find(value);found!=decoded_.end())return found->second.c_str();
        auto result=decode_references(input);
        if(result.empty())return value;
        // Map nodes and their strings remain stable through rehash. Allocation
        // failure leaves the native value intact and a later lookup can retry.
        return decoded_.try_emplace(value,std::move(result)).first->second.c_str();
    }
private:
    // Wrappers are destroyed before the upstream document owning native nodes.
    tinyxml2::XMLDocument document_;
    mutable std::unordered_map<const char*,std::string> decoded_;
    mutable std::unordered_map<const tinyxml2::XMLElement*,Element> elements_;
};
const char* Element::decoded(const char* value)const{return owner_.decoded(value);}
std::optional<std::string_view> Element::attribute(std::string_view name)const {
    const std::string key(name);
    const char* value=native_.Attribute(key.c_str());
    return value?std::optional<std::string_view>(decoded(value)):std::nullopt;
}
const msg::XmlNode* Element::first_element()const{return owner_.wrap(native_.FirstChildElement());}
const msg::XmlNode* Element::next_element()const{return owner_.wrap(native_.NextSiblingElement());}
const Element& checked(const msg::XmlNode& node) {
    auto* element=dynamic_cast<const Element*>(&node);
    if(!element)throw std::invalid_argument("XML reader received a node from another backend");
    return *element;
}
}
messages::ParseResult Backend::parse_c_string(std::string_view value) {
    // Preserve the staging string's former length rejection before inspecting
    // caller bytes, and keep the vendor's nBytes+1 allocation from overflowing.
    if(value.size()>std::string{}.max_size())throw std::length_error("XML input exceeds string size limit");
    auto document=std::make_unique<Document>();
    const int code=document->parse(value);
    const bool error=document->has_error();
    return {std::move(document),code,error};
}
std::unique_ptr<messages::XmlPrinter> Backend::printer() {
    return std::make_unique<xml::MemoryPrinter>();
}
std::vector<std::pair<std::string,std::string>> Backend::attributes_in_order(
        const messages::XmlNode& node) {
    std::vector<std::pair<std::string,std::string>> result;
    const auto& element=checked(node);
    for(auto* attribute=element.native().FirstAttribute();attribute;attribute=attribute->Next())
        result.emplace_back(attribute->Name(),element.decoded(attribute->Value()));
    return result;
}
const char* Backend::text_003860(const messages::XmlNode& node) {
    const auto& element=checked(node);
    const auto* child=element.native().FirstChild();
    while(child&&child->ToComment())child=child->NextSibling();
    const auto* text=child?child->ToText():nullptr;
    if(!text)return nullptr;
    return text->CData()?text->Value():element.decoded(text->Value());
}
} // namespace preloader_l::xml_reader_bridge
