#include "lcmap.hpp"
#include <algorithm>
#include <array>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace m=anadius::crt_lcmap_recovered;
using A=m::Address;
namespace {
std::uint64_t checks=0;
void check(bool pass) { ++checks; if(!pass) throw std::runtime_error("check "+std::to_string(checks)); }
struct Conv {std::uint32_t cp,flags;A src;std::int32_t count;A dst;std::int32_t capacity;A def,used;};
struct Map {A name;std::uint32_t flags;A src;std::int32_t count;A dst;std::int32_t capacity;A version,reserved;std::uint64_t sort;};
struct Fx final:m::Effects {
    std::map<A,std::uint8_t> bytes;
    std::vector<A> reads,freed,stack,heap;
    std::vector<std::uint64_t> stack_sizes,heap_sizes;
    std::vector<Conv> mb,wc;
    std::vector<Map> maps;
    m::Context context{0x2000,0x3000,0x4000,1};
    A next=0x10000,first=0,second=0,seen_locale=0;
    std::uint32_t locales=0;
    std::int32_t mb_query=3,mb_fill=3,map_query=4,map_fill=4,wc_result=7;
    std::uint32_t deny_allocation=0,allocations=0;
    std::function<void(Fx&)> map_hook,wc_hook,free_hook;
    Fx() {
        region(0x1000,32); bytes[0x1000]='a';bytes[0x1001]='b';
        region(0x2000,0x400);region(0x3000,0x20);region(0x4000,0x20);region(0x5000,32);
        write_u32(0x300c,932);write_u32(0x400c,1252);write_u32(0x23a8,0x97);
    }
    void region(A base,std::uint64_t size) { for(std::uint64_t i=0;i<size;++i) bytes[base+i]=0xa5; }
    m::Context locale_update_058b04(A locale) override {++locales;seen_locale=locale;return context;}
    std::uint8_t read_u8(A address) override {reads.push_back(address);return bytes.at(address);}
    std::uint32_t read_u32(A address) override {
        std::uint32_t value=0;for(unsigned i=0;i<4;++i)value|=std::uint32_t(bytes.at(address+i))<<(i*8);return value;
    }
    void write_u32(A address,std::uint32_t value) override {
        for(unsigned i=0;i<4;++i) bytes.at(address+i)=static_cast<std::uint8_t>(value>>(i*8));
    }
    A allocate(std::uint64_t size,bool is_stack) {
        ++allocations;
        (is_stack?stack_sizes:heap_sizes).push_back(size);
        if(allocations==deny_allocation || size>8192) return 0;
        A base=next;next+=0x10000;region(base,size);(is_stack?stack:heap).push_back(base);
        if(first==0)first=base+16;else second=base+16;
        return base;
    }
    A malloc_06a308(std::uint64_t size) override{return allocate(size,false);}
    A stack_allocate(std::uint64_t size) override{return allocate(size,true);}
    void free_0668c0(A address) override{freed.push_back(address);if(free_hook)free_hook(*this);}
    std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t flags,
        A src,std::int32_t count,A dst,std::int32_t capacity) override {
        mb.push_back({cp,flags,src,count,dst,capacity,0,0});return mb.size()==1?mb_query:mb_fill;
    }
    std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t cp,std::uint32_t flags,
        A src,std::int32_t count,A dst,std::int32_t capacity,A def,A used) override {
        wc.push_back({cp,flags,src,count,dst,capacity,def,used});if(wc_hook)wc_hook(*this);return wc_result;
    }
    std::int32_t lc_map_string_0671d4(A name,std::uint32_t flags,A src,std::int32_t count,
        A dst,std::int32_t capacity,A version,A reserved,std::uint64_t sort) override {
        maps.push_back({name,flags,src,count,dst,capacity,version,reserved,sort});
        if(map_hook)map_hook(*this);
        return maps.size()==1?map_query:map_fill;
    }
    std::uint32_t run(std::uint32_t flags=0x100,std::int32_t count=2,
        std::int32_t capacity=10,std::uint32_t cp=0,std::int32_t invalid=0) {
        return m::lc_map_string_06cc30(0x777,0x888,flags,0x1000,count,0x5000,capacity,cp,invalid,*this);
    }
};
void success_matrix() {
    for(auto wide:{1,3,504,505,700})for(auto mapped:{1,4,504,505,700})
    for(auto capacity:{0,9,-1})for(auto invalid:{0,1,-1,INT32_MIN}) {
        Fx f;f.mb_query=wide;f.mb_fill=17;f.map_query=mapped;f.map_fill=23;
        check(f.run(0x100,2,capacity,0,invalid)==7);
        check(f.locales==1 && f.seen_locale==0x777 && f.read_u32(0x23a8)==0x95);
        check(f.mb.size()==2 && f.maps.size()==2 && f.wc.size()==1);
        check(f.mb[0].cp==932 && f.mb[0].flags==(invalid==0?1u:9u));
        check(f.mb[0].src==0x1000 && f.mb[0].count==2 && f.mb[0].dst==0 && f.mb[0].capacity==0);
        check(f.mb[1].flags==1 && f.mb[1].dst==f.first && f.mb[1].capacity==wide);
        check(f.maps[0].name==0x888 && f.maps[0].flags==0x100 && f.maps[0].src==f.first);
        check(f.maps[0].count==wide && f.maps[0].dst==0 && f.maps[0].capacity==0);
        check(f.maps[1].count==wide && f.maps[1].dst==f.second && f.maps[1].capacity==mapped);
        check(f.maps[1].version==0 && f.maps[1].reserved==0 && f.maps[1].sort==0);
        check(f.wc[0].cp==932 && f.wc[0].flags==0 && f.wc[0].src==f.second && f.wc[0].count==mapped);
        check(f.wc[0].dst==(capacity==0?0:0x5000) && f.wc[0].capacity==capacity && f.wc[0].def==0 && f.wc[0].used==0);
        std::vector<A> expected;if(mapped>504)expected.push_back(f.second-16);if(wide>504)expected.push_back(f.first-16);
        check(f.freed==expected);
        for(auto base:f.stack)check(f.read_u32(base)==0xcccc && f.bytes.at(base+4)==0xa5 && f.bytes.at(base+16)==0xa5);
        for(auto base:f.heap)check(f.read_u32(base)==0xdddd && f.bytes.at(base+4)==0xa5 && f.bytes.at(base+16)==0xa5);
        if(wide==504)check(f.stack_sizes.front()==1024);
        if(wide==505)check(f.heap_sizes.front()==1026);
    }
}
void sort_keys() {
    for(auto wide:{3,505})for(auto queried:{-9,-1,1,7,INT32_MAX})for(auto capacity:{0,-10,-1,1,7,10}) {
        Fx f;f.mb_query=wide;f.map_query=queried;f.map_fill=0x12345678;
        const bool fill=capacity!=0 && queried<=capacity;
        const auto expected=capacity==0?static_cast<std::uint32_t>(queried):(fill?0x12345678u:0u);
        check(f.run(0x400,2,capacity)==expected);
        check(f.maps.size()==(fill?2u:1u) && f.wc.empty() && f.allocations==1);
        if(fill)check(f.maps[1].dst==0x5000 && f.maps[1].capacity==capacity && f.maps[1].count==wide);
        check(f.freed.size()==(wide>504?1u:0u));
    }
    Fx f;f.map_fill=0;check(f.run(0x400,2,9)==0 && f.maps.size()==2);
}
void counts() {
    Fx cnt;cnt.bytes[0x1002]=0;
    check(m::strncnt_063168(0x1000,0,cnt)==0 && cnt.reads==std::vector<A>({0x1000}));
    cnt.reads.clear();check(m::strncnt_063168(0x1000,UINT64_MAX,cnt)==2 && cnt.reads.size()==3);
    Fx wrap;wrap.bytes[UINT64_MAX]='x';wrap.bytes[0]=0;
    check(m::strncnt_063168(UINT64_MAX,1,wrap)==1 && wrap.reads==std::vector<A>({UINT64_MAX,0}));
    for(auto count:{0,-1,INT32_MIN,1,2,3,7}) {
        Fx f;f.bytes[0x1002]=0;f.run(0x100,count);
        const auto expected=count>2?3:count;
        check(f.mb[0].count==expected && f.mb[1].count==expected);
        check(f.reads.size()==(count<=0?0u:count==1?2u:3u));
    }
    Fx f;for(A i=0;i<8;++i)f.bytes[0x1000+i]='x';f.run(0x100,7);
    check(f.mb[0].count==7 && f.reads.size()==8 && f.reads.back()==0x1007);
    Fx zero;zero.bytes[0x1000]=0;zero.run(0x100,7);
    check(zero.mb[0].count==1 && zero.reads.size()==1);
    Fx explicit_cp;explicit_cp.run(0x100,2,10,65001);check(explicit_cp.mb[0].cp==65001);
}
void failures_and_markers() {
    for(auto stage:{0,1,2,3,4,5,6,7}) {
        Fx f;f.mb_query=505;f.map_query=505;
        if(stage==0)f.mb_query=0;
        if(stage==1)f.deny_allocation=1;
        if(stage==2)f.mb_fill=0;
        if(stage==3)f.map_query=0;
        if(stage==4)f.deny_allocation=2;
        if(stage==5)f.map_fill=0;
        if(stage==6)f.wc_result=0;
        if(stage==7)f.wc_result=INT32_MIN;
        check(f.run()==(stage==7?0x80000000u:0u));
        check(f.read_u32(0x23a8)==0x95);
        std::vector<A> expected;if(stage>=5)expected.push_back(f.second-16);if(stage>=2)expected.push_back(f.first-16);
        check(f.freed==expected);
    }
    for(auto count:{-1,-2,-8,-9,INT32_MIN}) {
        Fx f;f.mb_query=count;check(f.run()==0 && f.maps.empty());
        check(f.allocations==(count==-9||count==INT32_MIN?1u:0u));
    }
    Fx mapneg;mapneg.map_query=-1;check(mapneg.run()==0 && mapneg.allocations==1 && mapneg.wc.empty());
    for(auto denied:{1u,2u}) {Fx f;f.deny_allocation=denied;check(f.run()==0 && f.freed.empty());}
    Fx mutate;mutate.mb_query=505;mutate.map_query=505;
    mutate.wc_hook=[](Fx& f){f.write_u32(f.second-16,0xcccc);f.write_u32(f.first-16,0xcccc);};
    check(mutate.run()==7 && mutate.freed.empty());
    Fx forced;forced.wc_hook=[](Fx& f){f.write_u32(f.second-16,0xdddd);f.write_u32(f.first-16,0xdddd);};
    check(forced.run()==7 && forced.freed==std::vector<A>({forced.second-16,forced.first-16}));
    Fx alias;alias.mb_query=505;alias.map_query=505;
    alias.wc_hook=[](Fx& f){f.write_u32(f.first-16,0xcccc);};
    check(alias.run()==7 && alias.freed==std::vector<A>({alias.second-16}));
    Fx callback;callback.mb_query=505;callback.map_query=505;
    callback.free_hook=[](Fx& f){f.write_u32(f.first-16,0xcccc);};
    check(callback.run()==7 && callback.freed==std::vector<A>({callback.second-16}));
    Fx liveflags;liveflags.wc_hook=[](Fx& f){f.write_u32(0x23a8,0xe2);};
    check(liveflags.run()==7 && liveflags.read_u32(0x23a8)==0xe0);
    Fx flags;flags.context.clear_thread_flag=0;flags.run();check(flags.read_u32(0x23a8)==0x97);
    Fx fullflag;fullflag.context.clear_thread_flag=0xff;fullflag.run();check(fullflag.read_u32(0x23a8)==0x95);
    Fx bad;bad.bytes.erase(0x1002);bool caught=false;try{bad.run(0x100,2);}catch(const std::out_of_range&){caught=true;}
    check(caught && bad.mb.empty()); // Provider boundary, not an invented original error path.
}
}
int main() {
    try {success_matrix();sort_keys();counts();failures_and_markers();std::cout<<"lcmap: "<<checks<<" checks PASS\n";return 0;}
    catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
