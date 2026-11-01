#include "native_entries.hpp"
#include <atomic>
#include <mutex>
#include <process.h>
#include <stdexcept>
#include <type_traits>

namespace anadius::bootstrap_native_recovered {
namespace {
std::mutex gate;
recovered::Bootstrap* current{};
std::size_t calls{};
std::size_t application_calls{};
bool application_paused{};
std::atomic<Errors*> last_errors{};
class LastErrorPreserver {
public:
    LastErrorPreserver() : value_(GetLastError()) {}
    ~LastErrorPreserver() { SetLastError(value_); }
private:
    DWORD value_;
};
struct Lease {
    recovered::Bootstrap* bootstrap{};
    Errors* errors{};
    const bool application;
    explicit Lease(Entry entry):application(entry==Entry::Application) {
        LastErrorPreserver preserve;
        std::lock_guard lock(gate);
        if(!current) throw std::logic_error("bootstrap entry without binding");
        if(application&&application_paused)return;
        bootstrap=current;
        errors=last_errors.load(std::memory_order_acquire);
        ++calls;
        if(application)++application_calls;
    }
    void release() {
        if(bootstrap) {
            LastErrorPreserver preserve;
            std::lock_guard lock(gate);
            --calls;
            if(application)--application_calls;
            bootstrap=nullptr;
        }
    }
    ~Lease() { release(); }
};
template<class F> auto invoke(Entry entry,F&& f) noexcept
    -> decltype(f(std::declval<recovered::Bootstrap&>())) {
    auto* errors=last_errors.load(std::memory_order_acquire);
    try {
        Lease lease(entry);
        if(!lease.bootstrap) {
            // Only a still-bound, sealed Application route may decline.
            // Prebinding/closed attempts keep the existing exception policy.
            if constexpr(std::is_same_v<decltype(f(std::declval<recovered::Bootstrap&>())),BOOL>)return FALSE;
            else std::terminate();
        }
        errors=lease.errors; // policy belongs to this admission, even after release
        if constexpr(std::is_void_v<decltype(f(*lease.bootstrap))>) {
            f(*lease.bootstrap);
            lease.release();
        } else {
            const auto result=f(*lease.bootstrap);
            lease.release();
            return result;
        }
    } catch(...) {
        if(errors) errors->callback_exception(entry,std::current_exception());
        std::terminate(); // pre-binding calls have no supplied policy
    }
}
}
Binding::Binding(recovered::Bootstrap& bootstrap,Errors& errors)
    : bootstrap_(&bootstrap),errors_(errors) {
    bool occupied;
    {
        LastErrorPreserver preserve;
        std::lock_guard lock(gate);
        occupied=current!=nullptr;
        if(!occupied) { last_errors.store(&errors,std::memory_order_release); current=&bootstrap; application_paused=false; }
    }
    if(occupied) { bootstrap_=nullptr; errors_.lifecycle(LifecycleFault::AlreadyBound); }
}
bool Binding::pause_application_admission() {
    if(!bootstrap_)return true;
    bool wrong_owner=false,drained=false;
    {
        LastErrorPreserver preserve;
        std::lock_guard lock(gate);
        if(current!=bootstrap_)wrong_owner=true;
        else { application_paused=true;drained=application_calls==0; }
    }
    if(wrong_owner)errors_.lifecycle(LifecycleFault::WrongOwner);
    return drained;
}
void Binding::close() {
    if(!bootstrap_) return;
    LifecycleFault fault{};
    bool rejected=false;
    {
        LastErrorPreserver preserve;
        std::lock_guard lock(gate);
        if(current!=bootstrap_) { fault=LifecycleFault::WrongOwner; rejected=true; }
        else if(calls) { fault=LifecycleFault::Busy; rejected=true; }
        else { current=nullptr; bootstrap_=nullptr; }
    }
    if(rejected) errors_.lifecycle(fault);
}
Binding::~Binding() noexcept { close(); }
bool is_bound() { LastErrorPreserver preserve; std::lock_guard lock(gate); return current!=nullptr; }
std::size_t active_calls() { LastErrorPreserver preserve; std::lock_guard lock(gate); return calls; }

unsigned __stdcall no_argument_entry(void* payload) noexcept {
    return invoke(Entry::NoArgument,[&](recovered::Bootstrap& b) {
        return b.run_no_argument(static_cast<recovered::NoArgumentPayload*>(payload));
    });
}
unsigned __stdcall boolean_entry(void* payload) noexcept {
    return invoke(Entry::Boolean,[&](recovered::Bootstrap& b) {
        return b.run_boolean(static_cast<recovered::BooleanPayload*>(payload));
    });
}
void exit_entry() noexcept { invoke(Entry::Exit,[](recovered::Bootstrap& b) { b.on_exit(); }); }
[[noreturn]] void terminate_entry() noexcept {
    invoke(Entry::Terminate,[](recovered::Bootstrap& b) { b.on_terminate(); });
    std::terminate();
}
BOOL WINAPI application_entry(HINSTANCE,DWORD reason,LPVOID) noexcept {
    return invoke(Entry::Application,[&](recovered::Bootstrap& b) {
        return b.application_dll_main(reason) ? TRUE : FALSE;
    });
}
ThreadStart Resolver::thread_entry(recovered::ThreadEntry entry) const {
    switch(entry) {
    case recovered::ThreadEntry::no_argument_0388a0: return &no_argument_entry;
    case recovered::ThreadEntry::boolean_0388d0: return &boolean_entry;
    }
    throw std::out_of_range("invalid Bootstrap thread entry");
}
std::uintptr_t Resolver::registered_address(recovered::RegisteredCallback callback) const {
    switch(callback) {
    case recovered::RegisteredCallback::exit_038950: return reinterpret_cast<std::uintptr_t>(&exit_entry);
    case recovered::RegisteredCallback::terminate_038890: return reinterpret_cast<std::uintptr_t>(&terminate_entry);
    }
    throw std::out_of_range("invalid Bootstrap registered callback");
}
std::uintptr_t ThreadBridge::bind_application_entry(recovered::ThreadEntry entry) {
    return reinterpret_cast<std::uintptr_t>(Resolver{}.thread_entry(entry));
}
static_assert(std::is_convertible_v<decltype(&no_argument_entry),ThreadStart>);
static_assert(std::is_convertible_v<decltype(&boolean_entry),ThreadStart>);
static_assert(std::is_same_v<ThreadStart,_beginthreadex_proc_type>);
static_assert(std::is_convertible_v<decltype(&application_entry),ApplicationEntry>);
static_assert(std::is_convertible_v<decltype(&exit_entry),ExitCallback>);
static_assert(std::is_convertible_v<decltype(&terminate_entry),std::terminate_handler>);
static_assert(std::is_abstract_v<ThreadBridge>);
}
