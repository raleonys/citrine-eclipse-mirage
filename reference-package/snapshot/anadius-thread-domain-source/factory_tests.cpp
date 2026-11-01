#include "factory.hpp"
#include "domain_test_fixture.hpp"
#include <iostream>

namespace {
using namespace anadius;namespace td=thread_domain;using b6_core_test::check;using b6_core_test::rejects;
td::InitialPolicy policy(){return {0x8f713a5268c4d209ULL,0,nullptr,nullptr,1};}
LPVOID WINAPI application_alloc(HANDLE heap,DWORD flags,SIZE_T bytes){
    if(flags==HEAP_ZERO_MEMORY)return b6_core_test::alloc(heap,flags,bytes);
    auto& state=*b6_core_test::observed;++state.allocations;
    check(heap==::GetProcessHeap()&&!flags,"application malloc uses same process heap without calloc flag");
    if(state.deny_alloc){::SetLastError(ERROR_NOT_ENOUGH_MEMORY);return nullptr;}
    auto* result=::HeapAlloc(heap,flags,bytes);const auto error=::GetLastError();if(result)state.blocks.emplace(result,bytes);::SetLastError(error);return result;
}
struct Backing {
    b6_core_test::Observation state;unsigned& retired;bool& valid;
    Backing(unsigned& n,bool& v):retired(n),valid(v){b6_core_test::observed=&state;}
    ~Backing(){++retired;valid=valid&&state.blocks.empty()&&state.initialized.size()==state.destroyed.size();b6_core_test::observed=nullptr;::SetLastError(0xeeee);}
};
void complete_preflight(){
    // Every missing SDK slot rejects before taking a supplied owner or running
    // a native constructor; fixing that input is still allowed before consume.
    for(unsigned slot=0;slot<21;++slot){
        td::InitialInputs in(policy());auto backing=std::make_shared<unsigned>(23);in.backing=backing;
        switch(slot){
        case 0:in.apis.heap.get_process_heap=nullptr;break;case 1:in.apis.heap.heap_alloc=nullptr;break;
        case 2:in.apis.heap.heap_realloc=nullptr;break;case 3:in.apis.heap.heap_free=nullptr;break;
        case 4:in.apis.heap.heap_size=nullptr;break;case 5:in.apis.heap.get_last_error=nullptr;break;
        case 6:in.apis.fls.get=nullptr;break;case 7:in.apis.fls.set=nullptr;break;
        case 8:in.apis.fls.allocate=nullptr;break;case 9:in.apis.fls.release=nullptr;break;
        case 10:in.apis.fls.error=nullptr;break;case 11:in.apis.fls.set_error=nullptr;break;
        case 12:in.apis.locks.initialize_spin=nullptr;break;case 13:in.apis.locks.destroy=nullptr;break;
        case 14:in.apis.locks.enter=nullptr;break;case 15:in.apis.locks.leave=nullptr;break;
        case 16:in.apis.runtime.load=nullptr;break;case 17:in.apis.runtime.procedure=nullptr;break;
        case 18:in.apis.runtime.release=nullptr;break;case 19:in.apis.runtime.error=nullptr;break;
        case 20:in.apis.runtime.process_flags=nullptr;break;
        }
        ::SetLastError(0x7474);rejects<std::invalid_argument>([&]{td::make_initial_domain(in);},"incomplete SDK rejected");
        check(in.backing==backing&&::GetLastError()==0x7474,"preflight preserves donor and LastError");
        in.apis=td::Apis{};auto domain=td::make_initial_domain(in);check(!in.backing&&domain->phase()==td::Phase::Constructed,"fixed preflight consumes owner and returns passive root");check(domain->close(),"uninitialized source root closes without initialization");
    }
}
void owning_identity_and_retries(){
    unsigned retired=0;bool valid=true;auto backing=std::make_shared<Backing>(retired,valid);std::weak_ptr<Backing> weak=backing;auto& observation=backing->state;
    td::InitialInputs in(policy());in.apis=b6_core_test::apis();in.apis.heap.heap_alloc=application_alloc;in.backing=backing;
    ::SetLastError(0x8181);auto domain=td::make_initial_domain(in);backing.reset();
    check(!in.backing&&!weak.expired()&&!retired&&::GetLastError()==0x8181,"Domain retains consumed SDK backing and preserves error");
    check(domain->phase()==td::Phase::Constructed&&!observation.heap_gets&&!observation.allocations&&!observation.slot_allocations&&observation.initialized.empty(),"factory does not initialize heap/locks/FLS/PTD");
    const auto identity=domain->locale().ids().exception_table;check(identity!=0,"owned opaque PTD identity is nonzero");
    td::InitialInputs other_input(policy());rejects<std::logic_error>([&]{td::make_initial_domain(other_input);},"existing single FLS-binding reservation rejects a simultaneous Domain");
    rejects<std::logic_error>([&]{td::make_initial_domain(other_input);},"failed post-consumption construction remains single-use");
    check(domain->phase()==td::Phase::Constructed&&domain->locale().ids().exception_table==identity,"second root rejection preserves first Domain and identity");
    rejects<std::logic_error>([&]{td::make_initial_domain(in);},"consumed factory is single-use");
    observation.deny_slot=true;check(!domain->initialize()&&domain->phase()==td::Phase::Constructed&&!weak.expired()&&!retired,"known initialization failure retains root/backing for existing retry");
    observation.deny_slot=false;check(domain->initialize(),"existing initialize retries after known clean prefix");
    auto* ptd=domain->current_ptd();check(ptd&&crt_ptd_data_recovered::View(ptd).load<std::uintptr_t>(0)==identity,"actual controller PTD stores factory-owned identity");
    check(domain->locale_references()==2&&domain->multibyte_references()==1,"actual initial-locale references installed");
    auto memory=domain->make_memory();void* block=memory->allocate(37);check(block&&memory->domain_identity()==domain.get(),"factory root supplies actual same-Domain allocator");
    check(!domain->close()&&domain->phase()==td::Phase::Stopping&&!retired,"provider lease seals Domain into Stopping before SDK retirement");
    memory->deallocate(block,37);memory.reset();observation.deny_release=true;check(!domain->close()&&!weak.expired()&&!retired,"known native FLS release failure retains owned backing");
    const auto releases=observation.slot_releases;observation.deny_release=false;check(domain->close()&&observation.slot_releases==releases+1,"existing close retry releases only retained native FLS reference");
    check(domain->locale_references()==1&&!domain->multibyte_references()&&observation.blocks.empty()&&observation.initialized.size()==observation.destroyed.size(),"all PTD/heap/locks retired before backing");
    check(!retired&&!weak.expired(),"closed but retained Domain still owns identity/SDK backing");auto alias=domain;domain.reset();check(!retired,"shared Domain alias retains backing");alias.reset();check(retired==1&&weak.expired()&&valid,"final root destruction retires backing after all dependents");
}
}
int main(){try{complete_preflight();owning_identity_and_retries();std::cout<<"PASS: "<<b6_core_test::checks<<" initial-domain factory checks; complete SDK preflight, passive owned identity, actual PTD/allocator and existing initialization/FLS close retries\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
