#pragma once
#include "strings.hpp"
#include "../anadius-server-runtime-source/runtime.hpp"
#include "../anadius-socket-source/response_containers.hpp"
#include "../anadius-socket-source/composed_write_native.hpp"
#include <functional>
#include <stdexcept>

namespace anadius::session_profile {
namespace rt=server_runtime;namespace o=owned_server;
struct SessionToken {std::size_t slot=0;std::uint64_t generation=0;bool operator==(const SessionToken&)const=default;};
struct JobToken {std::size_t slot=0;std::uint64_t generation=0;bool operator==(const JobToken&)const=default;};
struct JobIdentity {
    JobToken job;SessionToken session;
    std::uintptr_t closure=0,owner=0,function=0,control=0;
};
struct JobHandoffState;
struct SessionView {SessionToken token;s::SessionOwner& owner;};
class SessionLogic {
public:virtual ~SessionLogic()=default;
    virtual std::string greeting()=0;
    virtual std::string process(std::string& request)=0;
};
using LogicFactory=std::function<std::unique_ptr<SessionLogic>(SessionView)>;
class Application;
// Backend controls execution only, not frame/message context construction.
// Runner must be invoked exactly once on the bound worker thread/fiber under
// this parent's whole Operation. Inline or deferred same-worker execution is
// supported; native detached-thread scheduling/concurrency is not established.
// Start must not block waiting for another thread to acquire the parent lock.
class Jobs {
public:virtual ~Jobs()=default;
    virtual s::FrameThreadHandle start(JobToken,std::function<void()> runner)=0;
    virtual std::int32_t detach(const s::FrameThreadHandle&)=0;
};
// Independent opt-in HOST interface; legacy Jobs virtual slots stay unchanged.
// Original worker/whole Parent except quiescent_prepared, which polls actual
// thread exit OUTSIDE Parent. Returned receipts each transfer one source-work
// pin to the dispatcher; none of these counts attest external/kernel drain.
class PreparedJobs {
public:virtual ~PreparedJobs()=default;
    virtual void bind_prepared(Application&,rt::Ports,std::function<void()> admission)=0;
    virtual s::FrameThreadHandle start_prepared(Application&,JobToken)=0;
    virtual void returned_prepared(Application&,std::uint32_t receipt)=0;
    virtual bool quiescent_prepared(Application&)=0;
    virtual bool close_prepared(Application&,bool externally_drained)=0;
};
class Policy:public rt::WorkerControl,public s::FrameReadTerminal,public s::QueuedWriteTerminal {
public:
    [[noreturn]]virtual void bad_weak_ptr_026860()override=0;
    virtual s::DispatcherEffects& guards()=0;
    virtual s::WriteFinishApi& logging()=0; // only logging/terminal methods used
    // Legacy explicit-environment route only. Empty Environment selects the
    // Profile-owned provider and does not invoke these compatibility methods.
    virtual s::IoHandlerEnvironment session_environment(SessionView){throw std::logic_error("Legacy session environment provider is not selected");}
    virtual void accept_environment(s::IoHandlerEnvironment&,std::uintptr_t){throw std::logic_error("Legacy accept environment provider is not selected");}
    // Compatibility override only. Profile now supplies owned026670 growth.
    virtual void grow_deque(s::WriteFinishOwner&,std::uint64_t){throw std::logic_error("Legacy deque growth policy is not selected by Profile");}
    virtual void construct_length_error(s::ReadLengthError&,std::uintptr_t)=0;
    [[noreturn]]virtual void throw_length_error(s::ReadLengthError&)=0;
    [[noreturn]]virtual void thread_error(int)=0;
    virtual void cleanup_exit_waits()=0;
    [[noreturn]]virtual void raise_write(s::Error&,std::uintptr_t)=0;
    virtual s::IocpSchedulerEntry* completion_owner(Application&,s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)=0;
    virtual void after_completion(Application&)=0;
    virtual void worker_returned(Application&,std::uint64_t,const s::WorkerRunResult&)=0;
    // Outside parent Operation: join/drain jobs/kernel before true. This remains
    // a trusted external proof; no counter, owner0 packet or cancel intent is it.
    virtual bool external_quiescent(o::ServerView,Application&)=0;
};
struct CategoryProvider {s::Category key;std::shared_ptr<s::WriteFinishCategory> object;};
struct Environment {
    s::IoHandlerSentinels sentinels;
    std::shared_ptr<s::IoEnvironmentTypeApi> types;
    std::vector<s::IoEnvironmentTypeBinding> queries;
    std::vector<s::IoEnvironmentFactoryBinding> factories;
    std::vector<s::HandlerScheduleBinding> handlers;
    std::vector<s::ResponseExecutorBinding> response_executors;
    std::vector<s::ResponseScheduleBinding> responses;
    std::vector<s::MessageEnvironmentCopyBinding> copies;
    std::vector<s::AcceptScheduleBinding> accepts;
};
struct Dependencies {
    std::shared_ptr<void> state_lifetime;
    std::shared_ptr<s::Api> errors;std::shared_ptr<s::AsyncReceiveApi> receive;
    std::shared_ptr<s::AsyncSendApi> send;std::shared_ptr<Policy> policy;
    std::shared_ptr<Jobs> jobs;LogicFactory logic;
    Environment environment;std::vector<CategoryProvider> categories;
    s::Category success{},misc{};
    // Host registry counts are initial storage, not live session/job/header limits.
    std::size_t sessions=16,jobs_capacity=64,headers=256,initial_output_slots=8;
    std::size_t environment_capacity=4096;
    // Only the automatically supplied provider's initial Slot-source classifier;
    // custom environments and tracked native executor families stay explicit.
    bool original_default_environment=false;
    // Used only when the corresponding receive/send provider is empty.
    s::AsyncReceiveImports receive_imports{};s::AsyncSendImports send_imports{};
};
enum class Phase {Fresh,Bound,Open,Closing,Closed,Poisoned,Retiring};
struct Snapshot {Phase phase;std::size_t live_sessions,live_jobs,active_calls;};
struct RegistryView {
    std::span<const s::FrameReadOwnerBinding> frames;
    std::span<const s::ReceiveStringBinding> strings;
    std::span<const s::ReceiveStreamBinding> reads;
    std::span<const s::SendStreamBinding> writes;
    std::span<const s::WriteFinishOwnerBinding> finish;
    std::span<const s::QueuedWriteOwnerBinding> queued;
    std::span<const s::MessageDispatchOwnerBinding> messages;
    std::span<const s::IoEnvironmentSourceBinding> environments;
};
class Profile;
// Owns contexts and session binding arrays that can grow without moving live
// session records. Session records originate only from actual023000 owners in
// greeting. Other selected resource limits and failure-prefix rules remain.
class Application final:public rt::Application,public rt::PreparedJobNotifications,public rt::PreparedJobMaintenance,public rt::PreparedJobSourceDrain {
public:
    class JobHandoff;
    ~Application()noexcept;
    rt::Bindings bindings()override;
    void construct_accept_environment(s::IoHandlerEnvironment&,std::uintptr_t)override;
    void reset_pending_owner(s::AcceptPending&)override;
    s::ReceiveBufferString& greeting(s::SessionOwner&,s::ReceiveBufferString&)override;
    [[noreturn]]void raise_write(s::Error&,std::uintptr_t)override;
    s::IocpSchedulerEntry* completion_owner(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override;
    void after_completion()override;
    void returned_job(std::uint32_t receipt)override;
    void maintain_jobs()override;
    bool pending_prepared_jobs()override;
    void worker_returned(std::uint64_t,const s::WorkerRunResult&)override;
    void reap(); // existing Operation; clear bindings/logic BEFORE observer drop
    Snapshot snapshot()const;
    // Borrowed metadata snapshots on the bound worker, including native free
    // callbacks. Reacquire after greeting can grow the registry; copied spans
    // do not follow relocation or permit dereferencing retired owner/control.
    RegistryView registry()const;
    rt::Ports ports()const;
    SessionView session(SessionToken)const;
    void run_job(JobToken); // narrow runner used by Jobs; never an image VA
    // Original worker + whole Parent Operation. Binds a pending/submitting
    // source job to its exact closure/session and prepared Parent memory offer.
    // Nonexecuting handoff: no native message provider becomes eligible here.
    JobHandoff prepare_job(JobToken);
    // Cancel unacquired offer, or release an UNPROCESSED child-returned offer,
    // after exact identity revalidation. Restores legacy runner eligibility.
    void release_job(JobHandoff&);
    // Original worker + whole Parent Operation, after return_memory. Consume
    // a prepared child result through existing response/021770 closure cleanup.
    // after_completion also consumes ready returned results before Policy's
    // opaque lifecycle hook. Neither route attests physical child/thread exit
    // or runs033d70 again; a scheduler must notify the original IOCP worker.
    void complete_job(JobHandoff&);
private:
    friend class Profile;struct Impl;std::unique_ptr<Impl> impl_;
    explicit Application(std::unique_ptr<Impl>);
};
class Application::JobHandoff final {
public:
    JobHandoff(JobHandoff&&)noexcept;
    JobHandoff& operator=(JobHandoff&&)=delete;
    JobHandoff(const JobHandoff&)=delete;JobHandoff& operator=(const JobHandoff&)=delete;
    ~JobHandoff()noexcept; // unresolved claim poisons/retains source and backing
    JobIdentity identity()const; // numeric snapshot only, no live access proof
    void acquire(); // distinct actual same-Domain job worker
    JobIdentity inspect(); // exact pinned Parent Operation; no message execution
    // Actual existing026670 on this job's exact session queue under a live
    // child Parent permit. This grows capacity only; no message/body dispatch.
    void grow_output(std::uint64_t extra);
    // Actual023dd0 owned Environment clone/query/temporary destruction only.
    // No handler/response scheduling or message Logic/Policy/Graph execution.
    void clone_environment();
    // Selected concrete Policy's existing04a47c for the current child's waits.
    // No message execution, closure release, job completion or exit proof.
    void cleanup_exit_waits();
    // Exact child Parent Operation -> optional JobLogic033d70 once + current
    // child's04a47c, retaining response + mutated request in this claim. No child
    // scheduling/closure cleanup. Return memory then notify the original worker
    // for complete_job/after_completion; release_job cannot restore its runner.
    void process_message();
    void return_memory(); // child, only after its inspections/source access end
private:
    friend class Application;
    explicit JobHandoff(std::shared_ptr<JobHandoffState>)noexcept;
    std::shared_ptr<JobHandoffState> state_;
};
class Profile final:public rt::Profile {
public:
    explicit Profile(Dependencies);
    ~Profile()noexcept;
    Profile(const Profile&)=delete;Profile& operator=(const Profile&)=delete;
    rt::Environment environment(o::ServerView,std::uintptr_t)override;
    std::unique_ptr<rt::Application> create(rt::Ports)override;
    bool external_quiescent(o::ServerView,rt::Application&)override;
    Snapshot snapshot()const;
private:friend class Application;struct State;std::shared_ptr<State> state_;
};
} // namespace anadius::session_profile
