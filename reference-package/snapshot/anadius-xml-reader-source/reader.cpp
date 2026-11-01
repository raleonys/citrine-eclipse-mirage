#include "reader.hpp"
#include "vendor/tinyxml2/tinyxml2.h"
#include <stdexcept>
#include <unordered_map>

namespace anadius::xml_reader_bridge {
namespace {
namespace msg=messages_recovered;
class Document;
class Element final:public msg::XmlNode {
public:
    Element(const tinyxml2::XMLElement& native,const Document& owner):native_(native),owner_(owner){}
    std::string_view name()const override{return native_.Name();}
    const msg::XmlNode* first_element()const override;
    const msg::XmlNode* next_element()const override;
    std::optional<std::string_view> attribute(std::string_view name)const override {
        const std::string key(name);
        const char* value=native_.Attribute(key.c_str());
        return value?std::optional<std::string_view>(value):std::nullopt;
    }
    const tinyxml2::XMLElement& native()const{return native_;}
private:
    const tinyxml2::XMLElement& native_;
    const Document& owner_;
};
class Document final:public msg::XmlDocument {
public:
    Document():document_(true,tinyxml2::PRESERVE_WHITESPACE){}
    int parse(const char* value){return document_.Parse(value,static_cast<std::size_t>(-1));}
    bool has_error()const{return document_.Error();}
    const msg::XmlNode* first_element()const override{return wrap(document_.FirstChildElement());}
    // Historical interface name: original 033fd7 calls ErrorStr (0032d0),
    // the full diagnostic, rather than tinyxml2::ErrorName.
    std::string_view error_name()const override{return document_.ErrorStr();}
    const Element* wrap(const tinyxml2::XMLElement* native)const {
        if(!native)return nullptr;
        auto found=elements_.find(native);
        if(found!=elements_.end())return found->second.get();
        auto wrapper=std::make_unique<Element>(*native,*this);
        auto* address=wrapper.get();elements_.emplace(native,std::move(wrapper));return address;
    }
private:
    // Wrappers are destroyed before the upstream document owning native nodes.
    tinyxml2::XMLDocument document_;
    mutable std::unordered_map<const tinyxml2::XMLElement*,std::unique_ptr<Element>> elements_;
};
const msg::XmlNode* Element::first_element()const{return owner_.wrap(native_.FirstChildElement());}
const msg::XmlNode* Element::next_element()const{return owner_.wrap(native_.NextSiblingElement());}
const Element& checked(const msg::XmlNode& node) {
    auto* element=dynamic_cast<const Element*>(&node);
    if(!element)throw std::invalid_argument("XML reader received a node from another backend");
    return *element;
}
}
messages_recovered::ParseResult Backend::parse_c_string(std::string_view value) {
    // Own a terminator even if the supplied view is a slice. Native Parse with
    // size_t(-1) reads only to its first NUL and copies the parse buffer itself.
    const std::string owned(value);
    auto document=std::make_unique<Document>();
    const int code=document->parse(owned.c_str());
    const bool error=document->has_error();
    return {std::move(document),code,error};
}
std::unique_ptr<messages_recovered::XmlPrinter> Backend::printer() {
    return std::make_unique<xml_recovered::MemoryPrinter>();
}
std::vector<std::pair<std::string,std::string>> Backend::attributes_in_order(
        const messages_recovered::XmlNode& node) {
    std::vector<std::pair<std::string,std::string>> result;
    for(auto* attribute=checked(node).native().FirstAttribute();attribute;attribute=attribute->Next())
        result.emplace_back(attribute->Name(),attribute->Value());
    return result;
}
const char* Backend::text_003860(const messages_recovered::XmlNode& node) {
    return checked(node).native().GetText();
}
} // namespace anadius::xml_reader_bridge
