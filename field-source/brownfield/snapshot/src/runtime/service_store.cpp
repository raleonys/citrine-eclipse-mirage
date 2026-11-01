#include "service_projection_internal.hpp"

namespace preloader_l { namespace runtime { namespace model_derived_internal {

bool materialize_record(
    ModelStoreEntitlement1E0* destination,
    const StoreMaterializationSource& source,
    size_t index,
    const preloader_l::config::StoreEntitlementView& view) noexcept {
    if (!destination) return false;
    initialize_record(destination);
    destination->numeric_id = view.numeric_id;
    destination->category = view.category;
    destination->release_ms = view.release_ms;
    destination->offer_id = view.offer_id;
    destination->flags = view.flags;

    if (!assign_string(&destination->tag, view.tag) ||
        !assign_string(&destination->nested.icon, view.icon) ||
        !assign_string(&destination->nested.title, view.title) ||
        !assign_string(&destination->nested.description, view.description) ||
        !assign_string(&destination->nested.img_thumb, view.img_thumb) ||
        !assign_string(&destination->nested.img_cropped, view.img_cropped) ||
        !assign_string(&destination->nested.img_hero, view.img_hero) ||
        !materialize_string_vector(&destination->nested.feature_desc, source,
                                   index, view.feature_desc_count, true) ||
        !assign_string(&destination->vendor, view.vendor) ||
        !materialize_string_vector(&destination->feature_flag, source,
                                   index, view.feature_flag_count, false) ||
        !assign_string(&destination->key, view.key) ||
        !assign_string(&destination->hash, view.hash) ||
        !copy_byte_vector(&destination->proto_bytes, view.proto_bytes) ||
        !copy_byte_vector(&destination->field2, view.field2) ||
        !copy_byte_vector(&destination->pad, view.pad)) {
        (void)release_record(destination);
        return false;
    }
    return true;
}

bool release_entitlement_vector(MsvcVector24* vector) noexcept {
    if (!vector || !vector->begin) {
        if (vector) clear_vector(vector);
        return true;
    }
    auto* current = reinterpret_cast<ModelStoreEntitlement1E0*>(vector->begin);
    auto* finish = reinterpret_cast<ModelStoreEntitlement1E0*>(vector->end);
    bool all = true;
    while (current != finish) {
        all = release_record(current) && all;
        ++current;
    }
    if (!all) return false;
    vector->end = vector->capacity;
    return release_byte_vector(vector);
}

bool source_valid(const StoreMaterializationSource& source) noexcept {
    return source.loaded && source.entitlement_count && source.entitlement &&
           source.preamble && source.trailer &&
           source.feature_desc && source.feature_flag;
}

bool materialize_entitlement_vector(
    model_abi::ReleaseServiceObject548* service,
    const StoreMaterializationSource& source) noexcept {
    if (!service || !source_valid(source)) return false;
    clear_vector(&service->store_entitlements);
    if (!source.loaded(source.context)) return true;

    const size_t count = source.entitlement_count(source.context);
    if (count == 0u) return true;
    if (count > static_cast<size_t>(-1) / sizeof(ModelStoreEntitlement1E0))
        return false;
    if (!allocate_raw_vector(&service->store_entitlements,
                             count * sizeof(ModelStoreEntitlement1E0)))
        return false;

    auto* begin = reinterpret_cast<ModelStoreEntitlement1E0*>(
        service->store_entitlements.begin);
    for (size_t i = 0; i < count; ++i) {
        preloader_l::config::StoreEntitlementView view{};
        if (!source.entitlement(source.context, i, &view) ||
            !materialize_record(begin + i, source, i, view)) {
            service->store_entitlements.end = reinterpret_cast<uintptr_t>(begin + i);
            (void)release_entitlement_vector(&service->store_entitlements);
            return false;
        }
        service->store_entitlements.end = reinterpret_cast<uintptr_t>(begin + i + 1u);
    }
    return true;
}

preloader_l::config::ModelStoreVectorBindings store_bindings() noexcept {
    const auto binding = vector_binding();
    return preloader_l::config::ModelStoreVectorBindings{binding, binding};
}

bool materialize_store_vectors(
    model_abi::ReleaseServiceObject548* service,
    const StoreMaterializationSource& source) noexcept {
    clear_vector(&service->preamble_bytes);
    clear_vector(&service->trailer_bytes);
    clear_vector(&service->store_payload);
    if (!source.loaded(source.context) || source.entitlement_count(source.context) == 0u)
        return true;

    preloader_l::config::ModelStoreVectorSet vectors{};
    const auto status = preloader_l::config::materialize_model_store_vectors_from_source(
        store_bindings(), source, &vectors);
    if (status != preloader_l::config::ModelStoreVectorStatus::ok)
        return false;
    vector_from_byte_layout(&service->preamble_bytes, &vectors.preamble);
    vector_from_byte_layout(&service->trailer_bytes, &vectors.trailer);
    vector_from_byte_layout(&service->store_payload, &vectors.payload);
    return true;
}

bool release_store_vectors(model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return true;
    bool ok = true;
    ok = release_byte_vector(&service->store_payload) && ok;
    ok = release_byte_vector(&service->trailer_bytes) && ok;
    ok = release_byte_vector(&service->preamble_bytes) && ok;
    return ok;
}

} } }
