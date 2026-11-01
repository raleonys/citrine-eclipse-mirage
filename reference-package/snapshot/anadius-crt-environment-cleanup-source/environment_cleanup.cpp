#include "environment_cleanup.hpp"
namespace anadius::crt_environment_cleanup_recovered {
void uninitialize_narrow_063f60(Environment& selected,State& state,Effects& effects) {
    const auto observed=selected;
    if(observed.data!=state.narrow.initial_0a52a0.data)
        crt_environment_recovered::free_environment_063d80(observed,effects);
}
void uninitialize_wide_063f7c(Environment& selected,State& state,Effects& effects) {
    const auto observed=selected;
    if(observed.data!=state.initial_wide_0a5298.data)
        crt_environment_recovered::free_environment_063d80(observed,effects);
}
void uninitialize_environment_064010(State& state,Effects& effects) {
    uninitialize_narrow_063f60(state.narrow.current_0a5288,state,effects);
    uninitialize_wide_063f7c(state.current_wide_0a5290,state,effects);
    // Read each initial identity/view only after earlier cleanup effects.
    crt_environment_recovered::free_environment_063d80(state.narrow.initial_0a52a0,effects);
    crt_environment_recovered::free_environment_063d80(state.initial_wide_0a5298,effects);
    // The original leaves all four globals unchanged, even after freeing them.
}
bool uninitialize_callback_0644a0(State& state,Effects& effects) {
    uninitialize_environment_064010(state,effects);
    return true;
}
}
