// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include "laureondo/xml/native_document.hpp"
#include "tests/support/socket/native_dispatch_test_fixture.hpp"
#include <bit>
#include <cstring>

namespace document_test {
namespace s=laureondo::socket;namespace m=laureondo::messages;
using response_test::check;using response_test::checks;
struct Failure {int code;};
struct State {
    static constexpr std::uintptr_t base=0x400000000ULL,input_address=0x500000000ULL;
    s::NativeMessageDocument& cell;std::array<std::uint8_t,0x308> raw{};
    s::ReceiveBufferMemory* upstream=nullptr;std::array<s::ReceiveBufferRegion,512> regions{};s::ReceiveBufferMemory memory{regions};
    struct Block{std::uintptr_t address;std::vector<std::uint8_t> bytes;};std::vector<std::unique_ptr<Block>> blocks;
    std::string input;std::uintptr_t next=0x600000000ULL;std::optional<std::pair<std::size_t,std::size_t>> omitted;
    explicit State(s::NativeMessageDocument& c,s::ReceiveBufferMemory* u=nullptr):cell(c),upstream(u){fill(0xa5);refresh();}
    void fill(std::uint8_t byte){raw.fill(byte);std::uint32_t word=std::uint32_t(byte)*0x01010101U;std::memcpy(&cell.error_6c,&word,4);}
    void refresh(){
        regions.fill({});if(upstream){check(upstream->regions.size()<=256,"bounded upstream region inventory");std::copy(upstream->regions.begin(),upstream->regions.end(),regions.begin());}
        std::size_t slot=256;for(auto& b:blocks)if(b)regions[slot++]={b->address,b->bytes};
        check(slot<400,"controlled document allocation inventory");slot=400;
        const auto add=[&](std::size_t first,std::size_t last){if(first>=last)return;auto split=[&](std::size_t a,std::size_t b){if(a<b)regions[slot++]={base+a,std::span(raw).subspan(a,b-a)};};if(!omitted)split(first,last);else{const auto [at,n]=*omitted;split(first,std::min(last,std::max(first,at)));split(std::min(last,std::max(first,at+n)),last);}};
        add(0,0x6c);add(0x70,raw.size());if(!omitted||omitted->first+omitted->second<=0x6c||omitted->first>=0x70)regions[slot++]={base+0x6c,{reinterpret_cast<std::uint8_t*>(&cell.error_6c),4}};
        if(!input.empty())regions[410]={input_address,{reinterpret_cast<std::uint8_t*>(input.data()),input.size()}};
    }
    void seed(std::string value){input=std::move(value);input.push_back(0);refresh();}
    std::uintptr_t allocate(std::uint64_t n){if(n>1024*1024)throw Failure{90};auto b=std::make_unique<Block>();b->address=next;next+=0x200000;b->bytes.resize(static_cast<std::size_t>(n),0xcc);const auto result=b->address;blocks.push_back(std::move(b));refresh();return result;}
    void release(std::uintptr_t p){for(auto& b:blocks)if(b&&b->address==p){b.reset();refresh();return;}}
    std::size_t live()const{std::size_t result=0;for(const auto& b:blocks)result+=bool(b);return result;}
    std::uint64_t get(std::size_t offset,unsigned width=8){const auto bytes=memory.view(base+offset,width);std::uint64_t value=0;for(unsigned i=0;i<width;++i)value|=std::uint64_t(bytes[i])<<(i*8);return value;}
    void put(std::size_t offset,std::uint64_t value,unsigned width=8){auto bytes=memory.view(base+offset,width);for(unsigned i=0;i<width;++i)bytes[i]=static_cast<std::uint8_t>(value>>(i*8));}
    std::array<std::uint8_t,0x308> snapshot()const{auto bytes=raw;std::memcpy(bytes.data()+0x6c,&cell.error_6c,4);return bytes;}
};
// Controlled native lower providers, not new implementations of clear/parser,
// XML nodes/mempools, allocator/free or native vtables/EH.
struct Effects:s::NativeDocumentEffects {
    State& state;native_message_test::Fixture* native=nullptr;laureondo::xml_reader_bridge::Backend xml;
    std::unique_ptr<m::XmlDocument> parsed;std::vector<std::string> trace;std::vector<std::uintptr_t> freed;std::vector<std::uint64_t> allocations;
    std::function<void(std::string)> hook;std::deque<int> codes;bool clear_model=true,null_allocate=false;
    unsigned parses=0,clears=0,destroys=0,node_clears=0;std::uintptr_t copy_destination=0,copy_source=0;std::uint64_t copy_count=0;
    explicit Effects(State& s,native_message_test::Fixture* n=nullptr):state(s),native(n){}
    void event(std::string name){trace.push_back(name);if(hook)hook(name);}
    void receiver(std::uintptr_t p){check(p==State::base,"same bound document receiver");}
    void reset_parsed(){parsed.reset();if(native)native->api.document.reset();}
    void clear_002e60(std::uintptr_t p)override{receiver(p);++clears;event("clear");state.refresh();if(clear_model){state.release(state.get(0x98));state.put(0x98,0);state.cell.error_6c=0;reset_parsed();}}
    void set_error_005820(std::uintptr_t p,std::int32_t code,std::uint32_t line,std::uintptr_t detail)override{receiver(p);check(code==13&&!line&&!detail,"empty document error arguments13/0/0");event("set-error");state.cell.error_6c=code;}
    std::uintptr_t allocate_04b100(std::uint64_t n)override{allocations.push_back(n);event("allocate");return null_allocate?0:state.allocate(n);}
    void copy_078d30(std::uintptr_t to,std::uintptr_t from,std::uint64_t n)override{copy_destination=to;copy_source=from;copy_count=n;event("copy");if(n){auto target=state.memory.view(to,n),source=state.memory.view(from,n);std::memmove(target.data(),source.data(),static_cast<std::size_t>(n));}event("after-copy");}
    void parse_0041b0(std::uintptr_t p)override{receiver(p);++parses;event("parse");if(native)native->api.event("parse");std::string input;const auto data=state.get(0x98);for(std::uint64_t i=0;;++i){const auto byte=state.memory.view(data+i,1)[0];if(!byte)break;input.push_back(static_cast<char>(byte));}
        auto result=native?native->core.xml.parse_c_string(input):xml.parse_c_string(input);state.cell.error_6c=result.has_error?result.returned_code:0;if(!codes.empty()){state.cell.error_6c=codes.front();codes.pop_front();}if(native)native->api.document=std::move(result.document);else parsed=std::move(result.document);}
    void delete_nodes_003110(std::uintptr_t p)override{receiver(p);++node_clears;event("delete-nodes");reset_parsed();}
    void clear_pool_002d70(std::uintptr_t p)override{check(p==State::base+0x108||p==State::base+0x188,"clear120/80 pool addresses");event(p==State::base+0x108?"clear120":"clear80");}
    void clear_pool_002e10(std::uintptr_t p)override{check(p==State::base+0x208,"clear112 pool address");event("clear112");}
    void clear_pool_002dc0(std::uintptr_t p)override{check(p==State::base+0x288,"clear104 pool address");event("clear104");}
    void destroy_pool_001c30(std::uintptr_t p)override{check(p==State::base+0x288,"destroy104 pool address");event("destroy104");}
    void destroy_pool_001cb0(std::uintptr_t p)override{check(p==State::base+0x208,"destroy112 pool address");event("destroy112");}
    void destroy_pool_001bb0(std::uintptr_t p)override{check(p==State::base+0x188,"destroy80 pool address");event("destroy80");}
    void destroy_pool_001d30(std::uintptr_t p)override{check(p==State::base+0x108,"destroy120 pool address");event("destroy120");}
    void free_04b0f8(std::uintptr_t p)override{freed.push_back(p);event("free");state.release(p);}
    void destroy_node_base_001ea0(std::uintptr_t p)override{receiver(p);event("base-destroy");++destroys;}
};
struct Api:s::MessageDocumentApi {
    s::NativeMessageApi& next;session_cipher_test::Effects& effects;
    Api(s::PrefixCipherContext& c,std::span<s::PrefixCipherBinding> b,s::NativeDispatchContext& d,s::NativeDocumentContext& docs,std::span<s::NativeDocumentBinding> bindings,s::NativeMessageApi& n,session_cipher_test::Effects& e):MessageDocumentApi(c,b,d,docs,bindings),next(n),effects(e){}
    void log(s::NativeMessageLog k,s::NativeMessageDocument& d,s::ReceiveBufferString& v)override{next.log(k,d,v);}
    s::NativeMessageEventQueue& event_queue_02c400()override{return next.event_queue_02c400();}
    s::NativeMessageMutex& event_mutex_02c620()override{return next.event_mutex_02c620();}
    int lock_04a6ac(s::NativeMessageMutex& m)override{return next.lock_04a6ac(m);}
    void unlock_04a6b4(s::NativeMessageMutex& m)override{next.unlock_04a6b4(m);}
    [[noreturn]]void thread_error_04a648(int n)override{next.thread_error_04a648(n);}
};
struct Fixture {
    s::NativeMessageDocument document;State state{document};Effects effects{state};s::NativeDocumentContext context{state.memory,effects};
    std::uintptr_t construct(std::uint8_t mode=1,std::uint32_t flags=0){return s::construct_native_document_001830(State::base,mode,flags,context);}
    std::int32_t parse(std::uintptr_t input=State::input_address,std::uint64_t count=UINT64_MAX){return s::parse_native_document_004280(State::base,input,count,context);}
    void destroy(){s::destroy_native_document_001e00(State::base,context);}
};
} // namespace document_test
