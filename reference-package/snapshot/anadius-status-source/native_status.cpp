#include "status.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cstdlib>
namespace anadius::status_recovered {
namespace {
const wchar_t* wide(const Path& path) { static_assert(sizeof(wchar_t) == sizeof(char16_t)); return reinterpret_cast<const wchar_t*>(path.c_str()); }
HANDLE handle(Handle value) { return reinterpret_cast<HANDLE>(value); }
std::uint64_t time_value(FILETIME time) { return (std::uint64_t(time.dwHighDateTime) << 32) | time.dwLowDateTime; }
}
std::uint32_t NativeSystemCalls::attributes_ex(const Path& path, Metadata& out) {
    WIN32_FILE_ATTRIBUTE_DATA data;
    if (!GetFileAttributesExW(wide(path), GetFileExInfoStandard, &data)) return GetLastError();
    out = {data.dwFileAttributes, time_value(data.ftLastWriteTime), (std::uint64_t(data.nFileSizeHigh) << 32) | data.nFileSizeLow};
    return 0;
}
FindResult NativeSystemCalls::find_first(const Path& path) {
    WIN32_FIND_DATAW data;
    const auto found = FindFirstFileW(wide(path), &data);
    if (found == INVALID_HANDLE_VALUE) return {invalid_handle, GetLastError(), {}};
    return {reinterpret_cast<Handle>(found), 0, {data.dwFileAttributes, time_value(data.ftLastWriteTime), (std::uint64_t(data.nFileSizeHigh) << 32) | data.nFileSizeLow}};
}
bool NativeSystemCalls::find_close(Handle value) { return FindClose(handle(value)) != FALSE; }
OpenResult NativeSystemCalls::open_attributes(const Path& path, std::uint32_t access, std::uint32_t share, std::uint32_t disposition, std::uint32_t flags) {
    const auto opened = CreateFileW(wide(path), access, share, nullptr, disposition, flags, nullptr);
    return {reinterpret_cast<Handle>(opened), opened == INVALID_HANDLE_VALUE ? GetLastError() : 0};
}
std::uint32_t NativeSystemCalls::basic_information(Handle value, Metadata& out) {
    FILE_BASIC_INFO info;
    if (!GetFileInformationByHandleEx(handle(value), FileBasicInfo, &info, 0x28)) return GetLastError();
    out.attributes = info.FileAttributes;
    out.last_write = static_cast<std::uint64_t>(info.LastWriteTime.QuadPart);
    return 0;
}
bool NativeSystemCalls::close(Handle value) { return CloseHandle(handle(value)) != FALSE; }
[[noreturn]] void NativeSystemCalls::abort_close_failure_05f03c() { std::abort(); }
}
