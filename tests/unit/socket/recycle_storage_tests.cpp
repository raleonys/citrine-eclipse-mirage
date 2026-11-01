// SPDX-License-Identifier: GPL-3.0-only

#include "preloader_l/net/transport_recycle_storage.hpp"
#include <algorithm>
#include <iostream>
#include <map>
#include <set>
#include <type_traits>
#include <vector>

namespace s=preloader_l::socket;
namespace srv=preloader_l::server;
namespace {
unsigned checks=0;
void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
[[noreturn]]void unused(){throw std::logic_error("Unexpected host integration effect");}
struct Failure final:s::RecycleFailureEffects {[[noreturn]]void length_error_047fc4(std::uintptr_t)override{unused();}};
struct Gate final:s::SocketGate {int depth=0;unsigned enters=0,leaves=0;void enter()override{check(!depth,"gate not recursively acquired");++depth;++enters;}void leave()override{check(depth==1,"gate held on release");--depth;++leaves;}};
std::vector<s::CancelNode*> traverse(s::CancelNode& sentinel,std::size_t bound){
    std::vector<s::CancelNode*> found;auto* previous=&sentinel;
    for(auto* n=sentinel.next;n!=&sentinel;n=n->next){check(n && found.size()<bound && n->previous==previous,"bounded circular list has consistent previous links");check(std::find(found.begin(),found.end(),n)==found.end(),"owned node appears once per list");found.push_back(n);previous=n;}
    check(sentinel.previous==previous && previous->next==&sentinel,"sentinel closes circular list");return found;
}
void invariant(s::OwnedRecycleMap& owned,const std::map<s::Socket,s::CancelNode*>& expected){
    const auto active=traverse(owned.active_sentinel(),owned.owned_nodes()+1),free=traverse(owned.free_sentinel(),owned.owned_nodes()+1);
    auto& metadata=owned.metadata();check(metadata.map_size==expected.size() && metadata.active_size==active.size() && metadata.free_size==free.size(),"logical counts agree with externally expected keys and lists");
    check(active.size()+free.size()==owned.owned_nodes(),"every owned node is active or pooled");std::set<s::CancelNode*> addresses(active.begin(),active.end());addresses.insert(free.begin(),free.end());check(addresses.size()==owned.owned_nodes(),"active and free nodes are disjoint");
    for(auto* node:active){auto wanted=expected.find(node->socket);check(wanted!=expected.end() && wanted->second==node,"active keys map to exact stable expected addresses");}
    for(auto* node:free)check(node->socket==0,"pooled keys cleared by recovered recycler");
    auto& map=owned.map();
    for(std::size_t i=0;i<map.buckets.size();++i){
        std::vector<s::CancelNode*> filtered;for(auto* node:active)if(node->socket%map.buckets.size()==i)filtered.push_back(node);
        auto& bucket=map.buckets[i];
        if(filtered.empty()){check(bucket.first==map.sentinel && bucket.last==map.sentinel,"empty bucket uses active sentinel for both endpoints");continue;}
        check(bucket.first==filtered.front() && bucket.last==filtered.back(),"bucket endpoints match all active keys of that remainder");
        auto begin=std::find(active.begin(),active.end(),filtered.front());check(std::equal(filtered.begin(),filtered.end(),begin),"collision range is contiguous within global ring");
    }
}
struct Api final:s::Api {
    unsigned sends=0,categories=0;std::int32_t error=0;s::Socket wake=0x9988776655443322ULL;
    void set_last_error(std::int32_t value)override{error=value;}
    s::Category system_category_028620()override{++categories;return {0x9922,srv::ErrorCategory::system};}
    std::int32_t get_last_error()override{return error;}
    s::Conversion convert_acp(const char*,std::int32_t)override{unused();}
    s::AddressCall string_to_address(s::Conversion&,std::int32_t,std::int32_t)override{unused();}
    s::SendCall wsa_send(s::Socket socket,std::span<s::Buffer> buffer,std::uint32_t flags)override{check(socket==wake && buffer.size()==1 && buffer[0].len==1 && buffer[0].buf[0]==0 && flags==0,"only injected one-byte wake on selected identity");++sends;return {0,1};}
    std::int32_t select_write(s::Socket,std::int32_t)override{unused();}
};
struct Post final:s::CompletionApi {
    std::vector<s::CancelOperation*> posted;bool fail=false;
    std::int32_t post(std::uintptr_t port,s::CancelOperation& operation)override{check(port==123 && operation.queued==1 && !operation.next,"real cancellation posts detached marked operations using selected scheduler");posted.push_back(&operation);return fail?0:1;}
};
void storage(){
    static_assert(!std::is_copy_constructible_v<s::OwnedRecycleMap> && !std::is_move_constructible_v<s::OwnedRecycleMap>);
    bool caught=false;try{s::OwnedRecycleMap zero(0);}catch(const s::UnsupportedInput&){caught=true;}check(caught,"host storage rejects zero buckets");
    for(std::size_t buckets:{std::size_t{1},std::size_t{3},std::size_t{7},std::size_t{31}}){
        s::OwnedRecycleMap owned(buckets);std::map<s::Socket,s::CancelNode*> expected;Failure failure;invariant(owned,expected);
        const std::array<s::Socket,12> keys{0,0xffffffffffffffffULL,0x100000001ULL,7,14,21,28,1,2,3,0xfedcba9876543210ULL,0x8000000000000000ULL};
        for(auto key:keys){auto& node=owned.insert(key);expected.emplace(key,&node);check(node.socket==key && !node.head && !node.tail,"host insertion initializes selected key and empty borrowed FIFO");invariant(owned,expected);}
        for(auto key:keys){auto* previous=expected.at(key);const auto count=owned.owned_nodes();check(&owned.insert(key)==previous && owned.owned_nodes()==count,"duplicate insert preserves node and allocation count");invariant(owned,expected);}
        std::vector<s::CancelNode*> released;
        for(std::size_t index=0;index<keys.size();index+=2){auto* node=expected.at(keys[index]);s::recycle_0283c0(owned.metadata(),*node,failure);expected.erase(keys[index]);released.push_back(node);invariant(owned,expected);}
        for(unsigned i=0;i<released.size();++i){const s::Socket key=0x1234567800000000ULL+i;auto& node=owned.insert(key);check(&node==released[released.size()-1-i] && owned.owned_nodes()==keys.size(),"free pool reused LIFO without allocating and with stable original node address");expected.emplace(key,&node);invariant(owned,expected);}
        // Test all remaining keys through a different removal order.
        while(!expected.empty()){auto it=std::prev(expected.end());auto* node=it->second;s::recycle_0283c0(owned.metadata(),*node,failure);expected.erase(it);invariant(owned,expected);}
    }
}
void cancellation(){
    for(bool fail:{false,true}){
        s::OwnedRecycleMap a(1),b(3),c(5),d(7);std::array<s::OwnedRecycleMap*,4> owned{&a,&b,&c,&d};
        Gate gate;s::CompletionScheduler scheduler(gate);scheduler.port=123;s::CancellationState state;state.scheduler=&scheduler;Api api;state.wake.socket=api.wake;Post post;post.fail=fail;Failure failure;
        std::array<s::RecycleMap*,4> metadata{};std::array<std::array<s::CancelOperation,2>,4> operations{};std::array<s::CancelNode*,4> nodes{};
        const s::Socket key=0xabcdef0123456789ULL;std::vector<s::CancelOperation*> expected;
        for(unsigned i=0;i<4;++i){auto& map=*owned[i];for(auto& operation:operations[i]){operation.queued=99;operation.overlapped.Internal=0x7788;map.append(key,operation);expected.push_back(&operation);}nodes[i]=&map.insert(key);state.maps[i]=map.map();metadata[i]=&map.metadata();}
        s::CancellationRecycler recycler(metadata,failure);s::cancel_queue_026aa0(state,key,{995,{0x5555,srv::ErrorCategory::other}},recycler,post,api);
        check(post.posted.size()==(fail?1:8) && api.sends==1,"four owned maps aggregate completions and wake once");
        check(scheduler.head==(fail?expected.front():nullptr) && scheduler.tail==(fail?expected.back():nullptr) && scheduler.dispatch_required==(fail?1:0),"real recovered recycler composes with both queue delivery outcomes");
        for(unsigned i=0;i<4;++i){invariant(*owned[i],{});check(!nodes[i]->head && !nodes[i]->tail,"real cancellation drains FIFO before moving node into free pool");auto& node=owned[i]->insert(key+1);check(&node==nodes[i] && owned[i]->owned_nodes()==1,"cancellation-recycled node is immediately reusable without growth");invariant(*owned[i],{{key+1,&node}});}
        for(unsigned i=0;i<expected.size();++i){const auto* op=expected[i];check(op->error.value==995 && op->error.category.name_identity==0x5555 && op->error.category.logical==srv::ErrorCategory::other && op->overlapped.Internal==0x7788,"cancel reason copied and borrowed SDK payload preserved");check(op->queued==(!fail || i==0?1U:99U),"fallback unattempted operations keep original marker");check(op->next==(fail && i+1<expected.size()?expected[i+1]:nullptr),"completion FIFO links survive node reuse");}
        check(!gate.depth && gate.enters==static_cast<unsigned>(fail) && gate.leaves==gate.enters,"fallback gate balanced and used only on failed delivery");
    }
    // Storage owns nodes only; destruction must leave caller-owned operations live.
    s::CancelOperation borrowed;borrowed.queued=812;{s::OwnedRecycleMap temporary(1);temporary.append(0,borrowed);}check(borrowed.queued==812 && !borrowed.next,"host destruction does not destroy borrowed operation data");
}
struct Owner final:s::Control {unsigned destroys=0,deletes=0;void destroy_resource()override{++destroys;}void delete_control()override{++deletes;}};
struct Factory final:s::ControlFactory {Owner owner;s::Control* allocate_04b0bc(std::uint32_t bytes)override{check(bytes==24,"lifecycle requests observed control size");return &owner;}};
struct Acceptor final:s::AcceptorApi {
    unsigned closes=0;
    s::Socket create_socket(std::int32_t family)override{check(family==2,"real acceptor uses IPv4");return 13;}
    std::uintptr_t attach_completion_port(s::Socket socket,std::uintptr_t port)override{check(socket==13 && port==99,"real acceptor passes selected IOCP identity");return 99;}
    std::uint32_t get_win32_error()override{unused();}
    std::int32_t set_option(s::Socket socket,std::int32_t level,std::int32_t name,std::int32_t value)override{check(socket==13 && level==65535 && name==128 && value==0,"real low-level close uses observed linger request");return 0;}
    std::int32_t bind_socket(s::Socket socket,std::span<const std::byte>,std::int32_t size)override{check(socket==13 && size==16,"real acceptor binds observed IPv4 size");return 0;}
    std::int32_t listen_socket(s::Socket socket,std::int32_t backlog)override{check(socket==13 && backlog==2147483647,"real acceptor uses observed listen backlog");return 0;}
    std::int32_t close_socket(s::Socket socket)override{check(socket==13,"real close retains selected socket");++closes;return 0;}
    std::int32_t set_nonblocking(s::Socket,std::uint32_t,std::uint32_t&)override{unused();}
};
struct Effects final:s::ClosingLifecycleEffects {
    s::SocketService& service;unsigned cleaned=0;
    Effects(s::SocketService& service,s::CloseContext& context,Api& errors,Acceptor& api,s::CancellationEffects& cancel):ClosingLifecycleEffects(service,context,errors,api,cancel),service(service){}
    s::SocketService& resolve_service_0239c0(s::ContextIdentity identity)override{check(identity.value==42,"real lifecycle resolves selected context");return service;}
    void executor_cleanup(s::SocketObject& object,std::uintptr_t table)override{check(!service.head && !object.next && !object.previous && object.state.socket==s::invalid_socket && !object.cancellation.control && table==0x180084928ULL,"real destruction resets/unlinks/releases before executor cleanup");++cleaned;}
};
struct Runtime final:s::LifecycleRuntime {using LifecycleRuntime::LifecycleRuntime;s::Category miscellaneous_category_0285b0()override{unused();}[[noreturn]]void raise_028240(s::Error,std::string_view)override{unused();}};
void full_chain(){
    for(bool fail:{false,true}){
        Gate engine_gate,service_gate,fallback_gate;s::CancellationEngine engine(engine_gate);s::CloseContext context;context.engine=&engine;
        s::CompletionScheduler scheduler(fallback_gate);scheduler.port=123;s::CancellationState queue;queue.scheduler=&scheduler;Api errors;queue.wake.socket=errors.wake;Post completion;completion.fail=fail;
        s::OwnedRecycleMap storage(3);s::CancelOperation operation;storage.append(13,operation);auto* original=&storage.insert(13);queue.maps[0]=storage.map();std::array<s::RecycleMap*,1> maps{&storage.metadata()};Failure failure;s::CancellationRecycler recycler(maps,failure);
        s::QueueCancellationEffects cancel(engine,queue,recycler,completion,errors);s::SocketService service(service_gate);service.completion_port=99;Acceptor api;Effects effects(service,context,errors,api,cancel);s::AcceptorState state;s::SocketObject object(state);Factory factory;Runtime runtime(object,{42},effects,factory);
        alignas(8)std::array<std::byte,128> endpoint{};endpoint[0]=std::byte{2};s::acceptor_020e20(state,endpoint,false,errors,api,runtime);
        check(service.head==&object && state.socket==13 && object.cancellation.control==&factory.owner,"actual acceptor/lifecycle creates object and shared cancellation owner");
        state.socket_state=0x1b;runtime.destroy_socket_024db0();invariant(storage,{});
        check(api.closes==1 && effects.cleaned==1 && factory.owner.destroys==1 && factory.owner.deletes==1,"actual destructor closes and releases owner once");
        check(completion.posted==std::vector<s::CancelOperation*>{&operation} && errors.sends==1 && operation.error.value==995 && operation.error.category.name_identity==0x9922,"actual close produces cancellation reason and real queue/recycler delivers operation");
        check(scheduler.head==(fail?&operation:nullptr) && scheduler.dispatch_required==(fail?1:0),"full source chain preserves success/fallback result");
        check(!engine_gate.depth && !service_gate.depth && !fallback_gate.depth && engine_gate.enters==1 && engine_gate.leaves==1,"full source chain balances locking around actual cancellation");
        check(&storage.insert(0x10000000dULL)==original && storage.owned_nodes()==1,"full destructor recycler output reused with full-width replacement key");
    }
}
}
int main(){try{storage();cancellation();full_chain();std::cout<<"PASS: "<<checks<<" socket recycler storage checks; selected host storage and actual source chain with injected effects only\n";}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}}
