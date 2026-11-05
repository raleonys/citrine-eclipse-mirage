#pragma once
#include "accept.hpp"
#include "../anadius-io-memory-source/io_memory.hpp"
#include <vector>

namespace anadius::socket_recovered {
struct AcceptHandle {std::size_t slot=0;std::uint64_t generation=0;bool operator==(const AcceptHandle&)const=default;};
enum class AcceptSubmission {NotSubmitted,Issuing,Kernel,Software,Unknown};
struct AcceptLoan {
    AcceptHandle handle;std::uint64_t submission;
    AcceptSubmission state;Socket listener;OVERLAPPED* overlapped;CancelOperation* operation;
    bool pending,consumed;
};
struct AcceptMemorySnapshot {std::size_t projections=0,pending=0,active_calls=0;bool closed=false,poisoned=false;};
// Composition only. Construct under existing parent Operation; holds a scoped
// provider lease so B35 cannot close context before this provider drains/closes.
// Base IO owner, lower helpers and bound context outlive this adapter. All source
// calls are serialized by parent Operation; genuine packet finality remains a
// caller obligation. No SessionControl is invented for a pending accept.
// A checked inert Posted projection reserves the base allocation/generation;
// its packet is replaced with a distinct accept operation, never a receive/send.
// capacity is the positive starting row count (at most 65536), not a live limit.
// Appended rows preserve operation/Task addresses; base packet-table growth
// repairs the registered packet field before releasing its old table.
// Route accept packets through AcceptDispatcherEffects before the base router.
class OwnedAcceptMemory final:public PostedMemoryEffects,public AsyncAcceptEffects {
public:
    OwnedAcceptMemory(std::shared_ptr<SessionMemoryDomain>,OwnedIoMemory&,std::size_t capacity,AcceptLower&,AsyncAcceptApi&);
    ~OwnedAcceptMemory()noexcept;
    OwnedAcceptMemory(const OwnedAcceptMemory&)=delete;OwnedAcceptMemory& operator=(const OwnedAcceptMemory&)=delete;
    void bind_context(AsyncAcceptContext&);
    AsyncAcceptApi& pinned_api();
    // Retry entry has already armed the same row. Validate its live pin,
    // authoritative address range and packet immediately before raw SDK call.
    AsyncAcceptApi& prearmed_api();
    void start(AsyncAcceptService&,AcceptorState&,AcceptPending&,AcceptEndpoint*,const std::uintptr_t&,std::uintptr_t);
    AcceptHandle handle_for(const OVERLAPPED*)const;
    // Whole Parent Operation, original worker only. Copies metadata, never a
    // controller-side raw borrow. Submission sequence also distinguishes native
    // retry on the SAME storage generation. TRUE/997 and software error paths
    // are recorded separately; none of these fields establishes finality.
    AcceptLoan loan(AcceptHandle)const;
    std::vector<AcceptLoan> pending_loans()const;
    bool contains(const CancelOperation*)const;
    void complete(AcceptHandle,std::uintptr_t owner,const Error&);
    void note_cancel_requested(AcceptHandle); // intent never drops a kernel loan
    void free_cached_identity(std::uintptr_t);
    std::span<OperationStorageBinding> storage_bindings(); // borrowed until base growth
    std::span<DispatcherOperationBinding> dispatcher_bindings(); // same lifetime
    AcceptMemorySnapshot snapshot()const;
    bool close(); // all base IO/cache work and provider calls must be drained
    OperationStorage& allocate_04b0bc(std::uint64_t n)override{return allocate(n);}
    void free_04b788(OperationStorage& s)override{release(s);}
    PostedOperation& project_posted(OperationStorage&)override;
    // Same parent Operation; retire only this adapter's sidecar before another
    // checked base projection. Valid also for native41-byte handler storage.
    void retire_for_projection(OperationStorage&);
    AsyncAcceptOperation& project(OperationStorage&)override;
    AcceptTask& project_task(OperationStorage&)override;
    void construct_environment(IoHandlerEnvironment&,std::uintptr_t)override;
    Socket create_socket(std::uint32_t,std::int32_t,std::int32_t,Error&)override;
    void reset_owned_socket(Socket&)override;
    void reset_pending_owner(AcceptPending&)override;
    Category misc_category()override;
    Category success_category()override;
    void retry(AsyncAcceptOperation&,CancelOperation&)override;
    void invoke(AcceptHandler&)override;
private:
    OperationStorage& allocate(std::uint64_t);void release(OperationStorage&);
    struct Impl;std::unique_ptr<Impl> impl_;
};
// Composes accept routing with an existing owned IO/posted router. The caller's
// B35 admitted API must cover full027c10 lookup/queued/complete, not just this
// callback. Guard/worker/CRT policies are forwarded to the selected lower.
class AcceptDispatcherEffects final:public DispatcherEffects {
public:AcceptDispatcherEffects(OwnedAcceptMemory& m,DispatcherEffects& lower):memory_(m),lower_(lower){}
    std::uintptr_t capture_tls_slot()override{return lower_.capture_tls_slot();}
    std::int32_t thread_epoch(std::uintptr_t p)override{return lower_.thread_epoch(p);}
    void guard_header_04b838(std::int32_t& n)override{lower_.guard_header_04b838(n);}
    void register_atexit_04b770(std::uintptr_t n)override{lower_.register_atexit_04b770(n);}
    void guard_footer_04b7cc(std::int32_t& n)override{lower_.guard_footer_04b7cc(n);}
    std::uint32_t read_stop_34(IocpSchedulerEntry& e)override{return lower_.read_stop_34(e);}
    std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>& d,const std::array<std::uintptr_t,2>& s)override{return lower_.copy_exception_024100(d,s);}
    [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>& s)override{lower_.rethrow_029880(s);}
    void complete(IocpSchedulerEntry*,CancelOperation&,const Error&,std::uint32_t)override;
private:OwnedAcceptMemory& memory_;DispatcherEffects& lower_;
};
} // namespace anadius::socket_recovered
