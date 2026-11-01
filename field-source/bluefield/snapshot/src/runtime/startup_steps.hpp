// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <condition_variable>
#include <cstdint>
#include <mutex>
namespace preloader_l {
// Source Execution predicate (036900..036958); owned cancellation replaces
// polling so shutdown can release waits without claiming phase success.
class StartupSteps final {
public:
 enum class Step { server, provider, extra_worker };
 static constexpr bool finished(std::int32_t count,bool server,bool provider,bool extra) noexcept {
  const auto limit=count==-1?3:count;
  return (limit<1||server)&&(limit<2||provider)&&(limit<3||extra);
 }
 void finish(Step step){
  {std::lock_guard lock(mutex_);if(cancelled_)return;
   switch(step){case Step::server:server_=true;break;case Step::provider:provider_=true;break;case Step::extra_worker:extra_=true;break;}}
  condition_.notify_all();
 }
 bool wait(std::int32_t count){
  std::unique_lock lock(mutex_);
  condition_.wait(lock,[&]{return cancelled_||finished(count,server_,provider_,extra_);});
  return !cancelled_;
 }
 void cancel(){
  {std::lock_guard lock(mutex_);cancelled_=true;}
  condition_.notify_all();
 }
private:
 std::mutex mutex_;std::condition_variable condition_;
 bool server_=false,provider_=false,extra_=false,cancelled_=false;
};
}
