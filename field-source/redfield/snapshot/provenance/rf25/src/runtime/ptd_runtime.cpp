// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime/ptd_runtime.hpp"

namespace redfield { namespace runtime {
namespace {

PtdPlatformOps g_ops{};
PtdSharedBindings g_bindings{};
uint32_t g_fls_index = ptd_fls_out_of_indexes;
bool g_ptd_runtime_initialized = false;
constexpr uintptr_t k_creation_sentinel = ~uintptr_t{0};
struct PtdPrivateTail {
    PtdPrivateTail* pending_next;
    uint32_t flags;
    uint32_t reserved;
};
constexpr uint32_t kPtdPendingFlag = 1u;
constexpr size_t k_ptd_allocation_size = ptd_size + sizeof(PtdPrivateTail);
PtdPrivateTail* g_pending_ptd_head = nullptr;
size_t g_pending_ptd_count = 0;

const ExceptionActionRecord g_default_actions[12] = {
    {0xC0000005u, 11, 0}, {0xC000001Du, 4, 0}, {0xC0000096u, 4, 0},
    {0xC000008Du, 8, 0}, {0xC000008Eu, 8, 0}, {0xC000008Fu, 8, 0},
    {0xC0000090u, 8, 0}, {0xC0000091u, 8, 0}, {0xC0000092u, 8, 0},
    {0xC0000093u, 8, 0}, {0xC00002B4u, 8, 0}, {0xC00002B5u, 8, 0},
};


PtdPrivateTail* ptd_tail(void* ptd) noexcept {
    return ptd ? reinterpret_cast<PtdPrivateTail*>(
        static_cast<unsigned char*>(ptd) + ptd_size) : nullptr;
}
template <typename T>
T& field(void* ptd, size_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<unsigned char*>(ptd) + offset);
}

template <typename T>
const T& field(const void* ptd, size_t offset) noexcept {
    return *reinterpret_cast<const T*>(static_cast<const unsigned char*>(ptd) + offset);
}

bool ops_complete(const PtdPlatformOps& o) noexcept {
    return o.get_last_error && o.set_last_error && o.fls_alloc && o.fls_free &&
           o.fls_get_value && o.fls_set_value && o.alloc_zero && o.free_mem &&
           o.enter_lock && o.leave_lock;
}

void retain_binding(void* value, void (*retain)(void*) noexcept, uint32_t lock_id) noexcept {
    if (!value || !retain) return;
    g_ops.enter_lock(g_ops.context, lock_id); retain(value); g_ops.leave_lock(g_ops.context, lock_id);
}
void release_binding(void* value, void (*release)(void*) noexcept, uint32_t lock_id) noexcept {
    if (!value || !release) return;
    g_ops.enter_lock(g_ops.context, lock_id); release(value); g_ops.leave_lock(g_ops.context, lock_id);
}

void initialize_ptd_fields(void* ptd) noexcept {
    field<const ExceptionActionRecord*>(ptd, ptd_offset::action_table) = g_default_actions;
    field<uint32_t>(ptd, ptd_offset::one_28) = 1;
    field<void*>(ptd, ptd_offset::shared_state) = g_bindings.shared_state;
    retain_binding(g_bindings.shared_state, g_bindings.retain_shared, 5);
    field<void*>(ptd, ptd_offset::locale_state) = g_bindings.locale_state;
    retain_binding(g_bindings.locale_state, g_bindings.retain_locale, 4);
    field<uint8_t>(ptd, ptd_offset::byte_bc) = 0x43;
    field<uint8_t>(ptd, ptd_offset::byte_1c2) = 0x43;
    field<uint64_t>(ptd, ptd_offset::zero_3a0) = 0;
    field<uint64_t>(ptd, ptd_offset::one_3a8) = 1;
}

bool free_owned_field(void* ptd, size_t offset) noexcept {
    void*& value = field<void*>(ptd, offset);
    if (!value) return true;
    if (!g_ops.free_mem(g_ops.context, value)) return false;
    value = nullptr;
    return true;
}

bool cleanup_materialized_ptd_fields(void* ptd) noexcept {
    if (!ptd || reinterpret_cast<uintptr_t>(ptd) == k_creation_sentinel) return true;
    bool ok = true;
    const auto* actions = field<const ExceptionActionRecord*>(ptd, ptd_offset::action_table);
    if (actions && actions != g_default_actions) {
        if (g_ops.free_mem(g_ops.context, const_cast<ExceptionActionRecord*>(actions)))
            field<const ExceptionActionRecord*>(ptd, ptd_offset::action_table) = g_default_actions;
        else ok = false;
    }
    const size_t owned[] = {ptd_offset::owned_70, ptd_offset::owned_58, ptd_offset::owned_60,
                            ptd_offset::owned_68, ptd_offset::owned_48, ptd_offset::owned_50,
                            ptd_offset::owned_78, ptd_offset::owned_80, ptd_offset::owned_3c0};
    for (size_t off : owned) ok = free_owned_field(ptd, off) && ok;
    if (!ok) return false;

    void*& shared = field<void*>(ptd, ptd_offset::shared_state);
    release_binding(shared, g_bindings.release_shared, 5); shared = nullptr;
    void*& locale = field<void*>(ptd, ptd_offset::locale_state);
    release_binding(locale, g_bindings.release_locale, 4); locale = nullptr;
    return true;
}

bool destroy_materialized_ptd(void* ptd) noexcept {
    if (!ptd || reinterpret_cast<uintptr_t>(ptd) == k_creation_sentinel) return true;
    if (!cleanup_materialized_ptd_fields(ptd)) return false;
    return g_ops.free_mem(g_ops.context, ptd) != 0;
}

void retain_pending_ptd(void* ptd) noexcept {
    if (!ptd) return;
    auto* tail = ptd_tail(ptd);
    g_ops.enter_lock(g_ops.context, 0);
    if ((tail->flags & kPtdPendingFlag) != 0u) {
        g_ops.leave_lock(g_ops.context,0);
        return;
    }
    tail->pending_next = g_pending_ptd_head;
    tail->flags |= kPtdPendingFlag;
    g_pending_ptd_head = tail;
    ++g_pending_ptd_count;
    g_ops.leave_lock(g_ops.context,0);
}

bool retry_pending_ptds() noexcept {
    if (!g_ops.free_mem) return g_pending_ptd_count == 0;
    bool all = true;
    for (;;) {
        void* ptd = nullptr;
        g_ops.enter_lock(g_ops.context,0);
        if (g_pending_ptd_head) {
            auto* tail = g_pending_ptd_head;
            g_pending_ptd_head = tail->pending_next;
            tail->pending_next = nullptr;
            tail->flags &= ~kPtdPendingFlag;
            --g_pending_ptd_count;
            ptd = reinterpret_cast<unsigned char*>(tail) - ptd_size;
        }
        g_ops.leave_lock(g_ops.context,0);
        if (!ptd) break;
        if (!destroy_materialized_ptd(ptd)) {
            retain_pending_ptd(ptd);
            all = false;
            break;
        }
    }
    return all && g_pending_ptd_count == 0;
}

void fls_destructor(void* value) noexcept {
    if (!destroy_materialized_ptd(value)) retain_pending_ptd(value);
}

void* materialize_current_ptd(uint32_t saved_last_error) noexcept {
    if (!g_ops.fls_set_value(g_ops.context, g_fls_index, reinterpret_cast<void*>(k_creation_sentinel))) {
        g_ops.set_last_error(g_ops.context, saved_last_error); return nullptr;
    }
    // F12: allocate a private intrusive retry tail after the public 0x3C8 PTD.
    // The returned PTD address and all public offsets remain unchanged, while
    // failed frees can always be retained without allocating or using a fixed
    // process-wide slot table.
    void* ptd = g_ops.alloc_zero(g_ops.context, k_ptd_allocation_size);
    if (!ptd) {
        // F321-S148-001: a failed clear leaves the sentinel intentionally live;
        // current_ptd(create=true) will retry clearing it on the next call.
        (void)g_ops.fls_set_value(g_ops.context, g_fls_index, nullptr);
        g_ops.set_last_error(g_ops.context, saved_last_error); return nullptr;
    }
    if (!g_ops.fls_set_value(g_ops.context, g_fls_index, ptd)) {
        // Confirm the failed publication did not make the candidate current.
        void* observed = g_ops.fls_get_value(g_ops.context, g_fls_index);
        if (observed == ptd) {
            if (!g_ops.fls_set_value(g_ops.context, g_fls_index, nullptr)) {
                g_ops.set_last_error(g_ops.context, saved_last_error); return nullptr;
            }
        } else if (reinterpret_cast<uintptr_t>(observed) == k_creation_sentinel) {
            (void)g_ops.fls_set_value(g_ops.context, g_fls_index, nullptr);
        }
        if (!g_ops.free_mem(g_ops.context, ptd)) retain_pending_ptd(ptd);
        g_ops.set_last_error(g_ops.context, saved_last_error); return nullptr;
    }
    initialize_ptd_fields(ptd);
    g_ops.set_last_error(g_ops.context, saved_last_error);
    return ptd;
}

} // namespace

bool initialize_ptd_runtime(const PtdPlatformOps& ops, const PtdSharedBindings& bindings) noexcept {
    if (g_ops.free_mem && !retry_pending_ptds()) return false;
    if (g_fls_index != ptd_fls_out_of_indexes) {
        if (g_ptd_runtime_initialized) return true;
        if (!g_ops.fls_free(g_ops.context, g_fls_index)) return false;
        g_fls_index = ptd_fls_out_of_indexes;
        if (!retry_pending_ptds()) return false;
        g_ops = {}; g_bindings = {};
    } else if (g_ops.free_mem) {
        if (!retry_pending_ptds()) return false;
        g_ops = {}; g_bindings = {};
    }
    if (!ops_complete(ops)) return false;
    g_ops=ops; g_bindings=bindings;
    const uint32_t index=g_ops.fls_alloc(g_ops.context,&fls_destructor);
    if(index==ptd_fls_out_of_indexes){g_ops={};g_bindings={};return false;}
    g_fls_index=index; g_ptd_runtime_initialized=false;
    if(!current_ptd(true)){
        if(g_ops.fls_free(g_ops.context,index)){g_fls_index=ptd_fls_out_of_indexes; if(retry_pending_ptds()){g_ops={};g_bindings={};}}
        return false;
    }
    g_ptd_runtime_initialized=true; return true;
}

void shutdown_ptd_runtime() noexcept {
    if (g_fls_index != ptd_fls_out_of_indexes) {
        const uint32_t index=g_fls_index;
        if(!g_ops.fls_free(g_ops.context,index)) return;
        g_fls_index=ptd_fls_out_of_indexes;
        g_ptd_runtime_initialized=false;
    }
    if(!retry_pending_ptds()) return;
    g_ops={}; g_bindings={};
}

void* current_ptd(bool create) noexcept {
    if(g_fls_index==ptd_fls_out_of_indexes||!g_ops.fls_get_value)return nullptr;
    const uint32_t saved=g_ops.get_last_error(g_ops.context);
    void* ptd=g_ops.fls_get_value(g_ops.context,g_fls_index);
    if(reinterpret_cast<uintptr_t>(ptd)==k_creation_sentinel){
        if(create && g_ops.fls_set_value(g_ops.context,g_fls_index,nullptr)) ptd=nullptr;
        else {g_ops.set_last_error(g_ops.context,saved);return nullptr;}
    }
    if(!ptd&&create)return materialize_current_ptd(saved);
    g_ops.set_last_error(g_ops.context,saved);return ptd;
}

int* current_errno_slot() noexcept {void*ptd=current_ptd(true);return ptd?&field<int>(ptd,ptd_offset::errno_value):nullptr;}

bool detach_current_thread_ptd() noexcept {
    if(g_fls_index==ptd_fls_out_of_indexes||!g_ops.fls_get_value)return true;
    void*ptd=g_ops.fls_get_value(g_ops.context,g_fls_index);
    if(!ptd)return true;
    if(reinterpret_cast<uintptr_t>(ptd)==k_creation_sentinel)
        return g_ops.fls_set_value(g_ops.context,g_fls_index,nullptr)!=0;
    // Keep FLS ownership published while field cleanup is incomplete.
    if(!cleanup_materialized_ptd_fields(ptd))return false;
    if(!g_ops.fls_set_value(g_ops.context,g_fls_index,nullptr))return false;
    if(g_ops.free_mem(g_ops.context,ptd))return true;
    // Prefer restoring the exact FLS owner. If that fails, transfer ownership
    // to the process-wide pending list so shutdown remains retryable.
    if(g_ops.fls_set_value(g_ops.context,g_fls_index,ptd))return false;
    retain_pending_ptd(ptd); return false;
}

void destroy_ptd_from_fls_callback(void* value) noexcept {fls_destructor(value);}
bool ptd_cleanup_pending() noexcept {return g_fls_index!=ptd_fls_out_of_indexes||g_pending_ptd_count!=0;}
uint32_t ptd_fls_index() noexcept{return g_fls_index;}
const ExceptionActionRecord* default_exception_action_table() noexcept{return g_default_actions;}
size_t default_exception_action_count() noexcept{return 12;}

} }
