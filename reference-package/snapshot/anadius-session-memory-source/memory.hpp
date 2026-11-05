#pragma once
#include "../anadius-session-dispose-source/dispose.hpp"
#include "../anadius-thread-domain-source/domain.hpp"
#include "../anadius-worker-handoff-source/handoff.hpp"
#include <mutex>

namespace anadius::socket_recovered {
struct DequeMapBinding;
class DequeMapResolver;
// Explicit composition policy. The default retains historical HOST whole-call
// serialization. Concurrent actors are a lower foundation, not permission to
// enable an otherwise single-invocation Profile/Graph/provider composition.
enum class OperationPolicy {Serialized,OriginalConcurrentActors};
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
    struct ActorState;
public:
    class JobPermit;
    // Hold one Operation for the ENTIRE recovered source call/chain, including
    // direct ReceiveBufferMemory::view uses. Explicit entry alone admits an
    // exact thread/fiber/captured-PTD record. Serialized holds its separate HOST
    // body gate; OriginalConcurrentActors holds only brief metadata gates.
    // Allocation APIs, backing effects and scope exit recheck that identity;
    // mismatch retains/poisons, never restores or frees an unknown PTD.
    // It does not pin asynchronous buffer borrowers or establish raw-job drain.
    class Operation final {
    public:
        ~Operation() noexcept;
        // Copyable proof of THIS live explicit job Operation. Copies retain
        // metadata, never extend permission beyond Operation destruction.
        JobPermit job_permit();
        Operation(const Operation&)=delete;Operation& operator=(const Operation&)=delete;
        Operation(Operation&&)=delete;Operation& operator=(Operation&&)=delete;
    private:
        friend class SessionMemoryDomain;
        explicit Operation(Impl&);
        Operation(Impl&,worker_handoff::PreparedMemory::JobMemory*);
        Impl& state_;
        std::unique_lock<std::recursive_mutex> serial_lock_;
        std::shared_ptr<ActorState> actor_;
    };
    class JobPermit final {
    public:
        JobPermit(const JobPermit&)=default;JobPermit& operator=(const JobPermit&)=default;
        JobPermit(JobPermit&&)noexcept=default;JobPermit& operator=(JobPermit&&)noexcept=default;
        // Exact Parent + original job actor; stale/cross-root proofs reject.
        // Not permission to close providers, dispatch messages or infer drain.
        void check(const SessionMemoryDomain&)const;
    private:
        friend class Operation;
        JobPermit(std::shared_ptr<SessionMemoryDomain>,std::shared_ptr<ActorState>)noexcept;
        std::shared_ptr<SessionMemoryDomain> parent_;
        std::shared_ptr<ActorState> seal_;
    };
    SessionMemoryDomain(std::shared_ptr<thread_domain::ApplicationMemory>&&,std::shared_ptr<LifecycleEffects>,OperationPolicy=OperationPolicy::Serialized);
    // Dedicated controller-owned provider, admitted only on the capability's
    // actual Domain worker. Successful close RETURNS the borrow; it never
    // destroys the controller's ApplicationMemory/provider lease on this worker.
    SessionMemoryDomain(worker_handoff::PreparedMemory::Borrow&&,std::shared_ptr<LifecycleEffects>,OperationPolicy=OperationPolicy::Serialized);
    // Successful close is mandatory before destruction, even for an empty
    // domain. Provider destructors may reenter while their refs are retired.
    ~SessionMemoryDomain() noexcept;
    SessionMemoryDomain(const SessionMemoryDomain&)=delete;SessionMemoryDomain& operator=(const SessionMemoryDomain&)=delete;
    Operation enter();
    // Explicit exact-backing child admission. Holds a permission pin for the
    // WHOLE Operation; logical JobMemory return is false until scope exit.
    // Does not grant Profile/Graph/Policy access or concurrent effect safety.
    Operation enter(worker_handoff::PreparedMemory::JobMemory&);
    // Original worker + whole normal Operation. Borrowed backing only.
    worker_handoff::PreparedMemory::JobMemory prepare_job();
    // Original worker outside its own Operation. Concurrent mode may reap
    // Returned/Canceled siblings while other admitted actors remain active.
    // Serialized mode tries its existing Operation gate and returns zero if a
    // foreign body owns it; validates original capability before that return.
    // No metadata gate spans OS wait/close. Not a source-drain proof.
    std::size_t reap_jobs();
    SessionControl* allocate_control_04b0bc(std::uint64_t) override;
    DequeProxy* allocate_proxy_04b0bc(std::uint64_t) override;
    WriteFinishString* allocate_output_block();
    std::span<WriteFinishString*> allocate_output_map(std::uint64_t power_of_two_slots);
    ReceiveBufferContext& buffer_context();
    // Authoritative exact live-ticket lookups. Concurrent consumers use these
    // instead of borrowed union catalogs; each found ticket is pinned only by
    // the explicitly admitted actor that requested it. Own explicit free drops
    // that actor's old views; other actor/SDK borrowers still prohibit free.
    AllocationHeaderResolver& allocation_header_resolver();
    DequeMapResolver& output_map_resolver();
    OperationPolicy operation_policy()const noexcept;
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
    // Explicit ownership handoff after ALL current-actor spans/aliases of the
    // matched allocation have ended. View pins are deduplicated per ticket,
    // not per range: this drops this actor's FULL ticket view only. It creates
    // no new borrow and never releases SDK or other actors' pins. The caller
    // must not use an old span after handing ownership to an eligible actor.
    void relinquish_operation_view(std::uintptr_t,std::uint64_t);
    bool owns_control(const IoReference*) const;
    bool owns_output_queue(const WriteFinishOwner*) const;
    // Exact typed array ownership observation only; no catalog/view pin and no
    // prefix/subwindow match. Callers retain their actual queue ownership.
    bool owns_output_map(std::span<WriteFinishString*>) const;
    // Serialized-only legacy metadata for LIVE typed Map tickets.
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
    // Metadata rejection guard for outside-Parent maintenance, even when PTD/
    // fiber admission is poisoned. True is NOT permission for source access.
    bool operation_on_this_thread()const;
    // Nonowning identity only; after close its former target may no longer live.
    const thread_domain::DomainStorage* memory_domain_identity() const noexcept;
private:
    std::unique_ptr<Impl> impl_;
};
} // namespace anadius::socket_recovered
