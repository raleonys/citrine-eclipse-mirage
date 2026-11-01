#include "api.hpp"
namespace recovered_hooks {
int NativeApi::initialize() {return MH_Initialize();}
int NativeApi::create(void* t,void* d,void** o) {return MH_CreateHook(t,d,o);}
int NativeApi::enable(void* t) {return MH_EnableHook(t);}
int NativeApi::disable(void* t) {return MH_DisableHook(t);}
int NativeApi::uninitialize() {return MH_Uninitialize();}
}
