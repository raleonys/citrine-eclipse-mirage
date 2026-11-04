// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/codec/encoding.hpp"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <locale.h>
#include <cstring>
#include <stdexcept>

namespace preloader_l::encoding {
static_assert(sizeof(wchar_t)==2);
std::uint32_t NativeCalls::locale_codepage_0629ec(){return ___lc_codepage_func();}
bool NativeCalls::are_file_apis_ansi(){return AreFileApisANSI()!=FALSE;}
std::uint32_t NativeCalls::last_error(){return GetLastError();}
int NativeCalls::multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* input,int size,char16_t* output,int capacity) {
    if(size<0||capacity<0)throw std::invalid_argument("Native conversion adapter supports the evidenced nonnegative-length route");
    int count;DWORD saved;const DWORD entry_error=GetLastError();
    {
        std::wstring buffer;
        if(output&&capacity) {
            buffer.resize(static_cast<std::size_t>(capacity));
            // The Win32 destination is output-only; do not read caller storage.
        }
        SetLastError(entry_error);
        count=MultiByteToWideChar(cp,flags,input,size,buffer.empty()?nullptr:buffer.data(),capacity);
        saved=GetLastError();
        // Win32 may partially fill the temporary on failure; commit only success.
        // Size queries and failures leave caller storage untouched.
        if(output&&capacity>0&&count>0)std::memcpy(output,buffer.data(),static_cast<std::size_t>(count)*sizeof(char16_t));
    }
    // Preserve API error across compatibility-buffer allocation/destruction.
    SetLastError(saved);return count;
}
int NativeCalls::wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* input,int size,char* output,int capacity,bool* used_default) {
    if(size<0||capacity<0)throw std::invalid_argument("Native conversion adapter supports the evidenced nonnegative-length route");
    int count;DWORD saved;BOOL replaced=FALSE;const DWORD entry_error=GetLastError();
    {
        std::wstring buffer;
        if(input)buffer.assign(input,input+size);
        SetLastError(entry_error);
        count=WideCharToMultiByte(cp,flags,input?buffer.data():nullptr,size,output,capacity,nullptr,used_default?&replaced:nullptr);
        saved=GetLastError();
    }
    if(used_default)*used_default=replaced!=FALSE;
    SetLastError(saved);return count;
}
} // namespace preloader_l::encoding
