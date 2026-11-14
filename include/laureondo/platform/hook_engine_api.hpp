// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif

namespace laureondo::platform::hook_engine {
// Stable application status values, independent of a vendor header. Native
// backends must preserve these values for existing diagnostics and callers.
namespace status_code {
inline constexpr int unknown=-1,ok=0,already_initialized=1,not_initialized=2;
inline constexpr int already_created=3,not_created=4,enabled=5,disabled=6;
inline constexpr int not_executable=7,unsupported_function=8,memory_alloc=9;
inline constexpr int memory_protect=10,module_not_found=11,function_not_found=12;
}
class Api {
public:
    virtual ~Api() = default;
    virtual int initialize() = 0;
    virtual int create(void* target, void* detour, void** original) = 0;
    virtual int enable(void* target) = 0;
    virtual int disable(void* target) = 0;
    virtual int uninitialize() = 0;
};
// These methods invoke the real selected Win32 hook engine from the application
// owner. Deterministic unit fixtures can independently inject another Api.
class BatchApi : public Api {
public:
    virtual int queue_enable(void*)=0;
    virtual int apply_queued()=0;
};
// Selected patched MinHook backend. Each DLL links its own engine state.
class NativeApi final : public BatchApi {
public:
    int initialize() override;
    int create(void*, void*, void**) override;
    int enable(void*) override;
    int queue_enable(void*) override;
    int apply_queued() override;
    int disable(void*) override;
    int uninitialize() override;
};
static_assert(sizeof(void*) == 8, "This reconstruction targets Windows x64");
}
