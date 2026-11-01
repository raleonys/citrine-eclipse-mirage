// Own synthetic DLL only. No original target code/data is included or loaded.
#include <cstdint>
#include <windows.h>
namespace {
struct Object {std::uintptr_t* table;};
unsigned destroys=0;
void destroy(void*){++destroys;}
std::int32_t status(void*){return 23;}
const char* username(void*){return "reference-fixture";}
const char* persona(void*){return "own-persona";}
const char* uid(void*){return "own-id";}
std::uint8_t avatar(void*,const char16_t*,const char*,std::int32_t){return 0;}
const char* auth(void*,const char*){return "own-auth";}
std::uintptr_t table[]={reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&uid),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};
Object object{table};
}
extern "C" __declspec(dllexport) std::int32_t version_check(){return 2;}
extern "C" __declspec(dllexport) std::uintptr_t create_online(const char16_t*){return reinterpret_cast<std::uintptr_t>(&object);}
extern "C" __declspec(dllexport) unsigned destroyed_count(){return destroys;}
