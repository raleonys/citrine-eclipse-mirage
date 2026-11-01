#define _WIN32 1
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include "../../src/platform/addon_hooks.cpp"

namespace {
std::string block="false", client="C:\\Client.exe", documents="C:\\Docs";
bool loaded=false;
int real_create_a=0, real_create_w=0, real_folder_a=0, real_folder_w=0;
DWORD last_error=0;
int hooks_created=0, hooks_removed=0;
BOOL original_a(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION) { ++real_create_a; return TRUE; }
BOOL original_w(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION) { ++real_create_w; return TRUE; }
HRESULT original_folder_a(HWND,int,HANDLE,DWORD,LPSTR) { ++real_folder_a;return 42; }
HRESULT original_folder_w(HWND,int,HANDLE,DWORD,LPWSTR) { ++real_folder_w;return 43; }
}
extern "C" {
void SetLastError(DWORD v){last_error=v;}
HMODULE GetModuleHandleW(LPCWSTR){return reinterpret_cast<HMODULE>(0x1111);}
HMODULE LoadLibraryW(LPCWSTR){return reinterpret_cast<HMODULE>(0x2222);}
BOOL FreeLibrary(HMODULE){return TRUE;}
FARPROC GetProcAddress(HMODULE,LPCSTR n) {
 if (!std::strcmp(n,"CreateProcessA")) return reinterpret_cast<FARPROC>(&original_a);
 if (!std::strcmp(n,"CreateProcessW")) return reinterpret_cast<FARPROC>(&original_w);
 if (!std::strcmp(n,"SHGetFolderPathA")) return reinterpret_cast<FARPROC>(&original_folder_a);
 if (!std::strcmp(n,"SHGetFolderPathW")) return reinterpret_cast<FARPROC>(&original_folder_w);
 return nullptr;
}
int MultiByteToWideChar(unsigned int,DWORD,LPCSTR s,int n,LPWSTR out,int cap) {
 if (n>cap) return 0;
 for (int i=0;i<n;++i) { if((unsigned char)s[i]>=128)return 0; out[i]=s[i]; }
 return n;
}
int WideCharToMultiByte(unsigned int,DWORD,LPCWSTR s,int n,LPSTR out,int cap,LPCSTR,BOOL* substit) {
 int len=0;while(s[len])++len; ++len;
 if(substit)*substit=FALSE;
 if(n!=-1)return 0;
 if(!out)return len;
 if(len>cap)return 0;
 for(int i=0;i<len;++i) { if(s[i]>127)return 0; out[i]=static_cast<char>(s[i]); }
 return len;
}
}
namespace preloader_l { namespace config {
bool runtime_configuration_loaded() noexcept{return ::loaded;}
bool begin_runtime_configuration_read(RuntimeConfigurationReadLease* l) noexcept {l->active=true;l->generation=1;return true;}
void end_runtime_configuration_read(RuntimeConfigurationReadLease* l) noexcept {l->active=false;}
ValueView runtime_configuration_setting(const char* n) noexcept {
 const std::string* s=nullptr;
 if(!std::strcmp(n,"BlockReruns"))s=&::block;
 if(!std::strcmp(n,"ClientPath"))s=&::client;
 if(!std::strcmp(n,"Documents"))s=&::documents;
 return s ? ValueView{s->data(),s->size(),true}:ValueView{nullptr,0,false};
}
} }
namespace preloader_l { namespace platform {
MinHookCallGuard::MinHookCallGuard(MinHookCallGate*) noexcept {}
MinHookCallGuard::~MinHookCallGuard() noexcept {}
uintptr_t minhook_gate_forwarding_target(MinHookCallGate* g) noexcept { return reinterpret_cast<uintptr_t>(g->forwarding_target); }
bool minhook_ticket_has_ownership(const MinHookTicket* t) noexcept {return t->created!=0;}
MinHookInstallStatus create_minhook(void* target,void* detour,MinHookTicket* ticket) noexcept {
 if(!target || !detour)return MinHookInstallStatus::target_unavailable;
 ticket->target=target;ticket->original=target;ticket->detour=detour;ticket->created=1;++::hooks_created;
 return MinHookInstallStatus::installed;
}
bool enable_minhook_batch(MinHookTicket* const* tickets,MinHookCallGate* const* gates,size_t n) noexcept {
 for(size_t i=0;i<n;++i){tickets[i]->enabled=1;gates[i]->forwarding_target=tickets[i]->original;}
 return true;
}
bool disable_minhook_batch(MinHookTicket* const* tickets,MinHookCallGate* const*,size_t n) noexcept {
 for(size_t i=0;i<n;++i)tickets[i]->enabled=0;
 return true;
}
bool remove_minhook(MinHookTicket* t) noexcept {if(!t->created)return true; ++::hooks_removed;*t=MinHookTicket{};return true;}
} }
int main(){
 assert(preloader_l::platform::stage_addon_hooks(3u));
 assert(hooks_created==4);
 assert(create_process_a("C:\\Client.exe",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr)==TRUE);
 assert(real_create_a==1); // config not yet published
 loaded=true;block="true";
 assert(create_process_a("c:\\CLIENT.EXE",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr)==FALSE);
 assert(last_error==ERROR_ACCESS_DENIED&&real_create_a==1);
 assert(create_process_a(nullptr,nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr)==TRUE);
 assert(real_create_a==2);
 assert(create_process_w(L"c:\\CLIENT.EXE",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr)==FALSE);
 assert(real_create_w==0);
 assert(create_process_w(L"NotClient",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr)==TRUE);
 assert(real_create_w==1);
 char patha[260]{};wchar_t pathw[260]{};
 assert(folder_path_a(nullptr,5,nullptr,0,patha)==S_OK);
 assert(!std::strcmp(patha,"C:\\Docs")&&real_folder_a==0);
 assert(folder_path_w(nullptr,5,nullptr,0,pathw)==S_OK);
 assert(!std::wcscmp(pathw,L"C:\\Docs")&&real_folder_w==0);
 assert(folder_path_a(nullptr,0x8005,nullptr,0,patha)==42);
 assert(real_folder_a==1);
 block="false";
 assert(create_process_a("C:\\Client.exe",nullptr,nullptr,nullptr,0,0,nullptr,nullptr,nullptr,nullptr)==TRUE);
 assert(real_create_a==3);
 documents=std::string(261,'x');
 assert(folder_path_w(nullptr,5,nullptr,0,pathw)==43);
 assert(real_folder_w==1);
 assert(preloader_l::platform::uninstall_addon_hooks());
 assert(hooks_removed==4);
 std::cout<<"HF17_OPTIONAL_HOOK_INTEGRATION PASS 4_HOOKS\n";
}
