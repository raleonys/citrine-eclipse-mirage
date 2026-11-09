// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/achievement_storage.hpp"
#include "citrine_eclipse_mirage/core/paths.hpp"
#include <bit>
#include <cerrno>
#include <cctype>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <string>

namespace citrine_eclipse_mirage::achievement_storage {
namespace {
std::optional<std::string_view> attribute(const messages::XmlNode& node,std::string_view key) {
    const auto value=node.attribute(key);
    if(!value)return std::nullopt;
    return value->substr(0,value->find('\0')); // Original attributes are C strings.
}
// These Config2 strings become filename/directory components, not paths.
// Reject lexical escapes and alternate-stream syntax before any filesystem or
// achievement-state side effect. This is not a symlink/reparse-point guarantee.
void require_safe_achievement_leaf(std::string_view value,bool watcher_directory) {
    if(value=="." || value==".." ||
       value.find_first_of("/\\:<>\"|?*")!=std::string_view::npos)
        throw std::invalid_argument("Achievement identifier is not one Windows path component");
    for(const unsigned char byte:value)
        if(byte<0x20)
            throw std::invalid_argument("Achievement identifier contains an invalid Windows filename byte");
    if(!watcher_directory||value.empty())return;
    // SteamAppId is a bare directory leaf; unlike the achievement XML file it
    // has no prefix/suffix to disambiguate Win32 device names.
    if(value.front()==' '||value.back()==' '||value.back()=='.')
        throw std::invalid_argument("Achievement watcher directory has an ambiguous Win32 name");
    auto stem=value.substr(0,value.find('.'));
    while(!stem.empty()&&stem.back()==' ')stem.remove_suffix(1);
    std::string upper;upper.reserve(stem.size());
    for(const char byte:stem)
        upper.push_back(byte>='a'&&byte<='z'?static_cast<char>(byte-'a'+'A'):byte);
    if(upper=="CON"||upper=="PRN"||upper=="AUX"||upper=="NUL"||
       (upper.size()==4&&(upper.starts_with("COM")||upper.starts_with("LPT"))&&
        upper[3]>='1'&&upper[3]<='9'))
        throw std::invalid_argument("Achievement watcher directory uses a reserved Win32 device name");
}
// The same narrow bytes can denote a device digit or an ordinary character
// under different filesystem code pages. Judge the converted directory too.
void require_safe_watcher_directory(std::u16string_view value) {
    if(value.empty()||value==u"."||value==u".."||
       value.find_first_of(u"/\\:<>\"|?*")!=std::u16string_view::npos||
       value.front()==u' '||value.back()==u' '||value.back()==u'.')
        throw std::invalid_argument("Achievement watcher directory has an invalid Win32 name");
    for(const char16_t c:value)if(c<0x20)
        throw std::invalid_argument("Achievement watcher directory contains an invalid Windows filename character");
    auto stem=value.substr(0,value.find(u'.'));
    while(!stem.empty()&&stem.back()==u' ')stem.remove_suffix(1);
    if(stem.size()!=3&&stem.size()!=4)return;
    std::u16string upper;upper.reserve(stem.size());
    for(const char16_t c:stem)
        upper.push_back(c>=u'a'&&c<=u'z'?static_cast<char16_t>(c-u'a'+u'A'):c);
    if(upper==u"CON"||upper==u"PRN"||upper==u"AUX"||upper==u"NUL")
        throw std::invalid_argument("Achievement watcher directory uses a reserved Win32 device name");
    if(upper.size()==4&&(upper.starts_with(u"COM")||upper.starts_with(u"LPT"))) {
        const auto digit=upper[3];
        if((digit>=u'1'&&digit<=u'9')||digit==u'\u00b9'||digit==u'\u00b2'||digit==u'\u00b3')
            throw std::invalid_argument("Achievement watcher directory uses a reserved Win32 device name");
    }
}
}
bool HostCrtNumbers::is_space(unsigned char c) {return std::isspace(c)!=0;}
std::optional<std::uint32_t> HostCrtNumbers::scan_decimal(std::string_view raw) {
    // sscanf("%d") with an out-of-range integer has undefined behavior.
    // This host adapter preserves the supported decimal prefix and low32
    // signed representation, but refuses values outside int32_t's domain.
    const std::string input(raw);
    char* end=nullptr;
    errno=0;
    const auto number=std::strtoll(input.c_str(),&end,10);
    if(end==input.c_str()||errno==ERANGE||
       number<std::numeric_limits<std::int32_t>::min()||
       number>std::numeric_limits<std::int32_t>::max())return std::nullopt;
    return std::bit_cast<std::uint32_t>(static_cast<std::int32_t>(number));
}
std::optional<std::uint32_t> HostCrtNumbers::scan_hex(std::string_view raw) {
    // The old sscanf("%x") likewise cannot safely store a value above
    // UINT32_MAX. Keep its optional sign, 0x prefix and trailing-suffix
    // behavior for values in range, including negative modulo-2^32 inputs.
    const std::string input(raw);
    const char* begin=input.c_str();
    while(std::isspace(static_cast<unsigned char>(*begin)))++begin;
    const bool negative=*begin=='-';
    if(negative||*begin=='+') {
        ++begin;
        if(*begin=='+'||*begin=='-'||std::isspace(static_cast<unsigned char>(*begin)))return std::nullopt;
    }
    char* end=nullptr;
    errno=0;
    const auto number=std::strtoull(begin,&end,16);
    if(end==begin||errno==ERANGE||
       number>std::numeric_limits<std::uint32_t>::max())return std::nullopt;
    const auto bits=static_cast<std::uint32_t>(number);
    return negative?std::uint32_t{0}-bits:bits;
}
std::uint32_t attribute_003b70(const messages::XmlNode& node,std::string_view key,
                             std::uint32_t fallback,Numbers& numbers) {
    const auto value=attribute(node,key);
    if(!value)return fallback;
    std::size_t cursor=0;
    while(cursor<value->size()) {
        const auto c=static_cast<unsigned char>((*value)[cursor]);
        if(c>=128 || !numbers.is_space(c))break;
        ++cursor;
    }
    const bool hex=cursor+1<value->size() && (*value)[cursor]=='0' &&
                   ((*value)[cursor+1]=='x' || (*value)[cursor+1]=='X');
    // 006190 detects prefix after whitespace but scans the ORIGINAL string.
    const auto parsed=hex?numbers.scan_hex(*value):numbers.scan_decimal(*value);
    return parsed.value_or(fallback);
}
namespace {
configuration::Achievement empty_achievement() {
    return {1,0,0,"0000-00-00T00:00:00",{},{},0};
}
configuration::Path append(configuration::Path base,const configuration::Path& component) {
    path::append_016b50(base,component);return base;
}
}
void Storage::update_01eb00(configuration::State& state) {
    // Validate both input components up front: rejecting either must not read
    // an XML file, update a retained path, or write a watcher INI elsewhere.
    if(!state.achievements_set.empty())require_safe_achievement_leaf(state.achievements_set,false);
    if(!state.steam_app_id.empty())require_safe_achievement_leaf(state.steam_app_id,true);
    // Preflight conversions before file IO/publication. Keep the established
    // relative conversion order: XML filename, INI filename, app id, watcher.
    // This deliberately rejects conversion errors and decoded device aliases
    // before importing XML, rather than leaving partially updated state.
    configuration::Path xml_component,ini_filename,app_directory,watcher_directory;
    if(!state.achievements_set.empty()) {
        const auto filename="achievement-"+state.achievements_set+".xml";
        xml_component=effects_.filesystem_widen_006d70(filename);
    }
    if(!state.steam_app_id.empty()) {
        ini_filename=effects_.filesystem_widen_006d70("achievements.ini");
        app_directory=effects_.filesystem_widen_006d70(state.steam_app_id);
        require_safe_watcher_directory(app_directory);
        watcher_directory=effects_.filesystem_widen_006d70("achievement_watcher");
    }
    if(!state.achievements_set.empty())paths_.xml_path=append(state.user_directory,xml_component);
    // Empty set preserves a previously supplied XML path; it does not disable it.
    if(!paths_.xml_path.empty()) {
        const auto bytes=file::read_binary_03b3f0(paths_.xml_path,files_);
        auto parsed=xml_.parse_c_string(bytes);
        if(!parsed.document)throw std::invalid_argument("Achievement XML backend returned no document");
        if(!parsed.has_error) {
            const auto* root=parsed.document->first_element();
            for(const auto* node=root?root->first_element():nullptr;node;node=node->next_element()) {
                const auto id=attribute(*node,"Id");
                if(!id)continue; // Present empty ID is deliberately a valid key.
                auto found=state.achievements.find(*id);
                if(found==state.achievements.end())found=state.achievements.emplace(std::string(*id),empty_achievement()).first;
                auto& achievement=found->second;
                achievement.field04=std::bit_cast<std::int32_t>(attribute_003b70(*node,"Count",0,numbers_));
                achievement.field08=std::bit_cast<std::int32_t>(attribute_003b70(*node,"Progress",0,numbers_));
                if(const auto grant=attribute(*node,"Grant"))achievement.timestamp.assign(*grant);
                if(achievement.display_name.empty())
                    if(const auto name=attribute(*node,"Name"))achievement.display_name.assign(*name);
            }
        }
    }
    if(!state.steam_app_id.empty()) {
        auto path=append(state.user_directory,watcher_directory);
        path=append(std::move(path),app_directory);
        paths_.ini_path=append(std::move(path),ini_filename);
        for(auto& [id,achievement]:state.achievements) {
            (void)id;
            achievement.field70=static_cast<std::uint64_t>(effects_.timestamp_to_epoch_041e10(achievement.timestamp));
        }
        const auto status=effects_.status_01ea20(paths_.ini_path,3);
        if(status.error!=0 && ((static_cast<std::uint32_t>(status.kind)-1U)&0xfffffff7U)!=0)
            effects_.filesystem_error_018130("status",status.error,paths_.ini_path);
        if(status.kind!=2)achievement_writer::write_ini_01f9a0(paths_.ini_path,state,effects_,files_);
    }
}
} // namespace citrine_eclipse_mirage::achievement_storage
