#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include <cstring>
#include <string>
using namespace preloader_l::xml;
int main() {
    RequestView request{};
    {
        std::string wire = "<LSX><Request id='A&amp;lt;' sender='src' recipient='EALS'>"
          "<GetAuthCode Key='A&amp;lt;'><ClientId>A&amp;lt;</ClientId>"
          "<Nested><Inner>literal</Inner></Nested><Plain><![CDATA[&lt;]]></Plain>"
          "</GetAuthCode></Request></LSX>";
        if (parse_request_view(wire.data(),wire.size(),&request)!=RequestStatus::ok) return 1;
        wire.assign(wire.size(),'#'); // XMLDocument, not caller storage, owns values
    }
    if (!span_equals(request.id,"A&lt;") || !request.id.decoded()) return 2;
    if (!request.command_open_tag.element || !request.command_open_tag.document ||
        request.command_open_tag.data || request.command_open_tag.size ||
        !request.command_content.element || !request.command_content.document ||
        request.command_content.data || request.command_content.size) return 3;
    TextSpan attr{};
    if (find_attribute(request.command_open_tag,"Key",&attr)!=AttributeStatus::found ||
        !attr.decoded() || !span_equals(attr,"A&lt;")) return 4;
    TextSpan child_text{};
    if (find_direct_child_text(request.command_content,"ClientId",&child_text)!=ChildTextStatus::found ||
        !child_text.decoded() || !span_equals(child_text,"A&lt;")) return 5;
    size_t cursor=0;
    DirectChildView child{};
    if (next_direct_child(request.command_content,&cursor,&child)!=DirectChildStatus::found ||
        !span_equals(child.name,"ClientId") || !span_equals(child.content,"A&lt;")) return 6;
    if (next_direct_child(request.command_content,&cursor,&child)!=DirectChildStatus::found ||
        !span_equals(child.name,"Nested") || child.content.data || !child.content.element ||
        child.content.size || !child.content.document) return 7;
    if (next_direct_child(request.command_content,&cursor,&child)!=DirectChildStatus::found ||
        !span_equals(child.name,"Plain") || !span_equals(child.content,"&lt;")) return 8;
    if (next_direct_child(request.command_content,&cursor,&child)!=DirectChildStatus::end) return 9;
    char escaped[32]{}; size_t written=0;
    if (escape_attribute_value(attr,escaped,sizeof(escaped),&written)!=ValueCodecStatus::ok ||
        std::string(escaped)!="A&amp;lt;") return 10;
    SemanticValue semantic;
    if (semantic.assign_decoded(attr)!=ValueCodecStatus::ok ||
        !span_equals(semantic.view(),"A&lt;")) return 11;
    if (semantic.assign_source(attr)!=ValueCodecStatus::invalid_argument) return 12;
    const char standalone[]="<GetAuthCode Key='unsafe'>";
    TextSpan raw{standalone,sizeof(standalone)-1};
    if (find_attribute(raw,"Key",&attr)!=AttributeStatus::malformed) return 13;
    if (find_direct_child_text(raw,"ClientId",&child_text)!=ChildTextStatus::malformed) return 14;
    if (next_direct_child(raw,&cursor,&child)!=DirectChildStatus::malformed) return 15;
    constexpr char pi[]="<LSX><QueryOffers><?legacy k='x'?></QueryOffers></LSX>";
    if (parse_request_view(pi,sizeof(pi)-1,&request)!=RequestStatus::malformed) return 16;
    return 0;
}
