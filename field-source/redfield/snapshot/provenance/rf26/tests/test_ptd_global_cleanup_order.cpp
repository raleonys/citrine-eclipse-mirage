// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/ptd_runtime.hpp"
#include <cstdlib>
using namespace preloader_l::runtime;
namespace { struct F{void*slot=nullptr;uint32_t seen=0;};
uint32_t gle(void*)noexcept{return 0;} void sle(void*,uint32_t)noexcept{} uint32_t fa(void*,void(*)(void*)noexcept)noexcept{return 17;}
int ff(void*c,uint32_t)noexcept{static_cast<F*>(c)->seen=ptd_fls_index();return 1;} void*fg(void*c,uint32_t)noexcept{return static_cast<F*>(c)->slot;}
int fs(void*c,uint32_t,void*p)noexcept{static_cast<F*>(c)->slot=p;return 1;} void*az(void*,size_t n)noexcept{return std::calloc(1,n);} int fm(void*,void*p)noexcept{std::free(p);return 1;} void lk(void*,uint32_t)noexcept{}
}
int main(){F f;PtdPlatformOps o{&f,gle,sle,fa,ff,fg,fs,az,fm,lk,lk};PtdSharedBindings b{};if(!initialize_ptd_runtime(o,b))return 1;detach_current_thread_ptd();shutdown_ptd_runtime();if(f.seen!=17)return 2;if(ptd_fls_index()!=ptd_fls_out_of_indexes)return 3;return 0;}
