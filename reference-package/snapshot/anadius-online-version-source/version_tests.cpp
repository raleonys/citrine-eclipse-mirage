#include "version.hpp"
#include <windows.h>
#include <bit>
#include <condition_variable>
#include <exception>
#include <iostream>
#include <thread>

namespace {
namespace v=anadius::online_version;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
struct Fault {};
struct State {
    v::Binding* binding=nullptr;unsigned calls=0,destroyed=0,reentry_rejected=0,unexpected_lower=0;
    bool observed_active=false,all_retire_active=true,block=false,entered=false,release=false;
    DWORD incoming=0;std::int32_t result=2;int prefix=0;std::exception_ptr failure;
    std::weak_ptr<void> code;std::mutex gate;std::condition_variable changed;
};
State* current=nullptr;
std::int32_t own_version(){
    auto& s=*current;s.incoming=::GetLastError();++s.calls;s.prefix=99;
    const auto view=s.binding->snapshot();s.observed_active=view.active&&view.code_retained&&!s.code.expired();
    {std::unique_lock lock(s.gate);if(s.block){s.entered=true;s.changed.notify_all();s.changed.wait(lock,[&]{return s.release;});}}
    ::SetLastError(0x3333);if(s.failure)std::rethrow_exception(s.failure);return s.result;
}
v::Procedure procedure(){return reinterpret_cast<v::Procedure>(&own_version);}
struct Code {
    State& s;explicit Code(State& state):s(state){}
    ~Code(){
        ++s.destroyed;
        if(s.binding){
            s.all_retire_active=s.all_retire_active&&s.binding->snapshot().active;
            auto denied=[&](auto&& call){try{call();}catch(const std::logic_error&){++s.reentry_rejected;}catch(...){++s.unexpected_lower;}};
            denied([&]{s.binding->load([&]{++s.unexpected_lower;return v::Module{0x99};});});
            denied([&]{s.binding->resolve(0x80,[&]{++s.unexpected_lower;return procedure();});});
            denied([&]{s.binding->call(procedure());});
            denied([&]{s.binding->free(0x80,[&]{++s.unexpected_lower;return true;});});
        }
        ::SetLastError(0xdead);
    }
};
v::CodeLease factory(State& s){return [&s](v::Module module){if(module!=0x80)throw Fault{};auto code=std::make_shared<Code>(s);s.code=code;::SetLastError(0xeeee);return code;};}
void loaded(v::Binding& b){require(b.load([]{return v::Module{0x80};})==0x80,"exact loaded module recorded");}
void bound(v::Binding& b){require(b.resolve(0x80,[]{::SetLastError(0x2222);return procedure();})==procedure(),"resolved actual own typed export");}
void normal_and_retirement(){
    rejects<std::invalid_argument>([]{v::Binding b({});},"explicit lease factory required");
    State s;current=&s;v::Binding b(factory(s));s.binding=&b;
    rejects<std::logic_error>([&]{b.call(procedure());},"unloaded procedure rejected");
    require(b.load([]{return v::Module{0};})==0&&!b.snapshot().poisoned,"normal missing module remains empty");
    loaded(b);unsigned lower=0;
    rejects<std::logic_error>([&]{b.resolve(0x81,[&]{++lower;return procedure();});},"foreign module resolution rejected");
    require(lower==0,"foreign module never reaches lookup");
    bound(b);require(::GetLastError()==0x2222&&!s.code.expired(),"lease bookkeeping preserves lookup LastError and retains code");
    rejects<std::logic_error>([&]{b.call(procedure()+1);},"unregistered procedure rejected");require(s.calls==0,"bad procedure never invokes typed export");
    ::SetLastError(0x1111);require(b.call(procedure())==2&&s.calls==1&&s.incoming==0x1111&&::GetLastError()==0x3333&&s.observed_active,"actual zero-input typed version preserves result/errors and sees active retained owner");
    s.result=std::bit_cast<std::int32_t>(std::uint32_t{0x87654321});require(static_cast<std::uint32_t>(b.call(procedure()))==0x87654321,"complete low32 result preserved without boolean coercion");
    const auto calls=s.calls;
    require(b.resolve(0x80,[]{::SetLastError(0x4444);return v::Procedure{0};})==0,"missing export returns exact zero");
    require(s.code.expired()&&s.destroyed==1&&s.reentry_rejected==4&&s.unexpected_lower==0&&s.all_retire_active&&::GetLastError()==0x4444,"missing lookup retires lease outside gate with reservation held and exact error");
    rejects<std::logic_error>([&]{b.call(procedure());},"stale procedure after missing lookup rejected");require(s.calls==calls,"stale lookup never invokes old export");
    bound(b);bound(b);
    require(s.destroyed==2&&s.reentry_rejected==8&&s.all_retire_active&&s.unexpected_lower==0,"rebind retirement cannot invoke freshly published export");
    bool before_free=false;
    require(!b.free(0x80,[&]{auto view=b.snapshot();before_free=view.active&&view.module==0x80&&view.procedure==0&&view.code_retained;::SetLastError(0x5555);return false;}),"false free return retained");
    require(before_free&&b.snapshot().module==0&&b.snapshot().procedure==0&&!b.snapshot().code_retained&&s.code.expired()&&::GetLastError()==0x5555,"free invalidates before lower and normalFALSE clears logical module/lease without changing error");
    require(s.destroyed==3&&s.reentry_rejected==12&&s.all_retire_active&&s.unexpected_lower==0,"free retirement blocks all four managed reentry routes outside mutex");
    rejects<std::logic_error>([&]{b.call(procedure());},"retired procedure cannot be called after normal free");
    loaded(b);bound(b);require(b.free(0x80,[]{return true;}),"explicit fresh load after known normal free is supported");
    require(s.destroyed==4&&s.reentry_rejected==16&&s.unexpected_lower==0,"fresh epoch retires only its own code lease");
}
void exception_prefixes(){
    State s;current=&s;auto expected=std::make_exception_ptr(Fault{});
    {
        v::Binding b(factory(s));s.binding=&b;loaded(b);bound(b);
        bool same=false;try{b.resolve(0x80,[&]()->v::Procedure{::SetLastError(0x6666);std::rethrow_exception(expected);});}catch(...){same=std::current_exception()==expected;}
        auto view=b.snapshot();require(same&&view.module==0x80&&view.procedure==0&&!view.poisoned&&!view.code_retained&&::GetLastError()==0x6666,"lookup throw keeps module, retires binding, preserves exact exception/error");
        require(s.destroyed==1&&s.reentry_rejected==4&&s.all_retire_active,"exceptional lookup retires lease under active reservation");
        bound(b);s.failure=expected;s.calls=0;::SetLastError(0x1111);same=false;
        try{b.call(procedure());}catch(...){same=std::current_exception()==expected;}
        require(same&&s.calls==1&&s.prefix==99&&s.incoming==0x1111&&::GetLastError()==0x3333&&!b.snapshot().active&&!b.snapshot().poisoned&&b.snapshot().code_retained,"synchronous version throw preserves prefix, exact exception/error and retained binding without retry");
        s.failure={};unsigned frees=0;same=false;
        try{b.free(0x80,[&]()->bool{++frees;::SetLastError(0x7777);std::rethrow_exception(expected);});}catch(...){same=std::current_exception()==expected;}
        view=b.snapshot();require(same&&frees==1&&view.module==0x80&&view.procedure==0&&view.poisoned&&view.code_retained&&::GetLastError()==0x7777,"unknown free throw poisons and retains exact module/code/error prefix");
        rejects<std::logic_error>([&]{b.free(0x80,[&]{++frees;return true;});},"unknown free outcome cannot be blindly retried");require(frees==1&&!s.code.expired(),"poisoned free never repeats lower and retains owner");
    }
    require(s.code.expired()&&s.destroyed==2&&s.all_retire_active&&s.reentry_rejected==8&&s.unexpected_lower==0,"destructor retires code safely without raw module cleanup or reentry");s.binding=nullptr;
    {
        v::Binding b(factory(s));s.binding=&b;unsigned loads=0;bool same=false;
        try{b.load([&]()->v::Module{++loads;::SetLastError(0x8888);std::rethrow_exception(expected);});}catch(...){same=std::current_exception()==expected;}
        require(same&&loads==1&&b.snapshot().poisoned&&b.snapshot().module==0&&::GetLastError()==0x8888,"unknown load throw preserves exception/error and poisons empty state");
        rejects<std::logic_error>([&]{b.load([&]{++loads;return v::Module{0x80};});},"unknown load cannot be blindly retried");require(loads==1,"poisoned load never repeats lower");
    }
    {
        v::Binding b([&](v::Module)->std::shared_ptr<void>{::SetLastError(0xffff);std::rethrow_exception(expected);});s.binding=&b;loaded(b);bool same=false;
        try{b.resolve(0x80,[]{::SetLastError(0x9999);return procedure();});}catch(...){same=std::current_exception()==expected;}
        require(same&&b.snapshot().module==0x80&&b.snapshot().procedure==0&&!b.snapshot().code_retained&&::GetLastError()==0x9999,"lease throw retains loaded prefix and lookup error without callable publication");
        require(b.free(0x80,[]{return true;}),"known module can explicitly clean up after failed lease construction");
    }
    s.binding=nullptr;
}
void concurrency(){
    State s;current=&s;v::Binding b(factory(s));s.binding=&b;loaded(b);bound(b);s.block=true;
    bool returned=false;DWORD error=0;
    std::thread worker([&]{::SetLastError(0xabcd);returned=b.call(procedure())==2;error=::GetLastError();});
    {std::unique_lock lock(s.gate);s.changed.wait(lock,[&]{return s.entered;});}
    unsigned rejected=0,lower=0;
    auto attempt=[&](auto&& fn){try{fn();}catch(const std::logic_error&){++rejected;}};
    attempt([&]{b.load([&]{++lower;return v::Module{0x80};});});
    attempt([&]{b.resolve(0x80,[&]{++lower;return procedure();});});
    attempt([&]{b.free(0x80,[&]{++lower;return true;});});
    attempt([&]{b.call(procedure());});const auto view=b.snapshot();
    {std::lock_guard lock(s.gate);s.release=true;s.changed.notify_all();}worker.join();
    require(rejected==4&&lower==0&&view.active&&view.code_retained,"active call rejects load/rebind/free/secondcall before any lower effect");
    require(returned&&error==0x3333&&s.incoming==0xabcd&&s.calls==1&&s.observed_active&&!b.snapshot().active,"in-flight typed call survives rejected overlap with exact result/error and owner");
    require(b.free(0x80,[]{return true;}),"free succeeds after joined version call");
}
void factory_capture_destruction(){
    State s;current=&s;
    {
        auto capture=std::make_shared<Code>(s);v::CodeLease lease=[capture](v::Module){return std::make_shared<int>(1);};
        v::Binding b(std::move(lease));s.binding=&b;lease={};capture.reset();
        require(s.destroyed==0,"factory capture retained by Binding after external owners released");
    }
    require(s.destroyed==1&&s.reentry_rejected==4&&s.all_retire_active&&s.unexpected_lower==0,"factory capture deleter runs before gate destruction with managed reentry denied");s.binding=nullptr;
}
}
int main(){try{normal_and_retirement();exception_prefixes();concurrency();factory_capture_destruction();std::cout<<"PASS: "<<checks<<" online typed version binding core checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}catch(...){std::cerr<<"Unexpected own fault\n";return 2;}}
