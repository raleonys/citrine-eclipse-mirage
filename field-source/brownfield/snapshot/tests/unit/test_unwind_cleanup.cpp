#include "preloader_l/runtime/exception_policy.hpp"
#include <cassert>
using namespace preloader_l::runtime;
namespace {
struct Marker { int cleanup=0; int caught=0; };
struct Escape { int value; };
void cleanup(void* p) noexcept { ++static_cast<Marker*>(p)->cleanup; }
void normal(void*) {}
void throwing(void*) { throw Escape{91}; }
}
int main(){
 Marker m{};
 invoke_with_unwind_cleanup(normal,nullptr,cleanup,&m);
 assert(m.cleanup==0);
 try { invoke_with_unwind_cleanup(throwing,nullptr,cleanup,&m); }
 catch(const Escape& e){m.caught=e.value;}
 assert(m.cleanup==1 && m.caught==91);
 return 0;
}
