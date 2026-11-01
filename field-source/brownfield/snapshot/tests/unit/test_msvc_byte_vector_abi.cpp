#include "preloader_l/config/msvc_byte_vector_abi.hpp"
#include <stdint.h>
using namespace preloader_l::config;


int main() {
    MsvcByteVectorReleasePlan plan{};
    MsvcByteVectorLayout empty{};
    if (msvc_byte_vector_release_plan(&empty, &plan) != MsvcByteVectorAbiStatus::ok) return 1;
    if (plan.allocation || plan.capacity_bytes || plan.request_bytes || plan.large_aligned) return 2;

    uint8_t small[64]{};
    MsvcByteVectorLayout small_vec{small, small + 7, small + sizeof(small)};
    if (msvc_byte_vector_release_plan(&small_vec, &plan) != MsvcByteVectorAbiStatus::ok) return 3;
    if (plan.allocation != small || plan.capacity_bytes != sizeof(small) ||
        plan.request_bytes != sizeof(small) || plan.large_aligned) return 4;

    alignas(32) uint8_t large_storage[0x1000u + 0x80u]{};
    const uintptr_t raw_address = reinterpret_cast<uintptr_t>(large_storage);
    const uintptr_t begin_address = (raw_address + 0x27u) & ~uintptr_t(0x1fu);
    auto* begin = reinterpret_cast<uint8_t*>(begin_address);
    *(reinterpret_cast<void**>(begin) - 1) = large_storage;
    MsvcByteVectorLayout large_vec{begin, begin + 3, begin + 0x1000u};
    if (msvc_byte_vector_release_plan(&large_vec, &plan) != MsvcByteVectorAbiStatus::ok) return 5;
    if (plan.allocation != large_storage || plan.capacity_bytes != 0x1000u ||
        plan.request_bytes != 0x1027u || !plan.large_aligned) return 6;

    uint8_t old_dst[16]{};
    uint8_t src_mem[24]{};
    MsvcByteVectorLayout dst{old_dst, old_dst + 2, old_dst + sizeof(old_dst)};
    MsvcByteVectorLayout src{src_mem, src_mem + 5, src_mem + sizeof(src_mem)};
    if (msvc_byte_vector_move_assign_layout(&dst, &src, &plan) != MsvcByteVectorAbiStatus::ok) return 7;
    if (plan.allocation != old_dst || plan.capacity_bytes != sizeof(old_dst)) return 8;
    if (dst.begin != src_mem || dst.end != src_mem + 5 || dst.capacity != src_mem + sizeof(src_mem)) return 9;
    if (src.begin || src.end || src.capacity) return 10;
    if (msvc_byte_vector_size(dst) != 5u) return 11;

    MsvcByteVectorLayout malformed{small + 8, small + 4, small + 16};
    if (msvc_byte_vector_release_plan(&malformed, &plan) != MsvcByteVectorAbiStatus::malformed_layout) return 12;
    return 0;
}
