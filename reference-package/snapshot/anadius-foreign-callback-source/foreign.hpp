#pragma once
#include "../anadius-callback-policy-source/policy.hpp"
#include <mutex>
#include <optional>

namespace anadius::foreign_callback {
using HookPublicationProof=std::function<bool(native_recovered::Callback)>;
// The optional OriginalCrt publication profile replaces the blanket ready
// proof with an actual per-hook successful-create receipt. Metadata is checked
// before PTD/body entry; source callback bodies retain their own native waits.
// General bodies retain single-active/reentry/PTD bounds. Original-profile
// connect uses counted per-actor phases: untouched->original; web compares the
// live shared port immediately; non-web including3216->actual wait, then
// decides redirect/original. Redirect effects require the same
// managed stream and atomic counter access; no general registry/allocator or
// private recovered CRT permission is granted and no private PTD is created.
// Complete may observe host setup while an admitted body waits.
// Six argument-only forwarding wrappers may nest on any admitted actor phase.
// Their automatic linked frames mask all parent permissions and grant only
// original SDK calls; no private callback PTD or shared registry state is used.
// The Original profile also admits same-thread general registry descendants
// only inside the current general registry body's typed original SDK call.
// Automatic linked SDK/body tokens keep one root PTD and serialize child bodies.
enum class Fault {Warming,Overlap,Reentry,Stopped};
class Rejected final:public std::logic_error {
public:explicit Rejected(Fault);const Fault fault;
};
// One general callback body at a time; sequential different host threads may enter.
// Mutex is NOT held across PTD/body/original/provider/wait calls. Reject overlap
// and reentry rather than queueing, except for the Original connect lane and
// counted pure-forward frames described above.
// In Cooperative, external ready proof establishes frozen
// host maps/originals/config/server port, safe Execution/cache/worker/fallback
// access, and exclusive stream/provider use for the entire admitted phase.
// With optional connect_original_proof, warming connect first owns only a PTD
// probe. Immutable length/family classification may admit an original SDK call;
// general Policy effects remain denied. Overlap/reentry/stop rules still apply.
// The following staged proof options apply only to Cooperative. Optional web
// proof certifies an immutable published bound port DIFFERENT
// from requested80/443. It may grant only the same original-only permission.
// A separate no-redirect proof may grant a FOREIGN-only wait scope for other
// ports excluding3216. Wait retains PTD/native/code owners; it neither cancels
// on pause nor grants general Policy effects. Original proof is rechecked after.
// Optional redirect snapshot additionally permits foreign3216/matching-port
// bodies with a managed log and private callback counter. Non-web paths still
// wait first; body permission excludes original SDK and general Policy calls.
class Domain final {
public:
    // Optional original-only proof must retain the exact Policy original owner.
    // It certifies successful connect create/code lifetime, not shared state.
    Domain(std::shared_ptr<thread_domain::DomainStorage>,std::function<bool()> ready_proof,
           std::function<bool()> connect_original_proof={},std::function<bool(std::uint16_t)> web_no_redirect_proof={},
           std::function<bool(std::uint16_t)> other_no_redirect_proof={},
           std::function<std::optional<std::uint16_t>()> redirect_port_proof={},HookPublicationProof={});
    ~Domain() noexcept;
    Domain(const Domain&)=delete;Domain& operator=(const Domain&)=delete;
    bool activate_foreign(); // controller, after completed host setup and drain
    bool pause(); // controller; permanently excludes new callbacks, false if active
    bool permits_current() const; // actual PTD probe, no Domain lock across SDK
    bool permits_original_current() const;
    bool permits_connect_wait_current() const;
    bool permits_connect_body_current() const;
    bool requires_managed_log()const noexcept;
    bool active() const;
    bool ready() const;
    const thread_domain::DomainStorage* memory_domain() const noexcept;
    // Original profile only: controller binds the exact stable Context cell
    // once, passively before Context publication. This does not read its value
    // or construct a persistent atomic_ref; startup parsing can still write it.
    void bind_connect_server_port(std::uint16_t&);
private:
    friend class Admission;
    void enter(native_recovered::Callback);
    void leave(native_recovered::Callback) noexcept;
    bool try_enter_original_forward(native_recovered::Callback,native_recovered::OriginalForwardFrame&);
    void leave_original_forward(native_recovered::OriginalForwardFrame&) noexcept;
    bool begin_registry_sdk(native_recovered::Callback,native_recovered::RegistrySdkFrame&);
    void end_registry_sdk(native_recovered::RegistrySdkFrame&) noexcept;
    bool try_enter_registry_nested(native_recovered::Callback,native_recovered::RegistryNestedFrame&);
    void leave_registry_nested(native_recovered::RegistryNestedFrame&) noexcept;
    bool try_enter_connect_original(std::uint16_t& exact_server_port,std::optional<std::uint16_t> port,bool wait_for_steps);
    bool connect_original_wait_pending()const;
    void complete_connect_original_wait();
    bool connect_original_redirect_pending()const;
    void complete_connect_original_redirect();
    void begin();void complete();
    bool scope(bool probe_allowed,bool original_allowed,bool wait_allowed=false,bool body_allowed=false) const;
    bool connect_probe() const;
    void connect_classified(bool);
    bool connect_web_passthrough(std::uint16_t);
    bool original_route(const std::uint16_t* requested,bool waiting=false);
    bool connect_wait_begin(std::uint16_t);
    void connect_wait_complete();
    bool connect_redirect_begin(std::uint16_t);
    std::uint16_t redirect_checkpoint(bool completed);
    std::shared_ptr<thread_domain::DomainStorage> domain_;
    std::shared_ptr<thread_domain::CallbackThreads> threads_;
    std::function<bool()> ready_proof_;
    std::function<bool()> connect_original_proof_;
    std::function<bool(std::uint16_t)> web_no_redirect_proof_;
    std::function<bool(std::uint16_t)> other_no_redirect_proof_;
    // Cooperative redirect snapshot only. Original profile instead borrows
    // the stable actual configuration cell, bound before Context publication.
    std::function<std::optional<std::uint16_t>()> redirect_port_proof_;
    std::uint16_t* connect_server_port_=nullptr;
    // Original profile checks actual successful-create publication for each
    // hook. Native callback bodies retain their own waits; no all-ready proof.
    HookPublicationProof hook_publication_proof_;
    const DWORD controller_;
    mutable std::mutex gate_;
    bool setup_done_=false;
    mutable bool ready_=false;
    bool closing_=false,active_=false;
    std::size_t original_connect_calls_=0;
    std::size_t original_forward_calls_=0;
    mutable bool poison_=false;
    bool control_=false;
    enum class Mode {Full,ConnectProbe,Checking,Waiting,RedirectPending,RedirectWaiting,RedirectBody,OriginalOnly};
    Mode mode_=Mode::Full;
    std::uint16_t waiting_port_=0;
    std::uint16_t redirect_server_port_=0;
    DWORD thread_=0;void* fiber_=nullptr;native_recovered::Callback callback_{};
};
class Admission final:public native_recovered::Admission {
public:
    explicit Admission(std::shared_ptr<Domain>);
    void enter(native_recovered::Callback) override;
    void leave(native_recovered::Callback) noexcept override;
    bool try_enter_original_forward(native_recovered::Callback,native_recovered::OriginalForwardFrame&) override;
    void leave_original_forward(native_recovered::OriginalForwardFrame&) noexcept override;
    bool begin_registry_sdk(native_recovered::Callback,native_recovered::RegistrySdkFrame&) override;
    void end_registry_sdk(native_recovered::RegistrySdkFrame&) noexcept override;
    bool try_enter_registry_nested(native_recovered::Callback,native_recovered::RegistryNestedFrame&) override;
    void leave_registry_nested(native_recovered::RegistryNestedFrame&) noexcept override;
    bool has_connect_original_lane()const noexcept override;
    bool try_enter_connect_original(std::uint16_t& exact_server_port,std::optional<std::uint16_t> port={},bool wait_for_steps=false)override;
    bool connect_original_wait_pending()const override;
    void complete_connect_original_wait()override;
    bool connect_original_redirect_pending()const override;
    void complete_connect_original_redirect()override;
    void host_setup_begin() override;
    void host_setup_complete() override;
    bool pause_before_cleanup() override;
    bool connect_probe() const override;
    void connect_classified(bool) override;
    bool connect_web_passthrough(std::uint16_t) override;
    bool connect_wait_begin(std::uint16_t) override;
    void connect_wait_complete() override;
    bool connect_redirect_begin(std::uint16_t) override;
    std::uint16_t connect_redirect_ready() override;
    void connect_redirect_complete() override;
private:std::shared_ptr<Domain> domain_;
};
// Same memory domain required. Domain and permission owners outlive Policy and
// Context. Lifecycle remains B4 stop-sources -> admission drain -> joins -> exit
// cleanup -> independent external hook quiescence -> native close.
bootstrap_backend::CallbackAttachmentFactory make_factory(std::shared_ptr<Domain>,
    native_context::CallbackSeed,callback_policy::Resources,native_recovered::Errors& permanent_errors,
    native_context::Quiescence external_quiescence,native_context::ClosureProfile=native_context::ClosureProfile::Cooperative);
} // namespace anadius::foreign_callback
