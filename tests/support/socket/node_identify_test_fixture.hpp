// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/node_identify.hpp"
#include "tests/support/socket/node_parse_test_fixture.hpp"

namespace node_identify_test {
namespace s=laureondo::socket;
using node_parse_test::check;using node_parse_test::checks;using node_parse_test::put;using node_parse_test::word;
using node_parse_test::Failure;using node_parse_test::fails;using node_parse_test::unmapped;
using Memory=node_parse_test::Memory;
struct Constructors:s::NativeNodeIdentifyEffects {
    struct Call {unsigned address;std::uintptr_t document,pool;};
    std::vector<Call> calls;std::function<std::uintptr_t(const Call&)> hook;
    std::uintptr_t invoke(unsigned a,std::uintptr_t d,std::uintptr_t p){calls.push_back({a,d,p});return hook?hook(calls.back()):Memory::child;}
    std::uintptr_t construct_001610(std::uintptr_t d,std::uintptr_t p)override{return invoke(0x1610,d,p);}
    std::uintptr_t construct_001590(std::uintptr_t d,std::uintptr_t p)override{return invoke(0x1590,d,p);}
    std::uintptr_t construct_001720(std::uintptr_t d,std::uintptr_t p)override{return invoke(0x1720,d,p);}
    std::uintptr_t construct_0017b0(std::uintptr_t d,std::uintptr_t p)override{return invoke(0x17b0,d,p);}
    std::uintptr_t construct_001690(std::uintptr_t d,std::uintptr_t p)override{return invoke(0x1690,d,p);}
};
struct Fixture {
    static constexpr std::uintptr_t slots=0x1800a10a8ULL,out=Memory::scratch_address+0x80;
    static constexpr std::uintptr_t token(unsigned i){return 0xfc000+i*0x10;}
    Memory raw;parse_entry_test::Lower characters;node_parse_test::Lower parser{raw};Constructors constructors;
    std::array<std::uint8_t,40> tokens{};std::size_t token_bytes=40;
    std::vector<s::ReceiveBufferRegion> extra_regions;
    s::NativeNodeIdentifyContext context{raw.memory,characters,parser,constructors};
    Fixture(){for(unsigned i=0;i<5;++i)put(tokens,i*8,token(i));characters.classifier=parse_entry_test::controlled_space;
        parser.compare_hook=[](const auto&){return 1;};raw.objects[1].fill(0xa5);refresh();raw.set(out,0xfeedfeedfeedfeedULL);}
    void refresh(){raw.extras=extra_regions;raw.extras.push_back({slots,std::span(tokens).first(token_bytes)});raw.refresh();}
    void input(std::string_view v){raw.input.fill(0);std::copy(v.begin(),v.end(),raw.input.begin());}
    std::uintptr_t identify(std::uintptr_t input=Memory::input_address,std::uintptr_t output=out,std::uintptr_t document=Memory::doc){return s::identify_node_0038b0(document,input,output,context);}
    void match(unsigned chosen){parser.compare_hook=[chosen](const auto& c){return c.right==token(chosen)?0:1;};}
};
} // namespace node_identify_test
