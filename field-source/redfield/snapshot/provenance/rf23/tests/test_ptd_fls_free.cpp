// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime/ptd_runtime.hpp"
#include <cstdint>
#include <cstdlib>

using namespace redfield::runtime;

namespace {
struct Fake {
    uint32_t last_error = 0x55AAu;
    uint32_t index = 23u;
    void* slot = nullptr;
    bool fail_alloc = false;
    bool fail_free = false;
    unsigned fls_alloc_calls = 0;
    unsigned fls_free_calls = 0;
    uint32_t last_freed_index = ptd_fls_out_of_indexes;
};

uint32_t get_last(void* c) noexcept { return static_cast<Fake*>(c)->last_error; }
void set_last(void* c, uint32_t v) noexcept { static_cast<Fake*>(c)->last_error = v; }
uint32_t fls_alloc(void* c, void (*)(void*) noexcept) noexcept {
    auto* f = static_cast<Fake*>(c);
    ++f->fls_alloc_calls;
    return f->index;
}
int fls_free(void* c, uint32_t index) noexcept {
    auto* f = static_cast<Fake*>(c);
    ++f->fls_free_calls;
    f->last_freed_index = index;
    return f->fail_free ? 0 : 1;
}
void* fls_get(void* c, uint32_t) noexcept { return static_cast<Fake*>(c)->slot; }
int fls_set(void* c, uint32_t, void* value) noexcept {
    static_cast<Fake*>(c)->slot = value;
    return 1;
}
void* alloc_zero(void* c, size_t n) noexcept {
    return static_cast<Fake*>(c)->fail_alloc ? nullptr : std::calloc(1, n);
}
int free_mem(void*, void* p) noexcept { std::free(p); return 1; }
void lock(void*, uint32_t) noexcept {}

PtdPlatformOps ops(Fake& f) noexcept {
    return PtdPlatformOps{&f, get_last, set_last, fls_alloc, fls_free,
                          fls_get, fls_set, alloc_zero, free_mem, lock, lock};
}
}

int main() {
    // Shutdown failure must preserve the exact live index and platform policy
    // so the same teardown can be retried instead of publishing false success.
    {
        Fake f{};
        auto o = ops(f);
        PtdSharedBindings b{};
        if (!initialize_ptd_runtime(o, b)) return 1;
        if (ptd_fls_index() != f.index || !current_ptd(false)) return 2;
        detach_current_thread_ptd();

        f.fail_free = true;
        shutdown_ptd_runtime();
        if (ptd_fls_index() != f.index) return 3;
        if (f.fls_free_calls != 1 || f.last_freed_index != f.index) return 4;

        f.fail_free = false;
        shutdown_ptd_runtime();
        if (ptd_fls_index() != ptd_fls_out_of_indexes) return 5;
        if (f.fls_free_calls != 2 || f.last_freed_index != f.index) return 6;
    }

    // Initialization rollback failure is also retained, but must never be
    // mistaken for an initialized runtime on the next initialize call. The next
    // call retries cleanup first, then performs a fresh allocation/materialize.
    {
        Fake f{};
        auto o = ops(f);
        PtdSharedBindings b{};
        f.fail_alloc = true;
        f.fail_free = true;
        if (initialize_ptd_runtime(o, b)) return 7;
        if (ptd_fls_index() != f.index) return 8;
        if (f.fls_alloc_calls != 1 || f.fls_free_calls != 1) return 9;
        if (f.slot != nullptr) return 10;

        f.fail_alloc = false;
        f.fail_free = false;
        if (!initialize_ptd_runtime(o, b)) return 11;
        if (ptd_fls_index() != f.index || !current_ptd(false)) return 12;
        if (f.fls_alloc_calls != 2 || f.fls_free_calls != 2) return 13;

        detach_current_thread_ptd();
        shutdown_ptd_runtime();
        if (ptd_fls_index() != ptd_fls_out_of_indexes) return 14;
        if (f.fls_free_calls != 3) return 15;
    }
    return 0;
}
