#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/config/msvc_byte_vector_abi.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"

namespace preloader_l { namespace config {

// Source-level binding between the curio x64 MSVC vector<uint8_t> physical
// layout and the curio allocator substrate. This is a consistency model until a
// specific native AE00/D5F0 production consumer is independently mapped.
struct ModelByteVectorAllocatorBinding {
    const preloader_l::runtime::AllocatorPlatformOps* ops;
    preloader_l::runtime::AllocationRetryPolicy retry_policy;
};

enum class ModelByteVectorStatus : uint8_t {
    ok = 0,
    invalid_argument,
    destination_not_empty,
    allocation_failed,
    size_overflow,
    malformed_layout,
    release_failed,
};

bool model_byte_vector_empty(const MsvcByteVectorLayout& value) noexcept;

ModelByteVectorStatus model_byte_vector_allocate_exact(
    size_t logical_size,
    const ModelByteVectorAllocatorBinding& binding,
    MsvcByteVectorLayout* output);

ModelByteVectorStatus model_byte_vector_copy_exact(
    const uint8_t* source,
    size_t size,
    const ModelByteVectorAllocatorBinding& binding,
    MsvcByteVectorLayout* output);

ModelByteVectorStatus model_byte_vector_release(
    const ModelByteVectorAllocatorBinding& binding,
    MsvcByteVectorLayout* value) noexcept;

} }
