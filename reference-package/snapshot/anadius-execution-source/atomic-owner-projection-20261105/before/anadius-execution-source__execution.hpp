#pragma once
#include <cstddef>
#include <cstdint>
#include <mutex>

namespace anadius::execution_recovered {
// Recovered MSVC mutex layout; this is not std::mutex ABI from this compiler.
struct alignas(8) MutexState {
    std::uint32_t type;
    std::uint32_t reserved_04;
    std::uintptr_t storage_08;
    std::uintptr_t srw_10;
    std::byte reserved_18[0x30];
    std::uint32_t owner_48;
    std::uint32_t count_4c;
};
static_assert(offsetof(MutexState, srw_10) == 0x10);
static_assert(offsetof(MutexState, owner_48) == 0x48);
static_assert(offsetof(MutexState, count_4c) == 0x4c);
static_assert(sizeof(MutexState) == 0x50);

// Required dependencies are exclusively OS/CRT boundaries. Application state,
// predicate, lock bookkeeping, QPC conversion and polling remain in execution.cpp.
struct Platform {
    virtual ~Platform() = default;
    virtual std::uint32_t current_thread_id() = 0;
    virtual void acquire_srw_exclusive(std::uintptr_t& lock) = 0;
    virtual void release_srw_exclusive(std::uintptr_t& lock) = 0;
    virtual std::int64_t query_performance_frequency() = 0;
    virtual std::int64_t query_performance_counter() = 0;
    virtual void sleep_milliseconds(std::uint32_t milliseconds) = 0;
    // Preserve original error-table index; original MSVC exception ABI is not
    // silently replaced with an arbitrary std::system_error mapping.
    [[noreturn]] virtual void raise_thread_error_04a648(int index) = 0;
};

// 04a698 writes only these recovered fields, leaving unrelated bytes untouched.
void initialize_mutex_04a698(MutexState&, std::uint32_t observed_type);
int lock_04a6ac(MutexState&, Platform&); // precisely the nullptr-deadline path
int unlock_04a6b4(MutexState&, Platform&);

struct FlagReferences {
    std::uint8_t& first_0a3c3d;
    std::uint8_t& second_0a3c3c;
    std::uint8_t& third_0a3c3e;
};
enum class ReadyStage {Server,Online,Libraries,Host};
// Optional SOURCE milestone observer. bind occurs once before publication;
// published is nonthrowing bookkeeping only, not controller activation.
class Publication {
public:
    virtual ~Publication()=default;
    virtual void bind(const void* execution_identity,FlagReferences)=0;
    virtual void published(ReadyStage,const void* execution_identity) noexcept=0;
    // Optional SOURCE payload before the real Server finish event. It does
    // not replace the event or permit raw concurrent port-cell reads.
    virtual void server_port_published(std::uint16_t,const void*) noexcept {}
};

// Neither construction nor any method invents/reset flags, mutex type or QPC
// cache state. Caller must supply their already-observed initial state.
class Execution {
public:
    Execution(MutexState& mutex_0a3c40, FlagReferences flags,
              std::int64_t& frequency_cache_0a45e0, Platform& platform,Publication* publication=nullptr)
        : mutex_(mutex_0a3c40), flags_(flags), frequency_cache_(frequency_cache_0a45e0),
          platform_(platform),publication_(publication) {if(publication_)publication_->bind(this,flags_);}
    Execution(const Execution&)=delete;Execution& operator=(const Execution&)=delete;
    Execution(Execution&&)=delete;Execution& operator=(Execution&&)=delete;

    void finish_flag_036820(std::uint8_t& supplied_flag);
    // Source composition of bound-port payload + the recovered flag routine.
    // Not a separately recovered native function. First/server flag only.
    void finish_server_startup(std::uint8_t& server_flag,std::uint16_t bound_port);
    void wait_steps_036880(std::int32_t count);
    std::int64_t now_nanoseconds_036740();

    // Extracted inline predicate at 036900..036958, NOT a separately identified
    // Windows function. Caller must already hold this Execution's mutex.
    bool steps_finished_initializing_under_lock(std::int32_t count) const;

    // Extracted inner timing loop at 036990..036a6e; no flag or mutex access.
    void sleep_until_observed_deadline(std::int64_t deadline_nanoseconds);

private:
    void acquire_checked();
    MutexState& mutex_;
    FlagReferences flags_;
    std::int64_t& frequency_cache_;
    Platform& platform_;
    Publication* publication_;
    // One wrapper owns ALL concurrent access to these refs. Direct helpers,
    // outside plain reads/writes and a second Execution sharing the same state
    // are excluded. OS lock/QPC callbacks must not reenter or throw unknown
    // outcomes. Error prefixes do not invent native unlock/rollback or retry.
    std::recursive_mutex source_gate_;
};

// Arithmetic helpers deliberately preserve x64 low-64-bit IMUL/ADD wrapping.
std::int64_t counter_to_nanoseconds(std::int64_t counter, std::int64_t frequency);
std::int64_t twenty_ms_deadline(std::int64_t now);
std::uint32_t observed_sleep_milliseconds(std::int64_t positive_remaining_nanoseconds);
}
