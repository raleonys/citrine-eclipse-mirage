// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/xml/reader.hpp"
#include "reader_policy.hpp"
#include <pugixml.hpp>
#include <stdexcept>
#include <unordered_map>

namespace citrine_eclipse_mirage::xml_reader_bridge {
namespace {
namespace msg=citrine_eclipse_mirage::messages;
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
    const char* decoded(const char* value,bool references=true)const {
        const std::string_view input(value);
        if(input.find('\r')==input.npos&&(!references||input.find('&')==input.npos))return value;
        if(auto found=values.find(value);found!=values.end())return found->second.c_str();
        auto normalized=detail::normalize_newlines(input);
        auto result=references?detail::decode_references(normalized.empty()?input:std::string_view(normalized)):std::string{};
        if(result.empty())result=std::move(normalized);
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
    if(!element)throw std::invalid_argument("XML reader received a node from another backend");
    return *element;
}
}
msg::ParseResult Backend::parse_c_string(std::string_view input) {
    if(input.size()>std::string{}.max_size())throw std::length_error("XML input exceeds string size limit");
    if(auto end=input.find('\0');end!=input.npos)input=input.substr(0,end);
    auto document=std::make_unique<Document>();
    auto policy=detail::inspect(input);
    const int code=policy.code;
    if(code)document->diagnostic=std::move(policy.diagnostic);
    else {
        // Keep values raw until the shared checked value policy is requested.
        constexpr auto flags=(pugi::parse_full|pugi::parse_fragment)
            &~pugi::parse_escapes&~pugi::parse_wconv_attribute&~pugi::parse_eol;
        const auto data=input.substr(policy.begin,policy.end-policy.begin);
        if(!policy.construct) {
            const auto result=document->native.load_buffer(data.empty()?"":data.data(),data.size(),flags,pugi::encoding_utf8);
            if(result.status==pugi::status_out_of_memory)throw std::bad_alloc();
            policy.construct=!result;
        }
        if(policy.construct) {
            document->native.reset();
            detail::construct(document->native,input);
        }
    }
    return {std::move(document),code,code!=0};
}
std::unique_ptr<msg::XmlPrinter> Backend::printer() {
    return std::make_unique<citrine_eclipse_mirage::xml::MemoryPrinter>();
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
        if(child.type()==pugi::node_cdata)return element.owner.decoded(child.value(),false);
        return child.type()==pugi::node_pcdata?element.owner.decoded(child.value()):nullptr;
    }
    return nullptr;
}
}
