// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/config.hpp"
#include "preloader_l/config/settings_values.hpp"
#include <initializer_list>
namespace preloader_l {
// A synchronous borrowed view. The caller retains the immutable snapshot for
// the complete lookup/format operation; returned bytes share that lifetime.
// No parser, normalization, publication or process-global state lives here.
class ConfigSnapshotView final {
public:
    explicit ConfigSnapshotView(const ConfigSnapshot& snapshot) noexcept : snapshot_(snapshot) {}
    ConfigSnapshotView(ConfigSnapshot&&)=delete;
    ConfigSnapshotView(const ConfigSnapshot&&)=delete;
    config::ValueView find(std::string_view section,std::string_view key) const noexcept {
        return find_any(section,{key});
    }
    config::ValueView find_any(std::string_view section,
                              std::initializer_list<std::string_view> keys) const noexcept {
        for(auto leaf=snapshot_.leaves.rbegin();leaf!=snapshot_.leaves.rend();++leaf) {
            if(leaf->path.size()!=1 || leaf->path[0]!=section) continue;
            for(const auto key:keys)
                if(leaf->key==key) return {leaf->raw_value.data(),leaf->raw_value.size(),true};
        }
        return config::missing_value();
    }
    config::ValueView or_default(std::string_view section,std::string_view key,
                                 const char* fallback,bool* used_fallback=nullptr) const noexcept {
        return or_default(section,{key},fallback,used_fallback);
    }
    config::ValueView or_default(std::string_view section,
                                 std::initializer_list<std::string_view> keys,
                                 const char* fallback,bool* used_fallback=nullptr) const noexcept {
        const auto value=find_any(section,keys);
        if(used_fallback)*used_fallback=!value.present;
        return value.present?value:config::literal_value(fallback);
    }
private:
    const ConfigSnapshot& snapshot_;
};
}
