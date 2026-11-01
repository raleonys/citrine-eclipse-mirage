#include "preloader_l/config/byte_vector_binding.hpp"

namespace preloader_l { namespace config {
namespace {

bool binding_valid(const ModelByteVectorAllocatorBinding& binding) noexcept {
    return binding.ops && binding.ops->heap_alloc && binding.ops->heap_free &&
           binding.ops->call_new_handler;
}

ModelByteVectorStatus map_terminal(preloader_l::runtime::AllocationTerminal terminal) noexcept {
    using preloader_l::runtime::AllocationTerminal;
    switch (terminal) {
        case AllocationTerminal::none: return ModelByteVectorStatus::ok;
        case AllocationTerminal::bad_array_new_length: return ModelByteVectorStatus::size_overflow;
        case AllocationTerminal::bad_alloc: default: return ModelByteVectorStatus::allocation_failed;
    }
}

}

bool model_byte_vector_empty(const MsvcByteVectorLayout& value) noexcept {
    return value.begin == nullptr && value.end == nullptr && value.capacity == nullptr;
}

ModelByteVectorStatus model_byte_vector_allocate_exact(
    size_t logical_size,
    const ModelByteVectorAllocatorBinding& binding,
    MsvcByteVectorLayout* output) {
    if (!output || !binding_valid(binding))
        return ModelByteVectorStatus::invalid_argument;
    if (!model_byte_vector_empty(*output))
        return ModelByteVectorStatus::destination_not_empty;
    if (logical_size == 0u)
        return ModelByteVectorStatus::ok;

    const auto result = preloader_l::runtime::allocate_block(
        logical_size, binding.retry_policy, *binding.ops);
    const auto terminal = map_terminal(result.terminal);
    if (terminal != ModelByteVectorStatus::ok)
        return terminal;
    if (!result.pointer)
        return ModelByteVectorStatus::allocation_failed;

    auto* begin = static_cast<uint8_t*>(result.pointer);
    output->begin = begin;
    output->end = begin + logical_size;
    output->capacity = output->end;
    return ModelByteVectorStatus::ok;
}

ModelByteVectorStatus model_byte_vector_copy_exact(
    const uint8_t* source,
    size_t size,
    const ModelByteVectorAllocatorBinding& binding,
    MsvcByteVectorLayout* output) {
    if ((!source && size != 0u) || !output)
        return ModelByteVectorStatus::invalid_argument;
    const auto status = model_byte_vector_allocate_exact(size, binding, output);
    if (status != ModelByteVectorStatus::ok)
        return status;
    for (size_t i = 0; i < size; ++i)
        output->begin[i] = source[i];
    return ModelByteVectorStatus::ok;
}

ModelByteVectorStatus model_byte_vector_release(
    const ModelByteVectorAllocatorBinding& binding,
    MsvcByteVectorLayout* value) noexcept {
    if (!value || !binding_valid(binding))
        return ModelByteVectorStatus::invalid_argument;

    MsvcByteVectorReleasePlan plan{};
    const auto abi = msvc_byte_vector_release_plan(value, &plan);
    if (abi != MsvcByteVectorAbiStatus::ok)
        return ModelByteVectorStatus::malformed_layout;

    // release_plan already reconstructs the original raw allocation pointer for
    // >=0x1000 aligned vectors, so the same direct free operation is sufficient
    // for both small and large physical layouts.
    if (plan.allocation) {
        const auto released = preloader_l::runtime::free_direct(plan.allocation, *binding.ops);
        if (released != preloader_l::runtime::AllocationReleaseStatus::ok)
            return ModelByteVectorStatus::release_failed;
    }

    value->begin = nullptr;
    value->end = nullptr;
    value->capacity = nullptr;
    return ModelByteVectorStatus::ok;
}

} }
