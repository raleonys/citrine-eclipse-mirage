#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include <cstring>
#include <string>
using namespace preloader_l::xml;
int main() {
    RequestView req{};
    {
        std::string frame="<LSX><Request id='5&amp;6' sender=\"EALS\"><GetSetting Name='A&amp;B'><Key>x&amp;y</Key><Empty/></GetSetting></Request></LSX>";
        if(parse_request_view(frame.data(),frame.size(),&req)!=RequestStatus::ok) return 1;
        frame.assign(frame.size(),'Z');
    }
    if(!span_equals(req.command,"GetSetting") || !span_equals(req.id,"5&6") ||
       !span_equals(req.sender,"EALS")) return 2;
    TextSpan attr{};
    if(find_attribute(req.command_open_tag,"Name",&attr)!=AttributeStatus::found ||
       !span_equals(attr,"A&B")) return 3;
    TextSpan text{};
    if(find_direct_child_text(req.command_content,"Key",&text)!=ChildTextStatus::found ||
       !span_equals(text,"x&y")) return 4;
    size_t pos=0; DirectChildView child{};
    if(next_direct_child(req.command_content,&pos,&child)!=DirectChildStatus::found ||
       !span_equals(child.name,"Key") || !span_equals(child.content,"x&y")) return 5;
    if(next_direct_child(req.command_content,&pos,&child)!=DirectChildStatus::found ||
       !span_equals(child.name,"Empty") || !child.self_closing) return 6;
    if(next_direct_child(req.command_content,&pos,&child)!=DirectChildStatus::end) return 7;
    // A second copied RequestView must retain DOM-projected bytes independent
    // of the original caller's frame and first RequestView lifetime.
    RequestView copy=req;
    req=RequestView{};
    if(!span_equals(copy.id,"5&6")) return 8;
    if(find_attribute(copy.command_open_tag,"None",&attr)!=AttributeStatus::not_found) return 9;
    if(find_attribute({"<R id='1' id='2'>",sizeof("<R id='1' id='2'>")-1},"id",&attr)!=AttributeStatus::malformed) return 10;
    // Known compatibility boundary: the retired scanner accepted inner PI,
    // whereas stock TinyXML2 document grammar rejects it (legacy test 71).
    constexpr char pi[]="<LSX><Request id='11'><GetAuthCode><?edge keep='closed'?></GetAuthCode></Request></LSX>";
    if(parse_request_view(pi,sizeof(pi)-1,&req)!=RequestStatus::malformed) return 11;
    return 0;
}
