#include "conversion.hpp"
#include <array>
#include <bit>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

namespace cv = anadius::crt_conversion_recovered;
namespace {
std::uint64_t checks{};
void require(bool value) { ++checks; if (!value) { std::cerr << "check failed " << checks << '\n'; std::abort(); } }
struct Fixture final : cv::Effects {
    cv::Buffer* observed{};
    bool wide{}, empty{}, fail_alloc{};
    std::int32_t query{4}, fill{4};
    std::uint32_t error{998}, mapped{84}, errno_value{77};
    std::vector<std::string> events;
    std::vector<std::uint64_t> allocations;
    std::vector<cv::Address> frees;
    std::vector<cv::Buffer> malloc_states, free_states;
    struct Call { std::uint32_t cp, flags; cv::Address source; std::int32_t source_count; cv::Address destination; std::int32_t capacity; };
    std::vector<Call> calls;
    std::uint8_t read_u8(cv::Address address) override {
        require(wide && address == 101); events.emplace_back("read8"); return empty ? 0 : 65;
    }
    std::uint16_t read_u16(cv::Address address) override {
        require(!wide && address == 101); events.emplace_back("read16"); return empty ? 0 : 0x100;
    }
    void write_zero_u8(cv::Address address) override {
        require(!wide && address == observed->data); events.emplace_back("zero8");
    }
    void write_zero_u16(cv::Address address) override {
        require(wide && address == observed->data); events.emplace_back("zero16");
    }
    cv::Address malloc_06a308(std::uint64_t bytes) override {
        events.emplace_back("malloc"); allocations.push_back(bytes); malloc_states.push_back(*observed);
        return fail_alloc ? 0 : 303;
    }
    void free_0668c0(cv::Address address) override {
        events.emplace_back("free"); frees.push_back(address); free_states.push_back(*observed);
    }
    std::int32_t call(std::uint32_t cp,std::uint32_t flags,cv::Address source,
                      std::int32_t count,cv::Address destination,std::int32_t capacity) {
        require(cp == 65001 && flags == (wide ? 9U : 0U) && source == 101 && count == -1);
        const bool first = calls.empty();
        calls.push_back({cp,flags,source,count,destination,capacity});
        events.emplace_back(first ? "query" : "fill");
        if (first) require(destination == 0 && capacity == 0);
        return first ? query : fill;
    }
    std::int32_t multi_byte_to_wide_06ccc8(std::uint32_t cp,std::uint32_t flags,
        cv::Address source,std::int32_t count,cv::Address destination,std::int32_t capacity) override {
        require(wide); return call(cp,flags,source,count,destination,capacity);
    }
    std::int32_t wide_char_to_multi_byte_06cd58(std::uint32_t cp,std::uint32_t flags,
        cv::Address source,std::int32_t count,cv::Address destination,std::int32_t capacity,
        cv::Address default_char,cv::Address used_default) override {
        require(!wide && default_char == 0 && used_default == 0);
        return call(cp,flags,source,count,destination,capacity);
    }
    std::uint32_t last_error() override { events.emplace_back("last"); return error; }
    void map_os_error_05d698(std::uint32_t value) override {
        require(value == error); events.emplace_back("map"); errno_value=mapped;
    }
    anadius::crt_errors_recovered::Cell32 errno_cell_05d724() override {
        events.emplace_back("errno"); return anadius::crt_errors_recovered::Cell32(&errno_value);
    }
    std::uint32_t run(cv::Address source,cv::Buffer& buffer) {
        observed=&buffer;
        return wide ? cv::to_wide_06004c(source,buffer,65001,*this)
                    : cv::to_narrow_06e430(source,buffer,65001,*this);
    }
};
void same(const cv::Buffer& a,const cv::Buffer& b) {
    require(a.data==b.data && a.capacity==b.capacity && a.length==b.length && a.owned==b.owned);
}
void null_and_empty() {
    for(bool wide : {false,true}) for(auto owned : {0,1,255}) {
        for(auto data : {cv::Address{0},cv::Address{202}}) {
            cv::Buffer b{data,8,99,static_cast<std::uint8_t>(owned)}; Fixture f; f.wide=wide;
            require(f.run(0,b)==0); same(b,{});
            require(f.events==(owned ? std::vector<std::string>{"free"} : std::vector<std::string>{}));
            if(owned) { require(f.frees==std::vector<cv::Address>{data}); require(f.free_states[0].owned==owned); }
        }
        for(auto capacity : {0ULL,8ULL}) for(bool failed : {false,true}) {
            cv::Buffer b{202,capacity,99,static_cast<std::uint8_t>(owned)}; Fixture f;
            f.wide=wide; f.empty=true; f.fail_alloc=failed;
            const auto status=f.run(101,b);
            if(capacity==0) {
                require(f.allocations==std::vector<std::uint64_t>{wide ? 2ULL : 1ULL});
                require(f.malloc_states[0].data==202 && f.malloc_states[0].capacity==0 && f.malloc_states[0].length==99 && f.malloc_states[0].owned==0);
                if(failed) { require(status==12); same(b,{0,0,99,0}); }
                else { require(status==0); same(b,{303,1,0,1}); }
            } else { require(status==0); same(b,{202,8,0,static_cast<std::uint8_t>(owned)}); require(f.allocations.empty()); }
            std::vector<std::string> expected{wide ? "read8" : "read16"};
            if(capacity==0) { if(owned) expected.emplace_back("free"); expected.emplace_back("malloc"); }
            if(capacity!=0 || !failed) expected.emplace_back(wide ? "zero16" : "zero8");
            require(f.events==expected); require(f.calls.empty()); require(f.errno_value==77);
        }
    }
}
void successful_and_failed_conversion() {
    for(bool wide : {false,true}) for(auto owned : {0,1,255})
    for(auto capacity : {0ULL,2ULL,4ULL,8ULL}) for(bool fail_alloc : {false,true})
    for(auto query : {0,4}) for(auto fill : {0,1,4,-2}) {
        cv::Buffer original{202,capacity,99,static_cast<std::uint8_t>(owned)},b=original;
        Fixture f; f.wide=wide; f.fail_alloc=fail_alloc; f.query=query; f.fill=fill;
        const auto status=f.run(101,b);
        std::vector<std::string> expected{wide ? "read8" : "read16","query"};
        if(query==0) {
            require(status==84); same(b,original); require(f.allocations.empty());
            expected.insert(expected.end(),{"last","map","errno"});
        } else {
            const bool allocate=capacity<4;
            if(allocate) {
                if(owned) expected.emplace_back("free"); expected.emplace_back("malloc");
                require(f.allocations==std::vector<std::uint64_t>{wide ? 8ULL : 4ULL});
                require(f.malloc_states[0].owned==0 && f.malloc_states[0].data==202 && f.malloc_states[0].length==99);
            } else require(f.allocations.empty());
            if(allocate && fail_alloc) {
                require(status==12); same(b,{0,0,99,0}); require(f.calls.size()==1); require(f.errno_value==77);
            } else {
                expected.emplace_back("fill"); require(f.calls.size()==2);
                require(f.calls[1].destination==(allocate ? 303ULL : 202ULL));
                require(f.calls[1].capacity==static_cast<std::int32_t>(allocate ? 4 : capacity));
                if(fill==0) {
                    require(status==84 && b.length==99); expected.insert(expected.end(),{"last","map","errno"});
                } else require(status==0 && b.length==static_cast<std::uint64_t>(static_cast<std::int64_t>(fill))-1);
                require(b.data==(allocate ? 303ULL : 202ULL)); require(b.capacity==(allocate ? 4ULL : capacity));
                require(b.owned==(allocate ? 1 : owned));
            }
        }
        require(f.events==expected);
        require(f.frees.size()==static_cast<std::size_t>(query!=0 && capacity<4 && owned!=0));
        if(!f.frees.empty()) { require(f.frees[0]==202); same(f.free_states[0],original); }
    }
}
void signed_and_capacity_boundaries() {
    for(bool wide : {false,true}) {
        for(auto capacity : {0x80000000ULL,0x100000004ULL,0xffffffffffffffffULL}) {
            cv::Buffer b{202,capacity,99,0}; Fixture f; f.wide=wide;
            require(f.run(101,b)==0 && b.length==3); require(f.allocations.empty());
            require(f.calls[1].capacity==std::bit_cast<std::int32_t>(static_cast<std::uint32_t>(capacity)));
        }
        for(auto query : {-1,std::numeric_limits<std::int32_t>::min()}) {
            cv::Buffer b{202,8,99,1}; Fixture f; f.wide=wide; f.query=query;
            require(f.run(101,b)==0);
            const auto required=static_cast<std::uint64_t>(static_cast<std::int64_t>(query));
            require(f.allocations==std::vector<std::uint64_t>{required*(wide ? 2ULL : 1ULL)});
            require(b.capacity==required && b.data==303 && b.length==3 && b.owned==1);
            require(f.calls[1].capacity==query);
        }
        // A failed query may map to errno zero; return that exact current cell.
        cv::Buffer b{202,8,99,0}; Fixture f; f.wide=wide; f.query=0; f.mapped=0;
        require(f.run(101,b)==0); same(b,{202,8,99,0});
    }
}
}
int main() {
    null_and_empty(); successful_and_failed_conversion(); signed_and_capacity_boundaries();
    std::cout << "conversion tests: " << checks << " checks PASS\n";
}
