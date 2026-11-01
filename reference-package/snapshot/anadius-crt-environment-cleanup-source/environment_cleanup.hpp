#pragma once
#include "../anadius-crt-environment-source/environment.hpp"
namespace anadius::crt_environment_cleanup_recovered {
using Environment=crt_environment_recovered::Environment;
using Effects=crt_environment_recovered::Effects;
// All references and pointer-vector storage remain live through cleanup.
// Wide fields are cleanup-only views of live char* address objects: payloads
// may contain UTF-16 bytes, but this interface is not a native char16_t** ABI.
// No reinterpretation of native wide pointer arrays is supported.
struct State {
    crt_environment_recovered::State& narrow;
    Environment& current_wide_0a5290;
    Environment& initial_wide_0a5298;
};
void uninitialize_narrow_063f60(Environment& selected,State&,Effects&);
void uninitialize_wide_063f7c(Environment& selected,State&,Effects&);
void uninitialize_environment_064010(State&,Effects&);
bool uninitialize_callback_0644a0(State&,Effects&);
}
