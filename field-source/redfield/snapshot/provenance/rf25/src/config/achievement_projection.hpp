// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "config2_pegtl.hpp"
#include "redfield/runtime/record_views.hpp"
namespace redfield { namespace achievement_projection { namespace {
using ServiceTextView=redfield::runtime::ServiceTextView;
using Tok=redfield::config2_pegtl::Span;
struct Cur {const char* d;};
bool same(ServiceTextView a,const char* b) noexcept {return std::string_view(a.data,a.size)==b;}
bool same2(ServiceTextView a,ServiceTextView b) noexcept {return std::string_view(a.data,a.size)==std::string_view(b.data,b.size);}
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
struct ParseStage {
    ConfigSnapshot& store;
    std::vector<AchName> names;
    std::vector<AchTotal> totals;
};
bool add_name(ParseStage& p,AchName x) noexcept {try{p.names.push_back(x);return true;}catch(...){return false;}}
bool add_total(ParseStage& p,AchTotal x) noexcept {try{p.totals.push_back(x);return true;}catch(...){return false;}}
void free_stage(ParseStage& p) noexcept {p.names.clear();p.totals.clear();p.store.achievements.clear();}
AchievementRecord* find_achievement(ConfigSnapshot& s,ServiceTextView id) noexcept {
    for(auto& a:s.achievements)if(a.id==std::string_view(id.data,id.size))return &a;
    return nullptr;
}
bool append_achievement(ConfigSnapshot& s,ServiceTextView id,ServiceTextView name,int32_t total) noexcept {
    if(!id.data||!id.size)return true;
    try {
        s.achievements.push_back({std::string(id.data,id.size),std::string(name.data,name.size),"","","","0000-00-00T00:00:00","0000-00-00T00:00:00",total,0,0});
        return true;
    }catch(...){return false;}
}
bool project_achievement(ParseStage& p,const char* source,const Tok* path,size_t depth,Tok key,Tok val) noexcept {
    const Cur c{source};
    if(depth>=2&&teq(c,path[0],"Achievements")&&teq(c,path[1],"AchievementNames"))return add_name(p,{tv(c,key),tv(c,val)});
    if(depth>=2&&teq(c,path[0],"Achievements")&&teq(c,path[1],"AchievementTotals")){int32_t x=0;if(!i32(tv(c,val),&x))return true;return add_total(p,{tv(c,key),x});}
    return true;
}

bool finalize(ParseStage& p) noexcept {
    for(size_t i=0;i<p.totals.size();++i){ServiceTextView name{"",0};for(size_t j=0;j<p.names.size();++j)if(same2(p.totals[i].id,p.names[j].id)){name=p.names[j].name;break;}if(!append_achievement(p.store,p.totals[i].id,name,p.totals[i].total)){free_stage(p);return false;}}
    for(size_t j=0;j<p.names.size();++j){if(find_achievement(p.store,p.names[j].id))continue;if(!append_achievement(p.store,p.names[j].id,p.names[j].name,1u)){free_stage(p);return false;}}
    return true;
}
} } }
