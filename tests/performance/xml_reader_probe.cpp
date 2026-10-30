#include "preloader_l/xml/reader.hpp"
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>

// Dedicated executable only: count real C++ heap requests without changing the
// production allocator or vendor. Allocation contracts have no timing threshold.
namespace allocation_probe {
std::atomic<bool> enabled{false};
std::atomic<std::size_t> calls{0},bytes{0},input_buffers{0};
std::atomic<long> fail_after{-1};
std::size_t lower=0,upper=0;
struct Counts {std::size_t calls,bytes,input_buffers;};
void account(std::size_t size) noexcept {
    if(!enabled.load(std::memory_order_relaxed))return;
    calls.fetch_add(1,std::memory_order_relaxed);bytes.fetch_add(size,std::memory_order_relaxed);
    if(size>=lower&&size<=upper)input_buffers.fetch_add(1,std::memory_order_relaxed);
}
void begin(std::size_t min=0,std::size_t max=0){
    lower=min;upper=max;calls=0;bytes=0;input_buffers=0;enabled=true;
}
Counts end(){enabled=false;return {calls.load(),bytes.load(),input_buffers.load()};}
}
void* operator new(std::size_t n){
    allocation_probe::account(n);
    if(allocation_probe::fail_after.load(std::memory_order_relaxed)>=0&&allocation_probe::fail_after.fetch_sub(1)==0){
        allocation_probe::fail_after=-1;throw std::bad_alloc();
    }
    if(auto* p=std::malloc(n?n:1))return p;throw std::bad_alloc();
}
void* operator new[](std::size_t n){return ::operator new(n);}
void operator delete(void* p) noexcept {std::free(p);}
void operator delete[](void* p) noexcept {std::free(p);}
void operator delete(void* p,std::size_t) noexcept {std::free(p);}
void operator delete[](void* p,std::size_t) noexcept {std::free(p);}

namespace {
namespace reader=preloader_l::xml_reader_bridge;
std::atomic<std::size_t> result_sink{0};
void require(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
std::size_t traverse(const preloader_l::messages_recovered::XmlNode* node){
    std::size_t result=0;
    for(;node;node=node->next_element()){
        result+=node->name().size();if(auto value=node->attribute("v"))result+=value->size();
        result+=traverse(node->first_element());
    }
    return result;
}
void contracts(){
    reader::Backend backend;
    const std::string input="<R>"+std::string(65536,'x')+"</R>";
    (void)backend.parse_c_string(input); // Warm compiler/vendor startup state.
    allocation_probe::begin(input.size()+1,input.size()+32);
    auto parsed=backend.parse_c_string(input);
    const auto copies=allocation_probe::end();
    require(!parsed.has_error,"large counted XML input parses");
    require(copies.input_buffers==1,"XML parsing must not allocate a second input-sized staging buffer");
    const auto* root=parsed.document->first_element();
    require(root&&root->name()=="R","owned root exists");
    allocation_probe::begin();
    for(unsigned i=0;i<1000;++i)result_sink.store(parsed.document->first_element()==root);
    const auto cache=allocation_probe::end();
    require(cache.calls==0,"repeated wrapper lookup allocates nothing");
    auto retry=backend.parse_c_string("<R><C/></R>");
    allocation_probe::fail_after=0;
    bool failed=false;
    try{(void)retry.document->first_element();}catch(const std::bad_alloc&){failed=true;}
    allocation_probe::fail_after=-1;
    require(failed,"wrapper allocation failure is injected");
    const auto* recovered=retry.document->first_element();
    require(recovered&&recovered->name()=="R"&&recovered->first_element()->name()=="C","wrapper lookup retries after allocation failure without losing document ownership");
    std::printf("PASS: one owned input-sized buffer; repeated wrapper lookups allocate zero times\n");
}
void measure(const char* name,const std::string& input,unsigned iterations){
    reader::Backend backend;
    for(unsigned i=0;i<10;++i){auto parsed=backend.parse_c_string(input);require(!parsed.has_error,"benchmark warmup parses");(void)traverse(parsed.document->first_element());}
    std::vector<long long> durations;durations.reserve(iterations);
    std::size_t sink=0;
    allocation_probe::begin(input.size()+1,input.size()+32);
    for(unsigned i=0;i<iterations;++i){
        const auto start=std::chrono::steady_clock::now();
        {auto parsed=backend.parse_c_string(input);if(parsed.has_error)throw std::runtime_error("benchmark parse failed");sink+=traverse(parsed.document->first_element());}
        durations.push_back(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now()-start).count());
    }
    const auto counts=allocation_probe::end();result_sink=sink;
    std::sort(durations.begin(),durations.end());
    auto percentile=[&](unsigned p){return durations[(durations.size()-1)*p/100];};
    std::printf("{\"case\":\"%s\",\"iterations\":%u,\"input_bytes\":%zu,\"allocation_calls\":%zu,\"allocated_bytes\":%zu,\"input_sized_buffers\":%zu,\"p50_ns\":%lld,\"p95_ns\":%lld,\"p99_ns\":%lld,\"result_digest\":%zu}\n",name,iterations,input.size(),counts.calls,counts.bytes,counts.input_buffers,percentile(50),percentile(95),percentile(99),sink);
}
}
int main(int argc,char** argv)try{
    if(argc>1&&std::strcmp(argv[1],"--benchmark")==0){
        const unsigned iterations=argc>2?static_cast<unsigned>(std::stoul(argv[2])):1000;
        require(iterations>0&&iterations<=100000,"benchmark iterations outside1..100000");
        measure("small-request","<LSX><Request id='1' recipient='EbisuSDK'><GetProfile index='0'/></Request></LSX>",iterations);
        measure("large-text","<R>"+std::string(65536,'x')+"</R>",iterations);
        std::string input="<R>";for(unsigned i=0;i<256;++i)input+="<C v='1'/>";input+="</R>";
        measure("many-elements",input,iterations);
    }else contracts();
    return 0;
}catch(const std::exception& error){allocation_probe::enabled=false;std::fprintf(stderr,"FAIL: %s\n",error.what());return 1;}
