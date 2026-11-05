#pragma once
#include "completion_dispatcher.hpp"

namespace anadius::socket_recovered {
struct OperationStorage {std::uintptr_t identity=0;std::span<std::byte> bytes;};
struct OperationStorageBinding {std::uintptr_t identity=0;OperationStorage* storage=nullptr;};
class OperationStorageResolver {
public:
    virtual ~OperationStorageResolver()=default;
    // Exact admitted owner lookup. Found supplies a stable live descriptor,
    // not authority or a payload pin. The owner outlives every returned borrow.
    virtual MemoryLookup lookup_storage(std::uintptr_t,OperationStorage*&)=0;
};
class PostedFunctor {
public:
    // Native slot00 boolean:1 consumes/invokes,0 discards. May end this lifetime.
    virtual void call_slot_00(std::uint8_t flag)=0;
protected:~PostedFunctor()=default;
};
struct PostedOperation {
    OperationStorage& storage;
    CancelOperation operation;
    std::uintptr_t callback_identity_28=0;
    PostedFunctor* functor_38=nullptr;
};
struct PostedCleanupFrame {
    std::uintptr_t unused_allocator_identity_00=0;
    OperationStorage* allocation_08=nullptr;
    PostedOperation* operation_10=nullptr;
};
struct PostedExecutor {
    // Stable original executor-address token. Low byte is an observed, unused
    // allocation argument; this is not a native object or executable pointer.
    std::uintptr_t identity=0;
    IocpSchedulerEntry* scheduler=nullptr;
    std::uint32_t flags_0c=0;
};
struct PostedEnvironment {
    std::uint32_t& tls_index;
    std::span<OperationStorageBinding> storage;
    // Optional borrowed owner view; copies follow coherent table replacement.
    // Owner and this span field outlive all contexts/calls using the view.
    const std::span<OperationStorageBinding>* live_storage=nullptr;
    std::span<OperationStorageBinding> current_storage()const noexcept{return live_storage?*live_storage:storage;}
    // Appended optional owner capability survives by-value context copies.
    // Miss/Ambiguous are authoritative; owner identity must be checked by the
    // integrating graph rather than inferred from a successful lookup.
    OperationStorageResolver* storage_resolver=nullptr;
    MemoryLookup lookup_storage(std::uintptr_t,OperationStorage*&)const;
};
class PostedMemoryEffects {
public:
    virtual ~PostedMemoryEffects()=default;
    // Actual allocator/free and the relationship between live typed projections
    // and bounded raw storage remain required. Allocation supplies enough bytes.
    virtual OperationStorage& allocate_04b0bc(std::uint64_t native_bytes)=0;
    virtual void free_04b788(OperationStorage&)=0;
    virtual PostedOperation& project_posted(OperationStorage&)=0;
};
OperationStorage& allocate_cached_021820(std::uint8_t unused_tag,WorkerThreadState*,
    std::uint64_t requested,std::uint64_t alignment,PostedEnvironment,PostedMemoryEffects&);
void consume_functor_021ac0(PostedFunctor*&);
void cleanup_posted_028fd0(PostedCleanupFrame&,PostedEnvironment,WorkerRunApi&,PostedMemoryEffects&);
void enqueue_posted_028ca0(IocpSchedulerEntry&,CancelOperation&,std::uint8_t unused_hint,DispatcherApi&);
void dispatch_posted_022e40(PostedExecutor&,PostedFunctor*&,PostedEnvironment,
    WorkerRunApi&,DispatcherApi&,PostedMemoryEffects&);
class OwnedHandlerTask;
// Uses the caller's real mutable slot. Transfer becomes visible before inline
// invocation or posting; no temporary base-pointer slot delays that update.
void dispatch_posted_022e40(PostedExecutor&,OwnedHandlerTask*&,PostedEnvironment,
    WorkerRunApi&,DispatcherApi&,PostedMemoryEffects&);
void complete_posted_027120(std::uintptr_t owner_identity,PostedOperation&,PostedEnvironment,
    WorkerRunApi&,PostedMemoryEffects&);
} // namespace anadius::socket_recovered
