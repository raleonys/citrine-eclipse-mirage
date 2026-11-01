#include "native_shims.hpp"
#include <atomic>
#include <cstddef>
#include <utility>

namespace recovered_preloader::native {
namespace {
// Constant initialization; no global Core, native Host, or dynamic DLL loading.
constinit std::atomic<Errors*> error_policy{nullptr};
constinit std::atomic_flag gate = ATOMIC_FLAG_INIT;
constinit Binding* owner = nullptr;
constinit Core* bound_core = nullptr;
constinit std::size_t active_calls = 0;
class Lock {
public:
    Lock() noexcept { while (gate.test_and_set(std::memory_order_acquire)) {} }
    ~Lock() { gate.clear(std::memory_order_release); }
};
Errors& policy() noexcept {
    auto* value = error_policy.load(std::memory_order_acquire);
    if (!value) std::terminate();
    return *value;
}
class Lease {
public:
    Lease() {
        { Lock lock;
          if (bound_core) { core_ = bound_core; ++active_calls; }
        }
        if (!core_) policy().reject(Failure::Unbound);
    }
    ~Lease() { if (core_) { Lock lock; --active_calls; } }
    Core& core() const noexcept { return *core_; }
    Lease(const Lease&) = delete;
    Lease& operator=(const Lease&) = delete;
private:
    Core* core_ = nullptr;
};
template<class F> auto invoke(F&& action) noexcept {
    Errors& errors = policy();
    try {
        Lease lease;
        return std::forward<F>(action)(lease.core());
    } catch (...) {
        // Lease has already released its count. No C++ exception crosses the
        // SDK callback ABI. This is an explicit new policy, not MSVC/SEH recovery.
        errors.escaped_exception(std::current_exception());
    }
}
} // namespace

void configure_errors(Errors& errors) {
    Errors* expected = nullptr;
    if (!error_policy.compare_exchange_strong(expected, &errors,
            std::memory_order_acq_rel, std::memory_order_acquire) && expected != &errors)
        expected->reject(Failure::DifferentErrors);
}
Binding::Binding(Core& core, Errors& errors) : errors_(errors) {
    configure_errors(errors);
    bool conflict;
    { Lock lock;
      conflict = owner != nullptr;
      if (!conflict) { owner = this; bound_core = &core; }
    }
    if (conflict) errors_.reject(Failure::AlreadyBound);
}
void Binding::close() {
    if (closed_) return;
    bool wrong_owner, busy;
    { Lock lock;
      wrong_owner = owner != this;
      busy = active_calls != 0;
      if (!wrong_owner && !busy) {
          owner = nullptr; bound_core = nullptr; closed_ = true;
      }
    }
    if (wrong_owner) errors_.reject(Failure::WrongOwner);
    if (busy) errors_.reject(Failure::CallsInFlight);
}
Binding::~Binding() noexcept {
    try { close(); }
    catch (...) { errors_.escaped_exception(std::current_exception()); }
}
BOOL WINAPI process_next(HANDLE snapshot, LPPROCESSENTRY32W entry) noexcept {
    return invoke([&](Core& core) { return core.process_next(snapshot, entry); });
}
int __cdecl mutex_trylock(void* mutex) noexcept {
    return invoke([&](Core& core) { return core.mutex_trylock(mutex); });
}
BOOL WINAPI crypt_query(DWORD type, const void* object, DWORD content_flags,
    DWORD format_flags, DWORD flags, DWORD* encoding, DWORD* content, DWORD* format,
    HCERTSTORE* store, HCRYPTMSG* message, const void** context) noexcept {
    return invoke([&](Core& core) {
        return core.crypt_query(type, object, content_flags, format_flags, flags,
                               encoding, content, format, store, message, context);
    });
}
BOOL WINAPI application_dll_main(HMODULE module, DWORD reason, LPVOID reserved) noexcept {
    return invoke([&](Core& core) { return core.dll_main(module, reason, reserved); });
}
Bindings with_detours(ProcessNext process, MutexRoutine mutex, CryptQuery crypt) noexcept {
    return {process, mutex, crypt, &process_next, &mutex_trylock, &crypt_query};
}
} // namespace recovered_preloader::native
