// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "projection_support.hpp"

namespace preloader_l::achievement_projection {
namespace {
using Tok=config2_pegtl::Span;

bool i32(std::string_view text,int32_t* out) noexcept {
    if(!out || text.empty())return false;
    size_t i=0;
    bool negative=false;
    if(text[0]=='-' || text[0]=='+') {
        negative=text[0]=='-';
        if(++i==text.size())return false;
    }
    uint32_t value=0;
    const uint32_t limit=negative?UINT32_C(2147483648):UINT32_C(2147483647);
    for(;i<text.size();++i) {
        const char ch=text[i];
        if(ch<'0' || ch>'9')return false;
        const uint32_t digit=uint32_t(ch-'0');
        if(value>(limit-digit)/10u)return false;
        value=value*10u+digit;
    }
    *out=negative?(value==UINT32_C(2147483648)?INT32_MIN:-static_cast<int32_t>(value)):static_cast<int32_t>(value);
    return true;
}

// Tokens borrow the parse source until finalize; published records own strings.
struct AchName {std::string_view id,name;};
struct AchTotal {std::string_view id;int32_t total;};
struct ParseStage {
    ConfigSnapshot& store;
    std::vector<AchName> names;
    std::vector<AchTotal> totals;
};
bool add_name(ParseStage& stage,AchName value) noexcept {
    try {stage.names.push_back(value);return true;}catch(...) {return false;}
}
bool add_total(ParseStage& stage,AchTotal value) noexcept {
    try {stage.totals.push_back(value);return true;}catch(...) {return false;}
}
bool append_achievement(ConfigSnapshot& snapshot,std::string_view id,
                        std::string_view name,int32_t total) noexcept {
    if(id.empty())return true;
    try {
        snapshot.achievements.push_back({std::string(id),std::string(name),"","","",
            "0000-00-00T00:00:00","0000-00-00T00:00:00",total,0,0});
        return true;
    } catch(...) {return false;}
}
bool project_achievement(ParseStage& stage,const char* source,const Tok* path,
                         size_t depth,Tok key,Tok value) noexcept {
    const config_projection::SourceView input{source};
    if(depth<2 || !input.is(path[0],"Achievements"))return true;
    // Preserve the existing nested-leaf policy below names/totals sections.
    if(input.is(path[1],"AchievementNames"))
        return add_name(stage,{input.text(key),input.text(value)});
    if(input.is(path[1],"AchievementTotals")) {
        int32_t total=0;
        if(!i32(input.text(value),&total))return true;
        return add_total(stage,{input.text(key),total});
    }
    return true;
}

bool finalize(ParseStage& stage) noexcept {
    // Totals preserve duplicates and source order; the first matching name wins.
    for(const auto& total:stage.totals) {
        std::string_view name{"",0};
        for(const auto& candidate:stage.names)
            if(total.id==candidate.id) {name=candidate.name;break;}
        if(!append_achievement(stage.store,total.id,name,total.total))return false;
    }
    for(const auto& name:stage.names) {
        if(config_projection::find_record(stage.store.achievements,name.id,&AchievementRecord::id))continue;
        if(!append_achievement(stage.store,name.id,name.name,1))return false;
    }
    // On failure ConfigStore discards the unpublished candidate. Vector/string
    // owners unwind there; a second manual stage-clearing path is unnecessary.
    return true;
}
}
}
