#include "src/windows/hook_callbacks.hpp"
#include <bit>
#include <limits>
namespace preloader_l::native_callbacks {
using namespace registry;
Status OriginalEffects::original_open_a(const OpenA& a){return std::bit_cast<Status>(original<decltype(&::RegOpenKeyExA)>(Callback::reg_open_a_00f800)(reinterpret_cast<HKEY>(a.root),a.subkey,a.options,a.access,static_cast<PHKEY>(a.result.raw())));}
Status OriginalEffects::original_open_w(const OpenW& a){return std::bit_cast<Status>(original<decltype(&::RegOpenKeyExW)>(Callback::reg_open_w_00f960)(reinterpret_cast<HKEY>(a.root),reinterpret_cast<LPCWSTR>(a.subkey),a.options,a.access,static_cast<PHKEY>(a.result.raw())));}
Status OriginalEffects::original_query_a(const QueryA& a){return std::bit_cast<Status>(original<decltype(&::RegQueryValueExA)>(Callback::reg_query_a_00fac0)(reinterpret_cast<HKEY>(a.key),a.name,static_cast<LPDWORD>(a.reserved),static_cast<LPDWORD>(a.type.raw()),static_cast<LPBYTE>(a.data),static_cast<LPDWORD>(a.size.raw())));}
Status OriginalEffects::original_query_w(const QueryW& a){return std::bit_cast<Status>(original<decltype(&::RegQueryValueExW)>(Callback::reg_query_w_00fb60)(reinterpret_cast<HKEY>(a.key),reinterpret_cast<LPCWSTR>(a.name),static_cast<LPDWORD>(a.reserved),static_cast<LPDWORD>(a.type.raw()),static_cast<LPBYTE>(a.data),static_cast<LPDWORD>(a.size.raw())));}
Status OriginalEffects::original_close(Handle key){return std::bit_cast<Status>(original<decltype(&::RegCloseKey)>(Callback::reg_close_00f5b0)(reinterpret_cast<HKEY>(key)));}
std::int32_t OriginalEffects::original_create_process_a(const host_callbacks::ProcessCall<char>& a){return original<decltype(&::CreateProcessA)>(Callback::create_process_a_00f3f0)(a.application,a.command_line,reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.process_attributes),reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.thread_attributes),a.inherit_handles,a.creation_flags,reinterpret_cast<LPVOID>(a.environment),a.current_directory,reinterpret_cast<LPSTARTUPINFOA>(a.startup_info),reinterpret_cast<LPPROCESS_INFORMATION>(a.process_information));}
std::int32_t OriginalEffects::original_create_process_w(const host_callbacks::ProcessCall<char16_t>& a){return original<decltype(&::CreateProcessW)>(Callback::create_process_w_00f4d0)(reinterpret_cast<LPCWSTR>(a.application),reinterpret_cast<LPWSTR>(a.command_line),reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.process_attributes),reinterpret_cast<LPSECURITY_ATTRIBUTES>(a.thread_attributes),a.inherit_handles,a.creation_flags,reinterpret_cast<LPVOID>(a.environment),reinterpret_cast<LPCWSTR>(a.current_directory),reinterpret_cast<LPSTARTUPINFOW>(a.startup_info),reinterpret_cast<LPPROCESS_INFORMATION>(a.process_information));}
std::uint32_t OriginalEffects::original_folder_a(const host_callbacks::FolderCall<char>& a){return std::bit_cast<std::uint32_t>(original<decltype(&::SHGetFolderPathA)>(Callback::folder_a_00fc00)(reinterpret_cast<HWND>(a.window),a.folder_id,reinterpret_cast<HANDLE>(a.token),a.flags,a.path));}
std::uint32_t OriginalEffects::original_folder_w(const host_callbacks::FolderCall<char16_t>& a){return std::bit_cast<std::uint32_t>(original<decltype(&::SHGetFolderPathW)>(Callback::folder_w_00fc80)(reinterpret_cast<HWND>(a.window),a.folder_id,reinterpret_cast<HANDLE>(a.token),a.flags,reinterpret_cast<LPWSTR>(a.path)));}
std::int32_t OriginalEffects::original_connect_0a3838(network_hooks::Socket s,std::span<std::byte> b,std::int32_t n){return original<decltype(&::connect)>(Callback::connect_00fd00)(s,reinterpret_cast<const sockaddr*>(b.data()),n);}
int OriginalEffects::crt_wcsicmp(const char16_t* a,const char16_t* b){return wide_.compare_utf16(a,b);}
int OriginalEffects::crt_wcscpy_s(char16_t* d,std::size_t n,const char16_t* s){return wide_.copy_utf16(d,n,s);}
BOOL OriginalEffects::query(DWORD type,const void* object,DWORD content,DWORD format,DWORD flags,DWORD* encoding,DWORD* content_type,DWORD* format_type,HCERTSTORE* store,HCRYPTMSG* message,const void** context){
    const DWORD incoming=GetLastError();
    std::u16string path;bool readable=false;
    if(type==CERT_QUERY_OBJECT_FILE&&object){
        const auto base=reinterpret_cast<std::uintptr_t>(object);
        for(std::size_t i=0;;++i){
            if(i>(std::numeric_limits<std::uintptr_t>::max()-base)/sizeof(char16_t))break;
            char16_t unit{};SIZE_T copied=0;
            if(!ReadProcessMemory(GetCurrentProcess(),reinterpret_cast<const void*>(base+i*sizeof(char16_t)),&unit,sizeof unit,&copied)||copied!=sizeof unit)break;
            if(!unit){readable=true;break;}
            path.push_back(unit);
        }
    }
    const auto selection=crypt_query::select_object(type,object==nullptr,path,readable);
    const void* effective=selection==crypt_query::Selection::companion?static_cast<const void*>(crypt_query::companion_filename):object;
    SetLastError(incoming);
    // The actual Windows API supplies the result and every output. No synthetic
    // success, output rewriting, extra trust API, or cached certificate is used.
    const BOOL result=original<decltype(&::CryptQueryObject)>(Callback::crypt_query_001264)(type,effective,content,format,flags,encoding,content_type,format_type,store,message,context);
    const DWORD outgoing=GetLastError();crypt_observed(selection);SetLastError(outgoing);
    return result;
}
}
