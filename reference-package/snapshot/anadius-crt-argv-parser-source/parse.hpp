#pragma once
#include <cstdint>
#include <span>

namespace anadius::crt_argv_parse_recovered {
struct Counts { std::uint64_t pointers{}, characters{}; };
struct Effects {
    virtual ~Effects() = default;
    virtual std::int32_t is_mb_lead_06f9a8(std::int32_t signed_byte) = 0;
};

// 0x1800636c0 normal flow. Empty output spans independently model NULL.
// Input and output storage must remain stable and disjoint; Counts is distinct.
// The provider may inspect progress but must not mutate these objects.
// Missing input/trail bytes and insufficient output storage throw out_of_range;
// these are new model guards, not recovered original exception behavior.
void parse_0636c0(std::span<const char> input, std::span<char*> argv,
                 std::span<char> text, Counts& counts, Effects& effects);
}
