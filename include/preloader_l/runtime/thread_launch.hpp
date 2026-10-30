#pragma once
#include <cstddef>
#include <cstdint>
#include "preloader_l/runtime/bootstrap.hpp"

namespace preloader_l::threads_recovered {
using Handle = std::uintptr_t;
// Only fields accessed by 05e7a8/05e818/05e87c/05e8e0. Entry is opaque:
// it is never interpreted as a data pointer or invoked by this module.
struct Context {
    std::uintptr_t entry;
    std::uintptr_t argument;
    Handle handle;
    Handle module;
    std::uint8_t runtime_initialized;
    std::byte unobserved_21[7];
};
static_assert(sizeof(Context) == 0x28);
static_assert(offsetof(Context, module) == 0x18);
static_assert(offsetof(Context, runtime_initialized) == 0x20);

class Effects {
public:
    virtual ~Effects() = default;
    // Supplies a zeroed, live Context or nullptr, as calloc(1,0x28).
    virtual Context* calloc_context_066848() = 0;
    virtual void free_0668c0(Context*) = 0;
    virtual bool get_module_handle_ex_w(std::uint32_t flags, std::uintptr_t address,
                                      Handle* result) = 0;
    // Entry passed to CreateThread is 05e7a8, NOT Context::entry. Backend must
    // bind that wrapper to this module's thread_entry_05e7a8 and its Effects.
    virtual Handle create_thread(void* security, std::uint64_t stack_size,
                                 Context*, std::uint32_t flags, std::uint32_t* id) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual void write_error_slot_05d724(std::uint32_t) = 0;
    virtual void invalid_parameter_05c954() = 0;
    virtual void map_os_error_05d698(std::uint32_t) = 0;
    virtual bool close_handle(Handle) = 0;
    virtual bool free_library(Handle) = 0;
    // Slot at PTD+0x3c0; required lookup may initialize the CRT PTD or terminate.
    virtual Context*& required_thread_slot_064b90() = 0;
    virtual Context* optional_thread_context_064d08() = 0;
    virtual int runtime_policy_06c0d0() = 0;
    virtual int initialize_runtime_067320(int) = 0;
    virtual void uninitialize_runtime_06736c() = 0;
    virtual std::uint32_t invoke_entry(std::uintptr_t entry, std::uintptr_t argument) = 0;
    [[noreturn]] virtual void free_library_and_exit_thread(Handle, std::uint32_t) = 0;
    [[noreturn]] virtual void exit_thread(std::uint32_t) = 0;
};
Context* make_context_05e87c(std::uintptr_t entry, std::uintptr_t argument, Effects&);
Handle begin_thread_05e8e0(void* security, std::uint32_t stack_size,
                         std::uintptr_t entry, std::uintptr_t argument,
                         std::uint32_t flags, std::uint32_t* id, Effects&);
[[noreturn]] void thread_entry_05e7a8(Context*, Effects&);
[[noreturn]] void common_end_thread_05e818(std::uint32_t code, Effects&);
[[noreturn]] void end_thread_05e9bc(std::uint32_t code, Effects&);

// New composition adapter. Other BootstrapDependencies remain mandatory.
// A caller must bind entry tokens to its own recovered Bootstrap trampolines;
// original-image addresses are documentation, never executable callbacks.
class BootstrapThreadBridge : public recovered::BootstrapDependencies {
public:
    explicit BootstrapThreadBridge(Effects& effects) : effects_(effects) {}
    recovered::Handle begin_thread_05e8e0(recovered::ThreadEntry, void*,
                                          std::uint32_t*) final;
protected:
    virtual std::uintptr_t bind_application_entry(recovered::ThreadEntry) = 0;
private:
    Effects& effects_;
};
}
