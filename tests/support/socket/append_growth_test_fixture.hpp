// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_append_growth.hpp"
#include "tests/support/socket/native_message_test_fixture.hpp"

namespace append_test {
namespace s=citrine_eclipse_mirage::socket;namespace b=buffer_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure{int code;};
struct Fixture {
    mutation_test::Fixture f;std::vector<std::uint8_t> external;
    void seed(std::string_view value,std::uint64_t capacity){f.seed(value,capacity);}
    s::ReceiveBufferString& value(){return f.f.value;}
    std::uintptr_t source(std::size_t n){external.resize(std::max<std::size_t>(1,n));for(std::size_t i=0;i<n;++i)external[i]=static_cast<std::uint8_t>((i*19+31)&255);f.f.regions[31]={0x900000,external};return 0x900000;}
    s::ReceiveBufferString& character(std::uint64_t extra,std::uint8_t byte){return s::grow_append_character_012350(value(),extra,0,byte,f.buffer);}
    s::ReceiveBufferString& bytes(std::uint64_t extra,std::uintptr_t source,std::uint64_t n){return s::grow_append_bytes_007e30(value(),extra,0,source,n,f.buffer);}
    std::string text(){return f.text();}
};
// Every effect except the two newly source-bound grow methods delegates to
// N9's controlled lower providers. N9 fixture source remains unchanged.
struct Api:s::MessageAppendApi {
    s::NativeMessageApi& next;Api(s::ReceiveBufferContext& strings,s::NativeMessageApi& next):MessageAppendApi(strings),next(next){}
    void construct_document_001830(s::NativeMessageDocument& d,bool a,std::uint32_t b)override{next.construct_document_001830(d,a,b);}
    int parse_document_004280(s::NativeMessageDocument& d,std::uintptr_t a,std::uint64_t n)override{return next.parse_document_004280(d,a,n);}
    void destroy_document_001e00(s::NativeMessageDocument& d)override{next.destroy_document_001e00(d);}
    bool decrypt_042480(std::uintptr_t id,s::ReceiveBufferString& v,std::uint32_t flags)override{return next.decrypt_042480(id,v,flags);}
    bool encrypt_042610(std::uintptr_t id,s::ReceiveBufferString& v,std::uint32_t flags)override{return next.encrypt_042610(id,v,flags);}
    s::ReceiveBufferString& dispatch_032d00(s::ReceiveBufferString& v,std::uintptr_t id,s::NativeMessageDocument& d)override{return next.dispatch_032d00(v,id,d);}
    void log(s::NativeMessageLog kind,s::NativeMessageDocument& d,s::ReceiveBufferString& v)override{next.log(kind,d,v);}
    s::NativeMessageEventQueue& event_queue_02c400()override{return next.event_queue_02c400();}
    s::NativeMessageMutex& event_mutex_02c620()override{return next.event_mutex_02c620();}
    int lock_04a6ac(s::NativeMessageMutex& m)override{return next.lock_04a6ac(m);}
    void unlock_04a6b4(s::NativeMessageMutex& m)override{next.unlock_04a6b4(m);}
    [[noreturn]]void thread_error_04a648(int code)override{next.thread_error_04a648(code);}
};
} // namespace append_test
