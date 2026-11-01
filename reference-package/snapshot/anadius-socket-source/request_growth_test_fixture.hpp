#pragma once
#include "request_growth.hpp"
#include "request_mutation_test_fixture.hpp"

namespace growth_test {
namespace s=anadius::socket_recovered;namespace b=buffer_test;namespace r=receive_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure {int code;};
struct Api:s::RecoveredRequestGrowthApi {
    unsigned ranges=0;explicit Api(s::ReceiveBufferContext& c):RecoveredRequestGrowthApi(c){}
    [[noreturn]]void out_of_range_00cec0()override{++ranges;throw Failure{1};}
};
struct Fixture {
    mutation_test::Fixture f;Api api{f.buffer};s::RequestMutationContext context{f.buffer,api};std::vector<std::uint8_t> external;
    void seed(std::string_view value,std::uint64_t cap){f.seed(value,cap);}
    std::uintptr_t source(std::size_t count){external.resize(std::max<std::size_t>(1,count));for(std::size_t i=0;i<count;++i)external[i]=static_cast<std::uint8_t>((i*17+31)&255);f.f.regions[31]={0x900000,external};return 0x900000;}
    std::string text(){return f.text();}
    s::ReceiveBufferString& value(){return f.f.value;}
    s::ReceiveBufferString& grow(std::uint64_t extra,std::uint64_t offset,std::uint64_t erase,std::uintptr_t source,std::uint64_t insert){return s::grow_replace_request_02b940(value(),extra,0,offset,erase,source,insert,f.buffer);}
    s::ReceiveBufferString& replace(std::uint64_t offset,std::uint64_t erase,std::uintptr_t source,std::uint64_t insert){return s::replace_request_034730(value(),offset,erase,source,insert,context);}
};
// Request growth allocation stays in the SAME request pool used by the outer
//028d30 cleanup. Response COPY/deque storage retains its separate owned pool.
struct RequestBytes:s::ReceiveBufferApi {
    container_test::ContainerFixture& c;s::ReceiveBufferRegion borrowed{};
    explicit RequestBytes(container_test::ContainerFixture& c):c(c){}
    void refresh(){c.bytes.refresh();if(!borrowed.bytes.empty())c.bytes.regions[254]=borrowed;}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{
        const auto raw=c.f.m.request.api.allocate_04b0bc(n);if(raw&&n>=4096){const auto data=(raw+39)&~std::uintptr_t{31};c.f.m.headers[c.f.m.header_count++]={data-8,raw};}refresh();return raw;
    }
    void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{c.bytes.copy_bytes(to,from,n);}
    void fill_bytes(std::uintptr_t to,std::uint8_t value,std::uint64_t n)override{c.bytes.fill_bytes(to,value,n);}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{c.f.m.request.api.free_sized_04b0f8(p,n);refresh();}
    [[noreturn]]void length_error_00ce80()override{throw Failure{10};}
    [[noreturn]]void allocation_overflow_00cd30()override{throw Failure{11};}
    [[noreturn]]void invalid_parameter_05c974()override{throw Failure{12};}
};
} // namespace growth_test
