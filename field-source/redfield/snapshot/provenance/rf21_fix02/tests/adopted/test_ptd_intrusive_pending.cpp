#include "preloader_l/runtime/ptd_runtime.hpp"
#include <cassert>
#include <cstdlib>
#include <cstdint>
using namespace preloader_l::runtime;
namespace {
struct F {
    uint32_t last_error=0x55u;
    uint32_t index=7u;
    void* slot=nullptr;
    bool fail_free=false;
    bool fail_next_nonnull_set=false;
    size_t allocs=0u;
    size_t frees=0u;
};
uint32_t gl(void* c) noexcept { return static_cast<F*>(c)->last_error; }
void sl(void* c,uint32_t v) noexcept { static_cast<F*>(c)->last_error=v; }
uint32_t fa(void* c,void(*)(void*) noexcept) noexcept { return static_cast<F*>(c)->index; }
int ff(void*,uint32_t) noexcept { return 1; }
void* fg(void* c,uint32_t) noexcept { return static_cast<F*>(c)->slot; }
int fs(void* c,uint32_t,void* v) noexcept {
    auto* f=static_cast<F*>(c);
    if(v && f->fail_next_nonnull_set){ f->fail_next_nonnull_set=false; return 0; }
    f->slot=v; return 1;
}
void* az(void* c,size_t n) noexcept { auto* f=static_cast<F*>(c); ++f->allocs; return std::calloc(1,n); }
int fm(void* c,void* p) noexcept {
    auto* f=static_cast<F*>(c);
    if(f->fail_free) return 0;
    ++f->frees; std::free(p); return 1;
}
void lk(void*,uint32_t) noexcept {}
PtdPlatformOps ops(F& f){ return {&f,gl,sl,fa,ff,fg,fs,az,fm,lk,lk}; }
}
int main(){
    F f{}; PtdSharedBindings b{}; auto o=ops(f);
    assert(initialize_ptd_runtime(o,b));
    // Create more failed-free owners than the historical fixed 64-entry table.
    for(size_t i=0;i<96u;++i){
        void* p=current_ptd(true); assert(p && f.slot==p);
        f.fail_free=true;
        f.fail_next_nonnull_set=true; // failed free cannot restore FLS owner -> intrusive pending list
        assert(!detach_current_thread_ptd());
        assert(f.slot==nullptr);
        f.fail_free=false;
    }
    assert(ptd_cleanup_pending());
    shutdown_ptd_runtime();
    assert(!ptd_cleanup_pending());
    assert(f.frees>=96u);
    return 0;
}
