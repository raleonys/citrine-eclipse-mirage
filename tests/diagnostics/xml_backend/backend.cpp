// SPDX-License-Identifier: GPL-3.0-only

#include "backend.hpp"
#include <pugixml.hpp>
#include <stdexcept>
#include <unordered_map>

namespace xml_comparison {
namespace {
namespace msg=preloader_l::messages;
pugi::xml_node first(pugi::xml_node node) {
    for(auto child:node.children())if(child.type()==pugi::node_element)return child;
    return {};
}
pugi::xml_node next(pugi::xml_node node) {
    for(auto child=node.next_sibling();child;child=child.next_sibling())
        if(child.type()==pugi::node_element)return child;
    return {};
}
class Document;
class Element final:public msg::XmlNode {
public:
    Element(pugi::xml_node node,const Document& owner):native(node),owner(owner){}
    std::string_view name()const override{return native.name();}
    const msg::XmlNode* first_element()const override;
    const msg::XmlNode* next_element()const override;
    std::optional<std::string_view> attribute(std::string_view key)const override;
    pugi::xml_node native;
    const Document& owner;
};
struct Hash {std::size_t operator()(pugi::xml_node node)const{return node.hash_value();}};
class Document final:public msg::XmlDocument {
public:
    pugi::xml_document native;
    std::string diagnostic;
    // Map nodes (and cached string views) survive rehash and later parses.
    mutable std::unordered_map<const char*,std::string> values;
    mutable std::unordered_map<pugi::xml_node,Element,Hash> elements;
    const Element* wrap(pugi::xml_node node)const {
        return node?&elements.try_emplace(node,node,*this).first->second:nullptr;
    }
    const char* decoded(const char* value)const {
        const std::string_view input(value);
        if(input.find('&')==input.npos)return value;
        if(auto found=values.find(value);found!=values.end())return found->second.c_str();
        auto result=decode_reference_value(input);
        if(result.empty())return value;
        return values.try_emplace(value,std::move(result)).first->second.c_str();
    }
    const msg::XmlNode* first_element()const override{return wrap(first(native));}
    std::string_view error_name()const override{return diagnostic;}
};
const msg::XmlNode* Element::first_element()const{return owner.wrap(first(native));}
const msg::XmlNode* Element::next_element()const{return owner.wrap(next(native));}
std::optional<std::string_view> Element::attribute(std::string_view key)const {
    const std::string name(key);
    auto attribute=native.attribute(name.c_str());
    return attribute?std::optional<std::string_view>(owner.decoded(attribute.value())):std::nullopt;
}
const Element& checked(const msg::XmlNode& node) {
    auto* element=dynamic_cast<const Element*>(&node);
    if(!element)throw std::invalid_argument("XML comparison received a foreign node");
    return *element;
}
// Approximate public status mapping, deliberately not a claim of error-policy
// equivalence. The corpus and real message flow record remaining differences.
int mapped(pugi::xml_parse_status status,std::string_view input) {
    switch(status) {
    case pugi::status_ok:return 0;
    case pugi::status_bad_attribute:return 7;
    case pugi::status_bad_pcdata:return 8;
    case pugi::status_bad_cdata:return 9;
    case pugi::status_bad_comment:return 10;
    case pugi::status_end_element_mismatch:return 14;
    case pugi::status_no_document_element:
        return input.find_first_not_of(" \r\n\t")==input.npos?13:8;
    default:return 15;
    }
}
}
msg::ParseResult Backend::parse_c_string(std::string_view input) {
    if(input.size()>std::string{}.max_size())throw std::length_error("XML input exceeds string size limit");
    if(auto end=input.find('\0');end!=input.npos)input=input.substr(0,end);
    auto document=std::make_unique<Document>();
    // Preserve comments/PI/whitespace and raw attribute whitespace; bypass
    // upstream entity conversion through its public parse option.
    constexpr auto flags=(pugi::parse_full|pugi::parse_ws_pcdata|pugi::parse_ws_pcdata_single)
        &~pugi::parse_escapes&~pugi::parse_wconv_attribute;
    auto result=document->native.load_buffer(input.empty()?"":input.data(),input.size(),flags,pugi::encoding_utf8);
    if(result.status==pugi::status_out_of_memory)throw std::bad_alloc();
    const int code=mapped(result.status,input);
    if(!result) {
        document->diagnostic=std::string(result.description())+" offset="+std::to_string(result.offset);
        // The public reader exposes no partial tree after a parse failure.
        document->native.reset();
    }
    return {std::move(document),code,!bool(result)};
}
std::unique_ptr<msg::XmlPrinter> Backend::printer() {
    return std::make_unique<preloader_l::xml::MemoryPrinter>();
}
std::vector<std::pair<std::string,std::string>> Backend::attributes_in_order(const msg::XmlNode& node) {
    const auto& element=checked(node);
    std::vector<std::pair<std::string,std::string>> result;
    for(auto attribute:element.native.attributes())
        result.emplace_back(attribute.name(),element.owner.decoded(attribute.value()));
    return result;
}
const char* Backend::text_003860(const msg::XmlNode& node) {
    const auto& element=checked(node);
    for(auto child:element.native.children()) {
        if(child.type()==pugi::node_comment)continue;
        if(child.type()==pugi::node_cdata)return child.value();
        return child.type()==pugi::node_pcdata?element.owner.decoded(child.value()):nullptr;
    }
    return nullptr;
}
}
