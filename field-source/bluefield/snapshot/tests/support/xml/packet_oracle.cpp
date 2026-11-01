// SPDX-License-Identifier: GPL-3.0-only
// The included packet compatibility files are literal archive bytes. Rename
// their namespace and depth symbol only inside this test TU to avoid ODR/name
// collisions with the active upstream build. Never linked into production.
#define tinyxml2 preloader_l_packet_tinyxml2
#define TINYXML2_MAX_ELEMENT_DEPTH PRELOADER_L_PACKET_XML_DEPTH
#include "tests/support/xml/packet/tinyxml2.cpp"
#undef TINYXML2_MAX_ELEMENT_DEPTH
#undef tinyxml2
#include "tests/support/xml/packet_oracle.hpp"
namespace preloader_l::test_packet_xml {
namespace packet=preloader_l_packet_tinyxml2;
static_assert(::PRELOADER_L_PACKET_XML_DEPTH==100);
static void node(std::string& out,const packet::XMLElement* n){
 for(;n;n=n->NextSiblingElement()){
  field(out,n->Name());out+='[';
  for(auto* a=n->FirstAttribute();a;a=a->Next()){field(out,a->Name());field(out,a->Value());}
  out+=']';const auto* text=n->GetText();out+=text?'T':'N';if(text)field(out,text);
  out+='{';node(out,n->FirstChildElement());out+='}';
 }
}
Result parse(std::string_view input){
 const std::string owned(input);packet::XMLDocument d(true,packet::PRESERVE_WHITESPACE);
 const int code=d.Parse(owned.c_str(),static_cast<std::size_t>(-1));
 Result r{code,d.Error(),d.ErrorStr(),{}};node(r.tree,d.FirstChildElement());return r;
}
}
