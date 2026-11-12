// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/runtime/thread_launch.hpp"
#include <limits>

namespace citrine_eclipse_mirage::threads {
Context* make_context_05e87c(std::uintptr_t entry, std::uintptr_t argument, Effects& e) {
    auto* context = e.calloc_context_066848();
    e.free_0668c0(nullptr); // Present even on failed allocation, 05e89e..05e8a3.
    if (!context) return nullptr;
    context->entry = entry;
    context->argument = argument;
    (void)e.get_module_handle_ex_w(4, entry, &context->module);
    return context;
}
Handle begin_thread_05e8e0(void* security, std::uint32_t stack_size,
                         std::uintptr_t entry, std::uintptr_t argument,
                         std::uint32_t flags, std::uint32_t* id, Effects& e) {
    if (!entry) {
        e.write_error_slot_05d724(0x16);
        e.invalid_parameter_05c954();
        return 0;
    }
    auto* context = make_context_05e87c(entry, argument, e);
    if (!context) return 0;
    std::uint32_t created_id; // Only consumed after successful CreateThread.
    const auto handle = e.create_thread(security, stack_size, context, flags, &created_id);
    if (!handle) {
        const auto error = e.last_error();
        e.map_os_error_05d698(error);
        if (context->handle) (void)e.close_handle(context->handle);
        if (context->module) (void)e.free_library(context->module);
        e.free_0668c0(context);
        return 0;
    }
    // Successful thread may already have exited/freed its Context. No context
    // access after CreateThread success, no returned-handle assignment to it.
    if (id) *id = created_id;
    return handle;
}
[[noreturn]] void common_end_thread_05e818(std::uint32_t code, Effects& e) {
    if (auto* context = e.optional_thread_context_064d08()) {
        if (context->runtime_initialized) e.uninitialize_runtime_06736c();
        constexpr auto invalid = std::numeric_limits<Handle>::max();
        // 05e849/05e85d: unsigned(value-1) <= UINT64_MAX-2.
        if (context->handle != 0 && context->handle != invalid)
            (void)e.close_handle(context->handle);
        if (context->module != 0 && context->module != invalid)
            e.free_library_and_exit_thread(context->module, code);
    }
    e.exit_thread(code);
}
[[noreturn]] void end_thread_05e9bc(std::uint32_t code, Effects& e) {
    common_end_thread_05e818(code, e);
    // The original has INT3 after common_end_thread; its exit APIs cannot return.
}
[[noreturn]] void thread_entry_05e7a8(Context* context, Effects& e) {
    if (!context) {
        const auto error = e.last_error();
        e.exit_thread(error);
    }
    e.required_thread_slot_064b90() = context;
    if (e.runtime_policy_06c0d0() == 2)
        context->runtime_initialized = e.initialize_runtime_067320(1) == 0;
    const auto result = e.invoke_entry(context->entry, context->argument);
    end_thread_05e9bc(result, e);
    // Worker exceptions are NOT caught here; no inferred SEH/EH cleanup.
}
system_services::Handle BootstrapThreadBridge::begin_thread_05e8e0(
    system_services::ThreadEntry entry, void* payload, std::uint32_t* id) {
    return threads::begin_thread_05e8e0(nullptr, 0,
        bind_application_entry(entry), reinterpret_cast<std::uintptr_t>(payload), 0, id, effects_);
}
}
