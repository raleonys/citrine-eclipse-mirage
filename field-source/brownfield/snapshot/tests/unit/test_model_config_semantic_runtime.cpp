#include "preloader_l/runtime/service_policy.hpp"
#include "preloader_l/runtime/release_server.hpp"
#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/heap_allocator.hpp"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <cstdio>

namespace {
size_t g_allocations=0, g_frees=0;
size_t g_last_alloc_size=0, g_last_release_size=0;
bool g_fail_next=false, g_fail_release=false;
uintptr_t load_bits(const uint16_t storage[8]) {
    uintptr_t raw=0;
    auto* out=reinterpret_cast<unsigned char*>(&raw);
    const auto* in=reinterpret_cast<const unsigned char*>(storage);
    for(size_t i=0;i<sizeof(raw);++i) out[i]=in[i];
    return raw;
}
const uint16_t* path_data(const preloader_l::runtime::model_abi::ModelWidePathString32& p) {
    return p.capacity<=7u ? p.storage : reinterpret_cast<const uint16_t*>(load_bits(p.storage));
}
}

namespace preloader_l { namespace runtime {
AllocationResult allocate_block(size_t size, AllocationRetryPolicy,
                                const AllocatorPlatformOps&) {
    if (g_fail_next) {
        g_fail_next=false;
        return AllocationResult{nullptr, AllocationTerminal::bad_alloc, size, size, 0u, false};
    }
    void* p=std::malloc(size?size:1u);
    if(p) { ++g_allocations; g_last_alloc_size=size; }
    return AllocationResult{p,p?AllocationTerminal::none:AllocationTerminal::bad_alloc,
                            size,size,0u,false};
}
AllocationReleaseStatus release_block(void* ptr,size_t logical_size,const AllocatorPlatformOps&) noexcept {
    if(ptr){
        g_last_release_size=logical_size;
        if(g_fail_release) return AllocationReleaseStatus::release_failed;
        ++g_frees;std::free(ptr);
    }
    return AllocationReleaseStatus::ok;
}
const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept {
    static const AllocatorPlatformOps ops{};
    return ops;
}
} }

int main(){
    using namespace preloader_l::runtime;
    using namespace preloader_l::runtime::model_abi;
    ReleaseServiceObject548 live{}, staged{};
    ReleaseServerContext ctx{};
    assert(initialize_model_service_policy(&live,&ctx));
    assert(initialize_model_service_policy(&staged,nullptr));

    assert(live.config.document.node.document==reinterpret_cast<uintptr_t>(&live.config.document));
    assert(live.config.document.process_entities==1u);
    assert(live.config.document.error_id==0);
    assert(live.config.document.char_buffer==0u);
    assert(live.config.path.capacity==7u && live.config.path.size==0u);

    const char xml[]="\xEF\xBB\xBF<r>\n<a/>\n</r>";
    assert(commit_model_service_document(&staged,xml,sizeof(xml)-1u));
    assert(staged.config.document.char_buffer!=0u);
    assert(staged.config.document.write_bom==1u);
    assert(staged.config.document.process_entities==1u);
    assert(staged.config.document.error_id==0);
    assert(staged.config.document.parsing_depth==0);
    assert(staged.config.document.parse_cur_line_num==3);
    const char* staged_buf=reinterpret_cast<const char*>(staged.config.document.char_buffer);
    assert(std::memcmp(staged_buf,xml,sizeof(xml)-1u)==0);

    const uintptr_t old_buf=staged.config.document.char_buffer;
    g_fail_next=true;
    assert(!commit_model_service_document(&staged,"<x/>",4u));
    assert(staged.config.document.char_buffer==old_buf);

    // D02: explicit input length, including embedded NUL, owns the allocator
    // byte count. Release must use that exact recorded allocation size and a
    // failed release must preserve the document for a later retry.
    const char embedded_nul[]={'<','r','>','A','\0','B','<','/','r','>'};
    assert(commit_model_service_document(&staged,embedded_nul,sizeof(embedded_nul)));
    const size_t embedded_allocation=g_last_alloc_size;
    assert(embedded_allocation==sizeof(embedded_nul)+1u);
    const uintptr_t embedded_ptr=staged.config.document.char_buffer;
    const char* embedded_data=reinterpret_cast<const char*>(embedded_ptr);
    assert(std::memcmp(embedded_data,embedded_nul,sizeof(embedded_nul))==0);
    g_fail_release=true;
    assert(!release_model_service_policy_retryable(&staged));
    assert(staged.config.document.char_buffer==embedded_ptr);
    assert(g_last_release_size==embedded_allocation);
    g_fail_release=false;
    assert(release_model_service_policy_retryable(&staged));
    assert(staged.config.document.char_buffer==0u);
    assert(g_last_release_size==embedded_allocation);

    // Reinitialize staged because the retryable release above retired all
    // policy-owned members, then continue the existing swap coverage.
    assert(initialize_model_service_policy(&staged,nullptr));
    assert(commit_model_service_document(&staged,xml,sizeof(xml)-1u));

    const uint16_t sso_path[7]={'C',':','\\','G','a','m','e'};
    assert(commit_model_service_current_directory_wide(&live,sso_path,7u));
    assert(live.config.path.capacity==7u && live.config.path.size==7u);
    assert(std::memcmp(path_data(live.config.path),sso_path,sizeof(sso_path))==0);

    const uint16_t heap_path[8]={'C',':','\\','G','a','m','e','s'};
    assert(commit_model_service_current_directory_wide(&live,heap_path,8u));
    assert(live.config.path.size==8u);
    assert(live.config.path.capacity==15u);
    assert(std::memcmp(path_data(live.config.path),heap_path,sizeof(heap_path))==0);

    const uintptr_t staged_doc_buffer=staged.config.document.char_buffer;
    swap_model_service_configuration_policy(&live,&staged);
    assert(live.config.document.char_buffer==staged_doc_buffer);
    assert(live.config.document.node.document==reinterpret_cast<uintptr_t>(&live.config.document));
    assert(staged.config.document.node.document==reinterpret_cast<uintptr_t>(&staged.config.document));

    release_model_service_policy(&staged);
    release_model_service_policy(&live);
    assert(g_allocations==g_frees);
    std::puts("MODEL_CONFIG_SEMANTIC_RUNTIME: PASS");
    return 0;
}
