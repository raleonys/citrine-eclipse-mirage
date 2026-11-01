#pragma once
#include "../anadius-session-dispose-source/dispose.hpp"
#include "../anadius-thread-domain-source/domain.hpp"
#include "../anadius-worker-handoff-source/handoff.hpp"
#include <mutex>

namespace anadius::socket_recovered {
struct DequeMapBinding;
enum class SessionMemoryPhase {Open,Retiring,Closed,Poisoned};
struct SessionMemorySnapshot {
    SessionMemoryPhase phase=SessionMemoryPhase::Open;
    std::size_t allocations=0,controls=0,proxies=0,blocks=0,maps=0,buffers=0;
    std::size_t reserved=0,pending_frees=0,partial_constructions=0;
    std::size_t successful_allocations=0,successful_frees=0,controls_destroyed=0;
    std::size_t provider_leases=0,buffer_borrows=0;
};
// New owned source composition, not recovered04b0bc/04b0f8 or native class ABI.
// A DEDICATED sole-owned ApplicationMemory retains the existing heap/PTD domain.
// LifecycleEffects must itself own its services/context. No original image VA
// becomes executable. The selected executor remains an explicit caller lower.
class SessionMemoryDomain final:public SessionAllocator,public SessionFreeEffects,public std::enable_shared_from_this<SessionMemoryDomain> {
    struct Impl;
public:
    // Hold one Operation for the ENTIRE recovered source call/chain, including
    // direct ReceiveBufferMemory::view uses. It serializes registry changes,
    // checks thread+fiber and captures a non-sentinel backing-domain PTD.
    // Allocation APIs, backing effects and scope exit recheck that identity;
    // mismatch retains/poisons, never restores or frees an unknown PTD.
    // It does not pin asynchronous buffer borrowers or establish raw-job drain.
    class Operation final {
    public:
        ~Operation() noexcept;
        Operation(const Operation&)=delete;Operation& operator=(const Operation&)=delete;
        Operation(Operation&&)=delete;Operation& operator=(Operation&&)=delete;
    private:
        friend class SessionMemoryDomain;
        explicit Operation(Impl&);
        Impl& state_;DWORD thread_;void* fiber_;
        std::unique_lock<std::recursive_mutex> lock_;
    };
    SessionMemoryDomain(std::shared_ptr<thread_domain::ApplicationMemory>&&,std::shared_ptr<LifecycleEffects>);
    // Dedicated controller-owned provider, admitted only on the capability's
    // actual Domain worker. Successful close RETURNS the borrow; it never
    // destroys the controller's ApplicationMemory/provider lease on this worker.
    SessionMemoryDomain(worker_handoff::PreparedMemory::Borrow&&,std::shared_ptr<LifecycleEffects>);
    // Successful close is mandatory before destruction, even for an empty
    // domain. Provider destructors may reenter while their refs are retired.
    ~SessionMemoryDomain() noexcept;
    SessionMemoryDomain(const SessionMemoryDomain&)=delete;SessionMemoryDomain& operator=(const SessionMemoryDomain&)=delete;
    Operation enter();
    SessionControl* allocate_control_04b0bc(std::uint64_t) override;
    DequeProxy* allocate_proxy_04b0bc(std::uint64_t) override;
    WriteFinishString* allocate_output_block();
    std::span<WriteFinishString*> allocate_output_map(std::uint64_t power_of_two_slots);
    ReceiveBufferContext& buffer_context();
    // Owner obtains a lease outside Operation (original worker for a prepared
    // borrow). Requires shared ownership;
    // keeps the C++ root alive and prevents close until dependent providers stop.
    std::shared_ptr<void> acquire_provider_lease();
    // Provider factory invoked by an already admitted source callback. Same
    // owning metadata lease, with existing thread/fiber/PTD admission required.
    std::shared_ptr<void> acquire_provider_lease_from_operation();
    // Whole-call Operation required. Pin one owned byte range until its SDK
    // borrower actually ends. Pin release is metadata-only and may cross threads.
    std::shared_ptr<void> pin_buffer(std::uintptr_t,std::uint64_t);
    bool owns_control(const IoReference*) const;
    bool owns_output_queue(const WriteFinishOwner*) const;
    // Read-only metadata for LIVE typed Map tickets, under the whole Operation.
    // Refreshed before backing free (including direct026880 disposal). The span
    // is borrowed only until the next allocation/free attempt; never cache it
    // across one. Binding metadata is const; pointer slots remain mutable.
    std::span<const DequeMapBinding> output_map_bindings();
    // Factory owns stable scratch on throw. Successful construction discards
    // only its already-unlinked temporary sidecar. A failed partial prefix is
    // retained and blocks close; native EH rollback is NOT guessed.
    SessionPair construct_session(SocketObject& pending,ExecutorMove);
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t native_bytes) override;
    [[noreturn]]void invalid_parameter_05c974() override;
    // Only RETIRED raw storage with confirmed live, quiescent backing ownership
    // may be retried (including a known failed HeapFree).
    // Never repeats a typed destructor or resumes a failed recovered algorithm.
    bool retry_pending_frees();
    // Owner operation (prepared borrow: original worker) outside Operation/
    // loader lock, after external stop
    // and async drain. False retains ownership and does not stop allocations.
    bool close();
    SessionMemorySnapshot snapshot() const;
    // Nonowning identity only; after close its former target may no longer live.
    const thread_domain::DomainStorage* memory_domain_identity() const noexcept;
private:
    std::unique_ptr<Impl> impl_;
};
} // namespace anadius::socket_recovered
