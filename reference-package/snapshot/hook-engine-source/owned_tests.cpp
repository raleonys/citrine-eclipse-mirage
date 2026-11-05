#include "owned.hpp"
#include "../anadius-host-platform-source/engine.hpp"
#include <array>
#include <atomic>
#include <csignal>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <process.h>
#include <thread>
#include <vector>

// Own test EXE only: deterministic allocation failure at the consuming factory's
// facade, Impl and shared-control-block allocations. No target injection.
std::atomic<unsigned> fail_allocation_after=0;
void* operator new(std::size_t size){const auto pending=fail_allocation_after.load();if(pending&&fail_allocation_after.fetch_sub(1)==1)throw std::bad_alloc();if(void* out=std::malloc(size?size:1))return out;throw std::bad_alloc();}
void operator delete(void* value)noexcept{std::free(value);}
void operator delete(void* value,std::size_t)noexcept{std::free(value);}

namespace {
using namespace recovered_hooks;namespace hp=anadius::host_platform;
unsigned checks=0;void require(bool value,const char* text){++checks;if(!value)throw std::runtime_error(text);}
template<class E,class F>void rejects(F&& f,const char* text){bool caught=false;try{f();}catch(const E&){caught=true;}require(caught,text);}
struct State {OwnedEngine* owner=nullptr;std::array<int,5> status{};std::array<unsigned,5> calls{};std::vector<int> retirement;std::function<void(Operation)> during;bool valid=true;void* target=nullptr;void* detour=nullptr;void** output=nullptr;};
struct Fake final:Api {
    State& s;explicit Fake(State& state):s(state){}
    ~Fake()override{s.retirement.push_back(1);if(s.owner){const auto snap=s.owner->snapshot();s.valid=s.valid&&snap.active&&snap.phase==OwnedPhase::Retiring;try{s.owner->enable(nullptr);s.valid=false;}catch(const std::logic_error&){}s.valid=s.valid&&s.owner->snapshot().phase==OwnedPhase::Retiring;}::SetLastError(0xaaaa);}
    int run(Operation op){const auto i=static_cast<std::size_t>(op);++s.calls[i];const auto snap=s.owner->snapshot();s.valid=s.valid&&snap.active&&!snap.has_result&&!s.owner->empty();if(s.during)s.during(op);::SetLastError(static_cast<DWORD>(0x4000+i));return s.status[i];}
    int initialize()override{return run(Operation::Initialize);}
    int create(void* t,void* d,void** o)override{s.target=t;s.detour=d;s.output=o;*o=reinterpret_cast<void*>(0xdead);return run(Operation::Create);}
    int enable(void*)override{return run(Operation::Enable);}int disable(void*)override{return run(Operation::Disable);}int uninitialize()override{return run(Operation::Uninitialize);}
};
struct Code {State& s;explicit Code(State& state):s(state){}~Code(){s.retirement.push_back(2);s.valid=s.valid&&s.retirement==std::vector<int>{1,2};::SetLastError(0xbbbb);}};
std::shared_ptr<OwnedEngine> make(State& s){std::unique_ptr<Api> lower=std::make_unique<Fake>(s);std::shared_ptr<void> code=std::make_shared<Code>(s);auto out=make_owned_engine(lower,code);s.owner=out.get();return out;}
void allocation_prefixes(){
    for(unsigned allocation:{1u,2u,3u}){
        State s;std::unique_ptr<Api> lower=std::make_unique<Fake>(s);std::shared_ptr<void> code=std::make_shared<Code>(s);
        ::SetLastError(0x5151);fail_allocation_after=allocation;
        rejects<std::bad_alloc>([&]{make_owned_engine(lower,code);},"known allocation failure propagated");fail_allocation_after=0;
        require(!lower&&!code&&s.valid&&s.retirement==std::vector<int>{1,2}&&::GetLastError()==0x5151,"facade/Impl/control allocation failure preserves lower-before-code ownership and LastError");
    }
}
void statuses_and_retirement(){
    rejects<std::invalid_argument>([]{OwnedEngine bad(nullptr,std::make_shared<int>(1));},"absent lower rejected");
    State s;auto engine=make(s);require(engine->empty()&&s.calls==std::array<unsigned,5>{},"passive fresh owner does not call lower");
    s.status[0]=MH_ERROR_ALREADY_INITIALIZED;require(engine->initialize()==MH_ERROR_ALREADY_INITIALIZED&&!engine->empty()&&::GetLastError()==0x4000,"normal init failure is forwarded and requires cleanup");
    void* original=nullptr;s.status[1]=MH_ERROR_UNSUPPORTED_FUNCTION;require(engine->create(reinterpret_cast<void*>(1),reinterpret_cast<void*>(2),&original)==MH_ERROR_UNSUPPORTED_FUNCTION&&original==reinterpret_cast<void*>(0xdead)&&s.target==reinterpret_cast<void*>(1)&&s.detour==reinterpret_cast<void*>(2)&&s.output==&original,"create operands and output on error preserved exactly");
    s.status[2]=MH_ERROR_ENABLED;s.status[3]=MH_ERROR_MEMORY_PROTECT;require(engine->enable(nullptr)==MH_ERROR_ENABLED&&engine->disable(nullptr)==MH_ERROR_MEMORY_PROTECT&&!engine->empty(),"enable/disable statuses do not manufacture empty proof");
    s.status[4]=MH_ERROR_MEMORY_PROTECT;require(engine->uninitialize()==MH_ERROR_MEMORY_PROTECT&&!engine->empty()&&engine->snapshot().has_result,"known failed uninitialize retains actual dirty state");
    s.status[4]=MH_OK;require(engine->uninitialize()==MH_OK&&engine->empty()&&::GetLastError()==0x4004,"explicit successful retry establishes logical empty state");
    rejects<std::logic_error>([&]{engine->initialize();},"closed wrapper cannot restart");s.status[4]=MH_ERROR_NOT_INITIALIZED;require(engine->uninitialize()==MH_ERROR_NOT_INITIALIZED&&engine->empty(),"upstream NOT_INITIALIZED also establishes logical empty state");
    bool wrong_thread=false;std::thread thread([&]{try{engine->disable(nullptr);}catch(const std::logic_error&){wrong_thread=true;}});thread.join();require(wrong_thread&&s.calls[3]==1&&engine->empty(),"wrong controller rejected before lower or empty-state mutation");
    ::SetLastError(0x9999);engine.reset();require(s.retirement==std::vector<int>{1,2}&&s.valid&&::GetLastError()==0x9999,"lower retires before code with facade snapshot live and LastError preserved");s.owner=nullptr;
}
void original_selection(){
    State s;auto engine=make(s);hp::AssemblyInputs in;::SetLastError(0x7472);hp::select_original_owned_engine(in,engine);
    require(::GetLastError()==0x7472&&in.engine==engine&&in.code_lifetime==engine&&!in.providers_ready&&!in.external_quiescence&&in.closure_profile==anadius::native_context::ClosureProfile::OriginalCrt&&engine->empty(),"Original host selection retains exact actual engine/code without executing it or installing proof callbacks");
    rejects<std::invalid_argument>([&]{hp::select_original_owned_engine(in,engine);},"Original host engine selection cannot replace committed owners");
    std::exception_ptr failure;std::thread crt([&]{try{require(engine->initialize()==MH_OK&&engine->uninitialize()==MH_OK,"Original engine performs actual lower initialize/cleanup on a distinct serialized CRT actor");}catch(...){failure=std::current_exception();}});crt.join();if(failure)std::rethrow_exception(failure);require(engine->empty()&&s.calls[0]==1&&s.calls[4]==1,"Distinct original cleanup actor preserves actual native statuses and empty engine state");
}
void matched_selection(){
    State s;auto engine=make(s);hp::AssemblyInputs in;
    rejects<std::invalid_argument>([&]{hp::select_owned_engine(in,engine);},"missing external proof rejected");require(!in.engine&&!in.code_lifetime,"rejected selection is passive");
    unsigned proofs=0;bool external=true;in.external_quiescence=[&]{++proofs;::SetLastError(0x7272);return external;};hp::select_owned_engine(in,engine);
    require(in.engine.get()==engine.get()&&!in.engine.owner_before(in.code_lifetime)&&!in.code_lifetime.owner_before(in.engine),"API and code token share exact actual engine owner");
    rejects<std::invalid_argument>([&]{hp::select_owned_engine(in,engine);},"selection refuses overwrite");
    require(in.external_quiescence()&&proofs==1&&::GetLastError()==0x7272,"fresh engine still requires external proof and preserves its error");
    engine->initialize();require(!in.external_quiescence()&&proofs==1,"dirty engine blocks external success before predicate");
    engine->uninitialize();external=false;require(!in.external_quiescence()&&proofs==2,"empty engine does not imply foreign drain");external=true;require(in.external_quiescence(),"both actual engine and external proof required");
    in.external_quiescence={};in.engine.reset();in.code_lifetime.reset();engine.reset();require(s.valid&&s.retirement==std::vector<int>{1,2},"matched owner retirement preserves lower/code order");s.owner=nullptr;
    State changed;auto other=make(changed);hp::AssemblyInputs altered;altered.external_quiescence=[&]{other->initialize();return true;};hp::select_owned_engine(altered,other);require(!altered.external_quiescence()&&!other->empty(),"post-predicate recheck rejects external proof that dirties engine");other->uninitialize();altered.external_quiescence={};altered.engine.reset();altered.code_lifetime.reset();other.reset();require(changed.valid,"postcheck repair retires cleanly");changed.owner=nullptr;
}
State* child_state=nullptr;
int child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,[](int){std::_Exit(child_state&&child_state->retirement.empty()?1462:1999);});State s;child_state=&s;auto engine=make(s);
    if(mode=="live-delete"){engine->initialize();engine.reset();return 1998;}
    if(mode=="failed-delete"){engine->initialize();s.status[4]=MH_ERROR_MEMORY_PROTECT;engine->uninitialize();engine.reset();return 1998;}
    s.during=[&](Operation){::SetLastError(0x3131);if(mode=="reentry"){try{engine->disable(nullptr);}catch(const std::logic_error&){}return;}throw std::runtime_error("unknown lower outcome");};
    try{if(mode=="create"){void* p=nullptr;engine->create(nullptr,nullptr,&p);}else if(mode=="enable")engine->enable(nullptr);else if(mode=="disable")engine->disable(nullptr);else if(mode=="uninitialize")engine->uninitialize();else engine->initialize();}catch(const std::exception&){const auto snap=engine->snapshot();if(snap.phase==OwnedPhase::Poisoned&&!snap.active&&!engine->empty()&&s.retirement.empty())std::_Exit(1461);}
    std::_Exit(1997);
}
}
int main(int argc,char** argv){if(argc==2)return child(argv[1]);try{allocation_prefixes();statuses_and_retirement();original_selection();matched_selection();for(const char* mode:{"initialize","create","enable","disable","uninitialize","reentry"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==1461,"unknown/reentrant lower outcomes poison and retain exact owners");for(const char* mode:{"live-delete","failed-delete"})require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==1462,"live/failed cleanup cannot silently retire owners");std::cout<<"PASS: "<<checks<<" owned-engine checks; allocation prefixes, exact status/output/error forwarding, matched external proof, owner order and eight isolated retained/terminal prefixes\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';std::_Exit(1);}}
