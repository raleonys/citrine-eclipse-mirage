// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/xml.hpp"
#include <iostream>
#include <string>
using namespace preloader_l::xml;
using redfield::xml::Result;
#define CHECK(x) do { if (!(x)) { std::cerr << "line " << __LINE__ << ": " #x "\n"; return 1; } } while (false)
int main() {
    RequestView request{};
    std::string source="<LSX><Request id='A&amp;lt;' recipient='EALS'><GetSetting Key='A&amp;lt;'><Key><![CDATA[</LSX>]]></Key><Nested><Inner/></Nested><Value>x&amp;lt;</Value></GetSetting></Request></LSX>";
    CHECK(redfield::xml::parse(source,request)==Result::ok);
    source.assign(source.size(),'!');
    CHECK(span_equals(request.id,"A&lt;") && request.id.decoded());
    TextSpan attr{},text{};
    CHECK(find_attribute(request.command_open_tag,"Key",&attr)==AttributeStatus::found && span_equals(attr,"A&lt;"));
    CHECK(find_direct_child_text(request.command_content,"Key",&text)==ChildTextStatus::found && span_equals(text,"</LSX>"));
    CHECK(find_direct_child_text(request.command_content,"Value",&text)==ChildTextStatus::found && span_equals(text,"x&lt;"));
    auto retained=request; request={}; CHECK(span_equals(retained.id,"A&lt;"));
    CHECK(span_equals(response_sender_policy(retained),"EALS"));
    const std::string good="<LSX><Request id='1'><GetSetting/></Request></LSX>";
    for (std::size_t i=0;i<good.size();++i) CHECK(redfield::xml::parse(std::string_view(good).substr(0,i),request)==Result::incomplete);
    CHECK(redfield::xml::parse(good+good,request)==Result::trailing_data && !request.command.data);
    CHECK(redfield::xml::parse(std::string(redfield::xml::max_bytes+1,'x'),request)==Result::too_large);
    for (const auto* bad : {"<BAD></BAD>","<LSX><Request><A/><B/></Request></LSX>","<LSX><Request id='1' id='2'><A/></Request></LSX>","<LSX><Request><A></B></Request></LSX>","<LSX></LSX>","<LSX><Request><A><?pi test?></A></Request></LSX>"}) {
        CHECK(redfield::xml::parse(bad,request)==Result::malformed && !request.command.data);
    }
    auto nul=good; nul.insert(8,1,'\0'); CHECK(redfield::xml::parse(nul,request)==Result::malformed);
    CHECK(redfield::xml::parse("<LSX><Request><!-- </LSX> --><A/></Request></LSX>",request)==Result::ok);
    CHECK(redfield::xml::parse("<LSX><Request><A Key='</LSX>'/></Request></LSX>",request)==Result::ok);
    CHECK(find_attribute({"<A Key='x'>",11},"Key",&attr)==AttributeStatus::malformed);
    std::cout << "XML_PASS\n";
}
