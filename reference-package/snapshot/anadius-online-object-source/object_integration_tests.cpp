#include "object.hpp"
#include "../anadius-online-version-source/version.hpp"
#include "../anadius-online-platform-source/platform_test_fixture.hpp"
#include <array>
#include <atomic>
#include <bit>
#include <csignal>
#include <cstdlib>
#include <new>
#include <process.h>

// Own EXE allocator observer. Only the exact long-cookie address armed by the
// factory is observed; no released bytes are read. The live source object slot
// is sampled at its deallocation, then the address is disarmed against reuse.
namespace cookie_lifetime {
using Object=anadius::online_recovered::Object;
constinit std::atomic<bool> enabled=false,released=false,committed=false;
constinit std::atomic<void*> tracked=nullptr;
constinit std::atomic<Object*> object_slot=nullptr;
constinit std::atomic<Object> expected_object=0;
constinit std::atomic<unsigned> arms=0,deletions=0;
void* allocate(std::size_t size){
    const auto error=::GetLastError();void* value=std::malloc(size?size:1);::SetLastError(error);
    if(!value)throw std::bad_alloc();return value;
}
void release(void* value)noexcept{
    const auto error=::GetLastError();void* expected=value;
    if(value&&tracked.compare_exchange_strong(expected,nullptr)){
        auto* slot=object_slot.load();committed.store(slot&&*slot==expected_object.load());++deletions;released.store(true);
    }
    std::free(value);::SetLastError(error);
}
void begin(Object& slot,Object expected)noexcept{
    const auto error=::GetLastError();tracked.store(nullptr);released.store(false);committed.store(false);arms.store(0);deletions.store(0);
    object_slot.store(&slot);expected_object.store(expected);enabled.store(true);::SetLastError(error);
}
void arm(const char16_t* cookie)noexcept{
    const auto error=::GetLastError();if(enabled.load()){++arms;tracked.store(const_cast<char16_t*>(cookie));}::SetLastError(error);
}
void end()noexcept{const auto error=::GetLastError();enabled.store(false);tracked.store(nullptr);object_slot.store(nullptr);::SetLastError(error);}
}
void* operator new(std::size_t size){return cookie_lifetime::allocate(size);}
void* operator new[](std::size_t size){return cookie_lifetime::allocate(size);}
void operator delete(void* value)noexcept{cookie_lifetime::release(value);}
void operator delete[](void* value)noexcept{cookie_lifetime::release(value);}
void operator delete(void* value,std::size_t)noexcept{cookie_lifetime::release(value);}
void operator delete[](void* value,std::size_t)noexcept{cookie_lifetime::release(value);}

// Only own typed exports, readable synthetic object/vtable storage, injected
// module/UI endpoints and memory files. No DLL/game load, actual hook or user I/O.
namespace {
using namespace anadius;namespace online=online_recovered;namespace version=online_version;
unsigned checks=0;
void require(bool value,const char* message){++checks;if(!value){std::cerr<<"FAILED: "<<message<<'\n';throw std::runtime_error(message);}}
template<class F>bool logic_rejected(F&& fn){try{fn();}catch(const std::logic_error&){return true;}return false;}
[[noreturn]]void terminate_own(unsigned code){::TerminateProcess(::GetCurrentProcess(),code);std::_Exit(240);}
void generic_abort(int){terminate_own(239);}
struct Fault {unsigned identity;};
enum class Unknown {None,Factory,Destroy,NullVptr,NullSlot};
struct Trace {
    std::weak_ptr<version::Binding> binding;std::vector<std::weak_ptr<void>> code;
    std::array<std::byte,16> storage{};std::array<std::uintptr_t,1> table{};online::Object object=0;
    unsigned leases=0,code_destroyed=0,versions=0,factories=0,destroys=0,queries=0,lower_wrong=0,frees=0,lookups=0;
    unsigned fail_lease=0;bool lease_null=false,factory_missing=false,factory_null=false,throw_free=false;
    bool factory_active=false,destroy_active=false,queries_active=true,free_invalidated=false,free_codes_alive=false;
    bool status_saw_cookie_release=false,status_saw_committed_object=false;DWORD status_incoming=0;
    bool free_reentry_blocked=false;std::int32_t version_result=2;DWORD factory_incoming=0;
    std::u16string cookie;std::uintptr_t borrowed_cookie=0;Unknown unknown=Unknown::None;
    std::exception_ptr failure=std::make_exception_ptr(Fault{26});
    bool codes_alive()const{for(const auto& item:code)if(item.expired())return false;return !code.empty();}
};
std::shared_ptr<Trace> current;
online::Procedure version_address();online::Procedure factory_address();
void own_destroy(void* object){
    auto& trace=*current;++trace.destroys;
    const auto snapshot=trace.binding.lock()->snapshot();
    trace.destroy_active=object==reinterpret_cast<void*>(trace.object)&&snapshot.active&&snapshot.object_unknown&&snapshot.object==trace.object&&snapshot.factory_code_retained&&trace.codes_alive();
    static_cast<std::byte*>(object)[8]=trace.unknown==Unknown::Destroy?std::byte{'X'}:std::byte{'D'};
    ::SetLastError(0x5454);if(trace.unknown==Unknown::Destroy){::SetLastError(0x5555);std::rethrow_exception(trace.failure);}
}
std::int32_t own_version(){++current->versions;::SetLastError(0x5151);return current->version_result;}
online::Object own_factory(const char16_t* cookie){
    auto& trace=*current;trace.factory_incoming=::GetLastError();++trace.factories;
    const auto snapshot=trace.binding.lock()->snapshot();
    trace.factory_active=snapshot.active&&snapshot.object_unknown&&!snapshot.object&&snapshot.factory==factory_address()&&snapshot.factory_code_retained&&trace.codes_alive();
    trace.borrowed_cookie=reinterpret_cast<std::uintptr_t>(cookie);trace.cookie=cookie;
    cookie_lifetime::arm(cookie);
    ::SetLastError(0x5353);if(trace.unknown==Unknown::Factory)std::rethrow_exception(trace.failure);
    return trace.factory_null?0:trace.object;
}
online::Procedure version_address(){return reinterpret_cast<online::Procedure>(&own_version);}
online::Procedure factory_address(){return reinterpret_cast<online::Procedure>(&own_factory);}
FARPROC WINAPI lookup(HMODULE module,LPCSTR symbol){
    if(!current||reinterpret_cast<std::uintptr_t>(module)!=0x80)throw std::logic_error("Own object lookup module");
    ++current->lookups;b13_test::lookup->queries.emplace_back(reinterpret_cast<std::uintptr_t>(module),symbol);
    FARPROC result=nullptr;
    if(std::string_view(symbol)=="version_check")result=reinterpret_cast<FARPROC>(&own_version);
    else if(std::string_view(symbol)=="create_online"){if(!current->factory_missing)result=std::bit_cast<FARPROC>(&own_factory);}
    else throw std::logic_error("Own object lookup symbol");
    ::SetLastError(std::string_view(symbol)=="create_online"?0x4242:0x4141);return result;
}
struct Code {
    std::shared_ptr<Trace> trace;explicit Code(std::shared_ptr<Trace> value):trace(std::move(value)){}
    ~Code(){++trace->code_destroyed;::SetLastError(0xdddd);}
};
class Os final:public recovered::libraries::SystemCalls {
    std::shared_ptr<Trace> trace_;
public:
    b12_test::Os inner;
    explicit Os(std::shared_ptr<Trace> trace):trace_(std::move(trace)){inner.load_hook=[](const wchar_t*){return online::Module{0x80};};}
    std::uint32_t get_error_mode()override{return inner.get_error_mode();}
    std::uint32_t set_error_mode(std::uint32_t value)override{return inner.set_error_mode(value);}
    recovered::libraries::Module load_library(const wchar_t* name)override{return inner.load_library(name);}
    bool free_library(recovered::libraries::Module module)override{
        ++trace_->frees;auto binding=trace_->binding.lock();const auto snapshot=binding->snapshot();
        trace_->free_invalidated=snapshot.active&&snapshot.module==module&&!snapshot.procedure&&!snapshot.factory&&!snapshot.object&&!snapshot.object_unknown;
        trace_->free_codes_alive=trace_->codes_alive();
        trace_->free_reentry_blocked=logic_rejected([&]{(void)binding->call(version_address());})&&logic_rejected([&]{(void)binding->create(factory_address(),u"borrowed");});
        (void)inner.free_library(module);::SetLastError(0x6161);
        if(trace_->throw_free){::SetLastError(0x6262);std::rethrow_exception(trace_->failure);}return false;
    }
    int wide_to_narrow(std::uint32_t cp,std::uint32_t flags,const wchar_t* in,int count,char* out,int capacity,bool* used)override{return inner.wide_to_narrow(cp,flags,in,count,out,capacity,used);}
    std::uint32_t last_error()override{return inner.last_error();}
};
class QueryLower final:public online::Effects {
    std::shared_ptr<Trace> trace_;b13_test::Foreign lower_;
    [[noreturn]]void wrong(){++trace_->lower_wrong;throw std::logic_error("Native object path unexpectedly delegated factory/version/destroy/platform");}
    template<class F>auto query(online::Object object,F&& lower)->decltype(lower()){
        const auto snapshot=trace_->binding.lock()->snapshot();++trace_->queries;
        trace_->queries_active=trace_->queries_active&&snapshot.active&&snapshot.object==object&&object==trace_->object&&!snapshot.object_unknown&&snapshot.factory_code_retained;
        try{const auto result=lower();::SetLastError(0x7171);return result;}catch(...){::SetLastError(0x7272);throw;}
    }
public:
    QueryLower(std::shared_ptr<Trace> trace,std::shared_ptr<b13_test::ForeignState> foreign):trace_(std::move(trace)),lower_(std::move(foreign)){}
    std::uint32_t suppress_errors_0450a0()override{wrong();}
    void restore_error_mode(std::uint32_t)override{wrong();}
    configuration::Path filesystem_widen_006d70(std::string_view)override{wrong();}
    configuration::Path parent_path_019820(const configuration::Path&)override{wrong();}
    configuration::Path join_016b00(const configuration::Path&,const configuration::Path&)override{wrong();}
    online::Module load_copy_0450d0(const configuration::Path&)override{wrong();}
    online::Procedure get_proc_address(online::Module,std::string_view)override{wrong();}
    std::int32_t call_version_check(online::Procedure)override{wrong();}
    online::Object call_create_online(online::Procedure,const char16_t*)override{wrong();}
    std::int32_t status_virtual_08(online::Object object)override{
        trace_->status_incoming=::GetLastError();trace_->status_saw_cookie_release=cookie_lifetime::released.load();trace_->status_saw_committed_object=cookie_lifetime::committed.load();
        return query(object,[&]{return lower_.status_virtual_08(object);});
    }
    const char* username_virtual_10(online::Object object)override{return query(object,[&]{return lower_.username_virtual_10(object);});}
    const char* persona_virtual_18(online::Object object)override{return query(object,[&]{return lower_.persona_virtual_18(object);});}
    const char* user_id_virtual_20(online::Object object)override{return query(object,[&]{return lower_.user_id_virtual_20(object);});}
    void destroy_virtual_00(online::Object)override{wrong();}
    bool free_library(online::Module)override{wrong();}
    status_recovered::Status status_01ea20(const configuration::Path&,std::uint32_t)override{wrong();}
    [[noreturn]]void filesystem_error_018130(std::string_view,std::uint32_t,const configuration::Path&)override{wrong();}
    std::string read_binary_03b3f0(const configuration::Path&)override{wrong();}
    bool write_binary_03c1d0(const configuration::Path&,std::string_view)override{wrong();}
    void log_literal_006880(std::string_view)override{wrong();}
    void log_finish_0088e0()override{wrong();}
    void dialog_038bc0(std::string_view,std::string_view,std::uint32_t)override{wrong();}
    void wait_steps_036880(std::int32_t value)override{lower_.wait_steps_036880(value);}
    void finish_flag_036820()override{lower_.finish_flag_036820();}
};
struct Graph {
    b13_test::Graph base;std::shared_ptr<Trace> trace=std::make_shared<Trace>();std::shared_ptr<version::Binding> binding;
    Graph(){
        current=trace;trace->table[0]=reinterpret_cast<std::uintptr_t>(&own_destroy);
        const auto table=reinterpret_cast<std::uintptr_t>(trace->table.data());std::memcpy(trace->storage.data(),&table,sizeof(table));
        trace->object=reinterpret_cast<online::Object>(trace->storage.data());base.foreign->object=trace->object;base.selected.reset();
        binding=std::make_shared<version::Binding>([trace=trace](online::Module module)->std::shared_ptr<void>{
            if(module!=0x80)throw std::logic_error("Own object lease module");++trace->leases;::SetLastError(0xeeee);
            if(trace->leases==trace->fail_lease){if(trace->lease_null)return {};std::rethrow_exception(trace->failure);}
            auto code=std::make_shared<Code>(trace);trace->code.emplace_back(code);return code;
        });trace->binding=binding;
        auto resources=base.resources();auto os=std::make_unique<Os>(trace);base.os=&os->inner;resources.os=std::move(os);
        resources.get_proc=lookup;resources.version=binding;resources.native_objects=true;
        base.selected=std::make_unique<online_platform::OnlinePlatform>(base.output,base.dialog,std::make_unique<QueryLower>(trace,base.foreign),std::move(resources));
    }
    ~Graph(){base.selected.reset();binding.reset();current.reset();}
};
void successful_source_cookie(const configuration::Path& cookie){
    Graph graph;b13_test::State state;state.cookie=cookie;state.use_last=1;
    const bool track_cookie=cookie.size()==2048;if(track_cookie)cookie_lifetime::begin(state.object,graph.trace->object);
    graph.base.stats->values[state.cookie]={0,0x80,std::nullopt};graph.base.stats->values[state.last_avatar]={0,0x80,std::nullopt};
    online::worker_035b60(state.refs(),*graph.base.selected);
    require(state.module==0x80&&state.object==graph.trace->object&&state.online_flag==1&&graph.trace->versions==1&&graph.trace->factories==1,"Actual worker adopts actual own typed factory object after version2 exactly once");
    require(graph.trace->factory_active&&graph.trace->factory_incoming==0x4242&&graph.trace->cookie==std::u16string(cookie.c_str())&&state.cookie==cookie&&graph.trace->borrowed_cookie!=0,"Factory borrows readable UTF16 cookie only during call; long text and embedded NUL follow C-string shape without modifying counted source");
    require(graph.trace->leases==2&&graph.trace->code.size()==2&&graph.trace->codes_alive(),"Version and factory retain separate exact-module code leases in the same Binding");
    require(graph.trace->queries==4&&graph.trace->queries_active&&graph.base.foreign->events==std::vector<std::string>{"status","get-1","get-2","get-3"}&&graph.trace->lower_wrong==0,"Four mandatory lower queries keep order under exact adopted-object active reservation");
    require(state.username=="own-user"&&state.persona=="own-persona"&&state.user_id=="own-id"&&state.avatar==state.last_avatar&&graph.base.files->outputs[state.last_profile]=="own-persona;own-id;own-user","Actual getter copies, memory profile field order and avatar behavior remain intact");
    require(graph.base.foreign->waits==std::vector<std::int32_t>{1}&&graph.base.foreign->finishes==1&&graph.base.output->str()=="online dll found!\nauthentication ok\n","Worker keeps lower phase publication and exact source success lines");
    const auto snapshot=graph.binding->snapshot();const auto lookups=graph.trace->lookups;
    require(snapshot.object==state.object&&!snapshot.object_unknown&&!snapshot.active&&!snapshot.poisoned&&snapshot.factory==factory_address()&&snapshot.factory_code_retained,"Normal factory result becomes exact known live object in shared lifecycle owner");
    require(logic_rejected([&]{(void)graph.base.selected->get_proc_address(state.module,"version_check");})&&logic_rejected([&]{(void)graph.base.selected->free_library(state.module);})&&graph.trace->lookups==lookups&&!graph.trace->frees,"Live object blocks version rebind and module free before lower lookup/free effects");
    const auto queries=graph.trace->queries;
    require(logic_rejected([&]{(void)graph.base.selected->status_virtual_08(state.object+8);})&&graph.trace->queries==queries,"Foreign query rejects a different object before mandatory lower provider");
    online::cleanup_0366f0(state.refs(),*graph.base.selected);const auto outgoing=::GetLastError();const auto retired=graph.binding->snapshot();
    require(graph.trace->destroys==1&&graph.trace->destroy_active&&graph.trace->storage[8]==std::byte{'D'}&&state.object==0&&state.module==0,"Core cleanup calls actual adopted slot0 as void(this), then zeros object before module release");
    require(graph.trace->frees==1&&graph.trace->free_invalidated&&graph.trace->free_codes_alive&&graph.trace->free_reentry_blocked,"Both callable exports are invalidated before lower free while both code owners stay held through it");
    require(!retired.module&&!retired.object&&!retired.object_unknown&&!retired.procedure&&!retired.factory&&!retired.code_retained&&!retired.factory_code_retained&&!retired.poisoned&&graph.trace->code_destroyed==2&&outgoing==0x6161,"Normal FALSE free clears core/module/object binding and releases two owners without losing lower LastError");
    online::cleanup_0366f0(state.refs(),*graph.base.selected);
    require(graph.trace->destroys==1&&graph.trace->frees==1&&graph.base.foreign->events.size()==4,"Repeated cleanup never retries factory, destroy, module release or lower queries");
    if(track_cookie){
        const auto arms=cookie_lifetime::arms.load(),deletions=cookie_lifetime::deletions.load();
        const auto released=cookie_lifetime::released.load(),committed=cookie_lifetime::committed.load();cookie_lifetime::end();
        require(arms==1&&deletions==1&&released&&committed,"Actual exact long factory-cookie heap buffer is deleted once after core object assignment");
        require(graph.trace->status_saw_cookie_release&&graph.trace->status_saw_committed_object,"Actual long cookie buffer must be freed after object commit and before first lower status call");
        require(graph.trace->status_incoming==0x5353,"Own allocator observation and cookie deletion preserve factory outgoing LastError at status entry");
    }
}
void known_absence_and_lookup_failures(){
    {
        Graph graph;b13_test::State state;graph.trace->factory_null=true;bool rejected=false;
        try{online::worker_035b60(state.refs(),*graph.base.selected);}catch(const online::OutOfEvidenceInput&){rejected=true;}
        const auto snapshot=graph.binding->snapshot();
        require(rejected&&state.object==0&&state.module==0x80&&snapshot.object==0&&!snapshot.object_unknown&&!snapshot.poisoned&&graph.trace->factories==1,"Known NULL factory return preserves existing core domain error and known-zero object");
        require(graph.trace->queries==0&&graph.trace->destroys==0&&graph.trace->frees==0&&graph.base.foreign->finishes==0&&graph.base.output->str()=="online dll found!\n","NULL factory does not invent lower queries, destroy, cleanup or worker finish");
        online::cleanup_0366f0(state.refs(),*graph.base.selected);
        require(state.module==0&&graph.trace->frees==1&&graph.trace->destroys==0&&graph.trace->code_destroyed==2,"Separate explicit cleanup can release module and both leases after known NULL result");
    }
    {
        Graph graph;b13_test::State state;graph.trace->factory_missing=true;online::worker_035b60(state.refs(),*graph.base.selected);
        require(graph.trace->factories==0&&graph.trace->leases==1&&graph.trace->queries==0&&state.module==0&&graph.trace->frees==1,"Missing factory never calls native factory and worker frees only retained version lease");
        require(graph.base.output->str()=="online dll found!\nfactory function not found\n"&&graph.base.foreign->finishes==1&&graph.trace->code_destroyed==1,"Missing factory preserves distinct source log and finish prefix");
    }
    for(bool null_lease:{false,true}){
        Graph graph;b13_test::State state;graph.trace->fail_lease=2;graph.trace->lease_null=null_lease;std::exception_ptr observed;
        try{online::worker_035b60(state.refs(),*graph.base.selected);}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();bool expected=!null_lease&&observed==graph.trace->failure;
        if(null_lease)try{if(observed)std::rethrow_exception(observed);}catch(const std::invalid_argument&){expected=true;}catch(...){}
        const auto snapshot=graph.binding->snapshot();
        require(expected&&outgoing==0x4242&&snapshot.module==0x80&&snapshot.procedure==version_address()&&!snapshot.factory&&!snapshot.object&&!snapshot.object_unknown&&!snapshot.poisoned,"Second lease throw/null preserves factory-lookup LastError and loaded version prefix without adopting object");
        require(state.module==0x80&&state.object==0&&graph.trace->factories==0&&graph.trace->frees==0&&graph.base.foreign->finishes==0&&graph.trace->codes_alive(),"Factory lease failure leaves retained version owner and no automatic source success/cleanup");
        online::cleanup_0366f0(state.refs(),*graph.base.selected);
        require(state.module==0&&graph.trace->frees==1&&graph.trace->code_destroyed==1,"Explicit module cleanup retires known factory-lookup failure prefix");
    }
    {
        Graph graph;b13_test::State state;graph.trace->version_result=3;online::worker_035b60(state.refs(),*graph.base.selected);
        require(graph.trace->factories==0&&graph.trace->leases==1&&graph.base.sdk.messages.size()==1&&graph.base.sdk.messages[0].text==L"Online library version mismatch!"&&graph.trace->frees==1,"Version mismatch still takes injected dialog and cleanup before any factory lease/call");
    }
}
void getter_failure_and_free_throw(){
    {
        Graph graph;b13_test::State state;graph.base.foreign->throw_getter=2;bool rejected=false;
        try{online::worker_035b60(state.refs(),*graph.base.selected);}catch(const std::runtime_error&){rejected=true;}
        const auto outgoing=::GetLastError();const auto snapshot=graph.binding->snapshot();
        require(rejected&&outgoing==0x7272&&state.online_flag==1&&state.username=="own-user"&&state.persona=="seed-persona"&&state.user_id=="seed-id","Lower getter exception preserves earlier online flag/username and outgoing error prefix");
        require(snapshot.object==state.object&&!snapshot.active&&!snapshot.object_unknown&&!snapshot.poisoned&&snapshot.factory_code_retained&&graph.trace->queries_active,"Lower query throw ends reservation while exact adopted object and factory lease remain known live");
        require(graph.trace->destroys==0&&graph.trace->frees==0&&graph.base.foreign->finishes==0&&graph.base.output->str()=="online dll found!\nauthentication ok\n","Getter failure does not invent destroy, free, phase finish or output rollback");
        online::cleanup_0366f0(state.refs(),*graph.base.selected);
        require(state.object==0&&state.module==0&&graph.trace->destroys==1&&graph.trace->frees==1&&graph.trace->code_destroyed==2,"Known retained object after getter failure supports separate normal native destroy and free");
    }
    {
        Graph graph;b13_test::State state;online::load_online_035bd0(state.refs(),*graph.base.selected);graph.trace->throw_free=true;std::exception_ptr observed;
        try{online::cleanup_0366f0(state.refs(),*graph.base.selected);}catch(...){observed=std::current_exception();}
        const auto outgoing=::GetLastError();const auto snapshot=graph.binding->snapshot();
        require(observed==graph.trace->failure&&outgoing==0x6262&&state.object==0&&state.module==0x80&&graph.trace->destroys==1,"Unknown free throw occurs only after normal native destroy and preserves core module prefix");
        require(!snapshot.object&&!snapshot.object_unknown&&snapshot.poisoned&&!snapshot.active&&snapshot.module==0x80&&!snapshot.procedure&&!snapshot.factory&&snapshot.code_retained&&snapshot.factory_code_retained&&graph.trace->codes_alive(),"Free throw retains both code leases in poisoned known-zero object owner with both exports invalidated");
        require(logic_rejected([&]{online::cleanup_0366f0(state.refs(),*graph.base.selected);})&&graph.trace->frees==1&&graph.trace->destroys==1&&state.module==0x80,"Poisoned free retry rejects before further native destroy or lower free");
    }
}
int terminal_child(std::string_view mode){
    ::SetErrorMode(SEM_FAILCRITICALERRORS|SEM_NOGPFAULTERRORBOX);std::signal(SIGABRT,generic_abort);
    Graph graph;b13_test::State state;unsigned expected=221;
    if(mode=="--factory-throw")graph.trace->unknown=Unknown::Factory;
    else if(mode=="--destroy-throw"){graph.trace->unknown=Unknown::Destroy;expected=222;}
    else if(mode=="--null-vptr"){graph.trace->unknown=Unknown::NullVptr;expected=223;}
    else if(mode=="--null-slot"){graph.trace->unknown=Unknown::NullSlot;expected=224;}
    else return 238;
    void* guarded=nullptr;
    if(graph.trace->unknown!=Unknown::Factory){
        guarded=::VirtualAlloc(nullptr,4096,MEM_COMMIT|MEM_RESERVE,PAGE_READWRITE);if(!guarded)terminate_own(237);
        auto table=graph.trace->unknown==Unknown::NullVptr?std::uintptr_t{0}:reinterpret_cast<std::uintptr_t>(graph.trace->table.data());
        if(graph.trace->unknown==Unknown::NullSlot)graph.trace->table[0]=0;
        std::memcpy(guarded,&table,sizeof(table));graph.trace->object=reinterpret_cast<online::Object>(guarded);graph.base.foreign->object=graph.trace->object;
    }
    std::exception_ptr observed;
    try{
        online::worker_035b60(state.refs(),*graph.base.selected);
        if(graph.trace->unknown==Unknown::Factory)terminate_own(230);
        ::SetLastError(0x9090);
        online::cleanup_0366f0(state.refs(),*graph.base.selected);terminate_own(230);
    }catch(...){observed=std::current_exception();}
    const auto outgoing=::GetLastError();const auto snapshot=graph.binding->snapshot();
    const bool factory=graph.trace->unknown==Unknown::Factory,invalid=graph.trace->unknown==Unknown::NullVptr||graph.trace->unknown==Unknown::NullSlot;
    bool typed=factory||graph.trace->unknown==Unknown::Destroy?observed==graph.trace->failure:false;
    if(invalid)try{std::rethrow_exception(observed);}catch(const online::OutOfEvidenceInput&){typed=true;}catch(...){}
    const bool prefix=factory?state.object==0&&state.online_flag==0&&graph.base.foreign->finishes==0:
        state.object==graph.trace->object&&state.online_flag==1&&graph.base.foreign->finishes==1;
    const bool destroy_prefix=graph.trace->unknown!=Unknown::Destroy||static_cast<std::byte*>(guarded)[8]==std::byte{'X'};
    if(guarded){DWORD previous=0;if(!::VirtualProtect(guarded,4096,PAGE_NOACCESS,&previous))terminate_own(237);}
    const auto destroys=graph.trace->destroys,queries=graph.trace->queries,lookups=graph.trace->lookups,versions=graph.trace->versions,factories=graph.trace->factories;
    const bool blocked=logic_rejected([&]{online::cleanup_0366f0(state.refs(),*graph.base.selected);})&&
        logic_rejected([&]{(void)graph.base.selected->get_proc_address(state.module,"version_check");})&&
        logic_rejected([&]{(void)graph.base.selected->get_proc_address(state.module,"create_online");})&&
        logic_rejected([&]{(void)graph.base.selected->call_create_online(factory_address(),u"retry");})&&
        logic_rejected([&]{(void)graph.base.selected->free_library(state.module);})&&
        logic_rejected([&]{(void)graph.base.selected->status_virtual_08(graph.trace->object);});
    const bool error=factory?outgoing==0x5353:graph.trace->unknown==Unknown::Destroy?outgoing==0x5555:outgoing==0x9090;
    if(typed&&prefix&&destroy_prefix&&error&&blocked&&state.module==0x80&&snapshot.module==0x80&&snapshot.poisoned&&!snapshot.active&&snapshot.object_unknown&&
        snapshot.object==(factory?0:graph.trace->object)&&snapshot.code_retained&&snapshot.factory_code_retained&&graph.trace->codes_alive()&&graph.trace->code_destroyed==0&&
        graph.trace->factories==factories&&factories==1&&graph.trace->versions==versions&&versions==1&&graph.trace->destroys==destroys&&destroys==(factory||invalid?0U:1U)&&
        graph.trace->queries==queries&&graph.trace->lookups==lookups&&graph.trace->frees==0&&graph.trace->lower_wrong==0)terminate_own(expected);
    terminate_own(237);
}
}
int main(int argc,char** argv){
    if(argc==2)return terminal_child(argv[1]);
    try{
        successful_source_cookie(configuration::Path(2048,u'Q'));successful_source_cookie(configuration::Path(u"A\0B",3));
        known_absence_and_lookup_failures();getter_failure_and_free_throw();unsigned expected=221;
        for(const auto* mode:{"--factory-throw","--destroy-throw","--null-vptr","--null-slot"})
            require(_spawnl(_P_WAIT,argv[0],argv[0],mode,static_cast<const char*>(nullptr))==expected++,"Own child proves typed unknown-state/code-retention prefix and no further SDK/object reads; unrelated abort cannot pass");
        std::cout<<"PASS: "<<checks<<" online object integration checks; actual factory/slot0 lifecycle, lower queries and retained unknown outcomes\n";return 0;
    }catch(const std::exception& error){std::cerr<<error.what()<<'\n';return 1;}
}
