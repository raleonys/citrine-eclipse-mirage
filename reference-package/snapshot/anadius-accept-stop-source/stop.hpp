#pragma once
#include "../anadius-accept-source/owned.hpp"
#include "../anadius-owned-server-source/server.hpp"

namespace anadius::accept_stop {
namespace s=socket_recovered;namespace o=owned_server;
enum class Scope {AcceptOnly,SessionIo};
struct Imports {
    decltype(&::CancelIoEx) cancel=&::CancelIoEx;
    decltype(&::PostQueuedCompletionStatus) post=&::PostQueuedCompletionStatus;
    decltype(&::GetLastError) error=&::GetLastError;
};
struct Options {
    std::shared_ptr<void> lifetime;
    // Controller's independent proof, thread-safe and never borrowing worker
    // Parent/raw views. AcceptOnly requires receive/send/session work drained;
    // SessionIo has the narrower external prerequisite described below. A
    // normal tracked route overlapping this callback invalidates it for retry.
    std::function<bool()> other_work_drained;
    Imports imports;std::size_t capacity=64;DWORD post_drain_timeout_ms=10000;
    // SessionIo also cancels tracked receive/send and discards owned Posted
    // callbacks. Its independent proof must stop NEW application/foreign/rawjob
    // admission and drain raw/deferred jobs; existing tracked IO/session owners
    // may still be live and are retired by observed source completions.
    Scope scope=Scope::AcceptOnly;
};
enum class Phase {Fresh,Bound,Running,Requested,Draining,Closing,Closed,Poisoned,Retiring};
struct Snapshot {
    Phase phase;std::size_t cancel_calls,not_found,kernel_packets,software_packets,acknowledged,receipts,active_posts;
};
struct Receipt {std::uint64_t serial=0;bool drop_owner=false;};
// Selected HOST cancellation/source-retirement policy; default is accept-only.
// Controller posts only a NULL-OVERLAPPED wake. CancelIoEx runs on the ORIGINAL
// admitted worker, after SDK submission returns, so no controller touches an
// OV/socket or races source packet cleanup. Native imports must be nonthrowing,
// nonblocking and must not invoke source callbacks inline.
// TRUE/FALSE/ERROR_NOT_FOUND is NEVER kernel finality. Actual physical packets,
// exact generation+submission and successful source routing are required.
// The selected stop policy drops a tracked observed SUCCESS as well as995;
// it never fabricates a terminal packet or decrements scheduler work.
class Stop final:public std::enable_shared_from_this<Stop> {
public:
    static std::shared_ptr<Stop> create(Options);
    ~Stop()noexcept;
    Stop(const Stop&)=delete;Stop& operator=(const Stop&)=delete;
    bool request(); // original controller/fiber; false before actual Runtime.run
    Snapshot snapshot()const;
    // Runtime-only protocol: source bind/pump/packet methods need the existing
    // WHOLE Parent Operation on their original worker/fiber/PTD.
    void bind(o::ServerView,s::OwnedAcceptMemory&);
    void running();
    void pump();
    bool physical(std::int32_t received,std::uint32_t bytes,std::uintptr_t key,OVERLAPPED*,std::uint32_t status);
    Receipt before(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t);
    void after(Receipt); // no old operation dereference after source completion
    bool close_worker(); // outside Parent, after independent external proof
private:
    struct Impl;explicit Stop(std::unique_ptr<Impl>)noexcept;
    std::unique_ptr<Impl> impl_;
};
} // namespace anadius::accept_stop
