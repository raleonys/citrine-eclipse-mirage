// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "config2_pegtl.hpp"
#include "redfield/runtime/record_views.hpp"
#include <cstring>
namespace redfield { namespace content_projection { namespace {
using ServiceTextView=redfield::runtime::ServiceTextView;
using ContentRecordInput=redfield::runtime::ContentRecordInput;
using Tok=redfield::config2_pegtl::Span;
struct Cur {const char* d;};
bool same(ServiceTextView a,const char* b) noexcept {return std::string_view(a.data,a.size)==b;}
struct ParseStage {std::vector<ContentRecord>& store;};
ContentRecord* find_content(std::vector<ContentRecord>& records,ServiceTextView id) noexcept {
    for(auto& r:records)if(r.content_id==std::string_view(id.data,id.size))return &r;
    return nullptr;
}
bool append_content(std::vector<ContentRecord>& records,const ContentRecordInput& input) noexcept {
    try {
        records.push_back({std::string(input.content_id.data,input.content_id.size),std::string(input.display_name.data,input.display_name.size),std::string(input.version.data,input.version.size),std::string(input.state.data,input.state.size)});
        return true;
    }catch(...){return false;}
}
bool replace_text(std::string& target,ServiceTextView value) noexcept {
    try {target.assign(value.data,value.size);return true;}catch(...){return false;}
}
ServiceTextView tv(const Cur& c,Tok t) noexcept{return{c.d+t.off,t.len};}
bool teq(const Cur& c,Tok t,const char* s) noexcept{return same(tv(c,t),s);}
bool ensure_content(ParseStage& p,ServiceTextView id,ContentRecord** out) noexcept {auto* r=find_content(p.store,id);if(!r){ContentRecordInput in{id,{"GAME",4},{"0",1},{"INSTALLED",9}};if(!append_content(p.store,in))return false;r=find_content(p.store,id);}*out=r;return true;}
bool project_content(ConfigSnapshot& snapshot,const char* source,const Tok* path,size_t depth,Tok key,Tok val) noexcept {
    ParseStage p{snapshot.content};const Cur c{source};
    if(depth>=2&&teq(c,path[0],"Content")){ContentRecord* r=nullptr;if(!ensure_content(p,tv(c,path[1]),&r))return false;if(depth!=2u)return true;const auto v=tv(c,val);if(teq(c,key,"Name"))return v.size?replace_text(r->display_name,v):true;if(teq(c,key,"Version"))return v.size?replace_text(r->version,v):true;if(teq(c,key,"State"))return v.size?replace_text(r->state,v):true;return true;}
    return true;
}
} } }
