#pragma once
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include "../anadius-thread-source/threads.hpp"

namespace anadius::crt_runtime_recovered {
using Address = std::uintptr_t;
using Module = std::uintptr_t;
inline constexpr Address missing = ~Address{0};

// Full table widths and all 21 module names are retained from the hash-bound
// target. This runtime subset consumes procedure slots 23/24/26.
struct State {
    // New source composition pre-initializes the 31 encoded-null cells with
    // the body corroborated at 067538. Cookie initialization is separate and
    // must already have selected the immutable security_cookie argument.
    explicit State(std::uint64_t security_cookie);
    const std::uint64_t cookie;
    std::array<std::atomic<Module>, 21> modules{};
    std::array<std::atomic<Address>, 31> procedures{};
    std::array<std::u16string_view, 21> module_names{};
    std::atomic<std::uint32_t> thread_policy{0};
};

class Effects {
public:
    virtual ~Effects() = default;
    virtual Module load_library_ex_w(std::u16string_view, Module file,
                                     std::uint32_t flags) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual bool free_library(Module) = 0;
    virtual Address get_proc_address(Module, std::string_view) = 0;
    // PEB->ProcessParameters->Flags (+8). No live PEB is read by this module.
    virtual std::uint32_t process_parameters_flags() = 0;
    // Address is an opaque rebuilt-provider address, never an original VA.
    virtual std::int32_t call_ro_initialize(Address, std::uint32_t mode) = 0;
    virtual void call_ro_uninitialize(Address) = 0;
    virtual std::uint32_t call_thread_initialization_policy(
        Address, std::uintptr_t token, std::int32_t* result) = 0;
};

Address encode_pointer(Address, std::uint64_t cookie) noexcept;
Address decode_pointer(Address, std::uint64_t cookie) noexcept;

// Supported inputs: in-range procedure/module indices; names are immutable and
// present for every candidate used and have no embedded NUL; no exception
// crosses an original Win32 ABI. All 21 names are recovered literals.
// Rebuilt C++ dependencies can throw; no invented cleanup or cache success is
// added on that path. Atomic cells model observed MOV/XCHG without C++ races.
Address resolve_0669b0(State&, Effects&, std::uint32_t procedure,
                      std::string_view name,
                      std::span<const std::uint32_t> module_candidates);
std::uint32_t process_parameters_flag_06c0b8(Effects&);
std::uint32_t query_thread_policy_066bc4(State&, Effects&, std::int32_t*);
std::uint32_t runtime_policy_06c0d0(State&, Effects&);
std::int32_t initialize_runtime_067320(State&, Effects&, std::uint32_t mode);
void uninitialize_runtime_06736c(State&, Effects&);

// New source composition: the three recovered runtime bodies satisfy these
// three CRT thread dependencies. Heap, PTD, thread creation and exit stay pure.
class ThreadRuntimeBridge : public threads_recovered::Effects {
public:
    ThreadRuntimeBridge(State& state, crt_runtime_recovered::Effects& effects)
        : state_(state), effects_(effects) {}
    int runtime_policy_06c0d0() final;
    int initialize_runtime_067320(int) final;
    void uninitialize_runtime_06736c() final;
private:
    State& state_;
    crt_runtime_recovered::Effects& effects_;
};
} // namespace anadius::crt_runtime_recovered
