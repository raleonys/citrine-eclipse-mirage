#include "errors.hpp"
#include <cstddef>
#include <cstdlib>
#include <new>

namespace anadius::permanent_errors {
namespace {
class CompilerAbort final:public native_recovered::Errors,public bootstrap_native_recovered::Errors {
public:
    [[noreturn]] void lifecycle(native_recovered::LifecycleFault) noexcept override {std::abort();}
    [[noreturn]] void lifecycle(bootstrap_native_recovered::LifecycleFault) noexcept override {std::abort();}
    [[noreturn]] void callback_exception(native_recovered::Callback,std::exception_ptr) noexcept override {std::abort();}
    [[noreturn]] void callback_exception(bootstrap_native_recovered::Entry,std::exception_ptr) noexcept override {std::abort();}
};
// Trivial static storage has no destructor registration. Never own the placed
// object with a static CompilerAbort, smart pointer or automatic lifetime.
alignas(CompilerAbort) constinit std::byte storage[sizeof(CompilerAbort)]{};
}
Policies compiler_abort_policy() noexcept {
    const DWORD error=::GetLastError();
    // One-time thread-safe initialization; no heap allocation. The pointer is
    // trivially destructible, and the placed object's destructor is never run.
    static auto* const policy=::new(static_cast<void*>(storage)) CompilerAbort;
    ::SetLastError(error);
    return {*policy,*policy}; // normal adjusted conversions for the two bases
}
}
