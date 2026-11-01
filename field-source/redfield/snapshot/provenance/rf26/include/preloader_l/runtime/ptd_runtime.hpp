// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace runtime {

constexpr size_t ptd_size = 0x3C8;
constexpr uint32_t ptd_fls_out_of_indexes = 0xFFFFFFFFu;

struct ExceptionActionRecord {
    uint32_t exception_number;
    int32_t signal_number;
    uintptr_t action;
};
static_assert(sizeof(ExceptionActionRecord) == 16, "x64 PTD action record must remain 16 bytes");

struct PtdPlatformOps {
    void* context;
    uint32_t (*get_last_error)(void*) noexcept;
    void (*set_last_error)(void*, uint32_t) noexcept;
    uint32_t (*fls_alloc)(void*, void (*)(void*) noexcept) noexcept;
    int (*fls_free)(void*, uint32_t) noexcept;
    void* (*fls_get_value)(void*, uint32_t) noexcept;
    int (*fls_set_value)(void*, uint32_t, void*) noexcept;
    void* (*alloc_zero)(void*, size_t) noexcept;
    int (*free_mem)(void*, void*) noexcept;
    void (*enter_lock)(void*, uint32_t) noexcept;
    void (*leave_lock)(void*, uint32_t) noexcept;
};

struct PtdSharedBindings {
    void* shared_state;
    void* locale_state;
    void (*retain_shared)(void*) noexcept;
    void (*release_shared)(void*) noexcept;
    void (*retain_locale)(void*) noexcept;
    void (*release_locale)(void*) noexcept;
};

// Curio process-wide PTD/FLS lifecycle. Process initialization allocates
// the FLS slot and immediately materializes the initializing thread PTD. Other
// threads materialize lazily on their first create=true lookup.
bool initialize_ptd_runtime(const PtdPlatformOps& ops,
                            const PtdSharedBindings& bindings) noexcept;
void shutdown_ptd_runtime() noexcept;

// Existing lookup preserves the caller's LastError. create=true reproduces the
// sentinel -> zero-allocation -> FLS install -> post-install initialization path.
void* current_ptd(bool create = true) noexcept;
int* current_errno_slot() noexcept;

// Explicit detach reproduces clear-before-destroy ordering. The FLS callback
// destroys the supplied block without mutating the slot itself.
bool detach_current_thread_ptd() noexcept;
void destroy_ptd_from_fls_callback(void* value) noexcept;
bool ptd_cleanup_pending() noexcept;

uint32_t ptd_fls_index() noexcept;
const ExceptionActionRecord* default_exception_action_table() noexcept;
size_t default_exception_action_count() noexcept;

namespace ptd_offset {
constexpr size_t action_table = 0x000;
constexpr size_t se_translator = 0x010;
constexpr size_t terminate_handler = 0x018;
constexpr size_t errno_value = 0x020;
constexpr size_t one_28 = 0x028;
constexpr size_t owned_48 = 0x048;
constexpr size_t owned_50 = 0x050;
constexpr size_t owned_58 = 0x058;
constexpr size_t owned_60 = 0x060;
constexpr size_t owned_68 = 0x068;
constexpr size_t owned_70 = 0x070;
constexpr size_t owned_78 = 0x078;
constexpr size_t owned_80 = 0x080;
constexpr size_t shared_state = 0x088;
constexpr size_t locale_state = 0x090;
constexpr size_t byte_bc = 0x0BC;
constexpr size_t byte_1c2 = 0x1C2;
constexpr size_t zero_3a0 = 0x3A0;
constexpr size_t one_3a8 = 0x3A8;
constexpr size_t invalid_parameter_handler = 0x3B8;
constexpr size_t owned_3c0 = 0x3C0;
}

} }
