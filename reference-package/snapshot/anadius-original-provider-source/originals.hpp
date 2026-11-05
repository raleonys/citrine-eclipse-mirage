#pragma once
#include "../anadius-native-source/native_callbacks.hpp"
#include <functional>
#include <cstdint>

namespace anadius::original_provider {
using Factory=std::function<std::shared_ptr<native_recovered::OriginalProvider>(const native_recovered::OriginalSlots&)>;
// Explicit code_lifetime must keep the selected code/trampolines callable.
// A nonnull arbitrary token is not proof of code validity or engine success.
// Bind after RuntimeSeed reaches its stable owner. No DLL is loaded or pinned.
std::shared_ptr<native_recovered::OriginalProvider> bind(const native_recovered::OriginalSlots&,std::shared_ptr<void> code_lifetime);
Factory make_factory(std::shared_ptr<void> code_lifetime);
enum class PublicationPhase {Unbound,Created,Setup,Ready,Stopped,Poisoned,Retiring};
struct PublicationSnapshot {PublicationPhase phase;std::uint16_t published_mask;bool creating;};
// Selected one-pass host setup: exact slot identity is bound once, then each
// successful create publishes its immutable original BEFORE enable. Other slots
// may still be under construction. load never treats seeded/nonzero failed
// output as a receipt. This is code-pointer publication, NOT callback/state/PTD
// admission or permission to uninitialize an engine/unload its code.
class Publication final:public native_recovered::OriginalProvider {
public:
    explicit Publication(std::shared_ptr<void> code_lifetime);
    ~Publication()noexcept override;
    Publication(const Publication&)=delete;Publication& operator=(const Publication&)=delete;
    void bind_slots(const native_recovered::OriginalSlots&);
    const native_recovered::OriginalSlots* slots_identity()const noexcept override;
    std::uintptr_t load(native_recovered::Callback)override;
    void setup_begin()override;
    void setup_complete()override;
    void stop()noexcept override;
    void before_create(native_recovered::Callback,std::uintptr_t target,std::uintptr_t& output);
    void after_create(native_recovered::Callback,std::uintptr_t target,std::uintptr_t& output,int status);
    void before_enable(std::uintptr_t target);
    void unknown()noexcept;
    PublicationSnapshot snapshot()const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
// Copied factories share this one single-bind owner; actual Runtime slots are
// adopted only when callback Policy is constructed at their stable address.
Factory make_publishing_factory(std::shared_ptr<Publication>);
// Protocol: Created -> setup_begin(controller) -> setup_complete(seal all10)
// -> Stopped. During setup only controller may load current nonzero slots.
// Ready loads require selected slot == sealed nonzero value. No reset/reuse,
// an observed sealed-slot mismatch stops further loads even if later restored.
// repair, global setter or assumption that nonzero means successfully hooked.
// Metadata locking preserves LastError and never spans an original SDK call.
// Caller serializes slot writes to setup; raw concurrent array writes are not
// made safe. Retention does not prevent explicit engine uninitialize/unload.
// The legacy bind/make_factory protocol above is unchanged. Publication has an
// additional controller/fiber-affine create transaction and per-slot receipts.
// After publication, the host must never write that slot again; unrelated slots
// are distinct memory locations. External raw writes remain excluded. Stop
// needs actual callback/source quiescence; poison/unknown never becomes Closed.
} // namespace anadius::original_provider
