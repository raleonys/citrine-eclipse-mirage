#include "preloader_l/runtime/fatal_policy.hpp"
#include <cassert>
#include <vector>
using namespace preloader_l::runtime;
namespace {
struct Log{ bool ff_available=false; std::vector<uint32_t> e; };
bool avail(void* p) noexcept { return static_cast<Log*>(p)->ff_available; }
void ff(void* p,uint32_t c){ static_cast<Log*>(p)->e.push_back(0xF0000000u|c); }
void clear(void* p) noexcept { static_cast<Log*>(p)->e.push_back(1); }
void report(void* p,uint32_t c,uint32_t,const void*) noexcept { static_cast<Log*>(p)->e.push_back(c); }
void term(void* p,uint32_t c) noexcept { static_cast<Log*>(p)->e.push_back(0xA0000000u|(c&0x0FFFFFFFu)); }
void exitp(void* p,uint32_t c) noexcept { static_cast<Log*>(p)->e.push_back(0xE0000000u|c); }
FatalPlatformOps ops(Log& l){ return {&l,&avail,&ff,&clear,&report,&term,&exitp}; }
}
int main(){
 auto a=fatal_recipe(FatalFamily::security_failure);
 assert(a.exception_status==status_stack_buffer_overrun && !a.uses_fast_fail_when_available && a.hard_terminate_process);
 auto i=fatal_recipe(FatalFamily::invalid_parameter);
 assert(i.fast_fail_code==5 && i.exception_status==status_invalid_crt_parameter && i.hard_terminate_process);
 auto f=fatal_recipe(FatalFamily::fatal_app_exit);
 assert(f.fast_fail_code==7 && f.exception_status==status_fatal_app_exit && f.process_exit_code==3 && f.enters_common_exit_engine);
 Log l; execute_fatal_recipe(FatalFamily::security_failure,ops(l));
 assert(l.e.size()==3 && l.e[0]==1 && l.e[1]==status_stack_buffer_overrun);
 l={}; execute_fatal_recipe(FatalFamily::invalid_parameter,ops(l));
 assert(l.e.size()==3 && l.e[0]==1 && l.e[1]==status_invalid_crt_parameter);
 l={}; l.ff_available=true; execute_fatal_recipe(FatalFamily::fatal_app_exit,ops(l));
 assert(l.e.size()==4 && l.e[0]==(0xF0000000u|7u) && l.e[1]==1 && l.e[2]==status_fatal_app_exit && l.e[3]==(0xE0000000u|3u));
 return 0;
}
