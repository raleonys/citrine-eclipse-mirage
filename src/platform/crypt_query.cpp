// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/platform/crypt_query.hpp"
#include <array>
#include <limits>

namespace preloader_l::hooks {
PathProbe probe_path(const void* object,ReadPathUnit read,void* context) noexcept {
    PathProbe result;
    if(!object||!read)return result;
    const auto base=reinterpret_cast<std::uintptr_t>(object);
    constexpr auto maximum=std::numeric_limits<std::uintptr_t>::max();
    // Check the entire two-byte unit, not just its starting address.
    if(base>maximum-(sizeof(char16_t)-1))return result;
    const auto last_index=(maximum-base-(sizeof(char16_t)-1))/sizeof(char16_t);
    std::array<char16_t,target_suffix.size()> tail{};
    bool pending_high=false;
    for(std::size_t i=0;i<max_path_units;++i) {
        if(i>last_index)return result;
        char16_t unit{};
        if(!read(base+i*sizeof(char16_t),unit,context))return result;
        ++result.units_examined;
        if(!unit) {
            if(pending_high)return result;
            result.valid=true;
            if(i<tail.size())return result;
            for(std::size_t j=0;j<tail.size();++j)
                if(tail[(i-tail.size()+j)%tail.size()]!=target_suffix[j])return result;
            result.suffix_matches=true;return result;
        }
        const auto value=static_cast<std::uint16_t>(unit);
        if(pending_high) {
            if(value<0xdc00||value>0xdfff)return result;
            pending_high=false;
        } else if(value>=0xd800&&value<=0xdbff)pending_high=true;
        else if(value>=0xdc00&&value<=0xdfff)return result;
        tail[i%tail.size()]=unit;
    }
    return result; // No terminator within the bounded scan: forward original.
}
Selection select_object(std::uint32_t object_type,bool null_object,const PathProbe& path) noexcept {
    if(object_type!=1)return Selection::original;
    if(null_object)return Selection::companion;
    return path.valid&&path.suffix_matches?Selection::companion:Selection::original;
}
}
