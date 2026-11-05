#include <winsock2.h>
#include <exception>
#include "handoff.hpp"
#include <optional>
#include <string_view>
#include <vector>
#include "../anadius-session-memory-source/memory.hpp"
#include "../anadius-owned-deque-source/deque.hpp"
#include "../anadius-io-memory-source/io_memory.hpp"
#include "../anadius-information-effects-source/effects.hpp"
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
 std::atomic<DWORD> paused_actor=0;std::atomic<bool> pause_once=false;
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
 if(size==113&&::GetCurrentThreadId()==ledger->paused_actor.load()&&ledger->pause_once.exchange(false)){
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
int main(int argc,char** argv){try{if(argc==2){if(std::string_view(argv[1])=="--abandoned-parent-job")abandoned_parent_job_child();else if(std::string_view(argv[1])=="--abandoned-job")abandoned_job_child();else abandoned_child();}validation_and_cancel();actual_worker_borrow();job_memory_handoff();parent_job_borrow();parallel_parent_actors();parallel_owned_io(false);parallel_owned_io(true);child_deque_permit();child_information_permit();parent_borrow();isolated();isolated(L"--abandoned-job",702);isolated(L"--abandoned-parent-job",703);std::cout<<"PASS: "<<checks<<" worker-handoff core checks; actual Domain NativeThreads/PTD/FLS exit, concurrent lower actors and stable OwnedIO effects, scoped child deque/Information source and three isolated retained borrows\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){std::cerr<<"Unknown own worker-handoff test exception\n"<<std::flush;::ExitProcess(2);}}
