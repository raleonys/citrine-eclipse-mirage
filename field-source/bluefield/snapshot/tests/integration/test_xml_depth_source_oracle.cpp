// SPDX-License-Identifier: GPL-3.0-only
#include "src/xml/reader.hpp"
#include "tests/support/xml/packet_oracle.hpp"
#include <array>
#include <iostream>
#include <stdexcept>
namespace packet=preloader_l::test_packet_xml;
namespace msg=preloader_l::messages;
static unsigned cases=0,checks=0;
static void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
static void node(std::string& out,const msg::XmlNode* n,preloader_l::xml_reader::Backend& b){
 for(;n;n=n->next_element()){
  packet::field(out,n->name());out+='[';
  for(auto& [name,value]:b.attributes_in_order(*n)){packet::field(out,name);packet::field(out,value);}
  out+=']';const auto* text=b.text_003860(*n);out+=text?'T':'N';if(text)packet::field(out,text);
  out+='{';node(out,n->first_element(),b);out+='}';
 }
}
static void compare(std::string_view input){
 ++cases;auto expected=packet::parse(input);preloader_l::xml_reader::Backend backend;
 auto actual=backend.parse_c_string(input);std::string tree;node(tree,actual.document->first_element(),backend);
 if(actual.returned_code!=expected.code||actual.has_error!=expected.error||actual.document->error_name()!=expected.diagnostic||tree!=expected.tree)
  throw std::runtime_error("XML source mismatch case="+std::to_string(cases)+" expected_code="+std::to_string(expected.code)+" actual_code="+std::to_string(actual.returned_code)+" expected_error="+expected.diagnostic+" actual_error="+std::string(actual.document->error_name()));
 checks+=4;
}
static std::string nested(unsigned depth,std::string_view body,bool lines=false){
 std::string input;for(unsigned i=0;i<depth;++i)input+=lines?"<R>\r\n":"<R>";
 input+=body;for(unsigned i=0;i<depth;++i)input+="</R>";return input;
}
int main(){try{
 // The comparison domain is depth/error precedence and ordinary structural XML.
 // Known upstream Unicode/entity fixes are retained separately, not reverted
 // or misrepresented as original packet/full-binary parser equivalence.
 const std::array<std::string,18> bodies{"","text","<R/>","<!--comment-->","<![CDATA[text]]>","<R a='x' b='&amp;'/>tests","<bad a=>","<bad>","</wrong>","<!--unterminated","<![CDATA[unterminated","<?bad?>","<!DOCTYPE r>","<R a='1' a='2'/>","&amp;","<R/><!--tail-->","\r\n",std::string("\0<bad a=>",10)};
 for(unsigned depth=0;depth<=110;++depth)for(auto& body:bodies)for(bool lines:{false,true})compare(nested(depth,body,lines));
 for(auto depth:{200u,499u,500u,510u})for(auto& body:bodies)compare(nested(depth,body));
 for(auto depth:{97u,98u,99u,100u})for(auto& body:bodies){
  compare(nested(depth,body)+"<bad a=>");compare("<Good/>"+nested(depth,body));
  compare(std::string("\xef\xbb\xbf")+nested(depth,body));
 }
 // Concrete earlier-parse-error versus later-syntax-error counterexample.
 auto failed=packet::parse(nested(99,"<bad a=>"));check(failed.code==18,"packet depth error precedence changed");
 check(packet::parse(nested(98,"<bad a=>")).code==7,"pre-boundary syntax error changed");
 std::cout<<"PASS live packet XML depth oracle cases="<<cases<<" checks="<<checks<<"; full diagnostics/tree compared; packet selected compatibility source, original DLL NOT_RUN\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
