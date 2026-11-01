#include "errors.hpp"
#include <array>

namespace anadius::crt_errors_recovered {
namespace {
struct Mapping { std::uint32_t os, crt; };
// 45 little-endian pairs, exactly in original table order at VA180089f70.
constexpr std::array<Mapping,45> mappings{{
    {1,22},{2,2},{3,2},{4,24},{5,13},{6,9},{7,12},{8,12},{9,12},
    {10,7},{11,8},{12,22},{13,22},{15,2},{16,13},{17,18},{18,2},
    {33,13},{53,2},{65,13},{67,2},{80,17},{82,13},{83,13},{87,22},
    {89,11},{108,13},{109,32},{112,28},{114,9},{128,10},{129,10},
    {130,9},{131,22},{132,13},{145,41},{158,13},{161,2},{164,11},
    {167,13},{183,17},{206,2},{215,11},{1113,42},{1816,12}
}};
Cell32 select_cell(void* ptd, std::size_t offset, std::uint32_t& fallback) {
    return Cell32(ptd ? static_cast<void*>(static_cast<std::byte*>(ptd)+offset)
                      : static_cast<void*>(&fallback));
}
}
std::uint32_t errno_from_os_error_05d650(std::uint32_t code) noexcept {
    for (const auto& mapping : mappings)
        if (mapping.os == code) return mapping.crt;
    // Deliberately unsigned subtraction, including uint32 wrap at zero.
    if (code-std::uint32_t{19} <= 17) return 13;
    if (code-std::uint32_t{188} <= 14) return 8;
    return 22;
}
Cell32 dos_error_cell_05d704(State& s, Effects& e) {
    return select_cell(e.optional_ptd_064d08(),0x24,s.dos_fallback);
}
Cell32 errno_cell_05d724(State& s, Effects& e) {
    return select_cell(e.optional_ptd_064d08(),0x20,s.errno_fallback);
}
void map_os_error_05d698(std::uint32_t code, State& s, Effects& e) {
    // First write is already observable when the second lookup occurs/fails.
    dos_error_cell_05d704(s,e).store(code);
    const auto mapped = errno_from_os_error_05d650(code);
    errno_cell_05d724(s,e).store(mapped);
}
}
