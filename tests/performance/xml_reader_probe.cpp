// SPDX-License-Identifier: GPL-3.0-only

#include "laureondo/xml/reader.hpp"
#include <atomic>
#include <pugixml.hpp>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <optional>
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
// A separate bounded campaign observes lazy-value lookups after parsing and
// retaining earlier views. Leave the existing counters and wrapper fault intact.
thread_local bool recovery_armed=false;
thread_local std::size_t recovery_calls=0,recovery_excluded=0,recovery_fail_at=0;
bool proxy_sized(std::size_t size) noexcept {
#if defined(_MSC_VER) && _ITERATOR_DEBUG_LEVEL != 0
    // Checked-container proxy allocation can occur in noexcept constructors.
    // Keep iterator checking enabled; this campaign covers recoverable OOM.
    return size==sizeof(std::_Container_proxy);
#else
    (void)size;return false;
#endif
}
struct RecoveryProbe {
    explicit RecoveryProbe(std::size_t fail_at=0) noexcept {
        recovery_calls=0;recovery_excluded=0;recovery_fail_at=fail_at;recovery_armed=true;
    }
    ~RecoveryProbe(){recovery_armed=false;}
};
}
void* operator new(std::size_t n){
    allocation_probe::account(n);
    if(allocation_probe::recovery_armed) {
        if(allocation_probe::proxy_sized(n))++allocation_probe::recovery_excluded;
        else if(++allocation_probe::recovery_calls==allocation_probe::recovery_fail_at) {
            allocation_probe::recovery_armed=false;throw std::bad_alloc();
        }
    }
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
void* pugi_allocate(std::size_t size){try{return ::operator new(size);}catch(const std::bad_alloc&){return nullptr;}}
void pugi_release(void* pointer){::operator delete(pointer);}
namespace reader=laureondo::xml_reader_bridge;
std::atomic<std::size_t> result_sink{0};
void require(bool value,const char* reason){if(!value)throw std::runtime_error(reason);}
std::size_t traverse(const laureondo::messages::XmlNode* node){
    std::size_t result=0;
    for(;node;node=node->next_element()){
        result+=node->name().size();if(auto value=node->attribute("v"))result+=value->size();
        result+=traverse(node->first_element());
    }
    return result;
}
struct RetainedViews {
    reader::Backend backend;
    const std::string stem=std::string(192,'v');
    const std::string kept_attribute=stem+"&old",kept_text=stem+"\nold&text";
    const std::string target_attribute=stem+"\nnew&value",target_text=stem+"\nnew&text";
    laureondo::messages::ParseResult parsed{};
    const laureondo::messages::XmlNode *root=nullptr,*old=nullptr,*target=nullptr;
    std::string_view attribute_view;
    const char* text_pointer=nullptr;
    std::optional<std::string_view> result;
    std::vector<std::pair<std::string,std::string>> ordered;
    RetainedViews() {
        std::string input="<R keep='"+stem+"&amp;old'><Old>"+stem+"\r\nold&amp;text</Old><Target a='"+
            stem+"\r\nnew&amp;value' plain='literal'>"+stem+"\n\rnew&amp;text</Target></R>";
        parsed=backend.parse_c_string(input);input.assign(input.size(),'X');
        require(!parsed.has_error,"lazy-cache fixture parses before injection");
        root=parsed.document->first_element();require(root!=nullptr,"lazy-cache fixture root");
        old=root->first_element();require(old!=nullptr,"lazy-cache retained child");
        target=old->next_element();require(target!=nullptr,"lazy-cache target child");
        const auto value=root->attribute("keep");
        require(value&&*value==kept_attribute,"prime an earlier decoded attribute");attribute_view=*value;
        text_pointer=backend.text_003860(*old);
        require(text_pointer&&std::string_view(text_pointer)==kept_text,"prime earlier normalized/decoded text");
    }
    void lookup(unsigned kind) {
        if(kind==0)result=target->attribute("a");
        else if(kind==1) {
            const auto* text=backend.text_003860(*target);
            result=text?std::optional<std::string_view>(text):std::nullopt;
        }else ordered=backend.attributes_in_order(*target);
    }
    void verify_result(unsigned kind) const {
        if(kind<2)require(result&&*result==(kind==0?target_attribute:target_text),"complete lazy value after failure/retry");
        else require(ordered==std::vector<std::pair<std::string,std::string>>{
            {"a",target_attribute},{"plain","literal"}},"complete owned attribute list after failure/retry");
    }
    void verify_retained() {
        require(parsed.document->first_element()==root&&root->first_element()==old&&old->next_element()==target,
            "existing wrappers keep identity after failed lazy lookup");
        const auto value=root->attribute("keep");
        require(value&&value->data()==attribute_view.data()&&*value==kept_attribute&&attribute_view==kept_attribute,
            "earlier borrowed attribute retains identity and complete bytes");
        require(backend.text_003860(*old)==text_pointer&&std::string_view(text_pointer)==kept_text,
            "earlier decoded text retains pointer and complete bytes");
    }
};
void decoded_view_failures() {
    std::size_t sites=0,excluded=0;
    for(unsigned kind=0;kind<3;++kind) {
        std::size_t count=0;
        {
            RetainedViews control;
            {allocation_probe::RecoveryProbe observe;control.lookup(kind);}
            count=allocation_probe::recovery_calls;excluded+=allocation_probe::recovery_excluded;
            require(count>0&&count<128,"bounded nonempty lazy-value allocation campaign");
            control.verify_result(kind);control.verify_retained();
        }
        for(std::size_t ordinal=1;ordinal<=count;++ordinal) {
            RetainedViews fixture;bool failed=false;
            {
                allocation_probe::RecoveryProbe inject(ordinal);
                try{fixture.lookup(kind);}catch(const std::bad_alloc&){failed=true;}
            }
            require(failed&&allocation_probe::recovery_calls==ordinal,"each observed lazy allocation propagates bad_alloc");
            fixture.verify_retained();
            fixture.lookup(kind);fixture.verify_result(kind);fixture.verify_retained();
            (void)fixture.backend.parse_c_string("<Other a='&amp;'/>");fixture.verify_retained();
            ++sites;
        }
    }
    std::printf("PASS: retained decoded views and same-document retry across %zu lazy allocation sites; %zu proxy-sized control requests excluded\n",sites,excluded);
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
    const auto* system_services=retry.document->first_element();
    require(system_services&&system_services->name()=="R"&&system_services->first_element()->name()=="C","wrapper lookup retries after allocation failure without losing document ownership");
    decoded_view_failures();
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
    pugi::set_memory_management_functions(pugi_allocate,pugi_release);
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
