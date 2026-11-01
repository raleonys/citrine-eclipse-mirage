#pragma once
#include <stddef.h>
#include <stdint.h>

namespace preloader_l { namespace platform {

// Recovered from the model x64 binary. This is an ABI description only;
// production code does not reinterpret live SafetyHook objects through it.
#pragma pack(push, 1)
struct CurioSafetyHookInlineStorage {
    uint64_t target;                 // +0x00
    uint64_t destination;            // +0x08
    uint8_t allocation[0x20];        // +0x10 safetyhook::Allocation
    uint8_t original_bytes[0x18];    // +0x30 std::vector<uint8_t>
    uint64_t trampoline_size;        // +0x48
    uint8_t mutex[0x50];             // +0x50 MSVC std::mutex
    uint8_t enabled;                 // +0xA0
    uint8_t padding_a1_a3[3];        // +0xA1
    uint32_t type;                   // +0xA4: 0 unset, 1 E9, 2 FF
};

struct CurioSafetyHookExpectedInlineStorage {
    CurioSafetyHookInlineStorage value; // +0x00
    uint8_t has_value;                     // +0xA8
    uint8_t tail_padding[7];                // +0xA9
};
#pragma pack(pop)

static_assert(sizeof(CurioSafetyHookInlineStorage) == 0xA8u,
              "curio SafetyHook InlineHook x64 ABI drift");
static_assert(offsetof(CurioSafetyHookInlineStorage, target) == 0x00u, "target offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, destination) == 0x08u, "destination offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, allocation) == 0x10u, "allocation offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, original_bytes) == 0x30u, "vector offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, trampoline_size) == 0x48u, "size offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, mutex) == 0x50u, "mutex offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, enabled) == 0xA0u, "enabled offset");
static_assert(offsetof(CurioSafetyHookInlineStorage, type) == 0xA4u, "type offset");
static_assert(sizeof(CurioSafetyHookExpectedInlineStorage) == 0xB0u,
              "curio expected<InlineHook,Error> x64 ABI drift");
static_assert(offsetof(CurioSafetyHookExpectedInlineStorage, has_value) == 0xA8u,
              "expected discriminator offset");

enum class CurioSafetyHookType : uint32_t {
    unset = 0,
    e9 = 1,
    ff = 2,
};

} } // namespace preloader_l::platform
