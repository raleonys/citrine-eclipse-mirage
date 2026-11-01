// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "config2_pegtl.hpp"
#include "redfield/runtime/record_views.hpp"
namespace redfield { namespace entitlement_projection { namespace {
using ServiceTextView=redfield::runtime::ServiceTextView;
using EntitlementRecordInput=redfield::runtime::EntitlementRecordInput;
using Tok=redfield::config2_pegtl::Span;
struct Cur {const char* d;};
bool same(ServiceTextView a,const char* b) noexcept {return std::string_view(a.data,a.size)==b;}
struct ParseStage {std::vector<EntitlementRecord>& store;};
EntitlementRecord* find_entitlement(std::vector<EntitlementRecord>& records,ServiceTextView id) noexcept {
    for(auto& r:records)if(r.item_id==std::string_view(id.data,id.size))return &r;
    return nullptr;
}
bool append_entitlement(std::vector<EntitlementRecord>& records,const EntitlementRecordInput& input) noexcept {
    try {
        records.push_back({std::string(input.item_id.data,input.item_id.size),std::string(input.group.data,input.group.size),std::string(input.version.data,input.version.size),std::string(input.type.data,input.type.size),std::string(input.entitlement_tag.data,input.entitlement_tag.size)});
        return true;
    }catch(...){return false;}
}
bool replace_text(std::string& target,ServiceTextView value) noexcept {
    try {target.assign(value.data,value.size);return true;}catch(...){return false;}
}
ServiceTextView tv(const Cur& c,Tok t) noexcept{return{c.d+t.off,t.len};}
bool teq(const Cur& c,Tok t,const char* s) noexcept{return same(tv(c,t),s);}
bool ensure_entitlement(ParseStage& p,ServiceTextView id,EntitlementRecord** out) noexcept {auto* r=find_entitlement(p.store,id);if(!r){EntitlementRecordInput in{id,{"",0},{"0",1},{"DEFAULT",7},{"ONLINE_ACCESS",13}};if(!append_entitlement(p.store,in))return false;r=find_entitlement(p.store,id);}*out=r;return true;}
bool project_entitlement(ConfigSnapshot& snapshot,const char* source,const Tok* path,size_t depth,Tok key,Tok val) noexcept {
    ParseStage p{snapshot.entitlements};const Cur c{source};
    if(depth>=2&&teq(c,path[0],"Entitlements")){EntitlementRecord* r=nullptr;if(!ensure_entitlement(p,tv(c,path[1]),&r))return false;if(depth!=2u)return true;const auto v=tv(c,val);if(teq(c,key,"Group"))return replace_text(r->group,v);if(teq(c,key,"Version"))return replace_text(r->version,v);if(teq(c,key,"Type"))return replace_text(r->type,v);if(teq(c,key,"EntitlementTag"))return replace_text(r->entitlement_tag,v);return true;}
    return true;
}
} } }
