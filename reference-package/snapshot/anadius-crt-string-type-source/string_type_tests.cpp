#include "string_type.hpp"
#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace s=anadius::crt_string_type_recovered;
namespace bc=anadius::crt_byteclass_recovered;
using s::Address;
static std::uint64_t checks=0;
static void require(bool value,const char* message) {
    ++checks;
    if(!value) throw std::runtime_error(message);
}
struct Fixture final:s::Effects {
    struct Conversion {std::uint32_t cp,flags;Address source;std::int32_t count;
        Address destination;std::int32_t capacity;};
    bc::LocaleContext context{0x1000,0x2000,0x3000,1};
    std::map<Address,std::uint32_t> words{{0x13a8,0xabcdefefU},{0x200c,932U}};
    std::vector<std::string> events;
    std::vector<Conversion> conversions;
    std::int32_t queried=2,filled=1;
    std::uint32_t native_result=0x87654321U;
    Address stack_base=0x4000,heap_base=0x8000;
    Address observed_locale=0,zero_address=0,native_source=0,native_output=0;
    std::uint64_t stack_bytes=0,heap_bytes=0,zero_count=0;
    std::uint32_t native_type=0;
    std::int32_t native_count=0;
    unsigned stack_calls=0,heap_calls=0,zero_calls=0,native_calls=0;
    bool mutate_marker_on_fill=false,mutate_marker_on_native=false;
    std::uint32_t replacement_marker=0;
    std::vector<Address> frees;
    bc::LocaleContext locale_update_058b04(Address locale) override {
        events.emplace_back("locale");observed_locale=locale;return context;
    }
    std::uint32_t read_u32(Address address) override {
        events.emplace_back(address==context.ptd+0x3a8 ? "read flags" :
            address==context.locale_info+0xc ? "read cp" : "read marker");
        return words.at(address);
    }
    void write_u32(Address address,std::uint32_t value) override {
        events.emplace_back(address==context.ptd+0x3a8 ? "write flags" : "write marker");
        words[address]=value;
    }
    Address malloc_06a308(std::uint64_t bytes) override {
        events.emplace_back("malloc");++heap_calls;heap_bytes=bytes;return heap_base;
    }
    void free_0668c0(Address address) override {
        events.emplace_back("free");frees.push_back(address);
    }
    Address stack_allocate(std::uint64_t bytes) override {
        events.emplace_back("stack");++stack_calls;stack_bytes=bytes;return stack_base;
    }
    void zero_bytes(Address address,std::uint64_t bytes) override {
        events.emplace_back("zero");++zero_calls;zero_address=address;zero_count=bytes;
        // Sparse addresses and recorded spans avoid allocating attacker-sized data.
    }
    std::int32_t multibyte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t flags,
        Address source,std::int32_t count,Address dest,std::int32_t capacity) override {
        events.emplace_back(conversions.empty() ? "query" : "fill");
        conversions.push_back({cp,flags,source,count,dest,capacity});
        if(conversions.size()==1) return queried;
        require(zero_calls==1,"wide payload zeroed before fill");
        if(mutate_marker_on_fill) words[dest-16U]=replacement_marker;
        words[context.ptd+0x3a8]=0x55aa55ffU;
        return filled;
    }
    std::uint32_t native_get_string_type_w(std::uint32_t type,Address source,
        std::int32_t count,Address output) override {
        events.emplace_back("native");++native_calls;native_type=type;
        native_source=source;native_count=count;native_output=output;
        if(mutate_marker_on_native) words[source-16U]=replacement_marker;
        words[context.ptd+0x3a8]=0xa5a5a5a7U;
        return native_result;
    }
    std::uint32_t run(std::uint32_t cp=0,std::int32_t strict=0) {
        return s::get_string_type_06e210(0x1234,0xfedcba98U,
            0x7777,-17,0x9999,cp,strict,*this);
    }
};

static void boundaries_and_raw_values() {
    constexpr auto max=std::numeric_limits<std::int32_t>::max();
    constexpr auto min=std::numeric_limits<std::int32_t>::min();
    constexpr std::array<std::int32_t,15> queries{0,1,2,7,8,503,504,505,506,max,-1,-8,-9,-504,min};
    constexpr std::array<std::int32_t,5> fills{0,1,2,-1,min};
    constexpr std::array<std::int32_t,4> stricts{0,1,-1,min};
    constexpr std::array<std::uint32_t,3> cps{0,65001,0xffffffffU};
    constexpr std::array<std::uint32_t,4> bools{0,1,2,0x87654321U};
    for(const auto query:queries) for(const auto fill:fills)
    for(const auto strict:stricts) for(const auto cp:cps) for(const auto native:bools) {
        Fixture f;f.queried=query;f.filled=fill;f.native_result=native;
        const auto actual=f.run(cp,strict);
        const bool allocated=query>0 || query<=-9;
        const bool stack=query>0 && query<=504;
        require(f.observed_locale==0x1234,"locale forwarded");
        require(f.events.front()=="locale","locale constructed first");
        require(f.conversions.size()==(allocated ? 2U : 1U),"query/fill branch");
        const auto& q=f.conversions[0];
        require(q.cp==(cp==0 ? 932U : cp),"explicit versus locale CP");
        require(q.flags==(strict==0 ? 1U : 9U),"query strict flags");
        require(q.source==0x7777 && q.count==-17 && q.destination==0 && q.capacity==0,
            "raw query addresses and signed count");
        require(f.stack_calls==(stack ? 1U : 0U),"stack threshold inclusive504");
        require(f.heap_calls==(allocated && !stack ? 1U : 0U),"heap threshold505");
        require(f.zero_calls==(allocated ? 1U : 0U),"zero only allocated payload");
        require(f.native_calls==(allocated && fill!=0 ? 1U : 0U),"native only nonzero fill");
        require(actual==(allocated && fill!=0 ? native : 0U),"full native BOOL result");
        if(allocated) {
            // Signed mathematical product fits int64, then is reinterpreted modulo64.
            const auto bytes=static_cast<std::uint64_t>(std::int64_t{query}*2);
            const auto total=bytes+16U;
            const auto base=stack ? f.stack_base : f.heap_base;
            require(f.zero_address==base+16 && f.zero_count==bytes,"exact payload zero span");
            require(stack ? f.stack_bytes==((total+15U)/16U)*16U : f.heap_bytes==total,
                "allocation marker+payload size");
            const auto& c=f.conversions[1];
            require(c.cp==q.cp && c.flags==1 && c.source==q.source && c.count==q.count,
                "fill fixed flags and original source");
            require(c.destination==base+16 && c.capacity==query,"queried fill capacity");
            require(f.frees.size()==(stack ? 0U : 1U),"marker controlled normal release");
            if(!stack) require(f.frees[0]==base,"free marker base rather than wide data");
            if(fill!=0) {
                require(f.native_type==0xfedcba98U && f.native_output==0x9999,
                    "type and untouched raw output pointer forwarded");
                require(f.native_source==base+16 && f.native_count==fill,
                    "native uses second count including negative result");
            }
        }
        const auto expected_flags=!allocated ? 0xabcdefedU : fill==0 ? 0x55aa55fdU : 0xa5a5a5a5U;
        require(f.words[0x13a8]==expected_flags,"owned flag clears latest value only bit2");
        require(f.events[f.events.size()-2]=="read flags" && f.events.back()=="write flags",
            "locale teardown after allocation cleanup");
    }
}
static void failures_and_header_reload() {
    for(const bool stack:{false,true}) for(const bool owned:{false,true}) {
        Fixture f;f.queried=stack ? 504 : 505;f.context.clear_thread_flag=owned ? 1 : 0;
        if(stack) f.stack_base=0;else f.heap_base=0;
        require(f.run()==0,"allocation failure returns zero");
        require(f.zero_calls==0 && f.native_calls==0 && f.conversions.size()==1 && f.frees.empty(),
            "failed allocation no data effects");
        require(f.words[0x13a8]==(owned ? 0xabcdefedU : 0xabcdefefU),"failure owned locale teardown");
    }
    for(const bool stack:{false,true}) for(const bool at_native:{false,true}) {
        Fixture f;f.queried=stack ? 504 : 505;
        f.replacement_marker=stack ? 0xddddU : 0xccccU;
        f.mutate_marker_on_fill=!at_native;f.mutate_marker_on_native=at_native;
        require(f.run()==f.native_result,"mutable header return");
        require(f.frees.size()==(stack ? 1U : 0U),"cleanup rereads header after providers");
        if(stack) require(f.frees[0]==f.stack_base,"header change controls release even stack token");
    }
    for(const std::int32_t query:{1,505}) {
        Fixture f;f.queried=query;f.stack_base=f.heap_base=0xfffffffffffffff0ULL;
        require(f.run()==0,"base plus16 wrapped null returns zero");
        require(f.words[0xfffffffffffffff0ULL]==(query==1 ? 0xccccU : 0xddddU),
            "marker written before wrapped data null check");
        require(f.zero_calls==0 && f.frees.empty() && f.conversions.size()==1,
            "wrapped null has no payload or cleanup");
    }
    Fixture f;f.context.clear_thread_flag=0;
    require(f.run()==f.native_result && f.words[0x13a8]==0xa5a5a5a7U,"unowned flags not cleared");
    require(f.events.back()=="read marker","unowned cleanup has no PTD flag access");
    Fixture order;
    order.run();
    const std::vector<std::string> expected{"locale","read cp","query","stack","write marker",
        "zero","fill","native","read marker","read flags","write flags"};
    require(order.events==expected,"observable normal provider order");
}
int main() {
    try {boundaries_and_raw_values();failures_and_header_reload();
        std::cout<<"PASS string type: "<<checks<<" checks; sparse address and API fixtures, no native API or target execution\n";
        return 0;
    } catch(const std::exception& e) {
        std::cerr<<"FAIL string type: "<<e.what()<<'\n';return 1;
    }
}
