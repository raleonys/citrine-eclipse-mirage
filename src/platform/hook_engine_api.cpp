#include "preloader_l/platform/hook_engine_api.hpp"
namespace preloader_l::platform::hook_engine {
int NativeApi::initialize() {return MH_Initialize();}
int NativeApi::create(void* t,void* d,void** o) {return MH_CreateHook(t,d,o);}
int NativeApi::enable(void* t) {return MH_EnableHook(t);}
int NativeApi::queue_enable(void* t) {return MH_QueueEnableHook(t);}
int NativeApi::apply_queued() {return MH_ApplyQueued();}
int NativeApi::disable(void* t) {return MH_DisableHook(t);}
int NativeApi::uninitialize() {return MH_Uninitialize();}
}
