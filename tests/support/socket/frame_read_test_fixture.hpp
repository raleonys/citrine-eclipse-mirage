// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_frame_read.hpp"
#include "tests/support/socket/composed_read_test_fixture.hpp"
#include <cstring>

namespace frame_test {
namespace s=laureondo::socket;namespace b=buffer_test;namespace r=receive_test;
inline unsigned checks=0;
inline void check(bool v,const char* why){++checks;if(!v)throw std::runtime_error(why);}
struct Failure {int code;};
template<class E,class F>void rejects(F f){bool caught=false;try{f();}catch(const E&){caught=true;}check(caught,"required exact failure propagates");}
struct Terminal:s::FrameReadTerminal {unsigned calls=0;[[noreturn]]void bad_weak_ptr_026860()override{++calls;throw Failure{26};}};
struct Copy:s::ReceiveBufferApi {
    b::Fixture& b;std::function<void()> after;unsigned calls=0;std::uint64_t count=0;
    explicit Copy(b::Fixture& b):b(b){}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{return b.api.allocate_04b0bc(n);}
    void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{++calls;count=n;b.api.copy_bytes(to,from,n);if(after)after();}
    void fill_bytes(std::uintptr_t to,std::uint8_t v,std::uint64_t n)override{b.api.fill_bytes(to,v,n);}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{b.api.free_sized_04b0f8(p,n);}
    [[noreturn]]void length_error_00ce80()override{throw Failure{10};}
    [[noreturn]]void allocation_overflow_00cd30()override{throw Failure{11};}
    [[noreturn]]void invalid_parameter_05c974()override{throw Failure{12};}
};
struct Fixture;
struct Api:s::FrameReadApi {
    Fixture& f;std::function<void(std::string)> hook;bool null_allocation=false;
    std::unique_ptr<s::FrameMessageClosure> closure;std::vector<std::string> trace;
    std::uintptr_t handle=0xabc;std::uint32_t id=57;std::int32_t detached=0;unsigned restarts=0;
    s::WriteFinishStream initial{0x10000,0},returned{0x20000,-24};std::vector<std::pair<std::uintptr_t,std::uint64_t>> frees;
    s::ReceiveBufferString* message=nullptr;std::string delivered;
    explicit Api(Fixture& f):f(f){}
    void event(std::string n){trace.push_back(n);if(hook)hook(n);}
    void construct_message_006b30(s::ReceiveBufferString&,std::uintptr_t,std::uint64_t)override;
    s::FrameMessageClosure* allocate_frame_04b0bc(std::uint64_t n)override{check(n==48,"native closure is 48 bytes");event("allocate");if(null_allocation)return nullptr;closure=std::make_unique<s::FrameMessageClosure>();return closure.get();}
    std::uintptr_t begin_frame_thread_05e8e0(std::uintptr_t sec,std::uint32_t stack,std::uintptr_t entry,s::FrameMessageClosure* p,std::uint32_t flags,std::uint32_t& out)override{
        check(!sec&&!stack&&entry==0x180021770ULL&&!flags&&p==closure.get(),"exact opaque entry and nullable closure passed to required CRT wrapper");event("begin");out=id;return handle;
    }
    std::int32_t detach_04a458(const s::FrameThreadHandle& h)override{check(h.handle==handle&&h.id==id,"same returned handle and low32 id passed to detach");event("detach");return detached;}
    [[noreturn]]void thread_error_04a648(int code)override{event("terminal"+std::to_string(code));throw Failure{code};}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override;
    [[noreturn]]void invalid_parameter_05c974()override{event("invalid");throw Failure{12};}
    void next_write_02a500(std::uintptr_t)override{r::unexpected();}
    s::WriteFinishStream& begin_log_006880(std::uintptr_t global,std::uintptr_t prefix)override{check(global==0x1800a4130ULL&&prefix==0x1800845c8ULL,"read log exact global and distinct prefix");event("log");return initial;}
    s::WriteFinishStream& append_007990(s::WriteFinishStream& stream,std::uintptr_t p,std::uint64_t n)override;
    std::uint64_t widen_01e510(std::uintptr_t p,std::uint8_t c)override{check(p==0x1ffe8&&c==10,"signed32 stream adjustment on returned stream");event("widen");return 0x123456789abcde7fULL;}
    void put_00fe90(s::WriteFinishStream& stream,std::uint8_t c)override{check(&stream==&returned&&c==0x7f,"returned stream and low8 character");event("put");}
    void flush_00e1e0(s::WriteFinishStream& stream)override{check(&stream==&returned,"returned stream flushed");event("flush");}
};
struct Category:s::WriteFinishCategory {
    Fixture& f;unsigned calls=0;std::int32_t code=0;
    explicit Category(Fixture& f):f(f){identity_08=0x2222;}
    void message_slot_10(s::WriteFinishString&,std::int32_t)override;
};
using StartCapture=std::function<void(s::ReceiveHandler&,const s::Error&,std::uint64_t,std::uint32_t,s::ComposedReadContext&,s::ComposedReadStart)>;
inline StartCapture start_capture;
inline void capture_start(s::ReceiveHandler& h,const s::Error& e,std::uint64_t n,std::uint32_t flag,s::ComposedReadContext& c,s::ComposedReadStart call){start_capture(h,e,n,flag,c,call);}
inline std::function<void(std::uintptr_t)> restart_capture;
inline void capture_restart(std::uintptr_t id,s::FrameReadContext&,s::FrameReadComposedCall call){check(call==&s::composed_receive_025c40,"restart forwards actual composed read default");restart_capture(id);}
struct Fixture {
    read_test::Fixture base;b::Fixture copied;Copy copy{base.buffer};s::ReceiveBufferContext buffer{base.buffer.memory,copy};Api api{*this};Category category{*this};Terminal terminal;r::Ref owner;
    s::FrameReadOwner projection{0xfeed,&owner,&base.buffer.value};
    std::array<s::FrameReadOwnerBinding,2> owners{{{0x12000,&projection},{}}};
    std::array<s::WriteFinishCategoryBinding,3> categories{{{{0x2222},&category},{{0x3333},&category},{}}};
    std::array<s::WriteFinishAllocationHeader,2> headers{};
    s::FrameReadContext context{buffer,base.completion,api,owners,categories,headers,terminal,{0x1111},&base.context};
    Fixture(){base.handler.words_88_98[2]=0x12000;base.incoming={0,{0x2222}};restart_capture=[this](auto id){check(id==0x12000,"live frame owner forwarded to restart");++api.restarts;api.event("restart");};}
    void seed(std::string value,std::uint64_t capacity=15){base.seed(value,capacity);}
    void copied_seed(std::uint64_t capacity,std::uint64_t size){copied.seed(capacity,size);if(capacity+1>=4096){const auto p=s::receive_buffer_address(copied.value);headers[0]={p-8,b::word(copied.memory.view(p-8,8))};}}
    void finish(std::uint64_t n){s::finish_frame_read_025480(base.handler,base.incoming,n,context,&capture_restart);}
    void start(){s::start_frame_read_0280d0(0x12000,context,&capture_start);}
};
inline void Api::construct_message_006b30(s::ReceiveBufferString& out,std::uintptr_t source,std::uint64_t n){
    check(out.size_10==0&&out.capacity_18==0,"native copy destination starts with zero string prefix");
    auto bytes=f.base.buffer.memory.view(source,n);delivered.assign(reinterpret_cast<const char*>(bytes.data()),bytes.size());
    f.copied_seed(std::max<std::uint64_t>(15,n),n);auto target=f.copied.memory.view(s::receive_buffer_address(f.copied.value),n);std::copy(bytes.begin(),bytes.end(),target.begin());out=f.copied.value;message=&out;event("construct");
}
inline void Api::free_sized_04b0f8(std::uintptr_t p,std::uint64_t n){frees.emplace_back(p,n);event("free");f.copied.api.free_sized_04b0f8(p,n);}
inline s::WriteFinishStream& Api::append_007990(s::WriteFinishStream& stream,std::uintptr_t p,std::uint64_t n){check(&stream==&initial&&message&&p==s::receive_buffer_address(*message)&&n==message->size_10,"category message pointer/length passed before logger callbacks");event("append");return returned;}
inline void Category::message_slot_10(s::WriteFinishString& out,std::int32_t n){++calls;code=n;out=f.copied.value;f.api.message=&out;f.api.event("message");}
} // namespace frame_test
