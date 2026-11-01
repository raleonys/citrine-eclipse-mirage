#include "service_state_internal.hpp"
#include "../config/config2_pegtl.hpp"
#include <stdint.h>
namespace preloader_l { namespace runtime { namespace behavioral_internal {
using Tok=config2_pegtl::Span;
struct Cur{const char* d;};
ServiceTextView tv(const Cur& c,Tok t) noexcept{return{c.d+t.off,t.len};}
bool teq(const Cur& c,Tok t,const char* s) noexcept{return same(tv(c,t),s);}
bool i32(ServiceTextView v,int32_t* out) noexcept {
    if(!out||!v.data||!v.size)return false;
    size_t i=0; bool neg=false; if(v.data[0]=='-'||v.data[0]=='+'){neg=v.data[0]=='-'; if(++i==v.size)return false;}
    uint32_t x=0; const uint32_t limit=neg?UINT32_C(2147483648):UINT32_C(2147483647);
    for(;i<v.size;++i){char ch=v.data[i];if(ch<'0'||ch>'9')return false;uint32_t d=uint32_t(ch-'0');if(x>(limit-d)/10u)return false;x=x*10u+d;}
    *out=neg?(x==UINT32_C(2147483648)?INT32_MIN:-static_cast<int32_t>(x)):static_cast<int32_t>(x);return true;
}

struct AchName { ServiceTextView id,name; };
struct AchTotal { ServiceTextView id; int32_t total; };
struct ParseStage { Store store; AchName* names;size_t nn,nc;AchTotal* totals;size_t tn,tc; };
void free_stage(ParseStage& p) noexcept {clear_store(p.store);dealloc(p.names,p.nc*sizeof(AchName));dealloc(p.totals,p.tc*sizeof(AchTotal));p=ParseStage{};}
template<class T> bool grow_arr(T** data,size_t* cap,size_t used,size_t need) noexcept {if(need<=*cap)return true;size_t n=*cap?*cap:8u;while(n<need)n=n+n/2u+1u;if(n>static_cast<size_t>(-1)/sizeof(T))return false;T* q=static_cast<T*>(alloc(n*sizeof(T)));if(!q)return false;for(size_t i=0;i<used;++i)q[i]=(*data)[i];dealloc(*data,*cap*sizeof(T));*data=q;*cap=n;return true;}
bool add_name(ParseStage& p,AchName x) noexcept {if(!grow_arr(&p.names,&p.nc,p.nn,p.nn+1u))return false;p.names[p.nn++]=x;return true;}
bool add_total(ParseStage& p,AchTotal x) noexcept {if(!grow_arr(&p.totals,&p.tc,p.tn,p.tn+1u))return false;p.totals[p.tn++]=x;return true;}

bool ensure_content(ParseStage& p,ServiceTextView id,ContentRecord** out) noexcept {auto* r=find_content(p.store,id);if(!r){ContentRecordInput in{id,{"GAME",4},{"0",1},{"INSTALLED",9}};if(!append_content(p.store,in))return false;r=find_content(p.store,id);}*out=r;return true;}
bool ensure_entitlement(ParseStage& p,ServiceTextView id,EntitlementRecord** out) noexcept {auto* r=find_entitlement(p.store,id);if(!r){EntitlementRecordInput in{id,{"",0},{"0",1},{"DEFAULT",7},{"ONLINE_ACCESS",13}};if(!append_entitlement(p.store,in))return false;r=find_entitlement(p.store,id);}*out=r;return true;}

bool leaf(ParseStage& p,const Cur& c,const Tok* path,size_t depth,Tok key,Tok val) noexcept {
    if(depth>=2&&teq(c,path[0],"Content")){ContentRecord* r=nullptr;if(!ensure_content(p,tv(c,path[1]),&r))return false;if(depth!=2u)return true;const auto v=tv(c,val);if(teq(c,key,"Name"))return v.size?replace_text(r->display_name,v):true;if(teq(c,key,"Version"))return v.size?replace_text(r->version,v):true;if(teq(c,key,"State"))return v.size?replace_text(r->state,v):true;return true;}
    if(depth>=2&&teq(c,path[0],"Entitlements")){EntitlementRecord* r=nullptr;if(!ensure_entitlement(p,tv(c,path[1]),&r))return false;if(depth!=2u)return true;const auto v=tv(c,val);if(teq(c,key,"Group"))return replace_text(r->group,v);if(teq(c,key,"Version"))return replace_text(r->version,v);if(teq(c,key,"Type"))return replace_text(r->type,v);if(teq(c,key,"EntitlementTag"))return replace_text(r->entitlement_tag,v);return true;}
    if(depth>=2&&teq(c,path[0],"Achievements")&&teq(c,path[1],"AchievementNames"))return add_name(p,{tv(c,key),tv(c,val)});
    if(depth>=2&&teq(c,path[0],"Achievements")&&teq(c,path[1],"AchievementTotals")){int32_t x=0;if(!i32(tv(c,val),&x))return true;return add_total(p,{tv(c,key),x});}
    return true;
}

struct VisitContext { ParseStage* stage; Cur cursor; };
bool on_section(void*,Tok) noexcept{return true;}
bool on_leaf(void* opaque,const Tok* path,size_t depth,Tok key,Tok value) noexcept {
    auto& x=*static_cast<VisitContext*>(opaque);
    return leaf(*x.stage,x.cursor,path,depth,key,value);
}
bool build_cfg_store(const char* cfg,size_t len,Store* output) noexcept {
    if(!output||!cfg||!len)return false;
    ParseStage p{};
    VisitContext context{&p,Cur{cfg}};
    const config2_pegtl::Visitor visitor{&context,on_section,on_leaf};
    if(config2_pegtl::parse(cfg,len,visitor,true)!=config2_pegtl::Result::ok){free_stage(p);return false;}
    for(size_t i=0;i<p.tn;++i){ServiceTextView name{"",0};for(size_t j=0;j<p.nn;++j)if(same2(p.totals[i].id,p.names[j].id)){name=p.names[j].name;break;}if(!append_achievement(p.store,p.totals[i].id,name,p.totals[i].total)){free_stage(p);return false;}}
    for(size_t j=0;j<p.nn;++j){if(find_achievement(p.store,p.names[j].id))continue;if(!append_achievement(p.store,p.names[j].id,p.names[j].name,1u)){free_stage(p);return false;}}
    dealloc(p.names,p.nc*sizeof(AchName));dealloc(p.totals,p.tc*sizeof(AchTotal));p.names=nullptr;p.totals=nullptr;p.nn=p.nc=p.tn=p.tc=0;*output=p.store;p.store=Store{};return true;
}
} } }
