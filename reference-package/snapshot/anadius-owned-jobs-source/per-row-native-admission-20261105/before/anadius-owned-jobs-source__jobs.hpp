#pragma once
#include "../anadius-session-profile-source/profile.hpp"

namespace anadius::owned_jobs {
namespace sp=session_profile;namespace rt=server_runtime;namespace s=socket_recovered;
enum class Phase {Fresh,Bound,Closed,Poisoned,Retiring};
struct Snapshot {Phase phase;std::size_t pending;bool active;std::uint64_t started,completed,detached;};
// Source execution tickets, NOT Win32 HANDLE/thread IDs or CRT05e8e0/04a458.
// Supported policy: nonreentrant inline execution on the bound admitted worker.
// Profile retains its existing strong/tombstone policy; this backend adds none.
class InlineJobs final:public sp::Jobs {
public:
    explicit InlineJobs(std::size_t capacity=64);
    ~InlineJobs()noexcept override;
    InlineJobs(const InlineJobs&)=delete;InlineJobs& operator=(const InlineJobs&)=delete;
    void bind(rt::Ports,std::function<void()> nonmutating_admission);
    s::FrameThreadHandle start(sp::JobToken,std::function<void()> runner)override;
    std::int32_t detach(const s::FrameThreadHandle&)override;
    // Trusted external drain + actual graph emptiness; completed tickets alone
    // are not SDK/kernel/job finality. Whole Parent Operation is required.
    bool close(bool externally_drained);
    Snapshot snapshot()const;
private:struct Impl;std::unique_ptr<Impl> impl_;
};
struct NativeJobApi {
    decltype(&::CreateEventW) create_event=::CreateEventW;
    decltype(&::SetEvent) set_event=::SetEvent;
    decltype(&::DuplicateHandle) duplicate=::DuplicateHandle;
    decltype(&::WaitForSingleObject) wait=::WaitForSingleObject;
    decltype(&::GetExitCodeThread) exit_code=::GetExitCodeThread;
    decltype(&::CloseHandle) close=::CloseHandle;
    decltype(&::PostQueuedCompletionStatus) post=::PostQueuedCompletionStatus;
};
// Explicit selected HOST native scheduler; never the default InlineJobs route.
// Exact same-Domain NativeThreads, prepared Parent and native IOCP required.
// Caller retains provider/code/SDK backing until successful close, including
// all FLS exit. Native SDK callbacks must obey nonthrowing Win32 contracts.
// Owns all prepared message-job launches on its Parent; external manual child
// offers must not race its outside-Parent maintenance/lifecycle reservation.
// Allocator/new-handler/foreign shared-state safety remains caller's contract.
// Capacity is an initial allocation. Additional physical rows have stable
// addresses through native thread/FLS retirement; no fixed peer/job limit.
class NativeJobs final:public sp::Jobs,public sp::PreparedJobs {
public:
    explicit NativeJobs(std::shared_ptr<bootstrap_providers::NativeThreads>,std::size_t capacity=64,NativeJobApi={});
    ~NativeJobs()noexcept override;
    NativeJobs(const NativeJobs&)=delete;NativeJobs& operator=(const NativeJobs&)=delete;
    void bind_prepared(sp::Application&,rt::Ports,std::function<void()>)override;
    // Legacy runner is deliberately unsupported; Profile selects PreparedJobs.
    s::FrameThreadHandle start(sp::JobToken,std::function<void()>)override;
    s::FrameThreadHandle start_prepared(sp::Application&,sp::JobToken)override;
    std::int32_t detach(const s::FrameThreadHandle&)override;
    void returned_prepared(sp::Application&,std::uint32_t receipt)override;
    bool quiescent_prepared(sp::Application&)override; // outside Parent; wait0
    bool close_prepared(sp::Application&,bool externally_drained)override;
    Snapshot snapshot()const; // pending includes consumed but physically live rows
private:struct Impl;std::unique_ptr<Impl> impl_;Impl& attached()const;
};
} // namespace anadius::owned_jobs
