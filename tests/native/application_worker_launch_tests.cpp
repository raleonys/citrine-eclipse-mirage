// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/runtime/application.hpp"
#include "src/runtime/application_internal.hpp"
#include "laureondo/net/asio_service.hpp"
#include <asio.hpp>
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <system_error>
#include <thread>
#include <utility>

namespace worker_launch_fixture {
struct Failure:std::system_error {
    unsigned attempt;
    explicit Failure(unsigned n):std::system_error(
        std::make_error_code(std::errc::resource_unavailable_try_again),"controlled owned-worker launch"),attempt(n){}
};
struct Probe {
    unsigned fail_on=0;
    bool hold_first=false;
    std::atomic<unsigned> attempts{},started{},entered{},finished{},joins{},emergency_joins{};
    std::atomic<bool> first_entered{},joining_first{},release_first{};
} probe;
class Thread;
std::array<Thread*,8> owners{};
class Thread {
    std::thread thread_;
    unsigned attempt_=0;
    void attach() noexcept {
        for(auto& owner:owners)if(!owner){owner=this;return;}
        std::terminate(); // Only two fields and at most one temporary are live.
    }
public:
    Thread()noexcept{attach();}
    template<class Function> explicit Thread(Function&& function) {
        const auto n=++probe.attempts;
        if(n==probe.fail_on)throw Failure(n);
        thread_=std::thread([body=std::forward<Function>(function),n]() mutable {
            struct Completed{~Completed(){++probe.finished;}} completed;
            ++probe.entered;
            if(n==1&&probe.hold_first){
                probe.first_entered.store(true,std::memory_order_release);
                while(!probe.release_first.load(std::memory_order_acquire))std::this_thread::yield();
            }
            body();
        });
        attempt_=n;++probe.started;attach();
    }
    Thread(Thread&& other)noexcept:thread_(std::move(other.thread_)),attempt_(std::exchange(other.attempt_,0)){attach();}
    Thread& operator=(Thread&& other)noexcept {
        thread_=std::move(other.thread_);attempt_=std::exchange(other.attempt_,0);return *this;
    }
    Thread(const Thread&)=delete;
    Thread& operator=(const Thread&)=delete;
    ~Thread(){for(auto& owner:owners)if(owner==this)owner=nullptr;}
    bool joinable()const noexcept{return thread_.joinable();}
    void join(){
        if(attempt_==1&&probe.hold_first)probe.joining_first.store(true,std::memory_order_release);
        thread_.join();++probe.joins;
    }
    static bool any_joinable(){for(auto* owner:owners)if(owner&&owner->joinable())return true;return false;}
    static void emergency_join(){
        // Assertion failures must release held workers before Application dies.
        // Observations are captured before this cleanup; it cannot make a case pass.
        for(auto* owner:owners)if(owner&&owner->joinable()){owner->thread_.join();++probe.emergency_joins;}
    }
};
void reset(unsigned failure,bool hold){
    for(auto* owner:owners)if(owner)throw std::logic_error("previous worker owner still exists");
    probe.fail_on=failure;probe.hold_first=hold;
    probe.attempts=probe.started=probe.entered=probe.finished=probe.joins=probe.emergency_joins=0;
    probe.first_entered=probe.joining_first=probe.release_first=false;
}
}

// The production default remains exactly std::thread. Only this source-included
// fixture raises a controlled launcher exception before delegating to real
// std::thread; it does not inject an OS thread-creation failure.
#define LAUREONDO_TEST_APPLICATION_WORKER_THREAD ::worker_launch_fixture::Thread
#include "src/runtime/application.cpp"
#undef LAUREONDO_TEST_APPLICATION_WORKER_THREAD

namespace {
using namespace laureondo;
namespace fs=std::filesystem;
unsigned checks=0;
void require(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct Directory {
    fs::path path;
    explicit Directory(unsigned scenario):path(fs::temp_directory_path()/(
        "surge-worker-launch-"+std::to_string(GetCurrentProcessId())+"-"+
        std::to_string(GetTickCount64())+"-"+std::to_string(scenario))){
        require(fs::create_directory(path),"create a new owned worker-launch directory");
        try{
            std::ofstream cfg(path/"surge.cfg",std::ios::binary);
            cfg<<R"("Config2" {"Emulator" {"Documents" "fixture-documents"}})";
            cfg.close();require(static_cast<bool>(cfg),"write isolated worker-launch configuration");
        }catch(...){std::error_code error;fs::remove_all(path,error);throw;}
    }
    ~Directory(){std::error_code error;fs::remove_all(path,error);}
    application::Options options()const{return {(path/"host.exe").u16string(),
        (path/"data").u16string(),false,(path/"program-data").u16string()};}
};
struct PortEnvironment {
    std::string old;
    bool present=false;
    PortEnvironment(){
        SetLastError(ERROR_SUCCESS);const auto n=GetEnvironmentVariableA("EALsxPort",nullptr,0);
        if(n){old.resize(n);SetLastError(ERROR_SUCCESS);
            const auto copied=GetEnvironmentVariableA("EALsxPort",old.data(),n);
            require(copied<n&&(copied||GetLastError()==ERROR_SUCCESS),"capture existing listener environment");
            old.resize(copied);present=true;}
        else{
            const auto error=GetLastError();
            require(error==ERROR_SUCCESS||error==ERROR_ENVVAR_NOT_FOUND,"query existing listener environment");
            present=error==ERROR_SUCCESS;
        }
        if(present)require(SetEnvironmentVariableA("EALsxPort",nullptr)!=FALSE,
            "clear prior port so the observer requires this startup's own publication");
    }
    ~PortEnvironment(){SetEnvironmentVariableA("EALsxPort",present?old.c_str():nullptr);}
};
std::uint16_t published_port(){
    char text[32]{};const auto n=GetEnvironmentVariableA("EALsxPort",text,sizeof text);
    require(n&&n<sizeof text,"startup published its bound port before worker launch");
    const auto value=std::stoul(std::string(text,n));
    require(value&&value<=65535,"published port is valid");return static_cast<std::uint16_t>(value);
}
template<class Predicate> bool wait_for(Predicate predicate){
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(5);
    while(!predicate()&&std::chrono::steady_clock::now()<until)Sleep(1);
    return predicate();
}
struct Starting {
    application::Application& app;
    std::atomic<bool> done{};
    std::exception_ptr error;
    std::thread controller;
    explicit Starting(application::Application& owner):app(owner),controller([this]{
        try{app.start();}catch(...){error=std::current_exception();}
        done.store(true,std::memory_order_release);
    }){}
    void join(){if(controller.joinable())controller.join();}
    ~Starting(){
        worker_launch_fixture::probe.release_first.store(true,std::memory_order_release);
        join();worker_launch_fixture::Thread::emergency_join();
    }
};
void listener_retired(std::uint16_t port){
    asio::io_context io;asio::ip::tcp::socket peer(io);asio::error_code error;
    peer.connect({asio::ip::address_v4::loopback(),port},error);
    require(static_cast<bool>(error),"failed startup retired the real bound listener before returning");
}
void failed_launch(unsigned failure){
    using namespace worker_launch_fixture;
    reset(failure,failure==2);PortEnvironment environment;Directory directory(failure);
    application::Application app(directory.options());Starting starting(app);
    bool held=false,observed=false;
    if(failure==2){
        observed=wait_for([&]{return starting.done.load(std::memory_order_acquire)||
            (probe.first_entered.load(std::memory_order_acquire)&&probe.joining_first.load(std::memory_order_acquire));});
        held=probe.first_entered&&probe.joining_first&&!starting.done&&probe.entered==1&&probe.finished==0;
    }else observed=wait_for([&]{return starting.done.load(std::memory_order_acquire);});
    const auto port=published_port();
    probe.release_first.store(true,std::memory_order_release);starting.join();
    require(observed,"selected worker launch failure reached the bounded observer");
    require(failure==1||held,"startup must join the already owned provider before returning its later launch failure");
    bool original=false;
    try{if(starting.error)std::rethrow_exception(starting.error);}
    catch(const Failure& error){original=error.attempt==failure&&error.code()==std::errc::resource_unavailable_try_again;}
    require(original,"Application preserves the selected original launcher exception");
    const unsigned prefix=failure-1;
    require(probe.attempts==failure&&probe.started==prefix&&probe.entered==prefix&&probe.finished==prefix,
        "failure preserves exactly the already launched native worker prefix");
    require(probe.joins==prefix&&!Thread::any_joinable()&&probe.emergency_joins==0,
        "Application joins the actual owned worker without fixture cleanup assistance");
    listener_retired(port); // Before explicit stop calls can repair a missed startup cleanup.
    app.stop();app.stop();
    require(probe.joins==prefix&&!Thread::any_joinable(),"repeated stop cannot join an already retired worker twice");
    std::cout<<"PASS: controlled worker launch "<<failure<<" failure; "<<prefix
        <<" actual native workers drained before return, listener retired, original error preserved\n";
}
void healthy(){
    using namespace worker_launch_fixture;
    reset(0,false);PortEnvironment environment;Directory directory(0);
    application::Application app(directory.options());app.start();
    require(probe.attempts==2&&probe.started==2&&probe.entered==2&&probe.finished==2&&probe.joins==2,
        "healthy Application uses and joins both actual native workers");
    const auto port=app.configuration().server_port;
    {
        asio::io_context io;asio::ip::tcp::socket peer(io);
        peer.connect({asio::ip::address_v4::loopback(),port});
        DWORD timeout=5000;
        require(setsockopt(peer.native_handle(),SOL_SOCKET,SO_RCVTIMEO,
            reinterpret_cast<const char*>(&timeout),sizeof timeout)==0,"bound the healthy greeting receive");
        asio::streambuf greeting;asio::read_until(peer,greeting,'\0');
        const std::string bytes(asio::buffers_begin(greeting.data()),asio::buffers_end(greeting.data()));
        require(bytes.find("<Challenge ")!=std::string::npos,"healthy control admits a real peer and delivers its greeting");
    }
    app.stop();app.stop();listener_retired(port);
    require(!Thread::any_joinable()&&probe.joins==2&&probe.emergency_joins==0,
        "healthy repeated cleanup retains ordinary worker ownership");
    std::cout<<"PASS: healthy real Application startup, greeting and repeated shutdown\n";
}
}
int main()try{
    failed_launch(1);failed_launch(2);healthy();
    std::cout<<"PASS: "<<checks<<" Application worker-launch checks; controlled launcher exceptions, real threads/listener, not OS launch-failure evidence\n";
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}
