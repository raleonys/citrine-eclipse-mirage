// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/net/transport_native_message.hpp"
#include "tests/support/socket/request_growth_test_fixture.hpp"

namespace native_message_test {
namespace s=laureondo::socket;namespace m=laureondo::messages;
using response_test::check;using response_test::checks;using response_test::rejects;
struct Failure{int code;};
struct Bytes:s::ReceiveBufferApi {
    container_test::ContainerFixture& c;s::NativeMessageLocals& locals;std::span<s::ReceiveBufferString> events;
    explicit Bytes(container_test::ContainerFixture& c,s::NativeMessageLocals& locals,std::span<s::ReceiveBufferString> events):c(c),locals(locals),events(events){
        auto& pool=c.f.m.response;pool.api.on_allocate=[&pool](){
            for(const auto& p:pool.blocks)if(p)pool.next_raw=std::max(pool.next_raw,p->raw+p->bytes.size()+0x1000);
            for(std::size_t i=1;i<pool.regions.size();++i)if(pool.regions[i].bytes.empty()){pool.next_region=i;break;}
        };refresh();
    }
    void refresh(){c.bytes.refresh();c.bytes.regions[253]={reinterpret_cast<std::uintptr_t>(locals.response.storage_00.data()),locals.response.storage_00};c.bytes.regions[252]={reinterpret_cast<std::uintptr_t>(locals.temporary.storage_00.data()),locals.temporary.storage_00};for(std::size_t i=0;i<events.size();++i)c.bytes.regions[240+i]={reinterpret_cast<std::uintptr_t>(events[i].storage_00.data()),events[i].storage_00};}
    std::uintptr_t allocate_04b0bc(std::uint64_t n)override{const auto raw=c.f.m.response.api.allocate_04b0bc(n);if(raw&&n>=4096){const auto data=(raw+39)&~std::uintptr_t{31};if(c.f.m.header_count>=c.f.m.headers.size())throw Failure{90};c.f.m.headers[c.f.m.header_count++]={data-8,raw};}refresh();return raw;}
    void copy_bytes(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{c.bytes.copy_bytes(to,from,n);}
    void fill_bytes(std::uintptr_t to,std::uint8_t value,std::uint64_t n)override{c.bytes.fill_bytes(to,value,n);}
    void free_sized_04b0f8(std::uintptr_t raw,std::uint64_t n)override{
        for(auto* pool:{&c.f.m.request,&c.f.m.response,&c.f.m.extra})for(const auto& p:pool->blocks)if(p&&p->raw==raw){pool->api.free_sized_04b0f8(raw,n);refresh();return;}
        throw Failure{91};
    }
    [[noreturn]]void length_error_00ce80()override{throw Failure{10};}
    [[noreturn]]void allocation_overflow_00cd30()override{throw Failure{11};}
    [[noreturn]]void invalid_parameter_05c974()override{throw Failure{12};}
};
struct Api:s::NativeMessageApi {
    core_test::Fixture& core;s::NativeMessageLocals& locals;s::ReceiveBufferContext& strings;
    std::unique_ptr<m::XmlDocument> document;std::vector<std::string> trace;
    std::deque<int> codes,errors;std::function<void(std::string)> hook;
    std::function<bool(s::ReceiveBufferString&)> decrypt,encrypt;
    std::function<s::ReceiveBufferString&(s::ReceiveBufferString&)> dispatch;
    std::array<s::ReceiveBufferString*,8> slots{};s::NativeMessageEventQueue queue{slots,8,0,0};s::NativeMessageMutex mutex{1};
    int lock_result=0;bool empty_on_lock=false;unsigned constructs=0,parses=0,destroys=0,decrypts=0,encrypts=0,dispatches=0,locks=0,unlocks=0,nul_grows=0,append_grows=0;
    explicit Api(core_test::Fixture& core,s::NativeMessageLocals& locals,s::ReceiveBufferContext& strings):core(core),locals(locals),strings(strings){}
    void event(std::string name){trace.push_back(name);if(hook)hook(name);}
    std::string text(const s::ReceiveBufferString& value){return core.containers.f.m.text(value);}
    void assign(s::ReceiveBufferString& value,std::string_view bytes){value={};s::resize_receive_buffer_010040(value,bytes.size(),0,strings);if(!bytes.empty()){auto to=strings.memory.view(s::receive_buffer_address(value),bytes.size());std::copy(bytes.begin(),bytes.end(),to.begin());}}
    void construct_document_001830(s::NativeMessageDocument& d,bool mode,std::uint32_t flags)override{check(&d==&locals.document&&mode&&!flags,"single native document projection receives ctor1/0");++constructs;d.error_6c=0;event("construct-document");}
    int parse_document_004280(s::NativeMessageDocument& d,std::uintptr_t data,std::uint64_t count)override{
        check(count==UINT64_MAX&&data==s::receive_buffer_address(core.request()),"native parser receives live request address and size_t(-1)");++parses;event("parse");std::string input;for(std::uint64_t i=0;;++i){auto byte=strings.memory.view(data+i,1)[0];if(!byte)break;input.push_back(static_cast<char>(byte));}
        auto result=core.xml.parse_c_string(input);auto code=result.returned_code;d.error_6c=result.has_error?1:0;document=std::move(result.document);
        if(!codes.empty()){code=codes.front();codes.pop_front();}if(!errors.empty()){d.error_6c=errors.front();errors.pop_front();}return code;
    }
    void destroy_document_001e00(s::NativeMessageDocument&)override{++destroys;event("destroy-document");document.reset();}
    bool decrypt_042480(std::uintptr_t id,s::ReceiveBufferString& value,std::uint32_t flags)override{check(id==0x120f0&&!flags&&&value==&core.request(),"native decrypt uses cached handler/live request and flag0");++decrypts;event("decrypt");return decrypt?decrypt(value):true;}
    bool encrypt_042610(std::uintptr_t id,s::ReceiveBufferString& value,std::uint32_t flags)override{check(id==0x120f0&&!flags,"native encrypt uses cached handler and flag0");++encrypts;event(&value==&locals.response?"encrypt-response":"encrypt-event");return encrypt?encrypt(value):true;}
    s::ReceiveBufferString& dispatch_032d00(s::ReceiveBufferString& out,std::uintptr_t id,s::NativeMessageDocument& d)override{
        check(id==0x120f0&&&d==&locals.document&&&out==&locals.temporary,"native dispatch receives temp sret, handler, same document");++dispatches;event("dispatch");if(dispatch)return dispatch(out);
        const auto answer=m::dispatch_032d00(*document,core.cipher,core.xml,core.factory,core.random,core.effects);assign(out,answer);return out;
    }
    void log(s::NativeMessageLog kind,s::NativeMessageDocument&,s::ReceiveBufferString& value)override{const auto tag=kind==s::NativeMessageLog::received?"received":kind==s::NativeMessageLog::response?"response":kind==s::NativeMessageLog::event?"event":kind==s::NativeMessageLog::bad_xml?"bad":"decrypt-fail";event("log-"+std::string(tag));core.effects.logs.push_back(std::string(tag)+":"+text(value));}
    void grow_nul_012350(s::ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uint8_t fill)override{check(extra==1&&!hint&&!fill,"012350 defined native args1/0/0");++nul_grows;event("grow-nul-provider");s::grow_receive_buffer_008350(value,extra,hint,extra,fill,strings);}
    void grow_append_007e30(s::ReceiveBufferString& value,std::uint64_t extra,std::uint64_t hint,std::uintptr_t source,std::uint64_t count)override{check(extra==count&&!hint,"007e30 defined native extra=count/hint0");++append_grows;event("grow-append-provider");s::grow_replace_request_02b940(value,extra,0,value.size_10,0,source,count,strings);}
    s::NativeMessageEventQueue& event_queue_02c400()override{event("event-queue");return queue;}
    s::NativeMessageMutex& event_mutex_02c620()override{event("event-mutex");return mutex;}
    int lock_04a6ac(s::NativeMessageMutex& value)override{check(&value==&mutex,"native cached mutex identity");++locks;event("lock");if(empty_on_lock)queue.count_20=0;return lock_result;}
    void unlock_04a6b4(s::NativeMessageMutex& value)override{check(&value==&mutex,"native unlock same mutex");++unlocks;event("unlock");}
    [[noreturn]]void thread_error_04a648(int code)override{event("thread-error"+std::to_string(code));throw Failure{code};}
};
struct Fixture {
    core_test::Fixture core;s::NativeMessageLocals locals;std::array<s::ReceiveBufferString,8> events{};
    Bytes bytes{core.containers,locals,events};s::ReceiveBufferContext strings{core.containers.bytes.memory,bytes};growth_test::Api growth{strings};s::RequestMutationContext mutation{strings,growth};Api api{core,locals,strings};
    std::array<s::NativeMessageBinding,1> bindings{{{0x120f0,&core.encrypted}}};s::NativeMessageContext context{bindings,core.show,strings,mutation,locals,api};
    Fixture(){core.containers.f.m.free_string(core.containers.f.m.response.value);bytes.refresh();}
    void seed(std::string input){core.seed(std::move(input));bytes.refresh();}
    void event(std::string_view input,std::size_t slot=0){api.assign(events[slot],input);api.slots[slot]=&events[slot];++api.queue.count_20;}
    void run(){s::process_native_message_033d70(0x120f0,core.response,core.request(),context);}
    std::string input(){return core.input();}std::string output(){return core.output();}
};
inline std::string response(std::string_view value){return "<LSX>\n    <Response id=\"bridge\" sender=\"fixture\">\n        <BridgeAnswer value=\""+std::string(value)+"\"/>\n    </Response>\n</LSX>\n";}
inline std::string framed(std::string bytes,bool encoded){if(encoded)bytes=core_test::hex(bytes);bytes.push_back(0);return bytes;}
} // namespace native_message_test
