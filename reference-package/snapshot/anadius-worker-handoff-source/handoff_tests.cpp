#include <winsock2.h>
#include <exception>
#include "handoff.hpp"
#include "../anadius-session-memory-source/memory.hpp"
#include <algorithm>
#include <atomic>
#include <cstring>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <thread>
namespace {
namespace h=anadius::worker_handoff;namespace d=anadius::thread_domain;namespace s=anadius::socket_recovered;namespace t=anadius::threads_recovered;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E=std::logic_error,class F>void rejects(F f,const char* text){bool seen=false;try{f();}catch(const E&){seen=true;}require(seen,text);}
struct Ledger {
 std::mutex gate;std::map<void*,std::size_t> blocks;std::atomic<std::size_t> allocations=0,frees=0;std::atomic<bool> valid=true;
 std::size_t size(){std::lock_guard lock(gate);return blocks.size();}
 bool contains(void* p){std::lock_guard lock(gate);return blocks.contains(p);}
};
Ledger* ledger=nullptr;
LPVOID WINAPI allocate(HANDLE heap,DWORD flags,SIZE_T size){auto* p=::HeapAlloc(heap,flags,size);const auto error=::GetLastError();if(p){std::lock_guard lock(ledger->gate);if(!ledger->blocks.emplace(p,size).second)ledger->valid=false;++ledger->allocations;}::SetLastError(error);return p;}
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
 void join(){require(handle&&::WaitForSingleObject(handle,10000)==WAIT_OBJECT_0,"Actual native worker handle signals after source wrapper/FLS callbacks");DWORD code=0;require(::GetExitCodeThread(handle,&code)&&code==63,"Actual source worker exits with selected own callback return");require(::CloseHandle(handle)!=0,"Own original worker handle closed after join");handle=nullptr;if(error)std::rethrow_exception(error);}
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
void isolated(){std::array<wchar_t,32768> path{};auto n=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));require(n&&n<path.size(),"Own handoff fixture EXE path found");std::wstring command=L"\""+std::wstring(path.data(),n)+L"\" --abandoned";STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};require(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"Own isolated unknown-prefix fixture launched");DWORD code=0;require(::WaitForSingleObject(process.hProcess,30000)==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&code)&&code==701,"Abandoned real worker capability reaches exact retained-prefix assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
}
int main(int argc,char**){try{if(argc==2)abandoned_child();validation_and_cancel();actual_worker_borrow();parent_borrow();isolated();std::cout<<"PASS: "<<checks<<" worker-handoff core checks; actual Domain NativeThreads/PTD/FLS exit and one isolated retained borrow\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){std::cerr<<"Unknown own worker-handoff test exception\n"<<std::flush;::ExitProcess(2);}}
