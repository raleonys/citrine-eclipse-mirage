// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "projection_support.hpp"

namespace redfield::content_projection {
namespace {
ContentRecord* ensure_content(std::vector<ContentRecord>& records,std::string_view id) noexcept {
    if(auto* record=config_projection::find_record(records,id,&ContentRecord::content_id))
        return record;
    try {
        records.push_back({std::string(id),"GAME","0","INSTALLED"});
        return &records.back();
    } catch(...) {return nullptr;}
}

bool project_content(ConfigSnapshot& snapshot,const char* source,
                     const config2_pegtl::Span* path,std::size_t depth,
                     config2_pegtl::Span key,config2_pegtl::Span value) noexcept {
    const config_projection::SourceView input{source};
    if(depth<2 || !input.is(path[0],"Content"))return true;
    auto* record=ensure_content(snapshot.content,input.text(path[1]));
    if(!record)return false;
    // A nested or unknown leaf still creates its record with defaults.
    if(depth!=2)return true;
    const auto text=input.text(value);
    // Content ignores empty field updates, including duplicate definitions.
    if(text.empty())return true;
    if(input.is(key,"Name"))return config_projection::replace_text(record->display_name,text);
    if(input.is(key,"Version"))return config_projection::replace_text(record->version,text);
    if(input.is(key,"State"))return config_projection::replace_text(record->state,text);
    return true;
}
}
}
