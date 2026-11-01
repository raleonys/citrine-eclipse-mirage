#include "query.hpp"
#include "../anadius-online-version-source/version.hpp"
#include <windows.h>
#include <array>
#include <chrono>
#include <condition_variable>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <new>
#include <thread>

// Observe the owned auth-string allocation without allocating in the observer.
namespace allocation {void(*observe)()noexcept=nullptr;}
void* operator new(std::size_t bytes){if(allocation::observe)allocation::observe();if(auto* p=std::malloc(bytes?bytes:1))return p;throw std::bad_alloc();}
void* operator new[](std::size_t bytes){return ::operator new(bytes);}
void operator delete(void* p)noexcept{std::free(p);}void operator delete[](void* p)noexcept{std::free(p);}
void operator delete(void* p,std::size_t)noexcept{std::free(p);}void operator delete[](void* p,std::size_t)noexcept{std::free(p);}

namespace {
namespace v=anadius::online_version;namespace q=anadius::online_query;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct Fault{};
struct State {
    v::Binding* binding=nullptr;std::array<std::byte,17> object_bytes{};std::array<std::byte,57> table_bytes{};
    unsigned avatars=0,auths=0,destroyed=0,code_drops=0,frees=0,unexpected=0,copy_allocations=0;
    bool known_active=true,args=true,copy_active=true,null_auth=false,throw_avatar=false,throw_auth=false;
    std::uint8_t avatar_result=1;std::int32_t size=0;DWORD incoming=0;
    const char16_t* path=nullptr;const char* owner=nullptr;const char* client=nullptr;
    std::u16string observed_path;std::string observed_owner,observed_client,auth_result=std::string(5000,'A');
    std::exception_ptr fault=std::make_exception_ptr(Fault{});std::function<void()> hook;
    std::mutex gate;std::condition_variable changed;bool block=false,entered=false,release=false,timed_out=false;
    std::byte* object_data(){return object_bytes.data()+1;}std::byte* table_data(){return table_bytes.data()+1;}
    v::Object object(){return reinterpret_cast<v::Object>(object_data());}
};
State* current=nullptr;
void observe(void* object){
    auto& s=*current;s.incoming=::GetLastError();const auto snapshot=s.binding->snapshot();
    s.known_active=s.known_active&&snapshot.active&&snapshot.object==s.object()&&!snapshot.poisoned&&!snapshot.object_unknown&&snapshot.factory_code_retained;
    s.args=s.args&&object==reinterpret_cast<void*>(s.object());if(s.hook)s.hook();
    std::unique_lock lock(s.gate);if(s.block){s.entered=true;s.changed.notify_all();if(!s.changed.wait_for(lock,std::chrono::seconds(5),[&]{return s.release;}))s.timed_out=true;}
}
std::uint8_t avatar(void* object,const char16_t* path,const char* owner,std::int32_t size){
    auto& s=*current;++s.avatars;observe(object);s.path=path;s.owner=owner;s.size=size;s.observed_path=path;s.observed_owner=owner;
    ::SetLastError(0x2828);if(s.throw_avatar)std::rethrow_exception(s.fault);return s.avatar_result;
}
const char* auth(void* object,const char* client){
    auto& s=*current;++s.auths;observe(object);s.client=client;s.observed_client=client;
    ::SetLastError(0x3030);if(s.throw_auth)std::rethrow_exception(s.fault);return s.null_auth?nullptr:s.auth_result.c_str();
}
void destroy(void* object){auto& s=*current;s.args=s.args&&object==reinterpret_cast<void*>(s.object());++s.destroyed;const std::uintptr_t zero=0;std::memcpy(s.object_data(),&zero,sizeof zero);}
std::int32_t unused_status(void*){++current->unexpected;return 0;}
const char* unused_getter(void*){++current->unexpected;return "";}
v::Object factory(const char16_t*){
    auto& s=*current;const auto table=reinterpret_cast<std::uintptr_t>(s.table_data());std::memcpy(s.object_data(),&table,sizeof table);
    const std::array<std::uintptr_t,7> slots{reinterpret_cast<std::uintptr_t>(&destroy),reinterpret_cast<std::uintptr_t>(&unused_status),reinterpret_cast<std::uintptr_t>(&unused_getter),reinterpret_cast<std::uintptr_t>(&unused_getter),reinterpret_cast<std::uintptr_t>(&unused_getter),reinterpret_cast<std::uintptr_t>(&avatar),reinterpret_cast<std::uintptr_t>(&auth)};
    std::memcpy(s.table_data(),slots.data(),sizeof slots);return s.object();
}
struct CodeOwner {State& state;~CodeOwner(){++state.code_drops;}};
struct Fixture {
    State state;v::Binding binding;
    Fixture():binding([this](v::Module module){if(module!=0x80)std::abort();return std::shared_ptr<void>(new CodeOwner{state});}){
        current=&state;state.binding=&binding;binding.load([]{return v::Module{0x80};});binding.resolve_factory(0x80,[]{return reinterpret_cast<v::Procedure>(&factory);});binding.create(reinterpret_cast<v::Procedure>(&factory),u"own-EXE-only");
    }
    void close(){if(binding.snapshot().object)binding.destroy(state.object());if(binding.snapshot().module)binding.free(0x80,[&]{++state.frees;return true;});}
    ~Fixture(){allocation::observe=nullptr;close();current=nullptr;}
};
void observe_copy_allocation()noexcept{auto& s=*current;++s.copy_allocations;const auto snap=s.binding->snapshot();s.copy_active=s.copy_active&&snap.active&&snap.object==s.object()&&!snap.object_unknown&&snap.factory_code_retained;::SetLastError(0xdead);}
void exact_arguments_and_results(){
    Fixture f;auto& s=f.state;const auto object=s.object();
    const char16_t path[]{u'M',u':',u'\\',char16_t{0xe9},char16_t{0x4e2d},0,u'x'};const char owner[]{'u','s','e','r',0,'x'};const char client[]{'c','l','i','e','n','t',0,'x'};
    require(q::query_slot(object,q::Slot::Avatar28)==reinterpret_cast<v::Procedure>(&avatar)&&q::query_slot(object,q::Slot::Auth30)==reinterpret_cast<v::Procedure>(&auth),"Unaligned own object and seven-slot vtable resolve exact28/30 addresses through byte copies");
    for(const auto result:std::array<std::uint8_t,4>{0,1,0x80,0xff}){s.avatar_result=result;::SetLastError(0x1111);const auto success=f.binding.avatar(object,path,owner,std::numeric_limits<std::int32_t>::min());require(success==(result!=0)&&s.incoming==0x1111&&::GetLastError()==0x2828,"Typed avatar preserves all nonzero AL results and incoming/outgoing LastError");}
    require(s.path==path&&s.owner==owner&&s.observed_path==std::u16string(path,5)&&s.observed_owner=="user"&&s.size==std::numeric_limits<std::int32_t>::min(),"Avatar forwards original UTF16/path and counted-NUL owner pointers plus unchanged signed size");
    ::SetLastError(0x2222);require(f.binding.auth(object,client)==s.auth_result.c_str()&&s.incoming==0x2222&&::GetLastError()==0x3030,"Raw auth preserves exact borrowed result pointer and native error");
    allocation::observe=observe_copy_allocation;auto copy=f.binding.copy_auth(object,client);allocation::observe=nullptr;
    require(copy==s.auth_result&&s.copy_allocations>0&&s.copy_active&&::GetLastError()==0x3030,"Long auth allocation/copy stays reserved and preserves getter error despite allocator observer changing it");
    s.auth_result[0]='B';require(copy.front()=='A'&&s.client==client&&s.observed_client=="client","Owned auth result is independent and client pointer retains native C-string boundary");
    s.auth_result.clear();require(f.binding.copy_auth(object,client).empty(),"Nonnull empty auth result is a valid empty owned string");s.auth_result=std::string("code\0ignored",12);require(f.binding.copy_auth(object,client)=="code","Auth owned copy scans to first native NUL");
    s.null_auth=true;require(f.binding.auth(object,client)==nullptr,"Raw auth preserves null result");rejects<anadius::online_recovered::OutOfEvidenceInput>([&]{(void)f.binding.copy_auth(object,client);},"Owned auth rejects normal null result without reading memory");
    require(s.known_active&&s.args&&!s.unexpected&&!f.binding.snapshot().active&&!f.binding.snapshot().poisoned&&!f.binding.snapshot().object_unknown&&::GetLastError()==0x3030,"All selected calls retain exact known object/code and balance reservations including null-copy failure");
    const auto avatars=s.avatars,auths=s.auths;constexpr v::Object unreadable=1;::SetLastError(0x5151);
    rejects<std::invalid_argument>([&]{(void)f.binding.avatar(unreadable,nullptr,owner,3);},"Null avatar path rejects before unreadable object access");rejects<std::invalid_argument>([&]{(void)f.binding.avatar(unreadable,path,nullptr,-3);},"Null avatar owner rejects before unreadable object access");rejects<std::invalid_argument>([&]{(void)f.binding.auth(unreadable,nullptr);},"Null auth client rejects before unreadable object access");rejects<std::invalid_argument>([&]{(void)f.binding.copy_auth(unreadable,nullptr);},"Null owned auth client rejects before unreadable object access");
    require(::GetLastError()==0x5151&&s.avatars==avatars&&s.auths==auths&&!f.binding.snapshot().poisoned,"Invalid input checks preserve error and perform no foreign call");
    f.binding.destroy(object);rejects<std::logic_error>([&]{(void)f.binding.avatar(object,path,owner,1);},"Stale avatar token rejects before cleared vptr access");rejects<std::logic_error>([&]{(void)f.binding.auth(object,client);},"Stale auth token rejects before cleared vptr access");rejects<std::logic_error>([&]{(void)f.binding.copy_auth(object,client);},"Stale owned auth token rejects before cleared vptr access");f.close();
    require(s.destroyed==1&&s.code_drops==1&&s.frees==1&&s.avatars==avatars&&s.auths==auths,"Explicit known cleanup destroys once then releases leased own code/module metadata");
}
void known_callback_exceptions_and_reentry(){
    Fixture f;auto& s=f.state;const auto object=s.object();
    s.throw_avatar=true;bool same=false;try{(void)f.binding.avatar(object,u"memory", "owner",-17);}catch(...){same=std::current_exception()==s.fault;}
    require(same&&::GetLastError()==0x2828&&!f.binding.snapshot().active&&!f.binding.snapshot().poisoned&&!f.binding.snapshot().object_unknown&&f.binding.snapshot().object==object,"Avatar callback exception preserves exact exception/error and known object while balancing reservation");
    s.throw_avatar=false;s.throw_auth=true;same=false;try{(void)f.binding.copy_auth(object,"client");}catch(...){same=std::current_exception()==s.fault;}
    require(same&&::GetLastError()==0x3030&&!f.binding.snapshot().active&&!f.binding.snapshot().poisoned&&!f.binding.snapshot().object_unknown,"Auth callback exception retains existing known-query policy without synthetic cleanup");s.throw_auth=false;
    unsigned denied=0,lower=0;s.hook=[&]{try{f.binding.destroy(object);}catch(const std::logic_error&){++denied;}try{f.binding.free(0x80,[&]{++lower;return true;});}catch(const std::logic_error&){++denied;}};
    require(f.binding.avatar(object,u"memory","owner",-17)&&denied==2&&lower==0&&!s.destroyed&&!s.code_drops,"Synchronous avatar reentry rejects destroy/free before lower effect while lease remains active");s.hook={};f.close();require(s.destroyed==1&&s.code_drops==1&&s.frees==1,"Callback throws/reentry still permit one valid explicit retirement");
}
void concurrent_reservation(){
    Fixture f;auto& s=f.state;s.block=true;std::string copied;std::exception_ptr failure;DWORD outgoing=0;
    std::thread worker([&]{try{copied=f.binding.copy_auth(s.object(),"client");outgoing=::GetLastError();}catch(...){failure=std::current_exception();}});
    bool entered;{std::unique_lock lock(s.gate);entered=s.changed.wait_for(lock,std::chrono::seconds(5),[&]{return s.entered;});}
    unsigned denied=0,lower=0;auto reject=[&](auto&& call){try{call();}catch(const std::logic_error&){++denied;}};
    if(entered){reject([&]{f.binding.destroy(s.object());});reject([&]{f.binding.free(0x80,[&]{++lower;return true;});});reject([&]{(void)f.binding.avatar(s.object(),u"memory","owner",4);});reject([&]{(void)f.binding.auth(s.object(),"client");});reject([&]{(void)f.binding.copy_auth(s.object(),"client");});}
    const auto during=f.binding.snapshot();{std::lock_guard lock(s.gate);s.release=true;s.changed.notify_all();}worker.join();if(failure)std::rethrow_exception(failure);
    require(entered&&!s.timed_out&&denied==5&&!lower&&during.active&&!during.poisoned&&!during.object_unknown&&during.factory_code_retained,"Blocked auth reserves object/code and rejects concurrent destroy/free/avatar/raw/owned auth before side effects");
    require(copied==s.auth_result&&outgoing==0x3030&&s.auths==1&&!s.avatars&&!s.destroyed&&!s.code_drops&&!f.binding.snapshot().active,"Reserved auth completes owned copy before releasing known object admission");f.close();require(s.destroyed==1&&s.code_drops==1&&s.frees==1,"Concurrent-query fixture retires through normal explicit destroy/free after actual join");
}
}
int main(){try{exact_arguments_and_results();known_callback_exceptions_and_reentry();concurrent_reservation();std::cout<<"PASS: "<<checks<<" online application-query checks; own EXE slots28/30 only\n";}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
