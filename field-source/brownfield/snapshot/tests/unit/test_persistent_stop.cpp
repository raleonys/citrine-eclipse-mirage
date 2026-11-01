#include "preloader_l/runtime/bootstrap_state.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include <cassert>
namespace {
struct Fake { int init=0,start=0,serve=0,stop=0,shutdown=0; };
bool initialize(void* c) noexcept { ++static_cast<Fake*>(c)->init; return true; }
bool start(void* c,uint16_t* port) noexcept { ++static_cast<Fake*>(c)->start; *port=3226; return true; }
bool serve(void* c) noexcept { auto* f=static_cast<Fake*>(c); ++f->serve; if(f->serve==2) preloader_l::runtime::request_bootstrap_stop(); return true; }
void stop(void* c) noexcept { ++static_cast<Fake*>(c)->stop; }
bool shutdown(void* c) noexcept { ++static_cast<Fake*>(c)->shutdown; return true; }
}
int main(){
 using namespace preloader_l::runtime;
 // Stop-before-start must not acquire runtime or listener authority.
 { Fake pre{}; BootstrapBackend pb{&pre,initialize,start,serve,stop,shutdown,false};
   reset_bootstrap_for_test(); clear_bootstrap_stop_request(); assert(mark_bootstrap_scheduled()); request_bootstrap_stop();
   assert(run_persistent_bootstrap(pb)==BootstrapRunResult::ok);
   assert(snapshot_bootstrap().phase==BootstrapPhase::completed);
   assert(pre.init==0&&pre.start==0&&pre.serve==0&&pre.stop==0&&pre.shutdown==0); }
 clear_bootstrap_stop_request();
 Fake f{}; BootstrapBackend b{&f,initialize,start,serve,stop,shutdown,false};
 reset_bootstrap_for_test(); clear_bootstrap_stop_request(); assert(mark_bootstrap_scheduled());
 assert(run_persistent_bootstrap(b)==BootstrapRunResult::ok);
 const auto s=snapshot_bootstrap();
 assert(s.phase==BootstrapPhase::completed && s.bound_port==3226 && s.client_count==2);
 assert(f.init==1&&f.start==1&&f.serve==2&&f.stop==1&&f.shutdown==1);
 return 0;
}
