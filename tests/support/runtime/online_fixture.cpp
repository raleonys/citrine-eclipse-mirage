// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <atomic>
namespace {
struct Object {std::uintptr_t* table;};
std::atomic<int> mode{0},destroyed{0};
std::atomic<bool> hold_auth{false},auth_entered{false},auth_released{true};
void destroy(Object*){destroyed.fetch_add(1);}
int status(Object*){return mode.load();}
const char* username(Object*){return "fixture-online";}
const char* persona(Object*){return "fixture-persona";}
const char* user(Object*){return "fixture-user-id";}
bool avatar(Object*,const wchar_t* path){auto* file=_wfopen(path,L"wb");if(!file)return false;std::fputs("fixture-provider-avatar",file);std::fclose(file);return true;}
const char* auth(Object*,const char*) {
    if(hold_auth.load()) {
        auth_entered.store(true);
        // A failed host assertion must not strand the test runner indefinitely.
        const auto deadline=GetTickCount64()+10000;
        while(!auth_released.load()&&GetTickCount64()<deadline)Sleep(1);
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
    if(auto* file=_wfopen(cookie,L"wb")){std::fputs("fixture-cookie",file);std::fclose(file);}return &object;
}
extern "C" __declspec(dllexport) void FixtureMode(int m){
    mode.store(m);destroyed.store(0);
    hold_auth.store(false);auth_entered.store(false);auth_released.store(true);
}
extern "C" __declspec(dllexport) void FixtureHoldAuth(int enabled){
    auth_entered.store(false);auth_released.store(!enabled);hold_auth.store(enabled!=0);
}
extern "C" __declspec(dllexport) int FixtureAuthEntered(){return auth_entered.load()?1:0;}
extern "C" __declspec(dllexport) void FixtureReleaseAuth(){
    auth_released.store(true);hold_auth.store(false);
}
extern "C" __declspec(dllexport) int FixtureDestroyed(){return destroyed.load();}
