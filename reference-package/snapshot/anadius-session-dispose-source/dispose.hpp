#pragma once
#include "../anadius-session-owner-source/owner.hpp"

namespace anadius::socket_recovered {
// Required sized-free/terminal effects. The allocator must recognize every
// projected string/block/map/proxy/control address and retain the correct C++
// allocation provenance. Native byte requests are not sizeof(host sidecars).
class SessionFreeEffects {
public:
    virtual ~SessionFreeEffects()=default;
    virtual void free_sized_04b0f8(std::uintptr_t,std::uint64_t native_bytes)=0;
    [[noreturn]] virtual void invalid_parameter_05c974()=0;
};
struct SessionDisposalContext {
    ReceiveBufferMemory& memory;SessionFreeEffects& allocation;LifecycleEffects& lifecycle;
};
// Live typed projections, not native vtable/layout overlays. All providers and
// projected allocations survive until their native last use. Serialized valid
// deque state and externally established operation/raw-job quiescence required.
void clear_session_output_026880(OutputDeque&,SessionDisposalContext&);
void dispose_session_control_026570(SessionControl&,SessionDisposalContext&);

//017420 is a nullable, virtual slot10(flags1) dispatch, not a direct026290 call.
class SessionDeleteTarget {
public:virtual std::uintptr_t delete_flags_slot_10(std::uint32_t flags)=0;
protected:~SessionDeleteTarget()=default;
};
void delete_session_control_017420(SessionDeleteTarget*);
// Restores084750 identity before optional sizedfree1c8. The returned numeric
// identity can denote freed storage; callers must never use it as a live owner.
std::uintptr_t delete_session_control_026290(SessionControl&,std::uint32_t flags,SessionFreeEffects&);

// Concrete session-control policy binds all three actual normal source bodies.
// Context is borrowed and must outlive the final weak release, including frees.
// No implicit cleanup, automatic partial-constructor rollback or native ABI claim.
class RecoveredSessionControl final:public SessionControl,public SessionDeleteTarget {
public:
    explicit RecoveredSessionControl(SessionDisposalContext& context):context_(context){}
    void dispose_slot_00() override;
    void delete_slot_08() override;
    std::uintptr_t delete_flags_slot_10(std::uint32_t flags) override;
private:SessionDisposalContext& context_;
};
} // namespace anadius::socket_recovered
