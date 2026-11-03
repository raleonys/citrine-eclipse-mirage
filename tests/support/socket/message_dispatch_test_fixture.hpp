// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_message_dispatch.hpp"
#include "tests/support/socket/frame_read_test_fixture.hpp"

namespace message_test {
namespace s=preloader_l::socket;namespace b=buffer_test;namespace r=receive_test;
inline unsigned checks=0;
inline void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Failure {int phase;};
template<class E,class F>void rejects(F f){bool caught=false;try{f();}catch(const E&){caught=true;}check(caught,"required failure propagates");}
struct Fixture;
inline Fixture* running=nullptr;
void copy_first(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&);
void copy_second(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&);
struct Table:s::IoHandlerVtable {
    Fixture& f;std::string label;Table(Fixture& f,std::string label):IoHandlerVtable(0x777),f(f),label(std::move(label)){}
    void destroy_slot_00(s::IoHandlerEnvironment&)override;
    void move_slot_10(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&)override{r::unexpected();}
    std::uintptr_t query_slot_18(s::IoHandlerEnvironment&)override{r::unexpected();}
};
struct MemoryApi:frame_test::Api {
    Fixture& m;explicit MemoryApi(Fixture& m,frame_test::Fixture& base):frame_test::Api(base),m(m){}
    void free_sized_04b0f8(std::uintptr_t,std::uint64_t)override;
};
struct Api:s::MessageDispatchApi {
    Fixture& f;unsigned processes=0,schedules=0,empty=0,cleanups=0;s::MessageWriteTask* task=nullptr;
    s::ReceiveBufferString *input=nullptr,*response=nullptr;s::IoHandlerEnvironment *first=nullptr,*second=nullptr;
    explicit Api(Fixture& f):f(f){}
    void process_message_033d70(std::uintptr_t,s::ReceiveBufferString&,s::ReceiveBufferString&)override;
    void dispatch_response_021420(s::IoHandlerEnvironment&,s::MessageWriteTask&,std::uint64_t,std::uint64_t)override;
    void destroy_empty_message_00aa50(s::ReceiveBufferString&)override;
    void cleanup_exit_waits_04a47c()override;
};
struct Fixture {
    frame_test::Fixture base;b::Fixture request,response,extra;MemoryApi memory{*this,base};Api api{*this};
    std::array<s::WriteFinishAllocationHeader,16> headers{};std::size_t header_count=0;
    s::FrameReadContext frame{base.buffer,base.base.completion,memory,base.owners,base.categories,headers,base.terminal,{0x1111},&base.base.context};
    Table a{*this,"a"},bb{*this,"b"},c{*this,"c"};s::IoHandlerExecutor executor{},other_executor{};
    s::IoHandlerEnvironment source{{11,12},&a,3,0x456,&executor};
    s::MessageDispatchOwner owner{&base.projection,&source};
    std::array<s::MessageDispatchOwnerBinding,2> owners{{{0x12000,&owner},{}}};
    std::array<s::MessageEnvironmentCopyBinding,3> copies{{{&a,&copy_first},{&bb,&copy_second},{&c,&copy_second}}};
    std::array<s::MessageClosureBinding,2> closures{};
    s::MessageDispatchContext context{frame,api,owners,copies,closures};
    std::vector<std::string> trace;std::function<void(std::string)> hook;
    std::function<void(std::uintptr_t,s::ReceiveBufferString&,s::ReceiveBufferString&)> core_hook;
    std::function<void(s::IoHandlerEnvironment&,s::IoHandlerEnvironment&,unsigned)> copy_hook;
    unsigned copy_calls=0;std::vector<std::pair<std::uintptr_t,std::uint64_t>> frees;std::string scheduled;
    Fixture(){running=this;request.next_raw=0x1000003;response.next_raw=0x2000003;extra.next_raw=0x3000003;base.copied.next_raw=0x4000003;base.base.buffer.next_raw=0x5000003;seed(request,"request");seed(response,"reply");}
    void event(std::string name){trace.push_back(name);if(hook)hook(name);}
    void seed(b::Fixture& value,std::string text,std::uint64_t capacity=15){
        value.seed(std::max(capacity,static_cast<std::uint64_t>(text.size())),text.size());
        auto bytes=value.memory.view(s::receive_buffer_address(value.value),text.size());std::copy(text.begin(),text.end(),bytes.begin());
        if(value.value.capacity_18+1>=4096){const auto data=s::receive_buffer_address(value.value);headers[header_count++]={data-8,b::word(value.memory.view(data-8,8))};}
    }
    std::string text(const s::ReceiveBufferString& value){
        if(value.capacity_18<=15)return {reinterpret_cast<const char*>(value.storage_00.data()),static_cast<std::size_t>(value.size_10)};
        for(auto* pool:{&request,&response,&extra,&base.copied,&base.base.buffer})for(const auto& block:pool->blocks)if(block){
            const auto data=s::receive_buffer_address(value);if(data>=block->raw&&data-block->raw<=block->bytes.size()&&value.size_10<=block->bytes.size()-(data-block->raw)){
                auto bytes=pool->memory.view(data,value.size_10);return {reinterpret_cast<const char*>(bytes.data()),bytes.size()};
            }
        }
        throw std::runtime_error("fixture string not owned");
    }
    void free_string(s::ReceiveBufferString& value){
        if(value.capacity_18>15){auto data=s::receive_buffer_address(value);auto count=value.capacity_18+1;if(count>=4096){for(const auto& h:headers)if(h.address==data-8){data=h.allocation_base;break;}count+=39;}memory.free_sized_04b0f8(data,count);}
        value.size_10=0;value.capacity_18=15;value.storage_00[0]=0;
    }
    void dispatch(){running=this;s::dispatch_message_028d30(0x12000,request.value,context);}
};
inline void Table::destroy_slot_00(s::IoHandlerEnvironment&){f.event("destroy-"+label);}
inline void MemoryApi::free_sized_04b0f8(std::uintptr_t p,std::uint64_t n){
    m.frees.emplace_back(p,n);
    if(closure&&p==reinterpret_cast<std::uintptr_t>(closure.get())){
        check(n==48&&closure->message_00.size_10==0&&closure->message_00.capacity_18==15&&closure->message_00.storage_00[0]==0,"closure reset precedes exact sized delete48");
        m.event("free-closure");closure.reset();return;
    }
    m.event("free-string");
    for(auto* pool:{&m.request,&m.response,&m.extra,&m.base.copied,&m.base.base.buffer})for(const auto& block:pool->blocks)if(block&&block->raw==p){pool->api.free_sized_04b0f8(p,n);return;}
    throw std::runtime_error("fixture sized free not owned");
}
inline void Api::process_message_033d70(std::uintptr_t member,s::ReceiveBufferString& out,s::ReceiveBufferString& in){
    check(member==0x120f0,"literal owner+f0 handler identity");++processes;input=&in;response=&out;
    if(f.core_hook)f.core_hook(member,out,in);else out=f.response.value;f.event("process");
}
inline void Api::dispatch_response_021420(s::IoHandlerEnvironment& env,s::MessageWriteTask& value,std::uint64_t a,std::uint64_t b){
    ++schedules;second=&env;task=&value;check(!a&&!b&&value.function_00==0x180028270ULL,"exact task function identity and zero register arguments");
    check(value.object_28==f.base.projection.object_00,"post-CAS shared object captured");f.scheduled=f.text(value.response_08);f.event("schedule");
}
inline void Api::destroy_empty_message_00aa50(s::ReceiveBufferString& value){++empty;f.event("empty");f.free_string(value);}
inline void Api::cleanup_exit_waits_04a47c(){++cleanups;f.event("exit-waits");}
inline void copy_first(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from){
    auto& f=*running;check(!f.copy_calls&&&from==&f.source&&to.operations_10==&f.a&&to.schedule_20==f.source.schedule_20,"first executor preseed and source identity");
    f.api.first=&to;to.payload_00=from.payload_00;to.state_18=from.state_18;to.operations_10=&f.bb;++f.copy_calls;f.event("copy1");if(f.copy_hook)f.copy_hook(to,from,1);
}
inline void copy_second(s::IoHandlerEnvironment& to,s::IoHandlerEnvironment& from){
    auto& f=*running;check(f.copy_calls==1&&&from==f.api.first&&to.operations_10==from.operations_10&&to.schedule_20==from.schedule_20,"second executor uses live first table/schedule");
    to.payload_00=from.payload_00;to.state_18=from.state_18;++f.copy_calls;f.event("copy2");if(f.copy_hook)f.copy_hook(to,from,2);
}
} // namespace message_test
