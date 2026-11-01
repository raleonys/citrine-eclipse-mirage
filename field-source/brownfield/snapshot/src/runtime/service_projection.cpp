#include "service_projection_internal.hpp"

namespace preloader_l { namespace runtime {
using namespace model_derived_internal;

namespace {
void swap_vector(model_abi::MsvcVector24* a, model_abi::MsvcVector24* b) noexcept {
    const model_abi::MsvcVector24 temporary = *a;
    *a = *b;
    *b = temporary;
}
}

bool materialize_model_service_derived_state_from_store(
    model_abi::ReleaseServiceObject548* service,
    const preloader_l::config::StoreMaterializationSource& source) noexcept {
    if (!service || !source_valid(source)) return false;
    if (!materialize_entitlement_vector(service, source)) return false;
    if (!materialize_store_vectors(service, source)) {
        release_entitlement_vector(&service->store_entitlements);
        return false;
    }
    return true;
}

void swap_model_service_derived_state(
    model_abi::ReleaseServiceObject548* live,
    model_abi::ReleaseServiceObject548* candidate) noexcept {
    if (!live || !candidate || live == candidate) return;
    swap_vector(&live->store_entitlements, &candidate->store_entitlements);
    swap_vector(&live->preamble_bytes, &candidate->preamble_bytes);
    swap_vector(&live->trailer_bytes, &candidate->trailer_bytes);
    swap_vector(&live->store_payload, &candidate->store_payload);
}

bool release_model_service_derived_state_retryable(
    model_abi::ReleaseServiceObject548* service) noexcept {
    if (!service) return true;
    bool ok = true;
    ok = release_store_vectors(service) && ok;
    ok = release_entitlement_vector(&service->store_entitlements) && ok;
    return ok;
}

void release_model_service_derived_state(
    model_abi::ReleaseServiceObject548* service) noexcept {
    (void)release_model_service_derived_state_retryable(service);
}

} }
