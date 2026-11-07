// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string_view>
#include <array>
#include <atomic>
namespace {
struct Object {std::uintptr_t* table;};
std::atomic<int> mode{0},destroyed{0};
std::atomic<bool> hold_auth{false},auth_entered{false},auth_released{true};
std::atomic<bool> registry_probe{false};
std::atomic<unsigned> registry_calls{0};
std::atomic<LSTATUS> registry_open{ERROR_IO_PENDING},registry_query{ERROR_IO_PENDING},registry_close{ERROR_IO_PENDING};
std::array<wchar_t,4096> registry_path{};
void destroy(Object*){destroyed.fetch_add(1);}
// Fault-only modes: 1 = offline status, 3 = null factory, 4/5/6 = null
// username/persona/user getter. Production provider ABI is unchanged.
int status(Object*){return mode.load()==1?1:0;}
const char* username(Object*){return mode.load()==4?nullptr:"fixture-online";}
const char* persona(Object*){return mode.load()==5?nullptr:"fixture-persona";}
const char* user(Object*){return mode.load()==6?nullptr:"fixture-user-id";}
std::uint8_t avatar(Object*,const wchar_t* path,const char* owner,std::int32_t size){
    if(!owner||std::string_view(owner)!="fixture-user-id"||(size!=0&&size!=2))return 0;
    auto* file=_wfopen(path,L"wb");if(!file)return 0;std::fputs("fixture-provider-avatar",file);std::fclose(file);return 0x80;
}
const char* auth(Object*,const char*){
    if(hold_auth.load()) {
        auth_entered.store(true);
        // A failed host assertion must not strand the test runner indefinitely.
        const auto deadline=GetTickCount64()+10000;
        while(!auth_released.load()&&GetTickCount64()<deadline)Sleep(1);
    }
    if(registry_probe.load(std::memory_order_acquire)){
        HKEY key=nullptr;
        const auto opened=RegOpenKeyExW(HKEY_CURRENT_USER,L"SOFTWARE\\Wow6432Node\\Origin",0,KEY_READ,&key);
        LSTATUS queried=ERROR_INVALID_HANDLE,closed=ERROR_INVALID_HANDLE;
        if(opened==ERROR_SUCCESS){
            DWORD bytes=static_cast<DWORD>(sizeof(registry_path)),type=0;
            queried=RegQueryValueExW(key,L"ClientPath",nullptr,&type,
                reinterpret_cast<BYTE*>(registry_path.data()),&bytes);
            closed=RegCloseKey(key);
        }
        registry_open.store(opened,std::memory_order_relaxed);
        registry_query.store(queried,std::memory_order_relaxed);
        registry_close.store(closed,std::memory_order_relaxed);
        // Publish the completed buffer before the observer reads it. The test
        // issues one request; no second auth call may mutate the borrowed view.
        registry_calls.fetch_add(1,std::memory_order_release);
    }
    return "fixture-auth";
}
std::uintptr_t table[]={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),
    reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&user),
    reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};
Object object{table};
}
extern "C" __declspec(dllexport) int version_check(){return 2;}
extern "C" __declspec(dllexport) Object* create_online(const wchar_t* cookie) {
    if(auto* file=_wfopen(cookie,L"wb")){std::fputs("fixture-cookie",file);std::fclose(file);}
    return mode.load()==3?nullptr:&object;
}
extern "C" __declspec(dllexport) void FixtureMode(int m){
    mode.store(m);destroyed.store(0);
    hold_auth.store(false);auth_entered.store(false);auth_released.store(true);
}
extern "C" __declspec(dllexport) int FixtureDestroyed(){return destroyed.load();}
extern "C" __declspec(dllexport) void FixtureHoldAuth(int enabled){
    auth_entered.store(false);auth_released.store(!enabled);hold_auth.store(enabled!=0);
}
extern "C" __declspec(dllexport) int FixtureAuthEntered(){return auth_entered.load()?1:0;}
extern "C" __declspec(dllexport) void FixtureReleaseAuth(){
    auth_released.store(true);hold_auth.store(false);
}
// Test-only controls; version_check/create_online and the seven slots above
// retain the same ABI and default behavior. Enable only after startup is ready.
extern "C" __declspec(dllexport) void FixtureRegistryProbe(int enabled){
    registry_path.fill(0);
    registry_calls.store(0,std::memory_order_relaxed);
    registry_open.store(ERROR_IO_PENDING,std::memory_order_relaxed);
    registry_query.store(ERROR_IO_PENDING,std::memory_order_relaxed);
    registry_close.store(ERROR_IO_PENDING,std::memory_order_relaxed);
    registry_probe.store(enabled!=0,std::memory_order_release);
}
extern "C" __declspec(dllexport) unsigned FixtureRegistryValue(unsigned index){
    switch(index){
    case 0:return registry_calls.load(std::memory_order_acquire);
    case 1:return static_cast<unsigned>(registry_open.load(std::memory_order_relaxed));
    case 2:return static_cast<unsigned>(registry_query.load(std::memory_order_relaxed));
    case 3:return static_cast<unsigned>(registry_close.load(std::memory_order_relaxed));
    default:return ~0u;
    }
}
extern "C" __declspec(dllexport) const wchar_t* FixtureRegistryPath(){return registry_path.data();}
