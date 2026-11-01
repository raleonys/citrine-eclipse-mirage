// SPDX-License-Identifier: GPL-3.0-only
#include "src/hooks/registry.hpp"
#include "src/hooks/callback_lock.hpp"
#include "src/runtime/startup_steps.hpp"
#include <array>
#include <atomic>
#include <chrono>
#include <cstring>
#include <future>
#include <iostream>
#include <thread>
#include <vector>
namespace r=preloader_l::registry;
namespace cl=preloader_l::callback_lock;
using namespace std::chrono_literals;
std::atomic<unsigned> checks{0};
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
struct Fixture;
struct Effects:r::Effects {
 Fixture& f;std::atomic<unsigned> waits{0};std::atomic<bool> wait_success{false};
 bool throw_wait=false;std::promise<void>* wait_entered=nullptr;
 explicit Effects(Fixture& fixture):f(fixture){}
 std::string filesystem_narrow(std::u16string_view s) override{std::string out;for(auto c:s){if(c>127)throw std::runtime_error("synthetic ASCII fixture");out+=static_cast<char>(c);}return out;}
 template<class C>static int compare(const C* a,const C* b){auto lower=[](C c){return c>=C('A')&&c<=C('Z')?c+C('a')-C('A'):c;};while(*a&&lower(*a)==lower(*b)){++a;++b;}return lower(*a)-lower(*b);}
 int crt_stricmp(const char* a,const char* b) override{return compare(a,b);}
 int crt_wcsicmp(const char16_t* a,const char16_t* b) override{return compare(a,b);}
 int crt_strcpy_s(char* a,std::size_t n,const char* b) override{require(std::strlen(b)+1<=n,"copyA capacity");std::memcpy(a,b,std::strlen(b)+1);return 0;}
 int crt_wcscpy_s(char16_t* a,std::size_t n,const char16_t* b) override{auto size=std::char_traits<char16_t>::length(b)+1;require(size<=n,"copyW capacity");std::memcpy(a,b,size*2);return 0;}
 r::Status original_open_a(const r::OpenA&) override{throw std::runtime_error("unused open A");}
 r::Status original_open_w(const r::OpenW&) override{throw std::runtime_error("unused open W");}
 template<class Q>r::Status query(const Q& q){if(q.size)q.size.store(777);if(q.type)q.type.store(0);return 87;}
 r::Status original_query_a(const r::QueryA& q) override{return query(q);}
 r::Status original_query_w(const r::QueryW& q) override{return query(q);}
 r::Status original_close(r::Handle) override{return 0;}
 r::Handle allocate_4_04b0bc() override{throw std::runtime_error("unused allocate");}
 void free_04b0f8(r::Handle) override{}
 void wait_036880(std::int32_t) override;
 void log_literal_006880(std::uint64_t) override{}
 void log_text_007990(std::string_view) override{}
 void finish_log_0088e0() override{}
};
struct Fixture {
 std::recursive_mutex mutex;preloader_l::StartupSteps steps;
 r::Values values{{"ClientValue",u"Install"},{"ClientPath",u"C:\\fixture"},{"LanguageValue",u"Locale"},{"Language",u"en_US"}};
 r::Handles client{4},language{8},fabricated;std::uint8_t enabled=1;Effects effects{*this};
 r::StateRefs state(){return {values,client,language,fabricated,enabled};}
 void complete(){steps.finish(preloader_l::StartupSteps::Step::server);steps.finish(preloader_l::StartupSteps::Step::provider);steps.finish(preloader_l::StartupSteps::Step::extra_worker);}
};
void Effects::wait_036880(std::int32_t count){
 require(count==-1,"source ClientValue waits all three stages");++waits;
 {cl::Pause pause;if(wait_entered)wait_entered->set_value();wait_success=f.steps.wait(count);if(throw_wait)throw std::runtime_error("fixture wait failure");}
}
bool peer_can_lock(std::recursive_mutex& mutex){bool acquired=false;std::thread peer([&]{acquired=mutex.try_lock();if(acquired)mutex.unlock();});peer.join();return acquired;}
void lock_scopes(){
 std::recursive_mutex mutex;
 {
  cl::Guard outer(mutex);cl::Guard inner(mutex);
  require(!peer_can_lock(mutex),"recursive callback guard serializes peers");
  {
   cl::Pause outer_pause;require(peer_can_lock(mutex),"pause releases every recursive acquisition");
   {
    cl::Guard reentry(mutex);require(!peer_can_lock(mutex),"reentry while outer pause has its own lock");
    {cl::Pause nested_pause;require(peer_can_lock(mutex),"nested pause releases only current owned frames");}
    require(!peer_can_lock(mutex),"nested pause restores reentrant ownership");
   }
   require(peer_can_lock(mutex),"outer frames remain paused until outer scope ends");
  }
  require(!peer_can_lock(mutex),"all recursive acquisitions restored");
  try{cl::Pause pause;throw std::runtime_error("fixture unwind");}catch(const std::runtime_error&){}
  require(!peer_can_lock(mutex),"exception restores callback lock ownership");
 }
 require(peer_can_lock(mutex),"guards release after final callback scope");
 {cl::Guard bypass(mutex,false);cl::Pause pause;require(peer_can_lock(mutex),"unserialized callback frame is not spuriously acquired");}
}
struct Threads {
 Fixture& f;std::jthread query,progress;
 ~Threads(){f.steps.cancel();}
};
void selected_wait(bool wide){
 Fixture f;f.steps.finish(preloader_l::StartupSteps::Step::server);
 std::promise<void> entered,progress;auto entered_future=entered.get_future(),progress_future=progress.get_future();
 f.effects.wait_entered=&entered;std::promise<r::Status> result;auto result_future=result.get_future();std::atomic<unsigned> leases{0};
 std::uint32_t size=64,type=7;std::array<char,64> bytes{};std::array<char16_t,32> wide_bytes{};Threads threads{f};
 threads.query=std::jthread([&]{
  struct Lease {std::atomic<unsigned>& n;explicit Lease(std::atomic<unsigned>& value):n(value){++n;}~Lease(){--n;}} lease(leases);
  try{
   // Three frames model native callback reentry, using the actual production guard.
   cl::Guard a(f.mutex),b(f.mutex),c(f.mutex);
   auto status=wide?r::query_w_00fb60(f.state(),f.effects,{4,u"Install",nullptr,&type,wide_bytes.data(),&size}):r::query_a_00fac0(f.state(),f.effects,{4,"Install",nullptr,&type,bytes.data(),&size});
   result.set_value(status);
  }catch(...){result.set_exception(std::current_exception());}
 });
 require(entered_future.wait_for(2s)==std::future_status::ready,"selected source query reaches phase wait");
 require(leases.load()==1,"native-style lifetime lease remains active while serialization paused");
 threads.progress=std::jthread([&]{
  try{
   cl::Guard callback(f.mutex);
   std::uint32_t n=12,t=9;char data[12]{};
   require(r::query_a_00fac0(f.state(),f.effects,{9,"Other",nullptr,&t,data,&n})==87,"unrelated callback progresses during selected wait");
   require(r::close_00f5b0(f.state(),f.effects,4)==0,"tracking can change during suspended query without live iterator");
   f.steps.finish(preloader_l::StartupSteps::Step::provider);progress.set_value();
  }catch(...){progress.set_exception(std::current_exception());}
 });
 const bool advanced=progress_future.wait_for(2s)==std::future_status::ready;
 if(!advanced)f.steps.cancel();
 require(advanced,"waiting registry callback must release all recursive serialization locks");progress_future.get();
 require(result_future.wait_for(50ms)==std::future_status::timeout,"registry query still waits third extra-worker phase");
 f.steps.finish(preloader_l::StartupSteps::Step::extra_worker);
 require(result_future.wait_for(2s)==std::future_status::ready&&result_future.get()==0,"source override completes after all phases");
 threads.query.join();threads.progress.join();
 require(f.effects.waits.load()==1&&f.effects.wait_success.load(),"one all-step wait with successful completion");
 require(type==1,"source type assignment preserved after original query");
 require(size==(wide?22u:11u),"source required byte size uses original caller capacity");
 require(wide?std::u16string(wide_bytes.data())==u"C:\\fixture":std::string(bytes.data())=="C:\\fixture","source ClientPath copied after wait");
 require(f.client.empty()&&leases.load()==0,"tracking and lifetime owner drain cleanly");
}
void other_branches(){
 Fixture f;std::array<char,64> bytes{};std::array<char16_t,32> wide{};std::uint32_t size=64,type=7;cl::Guard callback(f.mutex);
 require(r::query_a_00fac0(f.state(),f.effects,{4,"Other",nullptr,&type,bytes.data(),&size})==87,"unmatched client name forwards");
 size=64;require(r::query_w_00fb60(f.state(),f.effects,{8,u"Locale",nullptr,&type,wide.data(),&size})==0,"language branch remains independent of startup wait");
 require(size==12&&std::u16string(wide.data())==u"en_US","language response unchanged");
 size=64;require(r::query_w_00fb60(f.state(),f.effects,{99,u"Install",nullptr,&type,wide.data(),&size})==87,"untracked key does not wait");
 require(f.effects.waits.load()==0,"only source-selected ClientValue branch waits");
 f.complete();f.effects.throw_wait=true;size=64;type=9;bool failed=false;
 try{(void)r::query_a_00fac0(f.state(),f.effects,{4,"Install",nullptr,&type,bytes.data(),&size});}catch(const std::runtime_error& e){failed=std::string(e.what())=="fixture wait failure";}
 require(failed&&size==777&&type==0,"wait failure preserves original writes and propagates");
 require(!peer_can_lock(f.mutex),"query error restores serialization before outer callback continues");
}
void cancellation(){
 Fixture f;std::promise<void> entered;auto entered_future=entered.get_future();f.effects.wait_entered=&entered;
 std::promise<r::Status> result;auto future=result.get_future();Threads threads{f};
 threads.query=std::jthread([&]{try{cl::Guard callback(f.mutex);std::uint32_t size=0;result.set_value(r::query_w_00fb60(f.state(),f.effects,{4,u"Install",nullptr,nullptr,nullptr,&size}));}catch(...){result.set_exception(std::current_exception());}});
 require(entered_future.wait_for(2s)==std::future_status::ready,"cancel case reaches selected wait");f.steps.cancel();
 require(future.wait_for(2s)==std::future_status::ready&&future.get()==0,"owned cancellation releases wait without deadlock");threads.query.join();
 require(!f.effects.wait_success.load(),"cancellation is not phase success");
}
int main(){try{lock_scopes();selected_wait(false);selected_wait(true);other_branches();cancellation();std::cout<<"PASS registry wait checks="<<checks<<"; real recursive mutex/phase CV + unmodified source registry algorithm; Win32 API and native lease boundary NOT_RUN\n";return 0;}catch(const std::exception& e){std::cerr<<"FAIL registry wait: "<<e.what()<<'\n';return 1;}}
