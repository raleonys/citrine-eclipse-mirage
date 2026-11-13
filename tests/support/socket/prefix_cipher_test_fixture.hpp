// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "citrine_eclipse_mirage/net/transport_prefix_cipher.hpp"
#include "tests/support/socket/append_growth_test_fixture.hpp"

namespace cipher_test {
namespace s=citrine_eclipse_mirage::socket;namespace c=citrine_eclipse_mirage::crypto;namespace b=buffer_test;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure{int code;};
inline std::string unhex(std::string_view in){std::string out;const auto n=[](char ch){return ch<='9'?ch-'0':ch-'a'+10;};for(std::size_t i=0;i<in.size();i+=2)out.push_back(static_cast<char>((n(in[i])<<4)|n(in[i+1])));return out;}
inline c::Key key(std::string_view hex){const auto bytes=unhex(hex);c::Key out{};std::copy_n(reinterpret_cast<const std::uint8_t*>(bytes.data()),16,out.begin());return out;}
inline const c::Key test_key=key("2b7e151628aed2a6abf7158809cf4f3c"),test_iv=key("000102030405060708090a0b0c0d0e0f");
struct Effects:mutation_test::Copy {
    unsigned fills=0;std::function<void()> before_fill,after_fill;explicit Effects(b::Fixture& f):Copy(f){}
    void fill_bytes(std::uintptr_t p,std::uint8_t byte,std::uint64_t n)override{++fills;if(before_fill)before_fill();Copy::fill_bytes(p,byte,n);if(after_fill)after_fill();}
};
struct Blocks:s::PrefixCipherBlocks {
    s::PrefixCipherBlocksAdapter real;bool noop=false;
    struct Call{std::string kind;std::uintptr_t address;std::uint64_t count;};std::vector<Call> calls;
    std::function<void(const Call&)> before,after;
    explicit Blocks(s::ReceiveBufferMemory& memory):real(memory){}
    template<class F>void call(std::string kind,std::uintptr_t p,std::uint64_t n,F f){const Call value{kind,p,n};calls.push_back(value);if(before)before(value);if(!noop)f();if(after)after(value);}
    void decrypt_ecb_045620(s::PrefixCipherState& state,std::uintptr_t p)override{call("ecb-dec",p,16,[&]{real.decrypt_ecb_045620(state,p);});}
    void encrypt_ecb_045630(s::PrefixCipherState& state,std::uintptr_t p)override{call("ecb-enc",p,16,[&]{real.encrypt_ecb_045630(state,p);});}
    void decrypt_cbc_0454e0(s::PrefixCipherState& state,std::uintptr_t p,std::uint64_t n)override{call("cbc-dec",p,n,[&]{real.decrypt_cbc_0454e0(state,p,n);});}
    void encrypt_cbc_045580(s::PrefixCipherState& state,std::uintptr_t p,std::uint64_t n)override{call("cbc-enc",p,n,[&]{real.encrypt_cbc_045580(state,p,n);});}
};
struct Fixture {
    b::Fixture f;Effects effects{f};s::ReceiveBufferContext strings{f.memory,effects};c::Cipher schedule;
    s::PrefixCipherState state{2,schedule,test_iv};Blocks blocks{f.memory};s::PrefixCipherContext context{strings,blocks};
    Fixture(){schedule.set_key_042780(test_key);f.api.on_allocate=[&](){for(const auto& p:f.blocks)if(p)f.next_raw=std::max(f.next_raw,p->raw+p->bytes.size()+0x1000);};}
    void seed(std::string_view value,std::uint64_t capacity=15){f.seed(std::max<std::uint64_t>(capacity,value.size()),value.size());auto out=f.memory.view(s::receive_buffer_address(f.value),value.size());std::copy(value.begin(),value.end(),out.begin());}
    std::string text(){return f.text();}
    bool encrypt(std::uint64_t offset=0){return s::encrypt_prefix_042610(state,f.value,offset,context);}
    bool decrypt(std::uint64_t offset=0){return s::decrypt_prefix_042480(state,f.value,offset,context);}
};
// Full message API selects new cipher+append finals; all remaining providers
// retain N9's controlled XML/log/queue/mutex contract.
struct Api:s::MessageCipherApi {
    s::NativeMessageApi& next;Api(s::PrefixCipherContext& ctx,std::span<s::PrefixCipherBinding> bindings,s::NativeMessageApi& next):MessageCipherApi(ctx,bindings),next(next){}
    void construct_document_001830(s::NativeMessageDocument& d,bool a,std::uint32_t b)override{next.construct_document_001830(d,a,b);}
    int parse_document_004280(s::NativeMessageDocument& d,std::uintptr_t p,std::uint64_t n)override{return next.parse_document_004280(d,p,n);}
    void destroy_document_001e00(s::NativeMessageDocument& d)override{next.destroy_document_001e00(d);}
    s::ReceiveBufferString& dispatch_032d00(s::ReceiveBufferString& v,std::uintptr_t id,s::NativeMessageDocument& d)override{return next.dispatch_032d00(v,id,d);}
    void log(s::NativeMessageLog k,s::NativeMessageDocument& d,s::ReceiveBufferString& v)override{next.log(k,d,v);}
    s::NativeMessageEventQueue& event_queue_02c400()override{return next.event_queue_02c400();}
    s::NativeMessageMutex& event_mutex_02c620()override{return next.event_mutex_02c620();}
    int lock_04a6ac(s::NativeMessageMutex& m)override{return next.lock_04a6ac(m);}
    void unlock_04a6b4(s::NativeMessageMutex& m)override{next.unlock_04a6b4(m);}
    [[noreturn]]void thread_error_04a648(int code)override{next.thread_error_04a648(code);}
};
inline std::string raw_encrypt(std::string value){c::Cipher blocks;blocks.set_key_042780(test_key);for(std::size_t i=0;i<value.size();i+=16)blocks.encrypt_block_045670(std::span<std::uint8_t,16>(reinterpret_cast<std::uint8_t*>(value.data()+i),16));return value;}
} // namespace cipher_test
