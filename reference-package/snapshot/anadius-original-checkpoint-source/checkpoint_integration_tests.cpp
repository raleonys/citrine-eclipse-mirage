#include "checkpoint.hpp"
#include "../anadius-original-provider-source/originals.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <array>
#include <bit>
#include <csignal>
#include <process.h>

// Own SDK stubs/source Context, heap and FLS only. No target DLL/game, real
// original API, user data or actual hook operation is loaded or invoked.
namespace {
using namespace anadius;
namespace fc=foreign_callback;namespace cp=callback_policy;namespace n=native_recovered;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
[[noreturn]]void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
void generic_abort(int){terminate_own(239);}
enum class Mode {Normal,Throw,SDKNull,SDKSnapshot,ResolverNull,ResolverThrowNull};
struct Fault {unsigned identity;};
struct Trace {
    Mode mode=Mode::Normal;DWORD index=FLS_OUT_OF_INDEXES;
    unsigned reads=0,resolves=0,sdk_calls=0;std::vector<char> events;
    bool arguments=false,mutated=false;DWORD incoming=0;
    std::array<char,32> output{};void* expected_ptd=nullptr;
    std::weak_ptr<int> code;std::weak_ptr<n::OriginalProvider> provider;
    fc::Domain* domain=nullptr;std::exception_ptr failure;
    Trace(){output.fill('!');}
};
Trace* current=nullptr;
PVOID WINAPI observed_get(DWORD index){
    const auto incoming=::GetLastError();
    if(current&&current->index==index){++current->reads;current->events.push_back('G');}
    ::SetLastError(incoming);return ::FlsGetValue(index);
}
HRESULT WINAPI own_original(HWND window,int id,HANDLE token,DWORD flags,LPSTR out){
    auto* trace=current;if(!trace)terminate_own(237);
    const auto incoming=::GetLastError();++trace->sdk_calls;trace->events.push_back('S');trace->incoming=incoming;
    trace->arguments=window==reinterpret_cast<HWND>(0x11)&&id==7&&token==reinterpret_cast<HANDLE>(0x22)&&flags==0x33&&out==trace->output.data();
    std::memcpy(out,"SDK-prefix",11);
    if(trace->mode==Mode::SDKNull||trace->mode==Mode::SDKSnapshot){
        auto* ptd=::FlsGetValue(trace->index);if(!ptd||ptd==crt_ptd_recovered::construction_sentinel())terminate_own(237);
        trace->expected_ptd=ptd;
        if(trace->mode==Mode::SDKNull){if(!::FlsSetValue(trace->index,nullptr))terminate_own(237);}
        else static_cast<unsigned char*>(ptd)[0]^=1;
        trace->mutated=true;
    }
    ::SetLastError(0x789a);
    if(trace->mode==Mode::Throw)std::rethrow_exception(trace->failure);
    return static_cast<HRESULT>(0x80070005U);
}
class ObservedProvider final:public n::OriginalProvider {
    std::shared_ptr<n::OriginalProvider> lower_;std::shared_ptr<Trace> trace_;
public:
    ObservedProvider(std::shared_ptr<n::OriginalProvider> lower,std::shared_ptr<Trace> trace):lower_(std::move(lower)),trace_(std::move(trace)){}
    const n::OriginalSlots* slots_identity()const noexcept override{return lower_->slots_identity();}
    std::uintptr_t load(n::Callback callback)override{
        const auto incoming=::GetLastError();++trace_->resolves;trace_->events.push_back('R');::SetLastError(incoming);
        const auto address=lower_->load(callback);
        if(trace_->mode==Mode::ResolverNull||trace_->mode==Mode::ResolverThrowNull){
            trace_->expected_ptd=::FlsGetValue(trace_->index);
            if(!trace_->expected_ptd||!::FlsSetValue(trace_->index,nullptr))terminate_own(237);
            trace_->mutated=true;::SetLastError(0x6789);
            if(trace_->mode==Mode::ResolverThrowNull)std::rethrow_exception(trace_->failure);
        }
        return address;
    }
    void setup_begin()override{lower_->setup_begin();}
    void setup_complete()override{lower_->setup_complete();}
    void stop()noexcept override{lower_->stop();}
};
struct Fixture {
    b6_core_test::Fixture heap;b15_test::Driver driver;
    std::shared_ptr<thread_domain::ApplicationMemory> memory;
    std::shared_ptr<std::ostringstream> stream=std::make_shared<std::ostringstream>();
    std::shared_ptr<Trace> trace=std::make_shared<Trace>();
    Fixture(){
        heap.domain.reset();auto apis=b6_core_test::apis();
        apis.heap={b6_core_test::get_heap,b16_test::selected_alloc,::HeapReAlloc,b16_test::selected_free,::HeapSize,::GetLastError};
        apis.fls.get=observed_get;heap.domain=thread_domain::DomainStorage::create(b6_core_test::seed(),apis);
        if(!heap.domain->initialize())throw std::runtime_error("Own original checkpoint FLS initialization");
        memory=heap.domain->make_memory();trace->index=heap.domain->native_fls_index();current=trace.get();
        driver.runtime.originals.at(8)=reinterpret_cast<std::uintptr_t>(&own_original);
    }
    ~Fixture(){current=nullptr;memory.reset();}
    cp::Resources resources(bool owned){
        cp::Resources resources;resources.memory=memory;resources.stream=stream;
        if(owned){
            auto code=std::make_shared<int>(24);trace->code=code;
            resources.originals=[trace=trace,code](const n::OriginalSlots& slots){
                auto provider=std::make_shared<ObservedProvider>(original_provider::bind(slots,code),trace);trace->provider=provider;return provider;
            };
        }
        return resources;
    }
};
Trace* terminal_trace=nullptr;
struct Errors final:n::Errors {
    [[noreturn]]void lifecycle(n::LifecycleFault)override{terminate_own(237);}
    [[noreturn]]void callback_exception(n::Callback callback,std::exception_ptr failure)noexcept override{
        auto* trace=terminal_trace;const auto outgoing=::GetLastError();
        if(trace&&trace->domain&&callback==n::Callback::folder_a_00fc00&&failure==trace->failure&&trace->sdk_calls==1&&trace->resolves==1&&trace->arguments&&
            std::string(trace->output.data(),10)=="SDK-prefix"&&trace->output[11]=='!'&&outgoing==0x789a&&!trace->code.expired()&&
            !trace->domain->active()&&trace->domain->ready()&&::FlsGetValue(trace->index)==nullptr&&n::active_calls()==0)terminate_own(201);
        terminate_own(237);
    }
};
Errors errors;
host_callbacks_recovered::FolderCall<char> direct_call(Trace& trace){return {0x11,7,0x22,0x33,trace.output.data()};}
void actual_native_factory(bool owned){
    Fixture fixture;auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});fixture.trace->domain=domain.get();
    auto factory=fc::make_factory(domain,b15_test::seed(),fixture.resources(owned),errors,[]{return true;});
    auto attachment=factory(fixture.driver.view());factory={};attachment->publish();attachment->host_setup_begin();attachment->host_setup_complete();
    bool result=false,slot_clean=false;DWORD outgoing=0;std::thread worker([&]{
        ::SetLastError(0x1111);const auto value=n::folder_a_entry(reinterpret_cast<HWND>(0x11),7,reinterpret_cast<HANDLE>(0x22),0x33,fixture.trace->output.data());
        outgoing=::GetLastError();result=value==static_cast<HRESULT>(0x80070005U);slot_clean=::FlsGetValue(fixture.trace->index)==nullptr;
    });worker.join();
    require(result&&fixture.trace->arguments&&fixture.trace->incoming==0x1111&&outgoing==0x789a,"Actual foreign factory preserves exact original SDK arguments, scalar result and incoming/outgoing WinLastError");
    require(fixture.trace->sdk_calls==1&&fixture.trace->resolves==(owned?1U:0U)&&std::string(fixture.trace->output.data(),10)=="SDK-prefix"&&fixture.trace->output[11]=='!',"Owned and raw original paths call own SDK stub once and preserve written prefix and unused tail");
    require(slot_clean&&!domain->active()&&domain->ready()&&n::active_calls()==0,"Successful original boundary leaves transient PTD and native binding lease fully drained");
    require(!owned||!fixture.trace->code.expired(),"Selected code owner remains retained after successful native original call");
    require(attachment->pause_before_cleanup()&&attachment->close_after_cleanup(),"Separate external quiescence closes selected native factory after pause");
    attachment.reset();require(!owned||fixture.trace->code.expired(),"Context destruction releases optional original code owner after successful shutdown");
}
void exact_actual_permission_sequence(bool owned){
    Fixture fixture;auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});fixture.trace->domain=domain.get();
    fc::Admission admission(domain);auto resources=fixture.resources(owned);resources.callback_scope=[domain]{return domain->permits_current();};
    auto seed=b15_test::seed();cp::Policy policy({fixture.driver.view(),seed},std::move(resources));resources.originals={};
    if(auto provider=fixture.trace->provider.lock()){provider->setup_begin();provider->setup_complete();}
    admission.host_setup_begin();admission.host_setup_complete();std::exception_ptr failure;
    std::thread worker([&]{try{
        admission.enter(n::Callback::folder_a_00fc00);fixture.trace->events.clear();const auto before=fixture.trace->reads;
        ::SetLastError(0x2222);const auto result=policy.original_folder_a(direct_call(*fixture.trace));const auto outgoing=::GetLastError();
        require(fixture.trace->reads==before+3,"Actual admitted PTD is probed exactly before resolution, after resolution and after own SDK return");
        const std::vector<char> expected=owned?std::vector<char>{'G','R','G','S','G'}:std::vector<char>{'G','G','S','G'};
        require(fixture.trace->events==expected,"Actual FLS/resolver/SDK order isolates all three original checkpoints from attach and cleanup reads");
        require(result==0x80070005U&&fixture.trace->arguments&&fixture.trace->incoming==0x2222&&outgoing==0x789a,"Direct selected Policy original boundary preserves arguments and both SDK error states");
        admission.leave(n::Callback::folder_a_00fc00);
        require(!domain->active()&&::FlsGetValue(fixture.trace->index)==nullptr,"Known normal cleanup remains available after exact original boundary sequence");
    }catch(...){failure=std::current_exception();}});worker.join();if(failure)std::rethrow_exception(failure);
    require(domain->pause(),"Exact-boundary source fixture shuts down only under explicit pause");
}
int terminal_child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,generic_abort);
    Fixture fixture;auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});fixture.trace->domain=domain.get();
    fixture.trace->failure=std::make_exception_ptr(Fault{24});
    if(mode=="--native-valid-throw"){
        fixture.trace->mode=Mode::Throw;terminal_trace=fixture.trace.get();
        auto factory=fc::make_factory(domain,b15_test::seed(),fixture.resources(true),errors,[]{return true;});
        auto attachment=factory(fixture.driver.view());factory={};attachment->publish();attachment->host_setup_begin();attachment->host_setup_complete();
        std::thread worker([&]{(void)n::folder_a_entry(reinterpret_cast<HWND>(0x11),7,reinterpret_cast<HANDLE>(0x22),0x33,fixture.trace->output.data());terminate_own(230);});worker.join();return 230;
    }
    const bool owned=mode!="--raw-sdk-null";unsigned expected=202;
    if(mode=="--sdk-null"||mode=="--raw-sdk-null"){fixture.trace->mode=Mode::SDKNull;expected=owned?202:206;}
    else if(mode=="--sdk-snapshot"){fixture.trace->mode=Mode::SDKSnapshot;expected=203;}
    else if(mode=="--resolver-null"){fixture.trace->mode=Mode::ResolverNull;expected=204;}
    else if(mode=="--resolver-throw-null"){fixture.trace->mode=Mode::ResolverThrowNull;expected=205;}
    else return 238;
    fc::Admission admission(domain);auto resources=fixture.resources(owned);resources.callback_scope=[domain]{return domain->permits_current();};
    auto seed=b15_test::seed();cp::Policy policy({fixture.driver.view(),seed},std::move(resources));resources.originals={};
    if(auto provider=fixture.trace->provider.lock()){provider->setup_begin();provider->setup_complete();}
    admission.host_setup_begin();admission.host_setup_complete();
    std::thread worker([&]{
        admission.enter(n::Callback::folder_a_00fc00);fixture.trace->events.clear();
        const auto blocks=fixture.heap.memory.blocks.size(),locks=fixture.heap.memory.destroyed.size();const auto frees=fixture.heap.memory.frees;
        try{(void)policy.original_folder_a(direct_call(*fixture.trace));terminate_own(230);}
        catch(const std::logic_error&){
            const auto outgoing=::GetLastError();const auto reads=fixture.trace->reads,resolves=fixture.trace->resolves,sdk=fixture.trace->sdk_calls;
            bool next_denied=false;try{(void)policy.original_folder_a(direct_call(*fixture.trace));}catch(const std::logic_error&){next_denied=true;}
            const bool resolver=fixture.trace->mode==Mode::ResolverNull||fixture.trace->mode==Mode::ResolverThrowNull;
            const auto slot=::FlsGetValue(fixture.trace->index);
            const bool prefix=resolver?fixture.trace->output[0]=='!':std::string(fixture.trace->output.data(),10)=="SDK-prefix"&&fixture.trace->output[11]=='!';
            const bool pointer=fixture.trace->mode==Mode::SDKSnapshot?slot==fixture.trace->expected_ptd:slot==nullptr;
            if(next_denied&&fixture.trace->mutated&&prefix&&pointer&&sdk==(resolver?0U:1U)&&resolves==(owned?1U:0U)&&
                fixture.trace->sdk_calls==sdk&&fixture.trace->resolves==resolves&&fixture.trace->reads==reads&&
                domain->active()&&!domain->ready()&&!domain->permits_current()&&outgoing==(resolver?0x6789U:0x789aU)&&
                fixture.heap.memory.blocks.size()==blocks&&fixture.heap.memory.frees==frees&&fixture.heap.memory.destroyed.size()==locks&&
                (!owned||!fixture.trace->code.expired()))terminate_own(expected);
            terminate_own(237);
        }catch(...){terminate_own(237);}
    });worker.join();return 230;
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal_child(argv[1]);
    try{
        for(bool owned:{true,false}){actual_native_factory(owned);exact_actual_permission_sequence(owned);}
        unsigned code=201;for(const auto* mode:{"--native-valid-throw","--sdk-null","--sdk-snapshot","--resolver-null","--resolver-throw-null","--raw-sdk-null"})
            require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==code++,"Own child reaches exact typed original/PTD/prefix/retention predicate; generic abort cannot pass");
        std::cout<<"PASS: "<<checks<<" original checkpoint integration checks; real FLS boundaries, native exception drain and retained invalid prefixes\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
