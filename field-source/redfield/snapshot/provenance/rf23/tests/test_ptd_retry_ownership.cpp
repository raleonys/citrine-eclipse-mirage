// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime/ptd_runtime.hpp"
#include <cassert>
#include <cstdlib>
using namespace redfield::runtime;
namespace {
struct F{uint32_t le=9,index=4;void*slot=nullptr;bool fail_free=false;bool fail_clear_once=false;int clears=0;};
uint32_t gl(void*c)noexcept{return static_cast<F*>(c)->le;}void sl(void*c,uint32_t v)noexcept{static_cast<F*>(c)->le=v;}uint32_t fa(void*c,void(*)(void*)noexcept)noexcept{return static_cast<F*>(c)->index;}int ff(void*,uint32_t)noexcept{return 1;}void*fg(void*c,uint32_t)noexcept{return static_cast<F*>(c)->slot;}
int fs(void*c,uint32_t,void*v)noexcept{auto*f=static_cast<F*>(c);if(!v){++f->clears;if(f->fail_clear_once){f->fail_clear_once=false;return 0;}}f->slot=v;return 1;}void*az(void*,size_t n)noexcept{return std::calloc(1,n);}int fm(void*c,void*p)noexcept{auto*f=static_cast<F*>(c);if(f->fail_free)return 0;std::free(p);return 1;}void lk(void*,uint32_t)noexcept{}
PtdPlatformOps ops(F&f){return {&f,gl,sl,fa,ff,fg,fs,az,fm,lk,lk};}
}
int main(){F f{};PtdSharedBindings b{};auto o=ops(f);assert(initialize_ptd_runtime(o,b));void*p=current_ptd(false);assert(p);
 // Failed PTD block free restores exact FLS ownership; retry succeeds.
 f.fail_free=true;assert(!detach_current_thread_ptd());assert(f.slot==p);f.fail_free=false;assert(detach_current_thread_ptd());assert(f.slot==nullptr);
 // A stranded creation sentinel is recoverable: failed clear is retried later.
 f.slot=reinterpret_cast<void*>(~uintptr_t{0});f.fail_clear_once=true;assert(current_ptd(true)==nullptr);assert(f.slot==reinterpret_cast<void*>(~uintptr_t{0}));void*q=current_ptd(true);assert(q&&f.slot==q);assert(detach_current_thread_ptd());shutdown_ptd_runtime();assert(!ptd_cleanup_pending());return 0;}
