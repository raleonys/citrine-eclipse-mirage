#include "preloader_l/config/runtime_configuration_file.hpp"
#include "preloader_l/config/runtime_configuration.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace {
unsigned long g_last_error=2u;
int g_read_calls=0;
void* bad_handle(){ return reinterpret_cast<void*>(static_cast<intptr_t>(-1)); }
void* heap_alloc(void*,size_t n) noexcept { return std::malloc(n); }
int heap_free(void*,void* p) noexcept { std::free(p); return 1; }
int new_handler(void*,size_t){ return 0; }
}

extern "C" {
void* CreateFileA(const char*,unsigned long,unsigned long,void*,unsigned long,unsigned long,void*) { return bad_handle(); }
void* CreateFileW(const uint16_t*,unsigned long,unsigned long,void*,unsigned long,unsigned long,void*) { return bad_handle(); }
int ReadFile(void*,void*,unsigned long,unsigned long*,void*) { ++g_read_calls; return 0; }
int CloseHandle(void*) { return 1; }
unsigned long GetLastError() { return g_last_error; }
}

namespace preloader_l { namespace runtime {
const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept {
    static const AllocatorPlatformOps ops{nullptr,&heap_alloc,&heap_free,&new_handler};
    return ops;
}
AllocationResult allocate_block(size_t size,AllocationRetryPolicy,const AllocatorPlatformOps& ops){
    void* p=ops.heap_alloc?ops.heap_alloc(ops.context,size):nullptr;
    return AllocationResult{p,p?AllocationTerminal::none:AllocationTerminal::bad_alloc,size,size,0u,false};
}
AllocationReleaseStatus release_block(void* p,size_t,const AllocatorPlatformOps& ops) noexcept {
    if(!p) return AllocationReleaseStatus::ok;
    return ops.heap_free&&ops.heap_free(ops.context,p)?AllocationReleaseStatus::ok:AllocationReleaseStatus::release_failed;
}
} }

namespace preloader_l { namespace config {
RuntimeConfigurationStatus set_runtime_configuration_cfg(const char*,size_t) noexcept { return RuntimeConfigurationStatus::ok; }
} }

int main(){
    using preloader_l::config::RuntimeConfigurationFileStatus;
    using preloader_l::config::load_runtime_configuration_file;
    using preloader_l::config::load_runtime_configuration_file_wide;
    constexpr uint16_t wide_path[]={'C',':','\\','x','.','c','f','g',0};

    g_last_error=2u;
    if(load_runtime_configuration_file("missing.cfg")!=RuntimeConfigurationFileStatus::not_found) return 1;
    g_last_error=3u;
    if(load_runtime_configuration_file("missing-dir\\x.cfg")!=RuntimeConfigurationFileStatus::not_found) return 2;
    g_last_error=5u;
    if(load_runtime_configuration_file("denied.cfg")!=RuntimeConfigurationFileStatus::io_error) return 3;
    g_last_error=32u;
    if(load_runtime_configuration_file("shared.cfg")!=RuntimeConfigurationFileStatus::io_error) return 4;
    g_last_error=5u;
    if(load_runtime_configuration_file_wide(wide_path)!=RuntimeConfigurationFileStatus::io_error) return 5;
    if(g_read_calls!=0) return 6;
    return 0;
}
