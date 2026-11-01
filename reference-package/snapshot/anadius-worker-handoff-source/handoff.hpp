#pragma once
#include "../anadius-thread-domain-source/domain.hpp"

namespace anadius::worker_handoff {
enum class Phase {Prepared,Acquiring,Borrowed,Returned,Retiring,Closed,Poisoned};
struct Snapshot {
    Phase phase;DWORD controller_thread,worker_thread;
    bool worker_handle_retained,worker_exit_verified;
};
// Host composition, not an original native class or recovered bootstrap ABI.
// Prepare on the Domain controller; hold this root until explicit close/cancel.
// The sole ApplicationMemory OWNER never leaves the controller. Exactly one
// real same-Domain NativeThreads worker borrows it. No aliasing shared_ptr is
// used to bypass SessionMemoryDomain's legacy sole-owner constructor.
// The caller freezes shared application cells until worker retirement/join;
// source drain on the worker and OS thread/FLS exit are separate obligations.
class PreparedMemory final {
    struct State;
public:
    class Borrow final {
    public:
        Borrow(Borrow&&)noexcept;
        Borrow& operator=(Borrow&&)=delete;
        Borrow(const Borrow&)=delete;Borrow& operator=(const Borrow&)=delete;
        // An unreturned borrow poisons the retained ticket; no guessed cleanup.
        ~Borrow()noexcept;
        thread_domain::ApplicationMemory& memory()const;
        const thread_domain::DomainStorage* domain_identity()const noexcept;
        // Same worker/thread/fiber/PTD only, after every source owner drained.
        // False retains the borrow when known live allocations remain.
        bool retire();
    private:
        friend class PreparedMemory;
        explicit Borrow(std::shared_ptr<State>)noexcept;
        std::shared_ptr<State> state_;
    };
    static std::unique_ptr<PreparedMemory> prepare(std::shared_ptr<thread_domain::DomainStorage>);
    ~PreparedMemory()noexcept;
    PreparedMemory(const PreparedMemory&)=delete;PreparedMemory& operator=(const PreparedMemory&)=delete;
    Borrow acquire();
    // Controller, outside loader lock. Uses a duplicated ACTUAL worker handle;
    // a returned borrow or caller's boolean is not sufficient proof of exit.
    // Timeout/failed wait retains every owner. Unknown outcome forbids retry.
    bool close(DWORD timeout_ms=0);
    // Only before a worker has acquired the capability. No thread was launched
    // by this class; the caller still owns any independent launch/join tracker.
    bool cancel();
    Snapshot snapshot()const;
    const thread_domain::DomainStorage* domain_identity()const noexcept;
private:
    explicit PreparedMemory(std::shared_ptr<State>)noexcept;
    std::shared_ptr<State> state_;
};
} // namespace anadius::worker_handoff
