#pragma once
#include "domain.hpp"

namespace anadius::thread_domain {
// Explicit source policy. No original image/CRT initial values are inferred.
struct InitialPolicy {
    std::uint64_t security_cookie;
    std::int32_t new_mode;
    NewHandler new_handler;
    InvalidHandler invalid_handler;
    std::uint32_t thread_policy;
};
struct EntropyImports {
    // Injected callbacks obey nonthrowing Win32 successful-output contracts.
    decltype(&::GetSystemTimeAsFileTime) time=&::GetSystemTimeAsFileTime;
    decltype(&::GetCurrentThreadId) thread=&::GetCurrentThreadId;
    decltype(&::GetCurrentProcessId) process=&::GetCurrentProcessId;
    decltype(&::QueryPerformanceCounter) counter=&::QueryPerformanceCounter;
};
// Concrete initial policy for the rebuilt application Domain. Reuses the
// recovered04c134 cookie algorithm with actual native entropy and a real local
// source stack-cell identity. Does NOT initialize/reinitialize compiler CRT or
// write its security cookie. Selects source new_mode0, null handlers (existing
// abort policy) and thread_policy0 (existing real OS query). These are explicit
// source-policy choices, not observed MSVC image defaults. No Domain/FLS/heap
// is created here. Missing imports reject before calls; unknown/unwritten QPC
// does not silently become a zero observation. LastError is preserved.
InitialPolicy make_native_initial_policy(EntropyImports={});
struct InitialInputs {
    explicit InitialInputs(InitialPolicy value):policy(value){}
    InitialInputs(const InitialInputs&)=delete;InitialInputs& operator=(const InitialInputs&)=delete;
    InitialInputs(InitialInputs&&)=delete;InitialInputs& operator=(InitialInputs&&)=delete;
    // Optional actual backing for borrowed SDK/handler state/code. A nonnull
    // arbitrary token does not prove code provenance or prevent explicit unload.
    // Unsupplied backing stays a caller-lifetime obligation (native OS defaults
    // and null handlers do not require a dynamically owned backing object).
    std::shared_ptr<void> backing;
    InitialPolicy policy;
    Apis apis;
private:
    bool consumed_=false;
    friend std::shared_ptr<DomainStorage> make_initial_domain(InitialInputs&);
};
// Returns Constructed, without SDK initialization/publication or any workers.
// Validates all 21 SDK slots BEFORE consumption. Single-use after validation;
// allocation/construction failure does not authorize retry of consumed inputs.
// Owns one distinct opaque identity for the restricted initial-locale PTD path:
// PTD+0 is only stored/compared, never dereferenced/dispatched as an EH table.
// No exception table bytes/handler dispatch/native ABI are fabricated here.
// Identity and supplied backing outlive every Domain member, also after close
// or failed initialize. Existing controller/outside-loader-lock initialize,
// provider retirement, close retries and poisoned-owner retention still apply.
// This does not keep this factory/Domain image mapped across explicit unload.
std::shared_ptr<DomainStorage> make_initial_domain(InitialInputs&);
}
