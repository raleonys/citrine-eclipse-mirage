// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace redfield {
// Raw quoted bytes retain the adopted grammar's escape representation.
// This is a syntax document, not complete HF19 Config2 behavioral parity.
struct ConfigLeaf { std::vector<std::string> path; std::string key; std::string raw_value; };
// Typed content projection is owned by the same publication as raw leaves/Online.
struct ContentRecord {std::string content_id,display_name,version,state;};
// Same raw-ID/default policy, with empty field replacement for entitlements.
struct EntitlementRecord {std::string item_id,group,version,type,entitlement_tag;};
// Initial achievement state; no mutable GrantAchievement owner is implemented.
struct AchievementRecord {std::string id,name,description,how_to,image_id,grant_date,expiration;std::int32_t total=1,count=0,progress=0;};
struct ConfigSnapshot {
    std::vector<AchievementRecord> achievements;
    std::vector<EntitlementRecord> entitlements;
    std::vector<ContentRecord> content;
    std::vector<ConfigLeaf> leaves;
    bool online = false;
    // Published syntax document, including an explicitly empty Config2 overlay.
    bool overlay_loaded = false;
    // Immutable session context, not a Config2 leaf; no CWD/global path lookup.
    std::string avatar_fallback_path;
};
enum class ConfigResult { published, invalid, resource, too_large };
class ConfigStore final {
public:
    static constexpr std::size_t max_bytes = 1024 * 1024;
    static constexpr std::size_t max_leaves = 4096;
    explicit ConfigStore(std::string avatar_fallback_path = {});
    ConfigResult replace(std::string_view source) noexcept;
    std::shared_ptr<const ConfigSnapshot> snapshot() const noexcept;
private:
    const std::string avatar_fallback_path_;
    std::atomic<std::shared_ptr<const ConfigSnapshot>> current_;
};
}
