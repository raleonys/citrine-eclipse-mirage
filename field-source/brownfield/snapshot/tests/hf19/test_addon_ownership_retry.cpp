#define EXPECT_PATCHED 1
#define _WIN32 1
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
// MSVC test-only API symbol renaming. These mocks must not collide with kernel32.lib.
// Keep the original source-included addon_hooks.cpp unchanged.
#define SetLastError HF18P03_TestSetLastError
#define GetModuleHandleW HF18P03_TestGetModuleHandleW
#define LoadLibraryW HF18P03_TestLoadLibraryW
#define FreeLibrary HF18P03_TestFreeLibrary
#define GetProcAddress HF18P03_TestGetProcAddress
#define MultiByteToWideChar HF18P03_TestMultiByteToWideChar
#define WideCharToMultiByte HF18P03_TestWideCharToMultiByte
#include "src/platform/addon_hooks.cpp"

namespace {
std::string block="false", client="C:\\Client.exe", documents="C:\\Docs";
bool loaded=false;
int real_create_a=0, real_create_w=0, real_folder_a=0, real_folder_w=0;
DWORD last_error=0;
int hooks_created=0, hooks_removed=0; int free_calls=0; bool fail_next_free=false; bool fail_folder_w_lookup=false; bool fail_next_remove=false; bool fail_next_disable=false;
BOOL original_a(LPCSTR,LPSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCSTR,LPSTARTUPINFOA,LPPROCESS_INFORMATION) { ++real_create_a; return TRUE; }
BOOL original_w(LPCWSTR,LPWSTR,LPSECURITY_ATTRIBUTES,LPSECURITY_ATTRIBUTES,BOOL,DWORD,LPVOID,LPCWSTR,LPSTARTUPINFOW,LPPROCESS_INFORMATION) { ++real_create_w; return TRUE; }
HRESULT original_folder_a(HWND,int,HANDLE,DWORD,LPSTR) { ++real_folder_a;return 42; }
HRESULT original_folder_w(HWND,int,HANDLE,DWORD,LPWSTR) { ++real_folder_w;return 43; }
}
extern "C" {
void SetLastError(DWORD v){last_error=v;}
HMODULE GetModuleHandleW(LPCWSTR name){return std::wcscmp(name,L"shell32.dll")==0 ? nullptr : reinterpret_cast<HMODULE>(0x1111);}
HMODULE LoadLibraryW(LPCWSTR){return reinterpret_cast<HMODULE>(0x2222);}
BOOL FreeLibrary(HMODULE){++free_calls; if(fail_next_free){fail_next_free=false;return FALSE;} return TRUE;}
FARPROC GetProcAddress(HMODULE,LPCSTR n) {
 if (!std::strcmp(n,"CreateProcessA")) return reinterpret_cast<FARPROC>(&original_a);
 if (!std::strcmp(n,"CreateProcessW")) return reinterpret_cast<FARPROC>(&original_w);
 if (!std::strcmp(n,"SHGetFolderPathA")) return reinterpret_cast<FARPROC>(&original_folder_a);
 if (!std::strcmp(n,"SHGetFolderPathW")) return fail_folder_w_lookup ? nullptr : reinterpret_cast<FARPROC>(&original_folder_w);
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
 if(::fail_next_disable){::fail_next_disable=false;return false;}
 return true;
}
bool remove_minhook(MinHookTicket* t) noexcept {if(!t->created)return true; if(::fail_next_remove){::fail_next_remove=false;return false;} ++::hooks_removed;*t=MinHookTicket{};return true;}
} }

#ifndef EXPECT_PATCHED
#define EXPECT_PATCHED 1
#endif
static int checks=0;
#define CHECK(x) do { ++checks; if(!(x)) { std::cerr << "CHECK_FAIL:" << __LINE__ << " " << #x << "\n"; return 30; } } while(0)
static void reset_case() {
 g_addon=AddonHookState{};
 hooks_created=hooks_removed=free_calls=0;
 fail_next_free=fail_folder_w_lookup=fail_next_remove=fail_next_disable=false;
}
int main(){
 // A: shutdown FreeLibrary(FALSE), including retry boundary.
 reset_case();
 CHECK(preloader_l::platform::stage_addon_hooks(2u)); CHECK(g_addon.shell_owned);
 fail_next_free=true; auto first=preloader_l::platform::uninstall_addon_hooks();
#if EXPECT_PATCHED
 CHECK(!first); CHECK(g_addon.shell_owned);CHECK(g_addon.recovery_pending);CHECK(g_addon.shell!=nullptr);
 CHECK(free_calls==1);CHECK(preloader_l::platform::uninstall_addon_hooks());CHECK(free_calls==2);
 CHECK(!g_addon.shell_owned);CHECK(!g_addon.recovery_pending);
#else
 CHECK(first);CHECK(!g_addon.shell_owned);CHECK(g_addon.shell==nullptr);CHECK(free_calls==1);
 CHECK(preloader_l::platform::uninstall_addon_hooks());CHECK(free_calls==1);
#endif
 std::cout<<"SCENARIO_A_SHUTDOWN_FAILED_FREE OBSERVED expected_patched="<<EXPECT_PATCHED<<"\n";
 // B: failure in stage after shell LoadLibrary owned, FreeLibrary(FALSE) in unwind.
 reset_case(); fail_folder_w_lookup=true;fail_next_free=true;
 CHECK(!preloader_l::platform::stage_addon_hooks(2u));CHECK(free_calls==1);
#if EXPECT_PATCHED
 CHECK(g_addon.shell_owned);CHECK(g_addon.recovery_pending);CHECK(g_addon.shell!=nullptr);
 fail_folder_w_lookup=false;
 CHECK(preloader_l::platform::uninstall_addon_hooks());CHECK(free_calls==2);
 CHECK(!g_addon.shell_owned);CHECK(!g_addon.recovery_pending);
#else
 CHECK(!g_addon.shell_owned);CHECK(g_addon.shell==nullptr);CHECK(!g_addon.recovery_pending);
 CHECK(preloader_l::platform::uninstall_addon_hooks());CHECK(free_calls==1);
#endif
 std::cout<<"SCENARIO_B_STAGE_UNWIND_FAILED_FREE OBSERVED expected_patched="<<EXPECT_PATCHED<<"\n";
 // C: remove-mihook injected first failure; owner must remain, retry drains correctly.
 reset_case(); CHECK(preloader_l::platform::stage_addon_hooks(2u));fail_next_remove=true;
 CHECK(!preloader_l::platform::uninstall_addon_hooks());CHECK(g_addon.recovery_pending);CHECK(g_addon.shell_owned);CHECK(free_calls==0);
 CHECK(preloader_l::platform::uninstall_addon_hooks());CHECK(!g_addon.recovery_pending);CHECK(free_calls==1);
 std::cout<<"SCENARIO_C_REMOVE_RETRY PASS\n";
 // D: disable batch injected failure; must keep owner and allow retry.
 reset_case(); CHECK(preloader_l::platform::stage_addon_hooks(2u));fail_next_disable=true;
 CHECK(!preloader_l::platform::uninstall_addon_hooks());CHECK(g_addon.recovery_pending);CHECK(g_addon.shell_owned);CHECK(free_calls==0);
 CHECK(preloader_l::platform::uninstall_addon_hooks());CHECK(!g_addon.recovery_pending);CHECK(free_calls==1);
 std::cout<<"SCENARIO_D_DISABLE_RETRY PASS\n";
 std::cout << "SOURCE_LINKED_ADDON_FAULT_MATRIX PASS " << checks << " assertions, test_only_patched=" << EXPECT_PATCHED << "\n";
 return 0;
}
