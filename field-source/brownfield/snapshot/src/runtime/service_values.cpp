#include "service_policy_internal.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"

namespace preloader_l { namespace runtime { namespace model_release_service_policy_internal {
namespace {
constexpr uint64_t kSsoCapacity = 15u;
constexpr uint64_t kWidePathSsoCapacity = 7u;
}

void zero_bytes(void* p, size_t n) noexcept {
    auto* b = static_cast<uint8_t*>(p);
    if (!b) return;
    for (size_t i = 0; i < n; ++i) b[i] = 0u;
}

void* allocate_bytes(size_t size) noexcept {
    if (!size) return nullptr;
    const auto r = allocate_block(size, AllocationRetryPolicy::outer_retry_only,
                                  windows_allocator_platform_ops());
    return r.terminal == AllocationTerminal::none ? r.pointer : nullptr;
}
bool release_bytes(void* p, size_t logical_size) noexcept {
    if (!p) return true;
    return release_block(p, logical_size, windows_allocator_platform_ops()) ==
           AllocationReleaseStatus::ok;
}

void initialize_empty_string(model_abi::MsvcString32* value) noexcept {
    if (!value) return;
    zero_bytes(value->storage, sizeof(value->storage));
    value->size = 0u;
    value->capacity = kSsoCapacity;
}
uint8_t* string_heap_pointer(model_abi::MsvcString32* value) noexcept {
    if (!value || value->capacity <= kSsoCapacity) return nullptr;
    return *reinterpret_cast<uint8_t**>(value->storage);
}
const uint8_t* string_heap_pointer(const model_abi::MsvcString32* value) noexcept {
    if (!value || value->capacity <= kSsoCapacity) return nullptr;
    return *reinterpret_cast<uint8_t* const*>(value->storage);
}
const char* string_data(const model_abi::MsvcString32& value) noexcept {
    return value.capacity <= kSsoCapacity
        ? reinterpret_cast<const char*>(value.storage)
        : reinterpret_cast<const char*>(string_heap_pointer(&value));
}

bool release_string(model_abi::MsvcString32* value) noexcept {
    if (!value) return true;
    if (uint8_t* p = string_heap_pointer(value)) {
        if (value->capacity == static_cast<uint64_t>(-1)) return false;
        if (!release_bytes(p, static_cast<size_t>(value->capacity) + 1u))
            return false;
    }
    initialize_empty_string(value);
    return true;
}

bool assign_string(model_abi::MsvcString32* destination,
                   const char* source, size_t size) noexcept {
    if (!destination || (!source && size)) return false;
    model_abi::MsvcString32 replacement{};
    replacement.capacity = kSsoCapacity;
    if (size <= kSsoCapacity) {
        for (size_t i = 0; i < size; ++i)
            replacement.storage[i] = static_cast<uint8_t>(source[i]);
        replacement.storage[size] = 0u;
        replacement.size = size;
    } else {
        if (size == static_cast<size_t>(-1)) return false;
        auto* p = static_cast<uint8_t*>(allocate_bytes(size + 1u));
        if (!p) return false;
        for (size_t i = 0; i < size; ++i) p[i] = static_cast<uint8_t>(source[i]);
        p[size] = 0u;
        *reinterpret_cast<uint8_t**>(replacement.storage) = p;
        replacement.size = size;
        replacement.capacity = size;
    }
    if (!release_string(destination)) {
        if (replacement.capacity > kSsoCapacity) {
            auto* fresh = string_heap_pointer(&replacement);
            if (fresh) (void)release_bytes(fresh, static_cast<size_t>(replacement.capacity) + 1u);
        }
        return false;
    }
    *destination = replacement;
    return true;
}

bool string_equal(const model_abi::MsvcString32& value,
                  const char* data, size_t size) noexcept {
    if ((!data && size) || value.size != size) return false;
    const char* stored = string_data(value);
    if (size && !stored) return false;
    for (size_t i = 0; i < size; ++i)
        if (stored[i] != data[i]) return false;
    return true;
}

uintptr_t load_wide_path_pointer_bits(const uint16_t storage[8]) noexcept {
    uintptr_t raw = 0u;
    auto* out = reinterpret_cast<uint8_t*>(&raw);
    const auto* in = reinterpret_cast<const uint8_t*>(storage);
    for (size_t i = 0; i < sizeof(raw); ++i) out[i] = in[i];
    return raw;
}
void store_wide_path_pointer_bits(uint16_t storage[8], uintptr_t raw) noexcept {
    auto* out = reinterpret_cast<uint8_t*>(storage);
    const auto* in = reinterpret_cast<const uint8_t*>(&raw);
    for (size_t i = 0; i < sizeof(raw); ++i) out[i] = in[i];
}
uint16_t* wide_path_heap_pointer(model_abi::ModelWidePathString32* value) noexcept {
    if (!value || value->capacity <= kWidePathSsoCapacity) return nullptr;
    return reinterpret_cast<uint16_t*>(load_wide_path_pointer_bits(value->storage));
}

void initialize_empty_wide_path(model_abi::ModelWidePathString32* value) noexcept {
    if (!value) return;
    zero_bytes(value, sizeof(*value));
    value->capacity = kWidePathSsoCapacity;
}

bool release_path(model_abi::ModelWidePathString32* value) noexcept {
    if (!value) return true;
    if (uint16_t* p = wide_path_heap_pointer(value)) {
        if (value->capacity > (static_cast<uint64_t>(static_cast<size_t>(-1)) / 2u) - 1u)
            return false;
        const size_t bytes = (static_cast<size_t>(value->capacity) + 1u) * sizeof(uint16_t);
        if (!release_bytes(p, bytes)) return false;
    }
    initialize_empty_wide_path(value);
    return true;
}

bool assign_wide_path(model_abi::ModelWidePathString32* value,
                      const uint16_t* source, size_t size) noexcept {
    if (!value || (!source && size)) return false;
    if (!size) return release_path(value);
    model_abi::ModelWidePathString32 replacement{};
    replacement.capacity = kWidePathSsoCapacity;
    if (size <= kWidePathSsoCapacity) {
        for (size_t i = 0; i < size; ++i) replacement.storage[i] = source[i];
        replacement.storage[size] = 0u;
        replacement.size = size;
    } else {
        if (size > (static_cast<size_t>(-1) / sizeof(uint16_t)) - 1u) return false;
        size_t capacity = size | size_t{7u};
        if (capacity < 10u) capacity = 10u;
        if (capacity < size || capacity > (static_cast<size_t>(-1) / sizeof(uint16_t)) - 1u)
            return false;
        const size_t bytes = (capacity + 1u) * sizeof(uint16_t);
        auto* p = static_cast<uint16_t*>(allocate_bytes(bytes));
        if (!p) return false;
        for (size_t i = 0; i < size; ++i) p[i] = source[i];
        p[size] = 0u;
        store_wide_path_pointer_bits(replacement.storage, reinterpret_cast<uintptr_t>(p));
        replacement.size = size;
        replacement.capacity = capacity;
    }
    if (!release_path(value)) {
        if (replacement.capacity > kWidePathSsoCapacity) {
            auto* fresh = wide_path_heap_pointer(&replacement);
            if (fresh) {
                const size_t bytes = (static_cast<size_t>(replacement.capacity) + 1u) * sizeof(uint16_t);
                (void)release_bytes(fresh, bytes);
            }
        }
        return false;
    }
    *value = replacement;
    return true;
}

bool assign_path_from_narrow_bytes(model_abi::ModelWidePathString32* value,
                                   const char* source, size_t size) noexcept {
    if (!value || (!source && size)) return false;
    if (!size) return release_path(value);
    if (size > static_cast<size_t>(-1) / sizeof(uint16_t)) return false;
    const size_t bytes = size * sizeof(uint16_t);
    auto* temporary = static_cast<uint16_t*>(allocate_bytes(bytes));
    if (!temporary) return false;
    for (size_t i = 0; i < size; ++i)
        temporary[i] = static_cast<uint8_t>(source[i]);
    const bool ok = assign_wide_path(value, temporary, size);
    const bool released = release_bytes(temporary, bytes);
    return ok && released;
}

} } }
