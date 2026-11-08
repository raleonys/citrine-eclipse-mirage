// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_response_containers.hpp"
#include "tests/support/socket/owned_response_test_fixture.hpp"
#include <cstring>

namespace container_test {
namespace s=citrine_eclipse_mirage::socket;namespace b=buffer_test;namespace r=receive_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure {int code;};
struct MapBlock {std::uintptr_t raw=0,address=0;std::vector<s::WriteFinishString*> cells;};
struct Bytes:s::ReceiveBufferApi {
    b::Fixture pool;std::array<s::ReceiveBufferRegion,256> regions{};s::ReceiveBufferMemory memory{regions};
    std::array<s::DequeMapBinding,32> bindings{};s::DequeMapMemory maps{bindings};
    std::array<std::unique_ptr<MapBlock>,32> map_blocks;
    std::array<s::WriteFinishAllocationHeader,64> headers{};std::size_t header_count=0;
    message_test::Fixture* messages=nullptr;std::function<void(std::string)> hook;
    struct Copy {std::uintptr_t to,from;std::uint64_t count;};std::vector<Copy> copies;
    std::vector<std::string> events;
    Bytes(){pool.next_raw=0x8000003;refresh();}
    void refresh(){
        regions.fill({});std::size_t n=0;
        auto add=[&](b::Fixture& p){for(auto item:p.regions)if(!item.bytes.empty())regions[n++]=item;};add(pool);
        if(messages)for(auto* p:{&messages->request,&messages->response,&messages->extra,&messages->base.copied,&messages->base.base.buffer})add(*p);
    }
    void event(std::string name){events.push_back(name);if(hook)hook(name);}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{
        event("allocate");const auto raw=pool.api.allocate_04b0bc(n);if(!raw)return 0;refresh();
        const auto address=n>=4096?(raw+39)&~std::uintptr_t{31}:raw;
        if(n>=4096)headers[header_count++]={address-8,raw};
        for(std::size_t i=0;i<map_blocks.size();++i)if(!map_blocks[i]){
            auto p=std::make_unique<MapBlock>();p->raw=raw;p->address=address;
            p->cells.resize(static_cast<std::size_t>((n>=4096?n-39:n)/8),reinterpret_cast<s::WriteFinishString*>(0xfeed));
            bindings[i]={address,p->cells};map_blocks[i]=std::move(p);return raw;
        }
        r::unexpected();
    }
    void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{
        copies.push_back({to,from,n});event("copy");if(!n)return;
        const auto dst=memory.view(to,n),src=memory.view(from,n);std::memmove(dst.data(),src.data(),dst.size());
    }
    void fill_bytes(std::uintptr_t to,std::uint8_t value,std::uint64_t n)override{event("fill");if(n){const auto dst=memory.view(to,n);std::fill(dst.begin(),dst.end(),value);}}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{
        event("free");pool.api.free_sized_04b0f8(p,n);
        for(std::size_t i=0;i<map_blocks.size();++i)if(map_blocks[i]&&map_blocks[i]->raw==p){map_blocks[i].reset();bindings[i]={};}
        refresh();
    }
    [[noreturn]]void length_error_00ce80()override{event("string-length");throw Failure{1};}
    [[noreturn]]void allocation_overflow_00cd30()override{event("overflow");throw Failure{2};}
    [[noreturn]]void invalid_parameter_05c974()override{event("invalid");throw Failure{3};}
    std::string text(const s::ReceiveBufferString& value){
        if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};
        auto bytes=memory.view(s::receive_buffer_address(value),value.size_10);return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
    }
    void destroy(s::ReceiveBufferString& value){
        if(value.capacity_18>15){auto data=s::receive_buffer_address(value);auto n=value.capacity_18+1;if(n>=4096){data=b::word(memory.view(data-8,8));n+=39;}free_sized_04b0f8(data,n);}
        value.size_10=0;value.capacity_18=15;value.storage_00[0]=0;
    }
};
struct Bounded:s::BoundedDequeGrowthApi {
    using BoundedDequeGrowthApi::BoundedDequeGrowthApi;
    [[noreturn]]void length_error_018460()override{throw Failure{4};}
};
struct MapApi:s::DequeGrowthApi {
    Bounded bounded;std::vector<Bytes::Copy> copies;std::vector<std::pair<std::uintptr_t,std::uint64_t>> zeros;
    std::function<void(std::string)> hook;unsigned length_errors=0;
    explicit MapApi(s::DequeMapMemory& m):bounded(m){}
    void copy_slots(std::uintptr_t to,std::uintptr_t from,std::uint64_t bytes)override{copies.push_back({to,from,bytes});bounded.copy_slots(to,from,bytes);if(hook)hook("copy"+std::to_string(copies.size()));}
    void zero_slots(std::uintptr_t to,std::uint64_t bytes)override{zeros.emplace_back(to,bytes);bounded.zero_slots(to,bytes);if(hook)hook("zero"+std::to_string(zeros.size()));}
    [[noreturn]]void length_error_018460()override{++length_errors;throw Failure{4};}
};
struct Fixture {
    Bytes bytes;s::ReceiveBufferContext strings{bytes.memory,bytes};MapApi api{bytes.maps};
    s::DequeGrowthContext context{strings,bytes.maps,api};s::WriteFinishOwner queue;
    std::array<s::WriteFinishString,1024> cells{};
    void seed_string(std::uint64_t size,std::uint64_t capacity){bytes.pool.seed(capacity,size);bytes.refresh();}
    void seed_map(std::uint64_t size,std::uint64_t offset=0){
        if(size){const auto n=size*8;const auto raw=bytes.allocate_04b0bc(n+(n>=4096?39:0));const auto data=n>=4096?(raw+39)&~std::uintptr_t{31}:raw;
            if(n>=4096)b::word(bytes.memory.view(data-8,8),raw);queue.map_d0=bytes.maps.view(data,n);
            for(std::size_t i=0;i<size;++i)queue.map_d0[i]=i%3?&cells[i]:nullptr;
        }
        queue.map_size_d8=size;queue.offset_e0=offset;queue.count_e8=size?size-1:0;
        bytes.pool.api.allocations.clear();bytes.pool.api.frees.clear();bytes.events.clear();
    }
    void grow(std::uint64_t extra){s::grow_response_deque_026670(queue,extra,context);}
};
struct ContainerApi final:s::ResponseContainersApi {
    ContainerApi(s::OwnedResponseContext& c,s::ReceiveBufferContext& strings):ResponseContainersApi(c,strings){}
};
struct QueueApi final:s::ResponseQueueApiAdapter {
    std::vector<std::unique_ptr<s::WriteFinishString>> cells;unsigned allocations=0;
    explicit QueueApi(s::DequeGrowthContext& c):ResponseQueueApiAdapter(c){}
    s::WriteFinishString* allocate_string_04b0bc(std::uint64_t n)override{check(n==32,"queue cell allocation is separate32-byte object effect");++allocations;cells.push_back(std::make_unique<s::WriteFinishString>());return cells.back().get();}
};
struct ContainerFixture;inline ContainerFixture* active_container=nullptr;
void borrowed_containers(s::IoHandlerEnvironment&,s::BorrowedResponse&);
struct ContainerFixture {
    owned_test::OwnedFixture ownership;response_test::Fixture& f=ownership.f;
    Bytes bytes;s::ReceiveBufferContext strings{bytes.memory,bytes};MapApi map_api{bytes.maps};
    s::DequeGrowthContext growth{strings,bytes.maps,map_api};ContainerApi api{ownership.owned,strings};QueueApi queue_api{growth};
    s::ResponseQueueContext queue{f.m.frame,queue_api,f.owners,&f.queued};
    s::ResponseDispatchContext response{f.m.frame,f.sentinels,f.executors,f.schedules,api,&queue};
    ContainerFixture(){active_container=this;ownership.owned.response=&response;f.queue={{},0,0,0};bytes.messages=&f.m;bytes.refresh();}
};
inline void borrowed_containers(s::IoHandlerEnvironment& env,s::BorrowedResponse& descriptor){
    auto& c=*active_container;check(&env==c.f.final&&descriptor.invoke_identity==0x180022240ULL,"borrowed response uses new COPY/queue source context");descriptor.invoke(c.response);
}
} // namespace container_test
