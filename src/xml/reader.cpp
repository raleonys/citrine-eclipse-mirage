#include "preloader_l/xml/reader.hpp"
#include <tinyxml2.h>
#include <stdexcept>
#include <unordered_map>

namespace preloader_l::xml_reader_bridge {
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
private:
    // Wrappers are destroyed before the upstream document owning native nodes.
    tinyxml2::XMLDocument document_;
    mutable std::unordered_map<const tinyxml2::XMLElement*,Element> elements_;
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
    // Preserve the staging string's former length rejection before inspecting
    // caller bytes, and keep the vendor's nBytes+1 allocation from overflowing.
    if(value.size()>std::string{}.max_size())throw std::length_error("XML input exceeds string size limit");
    auto document=std::make_unique<Document>();
    const int code=document->parse(value);
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
} // namespace preloader_l::xml_reader_bridge
