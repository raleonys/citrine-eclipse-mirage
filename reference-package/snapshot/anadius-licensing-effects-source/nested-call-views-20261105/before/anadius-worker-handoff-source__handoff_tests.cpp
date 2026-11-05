#include <winsock2.h>
#include <exception>
#include "handoff.hpp"
#include <optional>
#include <string_view>
#include <vector>
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-owned-deque-source/deque.hpp"
#include "../anadius-io-memory-source/io_memory.hpp"
#include "../anadius-session-profile-source/strings.hpp"
#include "../anadius-native-profile-source/native.hpp"
#include "../anadius-xml-reader-source/reader.hpp"
#include "../anadius-crypto-source/crypto.hpp"
#include "../anadius-messages-source/control.hpp"
#include "../anadius-information-effects-source/effects.hpp"
#include "../anadius-content-effects-source/effects.hpp"
#include "../anadius-application-io-source/call.hpp"
#include <sstream>
#include <algorithm>
#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <new>
#include <span>
#include <thread>
namespace {
namespace h=anadius::worker_handoff;namespace d=anadius::thread_domain;namespace s=anadius::socket_recovered;namespace t=anadius::threads_recovered;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E=std::logic_error,class F>void rejects(F f,const char* text){bool seen=false;try{f();}catch(const E&){seen=true;}require(seen,text);}
struct Ledger {
 std::mutex gate;std::map<void*,std::size_t> blocks;std::atomic<std::size_t> allocations=0,frees=0;std::atomic<bool> valid=true;
 // Opt-in exact application allocation failure; native-thread/PTD startup and
 // all existing fixtures continue through the ordinary physical heap path.
 std::atomic<DWORD> failing_actor=0;std::atomic<bool> fail_once=false;
 HANDLE fail_entered=nullptr,fail_release=nullptr;
 std::atomic<DWORD> paused_actor=0;std::atomic<bool> pause_once=false;std::atomic<SIZE_T> pause_bytes=113;
 HANDLE pause_entered=nullptr,pause_release=nullptr;
 std::size_t size(){std::lock_guard lock(gate);return blocks.size();}
 bool contains(void* p){std::lock_guard lock(gate);return blocks.contains(p);}
};
Ledger* ledger=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T size){
 if(size==93&&::GetCurrentThreadId()==ledger->failing_actor.load()&&ledger->fail_once.exchange(false)){
  // No Ledger mutex is held across these waits or another actor's allocation.
  if(!::SetEvent(ledger->fail_entered)||::WaitForSingleObject(ledger->fail_release,10000)!=WAIT_OBJECT_0)::ExitProcess(240);
  ::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;
 }
 auto* p=::HeapAlloc(heap,flags,size);const auto error=::GetLastError();if(p){std::lock_guard lock(ledger->gate);if(!ledger->blocks.emplace(p,size).second)ledger->valid=false;++ledger->allocations;}
 if(size==ledger->pause_bytes.load()&&::GetCurrentThreadId()==ledger->paused_actor.load()&&ledger->pause_once.exchange(false)){
  if(!p||!::SetEvent(ledger->pause_entered)||::WaitForSingleObject(ledger->pause_release,10000)!=WAIT_OBJECT_0)::ExitProcess(241);
 }
 ::SetLastError(error);return p;
}
BOOL WINAPI free_heap(HANDLE heap,DWORD flags,LPVOID p){BOOL result;DWORD error;{std::lock_guard lock(ledger->gate);result=::HeapFree(heap,flags,p);error=::GetLastError();if(result){if(ledger->blocks.erase(p)!=1)ledger->valid=false;++ledger->frees;}}::SetLastError(error);return result;}
std::atomic<bool> module_valid=true;
BOOL WINAPI no_module(DWORD flags,LPCWSTR address,HMODULE* result){if(flags!=4||!address||!result)module_valid=false;if(result)*result=nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
anadius::thread_native::Api thread_api(){auto api=anadius::thread_native::native_api();api.get_module=no_module;return api;}
d::Seed seed(){auto identity=std::make_shared<std::array<std::byte,8>>();return{0x31c872450ab966d1ULL,0,nullptr,nullptr,1,reinterpret_cast<std::uintptr_t>(identity->data()),identity};}
struct Event {
 HANDLE handle=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
 Event(){require(handle!=nullptr,"Own worker coordination event created");}
 ~Event(){if(handle)::CloseHandle(handle);}
 void set(){require(::SetEvent(handle)!=0,"Own worker coordination event signaled");}
 void wait(){require(::WaitForSingleObject(handle,10000)==WAIT_OBJECT_0,"Own worker coordination event observed within finite bound");}
};
struct Task {
 std::function<void()> function;std::exception_ptr error;HANDLE handle=nullptr;std::uint32_t id=0;
 static unsigned __stdcall run(void* raw){auto& self=*static_cast<Task*>(raw);try{self.function();}catch(...){self.error=std::current_exception();}return 63;}
 void launch(anadius::bootstrap_providers::NativeThreads& effects,std::function<void()> f){function=std::move(f);const auto result=t::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&run),reinterpret_cast<std::uintptr_t>(this),0,&id,effects);handle=reinterpret_cast<HANDLE>(result);require(handle&&id,"Actual Domain NativeThreads source creates own native worker");}
 void join(){const bool signaled=handle&&::WaitForSingleObject(handle,10000)==WAIT_OBJECT_0;if(!signaled){std::cerr<<"Own task wait failed; retain live wait/token owners until process exit\n"<<std::flush;::ExitProcess(1);}require(signaled,"Actual native worker handle signals after source wrapper/FLS callbacks");DWORD code=0;require(::GetExitCodeThread(handle,&code)&&code==63,"Actual source worker exits with selected own callback return");require(::CloseHandle(handle)!=0,"Own original worker handle closed after join");handle=nullptr;if(error)std::rethrow_exception(error);}
};
struct Fixture {
 Ledger heap;std::shared_ptr<d::DomainStorage> domain;std::unique_ptr<anadius::bootstrap_providers::NativeThreads> threads;
 Fixture(){if(ledger)throw std::logic_error("Overlapping handoff fixtures");ledger=&heap;auto api=d::Apis{};api.heap.heap_alloc=allocate;api.heap.heap_free=free_heap;domain=d::DomainStorage::create(seed(),api);require(domain->initialize(),"Real own Domain initializes heap/FLS/initial locale/PTD graph");require(domain->current_ptd()&&::FlsGetValue(domain->native_fls_index())==domain->current_ptd(),"Controller has exact real native Domain FLS/PTD");threads=domain->make_threads(thread_api());}
 void close(){threads.reset();require(domain->provider_leases()==0,"All actual thread and prepared-memory provider leases retired on controller");require(domain->close()&&heap.valid&&heap.size()==0&&heap.allocations==heap.frees,"Actual main+worker PTD/Context/application heap allocations balance after FLS exit");require(domain->locale_references()==1&&domain->multibyte_references()==0&&module_valid,"Real locale counters return to initial baselines and own-EXE module boundary stayed valid");domain.reset();ledger=nullptr;}
};
struct Life final:s::LifecycleEffects {
 s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{throw std::logic_error("No source socket lifecycle selected");}
 void close_and_cancel_026fb0(s::SocketService&,s::SocketObject&)override{throw std::logic_error("No source socket close selected");}
 void executor_cleanup(s::SocketObject&,std::uintptr_t)override{throw std::logic_error("No source executor cleanup selected");}
};
void validation_and_cancel(){
 rejects<std::invalid_argument>([]{(void)h::PreparedMemory::prepare(nullptr);},"Null Domain cannot create prepared memory owner");auto* f=new Fixture;
 ::SetLastError(0x47a1);auto memory=h::PreparedMemory::prepare(f->domain);require(::GetLastError()==0x47a1,"Controller preparation preserves LastError across actual provider construction");auto snap=memory->snapshot();require(snap.phase==h::Phase::Prepared&&snap.controller_thread==::GetCurrentThreadId()&&!snap.worker_thread&&!snap.worker_handle_retained&&!snap.worker_exit_verified,"Prepared ticket owns memory without fabricated worker or OS exit proof");require(memory->domain_identity()==f->domain.get()&&f->domain->provider_leases()==2,"Dedicated controller memory owner and NativeThreads retain exact same Domain");
 rejects([&]{(void)memory->acquire();},"Controller PTD cannot impersonate distinct NativeThreads worker");require(memory->snapshot().phase==h::Phase::Prepared,"Rejected controller acquire preserves never-borrowed ticket");bool foreign=false;const auto before=f->heap.allocations.load();std::thread outsider([&]{try{(void)memory->acquire();}catch(const std::logic_error&){foreign=true;}});outsider.join();require(foreign&&f->heap.allocations==before&&memory->snapshot().phase==h::Phase::Prepared,"Foreign compiler-only thread lacks same-Domain native FLS/PTD and allocates no backing bytes");
 ::SetLastError(0x47a2);require(!memory->close(0)&&::GetLastError()==0x47a2,"Close does not guess cancellation or native exit for never-borrowed ticket");require(memory->cancel()&&memory->snapshot().phase==h::Phase::Closed&&!memory->snapshot().worker_exit_verified,"Known never-borrowed cancellation releases memory owner without invented worker-exit attestation");require(memory->close(0)&&memory->cancel(),"Closed prepared ticket supports metadata-only idempotent close/cancel");memory.reset();f->close();delete f;
}
void actual_worker_borrow(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);auto* controller_ptd=f->domain->current_ptd();Event borrowed,proceed,returned,exit;Task worker;std::atomic<h::PreparedMemory::Borrow*> live=nullptr;std::atomic<void*> worker_ptd=nullptr;
 worker.launch(*f->threads,[&]{auto ticket=prepared->acquire();worker_ptd=::FlsGetValue(f->domain->native_fls_index());require(worker_ptd&&worker_ptd!=controller_ptd,"Borrow worker has distinct actual same-Domain PTD");require(static_cast<anadius::crt_ptd_recovered::Ptd*>(worker_ptd.load())->thread_context!=nullptr,"Borrow accepts actual NativeThreads source Context marker");
  ::SetLastError(0x47b1);auto& bytes=ticket.memory();require(ticket.domain_identity()==f->domain.get()&&bytes.domain_identity()==f->domain.get()&&::GetLastError()==0x47b1,"Borrow exposes exact controller-owned ApplicationMemory on admitted worker without LastError changes");auto* p=bytes.allocate(91);require(p&&bytes.outstanding_allocations()==1&&f->heap.contains(p),"Borrowed ApplicationMemory allocates actual tracked own heap bytes");std::memset(p,0x6b,91);require(!ticket.retire()&&prepared->snapshot().phase==h::Phase::Borrowed,"Known live application block prevents return without discarding capability");live=&ticket;borrowed.set();proceed.wait();require(static_cast<unsigned char*>(p)[90]==0x6b,"Foreign/control rejection leaves actual live block intact");bytes.deallocate(p,0);require(bytes.outstanding_allocations()==0&&!bytes.tracking_poisoned(),"Actual source sized-delete bridge frees retained block cleanly");
  h::PreparedMemory::Borrow moved(std::move(ticket));rejects([&]{(void)ticket.memory();},"Moved-from borrow confers no memory access");::SetLastError(0x47b2);require(moved.retire()&&::GetLastError()==0x47b2,"Clean sole-worker retire returns ticket and preserves LastError");rejects([&]{(void)moved.memory();},"Returned borrow cannot resume allocation access");live=nullptr;returned.set();exit.wait();
 });borrowed.wait();
 require(prepared->snapshot().phase==h::Phase::Borrowed&&prepared->snapshot().worker_thread==worker.id&&prepared->snapshot().worker_handle_retained&&!prepared->snapshot().worker_exit_verified,"Ticket retains duplicated actual worker identity while callback is alive");
 rejects([&]{(void)live.load()->memory();},"Controller cannot use worker-only borrow memory");require(!prepared->close(0)&&!prepared->cancel(),"Borrowed phase prevents controller close and never-started cancellation");
 Task sibling;sibling.launch(*f->threads,[&]{const auto n=f->heap.allocations.load();rejects([&]{(void)prepared->acquire();},"Second same-Domain NativeThreads worker cannot acquire one-shot ticket");rejects([&]{(void)live.load()->memory();},"Sibling actual NativeThreads worker cannot use original borrow");rejects([&]{(void)live.load()->retire();},"Sibling cannot return another worker capability");rejects([&]{(void)prepared->close(0);},"PreparedMemory close belongs to original controller");require(f->heap.allocations==n,"Sibling rejected operations publish no backing application allocation");});sibling.join();
 require(prepared->snapshot().phase==h::Phase::Borrowed,"Wrong-worker rejected calls do not retire original live borrow");
 // The public original launch handle is discarded. PreparedMemory's separately
 // duplicated actual self handle must be sufficient for the eventual OS join.
 require(::CloseHandle(worker.handle)!=0,"Caller discards original launched worker handle independently");worker.handle=nullptr;proceed.set();returned.wait();
 ::SetLastError(0x47b3);require(prepared->snapshot().phase==h::Phase::Returned&&!prepared->close(0)&&::GetLastError()==0x47b3,"Returned source borrow before actual native exit is insufficient: zero-timeout join retains owner and LastError");require(prepared->snapshot().worker_handle_retained&&!prepared->snapshot().worker_exit_verified&&f->heap.size()==3,"Worker-held event proves its Context/PTD remain live through failed close timeout");exit.set();
 ::SetLastError(0x47b4);require(prepared->close(10000)&&::GetLastError()==0x47b4,"Controller joins duplicated actual worker handle through all native FLS callbacks, preserving LastError");auto final=prepared->snapshot();require(final.phase==h::Phase::Closed&&final.worker_exit_verified&&!final.worker_handle_retained&&f->heap.size()==1,"Only controller PTD remains after verified real worker exit and controller-only ApplicationMemory destruction");if(worker.error)std::rethrow_exception(worker.error);rejects([&]{(void)prepared->acquire();},"Closed one-shot ticket cannot launch replacement borrow");prepared.reset();f->close();delete f;
}
void job_memory_handoff(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);
 Event offered,acquired,release_job,returned,exit;Task worker,job;
 std::optional<h::PreparedMemory::JobMemory> offer;
 std::atomic<anadius::thread_domain::ApplicationMemory*> backing=nullptr;
 std::atomic<void*> transferred=nullptr;
 // Keep every wait owner outside this try. A failed assertion/join must stop
 // the process before unwinding Events that another actual thread still uses.
 try{
 worker.launch(*f->threads,[&]{
  auto ticket=prepared->acquire();backing=&ticket.memory();
  constexpr std::size_t held_offers=70;
  std::vector<h::PreparedMemory::JobMemory> pending;pending.reserve(held_offers);
  ::SetLastError(0x47c0);
  for(std::size_t i=0;i<held_offers;++i)pending.push_back(ticket.prepare_job());
  require(prepared->outstanding_jobs()==held_offers&&::GetLastError()==0x47c0,"Seventy distinct unacquired offers exceed the former64 ceiling without changing LastError");
  require(!ticket.retire(),"Unacquired job offer blocks main logical return even without allocations");
  for(auto& token:pending)require(token.cancel(),"Original worker cancels never-acquired offer");
  require(ticket.reap_jobs()==held_offers&&!prepared->outstanding_jobs(),"All seventy canceled offers recycle without fabricated worker handles");
  // Recycle actual exited siblings while the original Borrow remains alive.
  for(unsigned i=0;i<2;++i){
   auto token=ticket.prepare_job();Task child;
   child.launch(*f->threads,[&]{token.acquire();require(&token.memory()==backing.load(),"Sibling capability names exact controller-owned backing");require(token.retire(),"Sibling logically returns before actual wrapper/FLS exit");});
   child.join();::SetLastError(0x47c1);
   require(ticket.reap_jobs()==1&&!prepared->outstanding_jobs()&&::GetLastError()==0x47c1,"Only observed exited child duplicate is closed/recycled; caller LastError preserved");
  }
  auto token=ticket.prepare_job();offer.emplace(std::move(token));
  rejects([&]{(void)token.memory();},"Moved-from job offer gives no memory authority");
  rejects([&]{offer->acquire();},"Original worker cannot acquire its own sibling offer");
  offered.set();acquired.wait();
  rejects([&]{(void)offer->memory();},"Original worker cannot use sibling's acquired token");
  require(!offer->cancel()&&!ticket.retire(),"Acquired job cannot be canceled or hidden by main return");
  release_job.set();returned.wait();
  require(ticket.reap_jobs()==0&&prepared->outstanding_jobs()==1,"Logical job return before actual native exit cannot recycle its record");
  require(!ticket.retire(),"Transferred live job allocation still blocks main logical return");
  ticket.memory().deallocate(transferred.load(),0);
  require(ticket.retire(),"Main Borrow returns only after job logical return and actual backing allocation drain");
 });
 offered.wait();
 rejects([&]{offer->acquire();},"Controller cannot acquire sibling job offer");
 bool foreign=false;std::thread outsider([&]{try{offer->acquire();}catch(const std::logic_error&){foreign=true;}});outsider.join();
 require(foreign,"Compiler-only foreign thread lacks actual same-Domain job PTD/Context");
 job.launch(*f->threads,[&]{
  ::SetLastError(0x47c2);offer->acquire();
  require(offer->domain_identity()==f->domain.get()&& &offer->memory()==backing.load()&&::GetLastError()==0x47c2,"Actual distinct job binds exact backing Domain and preserves LastError");
  rejects([&]{offer->acquire();},"Acquired offer cannot bind again");
  transferred=offer->memory().allocate(37);acquired.set();release_job.wait();
  require(offer->retire(),"Logical job return may transfer a live source allocation to original owner");
  rejects([&]{(void)offer->memory();},"Returned job token gives no further backing access");
  returned.set();exit.wait();
 });
 worker.join();
 require(prepared->snapshot().phase==h::Phase::Returned&&!prepared->close(0)&&prepared->outstanding_jobs()==1,"Original worker OS exit does not stand in for still-live child worker/FLS exit");
 exit.set();job.join();
 require(prepared->close(10000)&&!prepared->outstanding_jobs()&&prepared->snapshot().phase==h::Phase::Closed,"Controller releases backing only after original and every admitted child OS/FLS exit");
 offer.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Job capability source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}
 catch(...){::ExitProcess(2);}
}
void parent_job_borrow(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);auto other=h::PreparedMemory::prepare(f->domain);
 Event ready,returned,exit;Task worker,job;
 std::shared_ptr<s::SessionMemoryDomain> parent,foreign_parent;
 std::optional<h::PreparedMemory::JobMemory> offer;
 std::atomic<std::uintptr_t> allocation=0;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>());
   auto wrong=other->acquire();foreign_parent=std::make_shared<s::SessionMemoryDomain>(std::move(wrong),std::make_shared<Life>());
   {auto operation=parent->enter();offer.emplace(parent->prepare_job());}
   require(!parent->close()&&parent->snapshot().phase==s::SessionMemoryPhase::Open&&prepared->snapshot().phase==h::Phase::Borrowed,"Pending child keeps empty Parent open before lifecycle/borrow retirement");
   ready.set();returned.wait();
   require(!parent->close()&&parent->snapshot().phase==s::SessionMemoryPhase::Open,"Child-created live typed allocation retains Parent without poison");
   {auto operation=parent->enter();auto bytes=parent->buffer_context().memory.view(allocation.load(),73);require(std::all_of(bytes.begin(),bytes.end(),[](auto value){return value==0x7c;}),"Original Parent reads exact child allocation after whole Operation ends");parent->free_sized_04b0f8(allocation.load(),73);}
   require(parent->reap_jobs()==0&&prepared->outstanding_jobs()==1,"Parent reaper does not turn logical job return into actual OS exit");
   require(parent->close()&&foreign_parent->close(),"Both exact Parents return source ownership on original worker after typed drain");
   parent.reset();foreign_parent.reset();
  });
  ready.wait();
  rejects([&]{auto operation=parent->enter(*offer);},"Controller cannot use offered child permission");
  job.launch(*f->threads,[&]{
   offer->acquire();
   require(parent->memory_domain_identity()==foreign_parent->memory_domain_identity(),"Mismatch fixture uses different backing roots in the SAME real Domain");
   rejects<std::invalid_argument>([&]{auto operation=foreign_parent->enter(*offer);},"Same-Domain job cannot enter different prepared backing root");
   require(foreign_parent->snapshot().phase==s::SessionMemoryPhase::Open,"Wrong-root preflight does not poison otherwise open Parent");
   rejects([&]{auto operation=parent->enter();},"Default Parent entry still rejects sibling without explicit permission");
   std::optional<h::PreparedMemory::JobMemory> moved;
   {
    ::SetLastError(0x47d1);auto operation=parent->enter(*offer);
    require(::GetLastError()==0x47d1&&!offer->retire(),"Whole job Operation preserves LastError and pins against logical return");
    moved.emplace(std::move(*offer));
    rejects([&]{(void)offer->memory();},"Operation does not depend on moved-from JobMemory object's address");
    rejects([&]{auto nested=parent->enter(*moved);},"Nested Parent Operation rejects before additional pin publication");
    rejects([&]{(void)parent->prepare_job();},"Child permission cannot offer original-worker jobs");
    rejects([&]{(void)parent->reap_jobs();},"No OS job reaping inside whole Parent Operation");
    require(!parent->close()&&!moved->retire(),"Active Parent prevents cleanup and moved token cannot bypass permission pin");
    auto& bytes=parent->buffer_context();allocation=bytes.api.allocate_04b0bc(73);bytes.api.fill_bytes(allocation.load(),0x7c,73);
   }
   require(moved->retire(),"Token becomes logically returnable only after whole Parent Operation unpins");
   rejects([&]{auto operation=parent->enter(*moved);},"Returned job token cannot reenter Parent");
   returned.set();exit.wait();
  });
  worker.join();
  require(!prepared->close(0)&&prepared->outstanding_jobs()==1&&prepared->snapshot().phase==h::Phase::Returned,"Parent source closure still retains backing for live actual child/FLS exit");
  require(other->close(10000),"Independent same-Domain backing closes after actual original worker exit");
  exit.set();job.join();require(prepared->close(10000),"Controller closes exact job backing only after actual original and child exits");
  offer.reset();other.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parent job source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}
 catch(...){::ExitProcess(2);}
}
void parallel_parent_actors(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);
 Event a_ready,fail_entered,fail_release,a_caught,b_published,a_recheck,a_checked,a_release;
 Task worker,a,b;std::shared_ptr<s::SessionMemoryDomain> parent;
 std::optional<h::PreparedMemory::JobMemory> offer_a,offer_b,further;
 std::optional<s::SessionMemoryDomain::JobPermit> old_a,old_b;
 std::atomic<bool> a_live=false,b_finished=false;std::atomic<std::uintptr_t> a_bytes=0;
 std::atomic<s::SessionMemoryDomain::Operation*> a_operation=nullptr;
 f->heap.fail_entered=fail_entered.handle;f->heap.fail_release=fail_release.handle;
 // All wait/token/Task owners remain outside this try. Any finite-bound failure
 // exits without unwinding a live child's retained permission or wait owners.
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>(),s::OperationPolicy::OriginalConcurrentActors);
   require(parent->operation_policy()==s::OperationPolicy::OriginalConcurrentActors,"Only this lower fixture explicitly selects concurrent actors; actual Graph composition remains unchanged");
   {auto operation=parent->enter();offer_a.emplace(parent->prepare_job());offer_b.emplace(parent->prepare_job());}
   a.launch(*f->threads,[&]{
    offer_a->acquire();
    {
     auto operation=parent->enter(*offer_a);auto permit=operation.job_permit();old_a.emplace(permit);
     auto& context=parent->buffer_context();a_bytes=context.api.allocate_04b0bc(73);context.api.fill_bytes(a_bytes,0x6d,73);
     const auto retained=context.memory.view(a_bytes,73);a_operation=&operation;a_live=true;a_ready.set();
     f->heap.failing_actor=::GetCurrentThreadId();f->heap.fail_once=true;
     rejects<std::bad_alloc>([&]{(void)context.api.allocate_04b0bc(93);},"Known A allocation failure remains known while B owns a concurrently published allocation");
     permit.check(*parent);require(parent->snapshot().phase==s::SessionMemoryPhase::Open,"Sibling allocation count cannot poison the failed attempt's clean rollback");a_caught.set();
     a_recheck.wait();permit.check(*parent);
     require(b_finished&&std::all_of(retained.begin(),retained.end(),[](auto byte){return byte==0x6d;}),"B allocation/map growth/free, scope exit and physical reap preserve exact A permit and raw bytes");
     a_checked.set();a_release.wait();permit.check(*parent);a_operation=nullptr;a_live=false;
    }
    rejects([&]{old_a->check(*parent);},"A copied seal expires only at its own Operation exit");require(offer_a->retire(),"A logically returns after its own exact permission pin ends");
   });
   a_ready.wait();
   // Original admission and offer preparation must progress before A is
   // released, rather than passing only after the long body has finished.
   {auto operation=parent->enter();further.emplace(parent->prepare_job());require(a_live,"Original worker prepares a further offer while A's child Operation remains live");}
   require(further->cancel(),"Original cancels its never-acquired further offer without touching A");
   fail_entered.wait();
   b.launch(*f->threads,[&]{
    offer_b->acquire();std::vector<std::span<s::WriteFinishString*>> maps;maps.reserve(70);
    require(a_live&&a_operation.load(),"A publishes its actual still-live Operation before B's wrong-actor probe");
    rejects([&]{(void)a_operation.load()->job_permit();},"B cannot mint a permit from A's actual live Operation");
    require(parent->snapshot().phase==s::SessionMemoryPhase::Open,"Wrong-actor Operation permit request rejects before validating A PTD on B and leaves both healthy");
    {
     auto operation=parent->enter(*offer_b);auto permit=operation.job_permit();old_b.emplace(permit);
     auto& context=parent->buffer_context();const auto own=context.api.allocate_04b0bc(89);context.api.fill_bytes(own,0x39,89);
     rejects([&]{parent->free_sized_04b0f8(a_bytes,73);},"B cannot free the exact ticket borrowed by still-live A");
     require(parent->snapshot().phase==s::SessionMemoryPhase::Open,"Foreign borrowed-range free rejection does not poison either actor");
     s::DequeMapMemory lookup{{},&parent->output_map_resolver()};
     for(unsigned index=0;index<70;++index)maps.push_back(parent->allocate_output_map(index%2?16:8));
     const auto key=lookup.address_of(maps.front());const auto seen=lookup.view(key,maps.front().size_bytes());
     require(seen.data()==maps.front().data()&&seen.size()==maps.front().size()&&a_live,"Exact typed map resolver preserves the first mapping through seventy sibling catalog insertions");
     const auto aligned=parent->allocate_output_map(512);const auto aligned_key=lookup.address_of(aligned);std::uintptr_t aligned_base=0;
     require(s::lookup_allocation_header(aligned_key-8,{},&parent->allocation_header_resolver(),aligned_base)==s::MemoryLookup::Found&&aligned_base&&aligned_key-aligned_base>=8&&aligned_key-aligned_base<=39,"Actual large map header is resolved from its exact live Parent ticket without a borrowed header ledger");
     b_published.set();a_caught.wait();permit.check(*parent);
     require(context.memory.view(own,89)[88]==0x39,"B's own exact byte mapping survives A's allocation failure");
     for(const auto map:maps)parent->free_sized_04b0f8(lookup.address_of(map),map.size_bytes());
     parent->free_sized_04b0f8(aligned_base,aligned.size_bytes()+39);
     parent->free_sized_04b0f8(own,89);
     require(parent->snapshot().allocations==1&&a_live,"B physically frees its own borrowed views and maps immediately; only A's actual ticket remains");
    }
    rejects([&]{old_b->check(*parent);},"B copied seal expires while independent A is still live");require(offer_b->retire(),"B returns its own JobMemory without waiting for A");b_finished=true;
   });
   b_published.wait();fail_release.set();b.join();
   require(a_live&&parent->reap_jobs()==2&&prepared->outstanding_jobs()==1,"Original reaps physically exited B and canceled offer promptly while actual A remains borrowed");
   require(!parent->close()&&parent->snapshot().phase==s::SessionMemoryPhase::Open&&a_live,"Close returns false without waiting for A or invalidating its operation");
   a_recheck.set();a_checked.wait();a_release.set();a.join();
   require(parent->reap_jobs()==1&&!prepared->outstanding_jobs(),"Only actual A wrapper/FLS exit permits its final physical reaping");
   {auto operation=parent->enter();parent->free_sized_04b0f8(a_bytes,73);require(!parent->snapshot().allocations,"A's exact range becomes immediately free after its Operation ends");}
   require(parent->close()&&prepared->snapshot().phase==h::Phase::Returned,"Natural actor/ticket drain returns the actual original Borrow");
  });
  worker.join();require(prepared->close(10000),"Controller observes real original and both child native/FLS exits before destroying backing");
  old_a.reset();old_b.reset();parent.reset();offer_a.reset();offer_b.reset();further.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parallel lower actor source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
struct IoWorker final:s::WorkerRunApi {
 unsigned gets=0;
 void* tls_get(std::uint32_t index)override{require(index==17,"Owned IO fixture uses its actual borrowed TLS selector");++gets;return nullptr;}
 std::int32_t tls_set(std::uint32_t,void*)override{throw std::logic_error("Unselected IO fixture TLS publication");}
 std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override{throw std::logic_error("Unselected IO fixture stop");}
 std::uint32_t get_last_error()override{throw std::logic_error("Unselected IO fixture error path");}
};
struct IoFunctor final:s::PostedFunctor {
 unsigned calls=0;std::function<void()> effect;
 void call_slot_00(std::uint8_t flag)override{require(flag==1,"Actual posted completion consumes the moved fixture functor exactly once");++calls;if(effect)effect();}
};
void parallel_owned_io(bool pause_allocation){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);
 Event paused,release;Task worker,a,b;std::shared_ptr<s::SessionMemoryDomain> parent;std::unique_ptr<s::OwnedIoMemory> io;
 std::optional<h::PreparedMemory::JobMemory> offer_a,offer_b;std::uint32_t index=17;
 std::optional<s::PostedEnvironment> environment;s::OperationStorage* guard=nullptr;s::DispatcherOperationBinding* packet=nullptr,*external=nullptr;
 OVERLAPPED* overlapped=nullptr;std::uintptr_t identity=0;s::IoMemoryHandle guard_handle{},a_completed{};
 std::atomic<bool> a_held=false;IoWorker api_a,api_b;IoFunctor held,completed;
 f->heap.pause_entered=paused.handle;f->heap.pause_release=release.handle;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>(),s::OperationPolicy::OriginalConcurrentActors);
   io=std::make_unique<s::OwnedIoMemory>(parent,1);
   require(io->registry_capacity()==1,"Concurrent IO fixture starts with one real owner index slot");
   {
    auto operation=parent->enter();guard=&io->allocate_04b0bc(73);auto& posted=io->project_posted(*guard);guard_handle=io->handle_for(*guard);
    packet=&io->packet_binding(*guard);packet->transferred_14=27;external=packet;io->bind_external_packet(*guard,external);
    overlapped=&posted.operation.overlapped;identity=guard->identity;
    environment.emplace(s::PostedEnvironment{index,{},nullptr,&io->storage_resolver()});
    offer_a.emplace(parent->prepare_job());offer_b.emplace(parent->prepare_job());
   }
   a.launch(*f->threads,[&]{
    offer_a->acquire();
    {
     auto operation=parent->enter(*offer_a);auto permit=operation.job_permit();
     if(pause_allocation){
      f->heap.paused_actor=::GetCurrentThreadId();f->heap.pause_once=true;a_held=true;
      auto& in_flight=io->allocate_04b0bc(113);
      require(!a_held,"A native allocation returns only after B progress/join/reap was observed before release");io->free_04b788(in_flight);
     }else{
      auto& storage=io->allocate_04b0bc(73);auto& posted=io->project_posted(storage);a_completed=io->handle_for(storage);
      held.effect=[&]{
       // Actual027120 already freed its envelope. Only this moved functor and
       // independent guard row remain live; never inspect the retired envelope.
       require(io->snapshot().active_calls==1&&io->snapshot().allocations==1,"A callback holds one actual completion Call after physical envelope free");
       a_held=true;paused.set();release.wait();require(!a_held,"A actual completion callback returns only after B physically finishes/reaps");
      };
      posted.functor_38=&held;io->complete_posted(a_completed,1,*environment,api_a);
      require(held.calls==1,"Actual A completion invokes its moved functor once after sibling reuse");
     }
     permit.check(*parent);
    }
    require(offer_a->retire(),"A IO source returns its exact child offer only after its Operation ends");
   });
   paused.wait();
   b.launch(*f->threads,[&]{
    offer_b->acquire();
    {
     auto operation=parent->enter(*offer_b);auto permit=operation.job_permit();
     std::vector<s::OperationStorage*> rows;rows.reserve(70);std::vector<s::IoMemoryHandle> handles;handles.reserve(70);
     for(unsigned n=0;n<70;++n){auto& row=io->allocate_04b0bc(73);io->project_posted(row);rows.push_back(&row);handles.push_back(io->handle_for(row));}
     s::OperationStorage* found=nullptr;s::DispatcherOperationBinding* found_packet=nullptr;
     require(environment->lookup_storage(identity,found)==s::MemoryLookup::Found&&found==guard&&&io->packet_binding(*guard)==packet,"Copied exact storage resolver and original packet reference survive sibling owner-index growth");
     require(io->dispatcher_resolver().lookup_operation(overlapped,found_packet)==s::MemoryLookup::Found&&found_packet==packet&&packet->transferred_14==27&&external==packet,"Exact OVERLAPPED resolver and external packet field retain stable Record binding and contents");
     require(io->handle_for(*guard)==guard_handle&&a_held,"Growth preserves guard generation while A remains inside its real source effect");
     if(!pause_allocation){
      rejects<std::invalid_argument>([&]{(void)io->kind(a_completed);},"B may reuse A's already-freed completion slot without resurrecting its old generation");
      require(io->snapshot().active_calls==1,"B source allocation does not wait for or discard A's actual completion Call");
     }
     auto& last=io->project_posted(*rows.back());last.functor_38=&completed;const auto last_handle=io->handle_for(*rows.back());
     io->complete_posted(last_handle,1,*environment,api_b);
     require(completed.calls==1,"B actual posted callback completes while independent A remains held");
     rejects<std::invalid_argument>([&]{io->complete_posted(last_handle,1,*environment,api_b);},"Freed B completion generation cannot execute again");
     for(std::size_t n=0;n+1<rows.size();++n)io->free_04b788(*rows[n]);
     rejects<std::invalid_argument>([&]{(void)io->kind(handles.front());},"B physically retired storage generation remains stale");
     require(io->snapshot().phase==s::IoMemoryPhase::Open&&a_held&&!io->close(),"B retires disjoint rows immediately without poisoning or closing A's live source effect");permit.check(*parent);
    }
    require(offer_b->retire(),"B logical child return does not wait for A callback/native allocation return");
   });
   b.join();require(parent->reap_jobs()==1&&prepared->outstanding_jobs()==1&&a_held,"B actual wrapper/FLS exit and physical reap precede release of A's held IO effect");
   a_held=false;release.set();a.join();require(parent->reap_jobs()==1&&!prepared->outstanding_jobs(),"A's physical row retires only after real IO effect return and native/FLS exit");
   {
    auto operation=parent->enter();require(io->handle_for(*guard)==guard_handle&&&io->packet_binding(*guard)==packet&&external==packet,"Independent guard is still exact after both source actors exit");
    io->unbind_external_packet(external);io->free_04b788(*guard);guard=nullptr;
    require(!io->snapshot().allocations&&!io->snapshot().active_calls&&!parent->snapshot().allocations,"Actual completion/allocation effects and stable raw rows drain once");
   }
   require(io->close(),"IO provider releases its actual Parent lease after source drain");io.reset();environment.reset();require(parent->close(),"Original returns actual Prepared Borrow after IO provider retirement");
  });
  worker.join();require(prepared->close(10000),"Controller observes actual original/child FLS exits before backing destruction");
  offer_a.reset();offer_b.reset();parent.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parallel OwnedIO source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
struct QueueGate final:s::SocketGate {void enter()override{}void leave()override{}};
struct QueueLife final:s::LifecycleEffects {
 QueueGate gate;s::SocketService service{gate};
 s::SocketService& resolve_service_0239c0(s::ContextIdentity)override{return service;}
 void close_and_cancel_026fb0(s::SocketService&,s::SocketObject& object)override{object.state.socket=s::invalid_socket;}
 void executor_cleanup(s::SocketObject&,std::uintptr_t)override{}
};
void queue_executor(void*,s::ExecutorFields& to,s::ExecutorFields& from,std::uintptr_t){to=from;}
void release_owned_pair(s::SessionPair& pair){auto* control=pair.control;if(control&&control->strong_08.fetch_sub(1)==1){control->dispose_slot_00();if(control->weak_0c.fetch_sub(1)==1)control->delete_slot_08();}pair={};}
void parallel_owned_deques(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);Event paused,release;Task worker,a,b;
 auto life=std::make_shared<QueueLife>();std::shared_ptr<s::SessionMemoryDomain> parent;std::unique_ptr<anadius::owned_deque::Deques> deques;
 std::optional<h::PreparedMemory::JobMemory> offer_a,offer_b;std::atomic<bool> a_held=false;
 s::AcceptorState state_a,state_b;s::SocketObject pending_a{state_a},pending_b{state_b};s::SessionPair pair_a{},pair_b{};
 f->heap.pause_entered=paused.handle;f->heap.pause_release=release.handle;f->heap.pause_bytes=128;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),life,s::OperationPolicy::OriginalConcurrentActors);
   {auto operation=parent->enter();s::initialize_0211b0(pending_a,{73},*life);s::initialize_0211b0(pending_b,{74},*life);
    pair_a=parent->construct_session(pending_a,{nullptr,queue_executor});pair_b=parent->construct_session(pending_b,{nullptr,queue_executor});
    deques=std::make_unique<anadius::owned_deque::Deques>(*parent);deques->grow(pair_a.object->output.fields,0,[]{});deques->grow(pair_b.object->output.fields,0,[]{});
    require(parent->snapshot().maps==2&&deques->snapshot().successful_growths==2&&deques->snapshot().allocated_maps==2,"Two actual control-owned queue maps are seeded through recovered026670");
    offer_a.emplace(parent->prepare_job());offer_b.emplace(parent->prepare_job());
   }
   a.launch(*f->threads,[&]{offer_a->acquire();{
    auto operation=parent->enter(*offer_a);auto permit=operation.job_permit();f->heap.paused_actor=::GetCurrentThreadId();f->heap.pause_once=true;a_held=true;
    deques->grow(pair_a.object->output.fields,1,[&]{permit.check(*parent);},permit);
    require(!a_held&&pair_a.object->output.fields.map_size_d8==16,"A actual026670 resumes only after B queue growth/disposal and native exit");permit.check(*parent);
   }require(offer_a->retire(),"A queue growth offer returns only after its actual Operation ends");});
   paused.wait();
   b.launch(*f->threads,[&]{offer_b->acquire();{
    auto operation=parent->enter(*offer_b);auto permit=operation.job_permit();
    rejects([&]{deques->grow(pair_a.object->output.fields,1,[&]{permit.check(*parent);},permit);},"Foreign actor cannot concurrently mutate A's exact queue prefix");
    require(deques->snapshot().phase==anadius::owned_deque::Phase::Ready&&deques->snapshot().active&&a_held,"Same-queue conflict is nonpoisoning and leaves actual A growth active");
    deques->grow(pair_b.object->output.fields,1,[&]{permit.check(*parent);},permit);require(pair_b.object->output.fields.map_size_d8==16,"B different queue runs actual026670 while A's128-byte native map allocation is held");
    release_owned_pair(pair_b);permit.check(*parent);
    require(parent->snapshot().maps==1&&parent->snapshot().reserved==1&&deques->snapshot().successful_growths==3&&deques->snapshot().allocated_maps==3&&a_held,"B immediately disposes its real map/control while A retains only its own old map and private allocation prefix");
   }require(offer_b->retire(),"B queue source offer retires independently of A");});
   b.join();require(parent->reap_jobs()==1&&prepared->outstanding_jobs()==1&&a_held,"Actual B native/FLS exit and physical reaping precede release of A's allocator");
   {auto operation=parent->enter();require(!deques->close()&&deques->snapshot().phase==anadius::owned_deque::Phase::Ready&&deques->snapshot().active,"Original close reports active concurrent growth promptly without poisoning A");}
   a_held=false;release.set();a.join();require(parent->reap_jobs()==1&&!prepared->outstanding_jobs(),"Only actual A exit permits final queue actor reaping");
   {auto operation=parent->enter();require(deques->snapshot().successful_growths==4&&deques->snapshot().allocated_maps==4&&!deques->snapshot().active&&parent->snapshot().maps==1,"Both distinct queue growth calls retain exact cumulative metrics after A completes");
    release_owned_pair(pair_a);s::destroy_024db0(pending_a,*life);s::destroy_024db0(pending_b,*life);require(!parent->snapshot().maps&&!parent->snapshot().allocations,"Actual source owner disposal drains maps, proxy and controls without a union-catalog inference");require(deques->close(),"Original closes the deque provider only after actual calls drain");}
   deques.reset();require(parent->close(),"Deque lease and source ownership drain return the actual prepared Borrow");
  });
  worker.join();require(prepared->close(10000),"Controller verifies all actual queue worker/FLS exits before backing destruction");offer_a.reset();offer_b.reset();parent.reset();life.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parallel deque source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
[[noreturn]]void unselected_transfer(){throw std::logic_error("Unselected large-string transfer dependency");}
struct TransferEffects final:s::IoCompletionEffects {
 s::Category misc_category_0285b0()override{unselected_transfer();}
 void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unselected_transfer();}
 void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{unselected_transfer();}
 void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{unselected_transfer();}
 void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{unselected_transfer();}
};
struct TransferApi final:s::FrameReadApi,s::FrameReadTerminal {
 anadius::session_profile::Strings& strings;Ledger& heap;std::uintptr_t expected_base=0;std::uint64_t expected_bytes=0;unsigned frees=0;
 TransferApi(anadius::session_profile::Strings& s,Ledger& h):strings(s),heap(h){}
 void free_sized_04b0f8(std::uintptr_t address,std::uint64_t bytes)override{
  require(address==expected_base&&bytes==expected_bytes&&heap.contains(reinterpret_cast<void*>(address)),"Actual022280 frees the child-produced aligned response by its exact Parent raw base and extent");
  strings.free_sized(address,bytes);++frees;require(!heap.contains(reinterpret_cast<void*>(address)),"Response cleanup physically frees its own allocation while unrelated A view remains live");
 }
 [[noreturn]]void invalid_parameter_05c974()override{unselected_transfer();}void next_write_02a500(std::uintptr_t)override{unselected_transfer();}
 s::WriteFinishStream& begin_log_006880(std::uintptr_t,std::uintptr_t)override{unselected_transfer();}
 s::WriteFinishStream& append_007990(s::WriteFinishStream&,std::uintptr_t,std::uint64_t)override{unselected_transfer();}
 std::uint64_t widen_01e510(std::uintptr_t,std::uint8_t)override{unselected_transfer();}void put_00fe90(s::WriteFinishStream&,std::uint8_t)override{unselected_transfer();}void flush_00e1e0(s::WriteFinishStream&)override{unselected_transfer();}
 void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override{unselected_transfer();}
 s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t)override{unselected_transfer();}
 std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t,std::uint32_t,std::uintptr_t,s::FrameMessageClosure*,std::uint32_t,std::uint32_t&)override{unselected_transfer();}
 std::int32_t detach_04a458(const s::FrameThreadHandle&)override{unselected_transfer();}[[noreturn]]void thread_error_04a648(int)override{unselected_transfer();}[[noreturn]]void bad_weak_ptr_026860()override{unselected_transfer();}
};
void parallel_string_transfer(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);Event a_ready,a_release,pin_checked,pin_released,b_returned;Task worker,a,b;
 std::shared_ptr<s::SessionMemoryDomain> parent;std::unique_ptr<s::OwnedIoMemory> io;std::unique_ptr<anadius::session_profile::Strings> strings;
 std::optional<h::PreparedMemory::JobMemory> offer_a,offer_b;std::shared_ptr<void> sdk_pin;
 s::ReceiveBufferString request,transferred,response;std::uintptr_t request_base=0,transfer_base=0;std::uint64_t request_extent=0,transfer_extent=0;
 std::atomic<bool> root_active=false,a_held=false;const std::string request_text(5000,'R');std::uint32_t index=17;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>(),s::OperationPolicy::OriginalConcurrentActors);
   strings=std::make_unique<anadius::session_profile::Strings>(*parent,1);io=std::make_unique<s::OwnedIoMemory>(parent,1);
   {
    auto operation=parent->enter();root_active=true;strings->assign(request,request_text);strings->assign(transferred,std::string(7000,'T'));
    auto* resolver=strings->header_resolver();require(resolver==&parent->allocation_header_resolver(),"Concurrent Strings expose exactly their Parent allocation-header resolver");
    require(s::lookup_allocation_header(s::receive_buffer_address(request)-8,{},resolver,request_base)==s::MemoryLookup::Found&&s::lookup_allocation_header(s::receive_buffer_address(transferred)-8,{},resolver,transfer_base)==s::MemoryLookup::Found,"Original's two actual large strings have distinct owned aligned raw bases");
    request_extent=request.capacity_18+1+39;transfer_extent=transferred.capacity_18+1+39;
    std::span<s::WriteFinishAllocationHeader> frame_headers,finish_headers;
    rejects([&]{(void)strings->bindings();},"Concurrent header catalog observation rejects instead of pretending empty storage is drained");
    rejects([&]{(void)strings->live_headers();},"Concurrent legacy header count rejects instead of inventing zero live headers");
    rejects([&]{strings->bind_header_views(frame_headers,finish_headers);},"Concurrent Strings cannot bind a moving global header-ledger view");
    require(parent->snapshot().buffers==2,"Actual Parent ownership still records both live large strings after legacy observer rejection");
    sdk_pin=parent->pin_buffer(s::receive_buffer_address(transferred),transferred.capacity_18+1);
    offer_a.emplace(parent->prepare_job());offer_b.emplace(parent->prepare_job());
    // Declared last use of ALL original-actor aliases to this allocation.
    // Original keeps its unrelated request view and entire Operation active.
    strings->relinquish(transferred);
    a.launch(*f->threads,[&]{offer_a->acquire();{
     auto operation=parent->enter(*offer_a);auto permit=operation.job_permit();strings->prune();const auto owned=strings->read(request);
     require(owned==request_text,"A resolves only its actually requested immutable large payload after Concurrent prune");a_held=true;a_ready.set();a_release.wait();
     permit.check(*parent);require(owned==request_text&&strings->read(request)==request_text,"B transfer/free and original response cleanup preserve both A's actual Parent payload and owned copy");a_held=false;
    }require(offer_a->retire(),"A string view ends before logical child return");});
    a_ready.wait();
    b.launch(*f->threads,[&]{offer_b->acquire();{
     auto operation=parent->enter(*offer_b);auto permit=operation.job_permit();require(root_active&&a_held,"B begins while actual original Operation and independent A remain admitted");
     rejects([&]{parent->free_sized_04b0f8(transfer_base,transfer_extent);},"Original relinquish cannot consume an outstanding actual SDK range pin");
     require(parent->snapshot().buffer_borrows==1&&parent->snapshot().phase==s::SessionMemoryPhase::Open,"Rejected SDK-borrowed free retains the lease without poisoning");pin_checked.set();pin_released.wait();
     strings->assign(transferred,std::string(6000,'B'));require(root_active&&!f->heap.contains(reinterpret_cast<void*>(transfer_base)),"B physically replaces the transferred original allocation without waiting for Root Operation exit or A prune");strings->release(transferred);
     strings->copy(response,request);require(strings->read(response)==request_text&&s::receive_buffer_address(response)!=s::receive_buffer_address(request),"Actual009eb0 child response owns independent aligned bytes copied from the immutable root request");
     strings->relinquish(request);
     rejects([&]{parent->free_sized_04b0f8(request_base,request_extent);},"B relinquishing its own request view cannot release original/A views of that ticket");
     require(root_active&&a_held&&parent->snapshot().buffers==2&&!parent->snapshot().buffer_borrows,"Only immutable request and actual child response remain; SDK loan ended through its real lease");permit.check(*parent);
    }require(offer_b->retire(),"Child response Operation ends before its actual JobMemory return");b_returned.set();});
    pin_checked.wait();require(root_active&&parent->snapshot().buffer_borrows==1,"Original remains inside its actual Operation during B's SDK-pin rejection");sdk_pin.reset();pin_released.set();b_returned.wait();require(root_active&&a_held,"Child mutates/frees its transfer and returns before original Operation ends");root_active=false;
   }
   b.join();require(parent->reap_jobs()==1&&prepared->outstanding_jobs()==1&&a_held,"Child response has real native/FLS exit and physical reap before original consumption while A stays held");
   {
    auto operation=parent->enter();rejects([&]{parent->free_sized_04b0f8(request_base,request_extent);},"A's exact request borrow remains protected after original and B operation exits");
    TransferEffects effects;TransferApi api(*strings,f->heap);IoWorker worker_api;
    s::FrameReadContext frame{parent->buffer_context(),effects,api,{},{},{},api,{}};frame.allocation_header_resolver=strings->header_resolver();
    require(s::lookup_allocation_header(s::receive_buffer_address(response)-8,{},frame.allocation_header_resolver,api.expected_base)==s::MemoryLookup::Found,"Original cleanup resolves actual child-produced header without a global span");api.expected_bytes=response.capacity_18+1+39;
    s::OwnedResponseContext context{{index,{},nullptr,&io->storage_resolver()},worker_api,io->response_memory(),frame,nullptr};
    s::MessageWriteTask source;source.response_08=response;response={};s::PostedFunctor* output=nullptr;s::make_owned_response_020cb0(output,source,context);
    require(output&&source.response_08.size_10==0,"Actual020cb0 moves child response prefix into its independently owned task");auto* task=static_cast<s::OwnedResponseTask*>(output);const auto handle=io->handle_for(task->storage);io->invoke_task(handle,0);
    require(api.frees==1&&!io->snapshot().allocations&&parent->snapshot().buffers==1&&a_held,"Actual022280 discard frees child response exactly once while unrelated A request remains live");
   }
   require(io->close(),"Original closes the already-drained IO response provider while A's unrelated view remains live");io.reset();a_release.set();a.join();require(parent->reap_jobs()==1&&!prepared->outstanding_jobs(),"Actual A native/FLS exit permits final request ownership retirement");
   {auto operation=parent->enter();strings->release(request);require(!parent->snapshot().allocations&&!parent->snapshot().buffers,"Actual final large payload free drains Parent ownership without a legacy header-count assertion");}
   strings.reset();require(parent->close(),"String transfer and real source provider drain return the prepared Borrow");
  });
  worker.join();require(prepared->close(10000),"Controller verifies all actual original/child FLS exits before transfer backing destruction");offer_a.reset();offer_b.reset();parent.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parallel string/header transfer source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
namespace np=anadius::native_profile;namespace msg=anadius::messages_recovered;namespace ch=anadius::content_handlers_recovered;namespace dr=anadius::dispatch_recovered;namespace hr=anadius::handlers_recovered;namespace mh=anadius::misc_handlers_recovered;
// Unselected defaults/Router leaves fail explicitly. They supply no invented
// successful business/SDK behavior to the selected real GetConfig route.
struct NativeUnused final:msg::XmlBackend,msg::CryptoFactory,msg::Random,msg::Effects,np::Events,hr::Effects,ch::Effects,dr::LicenseHandlers,mh::IgoEffects,mh::NoResponse,ch::Mutex,ch::EventQueue {
 std::atomic<unsigned> calls=0;[[noreturn]]void trap(){++calls;throw std::logic_error("Unselected constructor dependency or native message leaf was used");}
 msg::ParseResult parse_c_string(std::string_view)override{trap();}std::unique_ptr<msg::XmlPrinter> printer()override{trap();}
 std::unique_ptr<msg::Cipher> create_0423d0()override{trap();}msg::Key challenge_bytes_034cf0()override{trap();}msg::Key key_from_response_034c30(std::string_view)override{trap();}
 int atoi_05ee08(std::string_view)override{trap();}bool registered_handler(std::uint32_t,const msg::XmlNode&,msg::XmlPrinter&)override{trap();}
 [[noreturn]]void invalid_message_02bd30_04d228()override{trap();}[[noreturn]]void thread_error_04a648(int)override{trap();}
 void log_received(std::string_view)override{trap();}void log_response(std::string_view)override{trap();}void log_event(std::string_view)override{trap();}void log_bad_xml(std::string_view,std::string_view)override{trap();}void log_decryption_failure(std::string_view)override{trap();}
 s::NativeMessageEventQueue& queue()override{trap();}s::NativeMessageMutex& mutex()override{trap();}int lock(s::NativeMessageMutex&)override{trap();}void unlock(s::NativeMessageMutex&)override{trap();}
 void log_profile_name_084f48(std::string_view)override{trap();}std::string current_time_041b60()override{trap();}
 void unknown_entitlement_field(std::string_view)override{trap();}std::string utc_time_041b60(std::uint64_t&)override{trap();}std::unique_ptr<ch::DocumentWriter> printer_0019b0()override{trap();}
 void write_xml_03c1d0(const ch::Path&,std::string_view)override{trap();}void save_steam_01f9a0(const ch::AchievementMap&)override{trap();}
 ch::Path filesystem_widen_006d70(std::string_view)override{trap();}ch::Path path_append_016b50(const ch::Path&,const ch::Path&)override{trap();}
 bool online_avatar_virtual_28(const ch::Path&)override{trap();}bool copy_overwrite_048d20(const ch::Path&,const ch::Path&)override{trap();}std::string path_to_utf8_031ca0(const ch::Path&)override{trap();}void log_saved_size_085530(std::int32_t)override{trap();}
 bool auth_code_02e3b0(const msg::XmlNode&,hr::Xml&)override{trap();}bool request_license_0320c0(const msg::XmlNode&,hr::Xml&)override{trap();}bool invalidate_license_030c50(const msg::XmlNode&,hr::Xml&)override{trap();}
 int lock_02c620_04a6ac()override{trap();}std::int32_t& lock_count_4c()override{trap();}void acquire_event_queue_02c400()override{trap();}void enqueue_034660(std::string&&)override{trap();}void unlock_04a6b4()override{trap();}
 [[noreturn]]void throw_no_response_02bcd0_04d228()override{trap();}int lock_04a6ac()override{trap();}std::int32_t& count_4c()override{trap();}
 ch::Mutex& mutex_02c620()override{trap();}bool initialized_locked()const override{const_cast<NativeUnused*>(this)->trap();}void initialize_locked(std::string)override{trap();}void push_locked_034660(std::string)override{trap();}
};
struct NativeCallXml final:msg::XmlBackend,dr::NodeAccess {
 anadius::xml_reader_bridge::Backend actual;std::atomic<unsigned> parses=0,printers=0;
 msg::ParseResult parse_c_string(std::string_view value)override{++parses;return actual.parse_c_string(value);}
 std::unique_ptr<msg::XmlPrinter> printer()override{++printers;return actual.printer();}
 std::vector<std::pair<std::string,std::string>> attributes_in_order(const msg::XmlNode& node)override{return actual.attributes_in_order(node);}
 const char* text_003860(const msg::XmlNode& node)override{return actual.text_003860(node);}
};
struct NativeBusiness {
 anadius::configuration::State configuration{};anadius::configuration::Path achievement_file;std::uint8_t online=0;std::int32_t saved_size=0;
};
struct NativeCallEffects final:msg::Effects,msg::UnknownMessageEffects {
 const std::shared_ptr<NativeBusiness> business;const std::shared_ptr<NativeCallXml> xml;const std::shared_ptr<NativeUnused> unused;
 std::function<void(const msg::UnknownMessageInput&)> diagnostic;std::atomic<unsigned> handlers=0,invalid=0,diagnostics=0;
 NativeCallEffects(std::shared_ptr<NativeBusiness> b,std::shared_ptr<NativeCallXml> x,std::shared_ptr<NativeUnused> u):business(std::move(b)),xml(std::move(x)),unused(std::move(u)){}
 int atoi_05ee08(std::string_view)override{unused->trap();}
 bool registered_handler(std::uint32_t rva,const msg::XmlNode& node,msg::XmlPrinter& printer)override{
  require(rva==0x02e5f0&&node.name()=="GetConfig","Actual032d00 selects recovered GetConfig registration rather than a synthetic answer");++handlers;auto& c=business->configuration;
  hr::Profile profile{c.username,c.persona_id,c.user_id,c.country,c.currency,c.subscriber_level,c.is_subscriber,c.is_under_age,c.is_steam_subscriber};
  dr::Router router{{c.game_attributes,c.emulator_attributes,profile,business->online,c.pretend_connected},
   {c.content,c.entitlements,c.game_attributes,c.achievements,c.achievements_set,c.content_id,c.user_id,business->achievement_file,c.avatar,c.default_avatar,c.user_directory,c.last_avatar_path,business->online,business->saved_size,*unused,*unused},
   {*xml,*unused,*unused,*unused,*unused,*unused}};
  return router.registered_handler(rva,node,printer);
 }
 [[noreturn]]void invalid_message_02bd30_04d228()override{++invalid;throw msg::UnknownMessageError{};}
 void unknown_message_07bd70(const msg::UnknownMessageInput& input)override{++diagnostics;require(static_cast<bool>(diagnostic),"Only A's supported unknown-message diagnostic is selected");diagnostic(input);}
 [[noreturn]]void thread_error_04a648(int)override{unused->trap();}
 void log_received(std::string_view)override{unused->trap();}void log_response(std::string_view)override{unused->trap();}void log_event(std::string_view)override{unused->trap();}void log_bad_xml(std::string_view,std::string_view)override{unused->trap();}void log_decryption_failure(std::string_view)override{unused->trap();}
};
void parallel_native_invocations(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);Event entered,release,b_returned;Task worker,a,b;
 auto life=std::make_shared<QueueLife>();std::shared_ptr<s::SessionMemoryDomain> parent;std::unique_ptr<anadius::session_profile::Strings> strings;std::unique_ptr<np::Logic> logic;
 std::optional<h::PreparedMemory::JobMemory> offer_a,offer_b;s::AcceptorState pending_state;s::SocketObject pending{pending_state};s::SessionPair pair{};
 auto business=std::make_shared<NativeBusiness>();auto unused=std::make_shared<NativeUnused>();auto xml_a=std::make_shared<NativeCallXml>(),xml_b=std::make_shared<NativeCallXml>();
 auto effect_a=std::make_shared<NativeCallEffects>(business,xml_a,unused),effect_b=std::make_shared<NativeCallEffects>(business,xml_b,unused);
 auto factory=std::make_shared<s::SessionCipherFactory>();auto clock=std::make_shared<anadius::crypto_recovered::WindowsTimeSource>();auto random=std::make_shared<anadius::crypto_recovered::Random>(*clock);
 s::ReceiveBufferString request_a,request_b,output_a,output_b;std::atomic<bool> a_held=false,root_active=false;
 const std::string id_a="A-"+std::string(5000,'a'),id_b="B-"+std::string(6000,'b');
 const auto wire_a="<LSX><Request id=\""+id_a+"\" recipient=\"EbisuSDK\"><UnsupportedParallel/></Request></LSX>";
 const auto wire_b="<LSX><Request id=\""+id_b+"\" recipient=\"EbisuSDK\"><GetConfig/></Request></LSX>";
 np::Dependencies deps_a,deps_b;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),life,s::OperationPolicy::OriginalConcurrentActors);strings=std::make_unique<anadius::session_profile::Strings>(*parent,1);
   {
    auto operation=parent->enter();root_active=true;s::initialize_0211b0(pending,{77},*life);pair=parent->construct_session(pending,{nullptr,queue_executor});
    require(pair.object->cipher.encrypted_c4==0,"Both actual messages share one constructed plaintext SessionOwner/cipher; no readiness/cipher override");
    np::Dependencies defaults;defaults.lifetime=business;defaults.xml=unused;defaults.factory=unused;defaults.random=unused;defaults.effects=unused;defaults.events=unused;
    logic=std::make_unique<np::Logic>(anadius::session_profile::SessionView{{0,1},*pair.object},defaults);
    deps_a={business,xml_a,factory,random,effect_a,unused,0,"call-A"};deps_b={business,xml_b,factory,random,effect_b,unused,0,"call-B"};
    require(effect_a->business==effect_b->business&&deps_a.xml!=deps_b.xml&&deps_a.effects!=deps_b.effects,"Invocation providers are distinct explicit views over the SAME business roots");
    strings->assign(request_a,wire_a);strings->assign(request_b,wire_b);offer_a.emplace(parent->prepare_job());offer_b.emplace(parent->prepare_job());
    // Both heap closures leave original ownership before children are eligible,
    // while the original Operation itself remains live until B returns.
    strings->relinquish(request_a);strings->relinquish(request_b);
    effect_a->diagnostic=[&](const msg::UnknownMessageInput& input){
     require(input.view()==wire_a&&!logic->failed_prefix()&&logic->active_calls()==1,"A reaches the actual supported unknown diagnostic with its own live large request");a_held=true;entered.set();release.wait();
     require(input.view()==wire_a&&a_held&&!logic->failed_prefix(),"B completion/response free does not invalidate held A request or convert it to a failed prefix");
    };
    a.launch(*f->threads,[&]{offer_a->acquire();{
     auto operation=parent->enter(*offer_a);auto permit=operation.job_permit();logic->process_native_call(*parent,*strings,output_a,request_a,deps_a,[&]{permit.check(*parent);});
     require(output_a.size_10==0&&!logic->failed_prefix()&&!logic->retained_prefix()&&!logic->active_calls(),"Supported A UnknownMessage returns normally and retires only its own final invocation");permit.check(*parent);strings->release(request_a);strings->release(output_a);a_held=false;
    }require(offer_a->retire(),"A exact source Operation ends before logical native child return");});
    entered.wait();
    b.launch(*f->threads,[&]{offer_b->acquire();{
     auto operation=parent->enter(*offer_b);auto permit=operation.job_permit();require(root_active&&a_held,"B starts on the same real Session/Logic while Root and A remain admitted");
     logic->process_native_call(*parent,*strings,output_b,request_b,deps_b,[&]{permit.check(*parent);});
     const auto text=strings->read(output_b);require(text.size()>4096&&text.back()==0&&text.find("GetConfigResponse")!=std::string::npos&&text.find(id_b)!=std::string::npos&&text.find(id_a)==std::string::npos,"Actual033d70/032d00/Router GetConfig echoes the long real B id into an independent large framed response");
     require(logic->active_calls()==1&&logic->retained_prefix()&&!logic->failed_prefix()&&a_held,"B retirement leaves healthy A active rather than clearing it or treating it as retained failure");
     strings->release(request_b);strings->relinquish(output_b);permit.check(*parent);
    }require(offer_b->retire(),"B response Operation ends before its real JobMemory return");b_returned.set();});
    b_returned.wait();require(root_active&&a_held&&logic->active_calls()==1&&!logic->failed_prefix(),"B actual source completes before A release and before original Operation exit");
    const auto result=strings->read(output_b);require(result.size()>4096&&result.find(id_b)!=std::string::npos,"Original consumes the exact child-produced heap response through reverse ownership handoff");strings->release(output_b);
    require(a_held&&parent->snapshot().buffers==1&&unused->calls==0,"B heap request/response physically drain while A remains held; constructor-default/unselected providers were never used");root_active=false;
   }
   b.join();require(parent->reap_jobs()==1&&prepared->outstanding_jobs()==1&&a_held,"B real wrapper/FLS exit is physically reaped before releasing A's diagnostic");release.set();a.join();require(parent->reap_jobs()==1&&!prepared->outstanding_jobs(),"A native/FLS exit is physically reaped only after its supported diagnostic returns");
   require(xml_a->parses==1&&xml_a->printers==0&&effect_a->invalid==1&&effect_a->diagnostics==1&&!effect_a->handlers&&xml_b->parses==1&&xml_b->printers==1&&effect_b->handlers==1&&!effect_b->invalid&&!effect_b->diagnostics&&unused->calls==0,"Each invocation retains its own real XML/Effects identity; no default dependency swap or sibling dispatch leakage");
   {auto operation=parent->enter();require(!logic->retained_prefix()&&!logic->failed_prefix()&&!logic->active_calls()&&!parent->snapshot().buffers,"Both normal invocations and their large payloads drain without hiding active state");logic.reset();release_owned_pair(pair);s::destroy_024db0(pending,*life);require(!parent->snapshot().allocations,"Actual same-session control/proxy lifetime ends after both child source calls");}
   strings.reset();require(parent->close(),"Native per-message source and actual owner drain return the prepared Borrow");
  });
  worker.join();require(prepared->close(10000),"Controller observes original and both same-session native/FLS exits before backing retirement");offer_a.reset();offer_b.reset();parent.reset();life.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parallel native invocation source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
void child_deque_permit(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);auto other=h::PreparedMemory::prepare(f->domain);
 Event ready,returned,exit;Task worker,job;
 auto life=std::make_shared<QueueLife>();std::shared_ptr<s::SessionMemoryDomain> parent,foreign;
 std::unique_ptr<anadius::owned_deque::Deques> deques;
 std::optional<h::PreparedMemory::JobMemory> offer;
 std::optional<s::SessionMemoryDomain::JobPermit> retained;
 s::AcceptorState pending_state;s::SocketObject pending{pending_state};s::SessionPair pair{};
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),life);
   auto wrong=other->acquire();foreign=std::make_shared<s::SessionMemoryDomain>(std::move(wrong),std::make_shared<Life>());
   {
    auto operation=parent->enter();rejects([&]{(void)operation.job_permit();},"Normal original-worker Operation cannot mint child permit");
    s::initialize_0211b0(pending,{73},*life);pair=parent->construct_session(pending,{nullptr,queue_executor});
    deques=std::make_unique<anadius::owned_deque::Deques>(*parent);offer.emplace(parent->prepare_job());
   }
   ready.set();returned.wait();
   {
    auto operation=parent->enter();require(pair.object->output.fields.map_size_d8==16&&deques->snapshot().successful_growths==2,"Original worker sees both actual child026670 growths and exact typed queue");
    if(pair.control->strong_08.fetch_sub(1)==1){pair.control->dispose_slot_00();if(pair.control->weak_0c.fetch_sub(1)==1)pair.control->delete_slot_08();}pair={};
    s::destroy_024db0(pending,*life);require(deques->close(),"Only original worker closes deque provider after actual owner/map disposal");
   }
   deques.reset();require(parent->close()&&foreign->close(),"Copied expired permits do not masquerade as live source/provider leases");
   parent.reset();foreign.reset();
  });
  ready.wait();
  job.launch(*f->threads,[&]{
   offer->acquire();
   {
    auto operation=parent->enter(*offer);auto permit=operation.job_permit();auto same=operation.job_permit();retained.emplace(permit);
    permit.check(*parent);same.check(*parent);
    rejects<std::invalid_argument>([&]{permit.check(*foreign);},"Child permit requires exact Parent, not merely same Domain");
    Task outsider;outsider.launch(*f->threads,[&]{rejects([&]{permit.check(*parent);},"Wrong actor rejects before waiting on current Parent gate");});outsider.join();
    rejects([&]{deques->grow(pair.object->output.fields,1,[]{});},"Default deque growth still rejects child worker");
    rejects([&]{(void)deques->close();},"Child permit does not grant deque lifecycle close");
    deques->grow(pair.object->output.fields,1,[&]{permit.check(*parent);},permit);
    require(pair.object->output.fields.map_size_d8==8&&deques->snapshot().successful_growths==1&&parent->output_map_bindings().size()==1,"Explicit child route runs actual026670 and registers one real typed map");
   }
   rejects([&]{retained->check(*parent);},"Copied permit expires at exact whole Operation exit");
   {
    auto operation=parent->enter(*offer);auto fresh=operation.job_permit();
    rejects([&]{retained->check(*parent);},"Previous operation seal cannot authorize a new scope on same actor/Parent");
    rejects([&]{deques->grow(pair.object->output.fields,1,[]{},*retained);},"Expired proof rejects before modifying provider or queue");
    fresh.check(*parent);deques->grow(pair.object->output.fields,1,[&]{fresh.check(*parent);},fresh);
    require(pair.object->output.fields.map_size_d8==16&&deques->snapshot().successful_growths==2,"Fresh child scope preserves copy/free/publication through second real growth");
   }
   require(offer->retire(),"Child returns only after all permitted deque source scopes end");returned.set();exit.wait();
  });
  worker.join();require(!prepared->close(0)&&other->close(10000),"Source map cleanup does not substitute for still-live child native/FLS exit");
  exit.set();job.join();require(prepared->close(10000),"All actual native exits permit backing retirement");
  retained.reset();offer.reset();life.reset();other.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Child deque source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}
 catch(...){::ExitProcess(2);}
}
struct InformationTrace {unsigned time=0,gmtime=0,format=0,admissions=0;};
struct InformationClock final:anadius::achievement_runtime::Clock {
 InformationTrace& trace;explicit InformationClock(InformationTrace& t):trace(t){}
 std::int64_t time_05eeb8()override{++trace.time;return 1577836800;}
 std::optional<anadius::achievement_runtime::BrokenDown> gmtime_05fffc(std::int64_t n)override{++trace.gmtime;require(n==1577836800,"Child Information uses exact supplied time");return anadius::achievement_runtime::BrokenDown{0,0,0,1,0,120,3,0,0};}
};
struct InformationFormatter final:anadius::achievement_runtime::Formatter {
 InformationTrace& trace;explicit InformationFormatter(InformationTrace& t):trace(t){}
 std::string put_time_03d610(const anadius::achievement_runtime::BrokenDown& b,std::string_view p)override{++trace.format;require(b.year==120&&p=="%Y-%m-%dT%H:%M:%S","Child Information retains existing UTC format");return "2020-01-01T00:00:00";}
};
struct InformationAdmission {
 s::SessionMemoryDomain& parent;const s::SessionMemoryDomain::JobPermit& permit;InformationTrace& trace;std::function<void()> hook;
 static void check(void* p){auto& c=*static_cast<InformationAdmission*>(p);c.permit.check(c.parent);++c.trace.admissions;if(c.hook)c.hook();}
 anadius::content_handlers_recovered::CallCheckpoint callback(){return {this,check};}
};
struct ProviderPlatform final:anadius::execution_recovered::Platform {
 [[noreturn]]void unused(){throw std::logic_error("Unselected provider fixture Execution primitive");}
 std::uint32_t current_thread_id()override{unused();}void acquire_srw_exclusive(std::uintptr_t&)override{unused();}void release_srw_exclusive(std::uintptr_t&)override{unused();}
 std::int64_t query_performance_frequency()override{unused();}std::int64_t query_performance_counter()override{unused();}void sleep_milliseconds(std::uint32_t)override{unused();}[[noreturn]]void raise_thread_error_04a648(int)override{unused();}
};
struct ProviderFileFailure final:std::runtime_error {ProviderFileFailure():std::runtime_error("Selected actual call-view raw file outcome is unknown") {}};
struct ProviderFiles final:anadius::file_recovered::Factory {
 using Path=anadius::configuration::Path;
 struct Row {std::uintptr_t raw=0;unsigned initial=0,writes=0,closes=0,destroyed=0;std::string bytes;};
 std::mutex gate;std::map<Path,Row> rows;Event& entered;Event& release;const bool fail_after_release;std::atomic<bool> held=false;
 const Path first=u"MEM\\call-A.xml",second=u"MEM\\call-B.xml";
 ProviderFiles(Event& e,Event& r,bool fail):entered(e),release(r),fail_after_release(fail){}
 Row row(const Path& p){std::lock_guard lock(gate);return rows.at(p);}
 struct Output final:anadius::file_recovered::Output {
  ProviderFiles& owner;const Path path;Output(ProviderFiles& o,const Path& p):owner(o),path(p){}
  ~Output()override{std::lock_guard lock(owner.gate);++owner.rows.at(path).destroyed;}
  std::uint32_t initial_state()const override{
   {std::lock_guard lock(owner.gate);++owner.rows.at(path).initial;}
   if(path==owner.first){owner.held=true;owner.entered.set();owner.release.wait();require(!owner.held,"A actual raw file callback returns only after B's separate guarded stream completes and is physically reaped");if(owner.fail_after_release)throw ProviderFileFailure{};}return 0;
  }
  void write_03be80(std::string_view value)override{std::lock_guard lock(owner.gate);auto& r=owner.rows.at(path);++r.writes;r.bytes.assign(value.data(),value.size());}
  void close_03ae70()override{std::lock_guard lock(owner.gate);++owner.rows.at(path).closes;}
 };
 std::unique_ptr<anadius::file_recovered::Input> open_input_039f70(const Path&,std::uint32_t,std::uint32_t)override{throw std::logic_error("No provider fixture input file selected");}
 std::unique_ptr<anadius::file_recovered::Output> open_output_03a1a0(const Path& p,std::uint32_t mode,std::uint32_t share)override{
  require((p==first||p==second)&&mode==0x20&&share==0x40,"Actual03c1d0 selects distinct owned paths and recovered binary mode/share");auto out=std::make_unique<Output>(*this,p);
  Row registered;registered.raw=reinterpret_cast<std::uintptr_t>(out.get());{std::lock_guard lock(gate);require(!rows.contains(p),"Each actual raw output opens once without replay");rows.emplace(p,std::move(registered));}return out;
 }
};
struct ProviderAdmission {
 s::SessionMemoryDomain* parent=nullptr;std::atomic<unsigned> calls=0;DWORD checkpoint_error=0;
 static void check(void* p){auto& v=*static_cast<ProviderAdmission*>(p);require(v.parent&&v.parent->operation_on_this_thread(),"Provider checkpoint executes under the actual caller's live Parent Operation");++v.calls;if(v.checkpoint_error)::SetLastError(v.checkpoint_error);}
 anadius::content_handlers_recovered::CallCheckpoint callback(){return {this,check};}
};
void parallel_provider_call_views(bool fail_a=false){
 namespace ie=anadius::information_effects;namespace ce=anadius::content_effects;namespace ai=anadius::application_io;namespace er=anadius::execution_recovered;
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);Event entered,release,b_checked,root_checked,b_returned;Task worker,a,b;
 std::shared_ptr<s::SessionMemoryDomain> parent;std::optional<h::PreparedMemory::JobMemory> offer_a,offer_b;
 anadius::configuration::State configuration{};anadius::bootstrap_backend::RuntimeSeed runtime{};ProviderPlatform platform;
 er::Execution execution(runtime.execution_mutex,{runtime.server_ready,runtime.online_ready,runtime.libraries_ready},runtime.frequency_cache,platform);
 anadius::achievement_storage::Paths paths{u"MEM\\achievements.xml",u"MEM\\achievements.ini"};
 auto files=std::make_shared<ProviderFiles>(entered,release,fail_a);auto stream=std::make_shared<std::ostringstream>();auto dialog=std::make_shared<anadius::library_effects::Dialog>();
 InformationTrace time;auto clock=std::make_shared<InformationClock>(time);auto formatter=std::make_shared<InformationFormatter>(time);
 std::unique_ptr<anadius::online_factory::Online> online;std::unique_ptr<ce::Content> content;std::unique_ptr<ie::Information> information;
 std::shared_ptr<ce::Content::CallView> view_a,view_b;std::shared_ptr<ie::Information::CallView> info_b;
 std::shared_ptr<ai::CallContext> context_a;ProviderAdmission admission_a,admission_b;std::string xml_b;
 const std::string xml_a("A\0owned",7);std::atomic<bool> root_active=false;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>(),s::OperationPolicy::OriginalConcurrentActors);admission_a.parent=parent.get();admission_b.parent=parent.get();
   {auto operation=parent->enter();root_active=true;er::initialize_mutex_04a698(runtime.execution_mutex,2);
    anadius::online_factory::Options o;o.stream=stream;o.dialog=dialog;o.foreign_quiescent=[]{return true;};o.code_lease=[](anadius::online_recovered::Module)->std::shared_ptr<void>{throw std::logic_error("No provider fixture foreign module lease selected");};
    online=std::make_unique<anadius::online_factory::Online>(anadius::bootstrap_backend::CallbackStateView{configuration,runtime,execution},std::move(o));
    ce::Options c;c.lifetime=files;c.stream=stream;c.dialog=dialog;c.files=files;c.clock=clock;c.formatter=formatter;content=std::make_unique<ce::Content>(anadius::bootstrap_backend::CallbackStateView{configuration,runtime,execution},paths,*online,c);
    ie::Options i;i.lifetime=files;i.stream=stream;i.clock=clock;i.formatter=formatter;information=std::make_unique<ie::Information>(i);
    require(online->matches({configuration,runtime,execution})&&online->snapshot().binding.object==0,"Same passive actual Online/Runtime/configuration roots back every Content view; no fabricated foreign object");
    offer_a.emplace(parent->prepare_job());offer_b.emplace(parent->prepare_job());
    a.launch(*f->threads,[&]{offer_a->acquire();{
     auto operation=parent->enter(*offer_a);auto permit=operation.job_permit();context_a=std::make_shared<ai::CallContext>(*parent,permit,admission_a.callback());view_a=content->make_call_view(*context_a);
     if(fail_a){
      rejects<ProviderFileFailure>([&]{view_a->write_xml_03c1d0(files->first,xml_a);},"Actual call-view GuardedFiles raw initial-state failure propagates after B physical exit");
      rejects([&]{view_a->close();},"Unknown call-view raw ownership cannot claim normal close");
      const auto before=admission_a.calls.load();rejects([&]{(void)content->make_call_view(*context_a);},"Failed view poisons its owner so a STILLVALID actual child context cannot mint a replacement");require(admission_a.calls==before,"Shared-health factory denial performs no checkpoint or raw replay");
      const auto raw=files->row(files->first);require(raw.initial==1&&!raw.writes&&!raw.closes&&!raw.destroyed,"Unknown raw outcome remains owned without synthesized close/destructor");
     }else{
      view_a->write_xml_03c1d0(files->first,xml_a);require(!files->held&&!root_active,"A actual Engine/GuardedFiles write resumes after B finishes and the original Operation ends");
      view_a->close();rejects([&]{view_a->write_xml_03c1d0(files->first,"repeat");},"Explicitly closed view rejects without replaying its actual raw file effect");
     }permit.check(*parent);
    }
    const auto before=admission_a.calls.load();rejects([&]{(void)content->make_call_view(*context_a);},"Same actor's expired CallContext cannot mint a fresh Content view");rejects([&]{(void)information->make_call_view(*context_a);},"Expired CallContext cannot mint Information authority either");require(admission_a.calls==before,"Stale factory denial reaches no borrowed checkpoint");
    require(offer_a->retire(),"A normal view close precedes Operation end and logical child return");});
    entered.wait();
    b.launch(*f->threads,[&]{offer_b->acquire();{
     auto operation=parent->enter(*offer_b);auto permit=operation.job_permit();require(files->held&&root_active,"B starts while A owns an actual pending raw output inside GuardedFiles");
     const auto before=admission_a.calls.load();rejects([&]{(void)content->make_call_view(*context_a);},"B cannot mint a Content view from A's live foreign-actor context");rejects([&]{(void)information->make_call_view(*context_a);},"B cannot mint Information authority from A's context");require(admission_a.calls==before,"Foreign actor rejects before A's checkpoint or SDK effect");
     ai::CallContext context(*parent,permit,admission_b.callback());admission_b.checkpoint_error=0x6942;::SetLastError(0x6941);context.check();require(::GetLastError()==0x6941,"Actual B context preserves admission-checkpoint LastError while SDK leaf effects remain outside this metadata check");admission_b.checkpoint_error=0;view_b=content->make_call_view(context);info_b=information->make_call_view(context);
     {auto writer=view_b->printer_0019b0();writer->xml().open_004080("CallB",false);writer->xml().attribute_005560("value","b<&");writer->xml().close_virtual_48(false);xml_b=writer->c_string_without_nul();require(xml_b.find("b&lt;&amp;")!=std::string::npos,"B uses actual owned XML printer and escaping in its own execution view");view_b->write_xml_03c1d0(files->second,xml_b);}
     std::uint64_t epoch=9;require(view_b->utc_time_041b60(epoch)=="2020-01-01T00:00:00"&&epoch==1577836800&&info_b->current_time_041b60()=="2020-01-01T00:00:00","Content and Information call views use the SAME controlled clock/formatter roots through actual UTC bodies");
     info_b->log_profile_name_084f48("call-B");b_checked.set();root_checked.wait();info_b->close();view_b->close();
     const auto first=files->row(files->first),second=files->row(files->second);require(first.initial==1&&!first.writes&&!first.closes&&!first.destroyed&&second.initial==1&&second.writes==1&&second.closes==1&&second.destroyed==1&&first.raw!=second.raw&&second.bytes==xml_b&&files->held,"B closes/destroys its own raw output while A's distinct raw owner remains live and untouched");permit.check(*parent);
    }require(offer_b->retire(),"B normal view closes before exact Operation end and actual logical return");b_returned.set();});
    b_checked.wait();require(content->path_append_016b50(u"MEM",u"root")==u"MEM\\root"&&information->current_time_041b60()=="2020-01-01T00:00:00","Legacy original provider calls remain available while both healthy child view types are still open and A holds a raw file");
    require(time.time==3&&time.gmtime==3&&time.format==3&&stream->str()=="profile name: call-B\n","Shared controlled clock and stream are used in explicit event order without concurrent trace writes");root_checked.set();
    b_returned.wait();require(root_active&&files->held,"B completes and returns while the original Operation and A's real file callback remain live");root_active=false;
   }
   b.join();require(parent->reap_jobs()==1&&prepared->outstanding_jobs()==1&&files->held,"B actual native/FLS exit and physical reap outside the original Operation finish before releasing A's raw file callback");
   files->held=false;release.set();a.join();require(parent->reap_jobs()==1&&!prepared->outstanding_jobs(),"A actual native/FLS exit retires its row outside the original Operation only after the real file callback and child source Operation end");
   if(fail_a){
    auto operation=parent->enter();rejects([&]{(void)content->path_append_016b50(u"MEM",u"denied");},"Actual original owner remains poisoned after the failed child has physically exited and reaped");
    const auto raw=files->row(files->first),done=files->row(files->second);require(raw.initial==1&&!raw.writes&&!raw.closes&&!raw.destroyed&&done.closes==1&&done.destroyed==1&&parent->snapshot().phase==s::SessionMemoryPhase::Open&&!parent->snapshot().allocations,"Owner registry retains failed raw prefix despite both actual child exits; unrelated B retired normally and Parent stays healthy");
    ::ExitProcess(704); // Preserve actual failed owner/raw prefix; never fake cleanup.
   }
   {auto operation=parent->enter();const auto first=files->row(files->first);require(first.initial==1&&first.writes==1&&first.closes==1&&first.destroyed==1&&first.bytes==xml_a&&admission_a.calls>0&&admission_b.calls>0&&!parent->snapshot().allocations,"Counted NUL file bytes and both exact raw ownership lifetimes retire once through their own checkpoints");
    information.reset();content.reset();require(online->close_after_cleanup(),"Unused actual Online closes empty without any game SDK/load/query");online.reset();}
   require(parent->close(),"Closed call views leave no live authority or Parent source allocation");
  });
  worker.join();require(prepared->close(10000),"Controller observes actual original/child FLS exits before provider backing retirement");
  // Explicit close makes these destructors metadata-only on a later actor.
  info_b.reset();view_b.reset();view_a.reset();context_a.reset();offer_a.reset();offer_b.reset();parent.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Parallel provider call-view source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
void child_information_permit(){
 namespace ie=anadius::information_effects;
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);auto other=h::PreparedMemory::prepare(f->domain);
 Event ready,returned,exit;Task worker,job;InformationTrace trace;
 auto stream=std::make_shared<std::ostringstream>();auto clock=std::make_shared<InformationClock>(trace);auto formatter=std::make_shared<InformationFormatter>(trace);
 std::shared_ptr<s::SessionMemoryDomain> parent,foreign;std::unique_ptr<ie::Information> information,reentrant;
 std::optional<h::PreparedMemory::JobMemory> offer;std::optional<s::SessionMemoryDomain::JobPermit> retained;
 try{
  worker.launch(*f->threads,[&]{
   auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>());
   auto wrong=other->acquire();foreign=std::make_shared<s::SessionMemoryDomain>(std::move(wrong),std::make_shared<Life>());
   {auto operation=parent->enter();ie::Options options;options.stream=stream;options.clock=clock;options.formatter=formatter;information=std::make_unique<ie::Information>(options);reentrant=std::make_unique<ie::Information>(options);offer.emplace(parent->prepare_job());}
   ready.set();returned.wait();
   {auto operation=parent->enter();require(information->current_time_041b60()=="2020-01-01T00:00:00"&&trace.time==3&&trace.gmtime==3&&trace.format==3,"Original worker resumes ordinary Information after child scopes end");require(stream->str()=="profile name: child\n","Child log preserves exact prefix/name/newline once");information.reset();reentrant.reset();}
   require(parent->close()&&foreign->close(),"Information descriptor leaves no active permission or provider lease");parent.reset();foreign.reset();
  });
  ready.wait();job.launch(*f->threads,[&]{
   offer->acquire();
   {
    auto operation=parent->enter(*offer);auto permit=operation.job_permit();retained.emplace(permit);InformationAdmission admission{*parent,permit,trace,{}};
    rejects([&]{(void)information->current_time_041b60();},"Default Information remains original-worker only");
    rejects<std::invalid_argument>([&]{(void)information->current_time_job(*foreign,permit,admission.callback());},"Child Information rejects a different Parent before opaque calls");
    Task outsider;outsider.launch(*f->threads,[&]{rejects([&]{(void)information->current_time_job(*parent,permit,admission.callback());},"Wrong Information actor rejects before held Parent gate");});outsider.join();
    require(!trace.time&&!trace.gmtime&&!trace.format&&!trace.admissions&&stream->str().empty(),"Denied default/cross-root/cross-actor calls publish no leaf effects");
    ::SetLastError(0x47f1);require(information->current_time_job(*parent,permit,admission.callback())=="2020-01-01T00:00:00"&&::GetLastError()==0x47f1,"Explicit Information job route preserves UTC and LastError");
    information->log_profile_name_job("child",*parent,permit,admission.callback());
    unsigned swallowed=0;InformationAdmission recursive{*parent,permit,trace,{}};
    recursive.hook=[&]{try{(void)reentrant->current_time_job(*parent,permit,recursive.callback());}catch(const std::logic_error&){++swallowed;}};
    rejects([&]{(void)reentrant->current_time_job(*parent,permit,recursive.callback());},"Swallowed admission reentry cannot complete Information outer call");
    require(swallowed==1&&trace.time==1&&trace.gmtime==1&&trace.format==1,"Admission reentry rejects before any nested or outer clock primitive");
   }
   InformationAdmission expired{*parent,*retained,trace,{}};
   const auto before=trace.admissions;rejects([&]{(void)information->current_time_job(*parent,*retained,expired.callback());},"Copied expired Information permit cannot resume authority");require(trace.admissions==before&&trace.time==1,"Expired permission reaches neither callback nor clock");
   {auto operation=parent->enter(*offer);auto fresh=operation.job_permit();InformationAdmission admission{*parent,fresh,trace,{}};rejects([&]{(void)information->current_time_job(*parent,*retained,expired.callback());},"Old seal remains invalid during fresh same-actor scope");require(information->current_time_job(*parent,fresh,admission.callback())=="2020-01-01T00:00:00"&&trace.time==2,"Fresh explicit scope uses existing UTC helper again");}
   require(offer->retire(),"Information synchronous descriptor ends before logical child return");returned.set();exit.wait();
  });
  worker.join();require(!prepared->close(0)&&other->close(10000),"Information completion does not substitute for child native/FLS exit");exit.set();job.join();require(prepared->close(10000),"Physical exit permits Information fixture backing retirement");
  retained.reset();offer.reset();formatter.reset();clock.reset();stream.reset();other.reset();prepared.reset();f->close();delete f;
 }catch(const std::exception& e){std::cerr<<"Child Information source scenario failed: "<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){::ExitProcess(2);}
}
void parent_borrow(){
 auto* f=new Fixture;auto prepared=h::PreparedMemory::prepare(f->domain);Event ready,proceed,idle,finish,returned,exit;Task worker;std::shared_ptr<s::SessionMemoryDomain> parent;std::atomic<bool> life_retired=false;
 worker.launch(*f->threads,[&]{auto ticket=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(ticket),std::make_shared<Life>());rejects([&]{(void)ticket.memory();},"Parent owns moved worker borrow without aliasing shared_ptr");require(parent->memory_domain_identity()==f->domain.get(),"New Parent overload keeps exact originating Domain identity");
  std::uintptr_t allocation=0;{auto op=parent->enter();auto& bytes=parent->buffer_context();allocation=bytes.api.allocate_04b0bc(129);bytes.api.fill_bytes(allocation,0x3a,129);require(parent->snapshot().allocations==1,"Actual Parent typed bytes use controller-owned borrowed ApplicationMemory");}
  require(!parent->close()&&prepared->snapshot().phase==h::Phase::Borrowed,"Live source Parent block prevents implicit return of borrow");ready.set();proceed.wait();
  {auto op=parent->enter();auto bytes=parent->buffer_context().memory.view(allocation,129);require(std::all_of(bytes.begin(),bytes.end(),[](auto b){return b==0x3a;}),"Rejected foreign Parent admission preserves all source bytes");parent->free_sized_04b0f8(allocation,129);}
  idle.set();finish.wait();
  require(parent->close()&&parent->snapshot().phase==s::SessionMemoryPhase::Closed&&prepared->snapshot().phase==h::Phase::Returned,"Successful worker Parent close returns Borrow instead of destroying controller provider");life_retired=true;parent.reset();returned.set();exit.wait();
 });ready.wait();
 rejects([&]{auto op=parent->enter();},"Controller cannot enter Parent whose dedicated borrow belongs to actual worker");
 proceed.set();idle.wait();
 rejects([&]{(void)parent->close();},"Controller cannot close idle dedicated worker Parent");rejects([&]{(void)parent->acquire_provider_lease();},"Controller cannot mutate provider leases on worker-owned borrowed Parent");
 Task sibling;sibling.launch(*f->threads,[&]{rejects([&]{auto op=parent->enter();},"Sibling same-Domain worker cannot enter dedicated borrowed Parent");rejects([&]{(void)parent->close();},"Sibling cannot close idle borrowed Parent");rejects([&]{(void)parent->acquire_provider_lease();},"Sibling cannot acquire provider lease on borrowed Parent");});sibling.join();
 require(parent->snapshot().phase==s::SessionMemoryPhase::Open&&!parent->snapshot().provider_leases&&!parent->snapshot().allocations,"Rejected idle Parent mutation probes preserve open allocation-free source and zero leases");
 require(prepared->snapshot().phase==h::Phase::Borrowed,"Foreign Parent probes do not retire actual worker capability");finish.set();returned.wait();require(life_retired&&!prepared->close(0),"Parent logical return still cannot substitute for actual worker FLS exit");exit.set();worker.join();require(prepared->close(10000),"Controller closes actual worker handoff after borrowed Parent source drain");prepared.reset();f->close();delete f;
}
void abandoned_child(){
 auto* f=new Fixture;auto* prepared=h::PreparedMemory::prepare(f->domain).release();Task worker;void* retained=nullptr;worker.launch(*f->threads,[&]{auto ticket=prepared->acquire();retained=ticket.memory().allocate(37);require(retained!=nullptr,"Abandoned child owns actual application block before unknown return");});worker.join();auto snapshot=prepared->snapshot();require(snapshot.phase==h::Phase::Poisoned&&snapshot.worker_handle_retained&&!snapshot.worker_exit_verified,"Unreturned Borrow destruction leaves sticky retained ticket with actual worker handle");require(!prepared->close(0)&&!prepared->cancel()&&f->heap.contains(retained),"Abandoned capability cannot synthesize source drain or free retained application allocation");::ExitProcess(701);
}
void abandoned_job_child(){
 auto* f=new Fixture;auto* prepared=h::PreparedMemory::prepare(f->domain).release();Task worker;void* retained=nullptr;
 worker.launch(*f->threads,[&]{
  auto ticket=prepared->acquire();auto token=ticket.prepare_job();Task child;
  child.launch(*f->threads,[&]{auto abandoned=std::move(token);abandoned.acquire();retained=abandoned.memory().allocate(37);});
  child.join();require(prepared->snapshot().phase==h::Phase::Poisoned&&prepared->outstanding_jobs()==1,"Abandoned job token poisons root and retains exact child record after actual exit");
  rejects([&]{(void)ticket.memory();},"Original worker cannot resume a poisoned shared backing");
 });
 worker.join();require(!prepared->close(0)&&!prepared->cancel()&&retained&&f->heap.contains(retained),"Abandoned job neither synthesizes source drain nor releases actual retained allocation");::ExitProcess(702);
}
void abandoned_parent_job_child(){
 auto* f=new Fixture;auto* prepared=h::PreparedMemory::prepare(f->domain).release();Task worker;
 std::shared_ptr<s::SessionMemoryDomain> parent;std::uintptr_t retained=0;
 worker.launch(*f->threads,[&]{
  auto borrow=prepared->acquire();parent=std::make_shared<s::SessionMemoryDomain>(std::move(borrow),std::make_shared<Life>());
  std::optional<h::PreparedMemory::JobMemory> token;
  {auto operation=parent->enter();token.emplace(parent->prepare_job());}
  Task child;child.launch(*f->threads,[&]{
   token->acquire();auto operation=parent->enter(*token);auto& bytes=parent->buffer_context();retained=bytes.api.allocate_04b0bc(37);
   token.reset(); // Permission pin outlives token and retains exact row/root.
   rejects([&]{(void)parent->buffer_context();},"Abandoned active token cannot retain source permission through stable scope");
  });child.join();
  require(parent->snapshot().phase==s::SessionMemoryPhase::Poisoned&&prepared->snapshot().phase==h::Phase::Poisoned,"Active token destruction poisons Parent and retained root without dangling scope");
  require(!parent->close(),"Poisoned Parent cannot claim completed source retirement");
 });worker.join();
 require(retained&&f->heap.contains(reinterpret_cast<void*>(retained))&&!prepared->close(0)&&prepared->outstanding_jobs()==1,"Abandoned pinned Parent keeps exact allocation, child record and backing owners");::ExitProcess(703);
}
void isolated(const wchar_t* mode=L"--abandoned",DWORD expected=701){std::array<wchar_t,32768> path{};auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));require(n&&n<path.size(),"Own handoff fixture EXE path found");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" "+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};require(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"Own isolated unknown-prefix fixture launched");DWORD code=0;require(::WaitForSingleObject(process.hProcess,30000)==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==expected,"Abandoned real worker capability reaches exact retained-prefix assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
}
int main(int argc,char** argv){try{if(argc==2){if(std::string_view(argv[1])=="--failed-provider-view"){parallel_provider_call_views(true);return 1;}if(std::string_view(argv[1])=="--abandoned-parent-job")abandoned_parent_job_child();else if(std::string_view(argv[1])=="--abandoned-job")abandoned_job_child();else abandoned_child();}validation_and_cancel();actual_worker_borrow();job_memory_handoff();parent_job_borrow();parallel_parent_actors();parallel_owned_io(false);parallel_owned_io(true);parallel_owned_deques();parallel_string_transfer();parallel_native_invocations();parallel_provider_call_views();child_deque_permit();child_information_permit();parent_borrow();isolated();isolated(L"--abandoned-job",702);isolated(L"--abandoned-parent-job",703);isolated(L"--failed-provider-view",704);std::cout<<"PASS: "<<checks<<" worker-handoff core checks; actual Domain NativeThreads/PTD/FLS exit, concurrent lower actors and stable OwnedIO effects, scoped child deque/Information source and three isolated retained borrows\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){std::cerr<<"Unknown own worker-handoff test exception\n"<<std::flush;::ExitProcess(2);}}
