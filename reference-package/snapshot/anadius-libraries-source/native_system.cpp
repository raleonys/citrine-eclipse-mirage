#include "libraries.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace anadius::recovered::libraries {
static_assert(sizeof(wchar_t) == 2, "This adapter requires Windows UTF-16 wchar_t");
std::uint32_t NativeSystemCalls::get_error_mode() { return GetErrorMode(); }
std::uint32_t NativeSystemCalls::set_error_mode(std::uint32_t mode) {
    return SetErrorMode(mode);
}
Module NativeSystemCalls::load_library(const wchar_t* path) {
    return reinterpret_cast<Module>(LoadLibraryW(path));
}
bool NativeSystemCalls::free_library(Module module) {
    return FreeLibrary(reinterpret_cast<HMODULE>(module)) != 0;
}
int NativeSystemCalls::wide_to_narrow(std::uint32_t codepage, std::uint32_t flags,
    const wchar_t* input, int count, char* destination, int capacity, bool* used_default) {
    BOOL native_default = FALSE;
    const int result = WideCharToMultiByte(codepage, flags, input, count,
        destination, capacity, nullptr, used_default ? &native_default : nullptr);
    if (used_default) *used_default = native_default != FALSE;
    return result;
}
std::uint32_t NativeSystemCalls::last_error() { return GetLastError(); }
} // namespace anadius::recovered::libraries
