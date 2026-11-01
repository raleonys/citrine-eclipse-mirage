#include "onexit.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <vector>
using namespace anadius::crt_onexit_recovered;
namespace {
int checks;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("onexit check " + std::to_string(checks)); }
struct Fake final : Effects {
    struct Allocation { Address base; std::vector<std::uint64_t> words; };
    std::map<Address, Allocation> allocations;
    std::vector<Address> called, freed;
    std::vector<std::uint64_t> requested;
    std::function<void(Address)> callback;
    Address next = 0x10000;
    unsigned fail_calls = 0;
    int depth = 0, max_depth = 0;
    void lock_0625f8(std::uint32_t id) override { check(id == 2); ++depth; max_depth = std::max(depth,max_depth); }
    void unlock_06264c(std::uint32_t id) override { check(id == 2 && depth > 0); --depth; }
    Address recalloc_070434(Address old, std::uint64_t count, std::uint64_t size) override {
        check(depth > 0 && size == 8); requested.push_back(count);
        if (fail_calls) { --fail_calls; return 0; }
        check(count <= 16384);
        Allocation result{next, std::vector<std::uint64_t>(static_cast<std::size_t>(count),0)};
        next += 0x100000;
        if (old) {
            const auto& source = allocations.at(old).words;
            std::copy_n(source.begin(), std::min(source.size(),result.words.size()),result.words.begin());
            allocations.erase(old);
        }
        const auto base = result.base;
        allocations.emplace(base,std::move(result)); return base;
    }
    void free_0668c0(Address p) override { freed.push_back(p); if(p) check(allocations.erase(p) == 1); }
    std::uint64_t& word(Address p) {
        auto found = allocations.upper_bound(p);
        if (found == allocations.begin()) throw std::out_of_range("address before synthetic allocation");
        --found;
        const auto offset = p - found->first;
        if ((offset & 7) || offset / 8 >= found->second.words.size()) throw std::out_of_range("synthetic allocation boundary");
        return found->second.words[static_cast<std::size_t>(offset/8)];
    }
    std::uint64_t read_word(Address p) override { check(depth > 0); return word(p); }
    void write_word(Address p, std::uint64_t value) override { check(depth > 0); word(p) = value; }
    void invoke_callback(Address p) override {
        check(depth > 0 && p != 0); called.push_back(p); if (callback) callback(p);
    }
};
State make_state(std::uint64_t cookie = UINT64_C(0x81da329a7400b839)) {
    State s{cookie,{},{}};
    check(initialize_onexit_table_0643f8(&s.application_0a47a8,s) == 0);
    check(initialize_onexit_table_0643f8(&s.crt_0a52a8,s) == 0); return s;
}
bool same(Table a,Table b) { return a.begin == b.begin && a.end == b.end && a.capacity_end == b.capacity_end; }
void basics() {
    auto s = make_state(); Fake f;
    check(initialize_onexit_table_0643f8(nullptr,s) == -1);
    Table unusual{4,777,4}; check(initialize_onexit_table_0643f8(&unusual,s) == 0);
    check(same(unusual,s.application_0a47a8));
    unusual = {1,2,3}; check(initialize_onexit_table_0643f8(&unusual,s) == 0 && same(unusual,{1,2,3}));
    check(register_onexit_function_064420(nullptr,9,s,f) == -1 && f.depth == 0);
    check(execute_onexit_table_0643b4(nullptr,s,f) == -1 && f.depth == 0);
    const auto empty = s.application_0a47a8;
    check(execute_onexit_table_0643b4(&s.application_0a47a8,s,f) == 0);
    check(same(empty,s.application_0a47a8) && f.freed.empty());
    const auto max = std::numeric_limits<Address>::max();
    Table sentinel{encode_pointer(max,s.cookie_0a1580),12,34};
    const auto saved = sentinel;
    check(execute_onexit_table_0643b4(&sentinel,s,f) == 0 && same(saved,sentinel));
    check(register_onexit_function_064420(&s.application_0a47a8,11,s,f) == 0);
    check(f.requested == std::vector<std::uint64_t>{32} && f.freed == std::vector<Address>{0});
    const auto base = decode_pointer(s.application_0a47a8.begin,s.cookie_0a1580);
    check(f.word(base) == encode_pointer(11,s.cookie_0a1580));
    for (unsigned i=1;i<32;++i) check(f.word(base+i*8) == s.cookie_0a1580);
    check(register_onexit_function_064420(&s.application_0a47a8,0,s,f) == 0);
    check(register_onexit_function_064420(&s.application_0a47a8,22,s,f) == 0);
    f.callback = [&](Address p) {
        check(p == 11 || p == 22);
        const auto slot = base + (p == 22 ? 16 : 0);
        check(f.word(slot) == s.cookie_0a1580); // cleared before callback
    };
    check(execute_onexit_table_0643b4(&s.application_0a47a8,s,f) == 0);
    check(f.called == std::vector<Address>({22,11}));
    check(f.allocations.empty() && same(s.application_0a47a8,empty) && f.depth == 0);
    f.callback = {};
    check(atexit_04b770(0,s,f) == -1); // null registration succeeds but _onexit returns null
    check(f.allocations.size() == 1);
    check(execute_onexit_table_0643b4(&s.application_0a47a8,s,f) == 0 && f.allocations.empty());
}
void failures() {
    auto s = make_state(); Fake f; f.fail_calls = 1;
    check(register_onexit_function_064420(&s.application_0a47a8,13,s,f) == 0);
    check(f.requested == std::vector<std::uint64_t>({32,4}));
    check(f.freed == std::vector<Address>({0,0}));
    for (Address p=14;p<17;++p) check(register_onexit_function_064420(&s.application_0a47a8,p,s,f) == 0);
    const auto saved = s.application_0a47a8; const auto before = f.allocations;
    f.fail_calls = 2;
    check(register_onexit_function_064420(&s.application_0a47a8,99,s,f) == -1);
    check(same(saved,s.application_0a47a8));
    check(f.requested == std::vector<std::uint64_t>({32,4,8,8}));
    check(f.allocations.begin()->second.words == before.begin()->second.words);
    f.fail_calls = 2; check(onexit_04b734(99,s,f) == 0);
    f.fail_calls = 2; check(atexit_04b770(99,s,f) == -1);
    check(execute_onexit_table_0643b4(&s.application_0a47a8,s,f) == 0);
    // Malformed span's SAR result is large; preserve machine arithmetic and
    // delegate rejection to recalloc rather than performing C++ pointer math.
    Table invalid{encode_pointer(8,s.cookie_0a1580),encode_pointer(0,s.cookie_0a1580),encode_pointer(0,s.cookie_0a1580)};
    const auto invalid_saved = invalid; f.requested.clear(); f.fail_calls = 1;
    check(register_onexit_function_064420(&invalid,1,s,f) == -1);
    check(f.requested == std::vector<std::uint64_t>{3} && same(invalid,invalid_saved));
}
void growth_and_reentrancy() {
    auto s = make_state(); Fake f;
    for (Address p=1;p<=1200;++p) check(register_onexit_function_064420(&s.application_0a47a8,p,s,f) == 0);
    check(f.requested == std::vector<std::uint64_t>({32,64,128,256,512,1024,1536}));
    check(execute_onexit_table_0643b4(&s.application_0a47a8,s,f) == 0);
    check(f.called.size() == 1200);
    for (unsigned i=0;i<1200;++i) check(f.called[i] == 1200-i);
    f.called.clear(); f.requested.clear();
    for (Address p=1;p<=32;++p) check(register_onexit_function_064420(&s.application_0a47a8,p,s,f) == 0);
    f.callback = [&](Address p) {
        if(p == 32) check(register_onexit_function_064420(&s.application_0a47a8,99,s,f) == 0);
        if(p == 99) check(register_onexit_function_064420(&s.application_0a47a8,100,s,f) == 0);
    };
    check(execute_onexit_table_0643b4(&s.application_0a47a8,s,f) == 0);
    check(f.max_depth == 2 && f.depth == 0);
    check(f.requested == std::vector<std::uint64_t>({32,64}));
    check(f.called.size() == 34 && f.called[0] == 32 && f.called[1] == 99 && f.called[2] == 100);
    for (unsigned i=3;i<34;++i) check(f.called[i] == 34-i);
    check(f.allocations.empty());
}
void cookies_and_routing() {
    for (unsigned shift=0;shift<64;++shift) {
        const auto cookie = UINT64_C(0xfedcba9876543200) | shift;
        auto s = make_state(cookie); Fake f;
        for (Address address : {Address(0),Address(1),UINT64_C(0xfedcba9876543210),UINT64_MAX})
            check(decode_pointer(encode_pointer(address,cookie),cookie) == address);
        s.application_0a47a8 = {UINT64_MAX,17,19}; const auto saved = s.application_0a47a8;
        check(onexit_04b734(71,s,f) == 71);
        check(atexit_04b770(72,s,f) == 0);
        check(same(saved,s.application_0a47a8));
        check(execute_onexit_table_0643b4(&s.crt_0a52a8,s,f) == 0);
        check(f.called == std::vector<Address>({72,71}));
    }
}
}
int main() {
    try { basics(); failures(); growth_and_reentrancy(); cookies_and_routing();
        std::cout << "PASS " << checks << " checks: onexit table normal flow, opaque storage and rebuilt synthetic callbacks\n";
    } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
