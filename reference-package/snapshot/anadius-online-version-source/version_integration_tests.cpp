#include "version.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include <array>

// Only own typed version stub, synthetic module/export/UI endpoints and memory
// files. No target DLL/game, actual LoadLibrary/FreeLibrary or user profile I/O.
namespace {
using namespace anadius;
namespace online=online_recovered;namespace version=online_version;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
template<class F>bool logic_rejected(F&& fn){try{fn();}catch(const std::logic_error&){return true;}return false;}
struct Fault {unsigned identity;};
enum class LeaseFailure {None,Throw,Null};
struct Trace {
    std::weak_ptr<version::Binding> binding;std::weak_ptr<void> code;
    std::vector<std::string> events;std::vector<online::Module> leased;
    std::int32_t result=2;unsigned sdk_calls=0,leases=0,code_destroyed=0,free_calls=0;
    LeaseFailure lease_failure=LeaseFailure::None;
    bool throw_version=false,throw_free=false,throw_load=false,free_result=false;
    bool version_missing=false,factory_missing=false,free_invalidated=false,free_code_live=false,free_call_blocked=false;
    DWORD version_incoming=0;
    std::exception_ptr failure=std::make_exception_ptr(Fault{25});
};
std::shared_ptr<Trace> current;
struct Code {
    std::shared_ptr<Trace> trace;
    explicit Code(std::shared_ptr<Trace> value):trace(std::move(value)){}
    ~Code(){++trace->code_destroyed;::SetLastError(0xdddd);}
};
std::int32_t own_version(){
    if(!current)throw std::logic_error("Own version stub has no synthetic trace");
    auto& trace=*current;trace.version_incoming=::GetLastError();++trace.sdk_calls;trace.events.emplace_back("typed-version");
    ::SetLastError(0x5151);if(trace.throw_version)std::rethrow_exception(trace.failure);return trace.result;
}
online::Procedure own_procedure(){return reinterpret_cast<online::Procedure>(&own_version);}
FARPROC WINAPI own_get_proc(HMODULE module,LPCSTR name){
    if(!current||!b13_test::lookup)throw std::logic_error("Own GetProc context missing");
    b13_test::lookup->queries.emplace_back(reinterpret_cast<std::uintptr_t>(module),name);
    if(reinterpret_cast<std::uintptr_t>(module)!=0x80)throw std::logic_error("Own version lookup module identity");
    const std::string_view symbol(name);FARPROC result=nullptr;
    if(symbol=="version_check"){current->events.emplace_back("lookup-version");if(!current->version_missing)result=reinterpret_cast<FARPROC>(&own_version);}
    else if(symbol=="create_online"){current->events.emplace_back("lookup-factory");if(!current->factory_missing)result=reinterpret_cast<FARPROC>(0x82);}
    else throw std::logic_error("Own version lookup symbol domain");
    ::SetLastError(0x4141);return result;
}
class ControlledOs final:public recovered::libraries::SystemCalls {
    std::shared_ptr<Trace> trace_;
public:
    b12_test::Os inner;
    explicit ControlledOs(std::shared_ptr<Trace> trace):trace_(std::move(trace)){inner.load_hook=[](const wchar_t*){return online::Module{0x80};};}
    std::uint32_t get_error_mode()override{return inner.get_error_mode();}
    std::uint32_t set_error_mode(std::uint32_t value)override{return inner.set_error_mode(value);}
    recovered::libraries::Module load_library(const wchar_t* name)override{
        const auto result=inner.load_library(name);trace_->events.emplace_back("load");::SetLastError(0x3131);
        if(trace_->throw_load){::SetLastError(0x3030);std::rethrow_exception(trace_->failure);}return result;
    }
    bool free_library(recovered::libraries::Module module)override{
        ++trace_->free_calls;trace_->events.emplace_back("free");
        auto binding=trace_->binding.lock();if(!binding)throw std::logic_error("Own free missing Binding");
        const auto snapshot=binding->snapshot();trace_->free_invalidated=snapshot.active&&snapshot.module==module&&snapshot.procedure==0;
        trace_->free_code_live=!trace_->code.expired();
        trace_->free_call_blocked=logic_rejected([&]{(void)binding->call(own_procedure());});
        (void)inner.free_library(module);
        ::SetLastError(0x6161);if(trace_->throw_free){::SetLastError(0x6262);std::rethrow_exception(trace_->failure);}return trace_->free_result;
    }
    int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* in,int count,char* out,int capacity,bool* used)override{return inner.wide_to_narrow(cp,flags,in,count,out,capacity,used);}
    std::uint32_t last_error()override{return inner.last_error();}
};
struct Graph {
    b13_test::Graph base;
    std::shared_ptr<Trace> trace=std::make_shared<Trace>();
    std::shared_ptr<version::Binding> binding;
    Graph(){
        current=trace;base.selected.reset();
        binding=std::make_shared<version::Binding>([trace=trace](online::Module module)->std::shared_ptr<void>{
            ++trace->leases;trace->leased.push_back(module);trace->events.emplace_back("lease");::SetLastError(0xeeee);
            if(trace->lease_failure==LeaseFailure::Throw)std::rethrow_exception(trace->failure);
            if(trace->lease_failure==LeaseFailure::Null)return {};
            auto code=std::make_shared<Code>(trace);trace->code=code;return code;
        });trace->binding=binding;
        auto resources=base.resources();auto os=std::make_unique<ControlledOs>(trace);base.os=&os->inner;resources.os=std::move(os);
        resources.get_proc=own_get_proc;resources.version=binding;
        base.selected=std::make_unique<online_platform::OnlinePlatform>(base.output,base.dialog,std::make_unique<b13_test::Foreign>(base.foreign),std::move(resources));
    }
    ~Graph(){base.selected.reset();binding.reset();current.reset();}
};
void successful_worker_and_cleanup(){
    Graph graph;b13_test::State state;state.use_last=1;
    graph.base.stats->values[state.cookie]={0,0x80,std::nullopt};graph.base.stats->values[state.last_avatar]={0,0x80,std::nullopt};
    online::worker_035b60(state.refs(),*graph.base.selected);
    require(state.module==0x80&&state.object==0x90&&state.online_flag==1,"Actual worker publishes own loaded module/object and online flag after typed version2");
    require(graph.trace->sdk_calls==1&&graph.trace->leases==1&&graph.trace->leased==std::vector<online::Module>{0x80}&&graph.trace->version_incoming==0x4141,"Version export invokes actual own int32 stub exactly once under exact module lease while lease bookkeeping preserves lookup LastError");
    require(graph.base.foreign->events==std::vector<std::string>{"factory","status","get-1","get-2","get-3"}&&graph.base.foreign->wrong_calls==0,"Five non-destroy foreign calls keep mandatory lower order and version never reaches old token provider");
    require(graph.base.foreign->waits==std::vector<std::int32_t>{1}&&graph.base.foreign->finishes==1&&graph.base.foreign->cookie==state.cookie,"Actual worker preserves lower phase wait/finish and copied cookie contract");
    require(state.username=="own-user"&&state.persona=="own-persona"&&state.user_id=="own-id"&&state.avatar==state.last_avatar,"Actual core retains lower getter order and cached avatar selection");
    require(graph.base.files->outputs[state.last_profile]=="own-persona;own-id;own-user"&&graph.base.files->reads.empty(),"Profile source writes exact field order into own memory filesystem only");
    require(graph.base.output->str()=="online dll found!\nauthentication ok\n"&&graph.base.sdk.messages.empty()&&graph.base.os->mode==0x40,"Version opt-in preserves successful log/UI and mode-restoration prefixes");
    const auto live=graph.binding->snapshot();
    require(live.module==0x80&&live.procedure==own_procedure()&&live.code_retained&&!live.active&&!live.poisoned&&!graph.trace->code.expired(),"Normal worker retains exact callable version binding and code owner");
    online::cleanup_0366f0(state.refs(),*graph.base.selected);const auto outgoing=::GetLastError();const auto retired=graph.binding->snapshot();
    require(state.object==0&&state.module==0&&graph.base.foreign->events==std::vector<std::string>{"factory","status","get-1","get-2","get-3","destroy"},"Actual cleanup keeps sixth foreign destroy before module release and core zeroing");
    require(graph.trace->free_calls==1&&graph.trace->free_invalidated&&graph.trace->free_code_live&&graph.trace->free_call_blocked,"Procedure is invalidated before synthetic FreeLibrary while code owner survives its lower call");
    require(!retired.module&&!retired.procedure&&!retired.code_retained&&!retired.poisoned&&graph.trace->code.expired()&&graph.trace->code_destroyed==1&&outgoing==0x6161,"Normal FALSE free clears core/module binding and retires lease without replacing lower LastError");
    require(logic_rejected([&]{(void)graph.base.selected->call_version_check(own_procedure());})&&graph.trace->sdk_calls==1,"Retired exact version pointer cannot invoke own SDK after free");
    online::cleanup_0366f0(state.refs(),*graph.base.selected);
    require(graph.trace->free_calls==1&&graph.base.foreign->events.size()==6,"Repeated core cleanup makes no extra foreign or module call");
}
void missing_and_mismatched_exports(){
    {
        Graph graph;b13_test::State state;graph.trace->version_missing=true;
        online::worker_035b60(state.refs(),*graph.base.selected);
        require(graph.trace->sdk_calls==0&&graph.trace->leases==0&&graph.base.foreign->events.empty(),"Missing version export skips typed SDK, code lease and every foreign object call");
        require(graph.base.output->str()=="online dll found!\nversion function not found\n"&&state.module==0&&graph.trace->free_calls==1&&graph.base.foreign->finishes==1,"Missing version preserves actual worker log/cleanup/finish path");
        require(!graph.binding->snapshot().module&&!graph.binding->snapshot().procedure&&graph.base.sdk.messages.empty(),"Missing export has no callable binding or invented dialog after cleanup");
    }
    {
        Graph graph;b13_test::State state;graph.trace->result=3;bool found_before_dialog=false;
        graph.base.sdk.hook=[&](UINT cp,DWORD flags,LPCCH in,int count,LPWSTR out,int capacity){found_before_dialog=graph.base.output->str()=="online dll found!\n";return ::MultiByteToWideChar(cp,flags,in,count,out,capacity);};
        online::worker_035b60(state.refs(),*graph.base.selected);
        require(graph.trace->sdk_calls==1&&graph.base.foreign->events.empty()&&graph.base.exports.queries.size()==1,"Typed version3 rejects before factory lookup and leaves six foreign routes untouched");
        require(found_before_dialog&&graph.base.sdk.messages.size()==1&&graph.base.sdk.messages[0].text==L"Online library version mismatch!"&&graph.base.sdk.messages[0].flags==0x10,"Version mismatch keeps actual injected dialog after found log and before mismatch line");
        require(graph.base.output->str()=="online dll found!\nonline library version mismatch!\n"&&state.module==0&&state.object==0&&graph.trace->free_calls==1&&graph.trace->code.expired(),"Version mismatch worker retains log order then releases owned version binding");
    }
    {
        Graph graph;b13_test::State state;graph.trace->factory_missing=true;
        online::worker_035b60(state.refs(),*graph.base.selected);
        require(graph.trace->sdk_calls==1&&graph.base.foreign->events.empty()&&graph.base.exports.queries.size()==2,"Missing factory follows successful typed version and no lower foreign call");
        require(graph.base.output->str()=="online dll found!\nfactory function not found\n"&&state.module==0&&graph.trace->code.expired()&&graph.base.sdk.messages.empty(),"Factory absence keeps distinct source log and cleanup without mismatch dialog");
    }
}
void lookup_lease_failures(){
    for(const auto failure:{LeaseFailure::Throw,LeaseFailure::Null}){
        Graph graph;b13_test::State state;graph.trace->lease_failure=failure;std::exception_ptr observed;
        try{online::worker_035b60(state.refs(),*graph.base.selected);}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();bool expected=failure==LeaseFailure::Throw?observed==graph.trace->failure:false;
        if(failure==LeaseFailure::Null)try{if(observed)std::rethrow_exception(observed);}catch(const std::invalid_argument&){expected=true;}catch(...){}
        const auto snapshot=graph.binding->snapshot();
        require(expected&&outgoing==0x4141,"Lease throw/null failure preserves lookup LastError and exact selected failure shape");
        require(state.module==0x80&&state.object==0&&!state.online_flag&&snapshot.module==0x80&&!snapshot.procedure&&!snapshot.code_retained&&!snapshot.poisoned,"Lease failure preserves loaded core module and retires callable binding without inventing poison");
        require(graph.base.output->str()=="online dll found!\n"&&graph.trace->sdk_calls==0&&graph.trace->free_calls==0&&graph.base.foreign->finishes==0&&graph.base.foreign->events.empty(),"Lease failure stops before typed call, factory, automatic cleanup or worker finish");
        online::cleanup_0366f0(state.refs(),*graph.base.selected);
        require(state.module==0&&!graph.binding->snapshot().module&&graph.trace->free_calls==1,"Explicit known cleanup releases module retained by lease-binding failure");
    }
}
void version_and_destroy_exceptions(){
    {
        Graph graph;b13_test::State state;graph.trace->throw_version=true;std::exception_ptr observed;
        try{online::worker_035b60(state.refs(),*graph.base.selected);}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();const auto snapshot=graph.binding->snapshot();
        require(observed==graph.trace->failure&&outgoing==0x5151&&graph.trace->sdk_calls==1,"Actual typed version exception retains identity and outgoing LastError without retry");
        require(state.module==0x80&&state.object==0&&snapshot.procedure==own_procedure()&&snapshot.code_retained&&!snapshot.poisoned&&!snapshot.active&&!graph.trace->code.expired(),"Synchronous version exception retains exact live module/procedure/code owner");
        require(graph.base.foreign->events.empty()&&graph.trace->free_calls==0&&graph.base.foreign->finishes==0&&graph.base.output->str()=="online dll found!\n","Exceptional actual worker does not invent factory, cleanup, finish or success log");
        online::cleanup_0366f0(state.refs(),*graph.base.selected);
        require(state.module==0&&graph.trace->code.expired()&&graph.trace->sdk_calls==1,"Separate explicit cleanup retires retained version binding without retrying failed SDK");
    }
    {
        Graph graph;b13_test::State state;online::load_online_035bd0(state.refs(),*graph.base.selected);graph.base.foreign->throw_destroy=true;
        bool thrown=false;try{online::cleanup_0366f0(state.refs(),*graph.base.selected);}catch(const std::runtime_error&){thrown=true;}
        require(thrown&&state.object==0x90&&state.module==0x80&&graph.trace->free_calls==0,"Lower destroy exception retains original core object/module and skips module free");
        require(graph.binding->snapshot().procedure==own_procedure()&&!graph.trace->code.expired()&&graph.base.selected->call_version_check(own_procedure())==2&&graph.trace->sdk_calls==2,"Destroy failure leaves owned version binding callable; second call is explicit test action");
        graph.base.foreign->throw_destroy=false;online::cleanup_0366f0(state.refs(),*graph.base.selected);
        require(state.object==0&&state.module==0&&graph.trace->free_calls==1&&graph.trace->code.expired(),"Explicit retry after controlled destroy failure preserves destroy-before-free ordering");
    }
}
void unknown_load_and_free_failures(){
    {
        Graph graph;b13_test::State state;graph.trace->throw_load=true;std::exception_ptr observed;
        try{online::worker_035b60(state.refs(),*graph.base.selected);}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();const auto snapshot=graph.binding->snapshot();
        require(observed==graph.trace->failure&&outgoing==0x3030&&snapshot.poisoned&&!snapshot.active&&!snapshot.module,"Unknown lower load throw poisons managed version owner with exact exception/error");
        require(state.module==0&&graph.base.os->mode==0x41&&graph.base.os->loads.size()==1&&graph.base.exports.queries.empty()&&graph.base.foreign->finishes==0,"Load throw retains suppressed-mode/core-assignment prefix and skips lookup/finish");
        require(logic_rejected([&]{(void)graph.base.selected->load_copy_0450d0(u"C:/own/retry.dll");})&&graph.base.os->loads.size()==1&&graph.trace->free_calls==0,"Poisoned unknown load rejects new lower effects without automatic free or retry");
    }
    {
        Graph graph;b13_test::State state;online::load_online_035bd0(state.refs(),*graph.base.selected);graph.trace->throw_free=true;std::exception_ptr observed;
        try{online::cleanup_0366f0(state.refs(),*graph.base.selected);}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();const auto snapshot=graph.binding->snapshot();
        require(observed==graph.trace->failure&&outgoing==0x6262&&state.object==0&&state.module==0x80,"Free throw keeps earlier object-zero prefix but does not invent core module zeroing");
        require(snapshot.module==0x80&&!snapshot.procedure&&snapshot.poisoned&&!snapshot.active&&snapshot.code_retained&&!graph.trace->code.expired(),"Unknown free throw invalidates callable pointer and retains poisoned module/code lease");
        require(graph.trace->free_invalidated&&graph.trace->free_code_live&&graph.trace->free_call_blocked&&graph.trace->free_calls==1,"Throwing free observes invalidated procedure while exact code owner stays retained");
        require(logic_rejected([&]{(void)graph.base.selected->call_version_check(own_procedure());})&&graph.trace->sdk_calls==1,"Free poison blocks subsequent exact version call without new SDK invocation");
        require(logic_rejected([&]{online::cleanup_0366f0(state.refs(),*graph.base.selected);})&&graph.trace->free_calls==1&&state.module==0x80&&!graph.trace->code.expired(),"Unknown free retry is rejected before lower effects and preserves module/code retention");
    }
}
}
int main(){
    try{
        successful_worker_and_cleanup();missing_and_mismatched_exports();lookup_lease_failures();version_and_destroy_exceptions();unknown_load_and_free_failures();
        std::cout<<"PASS: "<<checks<<" online version integration checks; actual worker/cleanup, typed own version and explicit failure prefixes\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
