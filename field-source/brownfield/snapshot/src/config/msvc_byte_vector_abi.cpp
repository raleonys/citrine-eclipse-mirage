#include "preloader_l/config/msvc_byte_vector_abi.hpp"
#include <stdint.h>

namespace preloader_l { namespace config {
namespace {

void clear_plan(MsvcByteVectorReleasePlan* plan) noexcept {
    if (!plan) return;
    plan->allocation = nullptr;
    plan->capacity_bytes = 0u;
    plan->request_bytes = 0u;
    plan->large_aligned = false;
}

bool pointers_all_null(const MsvcByteVectorLayout& value) noexcept {
    return !value.begin && !value.end && !value.capacity;
}

}

MsvcByteVectorAbiStatus msvc_byte_vector_release_plan(
    const MsvcByteVectorLayout* value,
    MsvcByteVectorReleasePlan* plan) noexcept {
    if (!value || !plan) return MsvcByteVectorAbiStatus::invalid_argument;
    clear_plan(plan);

    if (pointers_all_null(*value)) return MsvcByteVectorAbiStatus::ok;
    if (!value->begin || !value->end || !value->capacity ||
        value->end < value->begin || value->capacity < value->end)
        return MsvcByteVectorAbiStatus::malformed_layout;

    const size_t capacity_bytes =
        static_cast<size_t>(value->capacity - value->begin);
    plan->capacity_bytes = capacity_bytes;

    if (capacity_bytes < 0x1000u) {
        plan->allocation = value->begin;
        plan->request_bytes = capacity_bytes;
        plan->large_aligned = false;
        return MsvcByteVectorAbiStatus::ok;
    }

    // MSVC's large-vector allocation path requests capacity + 0x27 bytes,
    // aligns the user pointer to 32 bytes and stores the raw allocation pointer
    // in the slot immediately preceding the aligned pointer.
    void* raw = *(reinterpret_cast<void**>(value->begin) - 1);
    if (!raw) return MsvcByteVectorAbiStatus::malformed_large_allocation;

    const uintptr_t begin = reinterpret_cast<uintptr_t>(value->begin);
    const uintptr_t raw_address = reinterpret_cast<uintptr_t>(raw);
    if (raw_address >= begin || begin - raw_address < 8u)
        return MsvcByteVectorAbiStatus::malformed_large_allocation;

    const uintptr_t recovered_delta = begin - raw_address - 8u;
    if (recovered_delta > 0x1fu)
        return MsvcByteVectorAbiStatus::malformed_large_allocation;
    if (capacity_bytes > static_cast<size_t>(-1) - 0x27u)
        return MsvcByteVectorAbiStatus::malformed_large_allocation;

    plan->allocation = raw;
    plan->request_bytes = capacity_bytes + 0x27u;
    plan->large_aligned = true;
    return MsvcByteVectorAbiStatus::ok;
}

MsvcByteVectorAbiStatus msvc_byte_vector_move_assign_layout(
    MsvcByteVectorLayout* destination,
    MsvcByteVectorLayout* source,
    MsvcByteVectorReleasePlan* old_destination) noexcept {
    if (!destination || !source || !old_destination)
        return MsvcByteVectorAbiStatus::invalid_argument;
    clear_plan(old_destination);
    if (destination == source) return MsvcByteVectorAbiStatus::ok;

    const auto status = msvc_byte_vector_release_plan(destination, old_destination);
    if (status != MsvcByteVectorAbiStatus::ok) return status;

    destination->begin = source->begin;
    destination->end = source->end;
    destination->capacity = source->capacity;
    source->begin = nullptr;
    source->end = nullptr;
    source->capacity = nullptr;
    return MsvcByteVectorAbiStatus::ok;
}

} }
