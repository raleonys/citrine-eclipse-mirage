#include "preloader_l/platform/architecture_provenance.hpp"
#include <cassert>
#include <cstdint>
using namespace preloader_l::platform;
int main() {
    {
        const uint8_t b[] = {0x90,0x48,0x8B,0x15,0x34,0x12,0x00,0x00,0x90};
        auto r = find_last_rdx_producer(b,sizeof(b));
        assert(r.kind == RdxProducerKind::mov_rip_relative);
        assert(r.offset == 1 && r.length == 7 && r.displacement == 0x1234);
    }
    {
        const uint8_t b[] = {0x48,0x8B,0x55,0xE8,0x33,0xD2};
        auto r = find_last_rdx_producer(b,sizeof(b));
        assert(r.kind == RdxProducerKind::zero_xor);
        assert(r.offset == 4);
    }
    {
        const uint8_t b[] = {0xBA,0x78,0x56,0x34,0x12};
        auto r = find_last_rdx_producer(b,sizeof(b));
        assert(r.kind == RdxProducerKind::mov_imm32 && r.immediate == 0x12345678u);
    }
    {
        const uint8_t b[] = {0x48,0x8B,0x93,0x44,0x33,0x22,0x11};
        auto r = find_last_rdx_producer(b,sizeof(b));
        assert(r.kind == RdxProducerKind::mov_from_rbx_disp32);
        assert(r.displacement == 0x11223344);
    }
    return 0;
}
