#include "preloader_l/xml/request_parser.hpp"
#include <string>
#include <cstring>
using namespace preloader_l::xml;
int main() {
    constexpr size_t count=1536;
    std::string wire="<LSX><Request><GetProfile>";
    for(size_t i=0;i<count;++i)wire+="<Child>X&amp;lt;</Child>";
    wire+="</GetProfile></Request></LSX>";
    RequestView req{};
    if(parse_request_view(wire.data(),wire.size(),&req)!=RequestStatus::ok) return 1;
    if(!req.command_content.direct_children || req.command_content.direct_children->count!=count) return 2;
    if(!req.command_content.direct_children->nodes || !req.command_content.document) return 3;
    wire.assign(wire.size(),'!');
    RequestView copied=req;
    req=RequestView{};
    size_t cursor=0;
    DirectChildView child{};
    for(size_t i=0;i<count;++i) {
        if(next_direct_child(copied.command_content,&cursor,&child)!=DirectChildStatus::found) return 4;
        if(!span_equals(child.name,"Child") || !span_equals(child.content,"X&lt;")) return 5;
    }
    if(cursor!=count || next_direct_child(copied.command_content,&cursor,&child)!=DirectChildStatus::end) return 6;
    if(next_direct_child(copied.command_content,&cursor,&child)!=DirectChildStatus::end) return 7;
    return 0;
}
