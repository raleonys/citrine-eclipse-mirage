#pragma once
#include "../anadius-content-handlers-source/content_handlers.hpp"
#include <atomic>
#include <deque>
#include <optional>

namespace anadius::event_queue_recovered {
inline constexpr std::string_view initial_down_event=
    "<LSX><Event sender=\"EbisuSDK\"><IGOEvent State=\"DOWN\"/></Event></LSX>";
static_assert(initial_down_event.size()==68);
using Mutex=content_handlers_recovered::Mutex;
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
private:
    std::deque<std::string>& data();
    const std::deque<std::string>& data() const;
    std::optional<std::deque<std::string>> entries_;
    std::atomic<std::size_t> published_size_{0};
};
struct Context {Guards& guards;Initialization& initialization;Mutex& mutex;Storage& storage;};
Mutex& mutex_02c620(Context&);
Storage& queue_02c400(Context&);
} // namespace anadius::event_queue_recovered
