#pragma once
#include <cstdint>

namespace anadius::crt_cpu_recovered {
struct Registers {std::uint32_t eax,ebx,ecx,edx;};
// Caller-owned source state, not an original packed struct. Defaults select a
// new zero state; original image initialization is separately recorded.
struct State {
    std::uint32_t a1550{},a1554{};
    std::uint64_t a1558{},a1560{};
    std::uint32_t a4788{};
};
class Effects {
public:
    virtual ~Effects()=default;
    virtual Registers cpuid(std::uint32_t leaf,std::uint32_t subleaf)=0;
    virtual std::uint64_t xgetbv(std::uint32_t index)=0;
};
std::uint32_t initialize_cpu_04b26c(State&,Effects&);
}
