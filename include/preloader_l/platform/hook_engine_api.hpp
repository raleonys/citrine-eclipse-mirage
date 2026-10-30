#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <MinHook.h>

namespace preloader_l::platform::hook_engine {
// Explicitly selected upstream compatibility engine; not the original source.
// Each DLL must statically link its own copy to preserve per-image engine state.
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
static_assert(MH_OK == 0 && MH_ERROR_ALREADY_INITIALIZED == 1 && MH_ERROR_NOT_INITIALIZED == 2);
static_assert(MH_ERROR_ALREADY_CREATED == 3 && MH_ERROR_NOT_CREATED == 4);
static_assert(MH_ERROR_ENABLED == 5 && MH_ERROR_DISABLED == 6 && MH_ERROR_NOT_EXECUTABLE == 7);
static_assert(MH_ERROR_UNSUPPORTED_FUNCTION == 8 && MH_ERROR_MEMORY_ALLOC == 9 && MH_ERROR_MEMORY_PROTECT == 10);
static_assert(MH_ERROR_MODULE_NOT_FOUND == 11 && MH_ERROR_FUNCTION_NOT_FOUND == 12);
}
