// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/handlers/content_handlers.hpp"
#include <atomic>
#include <deque>
#include <optional>

namespace preloader_l::event_queue {
inline constexpr std::string_view initial_down_event=
    "<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>";
static_assert(initial_down_event.size()==68);
using Mutex=content_handlers::Mutex;
enum class Cleanup {queue_07e000,mutex_07e030}; // Identities, not executable VAs.
struct Guards {std::int32_t mutex_0a3970,queue_0a39a0;}; // Caller-seeded globals.
class Initialization {
public:
    virtual ~Initialization()=default;
    virtual std::int32_t thread_epoch()=0;
    virtual void header_04b838(std::int32_t& guard)=0;
    virtual void footer(std::int32_t& guard)=0;
    virtual void construct_mutex_04a698(Mutex&,std::uint32_t flags)=0;
    virtual int register_atexit(Cleanup)=0;
    // Optional owned-runtime exception cleanup; pure normal-flow models keep
    // their existing failure prefix rather than inventing private CRT EH.
    virtual void abort_initialization(std::int32_t&) noexcept {}
};
class Storage {
public:
    virtual ~Storage()=default;
    // Native deque/proxy/string allocations and partial construction belong
    // here. Called only by the initializer branch, with the copied DOWN seed.
    virtual void construct_seed_copy(std::string_view)=0;
    virtual bool nonempty_relaxed() const=0;
    virtual bool nonempty_locked() const=0;
    virtual std::string copy_front_locked() const=0;
    virtual void push_move_034660(std::string&)=0;
    virtual void pop_front_locked()=0;
    virtual void push_persisted_locked(std::string&,const std::function<void()>&);
    virtual bool front_available_locked() {return nonempty_locked();}
    virtual std::uint64_t reserve_front_locked();
    virtual bool finish_reserved(std::uint64_t,bool) noexcept {return false;}
};
// Explicit chosen FIFO backend. std::deque/string/atomic publication do not
// reconstruct MSVC ring-map/allocator/ABI or native concurrency/SEH behavior.
// Except relaxed count inspection, callers must serialize storage access.
class OwnedStorage final:public Storage {
public:
    void construct_seed_copy(std::string_view) override;
    bool nonempty_relaxed() const override;
    bool nonempty_locked() const override;
    std::string copy_front_locked() const override;
    void push_move_034660(std::string&) override;
    void pop_front_locked() override;
    void push_persisted_locked(std::string&,const std::function<void()>&) override;
    bool front_available_locked() override;
    std::uint64_t reserve_front_locked() override;
    bool finish_reserved(std::uint64_t,bool) noexcept override;
private:
    void settle_delivery_locked();
    std::deque<std::string>& data();
    const std::deque<std::string>& data() const;
    std::optional<std::deque<std::string>> entries_;
    std::atomic<std::size_t> published_size_{0};
    static_assert(std::atomic<std::uint64_t>::is_always_lock_free);
    std::atomic<std::uint64_t> delivery_state_{0};
    std::uint64_t next_delivery_=0; // Locked; low two state bits encode outcome.
};
struct Context {Guards& guards;Initialization& initialization;Mutex& mutex;Storage& storage;};
Mutex& mutex_02c620(Context&);
Storage& queue_02c400(Context&);
} // namespace preloader_l::event_queue
