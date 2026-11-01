// SPDX-License-Identifier: GPL-3.0-only
// TEST_ONLY version-2 native provider. No network or real account is used.
#include <cstdint>
#include "src/application/avatar_assets.hpp"
#include <filesystem>
#include <fstream>
#include <windows.h>
namespace {
struct Account {const std::uintptr_t* table;};
void destroy(void* value){delete static_cast<Account*>(value);}
std::int32_t status(void*){char value[2]{};return GetEnvironmentVariableA("PRELOADER_L_FIXTURE_AUTH_FAIL",value,2)&&value[0]=='1'?1:0;}
const char* username(void*){return "FixtureOnlineUser";}
const char* persona(void*){return "31001";}
const char* user_id(void*){return "32001";}
std::uint8_t avatar(void*,const wchar_t* path,const char* owner,std::int32_t size){
 std::ofstream file(std::filesystem::path(path),std::ios::binary);file.write(reinterpret_cast<const char*>(preloader_l::assets::surge_png),sizeof preloader_l::assets::surge_png);
 char record[32768]{};const auto n=GetEnvironmentVariableA("PRELOADER_L_FIXTURE_AVATAR_RECORD",record,sizeof record);if(n&&n<sizeof record){std::ofstream log(record,std::ios::app);log<<owner<<":"<<size<<"\n";}return file?1:0;
}
const char* auth(void*,const char*){return "SYNTHETIC_ONLINE_AUTH";}
const std::uintptr_t table[]={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&user_id),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};
}
extern "C" __declspec(dllexport) std::int32_t version_check(){char value[2]{};return GetEnvironmentVariableA("PRELOADER_L_FIXTURE_BAD_VERSION",value,2)&&value[0]=='1'?1:2;}
extern "C" __declspec(dllexport) void* create_online(const wchar_t* cookie){char value[2]{};if(GetEnvironmentVariableA("PRELOADER_L_FIXTURE_NULL_FACTORY",value,2)&&value[0]=='1')return nullptr;std::ofstream file(std::filesystem::path(cookie),std::ios::binary);file<<"synthetic-cookie";return new Account{table};}
