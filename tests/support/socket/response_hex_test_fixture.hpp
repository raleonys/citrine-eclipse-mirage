// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_response_hex.hpp"
#include "tests/support/socket/request_growth_test_fixture.hpp"

namespace response_hex_test {
namespace s=laureondo::socket;namespace b=buffer_test;namespace r=receive_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure {int code;};
struct Effects:mutation_test::Copy {
    struct Fill{std::uintptr_t address;std::uint8_t byte;std::uint64_t count;};std::vector<Fill> fills;
    std::function<void()> before_fill,after_fill;
    explicit Effects(b::Fixture& f):mutation_test::Copy(f){}
    void fill_bytes(std::uintptr_t to,std::uint8_t value,std::uint64_t n)override{fills.push_back({to,value,n});if(before_fill)before_fill();mutation_test::Copy::fill_bytes(to,value,n);if(after_fill)after_fill();}
};
struct Fixture {
    b::Fixture f;Effects effects{f};s::ReceiveBufferContext context{f.memory,effects};
    void seed(std::string_view bytes,std::uint64_t capacity){f.seed(capacity,bytes.size());auto out=f.memory.view(s::receive_buffer_address(f.value),bytes.size());std::copy(bytes.begin(),bytes.end(),out.begin());}
    void encode(){s::encode_response_hex_03d190(f.value,context);}
    std::string text(){return f.text();}
};
inline std::string oracle(std::string_view bytes){const std::string digits="0123456789abcdef";std::string result;for(unsigned char value:bytes){result.push_back(digits.at(value/16));result.push_back(digits.at(value%16));}return result;}
// Controlled response preparation around N5's normal-return constructor.
// This is a test caller, not the native033d70 response-local orchestration.
struct ResponseBytes:s::ReceiveBufferApi {
    container_test::ContainerFixture& c;s::ReceiveBufferString* prefix=nullptr;
    explicit ResponseBytes(container_test::ContainerFixture& c):c(c){auto& pool=c.f.m.response;pool.api.on_allocate=[&pool](){for(const auto& p:pool.blocks)if(p)pool.next_raw=std::max(pool.next_raw,p->raw+p->bytes.size()+0x1000);};}
    void refresh(){c.bytes.refresh();if(prefix)c.bytes.regions[253]={reinterpret_cast<std::uintptr_t>(prefix->storage_00.data()),prefix->storage_00};}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{const auto raw=c.f.m.response.api.allocate_04b0bc(n);if(raw&&n>=4096){const auto data=(raw+39)&~std::uintptr_t{31};c.f.m.headers[c.f.m.header_count++]={data-8,raw};}refresh();return raw;}
    void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{c.bytes.copy_bytes(to,from,n);}
    void fill_bytes(std::uintptr_t to,std::uint8_t value,std::uint64_t n)override{c.bytes.fill_bytes(to,value,n);}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{c.f.m.response.api.free_sized_04b0f8(p,n);refresh();}
    [[noreturn]]void length_error_00ce80()override{throw Failure{10};}
    [[noreturn]]void allocation_overflow_00cd30()override{throw Failure{11};}
    [[noreturn]]void invalid_parameter_05c974()override{throw Failure{12};}
};
struct WireStrings:s::MessageCoreStrings {
    core_test::Fixture& core;ResponseBytes effects;s::ReceiveBufferContext context;
    bool enabled=false;unsigned encodes=0;
    explicit WireStrings(core_test::Fixture& core):core(core),effects(core.containers),context{core.containers.bytes.memory,effects}{}
    std::string read_request(const s::ReceiveBufferString& in)override{return core.strings.read_request(in);}
    void replace_request(s::ReceiveBufferString& in,std::string_view bytes)override{core.strings.replace_request(in,bytes);}
    void construct_response(s::ReceiveBufferString& out,std::string_view bytes)override{
        core.strings.construct_response(out,bytes);if(!enabled||!out.size_10)return;
        check(bytes.back()==0,"controlled plain producer has explicit payload NUL");
        effects.prefix=&out;effects.refresh();
        try{
            s::resize_receive_buffer_010040(out,out.size_10-1,0,context);
            s::encode_response_hex_03d190(out,context);++encodes;
            s::resize_receive_buffer_010040(out,out.size_10+1,0,context);
            core.containers.f.m.response.value=out;
        }catch(...){effects.prefix=nullptr;effects.refresh();throw;}
        effects.prefix=nullptr;effects.refresh();
    }
};
} // namespace response_hex_test
