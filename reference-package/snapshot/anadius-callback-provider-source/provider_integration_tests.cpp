#include "provider.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <array>
#include <cerrno>
#include <csignal>
#include <process.h>

// Synthetic source fixtures, own heap/FLS and native callback entry shims only.
// No original/rebuilt DLL, game, actual user configuration or network is opened.
namespace {
using namespace anadius;
namespace cp=callback_policy;namespace provider=callback_provider;
namespace fc=foreign_callback;namespace n=native_recovered;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
[[noreturn]]void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
void generic_abort(int){terminate_own(239);}
struct Failure {unsigned identity;};
struct Life {unsigned invalid_calls=0;};
struct Trace {
    DWORD index=FLS_OUT_OF_INDEXES;std::atomic<unsigned> reads=0;
    unsigned compare_a=0,compare_w=0,copy_a=0,copy_w=0;
    unsigned queries=0,fills=0;std::vector<std::uint32_t> codepages,flags;
    std::vector<bool> used_default;
    wchar_t* borrowed=nullptr;bool cleared=false,tail=false,mutated=false;
    std::exception_ptr failure;fc::Domain* domain=nullptr;std::weak_ptr<Life> life;
};
inline std::atomic<Trace*> current_trace=nullptr;
PVOID WINAPI observed_get(DWORD index){
    if(auto* trace=current_trace.load();trace&&trace->index==index)++trace->reads;
    return ::FlsGetValue(index);
}
class ObservedCrt final:public cp::CrtProvider {
    std::shared_ptr<cp::CrtProvider> lower_;std::shared_ptr<Trace> trace_;
public:
    ObservedCrt(std::shared_ptr<cp::CrtProvider> lower,std::shared_ptr<Trace> trace):lower_(std::move(lower)),trace_(std::move(trace)){}
    int compare_a(const char* a,const char* b)override{++trace_->compare_a;return lower_->compare_a(a,b);}
    int compare_w(const wchar_t* a,const wchar_t* b)override{++trace_->compare_w;return lower_->compare_w(a,b);}
    int copy_a(char* a,std::size_t n,const char* b)override{++trace_->copy_a;return lower_->copy_a(a,n,b);}
    int copy_w(wchar_t* a,std::size_t n,const wchar_t* b)override{++trace_->copy_w;return lower_->copy_w(a,n,b);}
};
class ObservedEncoding final:public encoding_recovered::Calls {
    encoding_recovered::NativeCalls lower_;std::shared_ptr<Trace> trace_;
public:
    explicit ObservedEncoding(std::shared_ptr<Trace> trace):trace_(std::move(trace)){}
    std::uint32_t locale_codepage_0629ec()override{throw std::logic_error("Ambient CRT locale must not be consulted");}
    bool are_file_apis_ansi()override{throw std::logic_error("Ambient file mode must not be consulted");}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* in,int count,char16_t* out,int capacity)override{return lower_.multi_byte_to_wide(cp,flags,in,count,out,capacity);}
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* in,int count,char* out,int capacity,bool* used)override{
        trace_->codepages.push_back(cp);trace_->flags.push_back(flags);trace_->used_default.push_back(used!=nullptr);
        if(out)++trace_->fills;else ++trace_->queries;
        return lower_.wide_to_multi_byte(cp,flags,in,count,out,capacity,used);
    }
    std::uint32_t last_error()override{return lower_.last_error();}
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
        if(!heap.domain->initialize())throw std::runtime_error("Own callback provider PTD init");
        memory=heap.domain->make_memory();trace->index=heap.domain->native_fls_index();current_trace.store(trace.get());
        driver.runtime.registry_values={{"Documents",u"C:/caf\u00e9"},{"ClientPath",u"C:/own/target.exe"}};
    }
    ~Fixture(){current_trace.store(nullptr);memory.reset();}
    cp::Resources resources(provider::InvalidHandler handler){
        cp::Resources resources;resources.memory=memory;resources.stream=stream;
        resources.owned_crt=std::make_shared<ObservedCrt>(provider::make_crt("C",std::move(handler)),trace);
        resources.encoding=provider::make_encoding({provider::Filesystem::Ansi,1252,437},std::make_unique<ObservedEncoding>(trace));
        return resources;
    }
};
Trace* terminal_trace=nullptr;
struct EntryErrors final:n::Errors {
    [[noreturn]]void lifecycle(n::LifecycleFault)override{terminate_own(237);}
    [[noreturn]]void callback_exception(n::Callback callback,std::exception_ptr failure)noexcept override{
        auto* trace=terminal_trace;
        const auto outgoing=::GetLastError();
        if(trace&&trace->domain&&callback==n::Callback::folder_w_00fc80&&failure==trace->failure&&
            trace->cleared&&trace->tail&&trace->copy_w==1&&trace->life.lock()&&trace->life.lock()->invalid_calls==1&&
            !trace->domain->active()&&trace->domain->ready()&&::FlsGetValue(trace->index)==nullptr&&
            n::active_calls()==0&&outgoing==0x7777)terminate_own(221);
        terminate_own(237);
    }
};
EntryErrors errors;
void foreign_native_routes_and_factory_lifetime(){
    Fixture fixture;auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});fixture.trace->domain=domain.get();
    auto life=std::make_shared<Life>();std::weak_ptr<Life> weak=life;
    auto resources=fixture.resources([life](provider::InvalidCall){++life->invalid_calls;throw std::logic_error("Unexpected invalid source input");});
    auto factory=fc::make_factory(domain,b15_test::seed(),std::move(resources),errors,[]{return true;});
    auto copied=factory;factory={};life.reset();
    require(!weak.expired(),"Copied attachment factory retains owned invalid-handler context before consumption");
    auto attachment=copied(fixture.driver.view());copied={};
    require(!weak.expired(),"Context retains compiler locale and invalid handler after all factory copies are released");
    attachment->publish();attachment->host_setup_begin();attachment->host_setup_complete();
    bool a=false,w=false,pa=false,pw=false,clean=false;std::exception_ptr failure;
    const auto locale_refs=fixture.heap.domain->locale_references();
    std::thread worker([&]{try{
        std::array<char,260> path_a{};std::array<wchar_t,260> path_w{};
        a=n::folder_a_entry(nullptr,5,nullptr,0,path_a.data())==0&&std::string(path_a.data())==std::string("C:/caf\xe9",7);
        w=n::folder_w_entry(nullptr,5,nullptr,0,path_w.data())==0&&std::wstring(path_w.data())==L"C:/caf\u00e9";
        STARTUPINFOA sa{};STARTUPINFOW sw{};PROCESS_INFORMATION process{};
        pa=n::process_a_entry("c:/OWN/TARGET.exe",nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,&sa,&process)==0;
        pw=n::process_w_entry(L"c:/OWN/TARGET.exe",nullptr,nullptr,nullptr,FALSE,0,nullptr,nullptr,&sw,&process)==0;
        clean=::FlsGetValue(fixture.trace->index)==nullptr;
    }catch(...){failure=std::current_exception();}});worker.join();if(failure)std::rethrow_exception(failure);
    require(a&&w,"Actual A/W native folder shims use owned byte and wide copy providers on a foreign thread");
    require(pa&&pw,"Actual A/W process matching uses explicit compiler locale without original process invocation");
    require(fixture.trace->copy_a==1&&fixture.trace->copy_w==1&&fixture.trace->compare_a==1&&fixture.trace->compare_w==1,"All four CRT virtual routes are wired through actual foreign factory");
    require(fixture.trace->queries==2&&fixture.trace->fills==2,"Folder and narrow comparison each perform exact numerical query/fill conversion");
    require(fixture.trace->codepages==std::vector<std::uint32_t>(4,1252)&&fixture.trace->flags==std::vector<std::uint32_t>(4,0x400)&&fixture.trace->used_default==std::vector<bool>(4,true),"Frozen ANSI alias resolves numerically to1252 while recovered flags and used-default arguments stay intact");
    require(clean&&!domain->active()&&domain->ready(),"Native success drains each actual temporary PTD and preserves ready publication");
    require(fixture.heap.domain->locale_references()==locale_refs&&!fixture.memory->outstanding_allocations(),"Compiler locale owner neither adopts recovered locale references nor leaks application memory");
    require(attachment->pause_before_cleanup()&&attachment->close_after_cleanup(),"Separate explicit source quiescence closes native binding");
    attachment.reset();
    require(weak.expired()&&!n::is_bound(),"Last Context owner releases invalid handler and locale after shutdown");
}
void admitted_wide_exception_prefix(){
    Fixture fixture;auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});
    fc::Admission admission(domain);admission.host_setup_begin();admission.host_setup_complete();
    std::array<wchar_t,4> out{L'!',L'!',L'!',L'!'};unsigned notifications=0;
    const auto expected=std::make_exception_ptr(Failure{22});
    auto resources=fixture.resources([&](provider::InvalidCall call){
        ++notifications;fixture.trace->cleared=out[0]==0;fixture.trace->tail=out[1]==L'!';
        if(call.operation!=provider::Operation::CopyW||call.reason!=provider::InvalidReason::Range||call.error!=ERANGE)throw std::logic_error("Wrong owned invalid notification");
        ::SetLastError(0x5555);std::rethrow_exception(expected);
    });
    resources.callback_scope=[domain]{return domain->permits_current();};
    resources.owned_crt=provider::check_crt(resources.crt,std::move(resources.owned_crt),resources.callback_scope);
    auto seed=b15_test::seed();cp::Policy policy({fixture.driver.view(),seed},std::move(resources));
    std::exception_ptr worker_failure;
    std::thread worker([&]{try{
        admission.enter(n::Callback::folder_w_00fc80);
        const auto before=fixture.trace->reads.load();std::exception_ptr observed;
        try{(void)policy.crt_wcscpy_s(reinterpret_cast<char16_t*>(out.data()),2,u"long");}catch(...){observed=std::current_exception();}
        require(observed==expected&&notifications==1,"Valid actual PTD postcondition retains identical owned invalid-handler exception without retry");
        require(fixture.trace->reads==before+2,"Inherited wide final route receives exact actual FLS pre/post checks");
        require(fixture.trace->cleared&&fixture.trace->tail&&out[0]==0&&out[1]==L'!',"Actual borrowed UTF16 first unit is cleared before throwing handler while unused tail is untouched");
        require(::GetLastError()==0x5555,"ByteWide staging and exceptional checks retain provider outgoing WinLastError");
        const auto compared=fixture.trace->reads.load();
        require(policy.crt_wcsicmp(u"OWN",u"own")==0&&fixture.trace->reads==compared+2,"Same admitted scope can use owned wide comparison after a valid provider exception");
        admission.leave(n::Callback::folder_w_00fc80);
        require(!domain->active()&&::FlsGetValue(fixture.trace->index)==nullptr,"Explicit known cleanup releases fresh PTD after handled exception");
    }catch(...){worker_failure=std::current_exception();}});worker.join();if(worker_failure)std::rethrow_exception(worker_failure);
    require(domain->pause(),"Handled provider exception leaves domain eligible for normal external shutdown");
}
int terminal_child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,generic_abort);
    Fixture fixture;auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});fixture.trace->domain=domain.get();
    if(mode=="--native-invalid"){
        fixture.driver.runtime.registry_values["Documents"]=std::u16string(300,u'Q');
        std::array<wchar_t,260> output;output.fill(L'!');fixture.trace->borrowed=output.data();
        fixture.trace->failure=std::make_exception_ptr(Failure{221});terminal_trace=fixture.trace.get();
        auto life=std::make_shared<Life>();fixture.trace->life=life;
        auto resources=fixture.resources([trace=fixture.trace,life](provider::InvalidCall call){
            ++life->invalid_calls;trace->cleared=trace->borrowed[0]==0;trace->tail=trace->borrowed[1]==L'!';
            if(call.operation!=provider::Operation::CopyW||call.reason!=provider::InvalidReason::Range)terminate_own(237);
            ::SetLastError(0x7777);std::rethrow_exception(trace->failure);
        });life.reset();
        auto factory=fc::make_factory(domain,b15_test::seed(),std::move(resources),errors,[]{return true;});
        auto attachment=factory(fixture.driver.view());factory={};attachment->publish();attachment->host_setup_begin();attachment->host_setup_complete();
        std::thread worker([&]{(void)n::folder_w_entry(nullptr,5,nullptr,0,output.data());terminate_own(230);});worker.join();return 230;
    }
    if(mode!="--mutated-handler")return 238;
    fc::Admission admission(domain);admission.host_setup_begin();admission.host_setup_complete();
    std::array<wchar_t,4> output{L'!',L'!',L'!',L'!'};
    auto resources=fixture.resources([&](provider::InvalidCall call){
        fixture.trace->cleared=output[0]==0;fixture.trace->tail=output[1]==L'!';
        if(call.operation!=provider::Operation::CopyW||call.reason!=provider::InvalidReason::Range||!::FlsSetValue(fixture.trace->index,nullptr))terminate_own(237);
        fixture.trace->mutated=true;::SetLastError(0x6666);
    });
    resources.callback_scope=[domain]{return domain->permits_current();};
    resources.owned_crt=provider::check_crt(resources.crt,std::move(resources.owned_crt),resources.callback_scope);
    auto seed=b15_test::seed();cp::Policy policy({fixture.driver.view(),seed},std::move(resources));
    std::thread worker([&]{
        admission.enter(n::Callback::folder_w_00fc80);
        try{(void)policy.crt_wcscpy_s(reinterpret_cast<char16_t*>(output.data()),2,u"long");terminate_own(230);}
        catch(const std::logic_error&){
            bool next_denied=false;try{(void)policy.crt_wcsicmp(u"a",u"A");}catch(const std::logic_error&){next_denied=true;}
            const auto outgoing=::GetLastError();
            if(next_denied&&fixture.trace->copy_w==1&&fixture.trace->compare_w==0&&fixture.trace->mutated&&
                fixture.trace->cleared&&fixture.trace->tail&&output[0]==0&&output[1]==L'!'&&domain->active()&&!domain->ready()&&
                !domain->permits_current()&&::FlsGetValue(fixture.trace->index)==nullptr&&outgoing==0x6666)terminate_own(222);
            terminate_own(237);
        }catch(...){terminate_own(237);}
    });worker.join();return 230;
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal_child(argv[1]);
    try{
        foreign_native_routes_and_factory_lifetime();admitted_wide_exception_prefix();
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--native-invalid",static_cast<const char*>(nullptr))==221,"Own native child proves identical provider exception only after actual PTD and binding lease drain");
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--mutated-handler",static_cast<const char*>(nullptr))==222,"Own poisoned child proves retained cleared prefix and rejection before next lower CRT call");
        std::cout<<"PASS: "<<checks<<" callback provider integration checks; actual foreign factory, owned context, numerical encoding and PTD boundaries\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
