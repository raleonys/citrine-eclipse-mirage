#include "../../adopted_hf19/src/config/config2_pegtl.hpp"
#include <cstring>
#include <cassert>
using namespace preloader_l::config2_pegtl;
struct Seen {unsigned section=0,leaf=0;};
static bool sec(void* u,Span){++static_cast<Seen*>(u)->section;return true;}
static bool leaf(void* u,const Span*,size_t,Span,Span){++static_cast<Seen*>(u)->leaf;return true;}
int main(){
  Seen a{}; Visitor v{&a,sec,leaf};
  constexpr char good[]=R"CFG("Config2" { "User" { "Username" "SURGE" "Country" "TR" } })CFG";
  assert(parse(good,sizeof(good)-1,v)==Result::ok);
  assert(a.section==1 && a.leaf==2);
  constexpr char invalid[]=R"CFG("Config2" { "User" { "Username" "SURGE" })CFG";
  assert(parse(invalid,sizeof(invalid)-1,v)==Result::invalid);
  constexpr char tail[]=R"CFG("Config2" {} /*not closed )CFG";
  assert(parse(tail,sizeof(tail)-1,v)==Result::invalid);
  assert(parse(tail,sizeof(tail)-1,v,true)==Result::ok);
  return 0;
}
