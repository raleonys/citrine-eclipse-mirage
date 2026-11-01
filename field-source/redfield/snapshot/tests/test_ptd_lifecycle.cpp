// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/runtime/ptd_runtime.hpp"
#include <stdint.h>
#include <stdlib.h>
#include <vector>
using namespace preloader_l::runtime;
namespace {
struct Fake { uint32_t last_error=0xA55Au,index=7; void*slot=nullptr; void(*callback)(void*) noexcept=nullptr; std::vector<size_t> allocations; std::vector<uintptr_t> stores; std::vector<void*> frees; std::vector<uint32_t> locks; };
uint32_t gle(void*c)noexcept{return static_cast<Fake*>(c)->last_error;} void sle(void*c,uint32_t v)noexcept{static_cast<Fake*>(c)->last_error=v;}
uint32_t fa(void*c,void(*cb)(void*)noexcept)noexcept{auto*f=static_cast<Fake*>(c);f->callback=cb;return f->index;} int ff(void*c,uint32_t i)noexcept{return i==static_cast<Fake*>(c)->index;}
void*fg(void*c,uint32_t)noexcept{return static_cast<Fake*>(c)->slot;} int fs(void*c,uint32_t,void*v)noexcept{auto*f=static_cast<Fake*>(c);f->slot=v;f->stores.push_back(reinterpret_cast<uintptr_t>(v));return 1;}
void*az(void*c,size_t n)noexcept{auto*f=static_cast<Fake*>(c);f->allocations.push_back(n);return calloc(1,n);} int fm(void*c,void*p)noexcept{static_cast<Fake*>(c)->frees.push_back(p);free(p);return 1;}
void el(void*c,uint32_t i)noexcept{static_cast<Fake*>(c)->locks.push_back(0x100u+i);} void ll(void*c,uint32_t i)noexcept{static_cast<Fake*>(c)->locks.push_back(0x200u+i);}
int sr=1,lr=1; void rs(void*)noexcept{++sr;} void xs(void*)noexcept{--sr;} void rl(void*)noexcept{++lr;} void xl(void*)noexcept{--lr;}
template<class T>T&at(void*p,size_t o){return *reinterpret_cast<T*>(static_cast<unsigned char*>(p)+o);}
}
int main(){
 Fake f; PtdPlatformOps o{&f,&gle,&sle,&fa,&ff,&fg,&fs,&az,&fm,&el,&ll}; int shared=1,locale=2; PtdSharedBindings b{&shared,&locale,&rs,&xs,&rl,&xl};
 if(!initialize_ptd_runtime(o,b)||ptd_fls_index()!=7)return 1;
 // Original process initializer immediately bootstraps the initializing thread.
 void*p=current_ptd(false); if(!p||f.last_error!=0xA55A)return 2;
 if(f.allocations.size()!=1||f.allocations[0]!=ptd_size+sizeof(void*)+2*sizeof(uint32_t))return 3;
 if(f.stores.size()!=2||f.stores[0]!=~uintptr_t{0}||f.stores[1]!=reinterpret_cast<uintptr_t>(p))return 4;
 if(at<const ExceptionActionRecord*>(p,0)!=default_exception_action_table())return 5;
 if(at<void*>(p,0x10)||at<void*>(p,0x18)||at<int>(p,0x20)!=0||at<void*>(p,0x3B8))return 6;
 if(at<uint32_t>(p,0x28)!=1||at<void*>(p,0x88)!=&shared||at<void*>(p,0x90)!=&locale)return 7;
 if(at<uint8_t>(p,0xBC)!=0x43||at<uint8_t>(p,0x1C2)!=0x43||at<uint64_t>(p,0x3A0)!=0||at<uint64_t>(p,0x3A8)!=1)return 8;
 if(sr!=2||lr!=2)return 9;
 int*e=current_errno_slot(); if(!e||e!=&at<int>(p,0x20))return 10; *e=12; if(*current_errno_slot()!=12||f.allocations.size()!=1)return 11;
 detach_current_thread_ptd(); if(f.slot||sr!=1||lr!=1)return 12;
 shutdown_ptd_runtime(); if(ptd_fls_index()!=ptd_fls_out_of_indexes)return 13; return 0;
}
