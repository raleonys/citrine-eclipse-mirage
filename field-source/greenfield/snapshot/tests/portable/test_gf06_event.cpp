#include "preloader_l/runtime/event_queue.hpp"
#include <cassert>
using namespace preloader_l::runtime;
int main(){
  // Unchanged HF19's non-Windows path has no process heap owner. This tests
  // truthful fail-closed behavior, not positive event delivery.
  clear_queued_events_for_test();
  constexpr char a[]="<Event>FIRST</Event>";
  PreparedQueuedEvent* p=nullptr;
  assert(!publish_achievement_event(a,sizeof(a)-1));
  assert(!prepare_achievement_event(a,sizeof(a)-1,&p) && p==nullptr);
  assert(queued_event_count_for_test()==0);
  assert(queued_event_preview_size(0)==0);
  char out[64]{};size_t n=999;
  assert(drain_one_queued_event(out,sizeof(out),&n) && n==0);
  assert(queued_event_count_for_test()==0);
  assert(clear_queued_events());
  return 0;
}
