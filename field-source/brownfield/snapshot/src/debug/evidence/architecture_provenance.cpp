#include "preloader_l/platform/architecture_provenance.hpp"

namespace preloader_l { namespace platform {
namespace {
uint32_t read_u32_le(const uint8_t* p) noexcept {
    return static_cast<uint32_t>(p[0]) |
           (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) |
           (static_cast<uint32_t>(p[3]) << 24);
}
uint64_t read_u64_le(const uint8_t* p) noexcept {
    uint64_t v = 0;
    for (unsigned i = 0; i < 8; ++i) v |= static_cast<uint64_t>(p[i]) << (i * 8);
    return v;
}
int32_t read_i32_le(const uint8_t* p) noexcept {
    return static_cast<int32_t>(read_u32_le(p));
}
}

RdxProducer find_last_rdx_producer(const uint8_t* code, size_t size) noexcept {
    RdxProducer best{RdxProducerKind::unknown, 0, 0, 0, 0, 0};
    if (!code || size == 0) return best;

    for (size_t i = 0; i < size; ++i) {
        RdxProducer cur{RdxProducerKind::unknown, i, 0, 0, 0, 0};
        const size_t rem = size - i;

        if (rem >= 2 && ((code[i] == 0x33 && code[i+1] == 0xD2) ||
                         (code[i] == 0x31 && code[i+1] == 0xD2))) {
            cur.kind = RdxProducerKind::zero_xor; cur.length = 2;
        } else if (rem >= 3 && code[i] == 0x48 &&
                   ((code[i+1] == 0x33 && code[i+2] == 0xD2) ||
                    (code[i+1] == 0x31 && code[i+2] == 0xD2))) {
            cur.kind = RdxProducerKind::zero_xor; cur.length = 3;
        } else if (rem >= 5 && code[i] == 0xBA) {
            cur.kind = RdxProducerKind::mov_imm32; cur.length = 5;
            cur.immediate = read_u32_le(code + i + 1);
        } else if (rem >= 10 && code[i] == 0x48 && code[i+1] == 0xBA) {
            cur.kind = RdxProducerKind::mov_imm64; cur.length = 10;
            cur.immediate = read_u64_le(code + i + 2);
        } else if (rem >= 7 && code[i] == 0x48 && code[i+1] == 0x8D && code[i+2] == 0x15) {
            cur.kind = RdxProducerKind::lea_rip_relative; cur.length = 7;
            cur.displacement = read_i32_le(code + i + 3);
        } else if (rem >= 7 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x15) {
            cur.kind = RdxProducerKind::mov_rip_relative; cur.length = 7;
            cur.displacement = read_i32_le(code + i + 3);
        } else if (rem >= 4 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x53) {
            cur.kind = RdxProducerKind::mov_from_rbx_disp8; cur.length = 4;
            cur.displacement = static_cast<int8_t>(code[i+3]);
        } else if (rem >= 7 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x93) {
            cur.kind = RdxProducerKind::mov_from_rbx_disp32; cur.length = 7;
            cur.displacement = read_i32_le(code + i + 3);
        } else if (rem >= 4 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x55) {
            cur.kind = RdxProducerKind::mov_from_rbp_disp8; cur.length = 4;
            cur.displacement = static_cast<int8_t>(code[i+3]);
        } else if (rem >= 7 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x95) {
            cur.kind = RdxProducerKind::mov_from_rbp_disp32; cur.length = 7;
            cur.displacement = read_i32_le(code + i + 3);
        } else if (rem >= 4 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x51) {
            cur.kind = RdxProducerKind::mov_from_rcx_disp8; cur.length = 4;
            cur.displacement = static_cast<int8_t>(code[i+3]);
        } else if (rem >= 7 && code[i] == 0x48 && code[i+1] == 0x8B && code[i+2] == 0x91) {
            cur.kind = RdxProducerKind::mov_from_rcx_disp32; cur.length = 7;
            cur.displacement = read_i32_le(code + i + 3);
        } else if (rem >= 3 && code[i] == 0x48 && code[i+1] == 0x8B &&
                   code[i+2] >= 0xD0 && code[i+2] <= 0xD7) {
            cur.kind = RdxProducerKind::mov_from_register; cur.length = 3;
            cur.source_register = static_cast<uint32_t>(code[i+2] - 0xD0); // rax..rdi legacy register set
        } else if (rem >= 3 && code[i] == 0x48 && code[i+1] == 0x89 &&
                   (code[i+2] & 0xC7u) == 0xC2u) {
            // 48 89 /r with mod=3, r/m=rdx: MOV RDX, r64. reg is bits 3..5.
            cur.kind = RdxProducerKind::mov_from_register; cur.length = 3;
            cur.source_register = static_cast<uint32_t>((code[i+2] >> 3) & 7u);
        }

        if (cur.kind != RdxProducerKind::unknown && i + cur.length <= size)
            best = cur;
    }
    return best;
}

} }
