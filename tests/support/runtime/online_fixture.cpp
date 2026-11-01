// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cstdint>
#include <cstdio>
namespace {
struct Object {std::uintptr_t* table;};
int mode=0,destroyed=0;
void destroy(Object*,unsigned){++destroyed;}
int status(Object*){return mode;}
const char* username(Object*){return "fixture-online";}
const char* persona(Object*){return "fixture-persona";}
const char* user(Object*){return "fixture-user-id";}
bool avatar(Object*,const wchar_t* path){auto* file=_wfopen(path,L"wb");if(!file)return false;std::fputs("fixture-provider-avatar",file);std::fclose(file);return true;}
const char* auth(Object*,const char*){return "fixture-auth";}
std::uintptr_t table[]={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),
    reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&user),
    reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};
Object object{table};
}
extern "C" __declspec(dllexport) int version_check(){return 2;}
extern "C" __declspec(dllexport) Object* create_online(const wchar_t* cookie) {
    if(auto* file=_wfopen(cookie,L"wb")){std::fputs("fixture-cookie",file);std::fclose(file);}return &object;
}
extern "C" __declspec(dllexport) void FixtureMode(int m){mode=m;destroyed=0;}
extern "C" __declspec(dllexport) int FixtureDestroyed(){return destroyed;}
