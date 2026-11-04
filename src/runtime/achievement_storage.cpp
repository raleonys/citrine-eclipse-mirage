// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/runtime/achievement_storage.hpp"
#include "preloader_l/core/paths.hpp"
#include <bit>
#include <cctype>
#include <cstdio>
#include <stdexcept>

namespace preloader_l::achievement_storage {
namespace {
std::optional<std::string_view> attribute(const messages::XmlNode& node,std::string_view key) {
    const auto value=node.attribute(key);
    if(!value)return std::nullopt;
    return value->substr(0,value->find('\0')); // Original attributes are C strings.
}
// These Config2 strings become filename/directory components, not paths.
// Reject lexical escapes and alternate-stream syntax before any filesystem or
// achievement-state side effect. This is not a symlink/reparse-point guarantee.
void require_safe_achievement_leaf(std::string_view value) {
    if(value=="." || value==".." ||
       value.find_first_of("/\\:")!=std::string_view::npos ||
       value.find('\0')!=std::string_view::npos)
        throw std::invalid_argument("Achievement identifier is not one path component");
}
}
bool HostCrtNumbers::is_space(unsigned char c) {return std::isspace(c)!=0;}
std::optional<std::uint32_t> HostCrtNumbers::scan_decimal(std::string_view raw) {
    const std::string input(raw);int value=0;
    if(std::sscanf(input.c_str(),"%d",&value)!=1)return std::nullopt;
    static_assert(sizeof(int)==sizeof(std::uint32_t));
    return std::bit_cast<std::uint32_t>(value);
}
std::optional<std::uint32_t> HostCrtNumbers::scan_hex(std::string_view raw) {
    const std::string input(raw);unsigned value=0;
    if(std::sscanf(input.c_str(),"%x",&value)!=1)return std::nullopt;
    return value;
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
    if(!state.achievements_set.empty())require_safe_achievement_leaf(state.achievements_set);
    if(!state.steam_app_id.empty())require_safe_achievement_leaf(state.steam_app_id);
    if(!state.achievements_set.empty()) {
        const auto filename="achievement-"+state.achievements_set+".xml";
        const auto component=effects_.filesystem_widen_006d70(filename);
        paths_.xml_path=append(state.user_directory,component);
    }
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
        // Preserve the observed conversion order: filename, app id, directory.
        const auto filename=effects_.filesystem_widen_006d70("achievements.ini");
        const auto app=effects_.filesystem_widen_006d70(state.steam_app_id);
        const auto watcher=effects_.filesystem_widen_006d70("achievement_watcher");
        auto path=append(state.user_directory,watcher);
        path=append(std::move(path),app);
        paths_.ini_path=append(std::move(path),filename);
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
} // namespace preloader_l::achievement_storage
