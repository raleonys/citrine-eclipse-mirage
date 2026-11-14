// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/net/transport_service_registry.hpp"
#include "laureondo/net/transport_recycle_storage.hpp"
#include <functional>
#include <iostream>
#include <vector>

namespace s=laureondo::socket;
namespace srv=laureondo::server;
namespace {
unsigned checks=0;
void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected controlled registry integration dependency");}
struct Gate final:s::SocketGate {
    int depth=0;unsigned enters=0,leaves=0;std::function<void(bool)> hook;
    void enter()override{check(!depth,"each integration gate acquired from unlocked state");++depth;++enters;if(hook)hook(true);}
    void leave()override{check(depth==1,"integration gate held on release");if(hook)hook(false);--depth;++leaves;}
};
struct Entry final:s::ServiceEntry {unsigned deletes=0;void scalar_delete(std::uint32_t flags)override{check(flags==1,"registry race delete flag1");++deletes;}};
struct RegistryEffects final:s::ServiceRegistryEffects {
    Gate& gate;Entry& entry;unsigned creates=0,comparisons=0;
    RegistryEffects(Gate& gate,Entry& entry):gate(gate),entry(entry){}
    std::uint32_t compare_type_info_04d2d0(std::uintptr_t left,std::uintptr_t right)override{check(gate.depth==1 && left==0x1800a2628ULL && right==left,"real registry uses required controlled type comparison under its gate");++comparisons;return 0;}
    s::ServiceEntry* create_socket_service_0228e0(s::ContextIdentity identity)override{check(!gate.depth && identity.value==42,"real registry constructor boundary receives exact context outside gate");++creates;return &entry;}
};
struct Errors final:s::Api {
    unsigned sends=0,categories=0,sets=0;std::int32_t error=0;s::Socket wake=0xfedcba9876543210ULL;
    void set_last_error(std::int32_t value)override{error=value;++sets;}
    s::Category system_category_028620()override{++categories;return {0x9922,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{return error;}
    s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
    s::SendCall wsa_send(s::Socket socket,std::span<s::Buffer> buffer,std::uint32_t flags)override{check(socket==wake && buffer.size()==1 && buffer[0].len==1 && buffer[0].buf[0]==0 && !flags,"actual queue wake uses injected one-byte NUL transfer only");++sends;return {0,1};}
    std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Acceptor final:s::AcceptorApi {
    unsigned creates=0,closes=0;
    s::Socket create_socket(std::int32_t family)override{check(family==2,"actual acceptor selects IPv4 family");++creates;return 13;}
    std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{check(socket==13 && port==99,"actual registry-resolved service supplies selected IOCP token");return 99;}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t name,std::int32_t value)override{check(socket==13 && level==65535 && name==128 && value==0,"actual low-level close requests observed linger option");return 0;}
    std::int32_t bind_socket(s::Socket socket,std::span<const std::byte>,std::int32_t size)override{check(socket==13 && size==16,"actual acceptor binds IPv4 endpoint length");return 0;}
    std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{check(socket==13 && backlog==2147483647,"actual acceptor listens with observed backlog");return 0;}
    std::int32_t close_socket(s::Socket socket)override{check(socket==13,"actual close retains selected socket identity");++closes;return 0;}
    std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Executor final:s::RegistryExecutorEffects {
    unsigned calls=0;s::SocketObject* expected=nullptr;std::uintptr_t token=0;std::function<void()> hook;
    void executor_cleanup(s::SocketObject& object,std::uintptr_t table)override{check(&object==expected && table==token,"bridge forwards exact borrowed object and full-width table token");++calls;if(hook)hook();}
};
struct NoCancel final:s::CancellationEffects {void cancel_026aa0(s::CancellationEngine&,s::Socket,const s::Error&)override{unused();}};
void projection(){
    Gate registry_gate,service_gate,other_gate;s::ServiceRegistry registry(registry_gate);Entry entry;RegistryEffects required(registry_gate,entry);
    s::SocketService service(service_gate),other(other_gate);s::CloseContext close_context,other_context;
    std::array<s::RegistryServiceBinding,1> bindings{{{&entry,&service,&close_context}}};Errors errors;Acceptor api;NoCancel cancel;Executor executor;
    s::RegistryLifecycleEffects bridge(registry,{42},required,bindings,errors,api,cancel,executor);
    bool caught=false;try{bridge.resolve_service_0239c0({43});}catch(const s::UnsupportedInput&){caught=true;}
    check(caught && !registry_gate.enters && !required.creates && !required.comparisons,"wrong context fails before registry/gate/constructor effects");
    check(&bridge.resolve_service_0239c0({42})==&service && registry.head==&entry && required.creates==1,"bridge resolves real registry entry to borrowed service");
    bindings[0]={&entry,&other,&other_context};
    check(&bridge.resolve_service_0239c0({42})==&other && required.creates==1 && required.comparisons==1,"borrowed binding is read live and cached native entry reused");
    for(unsigned field=0;field<3;++field){
        bindings[0]={&entry,&service,&close_context};
        if(field==0)bindings[0].entry=nullptr;else if(field==1)bindings[0].service=nullptr;else bindings[0].close_context=nullptr;
        caught=false;try{bridge.resolve_service_0239c0({42});}catch(const s::UnsupportedInput&){caught=true;}
        check(caught && required.creates==1 && !entry.deletes && registry.head==&entry && !registry_gate.depth,"missing projection component raises host guard without deleting cached entry");
    }
    bindings[0]={&entry,&service,&close_context};s::AcceptorState state;s::SocketObject object(state);
    caught=false;try{bridge.close_and_cancel_026fb0(other,object);}catch(const s::UnsupportedInput&){caught=true;}
    check(caught && !errors.sets && !api.closes,"unbound service close is rejected before low-level close effects");
    executor.expected=&object;executor.token=0xfedcba9876543210ULL;bridge.executor_cleanup(object,executor.token);
    check(executor.calls==1 && !entry.deletes,"executor projection forwards callback without owning service entry");
    check(!service.head && !other.head && !state.service,"direct bridge resolution does not invent object registration");
}
struct Failure final:s::RecycleFailureEffects {[[noreturn]]void length_error_047fc4(std::uintptr_t)override{unused();}};
struct Post final:s::CompletionApi {
    bool fail=false;std::vector<s::CancelOperation*> posted;
    std::int32_t post(std::uintptr_t port,s::CancelOperation& operation)override{check(port==123 && operation.queued==1 && !operation.next,"actual queue posts detached marked operations using selected scheduler token");posted.push_back(&operation);return fail?0:1;}
};
struct Owner final:s::Control {unsigned destroys=0,deletes=0;void destroy_resource()override{++destroys;}void delete_control()override{++deletes;}};
struct Factory final:s::ControlFactory {Owner owner;s::Control* allocate_04b0bc(std::uint32_t bytes)override{check(bytes==24,"actual lifecycle requests observed shared-owner allocation size");return &owner;}};
struct Runtime final:s::LifecycleRuntime {using LifecycleRuntime::LifecycleRuntime;s::Category miscellaneous_category_0285b0()override{unused();}[[noreturn]]void raise_028240(s::Error,std::string_view)override{unused();}};
void full_chain(){
    for(bool failed_post:{false,true})for(bool preexisting:{false,true}){
        Gate registry_gate,service_gate,engine_gate,fallback_gate;s::ServiceRegistry registry(registry_gate);Entry entry;RegistryEffects required(registry_gate,entry);
        if(preexisting){entry.key={0x1800a2620ULL,0};registry.head=&entry;}
        s::SocketService service(service_gate);service.completion_port=99;s::CancellationEngine engine(engine_gate);s::CloseContext context;context.engine=&engine;
        Errors errors;Acceptor api;Post completion;completion.fail=failed_post;s::CompletionScheduler scheduler(fallback_gate);scheduler.port=123;s::CancellationState queue;queue.scheduler=&scheduler;queue.wake.socket=errors.wake;
        s::OwnedRecycleMap storage(3);std::array<s::CancelOperation,3> operations;
        for(auto& op:operations){op.overlapped.Internal=0x7788;op.queued=99;storage.append(13,op);}auto* original=&storage.insert(13);queue.maps[0]=storage.map();
        std::array<s::RecycleMap*,1> metadata{&storage.metadata()};Failure failure;s::CancellationRecyclerAdapter recycler(metadata,failure);
        s::QueueCancellationEffects cancellation(engine,queue,recycler,completion,errors);std::array<s::RegistryServiceBinding,1> bindings{{{&entry,&service,&context}}};Executor executor;
        s::RegistryLifecycleEffects bridge(registry,{42},required,bindings,errors,api,cancellation,executor);s::AcceptorState state;s::SocketObject object(state);Factory factory;Runtime runtime(object,{42},bridge,factory);
        executor.expected=&object;executor.token=0x180084928ULL;
        executor.hook=[&]{check(!service.head && !object.next && !object.previous && state.socket==s::invalid_socket && !object.cancellation.control,"actual destroy unlinks/resets/releases before bridge executor callback");check(!engine_gate.depth && !service_gate.depth && !fallback_gate.depth && !registry_gate.depth,"all gates released before final executor callback");};
        engine_gate.hook=[&](bool enter){if(!enter)check(storage.metadata().free_size==1 && storage.metadata().active_size==0,"real recycler transfers node while cancellation engine gate held");};
        alignas(8)std::array<std::byte,128> endpoint{};endpoint[0]=std::byte{2};s::acceptor_020e20(state,endpoint,false,errors,api,runtime);
        check(service.head==&object && object.service==&service && state.service==&service && state.socket==13 && object.cancellation.control==&factory.owner,"actual acceptor resolves real registry and initializes selected lifecycle service");
        check(required.creates==static_cast<unsigned>(!preexisting) && required.comparisons==static_cast<unsigned>(preexisting) && registry.head==&entry,"preexisting and newly constructed registry paths converge without duplicate service");
        check(&bridge.resolve_service_0239c0({42})==&service && required.comparisons==1+static_cast<unsigned>(preexisting),"second service resolution uses cached published registry entry");
        state.socket_state=0x1b;runtime.destroy_socket_024db0();
        check(api.creates==1 && api.closes==1 && executor.calls==1 && factory.owner.destroys==1 && factory.owner.deletes==1,"actual full chain creates/closes socket token and releases owner exactly once");
        check(!storage.metadata().map_size && !storage.metadata().active_size && storage.metadata().free_size==1 && storage.free_sentinel().next==original && storage.active_sentinel().next==&storage.active_sentinel(),"actual cancellation uses real0283c0 and preserves host storage rings");
        check(completion.posted.size()==(failed_post?1:3) && errors.sends==1,"full chain posts operations or switches after first failure and wakes once");
        check(scheduler.head==(failed_post?&operations[0]:nullptr) && scheduler.tail==(failed_post?&operations[2]:nullptr) && scheduler.dispatch_required==(failed_post?1:0),"fallback chain preserves all three operation addresses");
        for(unsigned i=0;i<operations.size();++i){const auto& op=operations[i];check(op.error.value==995 && op.error.category.name_identity==0x9922 && op.error.category.logical==srv::ErrorCategory::system && op.overlapped.Internal==0x7788,"actual outer close supplies exact cancellation reason while preserving borrowed payload");check(op.queued==(!failed_post || !i?1U:99U) && op.next==(failed_post && i+1<operations.size()?&operations[i+1]:nullptr),"post success and fallback retain exact operation marker/link state");}
        check(!registry_gate.depth && !engine_gate.depth && !service_gate.depth && !fallback_gate.depth && registry_gate.enters==registry_gate.leaves && engine_gate.enters==1 && engine_gate.leaves==1,"actual acceptor through real registry and recycler balances independent gates");
        check(registry.head==&entry && !entry.deletes,"socket destruction leaves borrowed cached service entry alive");
        check(&storage.insert(0x10000000dULL)==original && storage.owned_nodes()==1,"real full-chain pooled node can be reused at exact address with full-width socket key");
    }
}
}
int main(){try{projection();full_chain();std::cout<<"PASS: "<<checks<<" socket service registry integration checks; borrowed projection and actual acceptor-to-registry-to-recycler chain with controlled effects only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
