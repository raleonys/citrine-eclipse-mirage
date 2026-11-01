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
bool g_fail_next=false;
}

namespace preloader_l { namespace runtime {
AllocationResult allocate_block(size_t size, AllocationRetryPolicy,
                                const AllocatorPlatformOps&) {
    if (g_fail_next) {
        g_fail_next=false;
        return AllocationResult{nullptr, AllocationTerminal::bad_alloc, size, size, 0u, false};
    }
    void* p = std::malloc(size ? size : 1u);
    if (p) ++g_allocations;
    return AllocationResult{p, p ? AllocationTerminal::none : AllocationTerminal::bad_alloc,
                            size, size, 0u, false};
}
AllocationReleaseStatus release_block(void* ptr, size_t, const AllocatorPlatformOps&) noexcept {
    if (ptr) { ++g_frees; std::free(ptr); }
    return AllocationReleaseStatus::ok;
}
const AllocatorPlatformOps& windows_allocator_platform_ops() noexcept {
    static const AllocatorPlatformOps ops{};
    return ops;
}
} }

static preloader_l::runtime::ModelSettingInput in(const char* k,const char* v) {
    return {k,std::strlen(k),v,std::strlen(v)};
}

int main() {
    using namespace preloader_l::runtime;
    using namespace preloader_l::runtime::model_abi;
    ReleaseServiceObject548 service{};
    ReleaseServerContext context{};
    assert(initialize_model_service_policy(&service,&context));
    assert(service.config.settings.max_load_factor == 1.0f);
    assert(service.config.settings.size == 0u);
    assert(service.config.settings.bucket_count == 8u);
    assert(service.config.settings.mask == 7u);
    assert(service.config.settings.sentinel != 0u);
    assert(service.config.settings.bucket_pairs.begin != 0u);

    ModelSettingInput ten[] = {
        in("k0","v0"),in("k1","v1"),in("k2","v2"),in("k3","v3"),in("k4","v4"),
        in("k5","v5"),in("k6","v6"),in("k7","v7"),in("k8","v8"),in("k9","v9")
    };
    assert(commit_model_service_settings(&service,ten,10));
    assert(service.config.settings.size == 10u);
    assert(service.config.settings.bucket_count == 64u);
    assert(service.config.settings.mask == 63u);
    auto v = model_service_setting(&service,"k9");
    assert(v.present && v.size==2 && std::memcmp(v.data,"v9",2)==0);

    ModelSettingInput dup[] = {in("same","first"),in("same","second"),in("other","x")};
    assert(commit_model_service_settings(&service,dup,3));
    assert(service.config.settings.size == 2u);
    v = model_service_setting(&service,"same");
    assert(v.present && v.size==6 && std::memcmp(v.data,"second",6)==0);
    assert(!model_service_configuration_loaded(&service));
    set_model_service_configuration_state(&service,true,0x15u);
    assert(model_service_configuration_loaded(&service));
    assert(model_service_present_sections(&service)==0x15u);

    // Candidate construction failure must leave the currently published map untouched.
    ModelSettingInput replacement[] = {in("new","value")};
    g_fail_next=true;
    assert(!commit_model_service_settings(&service,replacement,1));
    assert(service.config.settings.size == 2u);
    v = model_service_setting(&service,"same");
    assert(v.present && v.size==6 && std::memcmp(v.data,"second",6)==0);

    release_model_service_policy(&service);
    assert(service.config.settings.sentinel==0u);
    assert(service.config.settings.bucket_pairs.begin==0u);
    assert(g_allocations==g_frees);
    std::puts("MODEL_SETTINGS_SEMANTIC_RUNTIME: PASS");
    return 0;
}
