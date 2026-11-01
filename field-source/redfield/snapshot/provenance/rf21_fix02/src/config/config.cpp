// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/config.hpp"
#include "config2_pegtl.hpp"
#include "content_projection.hpp"
#include "entitlement_projection.hpp"
#include "achievement_projection.hpp"

namespace redfield {
namespace {
using Span = preloader_l::config2_pegtl::Span;
struct Context { std::string_view source; ConfigSnapshot* staged; achievement_projection::ParseStage* achievements; };
bool leaf(void* user, const Span* path, std::size_t depth, Span key, Span value) noexcept {
    auto& context = *static_cast<Context*>(user);
    try {
        auto& staged = *context.staged;
        if (staged.leaves.size() >= ConfigStore::max_leaves) return false;
        ConfigLeaf entry;
        for (std::size_t i = 0; i < depth; ++i)
            entry.path.emplace_back(context.source.substr(path[i].off, path[i].len));
        entry.key = context.source.substr(key.off, key.len);
        entry.raw_value = context.source.substr(value.off, value.len);
        if (entry.path.size() == 1 && entry.path[0] == "Emulator" && entry.key == "Online") {
            if (entry.raw_value == "true") staged.online = true;
            else if (entry.raw_value == "false") staged.online = false;
            // Numeric/other values have no policy effect. Exact literals only.
        }
        staged.leaves.push_back(std::move(entry));
        return content_projection::project_content(staged,context.source.data(),path,depth,key,value)
            && entitlement_projection::project_entitlement(staged,context.source.data(),path,depth,key,value)
            && achievement_projection::project_achievement(*context.achievements,context.source.data(),path,depth,key,value);
    } catch (...) { return false; }
}
}
ConfigStore::ConfigStore(std::string avatar_fallback_path)
    : avatar_fallback_path_(std::move(avatar_fallback_path)) {
    auto initial=std::make_shared<ConfigSnapshot>();
    initial->avatar_fallback_path=avatar_fallback_path_;
    current_.store(std::move(initial),std::memory_order_release);
}
std::shared_ptr<const ConfigSnapshot> ConfigStore::snapshot() const noexcept {
    return current_.load(std::memory_order_acquire);
}
ConfigResult ConfigStore::replace(std::string_view source) noexcept {
    if (source.size() > max_bytes) return ConfigResult::too_large;
    if (source.empty() || source.find('\0') != std::string_view::npos) return ConfigResult::invalid;
    try {
        auto staged = std::make_shared<ConfigSnapshot>();
        staged->avatar_fallback_path=avatar_fallback_path_;
        achievement_projection::ParseStage achievements{*staged,{},0,{},0};
        Context context{source, staged.get(), &achievements};
        const preloader_l::config2_pegtl::Visitor visitor{&context, nullptr, &leaf};
        const auto result = preloader_l::config2_pegtl::parse(source.data(), source.size(), visitor, false);
        if (result != preloader_l::config2_pegtl::Result::ok)
            return result == preloader_l::config2_pegtl::Result::resource ? ConfigResult::resource : ConfigResult::invalid;
        if(!achievement_projection::finalize(achievements))return ConfigResult::resource;
        staged->overlay_loaded = true;
        current_.store(std::move(staged), std::memory_order_release);
        return ConfigResult::published;
    } catch (...) { return ConfigResult::resource; }
}
}
