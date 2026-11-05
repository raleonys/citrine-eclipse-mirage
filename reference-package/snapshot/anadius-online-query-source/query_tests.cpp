#include "query.hpp"
#include "../anadius-online-version-source/version.hpp"
#include <windows.h>
#include <array>
#include <bit>
#include <cstring>
#include <iostream>
#include <process.h>
#include <thread>
#include <condition_variable>
#include <exception>
#include <chrono>

namespace {
namespace q=anadius::online_query;namespace v=anadius::online_version;
unsigned checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& fn,const char* text){bool yes=false;try{fn();}catch(const E&){yes=true;}require(yes,text);}
struct Fault{};
struct State {
    v::Binding* binding=nullptr;std::array<std::byte,17> bytes{};std::array<std::byte,49> table{};std::byte* page=nullptr;
    std::array<unsigned,4> calls{};unsigned destroyed=0,retired=0;bool claim=true,args=true;
    std::string username=std::string(4096,'U'),persona=std::string("pid\0tail",8),uid="";
    std::int32_t status=std::bit_cast<std::int32_t>(0xf1ff0100u);int throwing=-1,nullptr_slot=-1;
    std::exception_ptr failure=std::make_exception_ptr(Fault{});DWORD incoming=0;
    std::mutex gate;std::condition_variable changed;bool block=false,entered=false,release=false;
    std::byte* object_data(){return page?page+1:bytes.data()+1;}
    std::byte* table_data(){return page?page+129:table.data()+1;}
    v::Object object(){return reinterpret_cast<v::Object>(object_data());}
};
State* current=nullptr;
void observe(void* object,unsigned index){
    auto& s=*current;const auto incoming=::GetLastError();
    {std::unique_lock lock(s.gate);++s.calls[index];s.incoming=incoming;
     const auto view=s.binding->snapshot();s.args=s.args&&object==reinterpret_cast<void*>(s.object());
     s.claim=s.claim&&view.active&&view.object==s.object()&&!view.object_unknown&&!view.poisoned&&view.factory_code_retained;
     if(s.block&&index==1){s.entered=true;s.changed.notify_all();if(!s.changed.wait_for(lock,std::chrono::seconds(10),[&]{return s.release;}))throw std::runtime_error("query release timeout");}}
    ::SetLastError(0x8808+index);if(s.throwing==static_cast<int>(index))std::rethrow_exception(s.failure);
}
std::int32_t status(void* o){observe(o,0);return current->status;}
const char* username(void* o){observe(o,1);return current->nullptr_slot==1?nullptr:current->username.c_str();}
const char* persona(void* o){observe(o,2);return current->nullptr_slot==2?nullptr:current->persona.c_str();}
const char* user_id(void* o){observe(o,3);return current->nullptr_slot==3?nullptr:current->uid.c_str();}
void destroy(void*){++current->destroyed;const std::uintptr_t zero=0;std::memcpy(current->object_data(),&zero,8);}
v::Object factory(const char16_t*){
    auto& s=*current;const auto t=reinterpret_cast<std::uintptr_t>(s.table_data());std::memcpy(s.object_data(),&t,8);
    const std::array<std::uintptr_t,5> slots{reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&status),reinterpret_cast<std::uintptr_t>(&username),reinterpret_cast<std::uintptr_t>(&persona),reinterpret_cast<std::uintptr_t>(&user_id)};
    std::memcpy(s.table_data(),slots.data(),40);return s.object();
}
struct Pin {State& state;~Pin(){++state.retired;}};
struct Fixture {
    State state;v::Binding binding;
    explicit Fixture(bool paged=false,v::QueryPolicy policy=v::QueryPolicy::Serialized):binding([this](v::Module){return std::shared_ptr<void>(new Pin{state});},policy){
        current=&state;state.binding=&binding;
        if(paged){state.page=static_cast<std::byte*>(::VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE));if(!state.page)throw std::bad_alloc();}
        binding.load([]{return v::Module{0x80};});
        binding.resolve_factory(0x80,[]{return reinterpret_cast<v::Procedure>(&factory);});
        binding.create(reinterpret_cast<v::Procedure>(&factory),u"synthetic");
    }
    ~Fixture(){const auto s=binding.snapshot();if(s.object&&!s.poisoned&&!s.active)binding.destroy(s.object);if(binding.snapshot().module&&!binding.snapshot().poisoned)binding.free(0x80,[]{return true;});if(state.page&&!binding.snapshot().poisoned)::VirtualFree(state.page,0,MEM_RELEASE);}
};
void exact_queries(){
    Fixture f;auto& s=f.state;const auto object=s.object();
    require(q::query_slot(object,q::Slot::Status08)==reinterpret_cast<v::Procedure>(&status),"unaligned object/table decode exact status slot through byte copies");
    ::SetLastError(0xaaaa);require(f.binding.status(object)==s.status&&s.incoming==0xaaaa&&::GetLastError()==0x8808,"status preserves signed low32, exact this and incoming/outgoing error");
    require(f.binding.getter(object,q::Slot::Username10)==s.username.c_str(),"raw username returns exact borrowed pointer");
    require(f.binding.copy_getter(object,q::Slot::Username10)==s.username,"long username copies all bytes under selected binding");
    require(f.binding.copy_getter(object,q::Slot::Persona18)=="pid","persona stops at first NUL rather than counted provider buffer");
    require(f.binding.copy_getter(object,q::Slot::UserId20).empty(),"empty nonnull user ID remains valid empty string");
    require(s.claim&&s.args&&s.calls==std::array<unsigned,4>{1,2,1,1},"all four typed shapes observe same exact adopted object and common active reservation");
    require(!f.binding.snapshot().active&&!f.binding.snapshot().object_unknown&&f.binding.snapshot().object==object,"query normal returns preserve known live object and release reservation");
    rejects<std::logic_error>([&]{f.binding.status(0xdeadbeef);},"arbitrary unreadable status token rejected before object read");
    rejects<std::logic_error>([&]{f.binding.getter(0xdeadbeef,q::Slot::Username10);},"arbitrary unreadable getter token rejected before object read");
    rejects<std::logic_error>([&]{f.binding.copy_getter(0xdeadbeef,q::Slot::Persona18);},"arbitrary unreadable owned-copy token rejected before object read");
    rejects<std::invalid_argument>([&]{f.binding.getter(object,q::Slot::Status08);},"status slot cannot be called as byte-string getter");
    rejects<std::invalid_argument>([&]{f.binding.copy_getter(object,q::Slot::Avatar28);},"observed avatar slot cannot be called as a zero-argument string getter");
    require(s.calls==std::array<unsigned,4>{1,2,1,1}&&!f.binding.snapshot().poisoned,"invalid metadata requests have no SDK effects and do not poison known object");
    f.binding.destroy(object);const auto before=s.calls;
    rejects<std::logic_error>([&]{f.binding.status(object);},"destroyed object cannot be reread through status");
    rejects<std::logic_error>([&]{f.binding.copy_getter(object,q::Slot::UserId20);},"destroyed object cannot be reread through owned copy");
    require(before==s.calls&&s.destroyed==1,"post-destroy queries never reach erased vptr/SDK");
}
void exceptions_and_null(){
    Fixture f;auto& s=f.state;const auto object=s.object();
    for(unsigned i=0;i<4;++i){
        s.throwing=static_cast<int>(i);bool same=false;
        try{if(i==0)f.binding.status(object);else f.binding.copy_getter(object,static_cast<q::Slot>(8+i*8));}catch(...){same=std::current_exception()==s.failure;}
        const auto view=f.binding.snapshot();require(same&&::GetLastError()==0x8808+i&&!view.active&&!view.poisoned&&!view.object_unknown&&view.object==object,"non-owning query exception preserves exact identity/error and known object for explicit cleanup");
    }
    s.throwing=-1;
    for(unsigned i=1;i<4;++i){
        s.nullptr_slot=static_cast<int>(i);const auto slot=static_cast<q::Slot>(8+i*8);
        require(f.binding.getter(object,slot)==nullptr,"raw getter preserves a normal null result");
        rejects<anadius::online_recovered::OutOfEvidenceInput>([&]{f.binding.copy_getter(object,slot);},"owned copy rejects null before scanning or field assignment");
        require(::GetLastError()==0x8808+i&&!f.binding.snapshot().active&&!f.binding.snapshot().poisoned&&!f.binding.snapshot().object_unknown,"null-copy failure keeps getter error and known live ownership");
    }
    f.binding.destroy(object);require(s.destroyed==1&&f.binding.snapshot().object==0,"query/null exceptions permit normal explicit destroy exactly once");
}
void concurrent_query(){
    Fixture f;auto& s=f.state;s.block=true;std::string copied;DWORD outgoing=0;
    std::thread thread([&]{copied=f.binding.copy_getter(s.object(),q::Slot::Username10);outgoing=::GetLastError();});
    {std::unique_lock lock(s.gate);s.changed.wait(lock,[&]{return s.entered;});}
    unsigned denied=0,lower=0;auto reject=[&](auto&& fn){try{fn();}catch(const std::logic_error&){++denied;}};
    reject([&]{f.binding.destroy(s.object());});reject([&]{f.binding.free(0x80,[&]{++lower;return true;});});
    reject([&]{f.binding.status(s.object());});reject([&]{f.binding.getter(s.object(),q::Slot::Persona18);});
    reject([&]{f.binding.copy_getter(s.object(),q::Slot::UserId20);});reject([&]{f.binding.resolve_factory(0x80,[&]{++lower;return v::Procedure{};});});
    const auto before=f.binding.snapshot();{std::lock_guard lock(s.gate);s.release=true;s.changed.notify_all();}thread.join();
    require(denied==6&&lower==0&&before.active&&!before.object_unknown&&before.factory_code_retained,"shared reservation rejects overlapping destroy/free/rebind/raw/owned queries before side effects");
    require(copied==s.username&&outgoing==0x8809&&!f.binding.snapshot().active&&s.destroyed==0,"blocked query completes owned copy once before explicit cleanup");
}
void concurrent_original_queries(){
    Fixture f(false,v::QueryPolicy::Concurrent);auto& s=f.state;const auto object=s.object();s.block=true;const auto expected=s.username;std::string first,second;std::exception_ptr worker_failure,body_failure;
    std::thread worker([&]{try{first=f.binding.copy_getter(object,q::Slot::Username10);}catch(...){worker_failure=std::current_exception();}});
    try{
        {std::unique_lock lock(s.gate);require(s.changed.wait_for(lock,std::chrono::seconds(10),[&]{return s.entered;}),"first concurrent owned getter reached its bounded foreign-body hold");}
        second=f.binding.copy_getter(object,q::Slot::Persona18);s.persona="changed after owned copy";
        require(second=="pid"&&f.binding.snapshot().active&&s.retired==0,"Second owned query completes independently while first query retains object/code ownership");
        require(f.binding.status(object)==s.status&&f.binding.snapshot().active,"Third typed query remains admissible after its sibling ended without clearing the first reservation");
        rejects<std::logic_error>([&]{f.binding.destroy(object);},"Exact otherwise-valid destroy still rejects while first query remains live");
        unsigned lower=0;rejects<std::logic_error>([&]{f.binding.free(0x80,[&]{++lower;return true;});},"Free cannot release adopted module during first query");require(!lower&&!s.destroyed&&!s.retired&&f.binding.snapshot().factory_code_retained,"Rejected lifecycle effects do not retire code or call the lower SDK");
    }catch(...){body_failure=std::current_exception();}
    {std::lock_guard lock(s.gate);s.release=true;s.changed.notify_all();}worker.join();
    if(worker_failure)std::rethrow_exception(worker_failure);if(body_failure)std::rethrow_exception(body_failure);
    s.username="changed after first copy";require(first==expected&&second=="pid"&&!f.binding.snapshot().active&&s.claim&&s.args,"All query reservations drain independently and returned owned strings survive provider mutation");
    f.binding.destroy(object);require(s.destroyed==1&&!f.binding.snapshot().object,"Destroy becomes valid only after the actual last query returns");f.binding.free(0x80,[]{return true;});require(s.retired==1,"Existing factory code lease retires once only after quiescent explicit cleanup");
}
[[noreturn]]void terminal(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(239);}
int concurrent_invalid_layout(){
    Fixture f(true,v::QueryPolicy::Concurrent);auto& s=f.state;const auto object=s.object();s.block=true;std::string first;std::exception_ptr worker_failure,body_failure;bool retained=false;
    std::thread worker([&]{try{first=f.binding.copy_getter(object,q::Slot::Username10);}catch(...){worker_failure=std::current_exception();}});
    try{
        {std::unique_lock lock(s.gate);require(s.changed.wait_for(lock,std::chrono::seconds(10),[&]{return s.entered;}),"invalid-layout sibling waits until first foreign query is active");}
        const std::uintptr_t zero=0;std::memcpy(s.table_data()+0x18,&zero,8);bool caught=false;try{f.binding.copy_getter(object,q::Slot::Persona18);}catch(const anadius::online_recovered::OutOfEvidenceInput&){caught=true;}
        const auto view=f.binding.snapshot();retained=caught&&view.poisoned&&view.object_unknown&&view.active&&view.factory_code_retained&&s.retired==0&&s.destroyed==0;
        DWORD previous=0;require(::VirtualProtect(s.page,4096,PAGE_NOACCESS,&previous)!=0,"own invalid-layout page made unreadable before rejected retry");rejects<std::logic_error>([&]{f.binding.status(object);},"Poisoned sibling retry rejects metadata before unreadable vptr access");
    }catch(...){body_failure=std::current_exception();}
    {std::lock_guard lock(s.gate);s.release=true;s.changed.notify_all();}worker.join();
    const auto final=f.binding.snapshot();if(retained&&!worker_failure&&!body_failure&&first==s.username&&!final.active&&final.poisoned&&final.object_unknown&&final.factory_code_retained&&s.retired==0&&s.destroyed==0)terminal(193);
    terminal(239); // retained unknown layout; never fabricate cleanup/unwind
}
int child(bool null_vptr){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);Fixture f(true);auto& s=f.state;
    const auto object=s.object();const std::uintptr_t zero=0;
    if(null_vptr)std::memcpy(s.object_data(),&zero,8);else std::memcpy(s.table_data()+0x18,&zero,8);
    bool expected=false;::SetLastError(0xaabb);
    try{f.binding.copy_getter(object,q::Slot::Persona18);}catch(const anadius::online_recovered::OutOfEvidenceInput&){expected=true;}
    const auto view=f.binding.snapshot();const auto error=::GetLastError();unsigned denied=0,lower=0;
    auto reject=[&](auto&& fn){try{fn();}catch(const std::logic_error&){++denied;}};
    // A retry must reject metadata BEFORE rereading any vptr: own page is unreadable.
    DWORD previous=0;if(!::VirtualProtect(s.page,4096,PAGE_NOACCESS,&previous))return 232;
    reject([&]{f.binding.status(object);});reject([&]{f.binding.getter(object,q::Slot::Username10);});reject([&]{f.binding.copy_getter(object,q::Slot::UserId20);});
    reject([&]{f.binding.destroy(object);});reject([&]{f.binding.free(0x80,[&]{++lower;return true;});});
    if(expected&&view.poisoned&&view.object_unknown&&!view.active&&view.object==object&&view.factory_code_retained&&s.retired==0&&s.destroyed==0&&s.calls==std::array<unsigned,4>{}&&denied==5&&lower==0&&error==0xaabb)terminal(null_vptr?191:192);
    return 237;
}
}
int main(int argc,char** argv){if(argc==2){if(std::string_view(argv[1])=="--concurrent-null-slot")return concurrent_invalid_layout();return child(std::string_view(argv[1])=="--null-vptr");}try{
    exact_queries();exceptions_and_null();concurrent_query();concurrent_original_queries();unsigned code=191;
    for(const auto* mode:{"--null-vptr","--null-slot"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child reaches exact invalid-layout retention/no-retry predicate; generic abort cannot pass");
    require(_spawnl(_P_WAIT,argv[0],argv[0],"--concurrent-null-slot",static_cast<const char*>(nullptr))==193,"bad-layout sibling retains the first query count/code lease until actual completion");
    std::cout<<"PASS: "<<checks<<" online typed query core checks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
