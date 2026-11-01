// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/runtime/ptd_runtime.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <vector>
using namespace redfield::runtime;
namespace {
enum E:uint32_t{SET_SENTINEL=1,SET_PTD=2,CLEAR_SLOT=3,FREE_PTD=4,FLS_FREE=5};
struct F{uint32_t le=77,index=9;void*slot=nullptr;bool fail_alloc=false;int fail_set_call=0,set_calls=0;std::vector<uint32_t> events;std::vector<void*> frees;};
uint32_t gle(void*c)noexcept{return static_cast<F*>(c)->le;} void sle(void*c,uint32_t v)noexcept{static_cast<F*>(c)->le=v;}
uint32_t fa(void*c,void(*)(void*)noexcept)noexcept{return static_cast<F*>(c)->index;} int ff(void*c,uint32_t)noexcept{auto*f=static_cast<F*>(c);f->events.push_back(FLS_FREE);return 1;}
void*fg(void*c,uint32_t)noexcept{return static_cast<F*>(c)->slot;} int fs(void*c,uint32_t,void*v)noexcept{auto*f=static_cast<F*>(c);++f->set_calls; if(v==reinterpret_cast<void*>(~uintptr_t{0}))f->events.push_back(SET_SENTINEL);else if(v)f->events.push_back(SET_PTD);else f->events.push_back(CLEAR_SLOT); if(f->fail_set_call==f->set_calls)return 0; f->slot=v;return 1;}
void*az(void*c,size_t n)noexcept{auto*f=static_cast<F*>(c);return f->fail_alloc?nullptr:calloc(1,n);} int fm(void*c,void*p)noexcept{auto*f=static_cast<F*>(c);f->events.push_back(FREE_PTD);f->frees.push_back(p);free(p);return 1;}
void lk(void*,uint32_t)noexcept{} PtdPlatformOps ops(F&f){return PtdPlatformOps{&f,&gle,&sle,&fa,&ff,&fg,&fs,&az,&fm,&lk,&lk};}
template<class T>T&at(void*p,size_t o){return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+o);}
}
int main(){
 // Initializer must fail if immediate PTD bootstrap allocation fails, and must
 // perform global FLS cleanup.
 {F f; f.fail_alloc=true; auto o=ops(f); PtdSharedBindings b{}; if(initialize_ptd_runtime(o,b))return 1; if(ptd_fls_index()!=ptd_fls_out_of_indexes||f.slot)return 2; if(f.events.size()!=3||f.events[0]!=SET_SENTINEL||f.events[1]!=CLEAR_SLOT||f.events[2]!=FLS_FREE)return 3;}
 // Second FlsSetValue(PTD) failure must clear the slot before freeing PTD.
 {F f; f.fail_set_call=2; auto o=ops(f); PtdSharedBindings b{}; if(initialize_ptd_runtime(o,b))return 4; if(f.events.size()!=5||f.events[0]!=SET_SENTINEL||f.events[1]!=SET_PTD||f.events[2]!=CLEAR_SLOT||f.events[3]!=FREE_PTD||f.events[4]!=FLS_FREE)return 5; if(f.slot)return 6;}
 // Normal teardown keeps exact owned-field free order and FlsFree sees the
 // still-published global index; reset follows it.
 {F f; auto o=ops(f); PtdSharedBindings b{}; if(!initialize_ptd_runtime(o,b))return 7; void*p=current_ptd(false); if(!p)return 8; const size_t off[9]={0x70,0x58,0x60,0x68,0x48,0x50,0x78,0x80,0x3C0}; void*q[9]{}; for(int i=0;i<9;i++){q[i]=malloc(1);at<void*>(p,off[i])=q[i];}
  detach_current_thread_ptd(); if(f.slot)return 9; if(f.frees.size()!=10)return 10; for(int i=0;i<9;i++)if(f.frees[i]!=q[i])return 11; if(f.frees[9]!=p)return 12;
  size_t before=f.events.size(); shutdown_ptd_runtime(); if(f.events.size()!=before+1||f.events.back()!=FLS_FREE||ptd_fls_index()!=ptd_fls_out_of_indexes)return 13;}
 return 0;
}
