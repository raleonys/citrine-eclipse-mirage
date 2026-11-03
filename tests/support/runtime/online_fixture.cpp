// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <string_view>
namespace {
struct Object {std::uintptr_t* table;};
int mode=0,destroyed=0;
void destroy(Object*){++destroyed;}
int status(Object*){return mode;}
const char* username(Object*){return "fixture-online";}
const char* persona(Object*){return "fixture-persona";}
const char* user(Object*){return "fixture-user-id";}
std::uint8_t avatar(Object*,const wchar_t* path,const char* owner,std::int32_t size){
    if(!owner||std::string_view(owner)!="fixture-user-id"||(size!=0&&size!=2))return 0;
    auto* file=_wfopen(path,L"wb");if(!file)return 0;std::fputs("fixture-provider-avatar",file);std::fclose(file);return 0x80;
}
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
