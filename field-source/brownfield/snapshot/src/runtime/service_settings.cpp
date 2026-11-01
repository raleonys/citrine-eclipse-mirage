#include "service_policy_internal.hpp"

namespace preloader_l { namespace runtime { namespace model_release_service_policy_internal {
namespace {
constexpr uint64_t kModelInitialBuckets = 8u;
constexpr uint64_t kModelInitialMask = 7u;
constexpr uint64_t kModelFastGrowthThreshold = 0x200u;
constexpr uint64_t kModelMaxNodes = UINT64_C(0x0333333333333333);
constexpr uint64_t kFnvOffsetBasis = UINT64_C(0xcbf29ce484222325);
constexpr uint64_t kFnvPrime = UINT64_C(0x100000001b3);
}

uint64_t model_fnv1a(const char* data, size_t size) noexcept {
    uint64_t h = kFnvOffsetBasis;
    for (size_t i = 0; i < size; ++i) {
        h ^= static_cast<uint8_t>(data[i]);
        h *= kFnvPrime;
    }
    return h;
}

using SettingsMap = model_abi::ModelSettingsUnorderedMap40;
using SettingsNode = model_abi::ModelSettingsNode50;

static_assert(sizeof(float) == 4u, "model max_load_factor geometry requires 32-bit float");

void write_model_float_one(float* value) noexcept {
    if (!value) return;
    // Preserve the curio IEEE-754 1.0f object representation without
    // emitting a floating-point operation. Normalized 3.1.8 shipping uses /MD + /GS,
    // but this curio ABI field still does not need an FP operation merely to
    // materialize its exact object representation. Windows x64 is little-endian.
    auto* bytes = reinterpret_cast<uint8_t*>(value);
    bytes[0] = 0x00u;
    bytes[1] = 0x00u;
    bytes[2] = 0x80u;
    bytes[3] = 0x3fu;
}

SettingsNode* node_from(uintptr_t p) noexcept {
    return reinterpret_cast<SettingsNode*>(p);
}
uintptr_t* bucket_data(SettingsMap* map) noexcept {
    return map ? reinterpret_cast<uintptr_t*>(map->bucket_pairs.begin) : nullptr;
}

bool bucket_allocation_size(uint64_t bucket_count, size_t* out) noexcept {
    if (!out || bucket_count > static_cast<uint64_t>(static_cast<size_t>(-1) / (2u * sizeof(uintptr_t))))
        return false;
    *out = static_cast<size_t>(bucket_count) * 2u * sizeof(uintptr_t);
    return true;
}

bool initialize_settings_map(SettingsMap* map) noexcept {
    if (!map) return false;
    *map = SettingsMap{};
    write_model_float_one(&map->max_load_factor);
    map->mask = kModelInitialMask;
    map->bucket_count = kModelInitialBuckets;

    auto* sentinel = static_cast<SettingsNode*>(allocate_bytes(sizeof(SettingsNode)));
    if (!sentinel) { *map = SettingsMap{}; return false; }
    zero_bytes(sentinel, sizeof(*sentinel));
    sentinel->next = reinterpret_cast<uintptr_t>(sentinel);
    sentinel->prev = reinterpret_cast<uintptr_t>(sentinel);

    size_t bucket_bytes = 0u;
    if (!bucket_allocation_size(kModelInitialBuckets, &bucket_bytes)) {
        (void)release_bytes(sentinel, sizeof(*sentinel));
        *map = SettingsMap{};
        return false;
    }
    auto* buckets = static_cast<uintptr_t*>(allocate_bytes(bucket_bytes));
    if (!buckets) {
        (void)release_bytes(sentinel, sizeof(*sentinel));
        *map = SettingsMap{};
        return false;
    }
    const uintptr_t s = reinterpret_cast<uintptr_t>(sentinel);
    for (size_t i = 0; i < static_cast<size_t>(kModelInitialBuckets * 2u); ++i)
        buckets[i] = s;

    map->sentinel = s;
    map->size = 0u;
    map->bucket_pairs.begin = reinterpret_cast<uintptr_t>(buckets);
    map->bucket_pairs.end = reinterpret_cast<uintptr_t>(buckets + kModelInitialBuckets * 2u);
    map->bucket_pairs.capacity = map->bucket_pairs.end;
    return true;
}

void fill_empty_buckets(SettingsMap* map) noexcept {
    if (!map || !map->sentinel) return;
    auto* buckets = bucket_data(map);
    if (!buckets) return;
    const size_t slots = static_cast<size_t>(map->bucket_count) * 2u;
    for (size_t i = 0; i < slots; ++i) buckets[i] = map->sentinel;
}

bool clear_settings_map(SettingsMap* map) noexcept {
    if (!map || !map->sentinel) return true;
    auto* sentinel = node_from(map->sentinel);
    uintptr_t current = sentinel->next;
    uint64_t remaining = map->size;
    bool all = true;
    while (current && current != map->sentinel && remaining) {
        auto* node = node_from(current);
        const uintptr_t previous = node->prev;
        const uintptr_t next = node->next;
        const bool value_ok = release_string(&node->value);
        const bool key_ok = release_string(&node->key);
        if (value_ok && key_ok && release_bytes(node, sizeof(SettingsNode))) {
            node_from(previous)->next = next;
            node_from(next)->prev = previous;
            if (map->size) --map->size;
        } else {
            all = false;
        }
        current = next;
        --remaining;
    }
    if (map->size == 0u) {
        sentinel->next = map->sentinel;
        sentinel->prev = map->sentinel;
        fill_empty_buckets(map);
    }
    return all && map->size == 0u;
}

bool destroy_settings_map(SettingsMap* map) noexcept {
    if (!map) return true;
    if (!clear_settings_map(map)) return false;
    size_t bucket_bytes = 0u;
    if (map->bucket_pairs.begin) {
        if (!bucket_allocation_size(map->bucket_count, &bucket_bytes) ||
            !release_bytes(reinterpret_cast<void*>(map->bucket_pairs.begin), bucket_bytes))
            return false;
        map->bucket_pairs = model_abi::MsvcVector24{};
    }
    if (map->sentinel) {
        if (!release_bytes(reinterpret_cast<void*>(map->sentinel), sizeof(SettingsNode)))
            return false;
        map->sentinel = 0u;
    }
    *map = SettingsMap{};
    return true;
}

SettingsNode* find_settings_node(SettingsMap* map,
                                 const char* key, size_t key_size) noexcept {
    if (!map || !map->sentinel || !map->bucket_pairs.begin || !map->bucket_count ||
        (!key && key_size)) return nullptr;
    const uint64_t bucket = model_fnv1a(key, key_size) & map->mask;
    const auto* buckets = bucket_data(map);
    uintptr_t first = buckets[static_cast<size_t>(bucket * 2u)];
    uintptr_t current = buckets[static_cast<size_t>(bucket * 2u + 1u)];
    if (first == map->sentinel || current == map->sentinel) return nullptr;
    for (;;) {
        auto* node = node_from(current);
        if (string_equal(node->key, key, key_size)) return node;
        if (current == first) break;
        current = node->prev;
        if (!current || current == map->sentinel) break;
    }
    return nullptr;
}

const SettingsNode* find_settings_node(const SettingsMap* map,
                                       const char* key, size_t key_size) noexcept {
    return find_settings_node(const_cast<SettingsMap*>(map), key, key_size);
}

bool next_power_of_two_at_least(uint64_t value, uint64_t* out) noexcept {
    if (!out) return false;
    uint64_t n = kModelInitialBuckets;
    while (n < value) {
        if (n > (UINT64_MAX >> 1u)) return false;
        n <<= 1u;
    }
    *out = n;
    return true;
}

bool rehash_settings_map(SettingsMap* map, uint64_t requested_buckets) noexcept {
    if (!map || !map->sentinel || !map->bucket_pairs.begin) return false;
    uint64_t bucket_count = 0u;
    if (!next_power_of_two_at_least(requested_buckets, &bucket_count)) return false;
    if (bucket_count == map->bucket_count) return true;

    size_t bucket_bytes = 0u;
    if (!bucket_allocation_size(bucket_count, &bucket_bytes)) return false;
    auto* replacement = static_cast<uintptr_t*>(allocate_bytes(bucket_bytes));
    if (!replacement) return false;
    for (size_t i = 0; i < static_cast<size_t>(bucket_count * 2u); ++i)
        replacement[i] = map->sentinel;

    auto* sentinel = node_from(map->sentinel);
    uintptr_t current = sentinel->next;
    uint64_t remaining = map->size;
    const uint64_t new_mask = bucket_count - 1u;

    // First form one local doubly-linked chain per bucket while preserving the
    // old traversal's order within each bucket.
    while (current && current != map->sentinel && remaining) {
        auto* node = node_from(current);
        const uintptr_t old_next = node->next;
        const char* key = string_data(node->key);
        const uint64_t bucket = model_fnv1a(key, static_cast<size_t>(node->key.size)) & new_mask;
        const size_t base = static_cast<size_t>(bucket * 2u);
        if (replacement[base] == map->sentinel) {
            replacement[base] = current;
            replacement[base + 1u] = current;
            node->prev = 0u;
            node->next = 0u;
        } else {
            auto* tail = node_from(replacement[base + 1u]);
            tail->next = current;
            node->prev = reinterpret_cast<uintptr_t>(tail);
            node->next = 0u;
            replacement[base + 1u] = current;
        }
        current = old_next;
        --remaining;
    }

    // Concatenate bucket chains into the model list/sentinel topology.
    uintptr_t tail_ptr = map->sentinel;
    sentinel->next = map->sentinel;
    sentinel->prev = map->sentinel;
    for (uint64_t bucket = 0; bucket < bucket_count; ++bucket) {
        const size_t base = static_cast<size_t>(bucket * 2u);
        const uintptr_t first = replacement[base];
        const uintptr_t last = replacement[base + 1u];
        if (first == map->sentinel) continue;
        auto* tail = node_from(tail_ptr);
        auto* first_node = node_from(first);
        tail->next = first;
        first_node->prev = tail_ptr;
        tail_ptr = last;
    }
    auto* final_tail = node_from(tail_ptr);
    final_tail->next = map->sentinel;
    sentinel->prev = tail_ptr;

    size_t old_bucket_bytes = 0u;
    if (bucket_allocation_size(map->bucket_count, &old_bucket_bytes))
        (void)release_bytes(reinterpret_cast<void*>(map->bucket_pairs.begin), old_bucket_bytes);
    map->bucket_pairs.begin = reinterpret_cast<uintptr_t>(replacement);
    map->bucket_pairs.end = reinterpret_cast<uintptr_t>(replacement + bucket_count * 2u);
    map->bucket_pairs.capacity = map->bucket_pairs.end;
    map->mask = new_mask;
    map->bucket_count = bucket_count;
    return true;
}

bool ensure_settings_capacity_for_insert(SettingsMap* map) noexcept {
    if (!map || map->size >= kModelMaxNodes) return false;
    const uint64_t required = map->size + 1u;
    if (required <= map->bucket_count) return true; // max_load_factor is recovered as 1.0f
    uint64_t requested = required;
    if (map->bucket_count < kModelFastGrowthThreshold) {
        if (map->bucket_count > UINT64_MAX / 8u) return false;
        const uint64_t fast = map->bucket_count * 8u;
        if (fast > requested) requested = fast;
    }
    return rehash_settings_map(map, requested);
}

bool insert_or_assign_setting(SettingsMap* map,
                              const ModelSettingInput& input) noexcept {
    if (!map || (!input.key && input.key_size) || (!input.value && input.value_size)) return false;
    if (auto* existing = find_settings_node(map, input.key, input.key_size))
        return assign_string(&existing->value, input.value, input.value_size);

    auto* node = static_cast<SettingsNode*>(allocate_bytes(sizeof(SettingsNode)));
    if (!node) return false;
    zero_bytes(node, sizeof(*node));
    initialize_empty_string(&node->key);
    initialize_empty_string(&node->value);
    if (!assign_string(&node->key, input.key, input.key_size) ||
        !assign_string(&node->value, input.value, input.value_size)) {
        (void)release_string(&node->value);
        (void)release_string(&node->key);
        (void)release_bytes(node, sizeof(SettingsNode));
        return false;
    }

    if (!ensure_settings_capacity_for_insert(map)) {
        (void)release_string(&node->value);
        (void)release_string(&node->key);
        (void)release_bytes(node, sizeof(SettingsNode));
        return false;
    }

    const uint64_t bucket = model_fnv1a(input.key, input.key_size) & map->mask;
    auto* buckets = bucket_data(map);
    const size_t base = static_cast<size_t>(bucket * 2u);
    const uintptr_t sentinel = map->sentinel;
    const uintptr_t node_ptr = reinterpret_cast<uintptr_t>(node);
    if (buckets[base] == sentinel) {
        // Empty bucket: add as one new contiguous bucket group at list tail.
        auto* s = node_from(sentinel);
        const uintptr_t previous = s->prev;
        auto* previous_node = node_from(previous);
        node->prev = previous;
        node->next = sentinel;
        previous_node->next = node_ptr;
        s->prev = node_ptr;
        buckets[base] = node_ptr;
        buckets[base + 1u] = node_ptr;
    } else {
        // Existing bucket: append immediately after its current last node so
        // every bucket remains one contiguous range, matching MSVC lookup.
        const uintptr_t previous = buckets[base + 1u];
        auto* previous_node = node_from(previous);
        const uintptr_t next = previous_node->next;
        auto* next_node = node_from(next);
        node->prev = previous;
        node->next = next;
        previous_node->next = node_ptr;
        next_node->prev = node_ptr;
        buckets[base + 1u] = node_ptr;
    }
    ++map->size;
    return true;
}


} } }
