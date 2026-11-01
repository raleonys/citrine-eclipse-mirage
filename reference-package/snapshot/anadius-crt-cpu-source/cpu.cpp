#include "cpu.hpp"
#include <bit>
#include <limits>

namespace anadius::crt_cpu_recovered {
std::uint32_t initialize_cpu_04b26c(State& state,Effects& effects) {
    const auto zero=effects.cpuid(0,0);
    const auto one=effects.cpuid(1,0); // unconditional even when leaf0 EAX<1
    std::uint32_t seven_ebx=0;
    if(zero.ebx==0x756e6547 && zero.edx==0x49656e69 && zero.ecx==0x6c65746e) {
        state.a1560=std::numeric_limits<std::uint64_t>::max();
        state.a1558=0x8000;
        const auto model=one.eax&0x0fff3ff0U;
        const auto difference=model-std::uint32_t{0x30650};
        const bool sparse_model=difference<=32 &&
            ((std::uint64_t{0x100010001}>>difference)&1)!=0;
        if(model==0x106c0 || model==0x20660 || model==0x20670 || sparse_model)
            state.a4788|=1;
    }
    // R8D snapshots the flags before CPUID7. The actual instruction cannot
    // mutate globals; source dependency effects still observe this ordering.
    const auto previous_flags=state.a4788;
    if(std::bit_cast<std::int32_t>(zero.eax)>=7) { // original CMP/JL is signed
        seven_ebx=effects.cpuid(7,0).ebx;
        if((seven_ebx&(1U<<9))!=0)state.a4788=previous_flags|2;
    }
    state.a1550=1;
    state.a1554=2;
    if((one.ecx&(1U<<20))!=0) {
        state.a1550=2;
        state.a1554=6;
        if((one.ecx&(1U<<27))!=0 && (one.ecx&(1U<<28))!=0) {
            const auto xcr0=effects.xgetbv(0);
            if((xcr0&6)==6) {
                state.a1550=3;
                state.a1554|=8;
                if((seven_ebx&0x20)!=0) {
                    state.a1550=5;
                    state.a1554|=0x20;
                    if((seven_ebx&0xd0030000U)==0xd0030000U && (xcr0&0xe0)==0xe0) {
                        state.a1554|=0x40;
                        state.a1550=6;
                    }
                }
            }
        }
    }
    return 0;
}
}
