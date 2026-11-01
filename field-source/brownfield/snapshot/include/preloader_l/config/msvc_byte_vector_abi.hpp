#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace config {

// Physical x64 MSVC std::vector<uint8_t> object shape recovered from the
// model AE00/D5F0 call chain. The allocator is stateless, so the object is
// exactly the three pointer fields below: begin, logical end, capacity end.
//
// This adapter models the ABI and ownership transitions without depending on
// STL or allocating memory itself. That keeps the release path freestanding
// while making the original compiler object contract explicit and testable.
struct MsvcByteVectorLayout {
    uint8_t* begin;
    uint8_t* end;
    uint8_t* capacity;
};

static_assert(sizeof(MsvcByteVectorLayout) == 24u,
              "x64 MSVC byte-vector layout must be three pointers");
static_assert(offsetof(MsvcByteVectorLayout, begin) == 0u, "begin ABI drift");
static_assert(offsetof(MsvcByteVectorLayout, end) == 8u, "end ABI drift");
static_assert(offsetof(MsvcByteVectorLayout, capacity) == 16u, "capacity ABI drift");

enum class MsvcByteVectorAbiStatus : uint8_t {
    ok = 0,
    invalid_argument,
    malformed_layout,
    malformed_large_allocation,
};

// Model delete path recovered at 0x15280 / 0x26100:
//  * capacity span < 0x1000: delete(begin)
//  * capacity span >= 0x1000: raw pointer is stored at begin[-8], the aligned
//    begin pointer must be 8..39 bytes after the raw allocation and the
//    deallocation-size accounting adds 0x27 bytes.
// The original delete entry ignores the size value; request_bytes is retained
// here because it is part of the allocator provenance contract.
struct MsvcByteVectorReleasePlan {
    void* allocation;
    size_t capacity_bytes;
    size_t request_bytes;
    bool large_aligned;
};

MsvcByteVectorAbiStatus msvc_byte_vector_release_plan(
    const MsvcByteVectorLayout* value,
    MsvcByteVectorReleasePlan* plan) noexcept;

// Models the exact pointer-state part of the curio move assignment at
// 0x15280. The destination's old release plan is returned to the caller; then
// the three pointers are stolen and the source is reset to all-null. This
// function deliberately does not invoke a CRT allocator/deallocator.
MsvcByteVectorAbiStatus msvc_byte_vector_move_assign_layout(
    MsvcByteVectorLayout* destination,
    MsvcByteVectorLayout* source,
    MsvcByteVectorReleasePlan* old_destination) noexcept;

inline size_t msvc_byte_vector_size(const MsvcByteVectorLayout& value) noexcept {
    return value.begin && value.end && value.end >= value.begin
        ? static_cast<size_t>(value.end - value.begin)
        : 0u;
}

} }
