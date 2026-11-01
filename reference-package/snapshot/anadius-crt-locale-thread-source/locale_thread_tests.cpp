#include "locale_thread.hpp"
#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace ct = anadius::crt_locale_thread_recovered;
using ct::Address;
namespace {
std::uint64_t checks = 0;
void check(bool value) { ++checks; if (!value) throw std::runtime_error("check " + std::to_string(checks)); }
struct Abort {};
struct Fault {};
struct Fx final : ct::Effects {
    static constexpr Address ptd = 0x1000, mb_slot = 0x5000, old_mb = 0x6000, new_mb = 0x7000;
    static constexpr Address old_loc = 0x8000, new_loc = 0x9000, static_mb = 0xa000, static_loc = 0xb000;
    std::map<Address, Address> pointers;
    std::map<Address, std::uint32_t> words;
    std::vector<std::string> events;
    std::vector<Address> freed, locale_freed;
    std::uint32_t mask = 1;
    Address thread = ptd, locale = new_loc;
    int depth = 0;
    std::function<void(std::uint32_t)> on_lock, on_unlock;
    std::function<void(Address)> on_free, on_add, on_release;
    Fx() {
        pointers[ptd+0x88] = old_mb;
        pointers[ptd+0x90] = old_loc;
        pointers[mb_slot] = new_mb;
        words[ptd+0x3a8] = 0;
        words[old_mb] = 1; words[new_mb] = 5; words[static_mb] = 1;
        words[old_loc+0x10] = 1; words[new_loc+0x10] = 4; words[static_loc+0x10] = 1;
    }
    Address read_address(Address a) override { events.push_back("rptr:"+std::to_string(a)); return pointers.at(a); }
    void write_address(Address a, Address v) override { events.push_back("wptr:"+std::to_string(a)); pointers[a] = v; }
    std::uint32_t read_u32(Address a) override { events.push_back("r32:"+std::to_string(a)); return words.at(a); }
    std::uint32_t locale_mask_0a1c6c() override { events.push_back("mask"); return mask; }
    Address current_locale_0a52e0() override { events.push_back("locale"); return locale; }
    Address current_multibyte_slot_0a59d0() override { events.push_back("mbslot"); return mb_slot; }
    Address initial_locale_0a17e0() override { events.push_back("staticloc"); return static_loc; }
    Address initial_multibyte_0a1c70() override { events.push_back("staticmb"); return static_mb; }
    Address required_ptd_064b90() override { events.push_back("ptd"); return thread; }
    void lock_0625f8(std::uint32_t id) override { check(depth == 0); check(id == 4 || id == 5); ++depth; events.push_back("lock:"+std::to_string(id)); if(on_lock) on_lock(id); }
    void unlock_06264c(std::uint32_t id) override { check(depth == 1); --depth; events.push_back("unlock:"+std::to_string(id)); if(on_unlock) on_unlock(id); }
    std::uint32_t atomic_fetch_add_32(Address a, std::uint32_t n) override {
        check(depth == 1); events.push_back("atomic:"+std::to_string(a)+":"+std::to_string(n));
        if(a == 0) throw Fault{};
        const auto before = words.at(a); words[a] += n; return before;
    }
    void free_0668c0(Address a) override { check(depth == 1); events.push_back("free"); freed.push_back(a); if(on_free) on_free(a); }
    void add_locale_ref_070634(Address a) override { events.push_back("add"); ++words.at(a+0x10); if(on_add) on_add(a); }
    void release_locale_ref_0708c0(Address a) override { events.push_back("release"); --words.at(a+0x10); if(on_release) on_release(a); }
    void free_locale_0706c0(Address a) override { events.push_back("freelocale"); locale_freed.push_back(a); }
    [[noreturn]] void abort_05f03c() override { events.push_back("abort"); check(depth == 0); throw Abort{}; }
    bool saw(const std::string& s) const { return std::find(events.begin(),events.end(),s) != events.end(); }
};

void multibyte_matrix() {
    for (const auto flags : {0u, 1u, 2u, 3u, 0x80000000u, 0xffffffffu})
    for (const auto mask : {0u, 1u, 2u, 0x80000000u, 0xffffffffu})
    for (const bool has_locale : {false,true})
    for (const auto old : {Address{0},Fx::old_mb,Fx::static_mb,Fx::new_mb})
    for (const auto ref : {0u,1u,2u,0xffffffffu}) {
        Fx f; f.mask=mask; f.words[Fx::ptd+0x3a8]=flags; f.pointers[Fx::ptd+0x90]=has_locale?Fx::old_loc:0;
        f.pointers[Fx::ptd+0x88]=old; if(old) f.words[old]=ref;
        const auto new_before=f.words[Fx::new_mb];
        const bool pinned=(flags&mask)!=0 && has_locale;
        Address result=0; bool aborted=false;
        try { result=ct::update_thread_multibyte_data_internal_06f540(Fx::ptd,Fx::mb_slot,f); } catch(const Abort&) { aborted=true; }
        check(aborted == (pinned && old==0));
        if(!aborted) check(result == (pinned ? old : Fx::new_mb));
        check(f.depth==0); check(f.saw("lock:5")==!pinned);
        check(f.events[0]=="mask"); check(f.events[1]=="r32:"+std::to_string(Fx::ptd+0x3a8));
        check(f.saw("rptr:"+std::to_string(Fx::ptd+0x90)) == ((flags&mask)!=0));
        check(f.pointers[Fx::ptd+0x88] == (pinned?old:Fx::new_mb));
        const bool changed=!pinned && old!=Fx::new_mb;
        check(f.words[Fx::new_mb] == static_cast<std::uint32_t>(new_before+(changed?1u:0u)));
        if(old && old!=Fx::new_mb) check(f.words[old] == static_cast<std::uint32_t>(ref-(changed?1u:0u)));
        check(f.freed.size() == (changed && old==Fx::old_mb && ref==1 ? 1u:0u));
    }
}

void multibyte_mutations() {
    {
        Fx f; f.on_lock=[&](auto id) { check(id==5); f.pointers[Fx::ptd+0x88]=Fx::new_mb; };
        check(ct::update_thread_multibyte_data_internal_06f540(Fx::ptd,Fx::mb_slot,f)==Fx::new_mb);
        check(f.freed.empty()); check(f.words[Fx::new_mb]==5); check(f.words[Fx::old_mb]==1);
    }
    {
        Fx f; f.on_free=[&](Address a) { check(a==Fx::old_mb); f.pointers[Fx::mb_slot]=Fx::static_mb; };
        f.on_unlock=[&](auto) { f.pointers[Fx::ptd+0x88]=Fx::old_mb; };
        check(ct::update_thread_multibyte_data_internal_06f540(Fx::ptd,Fx::mb_slot,f)==Fx::static_mb);
        check(f.words[Fx::static_mb]==2); check(f.words[Fx::new_mb]==5); check(f.pointers[Fx::ptd+0x88]==Fx::old_mb);
    }
    {
        Fx f; f.pointers[Fx::ptd+0x88]=0; f.pointers[Fx::mb_slot]=0;
        bool aborted=false; try { ct::update_thread_multibyte_data_internal_06f540(Fx::ptd,Fx::mb_slot,f); } catch(const Abort&) {aborted=true;}
        check(aborted); check(f.events[f.events.size()-2]=="unlock:5"); check(f.events.back()=="abort");
    }
    {
        Fx f; f.on_free=[&](auto) { f.pointers[Fx::mb_slot]=0; };
        bool fault=false; try { ct::update_thread_multibyte_data_internal_06f540(Fx::ptd,Fx::mb_slot,f); } catch(const Fault&) {fault=true;}
        check(fault); check(f.depth==1); check(!f.saw("abort")); check(!f.saw("unlock:5"));
        // Deliberately unsupported fault/unwind domain: no invented null guard or cleanup.
    }
    {
        Fx f; check(ct::update_thread_multibyte_06f658(f)==Fx::new_mb);
        const Address second=0x2000; f.thread=second; f.pointers[second+0x88]=Fx::static_mb;
        f.pointers[second+0x90]=Fx::static_loc; f.words[second+0x3a8]=1;
        check(ct::update_thread_multibyte_06f658(f)==Fx::static_mb);
        check(std::count(f.events.begin(),f.events.end(),"ptd")==2);
        check(f.events[0]=="ptd"); check(f.events[1]=="mbslot");
    }
}

void locale_nolock_matrix() {
    constexpr Address cell=0xc000;
    for(const auto old : {Address{0},Fx::old_loc,Fx::static_loc,Fx::new_loc})
    for(const auto replacement : {Address{0},Fx::new_loc})
    for(const auto destination : {Address{0},cell})
    for(const auto ref : {0u,1u,2u,0xffffffffu}) {
        Fx f; f.pointers[cell]=old; if(old) f.words[old+0x10]=ref;
        const auto new_before=f.words[Fx::new_loc+0x10];
        check(ct::update_locale_nolock_0709d8(destination,replacement,f)==((destination&&replacement)?replacement:0));
        const bool changed=destination && replacement && old!=replacement;
        check(f.pointers[cell] == (changed?replacement:old));
        check(f.saw("add")==changed); check(f.saw("release")== (changed&&old));
        check(f.words[Fx::new_loc+0x10]==static_cast<std::uint32_t>(new_before+(changed?1u:0u)));
        if(old && old!=replacement) check(f.words[old+0x10]==static_cast<std::uint32_t>(ref-(changed?1u:0u)));
        check(f.locale_freed.size()==(changed && old==Fx::old_loc && ref==1 ? 1u:0u));
        check(!f.saw("lock:4")); check(f.depth==0);
        if(!destination || !replacement) check(f.events.empty());
    }
}

void locale_mutations() {
    constexpr Address cell=0xc000;
    {
        Fx f; f.pointers[cell]=Fx::old_loc;
        f.on_add=[&](auto a) { check(a==Fx::new_loc); check(f.pointers[cell]==Fx::new_loc); f.pointers[cell]=Fx::static_loc; };
        f.on_release=[&](auto a) { check(a==Fx::old_loc); f.words[a+0x10]=9; };
        check(ct::update_locale_nolock_0709d8(cell,Fx::new_loc,f)==Fx::new_loc);
        check(f.pointers[cell]==Fx::static_loc); check(f.locale_freed.empty());
        check(std::find(f.events.begin(),f.events.end(),"add")<std::find(f.events.begin(),f.events.end(),"release"));
    }
    {
        Fx f; f.pointers[cell]=Fx::old_loc; f.words[Fx::old_loc+0x10]=8;
        f.on_release=[&](auto a) { f.words[a+0x10]=0; };
        check(ct::update_locale_nolock_0709d8(cell,Fx::new_loc,f)==Fx::new_loc);
        check(f.locale_freed==std::vector<Address>{Fx::old_loc});
    }
    {
        Fx f; f.on_lock=[&](auto id) { check(id==4); f.locale=Fx::static_loc; };
        f.on_unlock=[&](auto) { f.pointers[Fx::ptd+0x90]=Fx::new_loc; };
        check(ct::update_thread_locale_070968(f)==Fx::static_loc);
        check(f.pointers[Fx::ptd+0x90]==Fx::new_loc); check(f.depth==0);
        check(f.locale_freed==std::vector<Address>{Fx::old_loc});
    }
    {
        Fx f; f.locale=0;
        bool aborted=false; try { ct::update_thread_locale_070968(f); } catch(const Abort&) {aborted=true;}
        check(aborted); check(f.events[f.events.size()-2]=="unlock:4"); check(f.events.back()=="abort");
        check(f.pointers[Fx::ptd+0x90]==Fx::old_loc); check(!f.saw("add"));
    }
    {
        Fx f; f.on_add=[](auto) { throw Fault{}; };
        bool fault=false; try { ct::update_thread_locale_070968(f); } catch(const Fault&) {fault=true;}
        check(fault); check(f.depth==1); check(!f.saw("unlock:4")); check(!f.saw("release"));
    }
}

void locale_thread_matrix() {
    for(const auto flags : {0u,1u,2u,3u,0x80000000u,0xffffffffu})
    for(const auto mask : {0u,1u,2u,0x80000000u,0xffffffffu})
    for(const auto old : {Address{0},Fx::old_loc,Fx::new_loc,Fx::static_loc}) {
        Fx f; f.words[Fx::ptd+0x3a8]=flags; f.mask=mask; f.pointers[Fx::ptd+0x90]=old;
        const bool pinned=(flags&mask)!=0 && old!=0;
        check(ct::update_thread_locale_070968(f)==(pinned?old:Fx::new_loc));
        check(f.depth==0); check(f.saw("lock:4")==!pinned); check(f.saw("locale")==!pinned);
        check(f.events[0]=="ptd"); check(f.events[1]=="r32:"+std::to_string(Fx::ptd+0x3a8)); check(f.events[2]=="mask");
        check(f.pointers[Fx::ptd+0x90]==(pinned?old:Fx::new_loc));
    }
}
}
int main() {
    try { multibyte_matrix(); multibyte_mutations(); locale_nolock_matrix(); locale_mutations(); locale_thread_matrix();
        std::cout<<"PASS: "<<checks<<" locale-thread checks\n"; return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
