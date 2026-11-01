#include "preloader_l/runtime/event_queue.hpp"
#include <cassert>
#include <cstdlib>
static bool g_fail_free=false; static int g_free_calls=0;
extern "C" void* GetProcessHeap(){return reinterpret_cast<void*>(1);}
extern "C" void* HeapAlloc(void*,unsigned long,size_t n){return std::malloc(n);}
extern "C" int HeapFree(void*,unsigned long,void* p){++g_free_calls;if(g_fail_free)return 0;std::free(p);return 1;}
int main(){using namespace preloader_l::runtime;clear_queued_events_for_test();
 assert(publish_achievement_event("x",1));assert(queued_event_count_for_test()==1);
 g_fail_free=true;assert(!clear_queued_events());assert(queued_event_count_for_test()==0);assert(queued_event_pending_free_count_for_test()==1);
 g_fail_free=false;assert(clear_queued_events());assert(queued_event_pending_free_count_for_test()==0);
 PreparedQueuedEvent*p=nullptr;assert(prepare_achievement_event("y",1,&p)&&p);g_fail_free=true;release_prepared_achievement_event(p);assert(queued_event_pending_free_count_for_test()==1);g_fail_free=false;assert(clear_queued_events());assert(queued_event_pending_free_count_for_test()==0);return 0;}
