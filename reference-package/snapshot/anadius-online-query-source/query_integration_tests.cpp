#include "query.hpp"
#include "../anadius-online-version-source/version.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include "../anadius-bootstrap-runtime-source/runtime_test_fixture.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <cstdlib>
#include <new>

// Own EXE allocator only. Its optional hook observes real std::string storage
// allocation; recursion is suppressed for expected rejection exceptions. Exact
// cookie-address deletion is observed without reading released memory.
namespace allocation_probe {
using Hook=void(*)(std::size_t,void*);
constinit std::atomic<Hook> hook=nullptr;
constinit std::atomic<void*> cookie=nullptr;
constinit std::atomic<bool> cookie_released=false;
thread_local bool inside=false;
void* allocate(std::size_t size){
    const auto error=::GetLastError();void* value=std::malloc(size?size:1);
    if(!value){::SetLastError(error);throw std::bad_alloc();}
    if(!inside){
        inside=true;
        try{if(auto callback=hook.load())callback(size,value);}
        catch(...){inside=false;std::free(value);::SetLastError(error);throw;}
        inside=false;
    }
    ::SetLastError(error);return value;
}
void release(void* value)noexcept{
    const auto error=::GetLastError();void* expected=value;
    if(value&&cookie.compare_exchange_strong(expected,nullptr))cookie_released.store(true);
    std::free(value);::SetLastError(error);
}
}
void* operator new(std::size_t size){return allocation_probe::allocate(size);}
void* operator new[](std::size_t size){return allocation_probe::allocate(size);}
void operator delete(void* value)noexcept{allocation_probe::release(value);}
void operator delete[](void* value)noexcept{allocation_probe::release(value);}
void operator delete(void* value,std::size_t)noexcept{allocation_probe::release(value);}
void operator delete[](void* value,std::size_t)noexcept{allocation_probe::release(value);}

// Actual SourceBackend worker and cleanup with own executable code, fake module
// APIs and memory files. No original DLL/game, network or user file is accessed.
namespace {
using namespace anadius;namespace online=online_recovered;namespace version=online_version;namespace backend=bootstrap_backend;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
template<class F>bool logic_rejected(F&& call){try{call();}catch(const std::logic_error&){return true;}return false;}
struct Fault {unsigned identity;};
struct Trace {
    std::weak_ptr<version::Binding> binding;std::array<std::uintptr_t,5> table{};std::array<std::byte,16> storage{};
    online::Object object=0;std::vector<char> events;std::array<std::string,3> strings{std::string(8192,'U'),std::string(12288,'P'),std::string(16384,'I')};
    unsigned versions=0,factories=0,statuses=0,destroys=0,lower_foreign=0,lower_platform=0,lower_waits=0,lower_finishes=0;
    unsigned free_probe_lower=0;std::array<unsigned,3> copy_allocations{};std::array<std::size_t,3> allocated_bytes{};std::array<bool,3> allocation_active{},destroy_blocked{},free_blocked{};
    std::int32_t status=0;int null_getter=-1,throw_getter=-1,fail_allocation=-1,armed_copy=-1;
    std::size_t armed_size=0;bool all_this=true,all_active=true,cookie_before_status=false,factory_cookie_heap=false;
    bool throwing_allocation_seen=false;std::u16string cookie_copy;
    std::exception_ptr failure=std::make_exception_ptr(Fault{27});
};
std::shared_ptr<Trace> current;
void observe_allocation(std::size_t size,void* destination){
    if(!current||current->armed_copy<0)return;
    auto& trace=*current;const auto index=static_cast<unsigned>(trace.armed_copy);trace.armed_copy=-1;
    ++trace.copy_allocations[index];trace.allocated_bytes[index]=size;const auto binding=trace.binding.lock();const auto snapshot=binding->snapshot();
    trace.allocation_active[index]=destination&&size>=trace.armed_size&&snapshot.active&&snapshot.object==trace.object&&!snapshot.object_unknown&&snapshot.factory_code_retained;
    // Never destroy an unguarded negative-control object. Missing reservation is
    // recorded and asserted after explicit normal source cleanup instead.
    if(snapshot.active){
        trace.destroy_blocked[index]=logic_rejected([&]{binding->destroy(trace.object);});
        trace.free_blocked[index]=logic_rejected([&]{(void)binding->free(0x80,[&]{++trace.free_probe_lower;return false;});});
    }
    ::SetLastError(0xbeef);
    if(trace.fail_allocation==static_cast<int>(index)){trace.throwing_allocation_seen=true;throw std::bad_alloc();}
}
void observe_call(void* object,char event){
    auto& trace=*current;trace.events.push_back(event);const auto snapshot=trace.binding.lock()->snapshot();
    trace.all_this=trace.all_this&&object==reinterpret_cast<void*>(trace.object);
    trace.all_active=trace.all_active&&snapshot.active&&snapshot.object==trace.object&&!snapshot.object_unknown&&snapshot.factory_code_retained;
}
std::int32_t own_version(){++current->versions;::SetLastError(0x5151);return 2;}
online::Object own_factory(const char16_t* cookie){
    ++current->factories;current->cookie_copy=cookie;
    if(current->cookie_copy.size()==2048){allocation_probe::cookie_released.store(false);allocation_probe::cookie.store(const_cast<char16_t*>(cookie));current->factory_cookie_heap=true;}
    ::SetLastError(0x5353);return current->object;
}
void own_destroy(void* object){
    auto& trace=*current;++trace.destroys;trace.all_this=trace.all_this&&object==reinterpret_cast<void*>(trace.object);
    static_cast<std::byte*>(object)[8]=std::byte{'D'};::SetLastError(0x5454);
}
extern "C" __attribute__((noinline,used)) std::int32_t b27_status_body(void* object)noexcept{
    observe_call(object,'S');++current->statuses;current->cookie_before_status=!current->factory_cookie_heap||allocation_probe::cookie_released.load();
    ::SetLastError(0x6050);return current->status;
}
// Declared int32 caller shape; intentionally set upper RAX bits after returning
// from the nonthrowing recorder. Only EAX controls the recovered source branch.
__attribute__((naked)) std::int32_t own_status(void*){
    __asm__ volatile("subq $40, %rsp\n\tcall b27_status_body\n\taddq $40, %rsp\n\tmovabsq $0x100000000, %r11\n\torq %r11, %rax\n\tretq");
}
const char* getter(void* object,unsigned index){
    observe_call(object,static_cast<char>('U'+index));::SetLastError(0x6060+index);
    if(current->throw_getter==static_cast<int>(index))std::rethrow_exception(current->failure);
    if(current->null_getter==static_cast<int>(index))return nullptr;
    current->armed_copy=static_cast<int>(index);current->armed_size=current->strings[index].size()+1;
    return current->strings[index].c_str();
}
const char* own_username(void* object){return getter(object,0);}
const char* own_persona(void* object){return getter(object,1);}
const char* own_user_id(void* object){return getter(object,2);}
FARPROC WINAPI own_lookup(HMODULE module,LPCSTR symbol){
    if(reinterpret_cast<std::uintptr_t>(module)!=0x80)throw std::logic_error("Own query module identity");
    b13_test::lookup->queries.emplace_back(reinterpret_cast<std::uintptr_t>(module),symbol);::SetLastError(0x4242);
    if(std::string_view(symbol)=="version_check")return std::bit_cast<FARPROC>(&own_version);
    if(std::string_view(symbol)=="create_online")return std::bit_cast<FARPROC>(&own_factory);
    throw std::logic_error("Own query export name");
}
class NoForeign final:public online::Effects {
    std::shared_ptr<Trace> trace_;
    [[noreturn]]void wrong(){++trace_->lower_platform;throw std::logic_error("Selected query unexpectedly reached lower platform");}
    [[noreturn]]void foreign(){++trace_->lower_foreign;throw std::logic_error("Selected query unexpectedly reached one of seven lower foreign calls");}
public:
    explicit NoForeign(std::shared_ptr<Trace> trace):trace_(std::move(trace)){}
    std::uint32_t suppress_errors_0450a0()override{wrong();}
    void restore_error_mode(std::uint32_t)override{wrong();}
    configuration::Path filesystem_widen_006d70(std::string_view)override{wrong();}
    configuration::Path parent_path_019820(const configuration::Path&)override{wrong();}
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&)override{wrong();}
    online::Module load_copy_0450d0(const configuration::Path&)override{wrong();}
    online::Procedure get_proc_address(online::Module,std::string_view)override{wrong();}
    std::int32_t call_version_check(online::Procedure)override{foreign();}
    online::Object call_create_online(online::Procedure,const char16_t*)override{foreign();}
    std::int32_t status_virtual_08(online::Object)override{foreign();}
    const char* username_virtual_10(online::Object)override{foreign();}
    const char* persona_virtual_18(online::Object)override{foreign();}
    const char* user_id_virtual_20(online::Object)override{foreign();}
    void destroy_virtual_00(online::Object)override{foreign();}
    bool free_library(online::Module)override{wrong();}
    status_recovered::Status status_01ea20(const configuration::Path&,std::uint32_t)override{wrong();}
    [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const configuration::Path&)override{wrong();}
    std::string read_binary_03b3f0(const configuration::Path&)override{wrong();}
    bool write_binary_03c1d0(const configuration::Path&,std::string_view)override{wrong();}
    void log_literal_006880(std::string_view)override{wrong();}
    void log_finish_0088e0()override{wrong();}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{wrong();}
    void wait_steps_036880(std::int32_t)override{++trace_->lower_waits;}
    void finish_flag_036820()override{++trace_->lower_finishes;}
};
struct Graph {
    b13_test::Graph base;std::shared_ptr<Trace> trace=std::make_shared<Trace>();std::shared_ptr<version::Binding> binding;
    Graph(){
        current=trace;allocation_probe::hook.store(observe_allocation);allocation_probe::cookie.store(nullptr);allocation_probe::cookie_released.store(false);
        trace->table={reinterpret_cast<std::uintptr_t>(&own_destroy),reinterpret_cast<std::uintptr_t>(&own_status),reinterpret_cast<std::uintptr_t>(&own_username),reinterpret_cast<std::uintptr_t>(&own_persona),reinterpret_cast<std::uintptr_t>(&own_user_id)};
        const auto table=reinterpret_cast<std::uintptr_t>(trace->table.data());std::memcpy(trace->storage.data(),&table,sizeof(table));trace->object=reinterpret_cast<online::Object>(trace->storage.data());
        base.selected.reset();binding=std::make_shared<version::Binding>([](online::Module module)->std::shared_ptr<void>{if(module!=0x80)throw std::logic_error("Own query code lease module");return std::make_shared<int>(27);});trace->binding=binding;
        auto resources=base.resources();resources.version=binding;resources.native_objects=true;resources.native_queries=true;resources.get_proc=own_lookup;
        base.selected=std::make_unique<online_platform::OnlinePlatform>(base.output,base.dialog,std::make_unique<NoForeign>(trace),std::move(resources));
    }
    ~Graph(){allocation_probe::hook.store(nullptr);allocation_probe::cookie.store(nullptr);base.selected.reset();binding.reset();current.reset();}
};
void own_exit_callback(){}
struct Assembly {
    b4_test::Fixture sources;Graph graph;configuration::State config{};std::array<std::uint8_t,3> flags{};
    std::unique_ptr<backend::SourceBackend> selected;bool stopped=false;
    Assembly(){
        config.executable_path=u"C:\\own\\host.exe";config.cookie_path=configuration::Path(2048,u'Q');config.last_profile_path=u"OWN-memory-profile";config.last_avatar_path=u"OWN-memory-avatar";
        config.username="seed-user";config.persona_id="seed-persona";config.user_id="seed-id";config.avatar=u"seed-avatar";
        auto resources=sources.resources();resources.online=std::move(graph.base.selected);sources.online=nullptr;
        auto seed=sources.runtime_seed();seed.server_ready=1;
        selected=std::make_unique<backend::SourceBackend>(std::move(seed),std::move(resources));
        selected->bind_state(config,{flags[0],flags[1],flags[2]});(void)selected->register_exit(own_exit_callback);
    }
    void run(){selected->worker(recovered::Worker::second_035b60)();}
    void close(){selected->cleanup_second();stopped=selected->stop_and_join();if(!stopped)throw std::logic_error("Own SourceBackend stop did not drain");}
    ~Assembly(){if(selected){try{if(!stopped)close();selected.reset();}catch(...){std::terminate();}}}
};
void full_source_backend_owned_copies(){
    Assembly assembly;assembly.config.use_last_online_profile=1;
    assembly.graph.base.stats->values[assembly.config.cookie_path]={0,0x80,std::nullopt};assembly.graph.base.stats->values[assembly.config.last_avatar_path]={0,0x80,std::nullopt};
    assembly.run();const auto snapshot=assembly.graph.binding->snapshot();
    const auto username=assembly.config.username,persona=assembly.config.persona_id,user_id=assembly.config.user_id;
    const auto events=assembly.graph.trace->events;const auto allocations=assembly.graph.trace->copy_allocations;
    const auto active=assembly.graph.trace->allocation_active,destroy_blocked=assembly.graph.trace->destroy_blocked,free_blocked=assembly.graph.trace->free_blocked;
    assembly.close();
    require(username==assembly.graph.trace->strings[0]&&persona==assembly.graph.trace->strings[1]&&user_id==assembly.graph.trace->strings[2],"Actual SourceBackend worker copies all three long typed getter strings into actual shared configuration");
    require(events==std::vector<char>{'S','U','V','W'}&&assembly.graph.trace->all_this&&assembly.graph.trace->all_active,"Typed status/getters keep exact this pointer, source order and shared active adopted-object ownership");
    require(snapshot.object==assembly.graph.trace->object&&!snapshot.object_unknown&&!snapshot.active&&!snapshot.poisoned&&snapshot.factory_code_retained,"Successful native copies return with known live object and released query reservation");
    require(allocations==std::array<unsigned,3>{1,1,1},"Real std::string allocation is observed once for each long native getter copy through SourceBackend OnlineExecution");
    require(active==std::array<bool,3>{true,true,true}&&destroy_blocked==std::array<bool,3>{true,true,true}&&free_blocked==std::array<bool,3>{true,true,true}&&assembly.graph.trace->free_probe_lower==0,"Owned-copy reservation spans actual heap allocation and rejects destroy/free reentry before effects");
    require(assembly.graph.trace->factory_cookie_heap&&assembly.graph.trace->cookie_before_status&&allocation_probe::cookie_released.load(),"Exact temporary factory-cookie heap allocation is released before typed native status call");
    require(assembly.graph.trace->lower_foreign==0&&assembly.graph.trace->lower_platform==0&&assembly.graph.trace->lower_waits==0&&assembly.graph.trace->lower_finishes==0,"Actual SourceBackend bypasses all seven lower foreign calls and intercepts phase methods into same Execution");
    require(assembly.selected->state().online_ready==1&&assembly.selected->state().online==1&&assembly.selected->state().online_object==0&&assembly.selected->state().online_module==0,"Worker publishes actual shared ready/online flags and explicit cleanup zeros actual source object/module slots");
    require(assembly.graph.trace->destroys==1&&assembly.graph.base.os->unloaded==std::vector<online::Module>{0x80}&&assembly.graph.trace->storage[8]==std::byte{'D'},"Actual backend cleanup calls own typed slot0 exactly once before synthetic module release");
    require(assembly.graph.base.files->outputs[assembly.config.last_profile_path]==persona+';'+user_id+';'+username&&assembly.graph.base.files->reads.empty(),"Actual worker writes complete owned getter copies in retained profile order only to own memory files");
}
void recovered_effects_owned_copy_forwarding(){
    Graph graph;auto& selected=*graph.base.selected;
    const auto module=selected.load_copy_0450d0(u"C:/own/query-proxy.dll");
    const auto version=selected.get_proc_address(module,"version_check");(void)selected.call_version_check(version);
    const auto factory=selected.get_proc_address(module,"create_online");const auto object=selected.call_create_online(factory,u"OWN-proxy-cookie");
    // These platform objects are constructor-only dependencies. The proxy below
    // invokes only owned-copy forwards; no native OS/file method is called.
    recovered::libraries::NativeSystemCalls unused_os;status_recovered::NativeStats unused_stats;
    encoding_recovered::NativeCalls unused_encoding;encoding_recovered::PortableErrors unused_errors;file_recovered::NativeFactory unused_files;
    online::RecoveredEffects proxy(selected,unused_os,unused_stats,unused_encoding,unused_errors,unused_files);
    const auto username=proxy.copy_username(object),persona=proxy.copy_persona(object),user_id=proxy.copy_user_id(object);const auto outgoing=::GetLastError();
    selected.destroy_virtual_00(object);(void)selected.free_library(module);const auto retired=graph.binding->snapshot();
    require(username==graph.trace->strings[0]&&persona==graph.trace->strings[1]&&user_id==graph.trace->strings[2]&&outgoing==0x6062&&graph.trace->events==std::vector<char>{'U','V','W'},"RecoveredEffects explicitly forwards all three owned copies with exact strings, native order and outgoing error");
    require(graph.trace->copy_allocations==std::array<unsigned,3>{1,1,1}&&graph.trace->allocation_active==std::array<bool,3>{true,true,true}&&graph.trace->destroy_blocked==std::array<bool,3>{true,true,true}&&graph.trace->free_blocked==std::array<bool,3>{true,true,true}&&!graph.trace->free_probe_lower,"RecoveredEffects owned-copy forwarding keeps actual allocation under same Binding lifecycle reservation");
    require(graph.trace->all_this&&graph.trace->all_active&&graph.trace->lower_foreign==0&&graph.trace->lower_platform==0&&graph.trace->destroys==1&&graph.base.os->unloaded==std::vector<online::Module>{0x80}&&!retired.module&&!retired.object&&!retired.object_unknown&&!retired.procedure&&!retired.factory&&!retired.code_retained&&!retired.factory_code_retained,"Proxy-copy fixture explicitly destroys/frees through selected Binding before assertions and never invokes native proxy dependencies");
}
void later_getter_failures(){
    for(unsigned mode=0;mode<3;++mode){
        Assembly assembly;
        if(mode==0)assembly.graph.trace->null_getter=1;
        else if(mode==1)assembly.graph.trace->throw_getter=2;
        else assembly.graph.trace->fail_allocation=1;
        std::exception_ptr observed;try{assembly.run();}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();const auto snapshot=assembly.graph.binding->snapshot();
        const auto username=assembly.config.username,persona=assembly.config.persona_id,user_id=assembly.config.user_id;
        const auto events=assembly.graph.trace->events;const auto ready=assembly.selected->state().online_ready;assembly.close();
        bool expected=mode==1&&observed==assembly.graph.trace->failure;
        if(mode==0)try{if(observed)std::rethrow_exception(observed);}catch(const online::OutOfEvidenceInput&){expected=true;}catch(...){}
        if(mode==2)try{if(observed)std::rethrow_exception(observed);}catch(const std::bad_alloc&){expected=true;}catch(...){}
        require(expected&&outgoing==(mode==1?0x6062U:0x6061U),"Later NULL/getter throw/actual owned-copy bad_alloc retains exact failure type or identity and native outgoing error");
        require(username==assembly.graph.trace->strings[0]&&persona==(mode==1?assembly.graph.trace->strings[1]:"seed-persona")&&user_id=="seed-id","Earlier successful getter field assignments remain while failing/later fields retain original values");
        require(events==(mode==1?std::vector<char>{'S','U','V','W'}:std::vector<char>{'S','U','V'})&&ready==0,"Typed query failure preserves call prefix and does not invent shared worker-ready publication");
        require(snapshot.object==assembly.graph.trace->object&&!snapshot.object_unknown&&!snapshot.active&&!snapshot.poisoned&&snapshot.factory_code_retained,"NULL/native query/allocation exception ends reservation without poisoning known live object");
        require(assembly.graph.trace->destroys==1&&assembly.graph.base.os->unloaded.size()==1&&assembly.graph.trace->lower_foreign==0,"Separate source cleanup destroys retained known object and releases module without legacy foreign fallback");
        if(mode==2)require(assembly.graph.trace->throwing_allocation_seen&&assembly.graph.trace->allocation_active[1]&&assembly.graph.trace->destroy_blocked[1]&&assembly.graph.trace->free_blocked[1],"Injected bad_alloc occurs at real native-owned persona copy allocation while full shared lifecycle reservation remains active");
    }
}
void low32_status_and_offline_source(){
    for(const auto value:{1,2,-1}){
        Assembly assembly;assembly.graph.trace->status=value;assembly.run();assembly.close();
        require(assembly.graph.trace->statuses==1&&assembly.graph.trace->events==std::vector<char>{'S'}&&assembly.graph.trace->all_this&&assembly.graph.trace->all_active,"Status callback uses low32 signed result despite deliberately nonzero upper RAX and skips all getters offline");
        const auto expected=value==1?"online dll found!\nauthentication failed, starting offline\n":"online dll found!\nstarting offline\n";
        require(assembly.graph.base.output->str()==expected&&assembly.config.username=="seed-user"&&assembly.selected->state().online==0&&assembly.selected->state().online_ready==1,"Actual offline worker keeps status1-specific prefix, untouched fields and normal ready publication");
        require(assembly.graph.trace->destroys==1&&assembly.graph.base.os->unloaded.size()==1&&assembly.selected->state().online_object==0&&assembly.selected->state().online_module==0&&assembly.graph.trace->lower_foreign==0,"Offline worker already drains native object/module; explicit later cleanup does not repeat effects");
    }
}
}
int main(){
    try{
        full_source_backend_owned_copies();recovered_effects_owned_copy_forwarding();later_getter_failures();low32_status_and_offline_source();
        std::cout<<"PASS: "<<checks<<" online query integration checks; actual SourceBackend worker, native owned-copy allocation and failure prefixes\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
