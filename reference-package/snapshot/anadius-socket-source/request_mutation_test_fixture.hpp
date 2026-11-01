#pragma once
#include "request_mutation.hpp"
#include "message_core_bridge_test_fixture.hpp"

namespace mutation_test {
namespace s=anadius::socket_recovered;namespace b=buffer_test;namespace r=receive_test;
using response_test::checks;using response_test::check;using response_test::rejects;
struct Failure {int code;};
struct Api:s::RequestMutationApi {
    struct Call {s::ReceiveBufferString* value;std::uint64_t growth,hint,offset,erase;std::uintptr_t source;std::uint64_t insert;};
    std::vector<Call> calls;unsigned range_errors=0;std::function<s::ReceiveBufferString&(Call)> grow;
    [[noreturn]]void out_of_range_00cec0()override{++range_errors;throw Failure{1};}
    s::ReceiveBufferString& grow_replace_02b940(s::ReceiveBufferString& v,std::uint64_t growth,std::uint64_t hint,std::uint64_t offset,std::uint64_t erase,std::uintptr_t source,std::uint64_t insert)override{
        calls.push_back({&v,growth,hint,offset,erase,source,insert});if(grow)return grow(calls.back());throw Failure{2};
    }
};
struct Copy:s::ReceiveBufferApi {
    b::Fixture& f;std::vector<container_test::Bytes::Copy> calls;std::function<void(unsigned)> before,after;
    explicit Copy(b::Fixture& f):f(f){}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{return f.api.allocate_04b0bc(n);}
    void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{calls.push_back({to,from,n});const auto ordinal=static_cast<unsigned>(calls.size());if(before)before(ordinal);f.api.copy_bytes(to,from,n);if(after)after(ordinal);}
    void fill_bytes(std::uintptr_t to,std::uint8_t v,std::uint64_t n)override{f.api.fill_bytes(to,v,n);}
    void free_sized_04b0f8(std::uintptr_t p,std::uint64_t n)override{f.api.free_sized_04b0f8(p,n);}
    [[noreturn]]void length_error_00ce80()override{f.api.length_error_00ce80();}
    [[noreturn]]void allocation_overflow_00cd30()override{f.api.allocation_overflow_00cd30();}
    [[noreturn]]void invalid_parameter_05c974()override{f.api.invalid_parameter_05c974();}
};
struct Fixture {
    b::Fixture f;Copy copy{f};s::ReceiveBufferContext buffer{f.memory,copy};Api api;s::RequestMutationContext context{buffer,api};
    std::array<std::uint8_t,128> external{};
    Fixture(){for(std::size_t i=0;i<external.size();++i)external[i]=static_cast<std::uint8_t>('A'+i%19);f.regions[31]={0x900000,external};}
    void seed(std::string_view value,std::uint64_t capacity){f.seed(capacity,value.size());auto bytes=f.memory.view(s::receive_buffer_address(f.value),value.size());std::copy(value.begin(),value.end(),bytes.begin());}
    std::string text(){return f.text();}
    s::ReceiveBufferString& replace(std::uint64_t offset,std::uint64_t erase,std::uintptr_t source,std::uint64_t insert){return s::replace_request_034730(f.value,offset,erase,source,insert,context);}
};
inline unsigned oracle_digit(unsigned value){const std::string digits="0123456789ABCDEFabcdef";const auto at=digits.find(static_cast<char>(value));return at==std::string::npos?0U:(at<16?static_cast<unsigned>(at):static_cast<unsigned>(at-6));}
inline std::string oracle_decode(std::string_view value){std::string result;for(std::size_t i=0;i+1<value.size();i+=2)result.push_back(static_cast<char>(oracle_digit(static_cast<unsigned char>(value[i]))*16+oracle_digit(static_cast<unsigned char>(value[i+1]))));return result;}
} // namespace mutation_test
