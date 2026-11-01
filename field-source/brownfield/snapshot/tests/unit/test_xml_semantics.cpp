#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include <cstring>
#include <string>
using namespace preloader_l::xml;
int main() {
    RequestView req{};
    {
        std::string source="<LSX><Request id='A&amp;lt;' sender='A&amp;B'><GetSetting Name='Q&amp;lt;'><Key>T&amp;lt;</Key><FilterGroups>G&amp;lt;</FilterGroups></GetSetting></Request></LSX>";
        if(parse_request_view(source.data(),source.size(),&req)!=RequestStatus::ok) return 1;
        source.assign(source.size(),'Z');
    }
    if (!req.id.decoded() || !span_equals(req.id,"A&lt;")) return 2;
    TextSpan attr{};
    if (find_attribute(req.command_open_tag,"Name",&attr)!=AttributeStatus::found ||
        !attr.decoded() || !span_equals(attr,"Q&lt;")) return 3;
    TextSpan txt{};
    if(find_direct_child_text(req.command_content,"Key",&txt)!=ChildTextStatus::found ||
       !txt.decoded() || !span_equals(txt,"T&lt;")) return 4;
    SemanticValue decoded{};
    if(decoded.assign_decoded(attr)!=ValueCodecStatus::ok ||
       !span_equals(decoded.view(),"Q&lt;")) return 5;
    if(decoded.assign_source(attr)!=ValueCodecStatus::invalid_argument) return 6;
    if(decoded.assign_parsed(attr)!=ValueCodecStatus::ok ||
       !span_equals(decoded.view(),"Q&lt;")) return 7;
    size_t n=0;
    if(decode_source_value(attr,nullptr,0,&n)!=ValueCodecStatus::ok||n!=5) return 8;
    char buffer[128]{};
    if(decode_source_value(attr,buffer,sizeof(buffer),&n)!=ValueCodecStatus::ok ||
       std::strcmp(buffer,"Q&lt;")) return 9;
    if(normalize_source_attribute_value(req.id,buffer,sizeof(buffer),&n)!=ValueCodecStatus::ok ||
       std::strcmp(buffer,"A&amp;lt;")) return 10;
    ValueCodecStatus status{};
    if(!source_value_equals(attr,{"Q&lt;",5},&status)||status!=ValueCodecStatus::ok) return 11;
    size_t cursor=0;DirectChildView child{};
    if(next_direct_child(req.command_content,&cursor,&child)!=DirectChildStatus::found ||
       !child.content.decoded() || !span_equals(child.content,"T&lt;")) return 12;
    if(next_direct_child(req.command_content,&cursor,&child)!=DirectChildStatus::found ||
       !child.content.decoded() || !span_equals(child.content,"G&lt;")) return 13;
    constexpr char src[]="X&amp;Y";
    if(decoded.assign_source({src,sizeof(src)-1})!=ValueCodecStatus::ok ||
       !span_equals(decoded.view(),"X&Y")) return 16;
    if(decoded.assign_decoded(decoded.view())!=ValueCodecStatus::ok ||
       !span_equals(decoded.view(),"X&Y")) return 17;
    RequestView copy=req;req=RequestView{};
    if(!span_equals(copy.id,"A&lt;")) return 14;
    if(find_attribute(copy.command_open_tag,"Name",&attr)!=AttributeStatus::found||
       !span_equals(attr,"Q&lt;")) return 15;
    return 0;
}
