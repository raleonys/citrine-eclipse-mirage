#include "factory.hpp"
#include "../anadius-bootstrap-policies-source/policies_test_fixture.hpp"
#include "../anadius-config-iteration-source/thread_error.hpp"
#include <csignal>
#include <functional>
#include <iostream>
#include <process.h>
#include <sstream>
#include <thread>

namespace {
using namespace anadius;namespace sf=services_factory;namespace sc=services_completion;
unsigned checks=0;void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Buffer:std::stringbuf {
    std::function<void()> on_write,on_retire,on_sync;unsigned flushes=0;bool throw_write=false;
    ~Buffer(){if(on_retire)on_retire();}
    std::streamsize xsputn(const char* p,std::streamsize n)override{if(on_write)on_write();const auto result=std::stringbuf::xsputn(p,n);if(throw_write)throw std::runtime_error("unknown stream prefix");return result;}
    int sync()override{++flushes;if(on_sync)on_sync();return 0;}
};
struct Sink {std::shared_ptr<Buffer> buffer=std::make_shared<Buffer>();std::shared_ptr<std::ostream> stream=std::make_shared<std::ostream>(buffer.get());};
struct Host {
    std::vector<void(*)()> callbacks;std::terminate_handler installed=nullptr;unsigned calls=0;int exit_result=0;
};Host* host=nullptr;std::vector<int> exits;
int at_exit(void(*callback)())noexcept{++host->calls;host->callbacks.push_back(callback);return host->exit_result;}
std::terminate_handler set_terminate(std::terminate_handler callback)noexcept{++host->calls;const auto old=host->installed;host->installed=callback;return old;}
std::terminate_handler get_terminate()noexcept{++host->calls;return host->installed;}
void exit1(){exits.push_back(1);}void exit2(){exits.push_back(2);}void exit3(){exits.push_back(3);}void terminate_callback(){}
struct Fixture {
    b7_test::DomainFixture domain;Host h;bootstrap_runtime::CallbackRegistry registry;
    std::shared_ptr<thread_domain::ApplicationMemory> memory;
    std::shared_ptr<Sink> sink=std::make_shared<Sink>();
    std::shared_ptr<sc::BootstrapLogger> logger;
    std::shared_ptr<sc::ExitWaitRegistry> waits;
    std::unique_ptr<sf::Remaining> remaining;
    Fixture():registry({at_exit,set_terminate,get_terminate}){
        host=&h;memory=domain.domain->make_memory();logger=std::make_shared<sc::BootstrapLogger>(std::shared_ptr<std::ostream>(sink,sink->stream.get()));waits=std::make_shared<sc::ExitWaitRegistry>(sc::InitialRegistry::empty_owned);
        remaining=std::make_unique<sf::Remaining>(registry,sf::Options{sink,memory,logger,waits});
    }
    void finish_registry(){require(registry.pause_admission()&&registry.drain_exit_callbacks()&&registry.finish(),"actual callback registry pauses/drains/finishes");}
    void close(){finish_registry();require(remaining->close_after_cleanup(),"remaining provider closes after actual known leaf drain");}
    ~Fixture(){if(remaining){try{if(remaining->snapshot().phase!=sf::Phase::Closed){finish_registry();if(!remaining->close_after_cleanup())std::terminate();}remaining.reset();}catch(...){std::terminate();}}waits.reset();logger.reset();memory.reset();host=nullptr;}
};
void passive_and_validation(){
    Fixture f;require(!f.h.calls&&f.sink->buffer->str().empty()&&f.memory->outstanding_allocations()==0&&f.remaining->snapshot().phase==sf::Phase::Open&&!f.remaining->snapshot().used,"construction has no registrar/log/heap source work");
    for(unsigned missing=0;missing<3;++missing){auto options=sf::Options{f.sink,f.memory,f.logger,f.waits};if(missing==0)options.memory.reset();if(missing==1)options.logger.reset();if(missing==2)options.waits.reset();rejects<std::invalid_argument>([&]{sf::Remaining invalid(f.registry,std::move(options));},"every required leaf owner validated");}
    require(!f.h.calls&&!f.memory->outstanding_allocations(),"invalid options publish no callback or application byte");
    std::weak_ptr<sc::ExitWaitRegistry> sole;
    {sf::Options invalid;invalid.logger=f.logger;invalid.waits=std::make_shared<sc::ExitWaitRegistry>(sc::InitialRegistry::empty_owned);sole=invalid.waits;rejects<std::invalid_argument>([&]{sf::Remaining fail(f.registry,std::move(invalid));},"invalid sole-owned empty waits are actually closed before dropping ownership");}
    require(sole.expired(),"invalid construction needs no hidden external waits alias to avoid abort");
    auto live_waits=std::make_shared<sc::ExitWaitRegistry>(sc::InitialRegistry::empty_owned);auto live_target=std::make_shared<sc::WaitTarget>(1,0);live_target->lock();live_waits->register_current_thread(live_target);
    rejects<std::invalid_argument>([&]{sf::Remaining fail(f.registry,sf::Options{f.sink,f.memory,{},live_waits});},"invalid constructor leaves known live input to retained caller aliases");
    require(live_waits->active()==1&&live_target->is_locked()&&live_target->registered(),"invalid constructor invents no live wait cleanup");live_waits->cleanup_current_thread();require(live_waits->close(),"caller explicitly drains retained live wait input");
    {auto unused=sf::make_remaining(f.registry,sf::Options{f.sink,f.memory,f.logger,std::make_shared<sc::ExitWaitRegistry>(sc::InitialRegistry::empty_owned)});unused.reset();require(f.registry.pending_exit_callbacks()==0&&!f.registry.published(),"never-used unpublished assembly owner safely retires actual empty leaves");}
    f.close();
}
void routes_and_known_retry(){
    Fixture f;auto& r=*f.remaining;::SetLastError(0x611);require(r.register_exit(exit1)==0&&r.register_exit(exit2)==0&&::GetLastError()==0x611,"two direct registrar routes preserve last error");r.install_terminate(terminate_callback);require(f.h.callbacks.size()==2&&f.h.installed&&f.registry.pending_exit_callbacks()==2,"registry receives exact source registrations and own terminate thunk");
    auto* token=r.log_insert(reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),0x180083cd8);require(token==f.logger->token(),"logger maps source identity to exact owned token");r.log_insert(token,0x180086720);r.log_insert(token,0x180086718);r.log_finish(token);require(f.sink->buffer->str()=="LSX emu version 2.0.1.0\n"&&f.sink->buffer->flushes==1,"actual retained logger bytes and endl route");
    rejects<std::domain_error>([&]{r.log_insert(reinterpret_cast<void*>(0x111),0x180083cd8);},"foreign logger token rejected");rejects<std::domain_error>([&]{r.log_insert(token,0x180000000);},"unknown literal rejected");
    for(int i=0;i<7;++i){const auto spec=bootstrap_policies::thread_error_spec(i);bool matched=false;try{r.thread_error(i);}catch(const std::system_error& e){matched=e.code().value()==spec.code&&e.code().category()==std::generic_category()&&std::string(e.what()).find(spec.context)!=std::string::npos;}require(matched,"actual indexed thread error code/context preserved");}
    rejects<std::out_of_range>([&]{r.thread_error(7);},"thread error rejects out of table index");r.guard_return();
    auto* block=r.allocate(57);require(block&&f.memory->allocation_state(block)==thread_domain::ApplicationMemory::AllocationState::Live,"actual same-Domain heap owns allocated bytes");
    require(!r.close_after_cleanup()&&r.snapshot().phase==sf::Phase::Open,"unfinished registry preflight returns false without stopping retry routes");
    f.finish_registry();require(exits==std::vector<int>({2,1})&&!f.h.installed,"actual registry drains LIFO and restores previous terminate handler");exits.clear();
    require(!r.close_after_cleanup()&&r.snapshot().phase==sf::Phase::Open,"known outstanding allocation prevents close with open deallocate retry");
    f.domain.sdk.deny_free=true;rejects<std::system_error>([&]{r.deallocate(block,999);},"known HeapFree failure propagates");require(f.memory->allocation_state(block)==thread_domain::ApplicationMemory::AllocationState::Live&&r.snapshot().phase==sf::Phase::Open,"known failed free retains exact live ticket without poison");f.domain.sdk.deny_free=false;r.deallocate(block,1);
    auto target=std::make_shared<sc::WaitTarget>(1,0);target->lock();f.waits->register_current_thread(target);require(!r.close_after_cleanup(),"live actual exit-wait target prevents close");r.cleanup_threads();require(target->ready_value_quiescent()==1&&!target->is_locked()&&!target->registered(),"actual source cleanup unlocks/notifies and consumes current target");
    ::SetLastError(0x622);require(r.close_after_cleanup()&&r.close_after_cleanup()&&::GetLastError()==0x622,"closed metadata is idempotent and preserves last error");require(!f.memory->tracking_poisoned()&&!f.memory->outstanding_allocations()&&r.snapshot().phase==sf::Phase::Closed,"successful close seals metadata after actual leaf checks");
    rejects<std::logic_error>([&]{r.allocate(1);},"Closed owner rejects new allocation before SDK");rejects<std::logic_error>([&]{r.guard_return();},"Closed owner rejects even actual RET admission");require(f.domain.domain->provider_leases()==1,"successful close retains ApplicationMemory/Domain lease through Resources retirement");
    const auto size=f.h.callbacks.size();for(auto callback:f.h.callbacks)callback();require(f.h.callbacks.size()==size&&exits.empty(),"permanent exit thunks inert after registry finish");
}
struct Worker {
    sf::Remaining* remaining;std::shared_ptr<sc::ExitWaitRegistry> waits;std::shared_ptr<sc::WaitTarget> target;
    HANDLE entered,release;std::atomic<bool> okay=false;std::atomic<bool> nested=false;
};
unsigned __stdcall own_worker(void* argument){auto& w=*static_cast<Worker*>(argument);try{w.target->lock();w.waits->register_current_thread(w.target);auto* p=w.remaining->allocate(91);auto* token=w.remaining->log_insert(reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),0x180083cd8);w.remaining->log_finish(token);w.remaining->deallocate(p,91);w.remaining->cleanup_threads();w.remaining->register_exit(exit3);w.okay=true;}catch(...){w.okay=false;}return 19;}
void actual_worker_and_active_close(){
    Fixture f;auto threads=f.domain.domain->make_threads(b7_test::thread_api());bootstrap_backend::TrackingThreads tracked(*threads,bootstrap_backend::native_join_api());
    Worker w{f.remaining.get(),f.waits,std::make_shared<sc::WaitTarget>(1,0),::CreateEventW(nullptr,TRUE,FALSE,nullptr),::CreateEventW(nullptr,TRUE,FALSE,nullptr)};require(w.entered&&w.release,"own native synchronization events created");
    f.sink->buffer->on_write=[&]{f.remaining->guard_return();w.nested=true;::SetEvent(w.entered);if(::WaitForSingleObject(w.release,10000)!=WAIT_OBJECT_0)throw std::runtime_error("worker lower release timeout");};
    std::uint32_t id=0;const auto handle=threads_recovered::begin_thread_05e8e0(nullptr,0,reinterpret_cast<std::uintptr_t>(&own_worker),reinterpret_cast<std::uintptr_t>(&w),0,&id,tracked);require(handle&&id&&tracked.close_handle(handle),"actual recovered Domain thread begins and tracker owns detached duplicate");
    require(::WaitForSingleObject(w.entered,10000)==WAIT_OBJECT_0&&w.nested&&f.remaining->snapshot().active_calls==1,"stream callback legitimately reenters Remaining RET without a busy exclusion");
    require(!f.remaining->close_after_cleanup()&&f.remaining->snapshot().phase==sf::Phase::Open,"active real worker source lease denies controller close before leaf checks");
    ::SetEvent(w.release);require(tracked.stop_and_join(10000)&&w.okay,"real worker/FLS joins after allocation/log/cleanup/registrar routes on noncreator worker");
    f.sink->buffer->on_write={};require(w.target->ready_value_quiescent()==1&&!w.target->registered()&&!w.target->is_locked()&&!f.memory->outstanding_allocations(),"actual worker consumes its own wait record and allocation");
    f.finish_registry();require(exits==std::vector<int>{3},"same-Domain worker registrar callback survives until actual controller drain");exits.clear();require(f.remaining->close_after_cleanup(),"joined worker leaves actual known-empty provider closable");
    std::exception_ptr foreign;std::thread probe([&]{try{f.remaining->close_after_cleanup();}catch(...){foreign=std::current_exception();}});probe.join();require(bool(foreign)&&f.remaining->snapshot().phase==sf::Phase::Closed,"foreign controller close rejected without corrupting Closed metadata");
    ::CloseHandle(w.entered);::CloseHandle(w.release);
}
void attached_retirement(){
    Fixture f;f.remaining->guard_return();f.close();auto* facade=f.remaining.get();auto sink=f.sink;auto memory=f.memory;auto waits=f.waits;std::weak_ptr<thread_domain::ApplicationMemory> weak_memory=memory;std::weak_ptr<sc::ExitWaitRegistry> weak_waits=waits;
    auto retired=std::make_shared<unsigned>(0);auto rejected=std::make_shared<unsigned>(0);auto after_sdk=std::make_shared<unsigned>(0);
    sink->buffer->on_retire=[facade,retired,rejected,after_sdk]{++*retired;if(facade->snapshot().phase!=sf::Phase::Retiring)++*after_sdk;try{facade->guard_return();}catch(const std::logic_error&){++*rejected;}try{facade->close_after_cleanup();}catch(const std::logic_error&){++*rejected;}};
    // Logger and optional backing hold this exact stream/buffer through Closed.
    f.sink.reset();f.logger.reset();sink.reset();f.memory.reset();memory.reset();f.waits.reset();waits.reset();require(!weak_memory.expired()&&!weak_waits.expired(),"Closed provider retains all leaf owners independently of caller aliases");
    f.remaining.reset();require(*retired==1&&*rejected==2&&!*after_sdk&&weak_memory.expired()&&weak_waits.expired(),"actual stream/backing deleter probes attached Retiring facade then all leaf owners retire");
}
void failed_backend_callable_retirement(){
    Fixture f;auto backing=std::make_shared<int>(19);std::weak_ptr<int> weak=backing;
    auto narrow=std::make_unique<sf::Remaining>(f.registry,sf::Options{backing,f.memory,f.logger,f.waits});auto* facade=narrow.get();backing.reset();
    struct Result {unsigned retired=0;bool late=false,open=false;};auto result=std::make_shared<Result>();
    struct Probe {
        std::weak_ptr<int> backing;sf::Remaining* facade;std::shared_ptr<Result> result;
        ~Probe(){++result->retired;auto pin=backing.lock();if(!pin){result->late=true;return;}result->open=facade->snapshot().phase==sf::Phase::Open;}
    };
    auto probe=std::make_shared<Probe>();probe->backing=weak;probe->facade=facade;probe->result=result;
    bootstrap_backend::Resources resources{};resources.stop_sources=[probe]{return true;};resources.join_timeout_ms=37;probe.reset();
    rejects<std::invalid_argument>([&]{bootstrap_backend::SourceBackend invalid(bootstrap_backend::RuntimeSeed{},bootstrap_backend::take_resources(resources),std::move(narrow));},"actual new backend constructor rejects missing providers with owned narrow argument");
    require(result->retired==1&&!result->late&&result->open&&weak.expired(),"SBO callable last-owner destructor runs before narrow backing expires during actual constructor failure");
    require(!resources.stop_sources&&!resources.remaining_factory&&!resources.callback_attachment&&!resources.application_worker&&resources.join_timeout_ms==0,"take_resources clears callable donors and scalar host ownership before source transfer");
    f.close();
}
int abort_code=0;bool fatal_output_seen=false;void abort_handler(int){::ExitProcess(static_cast<UINT>(abort_code==1201&&!fatal_output_seen?1198:abort_code));}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,abort_handler);std::set_terminate([]{::ExitProcess(static_cast<UINT>(abort_code));});auto* f=new Fixture;auto& r=*f->remaining;
    if(mode=="fatal-log"){abort_code=1201;f->sink->buffer->on_sync=[&]{fatal_output_seen=f->sink->buffer->str()=="Unhandled exception\n";};r.terminate_log();}
    if(mode=="fatal-return"){abort_code=1202;r.unexpected_terminate_return();}
    if(mode=="unknown-alloc"){f->domain.sdk.throw_after_alloc=true;bool caught=false;try{r.allocate(73);}catch(const std::runtime_error&){caught=true;}bool invalid=false;try{sf::Remaining fail(f->registry,sf::Options{f->sink,f->memory,{},f->waits});}catch(const std::invalid_argument&){invalid=true;}if(caught&&invalid&&r.snapshot().phase==sf::Phase::Poisoned&&f->memory->tracking_poisoned()&&f->memory->outstanding_allocations()==1&&!r.close_after_cleanup())::ExitProcess(1203);}
    if(mode=="unknown-log"){f->sink->buffer->throw_write=true;f->sink->stream->exceptions(std::ios::badbit);bool caught=false;try{r.log_insert(reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),0x180083cd8);}catch(...){caught=true;}if(caught&&r.snapshot().phase==sf::Phase::Poisoned&&!r.snapshot().active_calls&&!r.close_after_cleanup())::ExitProcess(1204);}
    if(mode=="unclosed"){r.guard_return();abort_code=1205;f->remaining.reset();}
    if(mode=="after-closed"){f->close();(void)f->memory->allocate(3);abort_code=1206;f->remaining.reset();}
    if(mode=="swallowed-poison"){f->sink->buffer->on_write=[&]{try{r.register_exit(nullptr);}catch(...){}};bool caught=false;try{r.log_insert(reinterpret_cast<void*>(sc::BootstrapLogger::stream_identity),0x180083cd8);}catch(const std::logic_error&){caught=true;}if(caught&&r.snapshot().phase==sf::Phase::Poisoned&&!r.snapshot().active_calls&&!r.close_after_cleanup())::ExitProcess(1207);}
    ::ExitProcess(1199);
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{passive_and_validation();routes_and_known_retry();actual_worker_and_active_close();attached_retirement();failed_backend_callable_retirement();for(const auto [mode,code]:{std::pair{"fatal-log",1201},std::pair{"fatal-return",1202},std::pair{"unknown-alloc",1203},std::pair{"unknown-log",1204},std::pair{"unclosed",1205},std::pair{"after-closed",1206},std::pair{"swallowed-poison",1207}})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code,"own isolated fatal/unknown prefix exits exactly and retains unprovable owners");std::cout<<"PASS: "<<checks<<" services-factory core checks; eleven concrete routes, real Domain worker/heap/FLS/wait cleanup, registry/logger and attached retirement; seven isolated children\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
