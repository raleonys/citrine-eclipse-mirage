#pragma once
#include "../anadius-callback-policy-source/policy.hpp"
#include <mutex>

namespace anadius::foreign_callback {
enum class Fault {Warming,Overlap,Reentry,Stopped};
class Rejected final:public std::logic_error {
public:explicit Rejected(Fault);const Fault fault;
};
// One callback body at a time; sequential different host threads may enter.
// Mutex is NOT held across PTD/body/original/provider/wait calls. Reject overlap
// and reentry rather than queueing. External ready proof must establish frozen
// host maps/originals/config/server port, safe Execution/cache/worker/fallback
// access, and exclusive stream/provider use for the entire admitted phase.
class Domain final {
public:
    Domain(std::shared_ptr<thread_domain::DomainStorage>,std::function<bool()> ready_proof);
    ~Domain() noexcept;
    Domain(const Domain&)=delete;Domain& operator=(const Domain&)=delete;
    bool activate_foreign(); // controller, after completed host setup
    bool pause(); // controller; permanently excludes new callbacks, false if active
    bool permits_current() const; // actual PTD probe, no Domain lock across SDK
    bool active() const;
    bool ready() const;
    const thread_domain::DomainStorage* memory_domain() const noexcept;
private:
    friend class Admission;
    void enter(native_recovered::Callback);
    void leave(native_recovered::Callback) noexcept;
    void begin();void complete();
    std::shared_ptr<thread_domain::DomainStorage> domain_;
    std::shared_ptr<thread_domain::CallbackThreads> threads_;
    std::function<bool()> ready_proof_;
    const DWORD controller_;
    mutable std::mutex gate_;
    bool setup_done_=false;
    mutable bool ready_=false;
    bool closing_=false,active_=false;
    mutable bool poison_=false;
    bool control_=false;
    DWORD thread_=0;void* fiber_=nullptr;native_recovered::Callback callback_{};
};
class Admission final:public native_recovered::Admission {
public:
    explicit Admission(std::shared_ptr<Domain>);
    void enter(native_recovered::Callback) override;
    void leave(native_recovered::Callback) noexcept override;
    void host_setup_begin() override;
    void host_setup_complete() override;
    bool pause_before_cleanup() override;
private:std::shared_ptr<Domain> domain_;
};
// Same memory domain required. Domain and permission owners outlive Policy and
// Context. Lifecycle remains B4 stop-sources -> admission drain -> joins -> exit
// cleanup -> independent external hook quiescence -> native close.
bootstrap_backend::CallbackAttachmentFactory make_factory(std::shared_ptr<Domain>,
    native_context::CallbackSeed,callback_policy::Resources,native_recovered::Errors& permanent_errors,
    native_context::Quiescence external_quiescence);
} // namespace anadius::foreign_callback
