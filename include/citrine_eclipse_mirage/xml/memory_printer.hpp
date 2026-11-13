// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/protocol/messages.hpp"
#include "citrine_eclipse_mirage/handlers/handlers.hpp"
#include <string>
#include <string_view>
#include <vector>

namespace citrine_eclipse_mirage::xml {
// Memory-only default constructor path of original XMLPrinter019b0. The FILE*
// writer, arbitrary constructor depth and other XMLVisitor entry points are not
// claimed here. Names/values follow original NUL-terminated string semantics.
class MemoryPrinter final : public messages::XmlPrinter {
public:
    void open(std::string_view name) override {open_element(name,false);}
    void attribute(std::string_view name,std::string_view value) override;
    void close() override {close_element(false);}
    std::string buffer_including_terminator() const override;
    void open_element(std::string_view name,bool compact);
    void close_element(bool compact);
    void push_text(std::string_view value,bool cdata=false);
    void print_string(std::string_view value,bool text_entities);
    std::string_view bytes_without_terminator() const {return buffer_;}
    int depth() const {return depth_;}
private:
    static std::string_view c_string(std::string_view);
    void indent(int depth);
    std::string buffer_;
    std::vector<std::string> elements_;
    bool element_just_opened_=false;
    bool first_element_=true;
    int depth_=0;
    int text_depth_=-1;
};

class AttributeReader {
public:
    virtual ~AttributeReader()=default;
    virtual const char* attribute(handlers::Request,const char*)=0;
};
// Concrete serialization bridge for the seven business handlers. Request XML
// parsing/ownership remains a mandatory reader, not a guessed parser.
class HandlerWriter final : public handlers::Xml {
public:
    HandlerWriter(MemoryPrinter& printer,AttributeReader& reader):printer_(printer),reader_(reader){}
    const char* attribute_002cc0(handlers::Request request,const char* key) override {
        return reader_.attribute(request,key);
    }
    void open_004080(const char* name,bool compact) override {printer_.open_element(name,compact);}
    void attribute_005560(const char* name,const char* value) override {printer_.attribute(name,value);}
    void close_virtual_48(bool compact) override {printer_.close_element(compact);}
private:
    MemoryPrinter& printer_;
    AttributeReader& reader_;
};
} // namespace citrine_eclipse_mirage::xml
