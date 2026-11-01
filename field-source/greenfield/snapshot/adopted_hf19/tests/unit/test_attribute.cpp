#include "preloader_l/xml/attribute_parser.hpp"
#include "preloader_l/xml/request_parser.hpp"
using namespace preloader_l;
int main(){
 constexpr char in[]="<LSX><Request id=\"24\"><GetSetting Name='Online' Scope=\"Game\" Empty=\"\"/></Request></LSX>";
 xml::RequestView req{}; if(xml::parse_request_view(in,sizeof(in)-1,&req)!=xml::RequestStatus::ok)return 1;
 xml::TextSpan v{};
 if(xml::find_attribute(req.command_open_tag,"Name",&v)!=xml::AttributeStatus::found || !xml::span_equals(v,"Online"))return 2;
 if(xml::find_attribute(req.command_open_tag,"Scope",&v)!=xml::AttributeStatus::found || !xml::span_equals(v,"Game"))return 3;
 if(xml::find_attribute(req.command_open_tag,"Empty",&v)!=xml::AttributeStatus::found || v.size!=0 || v.data==nullptr)return 4;
 if(xml::find_attribute(req.command_open_tag,"Missing",&v)!=xml::AttributeStatus::not_found || v.size!=0)return 5;
 constexpr char bad[]="<GetSetting Name=Online>"; xml::TextSpan b{bad,sizeof(bad)-1}; if(xml::find_attribute(b,"Name",&v)!=xml::AttributeStatus::malformed)return 6;
 if(xml::find_attribute(req.command_open_tag,nullptr,&v)!=xml::AttributeStatus::invalid_argument)return 7;
 constexpr char dup[]="<GetSetting Name=\"Online\" Scope=\"Game\" Scope=\"Other\">"; xml::TextSpan d{dup,sizeof(dup)-1};
 if(xml::find_attribute(d,"Name",&v)!=xml::AttributeStatus::malformed)return 8;
 constexpr char dup_key[]="<GetSetting Name=\"Online\" Name=\"Offline\">"; xml::TextSpan dk{dup_key,sizeof(dup_key)-1};
 if(xml::find_attribute(dk,"Name",&v)!=xml::AttributeStatus::malformed)return 9;
 return 0;
}
