#include "../anadius-bootstrap-native-source/native_entries.hpp"
#include <array>
#include <atomic>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
namespace {
namespace n=anadius::bootstrap_native_recovered;namespace r=anadius::recovered;
std::atomic<unsigned> checks=0;
void require(bool value,const char* text){++checks;if(!value){std::cerr<<"FAILED: "<<text<<'\n'<<std::flush;throw std::runtime_error(text);}}
struct Fault {n::LifecycleFault value;};
struct Event {HANDLE value=::CreateEventW(nullptr,TRUE,FALSE,nullptr);Event(){require(value!=nullptr,"Own admission test event created");}~Event(){::CloseHandle(value);}void signal(){require(::SetEvent(value)!=0,"Own admission event signaled");}void wait(){require(::WaitForSingleObject(value,10000)==WAIT_OBJECT_0,"Own admission event observed within finite bound");}};
struct Dependencies;std::atomic<Dependencies*> current=nullptr;
void first();void second();void boolean(bool);void blocked();
struct Dependencies final:r::BootstrapDependencies {
 std::atomic<unsigned> operations=0,registrations=0,installations=0,environments=0,consoles=0,configurations=0,cleanups=0,deletions=0,exit_steps=0,guards=0,noarg_calls=0,true_calls=0,false_calls=0,terminate_calls=0;
 std::mutex gate;std::map<void*,std::size_t> allocations;struct Job {r::ThreadEntry entry;void* payload;};std::vector<Job> jobs;
 Event app_entered,app_release,worker_entered,worker_release;bool block_app=false,throw_app=false;
 void effect(){++operations;}
 int register_exit(r::RegisteredCallback value)override{effect();require(value==r::RegisteredCallback::exit_038950,"Actual Bootstrap registers selected exit callback token");++registrations;return-1;}
 void install_terminate(r::RegisteredCallback value)override{effect();require(value==r::RegisteredCallback::terminate_038890,"Actual Bootstrap installs selected terminate token");++installations;}
 void setup_environment_019910()override{effect();++environments;}void setup_console_038760()override{effect();++consoles;}
 void* log_insert_006880(void* out,std::uintptr_t)override{effect();return out;}void log_finish_0088e0(void*)override{effect();}
 void load_configuration_01c870()override{effect();++configurations;if(block_app){app_entered.signal();app_release.wait();}if(throw_app)throw std::runtime_error("Own selected application provider failure");}
 void* allocate_04b0bc(std::size_t n)override{effect();auto* value=::operator new(n);std::lock_guard lock(gate);allocations.emplace(value,n);return value;}
 void delete_04b0f8(void* p,std::size_t n)override{effect();{std::lock_guard lock(gate);auto it=allocations.find(p);require(it!=allocations.end()&&it->second==n,"Native worker source deletes exact transferred payload once");allocations.erase(it);}++deletions;::operator delete(p);::SetLastError(0x51e1);}
 r::NoArgumentFunction worker(r::Worker value)override{effect();return value==r::Worker::first_029ea0?first:second;}
 r::BooleanFunction boolean_worker_035850()override{effect();return boolean;}
 r::Handle begin_thread_05e8e0(r::ThreadEntry entry,void* payload,std::uint32_t* id)override{effect();std::lock_guard lock(gate);jobs.push_back({entry,payload});*id=static_cast<std::uint32_t>(jobs.size());return 100+jobs.size();}
 bool close_handle(r::Handle value)override{effect();require(value>=101&&value<=103,"Only selected fake launch handles are detached; no native HANDLE reinterpretation");return true;}
 [[noreturn]]void throw_thread_error_04a648(int)override{throw std::runtime_error("Own thread source failure");}
 void between_workers_00e310()override{effect();}void guard_return_017a30()override{effect();++guards;::SetLastError(0x51a7);}
 void cleanup_threads_04a47c()override{effect();++cleanups;}
 void cleanup_035ab0()override{effect();require(exit_steps.fetch_add(1)%3==0,"Actual exit starts first source cleanup");}
 void cleanup_0366f0()override{effect();require(exit_steps.fetch_add(1)%3==1,"Actual exit then invokes second source cleanup");}
 void cleanup_0102b0()override{effect();require(exit_steps.fetch_add(1)%3==2,"Actual exit finally invokes network cleanup");::SetLastError(0x51d3);}
 void terminate_log_038920()override{effect();++terminate_calls;throw std::runtime_error("Own selected terminate boundary");}
 [[noreturn]]void unexpected_terminate_return_trap()override{throw std::runtime_error("Unexpected selected terminate continuation");}
 void drain_jobs(){std::vector<Job> pending;{std::lock_guard lock(gate);pending.swap(jobs);}for(const auto& job:pending){auto fn=n::Resolver{}.thread_entry(job.entry);require(fn(job.payload)==0,"Queued real Bootstrap payload remains admitted through worker route after application seal");}}
 std::size_t outstanding(){std::lock_guard lock(gate);return allocations.size();}
};
void first(){++current.load()->noarg_calls;}void second(){++current.load()->noarg_calls;}void boolean(bool value){auto* d=current.load();if(value)++d->true_calls;else ++d->false_calls;}
void blocked(){auto* d=current.load();d->worker_entered.signal();d->worker_release.wait();}
struct Errors final:n::Errors {
 std::atomic<unsigned> lifecycle_calls=0,exceptions=0;n::Binding* binding=nullptr;Dependencies* dependencies=nullptr;std::string child;
 [[noreturn]]void lifecycle(n::LifecycleFault value)override{++lifecycle_calls;throw Fault{value};}
 [[noreturn]]void callback_exception(n::Entry entry,std::exception_ptr exception)noexcept override{
  ++exceptions;bool typed=false;try{if(exception)std::rethrow_exception(exception);}catch(const std::exception&){typed=true;}catch(...){}
  if(!typed||n::active_calls()!=0)::ExitProcess(1197);
  if(child=="application"){if(entry!=n::Entry::Application||!binding||!binding->pause_application_admission()||!n::is_bound()||dependencies->configurations!=1)::ExitProcess(1196);auto before=dependencies->operations.load();::SetLastError(0x51c1);if(n::Resolver{}.application()(nullptr,1,nullptr)!=FALSE||::GetLastError()!=0x51c1||exceptions!=1||dependencies->operations!=before)::ExitProcess(1195);::ExitProcess(1101);}
  if(child=="terminate"){if(entry!=n::Entry::Terminate||dependencies->terminate_calls!=1||dependencies->registrations||dependencies->configurations)::ExitProcess(1194);::ExitProcess(1102);}
  if(child=="closed"){if(entry!=n::Entry::Application||n::is_bound()||dependencies->operations)::ExitProcess(1193);::ExitProcess(1104);}
  std::cerr<<"Unexpected own native-entry callback exception "<<static_cast<int>(entry)<<'\n'<<std::flush;::ExitProcess(1199);
 }
};
void main_admission(){Dependencies dependencies;current=&dependencies;r::Bootstrap bootstrap(dependencies);Errors errors;errors.dependencies=&dependencies;n::Binding binding(bootstrap,errors);errors.binding=&binding;auto cached=n::Resolver{}.application();dependencies.block_app=true;
 std::atomic<BOOL> app_result=FALSE;std::atomic<DWORD> app_error=0;std::thread app([&]{::SetLastError(0x51a0);app_result=cached(reinterpret_cast<HINSTANCE>(17),1,reinterpret_cast<void*>(18));app_error=::GetLastError();});dependencies.app_entered.wait();require(n::is_bound()&&n::active_calls()==1&&dependencies.configurations==1,"Actual std-thread Application provider blocks with one already-admitted lease");
 ::SetLastError(0x51f1);require(!binding.pause_application_admission()&&::GetLastError()==0x51f1,"Pause atomically seals new Application admission yet reports false while old Application lease remains");
 const auto side_effects=dependencies.operations.load();const auto lifecycle=errors.lifecycle_calls.load();std::atomic<bool> rejected=true;std::thread rejected_attach([&]{::SetLastError(0x51f2);if(cached(nullptr,1,nullptr)!=FALSE||::GetLastError()!=0x51f2)rejected=false;});std::thread rejected_other([&]{for(const DWORD reason:{0UL,2UL,3UL,0xffffffffUL}){::SetLastError(0x51f3);if(cached(nullptr,reason,nullptr)!=FALSE||::GetLastError()!=0x51f3)rejected=false;}});rejected_attach.join();rejected_other.join();require(rejected&&dependencies.operations==side_effects&&errors.lifecycle_calls==lifecycle&&!errors.exceptions&&n::active_calls()==1,"Cached sealed entry rejects every reason without Bootstrap/dependencies/Errors lease or LastError mutation");
 try{n::Binding duplicate(bootstrap,errors);require(false,"Failed duplicate binding must reject");}catch(const Fault& value){require(value.value==n::LifecycleFault::AlreadyBound,"Occupied binding reports established lifecycle fault");}require(cached(nullptr,1,nullptr)==FALSE&&dependencies.operations==side_effects,"Failed duplicate binding never unseals current Application route");
 auto* payload=static_cast<r::NoArgumentPayload*>(dependencies.allocate_04b0bc(sizeof(r::NoArgumentPayload)));::new(payload)r::NoArgumentPayload{blocked};std::atomic<unsigned> worker_result=99;std::thread worker([&]{worker_result=n::no_argument_entry(payload);});dependencies.worker_entered.wait();require(n::active_calls()==2&&!binding.pause_application_admission(),"Application and worker leases coexist while only old Application blocks pause success");
 dependencies.app_release.signal();app.join();require(app_result==TRUE&&app_error==0x51a7&&dependencies.registrations==1&&dependencies.installations==1&&dependencies.guards==1,"Already-admitted Application completes original Bootstrap startup despite seal and preserves actual lower LastError");require(n::active_calls()==1&&binding.pause_application_admission(),"Pause returns true with Application drained even while existing NoArgument worker lease remains");
 bool busy=false;try{binding.close();}catch(const Fault& value){busy=value.value==n::LifecycleFault::Busy;}require(busy&&n::is_bound()&&n::active_calls()==1,"Binding close still waits for ALL entry kinds, not merely Application drain");
 dependencies.drain_jobs();require(dependencies.noarg_calls==2&&dependencies.true_calls==1&&dependencies.false_calls==1&&dependencies.cleanups==3,"NoArgument/Boolean routes remain admitted after sticky application seal");n::Resolver{}.exit_callback()();require(dependencies.exit_steps==3&&::GetLastError()==0x51d3,"Exit source route remains admitted after application seal and keeps source-produced LastError");
 dependencies.worker_release.signal();worker.join();require(worker_result==0&&n::active_calls()==0&&dependencies.cleanups==4&&dependencies.deletions==4&&!dependencies.outstanding(),"Old application and all admitted/queued worker payloads finish with balanced native leases and payload ownership");::SetLastError(0x51f4);require(binding.pause_application_admission()&&::GetLastError()==0x51f4,"Repeated sticky pause is idempotent and preserves control LastError");binding.close();require(!n::is_bound()&&::GetLastError()==0x51f4&&binding.pause_application_admission(),"Known closed binding remains inert without reopening current admission");
 Dependencies next;current=&next;r::Bootstrap next_bootstrap(next);Errors next_errors;next_errors.dependencies=&next;n::Binding reopened(next_bootstrap,next_errors);next_errors.binding=&reopened;require(binding.pause_application_admission()&&cached(nullptr,0,nullptr)==TRUE&&!next.operations,"Old closed Binding pause cannot seal or poison a distinct fresh binding");require(cached(nullptr,1,nullptr)==TRUE&&next.registrations==1&&next.configurations==1,"Successful new binding resets only its own Application admission; no stale-pointer ABA guarantee is inferred");next.drain_jobs();require(reopened.pause_application_admission()&&cached(nullptr,1,nullptr)==FALSE,"Fresh binding obtains independent sticky Application seal");reopened.close();require(!n::is_bound()&&!n::active_calls()&&!next.outstanding()&&!next_errors.exceptions,"Reopened binding closes after explicit own worker drain");current=nullptr;
}
void terminal(std::string_view mode){if(mode=="unbound"){std::set_terminate([]{::ExitProcess(!n::is_bound()&&!n::active_calls()?1103:1192);});(void)n::application_entry(nullptr,0,nullptr);::ExitProcess(1191);}auto* dependencies=new Dependencies;current=dependencies;auto* bootstrap=new r::Bootstrap(*dependencies);auto* errors=new Errors;errors->dependencies=dependencies;errors->child=mode;auto* binding=new n::Binding(*bootstrap,*errors);errors->binding=binding;
 if(mode=="application"){dependencies->throw_app=true;(void)n::application_entry(nullptr,1,nullptr);}if(mode=="terminate"){require(binding->pause_application_admission(),"Selected terminal fixture seals Application only");require(n::application_entry(nullptr,1,nullptr)==FALSE,"Application seal is active before unaffected Terminate entry");n::terminate_entry();}if(mode=="closed"){require(binding->pause_application_admission(),"Closed-policy fixture starts from sealed bound state");binding->close();(void)n::application_entry(nullptr,0,nullptr);}::ExitProcess(1190);}
void isolated(std::wstring mode,DWORD expected){std::array<wchar_t,32768> path{};const auto nbytes=::GetModuleFileNameW(nullptr,path.data(),static_cast<DWORD>(path.size()));require(nbytes&&nbytes<path.size(),"Own application-admission fixture path found");std::wstring command=L"\""+std::wstring(path.data(),nbytes)+L"\" --"+mode;STARTUPINFOW startup{};startup.cb=sizeof(startup);PROCESS_INFORMATION process{};require(::CreateProcessW(nullptr,command.data(),nullptr,nullptr,FALSE,CREATE_NO_WINDOW,nullptr,nullptr,&startup,&process)!=0,"Isolated native error/terminal policy fixture launched");DWORD exit=0;require(::WaitForSingleObject(process.hProcess,30000)==WAIT_OBJECT_0&&::GetExitCodeProcess(process.hProcess,&exit)&&exit==expected,"Admission child reaches exact lease-balanced terminal policy assertions");::CloseHandle(process.hThread);::CloseHandle(process.hProcess);}
}
int main(int argc,char** argv){try{if(argc==2)terminal(std::string_view(argv[1]).substr(2));main_admission();isolated(L"application",1101);isolated(L"terminate",1102);isolated(L"unbound",1103);isolated(L"closed",1104);std::cout<<"PASS: "<<checks<<" application-admission core checks; sealed Application with live worker routes and four isolated terminal policies\n";return 0;}catch(const std::exception& ex){std::cerr<<ex.what()<<'\n'<<std::flush;::ExitProcess(1);}catch(...){std::cerr<<"Unknown own application-admission fixture failure\n"<<std::flush;::ExitProcess(2);}}
