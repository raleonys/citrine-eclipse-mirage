// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/platform/hook_engine_api.hpp"
#include <MinHook.h>
namespace laureondo::platform::hook_engine {
// This backend returns the application codes directly. Fail compilation if a
// future vendor selection changes that mapping; keep vendor details here.
static_assert(MH_UNKNOWN==status_code::unknown && MH_OK==status_code::ok);
static_assert(MH_ERROR_ALREADY_INITIALIZED==status_code::already_initialized && MH_ERROR_NOT_INITIALIZED==status_code::not_initialized);
static_assert(MH_ERROR_ALREADY_CREATED==status_code::already_created && MH_ERROR_NOT_CREATED==status_code::not_created);
static_assert(MH_ERROR_ENABLED==status_code::enabled && MH_ERROR_DISABLED==status_code::disabled);
static_assert(MH_ERROR_NOT_EXECUTABLE==status_code::not_executable && MH_ERROR_UNSUPPORTED_FUNCTION==status_code::unsupported_function);
static_assert(MH_ERROR_MEMORY_ALLOC==status_code::memory_alloc && MH_ERROR_MEMORY_PROTECT==status_code::memory_protect);
static_assert(MH_ERROR_MODULE_NOT_FOUND==status_code::module_not_found && MH_ERROR_FUNCTION_NOT_FOUND==status_code::function_not_found);
int NativeApi::initialize() {return MH_Initialize();}
int NativeApi::create(void* t,void* d,void** o) {return MH_CreateHook(t,d,o);}
int NativeApi::enable(void* t) {return MH_EnableHook(t);}
int NativeApi::queue_enable(void* t) {return MH_QueueEnableHook(t);}
int NativeApi::apply_queued() {return MH_ApplyQueued();}
int NativeApi::disable(void* t) {return MH_DisableHook(t);}
int NativeApi::uninitialize() {return MH_Uninitialize();}
}
