// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "redfield/config.hpp"
#include "projection_support.hpp"

namespace redfield::entitlement_projection {
namespace {
EntitlementRecord* ensure_entitlement(std::vector<EntitlementRecord>& records,std::string_view id) noexcept {
    if(auto* record=config_projection::find_record(records,id,&EntitlementRecord::item_id))
        return record;
    try {
        records.push_back({std::string(id),"","0","DEFAULT","ONLINE_ACCESS"});
        return &records.back();
    } catch(...) {return nullptr;}
}

bool project_entitlement(ConfigSnapshot& snapshot,const char* source,
                         const config2_pegtl::Span* path,std::size_t depth,
                         config2_pegtl::Span key,config2_pegtl::Span value) noexcept {
    const config_projection::SourceView input{source};
    if(depth<2 || !input.is(path[0],"Entitlements"))return true;
    auto* record=ensure_entitlement(snapshot.entitlements,input.text(path[1]));
    if(!record)return false;
    if(depth!=2)return true;
    const auto text=input.text(value);
    // Entitlements replaces fields even when the new value is empty.
    if(input.is(key,"Group"))return config_projection::replace_text(record->group,text);
    if(input.is(key,"Version"))return config_projection::replace_text(record->version,text);
    if(input.is(key,"Type"))return config_projection::replace_text(record->type,text);
    if(input.is(key,"EntitlementTag"))return config_projection::replace_text(record->entitlement_tag,text);
    return true;
}
}
}
