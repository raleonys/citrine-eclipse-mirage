#include "reference.hpp"
#include <bit>
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <process.h>
#include <string_view>
#include <system_error>

namespace {
using namespace anadius;namespace mr=module_reference;namespace ov=online_version;
unsigned checks=0;
void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct State {unsigned acquisitions=0,releases=0;bool valid=true,fail=false,wrong=false,null_success=false,dirty_failure=false,throw_acquire=false,throw_release=false,fail_release=false;ov::Binding* binding=nullptr;};
State* current=nullptr;
BOOL WINAPI acquire(DWORD flags,LPCWSTR address,HMODULE* result){
    auto& s=*current;++s.acquisitions;s.valid=s.valid&&flags==GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS&&reinterpret_cast<std::uintptr_t>(address)==0x80&&result&&!*result;
    if(s.throw_acquire)throw std::runtime_error("unknown acquisition");
    *result=s.null_success?nullptr:reinterpret_cast<HMODULE>(s.wrong?0x90:0x80);
    if(s.fail){*result=s.dirty_failure?reinterpret_cast<HMODULE>(0x80):nullptr;::SetLastError(ERROR_MOD_NOT_FOUND);return FALSE;}
    ::SetLastError(0x2222);return TRUE;
}
BOOL WINAPI release(HMODULE module){auto& s=*current;++s.releases;s.valid=s.valid&&reinterpret_cast<std::uintptr_t>(module)==0x80;if(s.binding){auto view=s.binding->snapshot();s.valid=s.valid&&view.active&&!view.module&&!view.procedure;}
    ::SetLastError(0x3333);if(s.throw_release)throw std::runtime_error("unknown release");return s.fail_release?FALSE:TRUE;
}
void ownership_and_failure(){
    State s;current=&s;
    rejects<std::invalid_argument>([]{mr::make_code_lease({nullptr,release});},"missing acquisition API rejected");
    rejects<std::invalid_argument>([]{mr::make_code_lease({acquire,nullptr});},"missing release API rejected");
    auto lease=mr::make_code_lease({acquire,release});
    ::SetLastError(0x1111);rejects<std::invalid_argument>([&]{lease(0);},"zero module rejected before SDK");require(!s.acquisitions&&::GetLastError()==0x1111,"validation preserves LastError without effects");
    auto first=lease(0x80);require(s.acquisitions==1&&!s.releases&&s.valid&&::GetLastError()==0x1111,"exact FROM_ADDRESS adds reference preserving LastError");
    auto alias=first;first.reset();require(!s.releases,"alias prevents premature release");
    auto second=lease(0x80);require(s.acquisitions==2,"separate successful leases acquire separately");
    second.reset();require(s.releases==1&&::GetLastError()==0x1111,"one lease releases one reference preserving LastError");
    alias.reset();require(s.releases==2&&s.valid,"last alias releases exactly once");
    s.fail=true;bool caught=false;try{lease(0x80);}catch(const std::system_error& e){caught=e.code().value()==ERROR_MOD_NOT_FOUND;}
    require(caught&&s.acquisitions==3&&s.releases==2&&::GetLastError()==0x1111,"known acquisition failure reports SDK error without guessed FreeLibrary");
    s.fail=false;
    struct Backing {unsigned& retired;explicit Backing(unsigned& n):retired(n){}~Backing(){++retired;::SetLastError(0x9999);}};
    unsigned retired=0;auto backing=std::make_shared<Backing>(retired);auto factory=mr::make_code_lease({acquire,release},backing);auto held=factory(0x80);backing.reset();factory={};require(!retired,"lease retains SDK backing after factory dies");
    held.reset();require(retired==1&&s.releases==3&&::GetLastError()==0x1111,"final release precedes backing destruction and preserves error");
    ov::Binding binding(lease);s.binding=&binding;require(binding.load([]{return 0x80;})==0x80,"actual Binding tracks synthetic loaded base");binding.resolve(0x80,[]{::SetLastError(0x5151);return 0x123;});require(::GetLastError()==0x5151,"lookup error survives acquiring real provider lease");
    require(!binding.free(0x80,[]{::SetLastError(0x6161);return false;}),"source false free outcome preserved");
    require(s.valid&&s.releases==4&&::GetLastError()==0x6161,"lease deleter observes reserved Binding snapshot outside its mutex");s.binding=nullptr;
}
void native_reference(const char* path){
    // The ONLY module loaded here is our separately compiled synthetic fixture.
    HMODULE module=::LoadLibraryA(path);require(module,"own fixture loaded");const auto base=reinterpret_cast<ov::Module>(module);
    auto lease=mr::make_code_lease();auto retained=lease(base);auto alias=retained;auto procedure=std::bit_cast<ov::VersionFunction>(::GetProcAddress(module,"version_check"));require(procedure,"own export resolved");
    require(::FreeLibrary(module)!=0,"own initial loader reference released");require(::GetModuleHandleA(path)==module&&procedure()==2,"real OS lease keeps fixture code callable after original reference retires");
    retained.reset();require(::GetModuleHandleA(path)==module,"shared alias retains actual OS reference");alias.reset();require(!::GetModuleHandleA(path),"last alias releases final actual OS reference and unloads own fixture");
    ov::Binding binding(mr::make_code_lease());const auto loaded=binding.load([&]{return reinterpret_cast<ov::Module>(::LoadLibraryA(path));});require(loaded,"actual Binding loads own fixture");
    const auto version=binding.resolve(loaded,[&]{return reinterpret_cast<ov::Procedure>(::GetProcAddress(reinterpret_cast<HMODULE>(loaded),"version_check"));});require(binding.call(version)==2,"actual Binding version call uses real module reference");
    const auto factory=binding.resolve_factory(loaded,[&]{return reinterpret_cast<ov::Procedure>(::GetProcAddress(reinterpret_cast<HMODULE>(loaded),"create_online"));});const auto object=binding.create(factory,u"own");require(object&&binding.status(object)==23,"actual object factory/status runs in retained own DLL");
    require(binding.copy_getter(object,online_query::Slot::Username10)=="reference-fixture"&&binding.copy_auth(object,"own-client")=="own-auth","actual getters and copied auth use own retained code");
    binding.destroy(object);using Count=unsigned(*)();const auto count=std::bit_cast<Count>(::GetProcAddress(reinterpret_cast<HMODULE>(loaded),"destroyed_count"));require(count&&count()==1,"object destroy executes exactly once before module release");
    require(binding.free(loaded,[&]{const auto result=::FreeLibrary(reinterpret_cast<HMODULE>(loaded));require(result&&::GetModuleHandleA(path)==reinterpret_cast<HMODULE>(loaded)&&count()==1,"version/factory leases retain code during lower FreeLibrary");::SetLastError(0x7373);return result!=0;}),"actual Binding free returns lower success");
    require(::GetLastError()==0x7373,"actual FreeLibrary and both lease retirements preserve lower LastError");
    require(!::GetModuleHandleA(path),"both Binding leases retired and own fixture unloaded");
    const auto view=binding.snapshot();require(!view.module&&!view.code_retained&&!view.factory_code_retained&&!view.active,"actual Binding closes all selected module references");
}
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,[](int){std::_Exit(1441);});State s;current=&s;
    s.throw_acquire=mode=="throw-acquire";s.throw_release=mode=="throw-release";s.fail_release=mode=="fail-release";s.wrong=mode=="wrong";s.null_success=mode=="null-success";s.fail=s.dirty_failure=mode=="dirty-failure";
    auto lease=mr::make_code_lease({acquire,release});auto held=lease(0x80);held.reset();return 3;
}
}
int main(int argc,char** argv){if(argc==3&&std::string_view(argv[1])=="--child")return child(argv[2]);try{require(argc==2,"own fixture path argument required");ownership_and_failure();const auto path=std::filesystem::absolute(argv[1]).make_preferred().string();native_reference(path.c_str());for(const char* mode:{"throw-acquire","throw-release","fail-release","wrong","null-success","dirty-failure"})require(_spawnl(_P_WAIT,argv[0],argv[0],"--child",mode,static_cast<const char*>(nullptr))==1441,"malformed/unknown reference outcomes fail-stop in isolated child");std::cout<<"PASS: "<<checks<<" module-reference checks; real OS reference/unload, actual Binding+own DLL object, controlled SDK/errors/backing and six isolated fail-stops\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
