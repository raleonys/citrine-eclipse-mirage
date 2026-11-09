// SPDX-License-Identifier: GPL-3.0-only

#include "citrine_eclipse_mirage/platform/hook_native_callbacks.hpp"
#include <bit>
namespace citrine_eclipse_mirage::native {
using namespace registry;
std::uint32_t OriginalEffects::increment_attempts(std::uint32_t& value) {
    const auto incoming=GetLastError();
    std::uint32_t result;
    {std::lock_guard lock(attempts_gate_);result=++value;}
    SetLastError(incoming);
    return result;
}
BOOL OriginalEffects::crypt_query(DWORD type,const void* object,DWORD content,DWORD format,
    DWORD flags,DWORD* encoding,DWORD* content_type,DWORD* format_type,
    HCERTSTORE* store,HCRYPTMSG* message,const void** context) {
    const DWORD incoming=GetLastError();
    hooks::PathProbe path;
    if(type==CERT_QUERY_OBJECT_FILE&&object)path=crypt_host_.inspect_path(object);
    const auto selection=hooks::select_object(type,object==nullptr,path);
    const void* effective=selection==hooks::Selection::companion?static_cast<const void*>(hooks::companion_filename):object;
    SetLastError(incoming);
    const BOOL result=original<decltype(&::CryptQueryObject)>(Callback::crypt_query_001264)(type,effective,content,format,flags,encoding,content_type,format_type,store,message,context);
    const DWORD outgoing=GetLastError();crypt_observed(selection);SetLastError(outgoing);
    return result;
}
Status OriginalEffects::original_open_a(const OpenA& a){return std::bit_cast<Status>(original<decltype(&::RegOpenKeyExA)>(Callback::reg_open_a_00f800)(reinterpret_cast<HKEY>(a.root),a.subkey,a.options,a.access,static_cast<PHKEY>(a.result.raw())));}
Status OriginalEffects::original_open_w(const OpenW& a){return std::bit_cast<Status>(original<decltype(&::RegOpenKeyExW)>(Callback::reg_open_w_00f960)(reinterpret_cast<HKEY>(a.root),reinterpret_cast<LPCWSTR>(a.subkey),a.options,a.access,static_cast<PHKEY>(a.result.raw())));}
Status OriginalEffects::original_query_a(const QueryA& a){return std::bit_cast<Status>(original<decltype(&::RegQueryValueExA)>(Callback::reg_query_a_00fac0)(reinterpret_cast<HKEY>(a.key),a.name,static_cast<LPDWORD>(a.reserved),static_cast<LPDWORD>(a.type.raw()),static_cast<LPBYTE>(a.data),static_cast<LPDWORD>(a.size.raw())));}
Status OriginalEffects::original_query_w(const QueryW& a){return std::bit_cast<Status>(original<decltype(&::RegQueryValueExW)>(Callback::reg_query_w_00fb60)(reinterpret_cast<HKEY>(a.key),reinterpret_cast<LPCWSTR>(a.name),static_cast<LPDWORD>(a.reserved),static_cast<LPDWORD>(a.type.raw()),static_cast<LPBYTE>(a.data),static_cast<LPDWORD>(a.size.raw())));}
Status OriginalEffects::original_close(Handle key){return std::bit_cast<Status>(original<decltype(&::RegCloseKey)>(Callback::reg_close_00f5b0)(reinterpret_cast<HKEY>(key)));}
std::int32_t OriginalEffects::original_create_process_a(const host_callbacks::ProcessCall<char>& a){return original<decltype(&::CreateProcessA)>(Callback::create_process_a_00f3f0)(a.application,a.command_line,reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.process_attributes),reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.thread_attributes),a.inherit_handles,a.creation_flags,reinterpret_cast<LPVOID>(a.environment),a.current_directory,reinterpret_cast<LPSTARTUPINFOA>(a.startup_info),reinterpret_cast<LPPROCESS_INFORMATION>(a.process_information));}
std::int32_t OriginalEffects::original_create_process_w(const host_callbacks::ProcessCall<char16_t>& a){return original<decltype(&::CreateProcessW)>(Callback::create_process_w_00f4d0)(reinterpret_cast<LPCWSTR>(a.application),reinterpret_cast<LPWSTR>(a.command_line),reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.process_attributes),reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.thread_attributes),a.inherit_handles,a.creation_flags,reinterpret_cast<LPVOID>(a.environment),reinterpret_cast<LPCWSTR>(a.current_directory),reinterpret_cast<LPSTARTUPINFOW>(a.startup_info),reinterpret_cast<LPPROCESS_INFORMATION>(a.process_information));}
std::uint32_t OriginalEffects::original_folder_a(const host_callbacks::FolderCall<char>& a){return std::bit_cast<std::uint32_t>(original<decltype(&::SHGetFolderPathA)>(Callback::folder_a_00fc00)(reinterpret_cast<HWND>(a.window),a.folder_id,reinterpret_cast<HANDLE>(a.token),a.flags,a.path));}
std::uint32_t OriginalEffects::original_folder_w(const host_callbacks::FolderCall<char16_t>& a){return std::bit_cast<std::uint32_t>(original<decltype(&::SHGetFolderPathW)>(Callback::folder_w_00fc80)(reinterpret_cast<HWND>(a.window),a.folder_id,reinterpret_cast<HANDLE>(a.token),a.flags,reinterpret_cast<LPWSTR>(a.path)));}
std::int32_t OriginalEffects::original_connect_0a3838(network_hook::Socket s,std::span<const std::byte> b,std::int32_t n){return original<decltype(&::connect)>(Callback::connect_00fd00)(s,reinterpret_cast<const sockaddr*>(b.data()),n);}
int OriginalEffects::crt_wcsicmp(const char16_t* a,const char16_t* b){return wide_.compare_utf16(a,b);}
int OriginalEffects::crt_wcscpy_s(char16_t* d,std::size_t n,const char16_t* s){return wide_.copy_utf16(d,n,s);}
}
