#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

enum class RdxProducerKind : uint32_t {
    unknown = 0,
    zero_xor = 1,
    mov_imm32 = 2,
    mov_imm64 = 3,
    lea_rip_relative = 4,
    mov_rip_relative = 5,
    mov_from_register = 6,
    mov_from_rbx_disp8 = 7,
    mov_from_rbx_disp32 = 8,
    mov_from_rbp_disp8 = 9,
    mov_from_rbp_disp32 = 10,
    mov_from_rcx_disp8 = 11,
    mov_from_rcx_disp32 = 12,
};

struct RdxProducer {
    RdxProducerKind kind;
    size_t offset;
    size_t length;
    int64_t displacement;
    uint64_t immediate;
    uint32_t source_register;
};

// Best-effort local x64 pattern classifier. `code` is the byte range ending
// immediately before the call instruction that consumed RDX. The nearest
// recognized write to RDX wins. This is diagnostic only and is never used to
// alter runtime behavior.
RdxProducer find_last_rdx_producer(const uint8_t* code, size_t size) noexcept;

} }
