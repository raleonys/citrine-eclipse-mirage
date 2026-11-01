#include "native_api.hpp"
#include <stdexcept>

namespace anadius::crt_heap_native {
static_assert(sizeof(SIZE_T) == sizeof(std::uint64_t));
static_assert(sizeof(DWORD) == sizeof(std::uint32_t));
const Api& native_api() noexcept {
    static const Api api{&::GetProcessHeap, &::HeapAlloc, &::HeapReAlloc,
                         &::HeapFree, &::HeapSize, &::GetLastError};
    return api;
}
Calls::Calls(const Api& api) : api_(api) {
    if (!api_.get_process_heap || !api_.heap_alloc || !api_.heap_realloc ||
        !api_.heap_free || !api_.heap_size || !api_.get_last_error)
        throw std::invalid_argument("native heap API table contains a null entry");
}
Heap Calls::get_process_heap() {
    return reinterpret_cast<Heap>(api_.get_process_heap());
}
void* Calls::alloc(Heap heap, std::uint32_t flags, std::uint64_t bytes) {
    return api_.heap_alloc(reinterpret_cast<HANDLE>(heap), flags,
                           static_cast<SIZE_T>(bytes));
}
void* Calls::realloc(Heap heap, std::uint32_t flags, void* block,
                     std::uint64_t bytes) {
    return api_.heap_realloc(reinterpret_cast<HANDLE>(heap), flags, block,
                             static_cast<SIZE_T>(bytes));
}
bool Calls::free(Heap heap, std::uint32_t flags, void* block) {
    return api_.heap_free(reinterpret_cast<HANDLE>(heap), flags, block) != 0;
}
std::uint64_t Calls::size(Heap heap, std::uint32_t flags, const void* block) {
    return static_cast<std::uint64_t>(
        api_.heap_size(reinterpret_cast<HANDLE>(heap), flags, block));
}
std::uint32_t Calls::last_error() {
    return api_.get_last_error();
}
}
