// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <mutex>
namespace laureondo::callback_lock {
class Pause;
// One frame per native callback, including reentrant callbacks. The native
// binding lease is separate and remains live while serialization is paused.
class Guard final {
 friend class Pause;
 inline static thread_local Guard* current_=nullptr;
 std::unique_lock<std::recursive_mutex> lock_;
 Guard* previous_;
 Guard* resume_next_=nullptr;
public:
 explicit Guard(std::recursive_mutex& mutex,bool enabled=true):lock_(mutex,std::defer_lock),previous_(current_){
  if(enabled)lock_.lock();
  current_=this;
 }
 ~Guard(){current_=previous_;}
 Guard(const Guard&)=delete;
 Guard& operator=(const Guard&)=delete;
};
// Release every callback-owned recursive acquisition on this thread. Restoring
// in original acquisition order also supports nested pauses without allocation.
// Does not unlock external/untracked locks or release a native binding lease.
class Pause final {
 Guard* resume_=nullptr;
 void restore(){
  while(resume_){auto* frame=resume_;frame->lock_.lock();resume_=frame->resume_next_;frame->resume_next_=nullptr;}
 }
public:
 Pause(){
  try{
   for(auto* frame=Guard::current_;frame;frame=frame->previous_){
    if(frame->lock_.owns_lock()){
     frame->lock_.unlock();frame->resume_next_=resume_;resume_=frame;
    }
   }
  }catch(...){restore();throw;}
 }
 ~Pause(){restore();}
 Pause(const Pause&)=delete;
 Pause& operator=(const Pause&)=delete;
};
}
