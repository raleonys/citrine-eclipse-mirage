#include "preloader_l/platform/file_copy.hpp"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <cstdlib>

namespace preloader_l::file_copy {
namespace {
const wchar_t* wide(const Path& path) {
    static_assert(sizeof(wchar_t)==sizeof(char16_t));
    return reinterpret_cast<const wchar_t*>(path.c_str());
}
HANDLE native(Handle handle) {return reinterpret_cast<HANDLE>(handle);}
}
Result NativeSystemCalls::copy(const Path& source,const Path& target,bool fail_if_exists) {
    if(CopyFileW(wide(source),wide(target),fail_if_exists?TRUE:FALSE))return {true,0};
    return {false,GetLastError()};
}
OpenResult NativeSystemCalls::open_attributes(const Path& path,std::uint32_t share) {
    const auto handle=CreateFileW(wide(path),FILE_READ_ATTRIBUTES,share,nullptr,
                                  OPEN_EXISTING,0,nullptr);
    return {reinterpret_cast<Handle>(handle),handle==INVALID_HANDLE_VALUE?GetLastError():0};
}
std::uint32_t NativeSystemCalls::file_id_info(Handle handle,Identity& out) {
    static_assert(sizeof(Identity)==24);
    // Use the recovered FileIdInfo value/layout without raising the project's
    // Windows target macro just to expose newer SDK type names.
    if(GetFileInformationByHandleEx(native(handle),static_cast<FILE_INFO_BY_HANDLE_CLASS>(0x12),
                                   &out,sizeof(out)))return 0;
    return GetLastError();
}
std::uint32_t NativeSystemCalls::legacy_file_info(Handle handle,LegacyIdentity& out) {
    BY_HANDLE_FILE_INFORMATION info{};
    if(!GetFileInformationByHandle(native(handle),&info))return GetLastError();
    out={info.dwVolumeSerialNumber,info.nFileIndexHigh,info.nFileIndexLow};
    return 0;
}
std::uint32_t NativeSystemCalls::last_write(Handle handle,std::int64_t& out) {
    FILE_BASIC_INFO info{};
    static_assert(sizeof(info)==40);
    if(!GetFileInformationByHandleEx(native(handle),FileBasicInfo,&info,sizeof(info)))
        return GetLastError();
    out=info.LastWriteTime.QuadPart;
    return 0;
}
bool NativeSystemCalls::close(Handle handle) {return CloseHandle(native(handle))!=FALSE;}
[[noreturn]] void NativeSystemCalls::abort_close_failure() {std::abort();}
Result copy_file_windows_048d20(const Path& source,const Path& target,std::uint32_t options) {
    NativeSystemCalls system;
    return copy_file_048d20(source,target,options,system);
}
}
