#include "identity.hpp"
#include "../anadius-foreign-callback-source/foreign_test_fixture.hpp"
#include <algorithm>
#include <array>
#include <csignal>
#include <exception>
#include <functional>
#include <iostream>
#include <process.h>
#include <string>
#include <thread>
#include <vector>

// Only own source objects, heap/FLS slots and synthetic provider inputs. No
// original DLL, user configuration, actual socket, module loading or UI.
namespace {
using namespace anadius;
namespace ci=callback_identity;
namespace fc=foreign_callback;
namespace n=native_recovered;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value)throw std::runtime_error(message);}
[[noreturn]]void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
void generic_abort(int){terminate_own(239);}
enum class Step {Locale,Ansi,NarrowQuery,NarrowFill,WideQuery,WideFill,Error};
struct LowerFault {const void* identity;};
struct Event {
    Step step;std::uint32_t cp=0,flags=0,error=0;int count=0,capacity=0;
    const void* input=nullptr;void* output=nullptr;bool* used=nullptr;
    unsigned probes=0;std::string bytes;std::u16string wide;
    Event(Step value):step(value){}
};
struct Trace {
    std::uint32_t locale=65001,api_error=1113,outgoing_error=0x2222;
    bool ansi=true,replaced=false,short_fill=false;
    DWORD index=FLS_OUT_OF_INDEXES;unsigned probes=0,original_calls=0;
    bool initial_armed=false,initial_mutated=false,provider_mutated=false;
    std::vector<Event> events;std::string written_prefix;
    std::function<void(Step)> after;
    std::exception_ptr failure;
    fc::Domain* domain=nullptr;
};
thread_local Trace* sdk_trace=nullptr;
HRESULT WINAPI own_original_folder_a(HWND,int,HANDLE,DWORD,LPSTR){
    if(!sdk_trace)terminate_own(237);++sdk_trace->original_calls;return 0;
}
PVOID WINAPI observed_get(DWORD index){
    auto* value=::FlsGetValue(index);
    if(sdk_trace&&index==sdk_trace->index){
        ++sdk_trace->probes;
        if(sdk_trace->initial_armed&&value&&value!=crt_ptd_recovered::construction_sentinel()){
            sdk_trace->initial_armed=false;
            if(!::FlsSetValue(index,nullptr))terminate_own(236);
            sdk_trace->initial_mutated=true;return nullptr;
        }
    }
    return value;
}
unsigned count(const Trace& trace,Step step){return static_cast<unsigned>(std::count_if(trace.events.begin(),trace.events.end(),[&](const Event& event){return event.step==step;}));}
class ProbeEncoding final:public encoding_recovered::Calls {
    std::shared_ptr<Trace> trace_;
    void record(Event event){
        event.error=::GetLastError();event.probes=trace_->probes;trace_->events.push_back(std::move(event));
        ::SetLastError(trace_->outgoing_error);
    }
    void after(Step step){if(trace_->after)trace_->after(step);}
public:
    explicit ProbeEncoding(std::shared_ptr<Trace> trace):trace_(std::move(trace)){}
    std::uint32_t locale_codepage_0629ec()override{record({Step::Locale});after(Step::Locale);return trace_->locale;}
    bool are_file_apis_ansi()override{record({Step::Ansi});after(Step::Ansi);return trace_->ansi;}
    int multi_byte_to_wide(std::uint32_t cp,std::uint32_t flags,const char* input,int size,char16_t* output,int capacity)override{
        const auto step=output?Step::NarrowFill:Step::NarrowQuery;
        Event event{step};event.cp=cp;event.flags=flags;event.input=input;event.output=output;event.count=size;event.capacity=capacity;
        if(input&&size>0)event.bytes.assign(input,input+size);record(std::move(event));
        if(output){if(capacity<size)throw std::logic_error("own narrow output capacity");for(int i=0;i<size;++i)output[i]=static_cast<unsigned char>(input[i]);}
        after(step);return size;
    }
    int wide_to_multi_byte(std::uint32_t cp,std::uint32_t flags,const char16_t* input,int size,char* output,int capacity,bool* used)override{
        const auto step=output?Step::WideFill:Step::WideQuery;
        Event event{step};event.cp=cp;event.flags=flags;event.input=input;event.output=output;event.count=size;event.capacity=capacity;event.used=used;
        if(input&&size>0)event.wide.assign(input,input+size);record(std::move(event));
        if(used)*used=trace_->replaced;
        if(output){
            if(capacity<size)throw std::logic_error("own wide output capacity");
            const auto amount=trace_->short_fill?1:size;
            for(int i=0;i<amount;++i)output[i]=trace_->short_fill?'Z':static_cast<char>(input[i]);
            trace_->written_prefix.assign(output,output+amount);
        }
        after(step);return output&&trace_->short_fill?1:size;
    }
    std::uint32_t last_error()override{record({Step::Error});after(Step::Error);return trace_->api_error;}
};

void checked_provider_forwarding(){
    auto trace=std::make_shared<Trace>();trace->ansi=false;trace->replaced=true;
    unsigned permissions=0;
    ci::CheckedEncoding selected(std::make_unique<ProbeEncoding>(trace),[&]{++permissions;::SetLastError(0xbeef);return true;});
    ::SetLastError(0x1111);
    require(selected.locale_codepage_0629ec()==65001&&::GetLastError()==0x2222&&trace->events.back().error==0x1111,"locale result and body LastError survive both permission checks");
    ::SetLastError(0x1112);
    require(!selected.are_file_apis_ansi()&&::GetLastError()==0x2222&&trace->events.back().error==0x1112,"ANSI selector forwards false and preserves incoming/outgoing LastError");
    const char bytes[]{'A',0,'B'};const char16_t wide[]{u'A',0,u'B'};
    std::array<char16_t,5> target_w{u'!',u'!',u'!',u'!',u'!'};
    std::array<char,5> target_a{'!','!','!','!','!'};bool replaced=false;
    require(selected.multi_byte_to_wide(65001,8,bytes,3,nullptr,0)==3,"checked native-width query preserves explicit count");
    require(selected.multi_byte_to_wide(65001,8,bytes,3,target_w.data(),3)==3,"checked native-width fill preserves result");
    const auto narrow=trace->events.back();
    require(narrow.input==bytes&&narrow.output==target_w.data()&&narrow.cp==65001&&narrow.flags==8&&narrow.count==3&&narrow.capacity==3&&narrow.bytes==std::string(bytes,3),"narrow conversion pointers, codepage, flags, count, capacity and embedded NUL pass unchanged");
    require(target_w[0]==u'A'&&target_w[1]==0&&target_w[2]==u'B'&&target_w[3]==u'!',"checked native-width fill preserves borrowed output and unused tail");
    require(selected.wide_to_multi_byte(1252,0x400,wide,3,nullptr,0,&replaced)==3&&replaced,"checked byte query forwards used-default output");
    replaced=false;
    require(selected.wide_to_multi_byte(1252,0x400,wide,3,target_a.data(),3,&replaced)==3&&replaced,"checked byte fill forwards used-default output");
    const auto wide_event=trace->events.back();
    require(wide_event.input==wide&&wide_event.output==target_a.data()&&wide_event.used==&replaced&&wide_event.cp==1252&&wide_event.flags==0x400&&wide_event.count==3&&wide_event.capacity==3&&wide_event.wide==std::u16string(wide,3),"wide conversion preserves every argument and counted UTF16 data");
    require(std::string(target_a.data(),3)==std::string(bytes,3)&&target_a[3]=='!',"checked byte fill preserves NUL and unused output tail");
    ::SetLastError(0x1113);
    require(selected.last_error()==1113&&::GetLastError()==0x2222&&trace->events.back().error==0x1113,"last_error result differs from and does not overwrite lower outgoing WinLastError");
    require(permissions==trace->events.size()*2&&trace->events.size()==7,"all five Calls virtual routes have one precheck and one postcheck per lower invocation");
}

void exceptions_and_output_prefix(){
    {
        auto trace=std::make_shared<Trace>();trace->failure=std::make_exception_ptr(LowerFault{trace.get()});
        trace->after=[trace](Step){std::rethrow_exception(trace->failure);};
        unsigned permissions=0;
        ci::CheckedEncoding selected(std::make_unique<ProbeEncoding>(trace),[&]{++permissions;::SetLastError(0xbeef);return true;});
        std::exception_ptr observed;::SetLastError(0x1114);
        try{(void)selected.locale_codepage_0629ec();}catch(...){observed=std::current_exception();}
        require(observed==trace->failure&&permissions==2&&trace->events.size()==1,"valid postcondition rethrows the identical lower exception without retry");
        require(::GetLastError()==0x2222,"valid exceptional postcheck preserves lower outgoing WinLastError");
        trace->after={};
    }
    {
        auto trace=std::make_shared<Trace>();trace->failure=std::make_exception_ptr(LowerFault{trace.get()});
        bool permitted=true;unsigned permissions=0;
        trace->after=[&](Step){permitted=false;std::rethrow_exception(trace->failure);};
        ci::CheckedEncoding selected(std::make_unique<ProbeEncoding>(trace),[&]{++permissions;::SetLastError(0xbeef);return permitted;});
        const char16_t input[]{u'A',0,u'B'};std::array<char,5> output{'!','!','!','!','!'};
        bool logic=false;try{(void)selected.wide_to_multi_byte(65001,0x80,input,3,output.data(),3,nullptr);}catch(const std::logic_error&){logic=true;}
        require(logic&&std::string(output.data(),3)==std::string("A\0B",3)&&output[3]=='!',"invalid postcondition takes precedence over lower exception without rolling back output prefix");
        require(permissions==2&&trace->events.size()==1&&::GetLastError()==0x2222,"failed postcondition does not retry provider or alter body WinLastError");
        bool denied=false;try{(void)selected.locale_codepage_0629ec();}catch(const std::logic_error&){denied=true;}
        require(denied&&trace->events.size()==1&&permissions==3,"subsequent invalid precondition prevents any next lower invocation");
    }
}

struct Fixture {
    b6_core_test::Fixture heap;b15_test::Driver driver;
    std::shared_ptr<thread_domain::ApplicationMemory> memory;
    std::shared_ptr<std::ostringstream> output=std::make_shared<std::ostringstream>();
    Fixture(){
        heap.domain.reset();auto apis=b6_core_test::apis();
        apis.heap={b6_core_test::get_heap,b16_test::selected_alloc,::HeapReAlloc,b16_test::selected_free,::HeapSize,::GetLastError};
        apis.fls.get=observed_get;heap.domain=thread_domain::DomainStorage::create(b6_core_test::seed(),apis);
        if(!heap.domain->initialize())throw std::runtime_error("own checked-provider domain initialize");memory=heap.domain->make_memory();
        driver.runtime.registry_values={{"Documents",u"C:/own/docs"}};
        driver.runtime.originals[host_init_recovered::slot(n::Callback::folder_a_00fc00)]=reinterpret_cast<std::uintptr_t>(&own_original_folder_a);
    }
    ~Fixture(){sdk_trace=nullptr;memory.reset();}
    callback_policy::Resources resources(const std::shared_ptr<Trace>& trace){
        callback_policy::Resources result;result.memory=memory;result.stream=output;result.encoding=std::make_unique<ProbeEncoding>(trace);return result;
    }
};
Trace* terminal_trace=nullptr;
unsigned terminal_mode=0;
struct Errors final:n::Errors {
    [[noreturn]]void lifecycle(n::LifecycleFault)override{terminate_own(237);}
    [[noreturn]]void callback_exception(n::Callback callback,std::exception_ptr failure)noexcept override{
        auto* trace=terminal_trace;
        if(!trace||!trace->domain||callback!=n::Callback::folder_a_00fc00)terminate_own(237);
        try{
            if(terminal_mode==211){
                bool logic=false;try{std::rethrow_exception(failure);}catch(const std::logic_error&){logic=true;}catch(...){}
                if(logic&&trace->initial_mutated&&trace->events.empty()&&trace->original_calls==0&&trace->domain->active()&&!trace->domain->ready()&&
                    !trace->domain->permits_current()&&::FlsGetValue(trace->index)==nullptr&&n::active_calls()==0)terminate_own(211);
            }else if(terminal_mode==215){
                if(failure==trace->failure&&count(*trace,Step::Locale)==1&&count(*trace,Step::WideQuery)==1&&count(*trace,Step::WideFill)==0&&
                    !trace->domain->active()&&trace->domain->ready()&&::FlsGetValue(trace->index)==nullptr&&n::active_calls()==0)terminate_own(215);
            }
        }catch(...){}
        terminate_own(237);
    }
};
Errors errors;
struct LiveFactory {
    Fixture fixture;std::shared_ptr<Trace> trace=std::make_shared<Trace>();
    std::shared_ptr<fc::Domain> domain;
    std::unique_ptr<bootstrap_backend::CallbackAttachment> attachment;
    LiveFactory(){
        domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});trace->domain=domain.get();trace->index=fixture.heap.domain->native_fls_index();
        auto factory=fc::make_factory(domain,b15_test::seed(),fixture.resources(trace),errors,[]{return true;});
        attachment=factory(fixture.driver.view());attachment->publish();attachment->host_setup_begin();attachment->host_setup_complete();
    }
    ~LiveFactory(){
        if(attachment){if(!attachment->pause_before_cleanup()||!attachment->close_after_cleanup())std::abort();attachment.reset();}
        domain.reset();
    }
};
void actual_foreign_factory_checks_encoding(){
    LiveFactory live;bool result=false,slot_clean=false;std::exception_ptr failure;
    const auto references=live.fixture.heap.domain->locale_references();
    std::thread worker([&]{
        sdk_trace=live.trace.get();
        try{
            std::array<char,260> path{};
            result=n::folder_a_entry(nullptr,5,nullptr,0,path.data())==0&&std::string(path.data())=="C:/own/docs";
            slot_clean=::FlsGetValue(live.trace->index)==nullptr;
        }catch(...){failure=std::current_exception();}
        sdk_trace=nullptr;
    });worker.join();if(failure)std::rethrow_exception(failure);
    require(result&&slot_clean&&!live.domain->active()&&live.domain->ready(),"actual foreign make_factory/native folder path returns own source value and drains transient PTD");
    require(live.trace->events.size()==3&&count(*live.trace,Step::Locale)==1&&count(*live.trace,Step::WideQuery)==1&&count(*live.trace,Step::WideFill)==1,"actual factory preserves UTF8 locale/query/fill route without extra lower calls");
    const auto& locale=live.trace->events[0];const auto& query=live.trace->events[1];const auto& fill=live.trace->events[2];
    require(query.probes==locale.probes+2&&fill.probes==query.probes+2,"factory automatically checks actual domain FLS identity before and after each lower call");
    require(query.cp==65001&&query.flags==0x80&&!query.output&&!query.used&&fill.cp==65001&&fill.flags==0x80&&fill.output&&!fill.used&&fill.capacity==11,"actual Policy encoding route retains strict UTF8 flags, query/fill capacity and no used-default pointer");
    require(live.fixture.heap.domain->locale_references()==references&&live.fixture.memory->outstanding_allocations()==0,"successful owned callback preserves recovered locale references and allocation ledger");
}

int terminal_child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,generic_abort);
    if(mode=="--initial"||mode=="--native-lower-throw"){
        LiveFactory live;terminal_trace=live.trace.get();
        if(mode=="--initial"){terminal_mode=211;live.trace->initial_armed=true;}
        else{
            terminal_mode=215;live.trace->failure=std::make_exception_ptr(LowerFault{live.trace.get()});
            live.trace->after=[&](Step step){if(step==Step::WideQuery)std::rethrow_exception(live.trace->failure);};
        }
        std::thread worker([&]{sdk_trace=live.trace.get();std::array<char,260> path{};
            // id7 would immediately reach the own original if native body began.
            (void)n::folder_a_entry(nullptr,terminal_mode==211?7:5,nullptr,0,path.data());terminate_own(230);});
        worker.join();return 230;
    }
    const auto target=mode=="--locale"?Step::Locale:mode=="--query"?Step::WideQuery:Step::WideFill;
    const unsigned expected=mode=="--locale"?212:mode=="--query"?213:214;
    Fixture fixture;auto trace=std::make_shared<Trace>();trace->index=fixture.heap.domain->native_fls_index();
    auto domain=std::make_shared<fc::Domain>(fixture.heap.domain,[]{return true;});trace->domain=domain.get();
    fc::Admission admission(domain);admission.host_setup_begin();admission.host_setup_complete();
    auto resources=fixture.resources(trace);resources.callback_scope=[domain]{return domain->permits_current();};
    resources.encoding=ci::check_encoding(std::move(resources.encoding),resources.callback_scope);
    auto seed=b15_test::seed();callback_policy::Policy policy({fixture.driver.view(),seed},std::move(resources));
    trace->after=[&](Step step){if(step==target){if(!::FlsSetValue(trace->index,nullptr))terminate_own(236);trace->provider_mutated=true;}};
    std::thread worker([&]{
        sdk_trace=trace.get();admission.enter(n::Callback::folder_a_00fc00);
        try{(void)policy.filesystem_narrow(std::u16string_view(u"A\0B",3));terminate_own(230);}
        catch(const std::logic_error&){
            const bool stopped=target==Step::Locale?trace->events.size()==1&&count(*trace,Step::WideQuery)==0:
                target==Step::WideQuery?count(*trace,Step::WideQuery)==1&&count(*trace,Step::WideFill)==0:
                count(*trace,Step::WideFill)==1&&trace->written_prefix==std::string("A\0B",3);
            if(stopped&&trace->provider_mutated&&domain->active()&&!domain->ready()&&!domain->permits_current()&&::FlsGetValue(trace->index)==nullptr)terminate_own(expected);
            terminate_own(237);
        }catch(...){terminate_own(237);}
    });worker.join();return 230;
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal_child(argv[1]);
    try{
        checked_provider_forwarding();exceptions_and_output_prefix();actual_foreign_factory_checks_encoding();
        unsigned expected=211;
        for(const auto* mode:{"--initial","--locale","--query","--fill","--native-lower-throw"})
            require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==expected++,"own child reaches exact typed rejection/state predicate; unrelated abort cannot pass");
        std::cout<<"PASS: "<<checks<<" callback identity integration checks; actual foreign factory, provider boundaries and exact predicate terminal children\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
