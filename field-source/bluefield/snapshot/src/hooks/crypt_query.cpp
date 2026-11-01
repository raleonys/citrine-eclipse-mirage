// SPDX-License-Identifier: GPL-3.0-only
#include "src/hooks/crypt_query.hpp"

namespace preloader_l::crypt_query {
Selection select_object(std::uint32_t object_type,bool null_object,
                        std::u16string_view path,bool readable) noexcept {
    if(object_type!=1)return Selection::original;
    if(null_object)return Selection::companion;
    if(!readable)return Selection::original;
    path=path.substr(0,path.find(u'\0'));
    for(std::size_t i=0;i<path.size();++i){
        const auto unit=static_cast<std::uint16_t>(path[i]);
        if(unit>=0xd800&&unit<=0xdbff){
            if(i+1==path.size())return Selection::original;
            const auto next=static_cast<std::uint16_t>(path[++i]);
            if(next<0xdc00||next>0xdfff)return Selection::original;
        }else if(unit>=0xdc00&&unit<=0xdfff)return Selection::original;
    }
    // For valid UTF-16 this ASCII suffix is equivalent to the source's final
    // UTF-8 byte comparison. Preserve case sensitivity and lack of separator test.
    return path.ends_with(u"preloader_l.dll")?Selection::companion:Selection::original;
}
}
