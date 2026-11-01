#pragma once
#include "../anadius-session-profile-source/profile.hpp"

namespace anadius::owned_jobs {
namespace sp=session_profile;namespace rt=server_runtime;namespace s=socket_recovered;
enum class Phase {Fresh,Bound,Closed,Poisoned};
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
} // namespace anadius::owned_jobs
