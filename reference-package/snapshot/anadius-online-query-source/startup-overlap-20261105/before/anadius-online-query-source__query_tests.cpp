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
    auto& s=*current;++s.calls[index];s.incoming=::GetLastError();
    const auto view=s.binding->snapshot();s.args=s.args&&object==reinterpret_cast<void*>(s.object());
    s.claim=s.claim&&view.active&&view.object==s.object()&&!view.object_unknown&&!view.poisoned&&view.factory_code_retained;
    {std::unique_lock lock(s.gate);if(s.block){s.entered=true;s.changed.notify_all();s.changed.wait(lock,[&]{return s.release;});}}
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
    explicit Fixture(bool paged=false):binding([this](v::Module){return std::shared_ptr<void>(new Pin{state});}){
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
[[noreturn]]void terminal(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(239);}
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
int main(int argc,char** argv){if(argc==2)return child(std::string_view(argv[1])=="--null-vptr");try{
    exact_queries();exceptions_and_null();concurrent_query();unsigned code=191;
    for(const auto* mode:{"--null-vptr","--null-slot"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"own child reaches exact invalid-layout retention/no-retry predicate; generic abort cannot pass");
    std::cout<<"PASS: "<<checks<<" online typed query core checks\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
