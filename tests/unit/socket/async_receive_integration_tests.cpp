#include "tests/support/socket/async_receive_test_fixture.hpp"
#include "preloader_l/net/transport_io_completion_dispatch.hpp"
// Compile-time regression: reproduce MSVC eh.h global lookup on other compilers.
// Only the qualified fixture helper below may be called; no definition is needed.
#if !defined(_MSC_VER)
void unexpected(void) noexcept(false);
#endif
using namespace receive_test;
namespace {
struct Table:s::IoHandlerVtable {
    bool empty;unsigned moves=0,destroys=0;
    explicit Table(bool e):IoHandlerVtable(e?0x777:0x123),empty(e){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override{++destroys;}
    void move_slot_10(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from)override{
        ++moves;if(empty){to.payload_00={};to.state_18=0;}else{to.payload_00=from.payload_00;to.state_18=from.state_18;}
    }
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{return 0x5678;}
};
struct Types:s::IoEnvironmentTypeApi {
    bool equal=false;unsigned comparisons=0;
    std::int32_t compare_type_names_04d2d0(std::uintptr_t a,std::uintptr_t b)override{
        check(a==0x1008&&b==0x1800a25c8ULL,"actual constructor type identity comparison");++comparisons;return equal?0:1;
    }
};
Table* factory_table=nullptr;unsigned factory_calls=0;
std::uintptr_t type_query(){return 0x1000;}
void factory(s::IoHandlerEnvironment& e,std::uintptr_t query,s::IoEnvironmentConstructorScratch&){
    check(query==0x5678,"cached actual factory receives query-slot result");++factory_calls;
    e.payload_00={21,22};e.operations_10=factory_table;e.state_18=0;e.schedule_20=0x456;e.executor_28=nullptr;
}
struct Recovered:s::RecoveredAsyncReceiveEffects {
    Fixture& f;
    Recovered(Fixture& f,s::IoHandlerSentinels sentinels,s::IoEnvironmentConstructorContext& constructor,std::span<s::IoEnvironmentSourceBinding> sources)
        :RecoveredAsyncReceiveEffects(sentinels,constructor,sources),f(f){}
    s::AsyncReceiveOperation& project_async_receive(s::OperationStorage& storage)override{return f.effects.project_async_receive(storage);}
};
struct CompletionEffects:s::IoCompletionEffects {
    unsigned calls=0,misc=0;s::ReceiveHandler observed;std::int32_t error=0;std::uint64_t bytes=0;
    s::Category misc_category_0285b0()override{++misc;return {0x9999};}
    void receive_body_025c40(s::ReceiveHandler& h,s::Error& copy,std::uint64_t n,std::uint32_t flag)override{
        check(flag==0&&copy.value==h.error.value&&n==h.transferred,"actual021a40 copies error/bytes into receive boundary");
        ++calls;observed=h;error=copy.value;bytes=n;copy.value=777;
        check(h.error.value==error,"receive boundary error is private copy");
    }
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{receive_test::unexpected();}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{receive_test::unexpected();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{receive_test::unexpected();}
};
struct Routing:s::IoDispatcherEffects {
    using IoDispatcherEffects::IoDispatcherEffects;
    void complete_unknown(s::IocpSchedulerEntry*,s::CancelOperation&,const s::Error&,std::uint32_t)override{receive_test::unexpected();}
    std::uintptr_t capture_tls_slot()override{receive_test::unexpected();}std::int32_t thread_epoch(std::uintptr_t)override{receive_test::unexpected();}
    void guard_header_04b838(std::int32_t&)override{receive_test::unexpected();}void register_atexit_04b770(std::uintptr_t)override{receive_test::unexpected();}
    void guard_footer_04b7cc(std::int32_t&)override{receive_test::unexpected();}std::uint32_t read_stop_34(s::IocpSchedulerEntry&)override{receive_test::unexpected();}
    std::array<std::uintptr_t,2>& copy_exception_024100(std::array<std::uintptr_t,2>&,const std::array<std::uintptr_t,2>&)override{receive_test::unexpected();}
    [[noreturn]]void rethrow_029880(std::array<std::uintptr_t,2>&)override{receive_test::unexpected();}
};
void chain(){
    struct Scenario{int sdk_result;std::uint32_t sdk_error,completion_error,bytes;std::int32_t expected;};
    const Scenario cases[]={{-1,997,0,7,0},{0,64,0,7,0},{-1,64,0,7,10054},{-1,1234,0,7,10061},
        {-1,997,0,0,2},{-1,997,10040,7,0},{-1,997,234,7,0},{-1,997,64,7,10054},{-1,997,1234,7,10061}};
    for(bool empty:{false,true})for(bool cache:{false,true})for(bool early:{false,true})for(const auto& c:cases){
        Fixture f;Table normal{false},sentinel{true};Types types;types.equal=empty;s::IoHandlerExecutor executor{};
        auto env=f.environment;env.sentinels={&sentinel,0x846e8,&executor,0x777};
        s::IoHandlerEnvironment allocator{{11,12},&normal,0,0x456,&executor};
        std::array<s::IoEnvironmentTypeBinding,1> queries{{{0x456,&type_query}}};
        std::array<s::IoEnvironmentFactoryBinding,1> factories{{{&executor,&factory}}};
        s::IoEnvironmentConstructorContext ctor{types,queries,factories};
        std::array<s::IoEnvironmentSourceBinding,1> sources{{{0x987,&allocator}}};
        Recovered recovered{f,env.sentinels,ctor,sources};factory_table=&normal;factory_calls=0;
        s::AsyncReceiveContext context{env,f.worker,f.memory,f.receive,f.completion,f.errors,recovered};
        Ref owner,weak;f.source.control_a8=&owner;f.implementation.weak_control_18=&weak;f.implementation.state_08=0x10;
        const auto source=f.source;f.receive.answer={c.sdk_result,c.sdk_error,c.bytes};
        f.receive.hook=[&]{if(early)f.effects.operation->io.operation.queued=1;};
        if(cache)f.worker.tls=&f.tls;
        s::ReceiveBufferSlice buffer{0x1111,11};s::start_async_receive_021d10(f.service,f.implementation,buffer,0,f.source,0x987,context);
        check(types.comparisons==1&&factory_calls==(empty?0U:1U),"receive bridge uses actual023dd0 empty/factory branches");
        check(weak.weak_0c==2&&owner.strong_08==1&&!f.source.control_a8,"pending operation holds retained weak and transferred strong owner");
        auto* operation=f.effects.operation.get();
        const auto packet=f.effects.packet.packet_error;
        const bool immediate_error=c.sdk_result!=0&&c.sdk_error!=997;
        const s::Error incoming=immediate_error?packet:s::Error{static_cast<std::int32_t>(c.completion_error),{0x2222}};
        const auto bytes=immediate_error?f.effects.packet.transferred_14:c.bytes;
        CompletionEffects callback;std::array<s::IoCompletionBinding,1> bindings{{{&operation->io.operation,&operation->io,nullptr}}};
        Routing routing{{},bindings,env,f.worker,f.memory,f.errors,callback};
        if(!cache)f.memory.free=[&]{f.effects.operation.reset();};
        routing.complete(&f.entry,operation->io.operation,incoming,bytes);
        check(callback.calls==1&&callback.error==c.expected&&callback.bytes==bytes,"actual route/027710/021a40 delivers expected completion including EOF and message-size normalization");
        check(callback.observed.words_60_78==source.words_60_78&&callback.observed.words_88_98==source.words_88_98&&callback.observed.byte_80==source.byte_80&&callback.observed.dword_84==source.dword_84,"defined receive state survives actual constructor/completion moves");
        check(callback.observed.object_a0==0xbeef&&owner.disposes==1&&owner.deletes==1&&owner.strong_08==0,"strong owner reaches callback then disposes exactly once");
        check(weak.weak_0c==1&&!weak.deletes,"completion releases only retained socket weak reference");
        check(callback.misc==(c.expected==2?1U:0U),"EOF uses misc category only on correct receive path");
        if(cache)check(f.memory.frees==0&&f.thread.cache_00[0]==f.memory.storage.identity&&f.memory.bytes[0]==std::byte{56},"actual receive cleanup returns0xe0 storage to IO cache");
        else check(f.memory.frees==1&&!f.effects.operation,"actual receive cleanup ends operation lifetime before callback");
        check(f.memory.allocations==std::vector<std::uint64_t>{225},"one actual0xe0 receive allocation");
    }
}
void missing_constructor_binding(){
    Fixture f;Table sentinel{true};Types types;std::array<s::IoEnvironmentTypeBinding,0> queries{};std::array<s::IoEnvironmentFactoryBinding,0> factories{};
    s::IoEnvironmentConstructorContext ctor{types,queries,factories};std::array<s::IoEnvironmentSourceBinding,0> sources{};
    Recovered recovered{f,{&sentinel,0,nullptr,0x777},ctor,sources};s::IoHandlerEnvironment destination;
    rejects([&]{recovered.construct_environment_023dd0(destination,0x987);});
    check(types.comparisons==0,"missing source rejected before constructor type effects");
}
}
int main(){try{chain();missing_constructor_binding();std::cout<<"PASS: "<<checks<<" async receive integration checks; actual023dd0 constructor and021d10 initiation, dispatcher IO route/027710/021a40, EOF/error normalization, retained owner/weak, actual cache/free lifetime;025c40 remains required boundary\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
