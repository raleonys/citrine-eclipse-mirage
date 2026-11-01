#pragma once
#include "../anadius-native-source/native_callbacks.hpp"
#include <functional>

namespace anadius::original_provider {
using Factory=std::function<std::shared_ptr<native_recovered::OriginalProvider>(const native_recovered::OriginalSlots&)>;
// Explicit code_lifetime must keep the selected code/trampolines callable.
// A nonnull arbitrary token is not proof of code validity or engine success.
// Bind after RuntimeSeed reaches its stable owner. No DLL is loaded or pinned.
std::shared_ptr<native_recovered::OriginalProvider> bind(const native_recovered::OriginalSlots&,std::shared_ptr<void> code_lifetime);
Factory make_factory(std::shared_ptr<void> code_lifetime);
// Protocol: Created -> setup_begin(controller) -> setup_complete(seal all10)
// -> Stopped. During setup only controller may load current nonzero slots.
// Ready loads require selected slot == sealed nonzero value. No reset/reuse,
// an observed sealed-slot mismatch stops further loads even if later restored.
// repair, global setter or assumption that nonzero means successfully hooked.
// Metadata locking preserves LastError and never spans an original SDK call.
// Caller serializes slot writes to setup; raw concurrent array writes are not
// made safe. Retention does not prevent explicit engine uninitialize/unload.
} // namespace anadius::original_provider
