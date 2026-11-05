#include "callbacks.hpp"
#include <atomic>
#include <csignal>
#include <functional>
#include <iostream>
#include <process.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace {
namespace r=anadius::bootstrap_runtime;
namespace o=anadius::bootstrap_owner;
namespace n=anadius::bootstrap_native_recovered;
namespace recovered=anadius::recovered;
unsigned checks=0;
void check(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool seen=false;try{f();}catch(const E&){seen=true;}check(seen,text);}
struct Failure {};
struct Host {
    std::vector<n::ExitCallback> registered;
    std::terminate_handler handler=nullptr;
    unsigned registrations=0,sets=0,gets=0;
    int fail=0;
};
Host* current=nullptr;
int at_exit(n::ExitCallback callback) noexcept {
    ++current->registrations;SetLastError(700);
    if(current->fail)return current->fail;
    current->registered.push_back(callback);return 0;
}
std::terminate_handler set_terminate(std::terminate_handler callback) noexcept {
    ++current->sets;SetLastError(701);const auto previous=current->handler;current->handler=callback;return previous;
}
std::terminate_handler get_terminate() noexcept{++current->gets;SetLastError(702);return current->handler;}
r::HostApi api(){return {at_exit,set_terminate,get_terminate};}
std::function<void()> action_a,action_b;
void a(){if(action_a)action_a();}
void b(){if(action_b)action_b();}
void foreign(){if(action_b)action_b();}
void previous_terminate(){}
void foreign_terminate(){}
void source_terminate(){}
void source_terminate_two(){}
void run_crt(Host& host){while(!host.registered.empty()){const auto f=host.registered.back();host.registered.pop_back();f();}}
void close(r::CallbackRegistry& registry){check(registry.pause_admission(),"pause obtains callback quiescence");check(registry.drain_exit_callbacks(),"manual callback drain succeeds");check(registry.finish(),"finished registry detaches safely");}
void initial_and_validation(){
    static_assert(std::is_same_v<decltype(r::HostApi::at_exit),decltype(&std::atexit)>);
    static_assert(std::is_same_v<decltype(r::HostApi::set_terminate),decltype(&std::set_terminate)>);
    static_assert(std::is_same_v<decltype(r::HostApi::get_terminate),decltype(&std::get_terminate)>);
    auto missing=api();missing.at_exit=nullptr;
    rejects<std::invalid_argument>([&]{r::CallbackRegistry invalid(missing);},"missing CRT registrar rejected");
    missing=api();missing.set_terminate=nullptr;
    rejects<std::invalid_argument>([&]{r::CallbackRegistry invalid(missing);},"missing terminate setter rejected");
    missing=api();missing.get_terminate=nullptr;
    rejects<std::invalid_argument>([&]{r::CallbackRegistry invalid(missing);},"missing terminate getter rejected");
    Host host;current=&host;r::CallbackRegistry registry(api());
    check(!registry.published()&&!registry.paused()&&!registry.finished()&&!registry.pending_exit_callbacks()&&!registry.active_callbacks(),"construction publishes no callbacks or registry route");
    rejects<std::invalid_argument>([&]{registry.register_exit(nullptr);},"null exit callback rejected");
    rejects<std::invalid_argument>([&]{registry.install_terminate(nullptr);},"null terminate callback rejected");
    check(!registry.published()&&host.registrations==0&&host.sets==0,"invalid calls have no host side effects");
    rejects<std::logic_error>([&]{registry.drain_exit_callbacks();},"manual drain requires explicit pause");
    check(!registry.finish(),"finish without pause cannot detach");
    close(registry);
    check(registry.finished()&&!registry.published()&&registry.paused(),"neverpublished registry closes without host mutation");
    check(registry.pause_admission()&&registry.drain_exit_callbacks()&&registry.finish(),"all close operations idempotent after finish");
    rejects<std::logic_error>([&]{registry.register_exit(a);},"finished registry rejects registration");
    rejects<std::logic_error>([&]{registry.install_terminate(source_terminate);},"finished registry rejects terminate install");
}
void natural_order(){
    Host host;current=&host;r::CallbackRegistry registry(api());std::vector<int> events;
    action_a=[&]{check(GetLastError()==41,"natural callback sees caller LastError");events.push_back(1);SetLastError(42);};
    action_b=[&]{events.push_back(2);};
    SetLastError(40);check(registry.register_exit(a)==0&&GetLastError()==40,"registration bookkeeping preserves incoming LastError");
    check(host.registrations==1&&registry.published()&&registry.pending_exit_callbacks()==1,"one accepted callback has one real CRT position");
    at_exit(foreign);check(registry.register_exit(b)==0&&host.registered.size()==3,"second callback has a distinct CRT registration across foreign entry");
    SetLastError(41);run_crt(host);
    check(events==std::vector<int>({2,2,1}),"real CRT LIFO interleaves own and foreign positions");
    check(GetLastError()==42&&registry.pending_exit_callbacks()==0&&registry.active_callbacks()==0,"callback output LastError and lease release preserved");
    close(registry);check(events.size()==3,"manual close never repeats naturally consumed callbacks");
    action_a={};action_b={};
}
void failure_and_manual_prefix(){
    Host host;current=&host;r::CallbackRegistry registry(api());host.fail=17;
    check(registry.register_exit(a)==17,"CRT registration failure returned unchanged");
    check(registry.published()&&registry.pending_exit_callbacks()==0&&host.registered.empty(),"failed registration leaves no phantom callback; controlled route remains");
    host.fail=0;std::vector<int> events;
    action_a=[&]{events.push_back(1);SetLastError(112);};
    action_b=[&]{events.push_back(2);SetLastError(111);throw Failure{};};
    check(registry.register_exit(a)==0&&registry.register_exit(b)==0,"accepted callbacks can follow failed registration");
    check(registry.pause_admission()&&!registry.finish(),"pending callback prevents finish after pause");
    rejects<std::logic_error>([&]{registry.register_exit(a);},"paused registry rejects new registration");
    SetLastError(110);rejects<Failure>([&]{registry.drain_exit_callbacks();},"manual callback exception propagates");
    check(GetLastError()==111&&events==std::vector<int>{2}&&registry.pending_exit_callbacks()==1&&registry.active_callbacks()==0,"throwing callback consumed once and retains remaining prefix/error without lease leak");
    check(registry.drain_exit_callbacks()&&GetLastError()==112&&events==std::vector<int>({2,1}),"retry resumes unconsumed callbacks in LIFO order");
    check(registry.finish(),"drained retry finishes");
    run_crt(host);check(events.size()==2,"retained CRT thunks after manual finish do not access retired callbacks");
    action_a={};action_b={};
}
void active_lease_and_reentry(){
    Host host;current=&host;r::CallbackRegistry registry(api());std::atomic<bool> entered=false,release=false;
    action_a=[&]{entered=true;while(!release.load())std::this_thread::yield();};
    registry.register_exit(a);const auto dispatch=host.registered.back();
    std::thread callback_thread([&]{dispatch();});
    while(!entered.load())std::this_thread::yield();
    check(registry.active_callbacks()==1&&registry.pending_exit_callbacks()==0,"natural callback lease acquired before provider invocation");
    check(!registry.pause_admission()&&registry.paused(),"pause atomically excludes new callbacks while admitted callback keeps close pending");
    check(!registry.drain_exit_callbacks()&&!registry.finish(),"busy lease rejects cleanup/finish without deadlock");
    dispatch();check(registry.active_callbacks()==1,"late dispatch after pause is a no-op and cannot admit another callback");
    release=true;callback_thread.join();
    check(registry.active_callbacks()==0&&registry.pause_admission(),"admitted callback completion permits close retry");
    close(registry);run_crt(host);action_a={};

    Host second;current=&second;r::CallbackRegistry nested(api());
    action_a=[&]{check(!nested.pause_admission()&&!nested.drain_exit_callbacks()&&!nested.finish(),"reentrant callback drain/finish reports busy without mutex recursion");};
    nested.register_exit(a);close(nested);action_a={};
}
void competing_and_rebinding(){
    Host host;current=&host;std::vector<int> events;
    action_a=[&]{events.push_back(1);};action_b=[&]{events.push_back(2);};
    n::ExitCallback old_dispatch;
    {
        r::CallbackRegistry first(api()),inactive(api());
        first.register_exit(a);old_dispatch=host.registered.back();
        rejects<std::logic_error>([&]{inactive.register_exit(b);},"second published registry rejected");
        check(!inactive.published()&&first.published(),"inactive contender never replaces another registry");
        close(inactive);check(first.published(),"closing inactive contender cannot detach active registry");
        close(first);
    }
    old_dispatch();check(events==std::vector<int>{1},"saved exit thunk after registry destruction accesses no old owner");
    {
        r::CallbackRegistry second(api());second.register_exit(b);run_crt(host);
        check(events==std::vector<int>({1,2}),"new registry at newer CRT position executes once; older canceled position is empty");
        close(second);
    }
    action_a={};action_b={};
}
void inactive_pause_preserves_admission(){
    Host host;current=&host;unsigned calls=0;
    action_a=[&]{++calls;};
    n::ExitCallback dispatch=nullptr;
    {
        r::CallbackRegistry active(api()),inactive(api());
        active.register_exit(a);dispatch=host.registered.back();
        close(inactive);
        SetLastError(861);dispatch();
        check(calls==1&&GetLastError()==861&&active.pending_exit_callbacks()==0,
            "pausing/finishing unpublished contender leaves current natural callback admitted");
        active.register_exit(a);
        check(active.pause_admission(),"current owner pause seals natural callback admission");
        SetLastError(862);dispatch();
        check(calls==1&&GetLastError()==862&&active.pending_exit_callbacks()==1,
            "paused natural thunk preserves pending manual drain and LastError");
        close(active);check(calls==2,"manual drain remains available after natural admission is sealed");
    }
    SetLastError(863);run_crt(host);dispatch();
    check(calls==2&&GetLastError()==863,"retained thunks after owner destruction preserve LastError and never repeat cleanup");
    action_a={};
}
void terminate_ownership(){
    Host host;host.handler=previous_terminate;current=&host;
    {
        r::CallbackRegistry registry(api());SetLastError(55);registry.install_terminate(source_terminate);
        const auto dispatch=host.handler;
        check(dispatch&&dispatch!=source_terminate&&GetLastError()==55&&registry.published(),"terminate install publishes permanent dispatcher and preserves LastError");
        registry.install_terminate(source_terminate_two);
        check(host.handler==dispatch&&host.sets==2,"repeat install retains same dispatcher without saving self as previous");
        close(registry);check(host.handler==previous_terminate,"finish restores original pre-registry terminate handler");
    }
    {
        r::CallbackRegistry registry(api());registry.install_terminate(source_terminate);host.handler=foreign_terminate;
        close(registry);check(host.handler==foreign_terminate,"finish preserves a sequential foreign handler replacement");
    }
    {
        r::CallbackRegistry registry(api());registry.install_terminate(source_terminate);host.handler=previous_terminate;
        registry.install_terminate(source_terminate_two);close(registry);
        check(host.handler==previous_terminate,"reinstall after foreign replacement records and restores that previous handler");
    }
}
void noop_worker(){}
void boolean_worker(bool){}
struct Services final:o::Services {
    std::vector<int> calls;std::size_t bytes=0;int index=0;
    int register_exit(n::ExitCallback)override{throw std::logic_error("old registrar must be replaced");}
    void install_terminate(std::terminate_handler)override{throw std::logic_error("old terminate registrar must be replaced");}
    void* log_insert(void* p,std::uintptr_t value)override{calls.push_back(1);index=static_cast<int>(value);return p;}
    void log_finish(void*)override{calls.push_back(2);}
    void* allocate(std::size_t nbytes)override{calls.push_back(3);bytes=nbytes;return this;}
    void deallocate(void*,std::size_t nbytes)override{calls.push_back(4);bytes=nbytes;}
    recovered::NoArgumentFunction worker(recovered::Worker value)override{calls.push_back(5);index=static_cast<int>(value);return noop_worker;}
    recovered::BooleanFunction boolean_worker()override{calls.push_back(6);return ::boolean_worker;}
    [[noreturn]]void thread_error(int value)override{calls.push_back(7);index=value;throw Failure{};}
    void between_workers()override{calls.push_back(8);}
    void guard_return()override{calls.push_back(9);}
    void cleanup_threads()override{calls.push_back(10);}
    void cleanup_first()override{calls.push_back(11);}
    void cleanup_second()override{calls.push_back(12);}
    void cleanup_network()override{calls.push_back(13);}
    void terminate_log()override{calls.push_back(14);}
    [[noreturn]]void unexpected_terminate_return()override{calls.push_back(15);throw Failure{};}
};
void service_forwarding(){
    Host host;current=&host;r::CallbackRegistry registry(api());
    rejects<std::invalid_argument>([&]{r::RegisteredServices invalid(registry,{});},"null remaining Services provider rejected");
    auto lower=std::make_unique<Services>();auto* raw=lower.get();r::RegisteredServices services(registry,std::move(lower));
    check(services.register_exit(a)==0,"concrete service registrar uses CallbackRegistry instead of lower registrar");
    services.install_terminate(source_terminate);check(raw->calls.empty()&&registry.pending_exit_callbacks()==1,"terminate policy replaced without lower side effects");
    check(services.log_insert(raw,77)==raw&&raw->index==77,"log pointer/operand forwarded");services.log_finish(raw);
    check(services.allocate(81)==raw&&raw->bytes==81,"allocation result and byte count forwarded");services.deallocate(raw,82);
    check(raw->bytes==82&&services.worker(recovered::Worker::first_029ea0)==noop_worker,"deallocation and worker pointer forwarded");
    check(services.boolean_worker()==boolean_worker,"boolean worker pointer forwarded");
    rejects<Failure>([&]{services.thread_error(83);},"terminal thread error forwarded");check(raw->index==83,"thread error index retained");
    services.between_workers();services.guard_return();services.cleanup_threads();services.cleanup_first();services.cleanup_second();services.cleanup_network();services.terminate_log();
    rejects<Failure>([&]{services.unexpected_terminate_return();},"unexpected-return terminal operation forwarded");
    check(raw->calls==std::vector<int>({1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}),"all fifteen non-registrar methods forward exactly once in caller order");
    close(registry);
}
int terminal(const std::string& mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);
    std::signal(SIGABRT,[](int){std::_Exit(88);});std::set_terminate([]{std::_Exit(87);});
    Host host;current=&host;
    if(mode=="--live-dtor"){r::CallbackRegistry registry(api());registry.register_exit(a);return 90;}
    r::CallbackRegistry registry(api());
    if(mode=="--natural-throw"){
        action_a=[]{throw Failure{};};registry.register_exit(a);host.registered.back()();std::_Exit(91);
    }
    registry.install_terminate(mode=="--term-calls"?+[]{std::_Exit(86);}:source_terminate);
    const auto dispatch=host.handler;
    if(mode=="--term-after-finish")close(registry);
    if(mode=="--term-paused")registry.pause_admission();
    dispatch();std::_Exit(92);
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal(argv[1]);
    try{
        initial_and_validation();natural_order();failure_and_manual_prefix();active_lease_and_reentry();competing_and_rebinding();inactive_pause_preserves_admission();terminate_ownership();service_forwarding();
        for(const auto* mode:{"--live-dtor","--term-after-finish","--term-paused","--term-return"})
            check(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==88,"own terminal child enforces inactive/return/destructor abort contract");
        check(_spawnl(_P_WAIT,argv[0],argv[0],"--term-calls",static_cast<const char*>(nullptr))==86,"active terminate dispatcher invokes selected callback");
        check(_spawnl(_P_WAIT,argv[0],argv[0],"--natural-throw",static_cast<const char*>(nullptr))==87,"natural exit dispatcher contains callback exceptions at CRT boundary");
        std::cout<<"PASS: "<<checks<<" bootstrap-runtime callback checks; permanent CRT thunks,LIFO foreign interleave,cancellable leases,manual drain retry,terminate ownership and all Services forwarding; originals never executed\n";
    }catch(const std::exception& e){std::cerr<<"FAIL after "<<checks<<" checks: "<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"FAIL after "<<checks<<" checks\n";return 1;}
}
