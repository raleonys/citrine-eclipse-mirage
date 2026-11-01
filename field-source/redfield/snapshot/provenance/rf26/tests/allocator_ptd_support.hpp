// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/ptd_runtime.hpp"

#include <cstdlib>
#include <vector>

namespace d2test {

struct PtdFake { uint32_t last_error=0; void* slot=nullptr; };
inline uint32_t get_last(void* c) noexcept { return static_cast<PtdFake*>(c)->last_error; }
inline void set_last(void* c,uint32_t v) noexcept { static_cast<PtdFake*>(c)->last_error=v; }
inline uint32_t fls_alloc(void*,void(*)(void*) noexcept) noexcept { return 1; }
inline int fls_free(void*,uint32_t) noexcept { return 1; }
inline void* fls_get(void* c,uint32_t) noexcept { return static_cast<PtdFake*>(c)->slot; }
inline int fls_set(void* c,uint32_t,void* p) noexcept { static_cast<PtdFake*>(c)->slot=p; return 1; }
inline void* alloc_zero(void*,size_t n) noexcept { return std::calloc(1,n); }
inline int free_mem(void*,void* p) noexcept { std::free(p); return 1; }
inline void lock(void*,uint32_t) noexcept {}
inline void unlock(void*,uint32_t) noexcept {}
inline void start_ptd(PtdFake& f) {
    preloader_l::runtime::PtdPlatformOps o{&f,get_last,set_last,fls_alloc,fls_free,fls_get,fls_set,alloc_zero,free_mem,lock,unlock};
    preloader_l::runtime::PtdSharedBindings b{};
    if(!preloader_l::runtime::initialize_ptd_runtime(o,b)) std::abort();
    if(!preloader_l::runtime::current_ptd(false)) std::abort();
}
inline void stop_ptd(PtdFake& f) {
    preloader_l::runtime::detach_current_thread_ptd();
    preloader_l::runtime::shutdown_ptd_runtime();
    f.slot=nullptr;
}

struct HeapFake {
    std::vector<void*> scripted_allocs;
    size_t alloc_index=0;
    std::vector<size_t> requests;
    std::vector<void*> frees;
    std::vector<int> handler_results;
    size_t handler_index=0;
    std::vector<size_t> handler_sizes;
};
inline void* heap_alloc(void* c,size_t n) noexcept {
    auto& f=*static_cast<HeapFake*>(c); f.requests.push_back(n);
    if(f.alloc_index>=f.scripted_allocs.size()) return nullptr;
    return f.scripted_allocs[f.alloc_index++];
}
inline int heap_free(void* c,void* p) noexcept { static_cast<HeapFake*>(c)->frees.push_back(p); return 1; }
inline int handler(void* c,size_t n) {
    auto& f=*static_cast<HeapFake*>(c); f.handler_sizes.push_back(n);
    if(f.handler_index>=f.handler_results.size()) return 0;
    return f.handler_results[f.handler_index++];
}
inline preloader_l::runtime::AllocatorPlatformOps ops(HeapFake& f) { return {&f,heap_alloc,heap_free,handler}; }

}
