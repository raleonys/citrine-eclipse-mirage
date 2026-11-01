#pragma once

#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

struct ServiceTextView { const char* data; size_t size; };

struct ContentRecordInput {
    ServiceTextView content_id;
    ServiceTextView display_name;
    ServiceTextView version;
    ServiceTextView state;
};
struct ContentRecordView {
    ServiceTextView content_id;
    ServiceTextView display_name;
    ServiceTextView version;
    ServiceTextView state;
};

struct EntitlementRecordInput {
    ServiceTextView item_id;
    ServiceTextView group;
    ServiceTextView version;
    ServiceTextView type;
    ServiceTextView entitlement_tag;
};
struct EntitlementRecordView {
    ServiceTextView item_id;
    ServiceTextView group;
    ServiceTextView version;
    ServiceTextView type;
    ServiceTextView entitlement_tag;
};

struct AchievementRecordView {
    ServiceTextView id;
    ServiceTextView name;
    ServiceTextView description;
    ServiceTextView how_to;
    ServiceTextView image_id;
    ServiceTextView grant_date;
    ServiceTextView expiration;
    int32_t total;
    int32_t count;
    int32_t progress;
};

// Service-owned stores are initialized empty, independently of Config2. The
// startup Config2 document is transactionally projected into these typed
// records when present; raw configuration fragments are never serialized as
// route response policy. The registration APIs remain the clean boundary
// for additional service/store producers.
struct BehavioralServiceStateCandidate;
bool prepare_behavioral_service_state_cfg(const char* cfg,size_t length,BehavioralServiceStateCandidate** candidate) noexcept;
void commit_behavioral_service_state_candidate(BehavioralServiceStateCandidate* candidate) noexcept;
void release_behavioral_service_state_candidate(BehavioralServiceStateCandidate* candidate) noexcept;
void clear_behavioral_service_state() noexcept;

bool register_content_record(const ContentRecordInput& input) noexcept;
bool register_entitlement_record(const EntitlementRecordInput& input) noexcept;

// Route serializers hold the service-state lock for their complete traversal
// so returned text views cannot be invalidated by a concurrent Config2 commit.
void acquire_behavioral_service_state() noexcept;
void release_behavioral_service_state() noexcept;

// GrantAchievement response-delivery reservation. The shipping socket path
// keeps a prepared mutation uncommitted until the encoded response has been
// delivered. The reservation prevents another GrantAchievement mutation or a
// store replacement/clear from invalidating that prepared transaction. W7 also
// makes GrantAchievement measure previews and QueryAchievements reads join this
// same gate so no achievement measure/render pair can straddle a post-send
// GrantAchievement commit.
void acquire_grant_achievement_delivery() noexcept;
void release_grant_achievement_delivery() noexcept;
size_t content_record_count_locked() noexcept;
bool content_record_at_locked(size_t index,ContentRecordView* output) noexcept;
size_t entitlement_record_count_locked() noexcept;
bool entitlement_record_at_locked(size_t index,EntitlementRecordView* output) noexcept;

// GrantAchievement persistent domain state. Caller must hold the service-state
// lock through mutation and response serialization.
bool preview_grant_achievement_locked(ServiceTextView id,int32_t increment,
                                      AchievementRecordView* output,bool* mutated_now,
                                      bool* completed_now) noexcept;
struct GrantAchievementTransaction;
// Prepare a GrantAchievement mutation while the caller holds the service-state
// lock. All allocations occur here. The returned projected record is stable
// until commit/release. commit_grant_achievement_locked() is no-fail.
bool prepare_grant_achievement_locked(ServiceTextView id,int32_t increment,
                                      ServiceTextView grant_timestamp,
                                      GrantAchievementTransaction** transaction,
                                      AchievementRecordView* output,bool* mutated_now,
                                      bool* completed_now,bool* inserted_now) noexcept;
void commit_grant_achievement_locked(GrantAchievementTransaction* transaction) noexcept;
void release_grant_achievement_transaction(GrantAchievementTransaction* transaction) noexcept;

// Compatibility helper for existing direct callers. New transactional route
// code should use prepare/commit so failures cannot leave partial mutation.
bool grant_achievement_locked(ServiceTextView id,int32_t increment,
                              ServiceTextView grant_timestamp,
                              AchievementRecordView* output,bool* mutated_now,
                              bool* completed_now) noexcept;
size_t achievement_record_count_locked() noexcept;
bool achievement_record_at_locked(size_t index,AchievementRecordView* output) noexcept;

void clear_behavioral_service_state_for_test() noexcept;

} }
