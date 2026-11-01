#include "preloader_l/runtime/fatal_policy.hpp"
#include <cassert>
#include <cstdint>
#include <vector>
using namespace preloader_l::runtime;
namespace {
struct ReportEvent { uint32_t status; uint32_t flags; const void* supplied; };
struct Log{ bool ff_available=false; std::vector<uint32_t> e; std::vector<ReportEvent> reports; };
bool avail(void* p) noexcept { return static_cast<Log*>(p)->ff_available; }
void ff(void* p,uint32_t c){ static_cast<Log*>(p)->e.push_back(0xF0000000u|c); }
void clear(void* p) noexcept { static_cast<Log*>(p)->e.push_back(1); }
void report(void* p,uint32_t c,uint32_t f,const void* supplied) noexcept {
 auto* l=static_cast<Log*>(p); l->e.push_back(c); l->reports.push_back({c,f,supplied});
}
void term(void* p,uint32_t c) noexcept { static_cast<Log*>(p)->e.push_back(0xA0000000u|(c&0x0FFFFFFFu)); }
void exitp(void* p,uint32_t c) noexcept { static_cast<Log*>(p)->e.push_back(0xE0000000u|c); }
FatalPlatformOps ops(Log& l){ return {&l,&avail,&ff,&clear,&report,&term,&exitp}; }
}
int main(){
 auto a=fatal_recipe(FatalFamily::security_failure);
 assert(a.exception_status==status_stack_buffer_overrun && a.exception_flags==exception_noncontinuable && !a.uses_fast_fail_when_available && a.hard_terminate_process);
 auto i=fatal_recipe(FatalFamily::invalid_parameter);
 assert(i.fast_fail_code==5 && i.exception_status==status_invalid_crt_parameter && i.exception_flags==exception_noncontinuable && i.hard_terminate_process);
 auto f=fatal_recipe(FatalFamily::fatal_app_exit);
 assert(f.fast_fail_code==7 && f.exception_status==status_fatal_app_exit && f.process_exit_code==3 && f.enters_common_exit_engine);

 // Security failure preserves the exact supplied exception-pointer identity.
 uintptr_t supplied_storage=0x12345678u; const void* supplied=&supplied_storage;
 Log l; execute_fatal_recipe(FatalFamily::security_failure,ops(l),{supplied});
 assert(l.e.size()==3 && l.e[0]==1 && l.e[1]==status_stack_buffer_overrun);
 assert(l.reports.size()==1 && l.reports[0].supplied==supplied && l.reports[0].flags==exception_noncontinuable);

 // Generalized reportfault families request platform-side live-context capture
 // by passing a null supplied-pointer identity.
 l={}; execute_fatal_recipe(FatalFamily::invalid_parameter,ops(l));
 assert(l.e.size()==3 && l.e[0]==1 && l.e[1]==status_invalid_crt_parameter);
 assert(l.reports.size()==1 && l.reports[0].supplied==nullptr && l.reports[0].flags==exception_noncontinuable);

 l={}; l.ff_available=true; execute_fatal_recipe(FatalFamily::fatal_app_exit,ops(l));
 assert(l.e.size()==4 && l.e[0]==(0xF0000000u|7u) && l.e[1]==1 && l.e[2]==status_fatal_app_exit && l.e[3]==(0xE0000000u|3u));
 assert(l.reports.size()==1 && l.reports[0].supplied==nullptr);
 return 0;
}
