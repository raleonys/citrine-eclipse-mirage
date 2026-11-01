// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include <stddef.h>
#include <stdint.h>

namespace redfield { namespace runtime {

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

} }
