// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/transient_allocator.hpp"
#include "preloader_l/worker_owner.hpp"
#include <cstdlib>
#include <iostream>
using namespace preloader_l;
using namespace preloader_l::runtime;
#define CHECK(x) do {if(!(x)){std::cerr<<__LINE__<<": " #x "\n";return 1;}}while(false)
struct Backend {
    bool fail=true,cleanup_ok=true;
    unsigned closes=0,unpins=0,finalizers=0;
    static void* alloc(void*,size_t n) noexcept {return std::malloc(n);}
    static int free(void* p,void* memory) noexcept {
        if(static_cast<Backend*>(p)->fail)return 0;
        std::free(memory);return 1;
    }
    static bool pin(void* p,void*,void** out) noexcept {*out=p;return true;}
    static void* spawn(void* p,WorkerOwner*) noexcept {return p;}
    static WaitResult wait(void*,void*,std::uint32_t) noexcept {return WaitResult::signaled;}
    static bool close(void* p,void*) noexcept {++static_cast<Backend*>(p)->closes;return true;}
    static bool unpin(void* p,void*) noexcept {++static_cast<Backend*>(p)->unpins;return true;}
    static bool cleanup(void* p,std::uint32_t) noexcept {return static_cast<Backend*>(p)->cleanup_ok;}
    static bool finalize(void* p,std::uint32_t timeout) noexcept {
        ++static_cast<Backend*>(p)->finalizers;return drain_transient_releases(timeout);
    }
};
int main() {
    for(unsigned mode=0;mode<2;++mode) {
        Backend b;WorkerOperations ops{&b,Backend::pin,Backend::spawn,Backend::wait,Backend::close,Backend::unpin,Backend::cleanup,Backend::finalize};WorkerOwner owner;
        CHECK(owner.schedule(ops,&b));
        if(mode)owner.mark_worker_clean();
        void* memory=allocate_transient_bytes(64,{&b,Backend::alloc,Backend::free});CHECK(memory);
        CHECK(!release_transient_bytes(memory));
        CHECK(owner.shutdown(0)==ShutdownResult::service_cleanup_pending);
        CHECK(b.closes==0 && b.unpins==0 && b.finalizers==1 && transient_release_pending());
        CHECK(owner.shutdown(15)==ShutdownResult::service_cleanup_pending);
        CHECK(b.closes==0 && b.unpins==0 && transient_pending_release_count()==1);
        b.fail=false;
        CHECK(owner.shutdown(1000)==ShutdownResult::stopped);
        CHECK(b.closes==1 && b.unpins==1 && !transient_release_pending());
        CHECK(owner.shutdown(0)==ShutdownResult::stopped && b.unpins==1);
    }
    std::cout<<"TRANSIENT_SHUTDOWN_CLEAN_AND_DIRTY_WORKER_PIN_RETAIN_RETRY_PASS\n";
}
