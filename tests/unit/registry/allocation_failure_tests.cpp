// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/platform/registry.hpp"
#include <array>
#include <cstdlib>
#include <iostream>
#include <new>

namespace allocation {
unsigned remaining=0;
bool fired=false;
void* acquire(std::size_t size){
    if(remaining&&!--remaining){fired=true;throw std::bad_alloc();}
    if(auto* p=std::malloc(size?size:1))return p;
    throw std::bad_alloc();
}
}
void* operator new(std::size_t size){return allocation::acquire(size);}
void* operator new[](std::size_t size){return allocation::acquire(size);}
void operator delete(void* p) noexcept{std::free(p);}
void operator delete[](void* p) noexcept{std::free(p);}
void operator delete(void* p,std::size_t) noexcept{std::free(p);}
void operator delete[](void* p,std::size_t) noexcept{std::free(p);}

namespace {
namespace r=preloader_l::registry;
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}
struct LogFailure {unsigned identity=73;};
enum Event {original,allocate,log,finish,release};
struct Effects final:r::Effects {
    unsigned fail_at=0,log_fault=0,allocations=0,frees=0;
    bool fail_allocate=false,live=false;
    r::Handle allocated=0;
    std::array<Event,8> events{};
    unsigned count=0;
    r::Status original_open_a(const r::OpenA&) override{events[count++]=original;return 5;}
    r::Status original_open_w(const r::OpenW&) override{events[count++]=original;return 5;}
    r::Handle allocate_4_04b0bc() override{
        events[count++]=allocate;++allocations;
        if(fail_allocate)throw std::bad_alloc();
        allocated=reinterpret_cast<r::Handle>(::operator new(4));live=true;
        allocation::remaining=fail_at;allocation::fired=false;
        return allocated;
    }
    void free_04b0f8(r::Handle key) override{
        require(live&&key==allocated,"release must own the exact live fabricated allocation");
        events[count++]=release;++frees;live=false;
        ::operator delete(reinterpret_cast<void*>(key));
    }
    void log_literal_006880(std::uint64_t) override{
        events[count++]=log;if(log_fault==1)throw LogFailure{};
    }
    void finish_log_0088e0() override{
        events[count++]=finish;if(log_fault==2)throw LogFailure{};
    }
    std::string filesystem_narrow(std::u16string_view text) override{return {text.begin(),text.end()};}
    int crt_stricmp(const char* a,const char* b) override{return std::string_view(a).compare(b);}
    int crt_wcsicmp(const char16_t* a,const char16_t* b) override{return std::u16string_view(a).compare(b);}
    int crt_strcpy_s(char*,std::size_t,const char*) override{throw std::logic_error("unexpected query copy");}
    int crt_wcscpy_s(char16_t*,std::size_t,const char16_t*) override{throw std::logic_error("unexpected query copy");}
    r::Status original_query_a(const r::QueryA&) override{throw std::logic_error("unexpected query");}
    r::Status original_query_w(const r::QueryW&) override{throw std::logic_error("unexpected query");}
    r::Status original_close(r::Handle) override{throw std::logic_error("fabricated key must not reach native close");}
    void wait_036880(std::int32_t) override{throw std::logic_error("unexpected wait");}
    void log_text_007990(std::string_view) override{throw std::logic_error("unexpected query log");}
};
struct Fixture {
    r::Values values{{"ClientKey",u"client"},{"ClientKey2",u"client2"},{"LanguageKey",u"language"}};
    r::Handles client{11},language{22},fabricated{33};
    std::uint8_t enabled=1;
    r::Handle output=99;
    Effects effects;
    r::StateRefs state(){return {values,client,language,fabricated,enabled};}
    r::Status open(bool wide,bool locale){
        return wide?r::open_w_00f960(state(),effects,{0,locale?u"language":u"client",0,0,&output}):
            r::open_a_00f800(state(),effects,{0,locale?"language":"client",0,0,&output});
    }
    void unchanged(){
        require(output==99,"failed creation must restore the caller output");
        require(client==r::Handles{11}&&language==r::Handles{22}&&fabricated==r::Handles{33},
            "failed creation must remove only its own partial ledger entries");
        require(!effects.live&&effects.frees==effects.allocations,"failed creation releases its allocation exactly once");
    }
};
void insertion_failures(){
    for(bool wide:{false,true})for(bool locale:{false,true}){
        unsigned failures=0;bool completed=false;
        for(unsigned point=1;point<=8;++point){
            Fixture f;f.effects.fail_at=point;
            try{
                const auto status=f.open(wide,locale);
                allocation::remaining=0;
                require(status==0&&!allocation::fired,"successful creation preserves its status");
                require(f.output==f.effects.allocated&&f.fabricated.contains(f.output),"successful output has an owning ledger");
                require((locale?f.language:f.client).contains(f.output),"successful output has its route ledger");
                require(f.effects.live&&f.effects.frees==0,"successful creation transfers allocation ownership");
                require(f.effects.count==4&&f.effects.events[0]==original&&f.effects.events[1]==allocate&&
                    f.effects.events[2]==log&&f.effects.events[3]==finish,"successful effect order is unchanged");
                require(r::close_00f5b0(f.state(),f.effects,f.output)==0,"ordinary close releases the committed key");
                require(f.effects.frees==1&&!f.effects.live,"committed key is freed exactly once");
                completed=true;break;
            }catch(const std::bad_alloc&){
                allocation::remaining=0;
                require(allocation::fired,"failure must come from an actual post-allocation container insertion");
                ++failures;f.unchanged();
            }
        }
        require(completed&&failures>=2,"both actual set insertion failures must be exercised before success");
    }
}
void other_failures(){
    for(bool wide:{false,true})for(bool locale:{false,true}){
        {Fixture f;f.effects.fail_allocate=true;
         try{(void)f.open(wide,locale);require(false,"expected allocator failure");}
         catch(const std::bad_alloc&){require(f.output==99&&f.effects.frees==0&&!f.effects.live,"allocation failure creates no release obligation");}}
        for(unsigned fault:{1u,2u}){
            Fixture f;f.effects.log_fault=fault;
            try{(void)f.open(wide,locale);require(false,"expected original logging exception");}
            catch(const LogFailure& e){require(e.identity==73,"logging exception identity survives rollback");f.unchanged();}
        }
    }
    {Fixture f;r::Status status=5;f.effects.fail_at=1;
     try{r::fabricate_010170(f.state(),f.effects,&f.output,status);require(false,"expected direct fabrication insertion failure");}
     catch(const std::bad_alloc&){allocation::remaining=0;require(status==5,"direct failure preserves original status");f.unchanged();}}
}
}
int main()try{insertion_failures();other_failures();std::cout<<"PASS: actual registry set insertion failures, ownership rollback and A/W success controls\n";}
catch(const std::exception& e){allocation::remaining=0;std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
catch(...){allocation::remaining=0;std::cerr<<"FAIL: unexpected exception\n";return 1;}
