#include "preloader_l/runtime/memory_allocator.hpp"
#include "preloader_l/runtime/exception_policy.hpp"
#include "preloader_l/runtime/exception_runtime.hpp"
#include "preloader_l/runtime/fatal_policy.hpp"
#include "preloader_l/runtime/ptd_runtime.hpp"
#include "allocator_ptd_support.hpp"

#include <cassert>
#include <cstdint>
#include <new>
#include <vector>

using namespace preloader_l::runtime;
namespace {
struct FatalLog { std::vector<uint32_t> events; };
bool no_fastfail(void*) noexcept { return false; }
void mock_fastfail(void*,uint32_t) {}
void mock_clear(void* p) noexcept { static_cast<FatalLog*>(p)->events.push_back(1u); }
void mock_report(void* p,uint32_t status,uint32_t flags,const void*) noexcept {
    auto* l=static_cast<FatalLog*>(p); l->events.push_back(status); l->events.push_back(flags);
}
void mock_term(void* p,uint32_t status) noexcept { static_cast<FatalLog*>(p)->events.push_back(status); }
void mock_exit(void* p,uint32_t status) noexcept { static_cast<FatalLog*>(p)->events.push_back(status); }
FatalPlatformOps fatal_ops(FatalLog& l) {
    return {&l,&no_fastfail,&mock_fastfail,&mock_clear,&mock_report,&mock_term,&mock_exit};
}

int g_terminate_calls=0;
void terminate_handler(){ ++g_terminate_calls; }

template<class T>T&at(void*p,size_t o){return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+o);}

struct CleanupState { int calls=0; };
void cleanup(void* p) noexcept { ++static_cast<CleanupState*>(p)->calls; }
struct ThrowFromHandler { };
int throwing_new_handler(void*,size_t){ throw ThrowFromHandler{}; }
int no_heap_free(void*,void*) noexcept { return 1; }
void* fail_heap(void*,size_t) noexcept { return nullptr; }
void invoke_alloc(void* p) {
    const auto* ops=static_cast<const AllocatorPlatformOps*>(p);
    (void)allocate_block_or_throw(64,AllocationRetryPolicy::outer_retry_only,*ops);
}
}

int main(){
    // D1 -> D2 -> D4: PTD-backed errno is updated by allocator failure and the
    // D2 terminal becomes the native D4 std::bad_alloc surface.
    d2test::PtdFake ptd_fake{}; d2test::start_ptd(ptd_fake);
    d2test::HeapFake heap{}; heap.handler_results={0};
    bool bad_alloc_seen=false;
    try { (void)allocate_block_or_throw(64,AllocationRetryPolicy::outer_retry_only,d2test::ops(heap)); }
    catch(const std::bad_alloc&) { bad_alloc_seen=true; }
    assert(bad_alloc_seen);
    assert(current_errno_slot() && *current_errno_slot()==allocation_errno_enomem);

    // D2 -> D4/D5 callback layer: exceptions escaping a curio new-handler
    // are not swallowed, and D4 unwind-only cleanup runs exactly once.
    AllocatorPlatformOps throwing_ops{nullptr,&fail_heap,&no_heap_free,&throwing_new_handler};
    CleanupState cleanup_state{}; bool callback_escape=false;
    try { invoke_with_unwind_cleanup(&invoke_alloc,&throwing_ops,&cleanup,&cleanup_state); }
    catch(const ThrowFromHandler&) { callback_escape=true; }
    assert(callback_escape && cleanup_state.calls==1);

    // D1 -> D5: terminate consumer reads PTD+0x18 before entering the default
    // fatal-app-exit recipe. Mock terminal APIs return so ordering is visible.
    void* ptd=current_ptd(false); assert(ptd);
    at<RuntimeTerminateHandler>(ptd,ptd_offset::terminate_handler)=&terminate_handler;
    FatalLog fatal{}; dispatch_terminate_then_abort(fatal_ops(fatal));
    assert(g_terminate_calls==1);
    assert(fatal.events.size()==4);
    assert(fatal.events[0]==1u);
    assert(fatal.events[1]==status_fatal_app_exit);
    assert(fatal.events[2]==exception_noncontinuable);
    // mock exit records the common-exit code as the final element
    assert(fatal.events[3]==3u);

    d2test::stop_ptd(ptd_fake);
    return 0;
}
