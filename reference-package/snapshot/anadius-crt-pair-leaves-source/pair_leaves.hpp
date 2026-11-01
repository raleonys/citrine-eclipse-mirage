#pragma once
#include "../anadius-crt-heap-source/heap.hpp"
#include "../anadius-vcrt-ptd-source/vcrt_ptd.hpp"
#include "../anadius-crt-argv-source/argv.hpp"
#include <span>
namespace anadius::crt_pair_leaves_recovered {
bool uninitialize_heap_0704e8(crt_heap_recovered::State&) noexcept;
bool uninitialize_vcrt_0644f0(vcrt_ptd_recovered::State&,vcrt_ptd_recovered::Effects&);
bool success_006500() noexcept;
struct CommandLines {
    crt_argv_recovered::State& narrow;
    std::span<const char16_t>& wide_0a5a08;
};
struct CommandLineEffects {
    virtual ~CommandLineEffects()=default;
    // Borrowed, provider-bounded views. Empty represents NULL; nonempty spans
    // include a terminator for downstream consumers. Storage stays live while
    // published. No raw pointer scanning, ownership transfer or OS acquisition.
    virtual std::span<const char> get_command_line_a()=0;
    virtual std::span<const char16_t> get_command_line_w()=0;
};
// Bounds are new model metadata; this interface does not claim native ABI.
bool initialize_command_lines_06f9bc(CommandLines&,CommandLineEffects&);
}
