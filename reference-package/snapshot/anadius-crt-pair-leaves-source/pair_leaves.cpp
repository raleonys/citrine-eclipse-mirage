#include "pair_leaves.hpp"
namespace anadius::crt_pair_leaves_recovered {
bool uninitialize_heap_0704e8(crt_heap_recovered::State& state) noexcept {
    // Original AND qword[A5A10],0; no HeapDestroy or free operation.
    state.heap_0a5a10=0;
    return true;
}
bool uninitialize_vcrt_0644f0(vcrt_ptd_recovered::State& state,vcrt_ptd_recovered::Effects& effects) {
    // Original XOR ECX,ECX; tail JMP04d5a8. Forward the actual shared core.
    return vcrt_ptd_recovered::uninitialize_04d5a8(state,effects,0);
}
bool success_006500() noexcept {return true;}
bool initialize_command_lines_06f9bc(CommandLines& state,CommandLineEffects& effects) {
    state.narrow.command_line=effects.get_command_line_a();
    state.wide_0a5a08=effects.get_command_line_w();
    return true;
}
}
