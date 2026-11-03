// SPDX-License-Identifier: GPL-3.0-only

#include "backend.hpp"
#include "pugixml.hpp"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <new>
#include <string_view>
namespace probe {
struct Slot {void* pointer=nullptr;std::size_t bytes=0;};
std::array<Slot,8192> slots;
bool enabled=false,persistent=false;long fail=-1;
std::size_t calls=0,allocated=0,live=0,peak=0,outstanding=0;bool overflow=false;
void begin(long n,bool all){for(auto& s:slots)s={};calls=allocated=live=peak=outstanding=0;overflow=false;fail=n;persistent=all;enabled=true;}
void* allocate(std::size_t n){
 if(enabled){++calls;if(fail>=0){if(fail==0){if(!persistent)fail=-1;throw std::bad_alloc();}--fail;}}
 auto p=std::malloc(n?n:1);if(!p)throw std::bad_alloc();
 if(enabled){allocated+=n;live+=n;if(live>peak)peak=live;++outstanding;bool found=false;for(auto& s:slots)if(!s.pointer){s={p,n};found=true;break;}if(!found)overflow=true;}
 return p;
}
void release(void* p)noexcept{if(enabled&&p){for(auto& s:slots)if(s.pointer==p){live-=s.bytes;--outstanding;s={};break;}}std::free(p);}
void* pugi_allocate(std::size_t n){try{return allocate(n);}catch(const std::bad_alloc&){return nullptr;}}
void pugi_release(void* p){release(p);}
}
void* operator new(std::size_t n){return probe::allocate(n);}
void* operator new[](std::size_t n){return probe::allocate(n);}
void operator delete(void* p)noexcept{probe::release(p);}
void operator delete[](void* p)noexcept{probe::release(p);}
void operator delete(void* p,std::size_t)noexcept{probe::release(p);}
void operator delete[](void* p,std::size_t)noexcept{probe::release(p);}
int main(int argc,char** argv){
 std::set_terminate([]{probe::enabled=false;std::fputs("ASSESSMENT_TERMINATE\n",stderr);std::_Exit(86);});
 if(argc<2)return 2;std::string_view name=argv[1];long fail=argc>2?std::strtol(argv[2],nullptr,10):-1;bool persistent=argc>3&&std::string_view(argv[3])=="persistent";
 pugi::set_memory_management_functions(probe::pugi_allocate,probe::pugi_release);
 preloader_l::xml_reader_bridge::Backend tiny;xml_comparison::Backend candidate;
 if(name!="tiny"&&name!="pugi_checked")return 2;
 auto& backend=name=="tiny"?static_cast<preloader_l::messages::XmlBackend&>(tiny):static_cast<preloader_l::messages::XmlBackend&>(candidate);
 auto& nodes=name=="tiny"?static_cast<preloader_l::dispatch::NodeAccess&>(tiny):static_cast<preloader_l::dispatch::NodeAccess&>(candidate);
 const std::string_view payload=argc>4?argv[4]:"small";
 std::string input=payload=="rewrite"?"<R a='pre&amp;&#65;&unknown;mid&#0;suffix'><C>&#x110000;&#xD800;</C></R>":"<LSX><Request id='1' recipient='EALS'><GetProfile index='0'/></Request></LSX>";
 if(payload=="legacy")input="<R><!odd>\v<C a='A&amp;'>\n\r</C></R>";
 if(payload=="attributes") {
  input="<R";for(unsigned i=0;i<32;++i)input+=" a"+std::to_string(i)+"='&amp;'";input+="/>";
 }
 if(payload=="depth") {
  input.clear();for(unsigned i=0;i<32;++i)input+="<R>";
  for(unsigned i=0;i<32;++i)input+="</R>";
 }
 if(payload=="error")input="<R bad=>";
 const bool cache=payload=="cache-attribute"||payload=="cache-text";
 std::optional<preloader_l::messages::ParseResult> retained;
 const preloader_l::messages::XmlNode* cached_root=nullptr;
 if(cache) {
  input="<R a='prefix&amp;&#65;&unknown;suffix'>prefix&amp;&#65;&unknown;suffix</R>";
  retained.emplace(backend.parse_c_string(input));cached_root=retained->document->first_element();
 }
 int outcome=0;std::size_t sink=0;
 probe::begin(fail,persistent);
 if(cache) {
  auto lookup=[&] {
   auto value=payload=="cache-attribute"?cached_root->attribute("a"):
       std::optional<std::string_view>(nodes.text_003860(*cached_root));
   if(!value||*value!="prefix&A&unknown;suffix")throw std::logic_error("decoded cache retry changed input");
   sink+=value->size();
  };
  try{lookup();}catch(const std::bad_alloc&){outcome=2;}catch(...){outcome=3;}
  // Lift the injected failure and retry the very same parsed document.
  probe::fail=-1;probe::persistent=false;
  try{lookup();}catch(...){outcome=3;}
  retained.reset();
 }else {
  try{
   auto parsed=backend.parse_c_string(input);if(parsed.has_error)outcome=1;
   else {auto r=parsed.document->first_element();for(auto n=r;n;n=n->first_element()){sink+=n->name().size();for(auto& [key,value]:nodes.attributes_in_order(*n))sink+=key.size()+value.size();if(auto text=nodes.text_003860(*n))sink+=std::char_traits<char>::length(text);}}
  }catch(const std::bad_alloc&){outcome=2;}catch(...){outcome=3;}
 }
 probe::enabled=false;probe::fail=-1;
 std::printf("{\"profile\":\"%s\",\"fail_at\":%ld,\"persistent\":%s,\"outcome\":%d,\"calls\":%zu,\"allocated_bytes\":%zu,\"peak_live_bytes\":%zu,\"outstanding\":%zu,\"tracking_overflow\":%s,\"sink\":%zu}\n",argv[1],fail,persistent?"true":"false",outcome,probe::calls,probe::allocated,probe::peak,probe::outstanding,probe::overflow?"true":"false",sink);
 std::fflush(stdout); // Preserve the counters before a possible LSan exit report.
 // The pointer registry must not make a real leak look reachable to LSan.
 // Keep the reported counters; release only the registry's artificial roots.
 for(auto& slot:probe::slots)slot={};
 return probe::outstanding||probe::overflow||outcome==3?1:0;
}
