#include "service_projection_internal.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"

namespace preloader_l { namespace runtime { namespace model_derived_internal {

constexpr uint64_t kMsvcCharSsoCapacity = 15u;

void zero_bytes(uint8_t* p, size_t n) noexcept {
    if (!p) return;
    for (size_t i = 0; i < n; ++i) p[i] = 0u;
}

void initialize_empty_string(MsvcString32* value) noexcept {
    if (!value) return;
    zero_bytes(value->storage, sizeof(value->storage));
    value->size = 0u;
    value->capacity = kMsvcCharSsoCapacity;
}

uint8_t* heap_string_pointer(MsvcString32* value) noexcept {
    if (!value || value->capacity <= kMsvcCharSsoCapacity) return nullptr;
    return *reinterpret_cast<uint8_t**>(value->storage);
}

bool release_string(MsvcString32* value) noexcept {
    if (!value) return true;
    if (uint8_t* p = heap_string_pointer(value)) {
        const auto status = release_block(
            p, static_cast<size_t>(value->capacity) + 1u,
            windows_allocator_platform_ops());
        if (status != AllocationReleaseStatus::ok) return false;
    }
    initialize_empty_string(value);
    return true;
}

bool assign_string(MsvcString32* value,
                   preloader_l::config::ValueView source) noexcept {
    if (!value) return false;
    if (!source.present || source.size == 0u) return release_string(value);
    if (!source.data) return false;

    if (source.size <= kMsvcCharSsoCapacity) {
        if (!release_string(value)) return false;
        for (size_t i = 0; i < source.size; ++i)
            value->storage[i] = static_cast<uint8_t>(source.data[i]);
        value->storage[source.size] = 0u;
        value->size = source.size;
        value->capacity = kMsvcCharSsoCapacity;
        return true;
    }

    if (source.size == static_cast<size_t>(-1)) return false;
    const auto allocation = allocate_block(
        source.size + 1u,
        AllocationRetryPolicy::outer_retry_only,
        windows_allocator_platform_ops());
    if (allocation.terminal != AllocationTerminal::none || !allocation.pointer)
        return false;

    auto* out = static_cast<uint8_t*>(allocation.pointer);
    for (size_t i = 0; i < source.size; ++i)
        out[i] = static_cast<uint8_t>(source.data[i]);
    out[source.size] = 0u;
    if (!release_string(value)) {
        (void)release_block(out, source.size + 1u, windows_allocator_platform_ops());
        return false;
    }
    zero_bytes(value->storage, sizeof(value->storage));
    *reinterpret_cast<uint8_t**>(value->storage) = out;
    value->size = source.size;
    value->capacity = source.size;
    return true;
}

void clear_vector(MsvcVector24* value) noexcept {
    if (!value) return;
    value->begin = value->end = value->capacity = 0u;
}

preloader_l::config::ModelByteVectorAllocatorBinding vector_binding() noexcept {
    return preloader_l::config::ModelByteVectorAllocatorBinding{
        &windows_allocator_platform_ops(),
        AllocationRetryPolicy::outer_retry_only,
    };
}

preloader_l::config::MsvcByteVectorLayout byte_layout_from_vector(
    const MsvcVector24& value) noexcept {
    return preloader_l::config::MsvcByteVectorLayout{
        reinterpret_cast<uint8_t*>(value.begin),
        reinterpret_cast<uint8_t*>(value.end),
        reinterpret_cast<uint8_t*>(value.capacity),
    };
}

void vector_from_byte_layout(
    MsvcVector24* destination,
    preloader_l::config::MsvcByteVectorLayout* source) noexcept {
    if (!destination || !source) return;
    destination->begin = reinterpret_cast<uintptr_t>(source->begin);
    destination->end = reinterpret_cast<uintptr_t>(source->end);
    destination->capacity = reinterpret_cast<uintptr_t>(source->capacity);
    source->begin = source->end = source->capacity = nullptr;
}

bool copy_byte_vector(
    MsvcVector24* destination,
    preloader_l::config::StoreByteView source) noexcept {
    if (!destination || (source.size && !source.data)) return false;
    preloader_l::config::MsvcByteVectorLayout temporary{};
    const auto status = preloader_l::config::model_byte_vector_copy_exact(
        source.data, source.size, vector_binding(), &temporary);
    if (status != preloader_l::config::ModelByteVectorStatus::ok)
        return false;
    vector_from_byte_layout(destination, &temporary);
    return true;
}

bool release_byte_vector(MsvcVector24* value) noexcept {
    if (!value) return true;
    auto layout = byte_layout_from_vector(*value);
    const auto status = preloader_l::config::model_byte_vector_release(
        vector_binding(), &layout);
    if (status != preloader_l::config::ModelByteVectorStatus::ok) return false;
    clear_vector(value);
    return true;
}

bool allocate_raw_vector(MsvcVector24* destination, size_t bytes) noexcept {
    if (!destination) return false;
    preloader_l::config::MsvcByteVectorLayout temporary{};
    const auto status = preloader_l::config::model_byte_vector_allocate_exact(
        bytes, vector_binding(), &temporary);
    if (status != preloader_l::config::ModelByteVectorStatus::ok)
        return false;
    vector_from_byte_layout(destination, &temporary);
    destination->end = destination->begin;
    return true;
}

bool release_string_vector(MsvcVector24* vector) noexcept {
    if (!vector || !vector->begin) {
        if (vector) clear_vector(vector);
        return true;
    }
    auto* current = reinterpret_cast<MsvcString32*>(vector->begin);
    auto* finish = reinterpret_cast<MsvcString32*>(vector->end);
    bool all = true;
    while (current != finish) {
        all = release_string(current) && all;
        ++current;
    }
    if (!all) return false;
    vector->end = vector->capacity;
    return release_byte_vector(vector);
}

bool materialize_string_vector(
    MsvcVector24* destination,
    const StoreMaterializationSource& source,
    size_t entitlement_index,
    size_t count,
    bool description_vector) noexcept {
    if (!destination || !source.feature_desc || !source.feature_flag)
        return false;
    clear_vector(destination);
    if (count == 0u) return true;
    if (count > static_cast<size_t>(-1) / sizeof(MsvcString32))
        return false;
    if (!allocate_raw_vector(destination, count * sizeof(MsvcString32)))
        return false;

    auto* begin = reinterpret_cast<MsvcString32*>(destination->begin);
    for (size_t i = 0; i < count; ++i) {
        initialize_empty_string(begin + i);
        const auto view = description_vector
            ? source.feature_desc(source.context, entitlement_index, i)
            : source.feature_flag(source.context, entitlement_index, i);
        if (!assign_string(begin + i, view)) {
            destination->end = reinterpret_cast<uintptr_t>(begin + i + 1u);
            (void)release_string_vector(destination);
            return false;
        }
        destination->end = reinterpret_cast<uintptr_t>(begin + i + 1u);
    }
    return true;
}

void initialize_record(ModelStoreEntitlement1E0* record) noexcept {
    if (!record) return;
    zero_bytes(reinterpret_cast<uint8_t*>(record), sizeof(*record));
    initialize_empty_string(&record->tag);
    initialize_empty_string(&record->nested.icon);
    initialize_empty_string(&record->nested.title);
    initialize_empty_string(&record->nested.description);
    initialize_empty_string(&record->nested.img_thumb);
    initialize_empty_string(&record->nested.img_cropped);
    initialize_empty_string(&record->nested.img_hero);
    initialize_empty_string(&record->vendor);
    initialize_empty_string(&record->key);
    initialize_empty_string(&record->hash);
}

bool release_record(ModelStoreEntitlement1E0* record) noexcept {
    if (!record) return true;
    bool ok = true;
    ok = release_byte_vector(&record->pad) && ok;
    ok = release_byte_vector(&record->field2) && ok;
    ok = release_byte_vector(&record->proto_bytes) && ok;
    ok = release_string(&record->hash) && ok;
    ok = release_string(&record->key) && ok;
    ok = release_string_vector(&record->feature_flag) && ok;
    ok = release_string(&record->vendor) && ok;
    ok = release_string_vector(&record->nested.feature_desc) && ok;
    ok = release_string(&record->nested.img_hero) && ok;
    ok = release_string(&record->nested.img_cropped) && ok;
    ok = release_string(&record->nested.img_thumb) && ok;
    ok = release_string(&record->nested.description) && ok;
    ok = release_string(&record->nested.title) && ok;
    ok = release_string(&record->nested.icon) && ok;
    ok = release_string(&record->tag) && ok;
    if (!ok) return false;
    record->numeric_id = 0u;
    record->category = 0u;
    record->release_ms = 0u;
    record->offer_id = 0u;
    record->flags = 0u;
    return true;
}

} } }
