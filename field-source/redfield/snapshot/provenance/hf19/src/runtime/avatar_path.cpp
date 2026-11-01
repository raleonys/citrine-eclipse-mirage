#if !defined(_WIN32)
#error avatar_path.cpp is Windows-only.
#endif

#include "preloader_l/runtime/avatar_path.hpp"
#include "preloader_l/runtime/runtime_state.hpp"
#include "../config/module_relative_filesystem.hpp"
#include <stddef.h>
#include <stdint.h>
#include <windows.h>  // Win32 API ABI must use Windows SDK prototypes.

namespace {
static_assert(sizeof(WCHAR)==sizeof(uint16_t),"Windows UTF-16 ABI mismatch");
constexpr uint16_t kAsset[]={'s','u','r','g','e','.','p','n','g',0u};
constexpr size_t kPathCapacity=32768u;
constexpr unsigned int kCpUtf8=65001u;
constexpr unsigned long kWcErrInvalidChars=0x80u;

struct HeapBytes {
    void* heap; void* data;
    HeapBytes() noexcept:heap(nullptr),data(nullptr){}
    ~HeapBytes() noexcept { if(heap&&data)(void)HeapFree(heap,0,data); }
    bool allocate(size_t bytes) noexcept {
        if(!bytes)return false;
        heap=GetProcessHeap();if(!heap)return false;
        data=HeapAlloc(heap,0,bytes);
        return data!=nullptr;
    }
};
}

namespace preloader_l { namespace runtime {

bool normalized_avatar_path(char* output,size_t capacity,size_t* written) noexcept {
    if(written)*written=0;
    if(!written||(!output&&capacity))return false;

    HeapBytes storage;
    if(!storage.allocate(kPathCapacity*sizeof(uint16_t)))return false;
    auto* path=static_cast<uint16_t*>(storage.data);

    const auto state=snapshot_state();
    if(!state.module_base)return false;

    const DWORD n=GetModuleFileNameW(
        reinterpret_cast<HMODULE>(state.module_base),reinterpret_cast<LPWSTR>(path),
        static_cast<DWORD>(kPathCapacity));
    if(!n||static_cast<size_t>(n)>=kPathCapacity)return false;

    constexpr size_t asset_units=(sizeof(kAsset)/sizeof(kAsset[0]))-1u;
    size_t logical_wide=0;
    if(!preloader_l::config::path_detail::join_module_relative(
           path,static_cast<size_t>(n),kAsset,asset_units,
           path,kPathCapacity,&logical_wide)) return false;

    if(logical_wide>0x7fffffffu)return false;
    const int needed=WideCharToMultiByte(
        kCpUtf8,kWcErrInvalidChars,reinterpret_cast<LPCWCH>(path),static_cast<int>(logical_wide),
        nullptr,0,nullptr,nullptr);
    if(needed<=0)return false;
    *written=static_cast<size_t>(needed);
    if(!output||capacity<=static_cast<size_t>(needed))return false;

    const int got=WideCharToMultiByte(
        kCpUtf8,kWcErrInvalidChars,reinterpret_cast<LPCWCH>(path),static_cast<int>(logical_wide),
        output,needed,nullptr,nullptr);
    if(got!=needed){output[0]='\0';return false;}
    output[needed]='\0';
    return true;
}

int32_t query_image_size_bucket(int32_t width) noexcept { return width/208; }

} }
