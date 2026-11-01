#include "storage.hpp"
#include <algorithm>
#include <deque>
#include <iostream>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
using namespace anadius::crt_wildcard_storage;
namespace {
std::size_t checks{};
void require(bool value) { ++checks; if (!value) throw std::runtime_error("fixture check failed"); }
struct Watson {};
struct Call { std::string name; Address a{},b{},c{},d{}; };
struct Fixture final : Effects {
    std::map<Address,std::vector<unsigned char>> memory;
    std::map<Address,std::uint64_t> fake_lengths;
    std::vector<Call> calls;
    std::deque<Address> calloc_results, recalloc_results;
    std::deque<std::uint32_t> copy_results;
    Address next{0x10000};
    bool opaque{};
    Address bytes(const std::string& text) {
        const auto address=next; next+=0x1000;
        memory[address]={text.begin(),text.end()}; memory[address].push_back(0);
        return address;
    }
    Address block(std::size_t size) { const auto p=next;next+=0x1000;memory[p].resize(size);return p; }
    std::pair<std::vector<unsigned char>*,std::size_t> range(Address p,std::uint64_t n) {
        auto it=memory.upper_bound(p);
        if(it==memory.begin()) throw std::domain_error("unmapped fixture address");
        --it; const auto offset=p-it->first;
        if(offset>it->second.size() || n>it->second.size()-offset) throw std::domain_error("fixture range");
        return {&it->second,static_cast<std::size_t>(offset)};
    }
    std::uint64_t length(Address p) override {
        calls.push_back({"length",p});
        if(fake_lengths.contains(p)) return fake_lengths.at(p);
        auto [v,o]=range(p,0); auto it=std::find(v->begin()+o,v->end(),0);
        if(it==v->end()) throw std::domain_error("fixture missing terminator");
        return static_cast<std::uint64_t>(it-(v->begin()+o));
    }
    Address allocate(std::deque<Address>& results,std::uint64_t count,std::uint64_t size) {
        if(!results.empty()) {const auto p=results.front();results.pop_front();return p;}
        if(count>4096 || size>4096 || count*size>4096) throw std::domain_error("oversized fixture allocation");
        return block(static_cast<std::size_t>(count*size));
    }
    Address calloc_066848(std::uint64_t n,std::uint64_t s) override {
        calls.push_back({"calloc",n,s}); return allocate(calloc_results,n,s);
    }
    Address recalloc_070434(Address old,std::uint64_t n,std::uint64_t s) override {
        calls.push_back({"recalloc",old,n,s});
        const auto p=allocate(recalloc_results,n,s);
        if(p && !opaque) {auto& to=memory.at(p);auto& from=memory.at(old);std::copy_n(from.begin(),std::min(from.size(),to.size()),to.begin());memory.erase(old);}
        return p;
    }
    void free_0668c0(Address p) override { calls.push_back({"free",p});if(p)memory.erase(p); }
    std::uint32_t memcpy_s_0767fc(Address to,std::uint64_t cap,Address from,std::uint64_t n) override {
        calls.push_back({"copy",to,cap,from,n});
        if(!copy_results.empty()) {const auto r=copy_results.front();copy_results.pop_front();if(r)return r;}
        if(opaque)return 0;
        if(!to || n>cap)return 22;
        auto [src,so]=range(from,n); auto [dst,ds]=range(to,n);
        std::copy_n(src->begin()+so,static_cast<std::size_t>(n),dst->begin()+ds);return 0;
    }
    std::uint64_t read_word(Address p) override { auto [v,o]=range(p,8);std::uint64_t r{};for(unsigned i=0;i<8;++i)r|=std::uint64_t((*v)[o+i])<<(8*i);return r; }
    void write_word(Address p,std::uint64_t x) override {calls.push_back({"write",p,x});if(!opaque){auto[v,o]=range(p,8);for(unsigned i=0;i<8;++i)(*v)[o+i]=static_cast<unsigned char>(x>>(8*i));}}
    [[noreturn]] void watson_05c9a4() override {calls.push_back({"watson"});throw Watson{};}
    std::vector<Call> named(const std::string& name) const {std::vector<Call> r;for(const auto& c:calls)if(c.name==name)r.push_back(c);return r;}
};
void normal_sweep() {
    for(std::size_t source_size=0;source_size<=23;++source_size) for(std::size_t prefix_size=0;prefix_size<=11;++prefix_size) for(unsigned state=0;state<3;++state) {
        Fixture f; const std::string source(source_size,'x'),prefix(prefix_size,'p');
        const auto s=f.bytes(source),p=f.bytes(prefix);Vector v{};Address old{};
        if(state) {old=f.block(32);v={old,old+(state==2?32:8),old+32};f.write_word(old,0xabc);f.calls.clear();}
        require(append_06e800(s,p,prefix_size,v,f)==0);
        const auto text=f.read_word(v.end-8);auto [bytes,off]=f.range(text,source_size+prefix_size+2);
        require(off==0);const std::string recovered(bytes->begin(),bytes->begin()+source_size+prefix_size);
        require(recovered==prefix+source);
        require((*bytes)[source_size+prefix_size]==0);
        require((*bytes)[source_size+prefix_size+1]==0);
        require(v.capacity_end-v.begin==(state==2?64:32));
        require(v.end-v.begin==(state==2?40:state==1?16:8));
        if(state)require(f.read_word(v.begin)==0xabc);
        auto ca=f.named("calloc");require(ca.front().a==source_size+prefix_size+2 && ca.front().b==1);
        require(ca.size()==(state==0?2:1));
        auto copies=f.named("copy");require(copies.size()==(prefix_size?2:1));
        require(copies.back().d==source_size+1);require(copies.back().b==source_size+2);
        auto frees=f.named("free");require(frees.size()==(state==1?1:2));
        for(const auto& free:frees)require(free.a==0);
        require(f.calls.back().name=="free");
    }
}
void failures() {
    { Fixture f;auto s=f.bytes("a");Vector v{};require(append_06e800(s,0,UINT64_MAX-1,v,f)==12);require(f.calls.size()==1);require(v.begin==0); }
    for(unsigned prefix_size:{0u,2u}) {Fixture f;auto s=f.bytes("a"),p=f.bytes("pp");Vector v{};f.calloc_results={0};try{append_06e800(s,p,prefix_size,v,f);require(false);}catch(const Watson&){}require(f.calls.back().name=="watson");require(f.named("free").empty());require(f.named("calloc").size()==1);require(v.begin==0);auto cp=f.named("copy");require(cp.size()==1);require(cp.front().a==0);}
    {Fixture f;auto s=f.bytes("a"),p=f.bytes("pp");Vector v{};f.copy_results={0,1};try{append_06e800(s,p,2,v,f);require(false);}catch(const Watson&){}require(f.named("copy").size()==2);require(f.named("calloc").size()==1);require(f.named("free").empty());}
    {Fixture f;auto s=f.bytes("a");Vector v{};f.calloc_results={f.block(3),0};const auto text=f.calloc_results.front();require(append_06e800(s,0,0,v,f)==12);require(v.begin==0 && v.end==0 && v.capacity_end==0);auto fr=f.named("free");require(fr.size()==3 && fr[0].a==0 && fr[1].a==text && fr[2].a==0);require(!f.memory.contains(text));}
    {Fixture f;auto s=f.bytes("a");const auto b=f.block(32);Vector v{b,b+32,b+32};f.recalloc_results={0};require(append_06e800(s,0,0,v,f)==12);require(v.begin==b && v.end==b+32 && v.capacity_end==b+32);require(f.memory.contains(b));auto rc=f.named("recalloc");require(rc.size()==1 && rc[0].a==b && rc[0].b==8 && rc[0].c==8);auto fr=f.named("free");require(fr.size()==3 && fr[0].a==0 && fr[1].a!=0 && fr[2].a==0);}
    {Fixture f;auto s=f.bytes("a");Vector v{0x2000,0x1ff8,0x1ff8};require(append_06e800(s,0,0,v,f)==12);require(f.named("recalloc").empty());auto fr=f.named("free");require(fr.size()==2 && fr[0].a!=0 && fr[1].a==0);}
    // Exact arithmetic boundary: length+NUL+prefix == UINT64_MAX passes
    // the observed guard; the extra allocation byte wraps to zero.
    {Fixture f;f.opaque=true;f.fake_lengths[1]=UINT64_MAX-2;f.calloc_results={0x1000,0x2000};Vector v{};require(append_06e800(1,2,1,v,f)==0);auto ca=f.named("calloc");require(ca[0].a==0 && ca[0].b==1);auto cp=f.named("copy");require(cp[0].b==0 && cp[1].b==UINT64_MAX);require(cp[1].d==UINT64_MAX-1);}
    // A full malformed vector with zero physical count still forwards recalloc(0,8).
    {Fixture f;f.opaque=true;f.fake_lengths[1]=0;f.calloc_results={0x1000};f.recalloc_results={0};Vector v{0x2000,0x2000,0x2000};require(append_06e800(1,0,0,v,f)==12);auto rc=f.named("recalloc");require(rc.size()==1 && rc[0].b==0 && rc[0].c==8);}
}
}
int main() {try{normal_sweep();failures();std::cout<<"PASS: "<<checks<<" wildcard storage fixture checks\n";return 0;}catch(const std::exception& e){std::cerr<<e.what()<<" after "<<checks<<" checks\n";return 1;}}
