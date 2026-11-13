// SPDX-License-Identifier: GPL-3.0-only

#include "backend.hpp"
#include "benchmark_cases.hpp"
#include "../../fixtures/xml_reader/wire_contracts.hpp"
#include <algorithm>
#include <numeric>
#include <cstdint>
#include "oracle_reader.hpp"
#include "messages_fixtures.hpp"
#include "storage_fixtures.hpp"
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <tuple>
namespace eval {
namespace msg=citrine_eclipse_mirage::messages;
namespace access=citrine_eclipse_mirage::dispatch;
struct Profile {
 std::string name;
 citrine_eclipse_mirage::xml_tiny_reference::Backend tiny;
 xml_comparison::Backend pugi;
 explicit Profile(std::string name):name(std::move(name)){}
 msg::XmlBackend& backend(){return name=="tiny"?static_cast<msg::XmlBackend&>(tiny):pugi;}
 access::NodeAccess& nodes(){return name=="tiny"?static_cast<access::NodeAccess&>(tiny):pugi;}
};
std::string hex(std::string_view s){std::string out;for(unsigned char c:s){out+="0123456789abcdef"[c>>4];out+="0123456789abcdef"[c&15];}return out;}
std::string unhex(std::string_view s){std::string out;for(std::size_t i=0;i<s.size();i+=2)out.push_back(char(std::stoul(std::string(s.substr(i,2)),nullptr,16)));return out;}
void require(bool b,const char* why){if(!b)throw std::runtime_error(why);}
std::string snapshot(const msg::XmlNode* root,access::NodeAccess& nodes){
 std::string out;std::vector<const msg::XmlNode*> stack;
 while(root||!stack.empty()){
  if(!root){out+=']';root=stack.back();stack.pop_back();continue;}
  out+='[';out+=hex(root->name());for(auto& [k,v]:nodes.attributes_in_order(*root)){out+=':';out+=hex(k);out+='=';out+=hex(v);}
  out+=';';const char* text=nodes.text_003860(*root);out+=text?hex(text):"NULL";
  stack.push_back(root->next_element());root=root->first_element();
 }
 return out;
}
std::vector<std::unique_ptr<Profile>> profiles(){
 std::vector<std::unique_ptr<Profile>> result;
 result.push_back(std::make_unique<Profile>("tiny"));
 result.push_back(std::make_unique<Profile>("pugi_checked"));
 return result;
}
void corpus(const char* corpus,const char* output){
 std::ifstream f(corpus);require(bool(f),"corpus input missing");std::ofstream detail(std::string(output)+".differences.tsv");
 detail<<"profile\trow\tbaseline_code\tcandidate_code\tkind\tinput_hex\tcandidate_diagnostic_hex\tbaseline_diagnostic_hex\tbaseline_tree\tcandidate_tree\n";
 auto ps=profiles();struct Totals{std::size_t acceptance=0,tree=0,failed_tree=0,code=0,diagnostic=0,exact=0,policy=0,depth=0,unexpected=0;};std::vector<Totals> totals(ps.size());
 std::string line;std::size_t rows=0;
 while(std::getline(f,line)){
  ++rows;auto at=line.find(' ');auto s=unhex(std::string_view(line).substr(at+1));auto count=std::stoull(line.substr(0,at));require(count<=s.size(),"bad corpus slice");s.resize(count);
  auto baseline=ps[0]->backend().parse_c_string({s.data(),s.size()});auto base_tree=snapshot(baseline.document->first_element(),ps[0]->nodes());
  // reference_corpus separately validates every frozen row and its reviewed corrections.
  for(std::size_t i=0;i<ps.size();++i){
   auto parsed=ps[i]->backend().parse_c_string({s.data(),s.size()});auto tree=snapshot(parsed.document->first_element(),ps[i]->nodes());auto& t=totals[i];
   bool a=baseline.has_error!=parsed.has_error,b=(!baseline.has_error&&!parsed.has_error&&base_tree!=tree),c=baseline.returned_code!=parsed.returned_code,d=baseline.document->error_name()!=parsed.document->error_name();
   bool declared_depth=false;
   if(i==1&&(rows==83||rows==84)) {
    const unsigned depth=rows==83?100:498;std::string source,old_tree;
    for(unsigned n=0;n<depth;++n){source+="<R>";old_tree+="[52;NULL";}
    for(unsigned n=0;n<depth;++n){source+="</R>";old_tree+=']';}
    require(s==source&&!baseline.has_error&&baseline.returned_code==0&&base_tree==old_tree&&baseline.document->error_name().empty(),"immutable old-control depth input/result");
    require(parsed.returned_code==18&&parsed.has_error&&tree.empty()&&parsed.document->error_name()==
      "Error=XML_ELEMENT_DEPTH_EXCEEDED ErrorID=18 (0x12) Line number=1: Element nesting is too deep.","explicit A12 corpus depth rejection");
    declared_depth=true;++t.depth;
   }
   t.acceptance+=a;t.tree+=b;t.failed_tree+=(baseline.has_error&&parsed.has_error&&base_tree!=tree);t.code+=c;t.diagnostic+=d;t.exact+=!(a||base_tree!=tree||c||d);t.policy+=parsed.returned_code>=1000;
   t.unexpected+=(a||base_tree!=tree||c||d)&&!declared_depth;
   if(a||b||c||d)detail<<ps[i]->name<<'\t'<<rows<<'\t'<<baseline.returned_code<<'\t'<<parsed.returned_code<<'\t'<<(declared_depth?"A12_DEPTH_100":a?"acceptance":b?"tree":c?"code":"diagnostic")<<'\t'<<hex(s)<<'\t'<<hex(parsed.document->error_name())<<'\t'<<hex(baseline.document->error_name())<<'\t'<<base_tree<<'\t'<<tree<<'\n';
  }
 }
 require(rows==12087,"corpus row count");std::ofstream summary(output);summary<<"profile\tcases\tacceptance_diff\tsuccess_DOM_diff\tfailed_DOM_diff\terror_code_diff\tdiagnostic_diff\texact_equal\tpolicy_rejections\tdeclared_depth_deltas\tunexpected_differences\n";
 for(std::size_t i=0;i<ps.size();++i){auto t=totals[i];summary<<ps[i]->name<<'\t'<<rows<<'\t'<<t.acceptance<<'\t'<<t.tree<<'\t'<<t.failed_tree<<'\t'<<t.code<<'\t'<<t.diagnostic<<'\t'<<t.exact<<'\t'<<t.policy<<'\t'<<t.depth<<'\t'<<t.unexpected<<'\n';}
 require(totals[1].depth==2&&totals[1].exact+2==rows&&totals[1].unexpected==0,"only the two declared A12 corpus depth changes are allowed");
 std::cout<<"PASS: differential results recorded (reference corpus validated by its separate target) for "<<rows<<" cases\n";
}
void compatibility_cases(const char* output){
 struct Case {const char* name;std::string input;int code;int candidate_code=-1;};
 std::vector<Case> cases={
  {"BOM-after-whitespace"," \t\n\xef\xbb\xbf<R>a\n\rb</R>",0},
  {"newline-pairs","<R a='a\r\n\rb'>a\n\r\nb</R>",0},
  {"reference-order","<R a='&#13;&#10;&#0;'>&amp;&#x100000000;&unknown;</R>",0},
  {"trailing-less-than","raw\ntext<",15},
  {"unknown-node","<R><!odd>text</R>",0},
  {"declarations-only","<?a?><?b?>",0},
  {"declaration-after-text","raw<?pi?>",11},
  {"declaration-in-element","<R><?pi?></R>",11},
  {"declaration-after-comment","<!--x--><?pi?>",11},
  {"attribute-less-than","<R a='<'/>",0},
  {"vertical-whitespace","<R> \r\v\n</R>",0},
  {"form-feed-before-CDATA","<R><!--c-->\f<![CDATA[]]>tail</R>",0},
  {"extended-tag-whitespace","<R \fa='x'\vb='y'/>",0},
  {"root-close-ignores-tail","</R>ignored<broken",0},
  {"closing-tag-attributes","<R></R a='x'>",0},
  {"slash-closed-closing-spelling","</R/>",0},
  {"whitespace-after-less-than","< \nR/>",0},
  {"missing-parent-close","<R><C/>",15},
  {"open-tag-at-EOF","<R>",14},
  {"unclosed-text","<R>payload",8},
  {"single-less-than","<",15},
  {"long-element-diagnostic","<"+std::string(10000,'N'),6},
  {"long-attribute-diagnostic","<"+std::string(10000,'N')+" a=>",7},
  {"long-declaration-diagnostic","<R><?"+std::string(10000,'x')+"\r\n?></R>",11}
 };
 std::string open,close;for(unsigned i=0;i<498;++i){open+="<R>";close+="</R>";}
 cases.push_back({"deep-self-closed-leaf",open+"<R/>"+close,0,18});
 cases.push_back({"deep-open-at-EOF",open+"<R>",14,18});
 cases.push_back({"deep-entered-child-list",open+"<R><C/></R>"+close,18});
 open.clear();close.clear();std::string legacy;
 for(unsigned i=0;i<98;++i){open+="<R>";legacy+="< R>";close+="</R>";}
 cases.push_back({"a12-paired-98",open+close,0});
 cases.push_back({"a12-paired-99",open+"<R></R>"+close,0,18});
 cases.push_back({"a12-self-closed-leaf-99",open+"<L/>"+close,0});
 cases.push_back({"a12-open-at-EOF",open+"<L>",14});
 cases.push_back({"a12-entered-child-list",open+"<L><C/></L>"+close,0,18});
 cases.push_back({"a12-legacy-entered-child-list",legacy+"<L><C/></L>"+close,0,18});
 auto ps=profiles();std::ofstream out(output);
 out<<"case\tprofile\tcode\tdiagnostic_hex\ttree\tpolicy_delta\n";
 unsigned depth_deltas=0;
 for(const auto& test:cases) {
  std::string reference_tree,reference_diagnostic;
  for(auto& p:ps) {
   auto result=p->backend().parse_c_string(test.input);
   auto tree=snapshot(result.document->first_element(),p->nodes());
   const int expected=p->name=="tiny"||test.candidate_code<0?test.code:test.candidate_code;
   require(result.returned_code==expected&&result.has_error==(expected!=0),test.name);
   require(result.document->error_name().size()<=999,"bounded legacy diagnostic");
   if(p->name=="tiny"){reference_tree=tree;reference_diagnostic=result.document->error_name();}
   else if(expected!=test.code) {
    // Only these named, explicit cases differ from the immutable depth-500
    // control. Keep its old expected status and assert the new exact failure.
    require(expected==18&&tree.empty()&&result.document->error_name()==
      "Error=XML_ELEMENT_DEPTH_EXCEEDED ErrorID=18 (0x12) Line number=1: Element nesting is too deep.",test.name);
    ++depth_deltas;
   }else require(tree==reference_tree&&result.document->error_name()==reference_diagnostic,test.name);
   out<<test.name<<'\t'<<p->name<<'\t'<<result.returned_code<<'\t'<<hex(result.document->error_name())<<'\t'<<tree<<'\t'
      <<(p->name!="tiny"&&expected!=test.code?"A12_DEPTH_100":"NONE")<<'\n';
  }
 }
 require(depth_deltas==5,"all five declared old-control/A12 depth deltas must be observed");
 std::cout<<"PASS: "<<cases.size()<<" explicit grammar, normalization, depth and full-diagnostic boundaries; "<<depth_deltas<<" declared A12 depth-policy deltas\n";
}
void safety(const char* output){
 std::vector<std::pair<std::string,std::string>> cases={
 {"valid-reference","<R a='&#65;&#x1F600;'>&#65;&#x1F600;</R>"},
 {"max-reference","<R a='&#x10FFFF;'>&#1114111;</R>"},
 {"overflow-32","<R a='prefix&#x100000000;suffix'>prefix&#x100000000;suffix</R>"},
 {"overflow-64","<R a='&#x10000000000000000;'>&#x10000000000000000;</R>"},
 {"outside-unicode","<R a='&#x110000;'>&#1114112;</R>"},
 {"surrogate-reference","<R a='&#xD800;'>&#xD800;</R>"},
 {"zero-reference","<R a='&#0;'>&#0;</R>"},
 {"invalid-digit","<R a='&#xGG;'>&#xGG;</R>"},
 {"unknown-after-valid","<R a='&amp;&#65;&unknown;'>&amp;&#65;&unknown;</R>"},
 {"escaped-reference","<R a='&amp;#x100000000;'>&amp;#x100000000;</R>"},
 {"cdata-reference","<R><![CDATA[&#x100000000;]]></R>"},
 {"comment-reference","<R><!--&#x100000000;-->safe</R>"},
 {"duplicate-attribute","<R a='1' a='2'/>"},
 {"attribute-less-than","<R a='<'/>"},
 {"multiple-roots","<R/><Other/>"},
 {"invalid-utf8",std::string("<R a='\xc0\xaf'>x</R>")},
 {"lfcr","<R a='a\n\rb'>a\n\rb</R>"},
 {"empty-cdata","<R><![CDATA[]]></R>"},
 {"external-file-entity","<!DOCTYPE R [<!ENTITY xxe SYSTEM 'file:///xml-assessment-canary-not-expanded'>]><R>&xxe;</R>"},
 {"external-http-entity","<!DOCTYPE R [<!ENTITY xxe SYSTEM 'http://127.0.0.1:9/xml-assessment-canary'>]><R>&xxe;</R>"},
 {"internal-entity-expansion","<!DOCTYPE R [<!ENTITY a '1234567890'><!ENTITY b '&a;&a;&a;&a;&a;&a;&a;&a;&a;&a;'><!ENTITY c '&b;&b;&b;&b;&b;&b;&b;&b;&b;&b;'>]><R>&c;</R>"},
 {"large-text","<R>"+std::string(1024*1024,'a')+"</R>"}
 };
 for(unsigned n:{97u,98u,99u,100u,498u,499u,1000u,20000u}){std::string s;for(unsigned i=0;i<n;++i)s+="<R>";for(unsigned i=0;i<n;++i)s+="</R>";cases.emplace_back("depth-"+std::to_string(n),std::move(s));}
 std::string attributes="<R";for(unsigned i=0;i<12000;++i)attributes+=" a"+std::to_string(i)+"='x'";attributes+="/>";cases.emplace_back("wide-attributes",std::move(attributes));
 std::string zeroes="<R a='&#"+std::string(20000,'0')+"65;'/>";cases.emplace_back("leading-zero-reference",std::move(zeroes));
 // These cases exercise the common value policy independently of each
 // parser's acceptance, whitespace, depth and diagnostic differences.
 const std::vector<std::string> common={"valid-reference","max-reference","overflow-32",
  "overflow-64","outside-unicode","surrogate-reference","zero-reference","invalid-digit",
  "unknown-after-valid","escaped-reference","cdata-reference","comment-reference","leading-zero-reference"};
 std::map<std::string,std::pair<std::optional<std::string>,std::optional<std::string>>> reference_values;
 auto ps=profiles();std::ofstream out(output);out<<"profile\tcase\tcode\troot\tattribute_hex\ttext_hex\telapsed_us\n";
 for(auto& p:ps)for(auto& [name,s]:cases){
  auto begin=std::chrono::steady_clock::now();auto parsed=p->backend().parse_c_string(s);auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-begin).count();auto n=parsed.document->first_element();
  auto attr=n?n->attribute("a"):std::nullopt;auto text=n?p->nodes().text_003860(*n):nullptr;
  if(name.starts_with("depth-")) {
   const auto depth=std::stoul(name.substr(6));const bool rejected=depth>=(p->name=="tiny"?499u:99u);
   require(parsed.returned_code==(rejected?18:0)&&parsed.has_error==rejected,"profile-specific depth safety boundary");
   require(rejected?!n:bool(n),"depth status agrees with document visibility");
  }
  if(std::find(common.begin(),common.end(),name)!=common.end()) {
   require(!parsed.has_error,"shared value-policy input was rejected");
   auto values=std::make_pair(attr?std::optional<std::string>(*attr):std::nullopt,
                             text?std::optional<std::string>(text):std::nullopt);
   if(p->name=="tiny")reference_values.emplace(name,values);
   else require(reference_values.at(name)==values,"shared value policy differs between readers");
  }
  out<<p->name<<'\t'<<name<<'\t'<<parsed.returned_code<<'\t'<<(n?std::string(n->name()):"NULL")<<'\t'<<(attr?hex(*attr):"NULL")<<'\t'<<(text?hex(std::string_view(text).substr(0,200)):"NULL")<<'\t'<<us<<'\n';
  if(p->name=="pugi_checked"&&name=="overflow-32")require(!parsed.has_error&&attr=="prefix&#x100000000;suffix"&&text&&std::string_view(text)=="prefix&#x100000000;suffix","checked decoder failed known overflow case");

  if((p->name=="tiny"||p->name=="pugi_checked")&&name=="cdata-reference")require(!parsed.has_error&&text&&std::string_view(text)=="&#x100000000;","CDATA must bypass reference processing");
  if((p->name=="tiny"||p->name=="pugi_checked")&&name=="comment-reference")require(!parsed.has_error&&text&&std::string_view(text)=="safe","comment must bypass reference processing");


  if(name=="external-file-entity"||name=="external-http-entity")require(parsed.has_error||(text&&std::string_view(text)=="&xxe;"),"external entity unexpectedly expanded");
  if(name=="internal-entity-expansion")require(parsed.has_error||(text&&std::string_view(text)=="&c;"),"internal DTD entity unexpectedly expanded");
 }
 std::cout<<"PASS: targeted safety/correctness matrix; "<<cases.size()<<" cases across "<<ps.size()<<" profiles\n";
}
struct Outcome {std::string output,input;unsigned encrypted=0,decrypts=0,encrypts=0,fixed=0,keys=0;std::vector<std::uint32_t> handlers;std::string error;};
Outcome flow(Profile& p,std::string input,bool encrypted=false,bool decrypt_fail=false){
 Factory factory;FakeRandom random;FakeEffects effects;FakeCipher cipher;Queue queue;
 std::uint8_t flag=encrypted,show=0;
 if(encrypted||input=="deadbeef"){cipher.trace->decrypt_outputs.push_back("<LSX><Request id='1' recipient='EALS'><GetConfig/></Request></LSX>");}
 if(decrypt_fail)cipher.trace->decrypt_results.push_back(false);
 Outcome r;try{r.output=msg::process_message_033d70({flag,show,cipher,queue},input,p.backend(),factory,random,effects);}catch(const Invalid&){r.error="invalid-message";}catch(const std::exception& e){r.error=e.what();}
 r.input=input;r.encrypted=flag;r.decrypts=cipher.trace->decrypted.size();r.encrypts=cipher.trace->encrypted.size();r.fixed=cipher.trace->fixed;r.keys=cipher.trace->key;r.handlers=effects.handlers;return r;
}
void protocol(const char* output){
 auto ps=profiles();std::ofstream out(output);out<<"profile\tcase\tresponse_hex\tfinal_input_hex\tencrypted\tdecrypts\tencrypts\terror\tcompared_with_tiny\n";
 std::vector<std::tuple<std::string,std::string,bool,bool>> cases;
 for(auto reg:msg::registrations_032d00())cases.emplace_back(std::string(reg.name),"<LSX><Request id='1' recipient='EALS'><"+std::string(reg.name)+" a='A&amp;B'/></Request></LSX>",false,false);
 cases.emplace_back("challenge-derived","<LSX><Request id='1' recipient='EALS'><ChallengeResponse key='ABC' version='3'/></Request></LSX>",false,false);
 cases.emplace_back("challenge-fixed","<LSX><Request id='1' recipient='EALS'><ChallengeResponse key='ABC' version='2'/></Request></LSX>",false,false);
 cases.emplace_back("encrypted-detection","deadbeef",false,false);
 cases.emplace_back("already-encrypted","00",true,false);
 cases.emplace_back("decrypt-failure","00",true,true);
 cases.emplace_back("empty-version-retry","<LSX version=\"\" version=\"3\"><Request id='1' recipient='EALS'><GetConfig/></Request></LSX>",false,false);
 cases.emplace_back("multiple-command","<LSX><Request id='1' recipient='EALS'><GetConfig/><GetProfile/></Request></LSX>",false,false);
 cases.emplace_back("invalid-root","<Other/>",false,false);

 for(auto& [name,s,enc,fail]:cases){
  auto reference=flow(*ps[0],s,enc,fail);
  for(auto& p:ps){auto r=flow(*p,s,enc,fail);bool same=std::tie(r.output,r.input,r.encrypted,r.decrypts,r.encrypts,r.fixed,r.keys,r.handlers,r.error)==std::tie(reference.output,reference.input,reference.encrypted,reference.decrypts,reference.encrypts,reference.fixed,reference.keys,reference.handlers,reference.error);
   out<<p->name<<'\t'<<name<<'\t'<<hex(r.output)<<'\t'<<hex(r.input)<<'\t'<<r.encrypted<<'\t'<<r.decrypts<<'\t'<<r.encrypts<<'\t'<<r.error<<'\t'<<same<<'\n';
   require(same,"candidate changed actual message state/output behavior");

  }
 }

 std::cout<<"PASS: "<<cases.size()<<" actual process/dispatch/challenge protocol cases across "<<ps.size()<<" reader profiles; fake crypto/effects, real XML and MemoryPrinter\n";
}
void wire_contracts() {
 auto ps=profiles();
 for(auto& profile:ps) {
  auto fields=[&](const std::string& input) {
   auto parsed=profile->backend().parse_c_string(input);require(!parsed.has_error,"wire fixture rejected");
   const auto* root=parsed.document->first_element();require(root&&root->name()=="LSX","wire root");
   const auto* request=root->first_element();require(request&&request->name()=="Request","wire request");
   const auto* command=request->first_element();require(command&&command->name()=="GetProfile","wire command");
   auto id=request->attribute("id"),recipient=request->attribute("recipient"),index=command->attribute("index");
   require(id&&recipient&&index,"wire required attributes");
   return std::make_tuple(std::string(*id),std::string(*recipient),std::string(*index));
  };
  using namespace xml_contract_fixture;
  const auto canonical=std::make_tuple(std::string("public-dll"),std::string("EbisuSDK"),std::string("0"));
  require(fields(profile_request())==canonical,"wire canonical fields differ from explicit oracle");
  for(const auto& [name,input]:equivalent_forms())require(fields(input)==canonical,"wire equivalent form changed parsed fields");
  for(const auto& value:equivalent_values()) {
   const auto expected=std::make_tuple(std::string("public-dll"),std::string("EbisuSDK"),value.decoded);
   require(fields(profile_request(value.literal))==expected,"wire literal differs from explicit value oracle");
   require(fields(profile_request(value.reference))==expected,"wire reference differs from explicit value oracle");
  }
  require(fields(profile_request("A","EbisuSDK","wire-next"))==std::make_tuple(std::string("wire-next"),std::string("EbisuSDK"),std::string("A")),"wire fields stale across requests");
  for(const auto& input:malformed_frames())require(profile->backend().parse_c_string(input).has_error,"malformed wire fixture accepted");
 }
 std::cout<<"PASS: actual DLL wire fixtures: 8 equivalent forms, 5 value pairs and 3 malformed inputs on both readers\n";
}
void ownership(){
 auto ps=profiles();for(auto& p:ps){
  std::string s="<R a='retained&amp;&#65;&unknown;'><C>payload&amp;&#65;</C>";
  for(unsigned i=0;i<2048;++i)s+="<N a='v&amp;'/>";
  s+="</R>";
  auto parsed=p->backend().parse_c_string(s);require(!parsed.has_error,"owned parse");s.assign(s.size(),'X');auto r=parsed.document->first_element();auto child=r->first_element();
  auto retained=r->attribute("a");const auto* retained_text=p->nodes().text_003860(*child);
  require(retained=="retained&A&unknown;","caller buffer and decoded value ownership");
  auto identity=child;unsigned count=0;
  for(auto n=child;n;n=n->next_element()){++count;(void)n->attribute("a");}
  require(count==2049&&r->first_element()==identity,"wrapper rehash identity");
  require(r->attribute("a")->data()==retained->data(),"decoded attribute view survives cache rehash");
  require(p->nodes().text_003860(*child)==retained_text,"decoded text pointer survives cache rehash");
  for(unsigned i=0;i<100;++i){auto other=p->backend().parse_c_string("<Other/>");require(!other.has_error,"independent doc");}
  require(std::string_view(p->nodes().text_003860(*identity))=="payload&A","document survives later parses");
  auto empty=p->backend().parse_c_string(std::string_view{});require(empty.has_error&&!empty.document->first_element(),"empty input fails cleanly");
  std::string nul="<R/>";nul.push_back(0);nul+="<broken";auto bounded=p->backend().parse_c_string(nul);require(!bounded.has_error&&bounded.document->first_element()->name()=="R","first NUL contract");
 }
 std::cout<<"PASS: document/buffer/wrapper lifetimes, independent parses and first-NUL semantics across "<<ps.size()<<" profiles\n";
}
void protocol_edges(const char* output){
 auto ps=profiles();std::ofstream out(output);out<<"profile\tcase\tresponse_hex\tfinal_input_hex\tencrypted\tdecrypts\tencrypts\terror\tequal_to_tiny\n";
 const std::string request="<LSX><Request id='1' recipient='EALS'><GetConfig/></Request></LSX>";
 std::vector<std::pair<std::string,std::string>> cases={
  {"trailing-junk",request+"junk"},
  {"leading-junk","junk"+request},
  {"duplicate-version","<LSX version='2' version='3'><Request id='1' recipient='EALS'><GetConfig/></Request></LSX>"},
  {"unknown-reference","<LSX><Request id='1' recipient='EALS'><GetConfig a='&unknown;'/></Request></LSX>"},
  {"invalid-scalar","<LSX><Request id='1' recipient='EALS'><GetConfig a='&#0;'/></Request></LSX>"},
  {"malformed-end","<LSX><Request id='1' recipient='EALS'><GetConfig/></Request></OTHER>"},
  {"comment-only","<!--comment-->"},
  {"processing-instruction","<?pi test?>"},
  {"empty",""}
 };
 for(auto& [name,input]:cases){auto baseline=flow(*ps[0],input);for(auto& p:ps){auto r=flow(*p,input);bool same=std::tie(r.output,r.input,r.encrypted,r.decrypts,r.encrypts,r.fixed,r.keys,r.handlers,r.error)==std::tie(baseline.output,baseline.input,baseline.encrypted,baseline.decrypts,baseline.encrypts,baseline.fixed,baseline.keys,baseline.handlers,baseline.error);
  out<<p->name<<'\t'<<name<<'\t'<<hex(r.output)<<'\t'<<hex(r.input)<<'\t'<<r.encrypted<<'\t'<<r.decrypts<<'\t'<<r.encrypts<<'\t'<<r.error<<'\t'<<same<<'\n';
  require(same,"candidate changed negative message state/output behavior");

 }}
 std::cout<<"PASS: negative protocol matrix recorded; "<<cases.size()<<" cases across "<<ps.size()<<" profiles, differences remain visible in the TSV\n";
}
void storage(const char* output){
 auto ps=profiles();std::ofstream out(output);out<<"profile\tcase\tstate_hex\tini_hex\tequal_to_tiny\n";
 std::vector<std::pair<std::string,std::string>> cases={
  {"valid","<Achievements><A Id='known' Count='4' Progress='0x10' Grant='stamp' Name='A&amp;B'/><A Id='new' Count='12tail' Name='new'/></Achievements>"},
  {"duplicate-row","<R><A Id='new' Count='4' Name='first'/><A Id='new' Name='second'/></R>"},
  {"invalid-document","<R><A Id='new'/></broken>"},
  {"empty-document",""},
  {"embedded-nul",std::string("<R><A Id='new'/></R>\0<broken",26)},
  {"overflow-id","<R><A Id='pre&#x100000000;suffix' Count='4'/></R>"},
  {"overflow-name","<R><A Id='new' Name='pre&#x100000000;suffix'/></R>"},
  {"lfcr-name","<R><A Id='new' Name='a\r\n\rb'/></R>"}
 };
 for(auto& [name,input]:cases){
  std::pair<std::string,std::string> baseline;
  for(auto& p:ps){
   eval_storage::Fixture f;f.paths.xml_path=u"owned-fixture.xml";f.state.steam_app_id="42";f.effects.status={1,0,2};f.state.achievements["known"]=eval_storage::record("configured");f.files.input[f.paths.xml_path]=input;
   citrine_eclipse_mirage::achievement_storage::Storage s(f.paths,p->backend(),f.numbers,f.effects,f.files);s.update_01eb00(f.state);
   std::string state;for(auto& [id,a]:f.state.achievements)state+=hex(id)+":"+std::to_string(a.field04)+":"+std::to_string(a.field08)+":"+hex(a.display_name)+":"+hex(a.timestamp)+";";
   require(f.files.output.size()==1,"storage must publish one INI fixture");auto ini=f.files.output.begin()->second;
   if(p->name=="tiny")baseline={state,ini};bool same=baseline==std::make_pair(state,ini);
   out<<p->name<<'\t'<<name<<'\t'<<hex(state)<<'\t'<<hex(ini)<<'\t'<<same<<'\n';
   require(same,"reader changed tested achievement state/INI bytes");
   if((p->name=="tiny"||p->name=="pugi_checked")&&name=="overflow-id")require(f.state.achievements.contains("pre&#x100000000;suffix"),"reader must preserve entire literal achievement key");
  }
 }
 std::cout<<"PASS: "<<cases.size()<<" real achievement Storage/writer cases across "<<ps.size()<<" profiles; in-memory files/effects, exact state and INI bytes\n";
}
void fuzz(unsigned iterations,const char* output,bool holdout=false){
 const unsigned seed=holdout?0xc17e527:0x51a7b00;
 std::mt19937 random(seed);auto ps=profiles();std::size_t parses=0,differences=0;
 std::ofstream detail(std::string(output)+".differences.tsv");
 detail<<"iteration\tinput_hex\treference_code\tcandidate_code\treference_diagnostic_hex\tcandidate_diagnostic_hex\treference_tree\tcandidate_tree\n";
 std::vector<std::string> seeds={"<LSX><Request id='1' recipient='EALS'><GetConfig a='&#65;'/></Request></LSX>","<R><![CDATA[&#0;]]></R>","<R a='prefix&#x100000000;suffix'/>","<!--x--><R a='x'/>","<R><C/></R>"};
 if(holdout)seeds={"<?a?><?b?><R/>","<?xml version='1.0'?><!--x--><R a='t\r\n\n\rb'/>",
  "<R><?x?></R>","<!DOCTYPE R [<!ENTITY e 'x'>]><R>&e;</R>","<R><!odd>text</R>",
  "</R>junk","</R/>","<R></R a='v'>","< R/><\nS/>","<R \fa='v'\vb='w'/>",
  "<R>a\n<","<R> \t\r\n</R>","<R><!--c--> \t <![CDATA[]]>x</R>",
  "<R a='<' b='&amp;#13;'/>"," \r\n\xef\xbb\xbf<R/>","raw\ntext<R/>",
  "<R><![CDATA[A\r\n\rB]]></R>","<R><C></R>"};
 for(unsigned i=0;i<iterations;++i){std::string s=seeds[random()%seeds.size()];unsigned edits=1+random()%12;
  if(holdout&&random()%4==0){s.resize(random()%193);for(auto& byte:s)byte=char(random()%256);}
  for(unsigned e=0;e<edits;++e){auto at=random()%(s.size()+1);switch(random()%3){case 0:s.insert(at,1,char(random()%256));break;case 1:if(at<s.size())s.erase(at,1);break;default:if(at<s.size())s[at]=char(random()%256);}}
  std::string reference_tree,reference_diagnostic;int reference_code=0;bool reference_error=false;
  for(auto& p:ps){auto parsed=p->backend().parse_c_string(s);auto tree=snapshot(parsed.document->first_element(),p->nodes());
   if(p->name=="tiny") {reference_tree=tree;reference_code=parsed.returned_code;reference_error=parsed.has_error;reference_diagnostic=parsed.document->error_name();}
   else if(reference_code!=parsed.returned_code||reference_error!=parsed.has_error||reference_tree!=tree||reference_diagnostic!=parsed.document->error_name()) {
    ++differences;detail<<i<<'\t'<<hex(s)<<'\t'<<reference_code<<'\t'<<parsed.returned_code<<'\t'<<hex(reference_diagnostic)<<'\t'<<hex(parsed.document->error_name())<<'\t'<<reference_tree<<'\t'<<tree<<'\n';
   }
   ++parses;
  }
 }
 std::ofstream out(output);out<<"seed,mutated_inputs,profiles,parse_attempts,differences\n"<<seed<<','<<iterations<<','<<ps.size()<<','<<parses<<','<<differences<<'\n';
 std::cout<<"Deterministic differential mutations: "<<iterations<<" inputs / "<<parses<<" parses / "<<differences<<" differences (not exhaustive fuzz assurance)\n";
 require(differences==0,"candidate differs from reference on deterministic mutations");
}
void probe() {
 auto ps=profiles();std::string line;
 while(std::getline(std::cin,line)) {
  const auto input=unhex(line);
  for(auto& p:ps) {
   auto parsed=p->backend().parse_c_string(input);
   std::cout<<p->name<<'\t'<<parsed.returned_code<<'\t'<<parsed.has_error<<'\t'<<hex(parsed.document->error_name())<<'\t'<<snapshot(parsed.document->first_element(),p->nodes())<<'\n';
  }
 }
}
// All profiles perform the same parse, complete traversal and destruction.
// Hash every value so benchmark work cannot disappear or silently change semantics.
std::uint64_t consume(Profile& profile,const msg::XmlNode* root) {
 return xml_comparison::consume_xml(root,profile.nodes());
}
void benchmark(unsigned iterations,unsigned repeats,unsigned seed,const char* output){
 require(iterations>=32&&iterations<=100000,"benchmark iterations must be in 32..100000");
 require(repeats>=10&&repeats<=100,"benchmark repeats must be in 10..100");
 auto ps=profiles();const auto cases=xml_comparison::benchmark_cases();
 std::mt19937 random(seed);std::vector<unsigned> first_profile(cases.size());
 for(auto& first:first_profile)first=random()%2;
 std::ofstream out(output);
 require(bool(out),"cannot open benchmark report");
 out<<"profile,case,repeat,order,iterations,input_bytes,mean_ns,p50_ns,p95_ns,p99_ns,sink,input_hash,seed\n"<<std::setprecision(17);
 for(unsigned repeat=0;repeat<repeats;++repeat) {
  std::vector<std::size_t> order(cases.size());std::iota(order.begin(),order.end(),0);
  std::shuffle(order.begin(),order.end(),random);
  for(auto index:order) {
   const auto& item=cases[index];const auto& input=item.input;
   auto reference=ps[0]->backend().parse_c_string(input);auto candidate=ps[1]->backend().parse_c_string(input);
   require(!reference.has_error&&!candidate.has_error,"benchmark fixture must parse successfully");
   require(snapshot(reference.document->first_element(),ps[0]->nodes())==snapshot(candidate.document->first_element(),ps[1]->nodes()),"benchmark semantic preflight mismatch");
   const auto expected=consume(*ps[0],reference.document->first_element());
   require(expected==consume(*ps[1],candidate.document->first_element()),"benchmark consumption mismatch");
   // Bound large-document work while retaining >= 32 separately timed samples.
   const auto count=std::min<unsigned>(iterations,std::max<std::size_t>(32,8*1024*1024/input.size()));
   std::uint64_t input_hash=1469598103934665603ULL;
   for(unsigned char c:input){input_hash^=c;input_hash*=1099511628211ULL;}
   const unsigned first=(first_profile[index]+repeat)%2;
   reference.document.reset();candidate.document.reset();
   for(unsigned position=0;position<2;++position) {
    auto& profile=*ps[(first+position)%2];
    auto& backend=profile.backend();auto& access=profile.nodes();
    for(unsigned i=0;i<50;++i){auto parsed=backend.parse_c_string(input);require(!parsed.has_error,"benchmark warmup failure");(void)xml_comparison::consume_xml(parsed.document->first_element(),access);}
    std::vector<double> durations;durations.reserve(count);std::uint64_t sink=0;
    const auto all_begin=std::chrono::steady_clock::now();
    for(unsigned i=0;i<count;++i) {
     const auto begin=std::chrono::steady_clock::now();
     {auto parsed=backend.parse_c_string(input);require(!parsed.has_error,"benchmark parse failure");const auto value=xml_comparison::consume_xml(parsed.document->first_element(),access);require(value==expected,"benchmark value changed");sink+=value;}
     durations.push_back(std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count());
    }
    const auto elapsed=std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-all_begin).count();
    std::sort(durations.begin(),durations.end());
    auto percentile=[&](unsigned p){return durations[(durations.size()-1)*p/100];};
    out<<profile.name<<','<<item.name<<','<<repeat<<','<<position<<','<<count<<','<<input.size()<<','<<elapsed/count<<','<<percentile(50)<<','<<percentile(95)<<','<<percentile(99)<<','<<sink<<','<<input_hash<<','<<seed<<'\n';
   }
  }
 }
 require(bool(out),"benchmark report write failed");
 std::cout<<"PASS: paired randomized Release parse/traverse/free timing with semantic preflight recorded\n";
}
}
int main(int argc,char** argv)try{
 using namespace eval;if(argc<2)return 2;std::string op=argv[1];
 if(op=="probe"&&argc==2)probe();
 else if(op=="corpus"&&argc==4)corpus(argv[2],argv[3]);
 else if(op=="compatibility"&&argc==3)compatibility_cases(argv[2]);
 else if(op=="safety"&&argc==3)safety(argv[2]);
 else if(op=="protocol"&&argc==3)protocol(argv[2]);
 else if(op=="protocol-edges"&&argc==3)protocol_edges(argv[2]);
 else if(op=="ownership")ownership();
 else if(op=="wire-contracts")wire_contracts();
 else if(op=="storage"&&argc==3)storage(argv[2]);
 else if(op=="fuzz"&&argc==4)fuzz(std::stoul(argv[2]),argv[3]);
 else if(op=="holdout"&&argc==4)fuzz(std::stoul(argv[2]),argv[3],true);
 else if(op=="benchmark"&&argc==6)benchmark(std::stoul(argv[2]),std::stoul(argv[3]),std::stoul(argv[4]),argv[5]);
 else return 2;return 0;
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL: unknown comparison exception\n";return 1;}
