// SPDX-License-Identifier: GPL-3.0-only

#include "tests/support/socket/composed_read_test_fixture.hpp"
using namespace read_test;
namespace {
void initial_scan(){
    for(std::uint64_t size:{0ULL,1ULL,7ULL,15ULL,16ULL,31ULL,64ULL})
    for(std::uint8_t delimiter:{std::uint8_t{0},std::uint8_t{10},std::uint8_t{255}})
    for(int where:{-1,0,3,14,30,63})for(std::uint64_t cursor:{0ULL,1ULL,7ULL,100ULL})for(int error:{0,9}){
        if(size==0&&cursor!=0)continue;
        Fixture f;std::string data(size,'x');if(where>=0&&static_cast<std::uint64_t>(where)<size)data[where]=static_cast<char>(delimiter);
        f.seed(data);f.handler.byte_80=delimiter;f.handler.words_60_78[3]=size;f.handler.words_88_98[0]=cursor;f.incoming.value=error;
        const auto begin=std::min(cursor,size);const auto pos=data.find(static_cast<char>(delimiter),static_cast<std::size_t>(begin));
        const auto expected=pos==std::string::npos?UINT64_MAX:static_cast<std::uint64_t>(pos)+1;
        f.call();
        check(f.calls==1&&!f.api.finishes&&!f.completion.misc&&f.slice.size==0,"flag1 defers even found/limit/error via zero receive");
        check(f.handler.words_88_98[0]==expected&&f.handler.words_88_98[1]==0&&f.handler.dword_84==1,"independent string::find scan/result oracle");
        check(f.allocator==0x123a8&&f.slice.data==(size?s::receive_buffer_address(f.buffer.value)+size:0),"literal allocator token and zero slice at effective end");
    }
}
void requests(){
    struct Case{std::uint64_t size,capacity,maximum,expected;};
    const Case cases[]={{0,15,4096,512},{7,15,4096,512},{7,1024,4096,1017},{16,100000,200000,65536},
        {7,15,8,1},{7,15,518,511},{7,15,519,512},{7,1024,65543,1017},{16,100000,65551,65535},{16,100000,65552,65536}};
    for(const auto& c:cases){
        Fixture f;f.seed(std::string(c.size,'x'),c.capacity);f.handler.words_60_78[3]=c.maximum;f.call();
        check(f.calls==1&&f.slice.size==c.expected&&f.handler.words_88_98[1]==c.expected&&f.handler.words_88_98[0]==c.size,"explicit512/spare/remaining/65536 anchors");
        check(f.buffer.value.size_10==c.size+c.expected&&f.buffer.text().substr(0,c.size)==std::string(c.size,'x'),"actual010040 prepares extent and retains prefix");
        check(f.slice.data==s::receive_buffer_address(f.buffer.value)+c.size,"prepared buffer pointer uses cached effective offset");
    }
    Fixture clipped;clipped.seed("abcdef\nrest",32);clipped.handler.words_60_78[2]=3;clipped.handler.words_60_78[3]=4096;clipped.call();
    check(clipped.handler.words_88_98[0]==3&&clipped.slice.size==512&&clipped.slice.data==s::receive_buffer_address(clipped.buffer.value)+3,"fixed effective size hides later delimiter");
}
void continuation(){
    Fixture found;found.seed("oldabc\nxxxx",32);found.handler.words_88_98={3,8,0xbeef};found.call(4,0);
    check(found.buffer.text()=="oldabc\n"&&!found.calls&&found.api.finishes==1&&found.api.matched==7&&found.api.error.value==0&&found.handler.words_88_98[1]==0,"continuation commits before searching only new bytes");
    Fixture more;more.seed("oldabcxxxxx",32);more.handler.words_88_98={3,8,0xbeef};more.call(3,0);
    check(more.calls==1&&!more.api.finishes&&more.handler.words_88_98[0]==6&&more.buffer.value.size_10==518&&more.slice.size==512,"partial frame reuses actual resize and continues receiving");
    for(int code:{0,5})for(std::uint64_t cursor:{4ULL,UINT64_MAX}){
        Fixture f;f.seed("dataxxxxxxxx",32);f.handler.words_88_98={cursor,8,0xbeef};f.incoming.value=code;f.call(0,0);
        check(f.buffer.text()=="data"&&!f.calls&&f.api.finishes==1&&f.handler.words_88_98[1]==8,"zero bytes early finish keeps pending request and commits unused extent");
        check(f.api.error.value==(cursor==UINT64_MAX?3:code)&&f.api.matched==(code==0&&cursor!=UINT64_MAX?cursor:0)&&f.completion.misc==(cursor==UINT64_MAX?1U:0U),"sentinel overrides local error; incoming error suppresses result");
    }
    Fixture over;over.seed("dataxxxx",15);over.handler.words_88_98={0,4,0};over.incoming.value=9;over.call(5,0);
    check(over.buffer.value.size_10==0&&over.api.error.value==9,"unsigned request-bytes underflow saturates commit to0 before error finish");
    Fixture flag2;flag2.seed("abc\nxxxx",15);flag2.handler.words_88_98={0,8,0};flag2.call(4,2);
    check(flag2.calls==1&&flag2.slice.size==0&&!flag2.api.finishes&&flag2.handler.dword_84==2,"flag2 commits like continuation but defers like nonzero flag");
}
void category_and_lifetimes(){
    Fixture f;f.seed("",15);f.handler.words_60_78[3]=0;f.handler.words_88_98[0]=UINT64_MAX;f.incoming.value=9;
    f.completion.hook=[&]{f.handler.words_88_98[0]=7;f.incoming.value=0;};f.call(0,0);
    check(f.api.error.value==3&&f.api.error.category.name_identity==0x3333&&f.api.matched==7,"category decision precedes callback; result/input code reload follows it");
    Fixture destroyed;destroyed.seed("abc\n",15);auto handler=std::make_unique<s::ReceiveHandler>(destroyed.handler);
    handler->words_88_98={0,4,0};destroyed.api.finish=[&]{handler.reset();};
    s::composed_receive_025c40(*handler,destroyed.incoming,4,0,destroyed.context,&capture);
    check(!handler&&destroyed.api.matched==4,"frame completion may end handler lifetime before body returns");
    Fixture asynchronous;asynchronous.seed("abc\n",15);auto pending=std::make_unique<s::ReceiveHandler>(asynchronous.handler);asynchronous.start=[&]{pending.reset();};
    s::composed_receive_025c40(*pending,asynchronous.incoming,0,1,asynchronous.context,&capture);
    check(!pending&&asynchronous.calls==1,"receive callee may end caller handler lifetime; no later body access");
}
void live_reload_and_terminals(){
    Fixture f;f.seed("abcdefg",15);b::Fixture replacement;replacement.next_raw=0x90003;replacement.seed(32,12);
    f.strings[1]={0x6000,&replacement.value};const auto new_stream=std::numeric_limits<std::uintptr_t>::max()-0x20;
    f.streams[1]={new_stream,&f.io.service,&f.io.implementation};
    f.buffer.api.on_allocate=[&]{f.handler.words_60_78[0]=new_stream;f.handler.words_60_78[1]=0x6000;f.handler.words_60_78[3]=17;f.handler.words_88_98[1]=99;};f.call();
    check(f.buffer.value.size_10==519&&f.slice.data==s::receive_buffer_address(replacement.value)+7&&f.slice.size==5,"post-resize live string/request/max and cached old offset select SDK slice");
    check(f.allocator==new_stream+std::uintptr_t{0x68},"literal allocator token wraps at pointer width");
    Fixture invalid;invalid.seed("abc",15);invalid.handler.words_60_78[2]=100;invalid.handler.words_60_78[3]=10;
    b::rejects<LengthTerminal>([&]{invalid.call();});
    check(invalid.api.constructs==1&&invalid.api.throws==1&&!invalid.calls&&!invalid.api.finishes&&invalid.buffer.value.size_10==3,"construct024390 then throw023930 before prepare or receive");
    Fixture negative;negative.seed("x\n",15);negative.handler.words_60_78[3]=2;negative.handler.words_88_98[0]=UINT64_MAX;negative.call();
    check(negative.handler.words_88_98[0]==2,"single-buffer negative advance from begin clamps to begin");
    Fixture empty;empty.seed("",15);empty.handler.words_88_98[0]=1;b::rejects<s::UnsupportedInput>([&]{empty.call();});
    check(empty.handler.dword_84==1&&!empty.calls,"non-progressing empty iterator state rejected explicitly");
    Fixture missing;missing.seed("abc",15);missing.strings[0].string=nullptr;b::rejects<s::UnsupportedInput>([&]{missing.call();});
}
}
int main(){try{initial_scan();requests();continuation();category_and_lifetimes();live_reload_and_terminals();std::cout<<"PASS: "<<checks+b::checks+r::checks<<" composed read core checks; actual025c40 with010040/008350, independent delimiter oracle and explicit request anchors, sentinel/flags/commit/error/category reloads/terminal ordering/lifetime; isolated021d10 capture only\n";}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
