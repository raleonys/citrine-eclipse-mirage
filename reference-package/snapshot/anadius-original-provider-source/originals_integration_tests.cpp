#include "originals.hpp"
#include "../anadius-callback-provider-source/provider.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <array>
#include <condition_variable>
#include <csignal>
#include <process.h>

// Own source Context/native SDK shims, synthetic engine, heap and FLS only.
// No target DLL/game is loaded, no real hook engine operation is invoked.
namespace {
using namespace anadius;
namespace op=original_provider;namespace fc=foreign_callback;namespace n=native_recovered;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
[[noreturn]]void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
void generic_abort(int){terminate_own(239);}
template<class F>bool logic_rejected(F&& call){try{call();}catch(const std::logic_error&){return true;}return false;}
struct Code {
    std::shared_ptr<std::atomic<unsigned>> destroyed;
    explicit Code(std::shared_ptr<std::atomic<unsigned>> value):destroyed(std::move(value)){}
    ~Code(){++*destroyed;}
};
struct Trace {
    std::weak_ptr<Code> code;std::weak_ptr<n::OriginalProvider> provider;
    std::shared_ptr<std::atomic<unsigned>> destroyed=std::make_shared<std::atomic<unsigned>>(0);
    std::shared_ptr<fc::Domain> domain;
    n::OriginalSlots* slots=nullptr;DWORD index=FLS_OUT_OF_INDEXES;
    std::atomic<unsigned> originals=0,replacements=0,proofs=0;
    bool proof_saw_sealed=false,arguments=false,metadata_reentered=false;
    DWORD incoming=0;char* expected_output=nullptr;
    std::mutex gate;std::condition_variable changed;bool block=false,entered=false,release=false;
};
Trace* sdk_trace=nullptr;
HRESULT WINAPI own_folder_a(HWND window,int id,HANDLE token,DWORD flags,LPSTR out){
    auto* trace=sdk_trace;if(!trace||trace->code.expired())terminate_own(237);
    ++trace->originals;trace->incoming=::GetLastError();
    trace->arguments=window==reinterpret_cast<HWND>(0x11)&&id==7&&token==reinterpret_cast<HANDLE>(0x22)&&flags==0x33&&out==trace->expected_output;
    auto provider=trace->provider.lock();
    trace->metadata_reentered=provider&&provider->load(n::Callback::folder_a_00fc00)==reinterpret_cast<std::uintptr_t>(&own_folder_a)&&::GetLastError()==trace->incoming;
    {std::unique_lock lock(trace->gate);if(trace->block){trace->entered=true;trace->changed.notify_all();trace->changed.wait(lock,[&]{return trace->release;});}}
    if(out)std::memcpy(out,"own original",13);
    ::SetLastError(0x4321);return static_cast<HRESULT>(0x80070005U);
}
HRESULT WINAPI replacement_folder_a(HWND,int,HANDLE,DWORD,LPSTR){if(sdk_trace)++sdk_trace->replacements;return 77;}
struct Fixture {
    b18_test::Fixture base;std::shared_ptr<Trace> trace=std::make_shared<Trace>();
    Fixture(){base.driver.runtime.originals.fill(0);trace->slots=&base.driver.runtime.originals;trace->index=base.heap.domain->native_fls_index();sdk_trace=trace.get();}
    ~Fixture(){sdk_trace=nullptr;trace->domain.reset();}
    callback_policy::Resources resources(const std::shared_ptr<Code>& code){
        auto resources=base.resources();
        callback_provider::select(resources,"C",[](callback_provider::InvalidCall){throw std::logic_error("Unexpected synthetic invalid CRT input");},{callback_provider::Filesystem::Utf8,1252,437});
        resources.originals=[trace=trace,code](const n::OriginalSlots& slots){
            auto provider=op::bind(slots,code);trace->provider=provider;return provider;
        };
        return resources;
    }
    std::shared_ptr<fc::Domain> make_domain(bool require_sealed=true){
        auto domain=std::make_shared<fc::Domain>(base.heap.domain,[trace=trace,require_sealed]{
            ++trace->proofs;auto provider=trace->provider.lock();bool permitted=false;
            // Setup permits only controller loads. A successful foreign metadata
            // load here proves Owner sealed the provider before ready_proof.
            std::thread observer([&]{try{permitted=provider&&provider->load(n::Callback::folder_a_00fc00)==reinterpret_cast<std::uintptr_t>(&own_folder_a);}catch(...){}});observer.join();
            trace->proof_saw_sealed=permitted;return !require_sealed||permitted;
        });trace->domain=domain;return domain;
    }
};
Trace* terminal_trace=nullptr;unsigned terminal_code=0;
struct Errors final:n::Errors {
    [[noreturn]]void lifecycle(n::LifecycleFault)override{terminate_own(237);}
    [[noreturn]]void callback_exception(n::Callback callback,std::exception_ptr failure)noexcept override{
        auto* trace=terminal_trace;if(!trace||!trace->domain||callback!=n::Callback::folder_a_00fc00)terminate_own(237);
        const auto outgoing=::GetLastError();bool expected=false;
        try{std::rethrow_exception(failure);}catch(const fc::Rejected& error){expected=terminal_code==231&&error.fault==fc::Fault::Warming;}
        catch(const std::logic_error&){expected=terminal_code==233||terminal_code==234;}catch(...){}
        const auto current=trace->slots->at(host_init_recovered::slot(n::Callback::folder_a_00fc00));
        const bool slot=terminal_code==231?current==reinterpret_cast<std::uintptr_t>(&own_folder_a):
            terminal_code==233?current==reinterpret_cast<std::uintptr_t>(&replacement_folder_a):current==0;
        if(expected&&slot&&trace->originals==0&&trace->replacements==0&&!trace->code.expired()&&
            !trace->domain->active()&&trace->domain->ready()==(terminal_code!=231)&&
            ::FlsGetValue(trace->index)==nullptr&&n::active_calls()==0&&outgoing==0x9876)terminate_own(terminal_code);
        terminate_own(237);
    }
};
Errors errors;
void original_setup_publication_and_lifetime(){
    Fixture fixture;auto code=std::make_shared<Code>(fixture.trace->destroyed);fixture.trace->code=code;
    auto domain=fixture.make_domain();bool quiescent=false;
    auto factory=fc::make_factory(domain,b15_test::seed(),fixture.resources(code),errors,[&]{return quiescent;});
    auto copied=factory;factory={};code.reset();
    require(!fixture.trace->code.expired(),"Copied foreign factory retains explicit code lease before Context construction");
    auto attachment=copied(fixture.base.driver.view());copied={};
    require(!fixture.trace->code.expired()&&fixture.trace->provider.lock()->slots_identity()==fixture.trace->slots,"Stable Context policy binds exact Runtime slot array and retains code after factory release");
    attachment->publish();
    require(n::is_bound()&&fixture.trace->slots->at(8)==0,"Context can publish before host setup populates original slots");
    require(logic_rejected([&]{(void)fixture.trace->provider.lock()->load(n::Callback::folder_a_00fc00);}),"Created provider refuses original load even on controller before setup begin");
    attachment->host_setup_begin();fixture.trace->slots->at(8)=reinterpret_cast<std::uintptr_t>(&own_folder_a);
    std::array<char,260> controller_output{};fixture.trace->expected_output=controller_output.data();::SetLastError(0x1111);
    const auto controller_result=n::folder_a_entry(reinterpret_cast<HWND>(0x11),7,reinterpret_cast<HANDLE>(0x22),0x33,controller_output.data());
    require(controller_result==static_cast<HRESULT>(0x80070005U)&&std::string(controller_output.data())=="own original"&&::GetLastError()==0x4321,"Controller setup callback uses newly populated exact typed original and preserves HRESULT/output/LastError");
    require(fixture.trace->arguments&&fixture.trace->metadata_reentered&&fixture.trace->incoming==0x1111,"Original receives exact SDK arguments and can query provider metadata outside its mutex");
    require(!domain->ready()&&fixture.trace->originals==1,"Controller setup call does not implicitly publish foreign readiness");
    attachment->host_setup_complete();
    require(domain->ready()&&fixture.trace->proofs==1&&fixture.trace->proof_saw_sealed,"Owner seals original provider before foreign-domain ready proof executes");
    std::array<char,260> foreign_output{};fixture.trace->expected_output=foreign_output.data();fixture.trace->block=true;
    bool returned=false,clean=false;DWORD outgoing=0;
    std::thread worker([&]{::SetLastError(0x2222);const auto result=n::folder_a_entry(reinterpret_cast<HWND>(0x11),7,reinterpret_cast<HANDLE>(0x22),0x33,foreign_output.data());
        outgoing=::GetLastError();returned=result==static_cast<HRESULT>(0x80070005U)&&std::string(foreign_output.data())=="own original";
        clean=::FlsGetValue(fixture.trace->index)==nullptr;});
    {std::unique_lock lock(fixture.trace->gate);fixture.trace->changed.wait(lock,[&]{return fixture.trace->entered;});}
    const bool close_failed=!attachment->close_after_cleanup();
    const bool pause_failed=!attachment->pause_before_cleanup();
    const bool still_loads=fixture.trace->provider.lock()->load(n::Callback::folder_a_00fc00)==reinterpret_cast<std::uintptr_t>(&own_folder_a);
    require(close_failed&&pause_failed&&still_loads&&n::is_bound()&&!fixture.trace->code.expired(),"False quiescence close and active callback pause retain binding, live provider and code lease");
    {std::lock_guard lock(fixture.trace->gate);fixture.trace->release=true;fixture.trace->changed.notify_all();}worker.join();
    require(returned&&clean&&outgoing==0x4321&&fixture.trace->originals==2,"In-flight own SDK call returns exact prefix and drains transient foreign PTD after failed pause");
    require(fixture.trace->arguments&&fixture.trace->metadata_reentered&&fixture.trace->incoming==0x2222&&!domain->active(),"Ready foreign callback keeps exact arguments and actual admission ownership through original call");
    require(attachment->pause_before_cleanup()&&logic_rejected([&]{(void)fixture.trace->provider.lock()->load(n::Callback::folder_a_00fc00);}),"Successful native drain stops provider only after active call count reaches zero");
    require(!attachment->close_after_cleanup()&&n::is_bound()&&!fixture.trace->code.expired()&&*fixture.trace->destroyed==0,"Stopped provider still retains code and binding while external quiescence proof is false");
    quiescent=true;require(attachment->close_after_cleanup()&&!n::is_bound()&&!fixture.trace->code.expired(),"Successful binding close preserves explicit code lease through Context lifetime");
    attachment.reset();
    require(fixture.trace->code.expired()&&fixture.trace->provider.expired()&&*fixture.trace->destroyed==1,"Final Context destruction releases code owner exactly once after shutdown");
}
void failed_hook_prefix_is_not_ready_proof(){
    Fixture fixture;auto code=std::make_shared<Code>(fixture.trace->destroyed);fixture.trace->code=code;
    auto domain=std::make_shared<fc::Domain>(fixture.base.heap.domain,[&]{return !fixture.base.driver.runtime.enabled_targets.empty();});fixture.trace->domain=domain;
    auto factory=fc::make_factory(domain,b15_test::seed(),fixture.resources(code),errors,[]{return true;});
    auto attachment=factory(fixture.base.driver.view());factory={};code.reset();attachment->publish();attachment->host_setup_begin();
    b14_test::Graph host;host.engine->create_results[0]=8;
    host_init_recovered::install_hook_00f120({fixture.base.driver.configuration,fixture.base.driver.runtime.registry_values,fixture.base.driver.runtime.enabled_targets,fixture.base.driver.runtime.originals},
        *host.host,u"shell32.dll","SHGetFolderPathA",n::Callback::folder_a_00fc00);
    require(host.engine->targets.size()==1&&host.engine->enabled.empty()&&fixture.base.driver.runtime.enabled_targets.empty(),"Controlled create failure8 skips enable and enabled-target publication");
    const auto prefix=fixture.trace->slots->at(8);
    require(prefix!=0&&prefix==host.engine->targets[0]+0x100000,"Controlled failed create still transfers nonzero original-output prefix");
    attachment->host_setup_complete();
    require(fixture.trace->provider.lock()->load(n::Callback::folder_a_00fc00)==prefix&&!domain->ready(),"Sealed nonzero original metadata does not replace independent failed-hook readiness proof");
    bool warming=false;fc::Admission probe(domain);
    std::thread worker([&]{try{probe.enter(n::Callback::folder_a_00fc00);}catch(const fc::Rejected& error){warming=error.fault==fc::Fault::Warming;}});worker.join();
    require(warming&&!domain->active()&&fixture.trace->originals==0,"Foreign admission rejects failed-hook prefix without executing its synthetic address");
    require(attachment->pause_before_cleanup()&&attachment->close_after_cleanup(),"Failed-hook fixture can close under separately supplied external quiescence");
    attachment.reset();
    require(fixture.trace->code.expired()&&*fixture.trace->destroyed==1,"Failed-hook metadata owner is released only after explicit Context shutdown");
}
int terminal_child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,generic_abort);
    Fixture fixture;auto code=std::make_shared<Code>(fixture.trace->destroyed);fixture.trace->code=code;auto domain=fixture.make_domain();
    auto factory=fc::make_factory(domain,b15_test::seed(),fixture.resources(code),errors,[]{return true;});
    auto attachment=factory(fixture.base.driver.view());factory={};code.reset();attachment->publish();attachment->host_setup_begin();
    fixture.trace->slots->at(8)=reinterpret_cast<std::uintptr_t>(&own_folder_a);
    if(mode=="--warming")terminal_code=231;
    else{
        attachment->host_setup_complete();
        if(mode=="--changed"){terminal_code=233;fixture.trace->slots->at(8)=reinterpret_cast<std::uintptr_t>(&replacement_folder_a);}
        else if(mode=="--zero"){terminal_code=234;fixture.trace->slots->at(8)=0;}
        else return 238;
    }
    terminal_trace=fixture.trace.get();std::array<char,260> out{};fixture.trace->expected_output=out.data();
    std::thread worker([&]{::SetLastError(0x9876);(void)n::folder_a_entry(reinterpret_cast<HWND>(0x11),7,reinterpret_cast<HANDLE>(0x22),0x33,out.data());terminate_own(230);});worker.join();return 230;
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal_child(argv[1]);
    try{
        original_setup_publication_and_lifetime();failed_hook_prefix_is_not_ready_proof();
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--warming",static_cast<const char*>(nullptr))==231,"Own native child proves foreign setup rejection before original stub execution");
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--changed",static_cast<const char*>(nullptr))==233,"Own native child rejects changed sealed nonzero slot before replacement execution and after admission cleanup");
        require(_spawnl(_P_WAIT,argv[0],argv[0],"--zero",static_cast<const char*>(nullptr))==234,"Own native child rejects cleared sealed slot without stub call and drains PTD/binding lease");
        std::cout<<"PASS: "<<checks<<" original provider integration checks; delayed setup, exact native originals, code lifetime and sealed-slot rejection\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
