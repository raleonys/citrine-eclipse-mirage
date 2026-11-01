// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "preloader_l/net/transport_composed_read.hpp"
#include "tests/support/socket/receive_buffer_test_fixture.hpp"
#include "tests/support/socket/async_receive_test_fixture.hpp"
#include <algorithm>

namespace read_test {
namespace s=preloader_l::socket;
namespace b=buffer_test;
namespace r=receive_test;
inline unsigned checks=0;
inline void check(bool value,const char* why){++checks;if(!value)throw std::runtime_error(why);}
struct LengthTerminal:std::runtime_error {LengthTerminal():std::runtime_error("synthetic composed-read length throw") {}};
struct Completion:s::IoCompletionEffects {
    unsigned misc=0;std::function<void()> hook;
    s::Category misc_category_0285b0()override{++misc;if(hook)hook();return {0x3333};}
    void receive_body_025c40(s::ReceiveHandler&,s::Error&,std::uint64_t,std::uint32_t)override{r::unexpected();}
    void send_body_0261d0(s::SendHandler&,s::Error&,std::uint64_t,std::uint32_t)override{r::unexpected();}
    void schedule_send_0229d0(s::IoHandlerEnvironment&,s::SendHandler&)override{r::unexpected();}
    void schedule_receive_022c60(s::IoHandlerEnvironment&,s::ReceiveHandler&)override{r::unexpected();}
};
struct Api:s::ComposedReadApi {
    unsigned finishes=0,constructs=0,throws=0;s::Error error;std::uint64_t matched=0;s::ReceiveHandler handler;
    s::ReadLengthError* exception=nullptr;std::function<void()> finish,construct;
    void finish_frame_025480(s::ReceiveHandler& h,s::Error& e,std::uint64_t n)override{
        ++finishes;handler=h;error=e;matched=n;if(finish)finish();
    }
    void construct_length_error_024390(s::ReadLengthError& e,std::uintptr_t message)override{
        check(message==0x180084840ULL,"exact native length error message identity");++constructs;exception=&e;if(construct)construct();
    }
    [[noreturn]]void throw_length_error_023930(s::ReadLengthError& e)override{
        check(exception==&e&&constructs==1,"same opaque exception is constructed before throw");++throws;throw LengthTerminal();
    }
};
struct Fixture;
inline Fixture* active=nullptr;
void capture(s::AsyncReceiveService&,s::ReceiveSocketImplementation&,s::ReceiveBufferSlice&,std::uint32_t,
    s::ReceiveHandler&,std::uintptr_t,s::AsyncReceiveContext&);
struct Fixture {
    b::Fixture buffer;r::Fixture io;Completion completion;Api api;s::ReceiveHandler handler;s::Error incoming{0,{0x2222}};
    std::array<s::ReceiveStringBinding,2> strings{{{0x5000,&buffer.value},{}}};
    std::array<s::ReceiveStreamBinding,2> streams{{{0x12340,&io.service,&io.implementation},{}}};
    s::ComposedReadContext context{buffer.context,io.context,completion,api,strings,streams};
    unsigned calls=0;s::ReceiveBufferSlice slice;std::uintptr_t allocator=0;std::function<void()> start;
    Fixture(){active=this;handler.words_60_78={0x12340,0x5000,UINT64_MAX,4096};handler.byte_80=10;handler.words_88_98={0,0,0xbeef};}
    void seed(std::string data,std::uint64_t capacity=15){
        buffer.seed(std::max(capacity,static_cast<std::uint64_t>(data.size())),data.size());
        auto bytes=buffer.memory.view(s::receive_buffer_address(buffer.value),data.size());
        std::copy(data.begin(),data.end(),bytes.begin());
    }
    void call(std::uint64_t bytes=0,std::uint32_t flag=1){active=this;s::composed_receive_025c40(handler,incoming,bytes,flag,context,&capture);}
};
inline void capture(s::AsyncReceiveService& service,s::ReceiveSocketImplementation& implementation,s::ReceiveBufferSlice& slice,
    std::uint32_t flags,s::ReceiveHandler&,std::uintptr_t allocator,s::AsyncReceiveContext& context){
    auto& f=*active;check(&service==&f.io.service&&&implementation==&f.io.implementation&&&context==&f.io.context&&flags==0,"actual receive callee signature receives selected members and flags0");
    ++f.calls;f.slice=slice;f.allocator=allocator;if(f.start)f.start();
}
} // namespace read_test
