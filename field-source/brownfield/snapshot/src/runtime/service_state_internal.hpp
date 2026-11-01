#pragma once

#include "preloader_l/runtime/service_state.hpp"
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {
namespace behavioral_internal {

struct OwnedText { char* data; size_t size; size_t bytes; };
struct ContentRecord { OwnedText content_id,display_name,version,state; };
struct EntitlementRecord {
    // Exact behavioral policy record: five std::string-like fields in x64 order
    // +0x00 ItemId, +0x20 Group, +0x40 Version, +0x60 Type, +0x80 EntitlementTag.
    OwnedText item_id,group,version,type,entitlement_tag;
};
struct AchievementRecord {
    OwnedText id,name,description,how_to,image_id,grant_date,expiration;
    int32_t total,count,progress;
};
struct Store {
    ContentRecord** content; size_t content_count,content_capacity;
    EntitlementRecord** entitlements; size_t entitlement_count,entitlement_capacity;
    AchievementRecord** achievements; size_t achievement_count,achievement_capacity;
};

void replace_active_store(Store* replacement,Store* retired) noexcept;
void clear_active_store() noexcept;
bool append_active_content(const ContentRecordInput& in) noexcept;
bool append_active_entitlement(const EntitlementRecordInput& in) noexcept;
AchievementRecord* find_active_achievement(ServiceTextView id) noexcept;
bool reserve_active_achievement_slot() noexcept;
void append_reserved_active_achievement(AchievementRecord* record) noexcept;
size_t active_content_count() noexcept;
bool active_content_at(size_t index,ContentRecordView* out) noexcept;
size_t active_entitlement_count() noexcept;
bool active_entitlement_at(size_t index,EntitlementRecordView* out) noexcept;
size_t active_achievement_count() noexcept;
bool active_achievement_at(size_t index,AchievementRecordView* out) noexcept;

void* alloc(size_t bytes) noexcept;
void dealloc(void* p,size_t bytes) noexcept;
void lock_state() noexcept;
void unlock_state() noexcept;
void lock_grant_delivery() noexcept;
void unlock_grant_delivery() noexcept;

struct Guard { Guard() noexcept { lock_state(); } ~Guard() noexcept { unlock_state(); } };
struct GrantDeliveryGuard { GrantDeliveryGuard() noexcept { lock_grant_delivery(); } ~GrantDeliveryGuard() noexcept { unlock_grant_delivery(); } };

size_t cstrlen(const char* s) noexcept;
bool same(ServiceTextView a,const char* b) noexcept;
bool same2(ServiceTextView a,ServiceTextView b) noexcept;
bool set_text(OwnedText& dst,ServiceTextView src) noexcept;
void free_text(OwnedText& v) noexcept;
ServiceTextView view(const OwnedText& v) noexcept;
bool grow_ptrs(void*** raw,size_t* cap,size_t used,size_t need) noexcept;
void free_content(ContentRecord* r) noexcept;
void free_entitlement(EntitlementRecord* r) noexcept;
void free_achievement(AchievementRecord* r) noexcept;
void clear_store(Store& s) noexcept;
bool append_content(Store& s,const ContentRecordInput& in) noexcept;
bool append_entitlement(Store& s,const EntitlementRecordInput& in) noexcept;
AchievementRecord* create_achievement(ServiceTextView id,ServiceTextView name,int32_t total) noexcept;
bool append_achievement(Store& s,ServiceTextView id,ServiceTextView name,int32_t total) noexcept;
ContentRecord* find_content(Store& s,ServiceTextView id) noexcept;
EntitlementRecord* find_entitlement(Store& s,ServiceTextView id) noexcept;
AchievementRecord* find_achievement(Store& s,ServiceTextView id) noexcept;
bool replace_text(OwnedText& dst,ServiceTextView src) noexcept;
bool build_cfg_store(const char* cfg,size_t len,Store* output) noexcept;
void export_achievement(const AchievementRecord& r,AchievementRecordView* out) noexcept;

} // namespace behavioral_internal

struct BehavioralServiceStateCandidate { behavioral_internal::Store store; };

} }
