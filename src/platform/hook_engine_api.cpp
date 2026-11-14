// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/hook_engine_api.hpp"
#include <MithrilHook.h>
namespace laureondo::platform::hook_engine {
// This backend returns the application codes directly. Fail compilation if a
// future vendor selection changes that mapping; keep vendor details here.
static_assert(MTH_UNKNOWN==status_code::unknown && MTH_OK==status_code::ok);
static_assert(MTH_ERROR_ALREADY_INITIALIZED==status_code::already_initialized && MTH_ERROR_NOT_INITIALIZED==status_code::not_initialized);
static_assert(MTH_ERROR_ALREADY_CREATED==status_code::already_created && MTH_ERROR_NOT_CREATED==status_code::not_created);
static_assert(MTH_ERROR_ENABLED==status_code::enabled && MTH_ERROR_DISABLED==status_code::disabled);
static_assert(MTH_ERROR_NOT_EXECUTABLE==status_code::not_executable && MTH_ERROR_UNSUPPORTED_FUNCTION==status_code::unsupported_function);
static_assert(MTH_ERROR_MEMORY_ALLOC==status_code::memory_alloc && MTH_ERROR_MEMORY_PROTECT==status_code::memory_protect);
static_assert(MTH_ERROR_MODULE_NOT_FOUND==status_code::module_not_found && MTH_ERROR_FUNCTION_NOT_FOUND==status_code::function_not_found);
static_assert(MTH_ERROR_REENTRANT_CALL==status_code::reentrant_call);
int NativeApi::initialize() {return MTH_Initialize();}
int NativeApi::create(void* t,void* d,void** o) {return MTH_CreateHook(t,d,o);}
int NativeApi::enable(void* t) {return MTH_EnableHook(t);}
int NativeApi::queue_enable(void* t) {return MTH_QueueEnableHook(t);}
int NativeApi::apply_queued() {return MTH_ApplyQueued();}
int NativeApi::disable(void* t) {return MTH_DisableHook(t);}
int NativeApi::uninitialize() {return MTH_Uninitialize();}
}
