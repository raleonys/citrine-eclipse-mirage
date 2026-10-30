#pragma once
#include "preloader_l/net/transport_owned_response.hpp"
#include "tests/support/socket/response_dispatch_test_fixture.hpp"

namespace owned_test {
namespace s=preloader_l::socket_recovered;namespace r=receive_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct OwnedFixture;inline OwnedFixture* active_owned=nullptr;
void publish_owned(s::IoHandlerEnvironment&,s::PostedFunctor*&);
struct Worker:s::WorkerRunApi {
    std::vector<std::uint32_t> indices;std::function<void*()> get;
    void* tls_get(std::uint32_t n)override{indices.push_back(n);return get?get():nullptr;}
    std::int32_t tls_set(std::uint32_t,void*)override{r::unexpected();}
    std::int32_t post_stop(std::uintptr_t,std::uint32_t,std::uintptr_t,OVERLAPPED*)override{r::unexpected();}
    std::uint32_t get_last_error()override{r::unexpected();}
};
struct Block {
    std::vector<std::byte> bytes;s::OperationStorage storage;std::unique_ptr<s::OwnedResponseTask> task;
    Block(std::uintptr_t id,std::size_t n):bytes(n,std::byte{0xa5}),storage{id,bytes}{}
};
struct Memory:s::OwnedResponseMemory {
    std::array<std::unique_ptr<Block>,16> blocks;std::array<s::OperationStorageBinding,16> bindings{};
    std::vector<std::uint64_t> allocations;std::vector<std::uintptr_t> frees;
    std::function<void()> allocating;std::function<void(s::OperationStorage&)> freeing;std::function<void()> projecting;
    std::uintptr_t next=0x100000;unsigned projected=0;
    Block& at(std::uintptr_t id){for(auto& p:blocks)if(p&&p->storage.identity==id)return *p;throw std::runtime_error("unowned allocation");}
    s::OperationStorage& allocate_04b0bc(std::uint64_t n)override{
        allocations.push_back(n);if(allocating)allocating();
        for(std::size_t i=0;i<blocks.size();++i)if(!blocks[i]){blocks[i]=std::make_unique<Block>(next,static_cast<std::size_t>(n));next+=0x1000;bindings[i]={blocks[i]->storage.identity,&blocks[i]->storage};return blocks[i]->storage;}
        r::unexpected();
    }
    void free_04b788(s::OperationStorage& value)override{
        const auto id=value.identity;frees.push_back(id);if(freeing)freeing(value);
        for(std::size_t i=0;i<blocks.size();++i)if(blocks[i]&&blocks[i]->storage.identity==id){blocks[i].reset();bindings[i]={};return;}
        r::unexpected();
    }
    s::PostedOperation& project_posted(s::OperationStorage&)override{r::unexpected();}
    s::OwnedResponseTask& project_owned_response(s::OperationStorage& p,s::OwnedResponseContext& c)override{
        if(projecting)projecting();++projected;auto& b=at(p.identity);b.task=std::make_unique<s::OwnedResponseTask>(p,c);return *b.task;
    }
    bool live(std::uintptr_t id)const{for(auto& p:blocks)if(p&&p->storage.identity==id)return true;return false;}
};
struct Api:s::RecoveredOwnedResponseApi {
    OwnedFixture& f;std::function<void(s::ReceiveBufferString&)> copying;unsigned copies=0;
    Api(s::OwnedResponseContext& c,OwnedFixture& f):RecoveredOwnedResponseApi(c),f(f){}
    s::ReceiveBufferString& copy_response_009eb0(s::ReceiveBufferString&,s::ReceiveBufferString&)override;
};
struct Ref:s::IoReference {
    unsigned disposes=0,deletes=0;std::function<void()> disposing,deleting;
    void dispose_slot_00()override{++disposes;if(disposing)disposing();}
    void delete_slot_08()override{++deletes;if(deleting)deleting();}
};
struct OwnedFixture {
    response_test::Fixture f;Worker worker;Memory memory;std::uint32_t index=17;
    s::WorkerThreadState state{};s::WorkerTlsContext tls{nullptr,&state,nullptr};
    s::OwnedResponseContext owned{{index,memory.bindings},worker,memory,f.m.frame};Api api{owned,*this};
    s::ResponseDispatchContext response{f.m.frame,f.sentinels,f.executors,f.schedules,api,&f.queue_context};
    s::PostedFunctor* output=nullptr;std::vector<s::PostedFunctor*> held;std::uintptr_t allocated_id=0;
    unsigned writes=0;std::string result;
    OwnedFixture(){active_owned=this;owned.response=&response;f.schedule.owned_slot_10=&publish_owned;}
    void make(){active_owned=this;api.make_owned_response_020cb0(output,f.task);allocated_id=static_cast<s::OwnedResponseTask*>(output)->storage.identity;}
    void dispatch(){active_owned=this;response_test::active=&f;s::dispatch_response_021420(f.source,f.task,response);}
    void use_tls(){worker.get=[&](){return &tls;};}
};
inline s::ReceiveBufferString& Api::copy_response_009eb0(s::ReceiveBufferString& out,s::ReceiveBufferString& in){++copies;if(copying)copying(in);return f.f.api.copy_response_009eb0(out,in);}
inline void publish_owned(s::IoHandlerEnvironment& env,s::PostedFunctor*& slot){
    auto& o=*active_owned;auto& f=o.f;check(&env==f.final&&slot,"owned publisher gets final environment and real functor");
    auto* task=static_cast<s::OwnedResponseTask*>(slot);o.allocated_id=task->storage.identity;
    check(task->callback_identity_00==0x180022280ULL,"real020cb0 publishes native022280 callback identity");
    f.event("publish");if(f.take_owned){auto* p=slot;slot=nullptr;if(f.invoke_owned)p->call_slot_00(1);else o.held.push_back(p);}
}
inline void capture(std::uintptr_t function,std::uintptr_t owner,s::ReceiveBufferString& value,s::ResponseDispatchContext&){
    auto& o=*active_owned;check(function==0x180028270ULL&&owner==0x12000,"0219d0 cached function and owner forwarded");
    ++o.writes;o.result=o.f.m.text(value);o.f.m.free_string(value);
}
} // namespace owned_test
