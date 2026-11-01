#pragma once
#include <cstdint>

namespace anadius::crt_heap_recovered {
using Heap = std::uintptr_t;
static_assert(sizeof(std::uintptr_t) == 8, "Recovered Windows x64 address width");
// Module globals relocated into caller-owned state. Initialize the cookie and
// encoded handler coherently before use; this module does not recover that setup.
struct State {
    Heap heap_0a5a10;
    std::int32_t new_mode_0a5a20;
    std::uint64_t cookie_0a1580;
    std::uintptr_t encoded_handler_0a5178;
};
struct Effects {
    virtual ~Effects() = default;
    virtual Heap get_process_heap() = 0;
    // HEAP_ZERO_MEMORY (8) is an essential provider contract. Returned storage
    // and any C++ typed object lifetimes remain the provider's responsibility.
    virtual void* heap_alloc(Heap, std::uint32_t flags, std::uint64_t bytes) = 0;
    virtual bool heap_free(Heap, std::uint32_t flags, void*) = 0;
    virtual std::uint32_t last_error() = 0;
    virtual std::uint32_t errno_from_os_error_05d650(std::uint32_t) = 0;
    virtual void write_error_slot_05d724(std::uint32_t) = 0;
    virtual void lock_0625f8(std::uint32_t) = 0;
    virtual void unlock_06264c(std::uint32_t) = 0;
    // Address resolves to a rebuilt/explicitly bound callback, never an original VA.
    virtual std::int32_t invoke_new_handler(std::uintptr_t, std::uint64_t bytes) = 0;
};
bool initialize_heap_0704cc(State&, Effects&);
std::int32_t query_new_mode_07301c(const State&);
std::uintptr_t get_new_handler_0634f0(State&, Effects&);
bool call_new_handler_0634b0(std::uint64_t bytes, State&, Effects&);
void* calloc_base_066848(std::uint64_t count, std::uint64_t size, State&, Effects&);
void free_0668c0(void*, State&, Effects&);
}
