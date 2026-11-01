#include "invalid.hpp"
#include <bit>
#include <cstring>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace anadius::crt_invalid_recovered;
namespace {
unsigned checks;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("check " + std::to_string(checks)); }
struct Watson {};
struct Missing {};
struct HandlerThrow {};
template<class T> void put(void* p, std::size_t offset, T value) {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof value);
}
template<class T> T get(void* p, std::size_t offset) {
    T value;
    std::memcpy(&value, static_cast<std::byte*>(p) + offset, sizeof value);
    return value;
}
struct Fake final : Effects {
    std::array<std::byte, 0x3c1> bytes{};
    void* ptd{bytes.data()+1}; // Deliberately unaligned valid borrowed storage.
    std::uint32_t error{0xdeadbeef};
    std::uint32_t changed{};
    Address cookie{0x1122334455667788};
    Address encoded{cookie};
    std::array<Address, 2> pair{123,456};
    Address called{};
    Arguments arguments{};
    std::uint64_t acquired_index{}, global_index{};
    bool throwing_handler{};
    std::function<void()> on_set;
    std::vector<std::string> events;
    std::uint32_t last_error() override { events.push_back("get-error"); return error; }
    void set_last_error(std::uint32_t value) override {
        events.push_back("set-error"); error = value;
        if (on_set) on_set();
    }
    void* optional_ptd_064dd0(std::uint32_t*, std::uint64_t index) override {
        events.push_back("acquire"); acquired_index=index; error=99; return ptd;
    }
    std::uint32_t locale_changed_0a52d8() override { events.push_back("locale-changed"); return changed; }
    std::array<Address,2> locale_pair_0a1938() override { events.push_back("locale-pair"); return pair; }
    Address encoded_global_handler_0a4f00(std::uint64_t index) override {
        events.push_back("global"); global_index=index; return encoded;
    }
    Address security_cookie_0a1580() override { events.push_back("cookie"); return cookie; }
    void invoke_handler(Address address, const Arguments& args) override {
        events.push_back("handler"); called=address; arguments=args;
        if (throwing_handler) throw HandlerThrow{};
    }
    [[noreturn]] void watson_05c9a4(const Arguments& args) override {
        events.push_back("watson"); arguments=args; throw Watson{};
    }
    [[noreturn]] void missing_ptd_05f03c() override { events.push_back("missing"); throw Missing{}; }
    void global(Address address) { encoded=std::rotl(address, static_cast<int>(cookie&63)) ^ cookie; }
};
void args_and_precedence() {
    const char16_t expr[]=u"size > 0", fn[]=u"sample", file[]=u"test.cpp";
    const Arguments args{expr,fn,file,0xffffffffU,~Address{0}};
    Fake f;
    put(f.ptd,0x3b8,Address{0x123456789abcdef0});
    f.global(0xaaa);
    invalid_parameter_05c7ec(args,f);
    check(f.called==0x123456789abcdef0);
    check(f.arguments==args);
    check(f.error==0xdeadbeef);
    check(f.acquired_index==0);
    check(f.events==std::vector<std::string>{"locale-changed","locale-pair","get-error","acquire","set-error","handler"});
    f.events.clear(); f.changed=1; put(f.ptd,0x3b8,Address{0});
    invalid_parameter_05c7ec(args,f);
    check(f.called==0xaaa && f.arguments==args);
    check(f.events==std::vector<std::string>{"locale-changed","get-error","acquire","set-error","global","cookie","handler"});
    f.ptd=nullptr; f.events.clear(); f.global(0xbbb);
    invalid_parameter_05c7ec(args,f);
    check(f.called==0xbbb && f.error==0xdeadbeef);
    check(f.events==std::vector<std::string>{"locale-changed","get-error","acquire","set-error","global","cookie","handler"});
    f.global(0); f.events.clear();
    bool caught=false; try { invalid_parameter_05c7ec(args,f); } catch(const Watson&) { caught=true; }
    check(caught && f.arguments==args);
    check(f.events.back()=="watson");
}
void all_cookie_bits() {
    constexpr Address handlers[]{0,1,~Address{0},Address{1}<<63,0x1020304050607080,0x1122334455667788};
    for(unsigned shift=0;shift<64;++shift) {
        for(const Address address:handlers) {
            Fake f; f.ptd=nullptr; f.changed=1;
            f.cookie=(0xfedcba9876543200ULL | shift);
            f.global(address);
            if(address==0) {
                bool caught=false;
                try { invalid_parameter_noinfo_05c954(f); } catch(const Watson&) { caught=true; }
                check(caught);
                check(f.events.back()=="watson");
            } else {
                invalid_parameter_noinfo_05c954(f);
                check(f.called==address);
                check(f.events.back()=="handler");
            }
            check(f.arguments==Arguments{});
            check(f.error==0xdeadbeef);
        }
    }
}
void context_helpers() {
    Fake f;
    auto c=make_context(f);
    check(c.ptd==nullptr && c.state_index_ready==0 && c.locale_mode==1);
    check(c.locale_snapshot==f.pair && c.restore_errno==0 && c.restore_doserrno==0);
    f.changed=9; f.events.clear(); auto other=make_context(f);
    check(other.locale_mode==0);
    check(f.events==std::vector<std::string>{"locale-changed"});
    f.events.clear(); c.state_index=555;
    check(optional_context_ptd_05c5d0(c,f)==f.ptd);
    check(c.state_index==0 && c.state_index_ready==1 && f.acquired_index==0);
    check(f.error==0xdeadbeef);
    f.events.clear(); check(optional_context_ptd_05c5d0(c,f)==f.ptd);
    check(required_context_ptd_052500(c,f)==f.ptd && f.events.empty());
    c.ptd=nullptr; c.state_index=7; c.state_index_ready=3;
    check(optional_context_ptd_05c5d0(c,f)==f.ptd && f.acquired_index==7);
    Context s; s.state_index=~std::uint64_t{0}; f.events.clear();
    check(state_index_05c63c(s,f)==0 && s.state_index==0 && s.state_index_ready==1);
    check(f.events==std::vector<std::string>{"get-error","set-error"});
    f.events.clear(); s.state_index=9;
    check(state_index_05c63c(s,f)==9 && f.events.empty());
    f.ptd=nullptr; c.ptd=nullptr; f.events.clear();
    check(optional_context_ptd_05c5d0(c,f)==nullptr);
    check(optional_context_ptd_05c5d0(c,f)==nullptr);
    check(f.events.size()==6); // Null is not cached as a successful lookup.
    bool caught=false; try { required_context_ptd_052500(c,f); } catch(const Missing&) { caught=true; }
    check(caught && f.events.back()=="missing");
}
void cleanup() {
    Fake f; Context c; c.ptd=f.ptd;
    put(f.ptd,0x3a8,0xffffffffU);
    put(f.ptd,0x20,123U); put(f.ptd,0x24,456U);
    release_context(c,f);
    check(f.events.empty());
    check(get<std::uint32_t>(f.ptd,0x3a8)==0xffffffffU);
    c.locale_mode=2; c.saved_errno=0x80000000; c.saved_doserrno=0xffffffff;
    c.restore_errno=1; c.restore_doserrno=2;
    release_context(c,f);
    check(get<std::uint32_t>(f.ptd,0x3a8)==0xfffffffdU);
    check(get<std::uint32_t>(f.ptd,0x20)==c.saved_errno);
    check(get<std::uint32_t>(f.ptd,0x24)==c.saved_doserrno);
    check(f.events.empty());
    c.ptd=nullptr; c.locale_mode=0; f.events.clear();
    release_context(c,f);
    check(c.ptd==f.ptd);
    check(f.events==std::vector<std::string>{"get-error","acquire","set-error"});
    check(f.error==0xdeadbeef);
}
void terminal_and_throwing() {
    Fake f; put(f.ptd,0x3b8,Address{15});
    bool caught=false; try { invalid_parameter_noreturn_05c974(f); } catch(const Watson&) { caught=true; }
    check(caught && f.called==15 && f.arguments==Arguments{});
    check(f.events[f.events.size()-2]=="handler" && f.events.back()=="watson");
    f.events.clear(); f.throwing_handler=true;
    caught=false; try { invalid_parameter_noinfo_05c954(f); } catch(const HandlerThrow&) { caught=true; }
    check(caught && f.events.back()=="handler");
    f.ptd=nullptr; f.global(0); f.events.clear();
    caught=false; try { invalid_parameter_noreturn_05c974(f); } catch(const Watson&) { caught=true; }
    check(caught && f.events.back()=="watson");
}
void snapshot_order() {
    Fake f;
    std::array<std::byte,0x3c0> other{};
    Context c;
    f.on_set=[&] { c.ptd=other.data(); };
    check(optional_context_ptd_05c5d0(c,f)==f.ptd);
    check(c.ptd==other.data());
    c.ptd=nullptr;
    check(required_context_ptd_052500(c,f)==other.data());
    c.ptd=nullptr; c.restore_errno=1; c.restore_doserrno=1;
    c.saved_errno=17; c.saved_doserrno=18;
    f.on_set=[&] { c.ptd=other.data(); c.saved_errno=19; c.saved_doserrno=20; };
    release_context(c,f);
    check(get<std::uint32_t>(other.data(),0x20)==17);
    check(get<std::uint32_t>(other.data(),0x24)==20);
    check(get<std::uint32_t>(f.ptd,0x20)==0 && get<std::uint32_t>(f.ptd,0x24)==0);
}
}
int main() {
    try {
        args_and_precedence(); all_cookie_bits(); context_helpers(); cleanup(); terminal_and_throwing(); snapshot_order();
        std::cout << "PASS: " << checks << " invalid-parameter checks\n";
    } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
