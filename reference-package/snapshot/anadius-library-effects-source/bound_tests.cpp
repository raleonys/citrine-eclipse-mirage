#include "bound.hpp"
#include "effects_test_fixture.hpp"
#include <atomic>
#include <thread>
#include <iostream>

namespace {
using namespace anadius;namespace le=library_effects;namespace e=execution_recovered;
unsigned checks=0;
void require(bool v,const char* s){++checks;if(!v)throw std::runtime_error(s);}
template<class E=std::exception,class F>void rejects(F&& f,const char* s){bool seen=false;try{f();}catch(const E&){seen=true;}require(seen,s);}
struct Platform final:e::Platform {
    std::atomic<unsigned> sleeps=0;HANDLE sleeping=::CreateEventW(nullptr,TRUE,FALSE,nullptr);
    ~Platform()override{if(sleeping)::CloseHandle(sleeping);}
    std::uint32_t current_thread_id()override{return ::GetCurrentThreadId();}
    void acquire_srw_exclusive(std::uintptr_t& p)override{::AcquireSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&p));}
    void release_srw_exclusive(std::uintptr_t& p)override{::ReleaseSRWLockExclusive(reinterpret_cast<PSRWLOCK>(&p));::SetLastError(0x9696);}
    std::int64_t query_performance_frequency()override{LARGE_INTEGER n{};if(!::QueryPerformanceFrequency(&n))throw std::runtime_error("QPF");return n.QuadPart;}
    std::int64_t query_performance_counter()override{LARGE_INTEGER n{};if(!::QueryPerformanceCounter(&n))throw std::runtime_error("QPC");return n.QuadPart;}
    void sleep_milliseconds(std::uint32_t n)override{++sleeps;::SetEvent(sleeping);::Sleep(n);}
    [[noreturn]]void raise_thread_error_04a648(int)override{throw std::runtime_error("Selected thread error");}
};
struct Fixture {
    b12_test::Ui ui;std::shared_ptr<b11_test::OwnedStream> output=std::make_shared<b11_test::OwnedStream>();
    std::shared_ptr<le::Dialog> dialog=std::make_shared<le::Dialog>(b12_test::dialog_api());
    Platform platform;e::MutexState mutex{};std::uint8_t server=1,online=1,libraries=0;std::int64_t frequency=0;
    e::Execution execution{mutex,{server,online,libraries},frequency,platform};
    Fixture(){b12_test::ui=&ui;e::initialize_mutex_04a698(mutex,2);}
    ~Fixture(){b12_test::ui=nullptr;}
    le::BoundOptions options(){le::BoundOptions o;o.stream=output;o.dialog=dialog;return o;}
};
void passive_and_logs(){
    Fixture f;auto options=f.options();options.dialog.reset();rejects<std::invalid_argument>([&]{le::BoundLibrary wrong(std::move(options));},"Missing owned dialog rejects before publication");
    le::BoundLibrary library(f.options());require(f.output->str().empty()&&f.ui.messages.empty()&&!f.libraries,"Concrete library construction performs no stream/UI/phase operation");
    library.begin_load_log_0863e0();library.append_log_path_and_flush(std::string_view("a\0b",3));require(f.output->str()==std::string("Loading a\0b\n",12)&&f.output->flushes==1,"Concrete source log keeps counted path and exactly one newline/flush");
    library.message_038bc0("own-title","own-body",0x40);require(f.ui.messages.size()==1&&f.ui.messages[0].title==L"own-title"&&f.ui.messages[0].text==L"own-body"&&f.ui.messages[0].flags==0x40,"Actual shared Dialog sees owned strings and exact flags without native UI");
    le::BoundLibrary unbound(f.options());rejects<std::logic_error>([&]{unbound.finish_flag_036820();},"Unbound direct phase refuses instead of pretending success");require(!f.libraries,"Unbound direct phase writes no ready byte");
}
void bound_wait_and_finish(){
    Fixture f;le::BoundLibrary library(f.options());bool foreign_denied=false;std::thread foreign([&]{try{library.bind_library_phase(f.execution,f.libraries);}catch(const std::logic_error&){foreign_denied=true;}});foreign.join();require(foreign_denied&&!f.libraries,"Foreign worker cannot bind phase referents");
    library.bind_library_phase(f.execution,f.libraries);rejects<std::logic_error>([&]{library.bind_library_phase(f.execution,f.server);},"Phase root identity binds once");
    std::exception_ptr failure;std::thread waiting([&]{try{library.wait_steps_036880(-1);}catch(...){failure=std::current_exception();}});
    if(::WaitForSingleObject(f.platform.sleeping,5000)!=WAIT_OBJECT_0)std::terminate();
    ::SetLastError(0x1212);library.finish_flag_036820();const auto outgoing=::GetLastError();waiting.join();if(failure)std::rethrow_exception(failure);
    require(f.platform.sleeps>0&&f.libraries==1&&f.server==1&&f.online==1,"Actual Execution wait and concurrent finish use same third-ready byte without holding provider gate across sleep");
    require(outgoing==0x9696,"Provider metadata postchecks preserve last actual SDK error");
    require(f.mutex.owner_48==0xffffffffU&&!f.mutex.count_4c&&!f.mutex.srw_10,"Recovered phase operations leave exact shared mutex idle");
}
struct Retirement {le::BoundLibrary* owner=nullptr;bool backing_alive=true,valid=true;unsigned leaf=0,backing=0;};
struct Backing {Retirement& r;~Backing(){r.backing_alive=false;++r.backing;}};
struct Errors final:encoding_recovered::Errors {
    Retirement& r;explicit Errors(Retirement& r):r(r){}
    ~Errors()override{++r.leaf;r.valid&=r.backing_alive;if(r.owner){try{r.owner->begin_load_log_0863e0();r.valid=false;}catch(const std::logic_error&){}}}
    [[noreturn]]void runtime_00cd70(int)override{throw std::runtime_error("owned runtime error");}
    [[noreturn]]void filesystem_00cdb0(std::uint32_t)override{throw std::runtime_error("owned filesystem error");}
};
void errors_and_retirement(){
    Fixture f;Retirement r;auto options=f.options();options.lifetime=std::shared_ptr<void>(new Backing{r});options.errors=std::make_shared<Errors>(r);
    auto library=std::make_unique<le::BoundLibrary>(std::move(options));r.owner=library.get();rejects([&]{library->throw_runtime_00cd70(5);},"Concrete error route propagates only injected source policy");
    const auto before=f.output->str();rejects<std::logic_error>([&]{library->begin_load_log_0863e0();},"Unknown error prefix closes later admission");library.reset();
    require(r.valid&&r.leaf==1&&r.backing==1&&!r.backing_alive&&f.output->str()==before,"Opaque error deleter sees attached Retiring facade and live backing, with no resumed stream operation");
}
}
int main(){try{passive_and_logs();bound_wait_and_finish();errors_and_retirement();std::cout<<"PASS: "<<checks<<" bound-library checks; actual shared Execution wait/finish, owned source leaves and retirement\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
