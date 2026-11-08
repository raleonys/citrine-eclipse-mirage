// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/xml/memory_printer.hpp"

#include <functional>

namespace preloader_l::xml {
namespace {
// bytes_without_terminator() lends a view directly into buffer_. Appending
// to that same string may reallocate before the writer finishes reading it.
// Snapshot only overlapping inputs, leaving all ordinary borrowed inputs on
// their existing no-copy path.
std::string_view stable_input(std::string_view value,const std::string& destination,
                              std::string& snapshot) {
    if(!value.empty()&&!destination.empty()) {
        const std::less<const char*> less;
        if(!less(value.data(),destination.data()) &&
           less(value.data(),destination.data()+destination.size())) {
            snapshot.assign(value);
            return snapshot;
        }
    }
    return value;
}
}
std::string_view MemoryPrinter::c_string(std::string_view value) {
    const auto end=value.find('\0');
    return end==std::string_view::npos?value:value.substr(0,end);
}
void MemoryPrinter::indent(int depth) {
    // 005270 writes exactly four ASCII spaces for each positive depth.
    for(int i=0;i<depth;++i)buffer_.append("    ");
}
void MemoryPrinter::open_element(std::string_view name,bool compact) {
    std::string snapshot;
    name=stable_input(c_string(name),buffer_,snapshot);
    if(element_just_opened_) {element_just_opened_=false;buffer_.push_back('>');}
    if(!compact) {
        if(first_element_ || text_depth_<0) {
            if(!first_element_)buffer_.push_back('\n');
            indent(depth_);
        }
        // The compact branch bypasses this write in the original body.
        first_element_=false;
    }
    elements_.emplace_back(c_string(name));
    buffer_.push_back('<');buffer_.append(c_string(name));
    ++depth_;
    element_just_opened_=true;
}
void MemoryPrinter::print_string(std::string_view value,bool text_entities) {
    std::string snapshot;
    value=stable_input(c_string(value),buffer_,snapshot);
    for(const unsigned char byte:value) {
        switch(byte) {
        case '&':buffer_.append("&amp;");break;
        case '<':buffer_.append("&lt;");break;
        case '>':buffer_.append("&gt;");break;
        case '"':if(!text_entities)buffer_.append("&quot;");else buffer_.push_back('"');break;
        case '\'':if(!text_entities)buffer_.append("&apos;");else buffer_.push_back('\'');break;
        default:buffer_.push_back(static_cast<char>(byte));break;
        }
    }
}
void MemoryPrinter::attribute(std::string_view name,std::string_view value) {
    std::string name_snapshot,value_snapshot;
    name=stable_input(c_string(name),buffer_,name_snapshot);
    value=stable_input(c_string(value),buffer_,value_snapshot);
    buffer_.push_back(' ');buffer_.append(c_string(name));buffer_.append("=\"");
    print_string(value,false);buffer_.push_back('"');
}
void MemoryPrinter::push_text(std::string_view value,bool cdata) {
    std::string snapshot;
    value=stable_input(c_string(value),buffer_,snapshot);
    text_depth_=depth_-1;
    if(element_just_opened_){element_just_opened_=false;buffer_.push_back('>');}
    if(cdata) {buffer_.append("<![CDATA[");buffer_.append(c_string(value));buffer_.append("]]>");}
    else print_string(value,true);
}
void MemoryPrinter::close_element(bool compact) {
    --depth_;
    const auto& name=elements_.back(); // Original valid-use precondition: nonempty stack.
    if(element_just_opened_)buffer_.append("/>");
    else {
        if(text_depth_<0 && !compact){buffer_.push_back('\n');indent(depth_);}
        buffer_.append("</");buffer_.append(name);buffer_.push_back('>');
    }
    elements_.pop_back();
    if(text_depth_==depth_)text_depth_=-1;
    if(depth_==0 && !compact)buffer_.push_back('\n');
    element_just_opened_=false;
}
std::string MemoryPrinter::buffer_including_terminator() const {
    auto output=buffer_;output.push_back('\0');return output;
}
} // namespace preloader_l::xml
