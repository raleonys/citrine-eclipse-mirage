#include "preloader_l/runtime/exception_runtime.hpp"
#include <cassert>
using namespace preloader_l::runtime;
namespace { struct Seen{uint32_t c=0,f=0,n=0; const uintptr_t* i=nullptr;}; void raise(uint32_t c,uint32_t f,uint32_t n,const uintptr_t* i,void* p){auto&s=*static_cast<Seen*>(p);s={c,f,n,i};} }
int main(){ uintptr_t info[3]={11,22,33}; ForwardExceptionRecord r{0xDEADBEEFu,9,3,info}; Seen s; forward_exception_record(r,&raise,&s); assert(s.c==r.code&&s.f==r.flags&&s.n==r.parameter_count&&s.i==info); return 0; }
