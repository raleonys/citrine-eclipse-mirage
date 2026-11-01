#include "object.hpp"
#include "../anadius-online-version-source/version.hpp"
#include <windows.h>
#include <array>
#include <csignal>
#include <cstring>
#include <iostream>
#include <process.h>
#include <thread>
#include <condition_variable>
#include <exception>

namespace {
namespace v=anadius::online_version;namespace o=anadius::online_object;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& fn,const char* text){bool caught=false;try{fn();}catch(const E&){caught=true;}require(caught,text);}
struct Fault {};
struct State {
    v::Binding* binding=nullptr;std::array<std::byte,33> bytes{};std::array<std::byte,17> table{};
    const char16_t* cookie=nullptr;unsigned created=0,destroyed=0,retired=0,rejected=0,unexpected=0,queries=0;
    bool null_result=false,throw_factory=false,throw_destroy=false,retire_active=true,claim=false,args=false;
    bool block=false,entered=false,release=false;DWORD incoming=0;
    std::mutex gate;std::condition_variable changed;std::exception_ptr failure=std::make_exception_ptr(Fault{});
    std::vector<std::weak_ptr<void>> pins;
    v::Object object()const{return reinterpret_cast<v::Object>(bytes.data()+1);}
};
State* current=nullptr;
std::int32_t own_version(){return 2;}
void own_destroy(void* object){
    auto& s=*current;++s.destroyed;s.args=object==reinterpret_cast<void*>(s.object());s.incoming=::GetLastError();
    const auto view=s.binding->snapshot();s.claim=view.active&&view.object_unknown&&view.object==s.object()&&view.factory_code_retained;
    {std::unique_lock lock(s.gate);if(s.block){s.entered=true;s.changed.notify_all();s.changed.wait(lock,[&]{return s.release;});}}
    ::SetLastError(0x4444);if(s.throw_destroy)std::rethrow_exception(s.failure);
    // Simulate immediate object destruction. No post-call vptr reread is valid.
    const std::uintptr_t zero=0;std::memcpy(s.bytes.data()+1,&zero,8);
}
v::Object own_factory(const char16_t* cookie){
    auto& s=*current;++s.created;s.incoming=::GetLastError();s.args=cookie==s.cookie;
    const auto view=s.binding->snapshot();s.claim=view.active&&view.object_unknown&&view.object==0&&view.factory_code_retained;
    const auto table=reinterpret_cast<std::uintptr_t>(s.table.data()+1),destroy=reinterpret_cast<std::uintptr_t>(&own_destroy);
    std::memcpy(s.bytes.data()+1,&table,8);std::memcpy(s.table.data()+1,&destroy,8);
    ::SetLastError(0x3333);if(s.throw_factory)std::rethrow_exception(s.failure);return s.null_result?0:s.object();
}
v::Procedure factory_address(){return reinterpret_cast<v::Procedure>(&own_factory);}
struct Code {
    State& s;explicit Code(State& state):s(state){}
    ~Code(){
        ++s.retired;if(s.binding){
            s.retire_active=s.retire_active&&s.binding->snapshot().active;
            auto attempt=[&](auto&& fn){try{fn();}catch(const std::logic_error&){++s.rejected;}catch(...){++s.unexpected;}};
            attempt([&]{s.binding->load([&]{++s.unexpected;return v::Module{0x80};});});
            attempt([&]{s.binding->resolve(0x80,[&]{++s.unexpected;return factory_address();});});
            attempt([&]{s.binding->call(reinterpret_cast<v::Procedure>(&own_version));});
            attempt([&]{s.binding->free(0x80,[&]{++s.unexpected;return true;});});
            attempt([&]{s.binding->resolve_factory(0x80,[&]{++s.unexpected;return factory_address();});});
            attempt([&]{s.binding->create(factory_address(),u"x");});
            attempt([&]{s.binding->destroy(s.object());});
            attempt([&]{s.binding->object_call(s.object(),[&]{++s.unexpected;return 0;});});
        }::SetLastError(0xdead);
    }
};
struct Fixture {
    State state;v::Binding binding;
    Fixture():binding([this](v::Module module){if(module!=0x80)throw Fault{};auto code=std::make_shared<Code>(state);state.pins.push_back(code);::SetLastError(0xeeee);return code;}){
        current=&state;state.binding=&binding;binding.load([]{return v::Module{0x80};});
        binding.resolve(0x80,[]{return reinterpret_cast<v::Procedure>(&own_version);});
    }
    void resolve(){require(binding.resolve_factory(0x80,[]{::SetLastError(0x1111);return factory_address();})==factory_address(),"exact own factory export bound");}
    v::Object create(){static const char16_t cookie[]{u'A',0,u'B',0};state.cookie=cookie;::SetLastError(0x2222);return binding.create(factory_address(),cookie);}
};
void normal_lifetime(){
    Fixture f;auto& s=f.state;unsigned lower=0;
    rejects<std::logic_error>([&]{f.binding.resolve_factory(0x81,[&]{++lower;return factory_address();});},"different factory module rejected");require(lower==0,"foreign module never reaches resolver");
    f.resolve();require(::GetLastError()==0x1111&&f.binding.snapshot().factory_code_retained,"factory lease bookkeeping preserves lookup error");
    rejects<std::logic_error>([&]{f.binding.create(factory_address()+1,u"x");},"wrong factory procedure rejected");
    rejects<anadius::online_recovered::OutOfEvidenceInput>([&]{f.binding.create(factory_address(),nullptr);},"null cookie rejected before SDK");require(s.created==0&&!f.binding.snapshot().object_unknown,"known invalid inputs create no object/unknown claim");
    const auto object=f.create();require(object==s.object()&&s.created==1&&s.args&&s.claim&&s.incoming==0x2222&&::GetLastError()==0x3333,"typed factory passes exact borrowed UTF16 pointer and commits exact object with error preserved");
    auto view=f.binding.snapshot();require(view.object==object&&!view.object_unknown&&view.factory_code_retained&&!view.active,"nonnull object adopted before reservation ends");
    rejects<std::logic_error>([&]{f.binding.free(0x80,[&]{++lower;return true;});},"live object blocks free");
    rejects<std::logic_error>([&]{f.binding.resolve(0x80,[&]{++lower;return factory_address();});},"live object blocks version rebind");
    rejects<std::logic_error>([&]{f.binding.resolve_factory(0x80,[&]{++lower;return factory_address();});},"live object blocks factory rebind");
    rejects<std::logic_error>([&]{f.binding.create(factory_address(),u"x");},"second object creation rejected");
    require(lower==0&&s.created==1&&f.binding.snapshot().factory==factory_address(),"live-object rejection preserves binding before any lower effects");
    rejects<std::logic_error>([&]{f.binding.destroy(0xdeadbeef);},"different unreadable object rejected before any vptr read");
    rejects<std::logic_error>([&]{f.binding.object_call(0xdeadbeef,[&]{++lower;return 0;});},"different query object never reaches lower");
    ::SetLastError(0x5555);require(f.binding.object_call(object,[&]{++s.queries;const auto q=f.binding.snapshot();require(q.active&&q.object==object,"query holds same module reservation");::SetLastError(0x6666);return 9;})==9&&::GetLastError()==0x6666,"mandatory lower query keeps scalar result and error");
    bool same=false;try{f.binding.object_call(object,[&]()->int{::SetLastError(0x7777);std::rethrow_exception(s.failure);});}catch(...){same=std::current_exception()==s.failure;}
    require(same&&::GetLastError()==0x7777&&!f.binding.snapshot().poisoned&&f.binding.snapshot().object==object,"query exception preserves identity/error and known object for explicit cleanup");
    ::SetLastError(0x8888);f.binding.destroy(object);
    require(s.destroyed==1&&s.args&&s.claim&&s.incoming==0x8888&&::GetLastError()==0x4444,"typed destroy passes only exact object and preserves call error");
    view=f.binding.snapshot();require(view.object==0&&!view.object_unknown&&!view.active&&view.factory_code_retained,"normal destroy clears adopted object without rereading erased vptr");
    rejects<std::logic_error>([&]{f.binding.destroy(object);},"destroyed object cannot be read/destroyed again");
    require(s.destroyed==1&&s.retired==0,"code owners retained through entire object lifetime until explicit module release");
    bool invalidated=false;require(!f.binding.free(0x80,[&]{const auto q=f.binding.snapshot();invalidated=q.active&&q.factory==0&&q.procedure==0&&q.factory_code_retained;::SetLastError(0x9999);return false;}),"normal false module free preserved");
    require(invalidated&&f.binding.snapshot().module==0&&s.retired==2&&s.rejected==16&&s.retire_active&&s.unexpected==0&&::GetLastError()==0x9999,"both code leases retire outside gate while all8managed reentry paths stay blocked");
}
void null_and_missing_factory(){
    Fixture f;auto& s=f.state;
    require(f.binding.resolve_factory(0x80,[]{return v::Procedure{0};})==0&&!f.binding.snapshot().factory_code_retained,"missing factory leaves loaded module without invented callable");
    f.resolve();s.null_result=true;require(f.create()==0&&s.created==1&&!f.binding.snapshot().object_unknown&&!f.binding.snapshot().object,"normal null factory result is known empty and preserved exactly");
    require(f.binding.free(0x80,[]{return true;}),"known null object permits explicit module cleanup without destroy");require(s.destroyed==0&&s.retired==2,"null factory never invents destroy call");
}
void concurrent_destroy(){
    Fixture f;f.resolve();const auto object=f.create();auto& s=f.state;s.block=true;DWORD outgoing=0;
    std::thread worker([&]{f.binding.destroy(object);outgoing=::GetLastError();});
    {std::unique_lock lock(s.gate);s.changed.wait(lock,[&]{return s.entered;});}
    unsigned denied=0,lower=0;auto attempt=[&](auto&& fn){try{fn();}catch(const std::logic_error&){++denied;}};
    attempt([&]{f.binding.free(0x80,[&]{++lower;return true;});});
    attempt([&]{f.binding.resolve_factory(0x80,[&]{++lower;return factory_address();});});
    attempt([&]{f.binding.call(reinterpret_cast<v::Procedure>(&own_version));});
    attempt([&]{f.binding.object_call(object,[&]{++lower;return 0;});});
    const auto before=f.binding.snapshot();{std::lock_guard lock(s.gate);s.release=true;s.changed.notify_all();}worker.join();
    require(denied==4&&lower==0&&before.active&&before.object_unknown&&before.factory_code_retained,"same reservation blocks free/rebind/version/query while destroy owns borrowed object");
    require(outgoing==0x4444&&s.destroyed==1&&f.binding.snapshot().object==0,"in-flight destroy completes once after rejected overlapping effects");
    require(f.binding.free(0x80,[]{return true;}),"module can free after destroy thread joined");
}
[[noreturn]]void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
bool armed=false;unsigned abort_code=0;
void abort_witness(int){
    auto* s=current;
    if(armed&&s&&s->binding){const auto q=s->binding->snapshot();bool pins=true;for(auto& p:s->pins)pins=pins&&!p.expired();
        if(q.module==0x80&&q.object==s->object()&&q.factory_code_retained&&pins&&s->retired==0&&s->created==1&&s->destroyed==0)terminate_own(abort_code);}
    terminate_own(239);
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,abort_witness);
    if(mode=="--live-dtor"){
        auto f=std::make_unique<Fixture>();f->resolve();f->create();armed=true;abort_code=181;f.reset();return 230;
    }
    Fixture f;f.resolve();auto& s=f.state;
    if(mode=="--factory-throw"){
        s.throw_factory=true;bool same=false;try{f.create();}catch(...){same=std::current_exception()==s.failure;}
        const auto q=f.binding.snapshot();unsigned lower=0;bool blocked=false;try{f.binding.free(0x80,[&]{++lower;return true;});}catch(const std::logic_error&){blocked=true;}
        if(same&&q.poisoned&&q.object_unknown&&q.object==0&&q.module==0x80&&q.factory_code_retained&&s.created==1&&s.retired==0&&blocked&&lower==0&&::GetLastError()==0x3333)terminate_own(182);return 237;
    }
    const auto object=f.create();if(mode=="--destroy-throw")s.throw_destroy=true;
    else if(mode=="--null-vptr"){const std::uintptr_t zero=0;std::memcpy(s.bytes.data()+1,&zero,8);}
    else if(mode=="--null-slot"){const std::uintptr_t zero=0;std::memcpy(s.table.data()+1,&zero,8);}
    else return 238;
    const bool invoked=s.throw_destroy;bool expected=false;::SetLastError(0xabcd);
    try{f.binding.destroy(object);}catch(...){if(invoked)expected=std::current_exception()==s.failure;else try{std::rethrow_exception(std::current_exception());}catch(const anadius::online_recovered::OutOfEvidenceInput&){expected=true;}catch(...){}}
    const auto q=f.binding.snapshot();const auto outgoing=::GetLastError();unsigned lower=0,denied=0;
    auto reject=[&](auto&& fn){try{fn();}catch(const std::logic_error&){++denied;}};
    reject([&]{f.binding.destroy(object);});reject([&]{f.binding.free(0x80,[&]{++lower;return true;});});reject([&]{f.binding.resolve_factory(0x80,[&]{++lower;return factory_address();});});
    if(expected&&q.poisoned&&q.object_unknown&&q.object==object&&q.module==0x80&&q.factory_code_retained&&s.created==1&&s.destroyed==(invoked?1U:0U)&&s.retired==0&&denied==3&&lower==0&&outgoing==(invoked?0x4444U:0xabcdU))terminate_own(invoked?183:mode=="--null-vptr"?184:185);
    return 237;
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{
    normal_lifetime();null_and_missing_factory();concurrent_destroy();unsigned code=181;
    for(const auto* mode:{"--live-dtor","--factory-throw","--destroy-throw","--null-vptr","--null-slot"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child reaches exact object/code/failure predicate, generic abort cannot pass");
    std::cout<<"PASS: "<<checks<<" online factory/object lifetime core checks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
