#include "native_api.hpp"
#include <array>
#include <cstdio>
#include <limits>
#include <new>
#include <stdexcept>
#include <type_traits>

namespace n = anadius::crt_heap_native;
namespace {
unsigned checks;
void require(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
struct Trace {
    HANDLE heap{};
    DWORD flags{};
    SIZE_T bytes{};
    const void* block{};
    unsigned operation{};
    unsigned calls{};
    void* result{};
    SIZE_T size_result{};
    BOOL bool_result{};
    DWORD error{};
} trace;
HANDLE WINAPI process() { ++trace.calls; trace.operation=1; return trace.heap; }
LPVOID WINAPI allocate(HANDLE h, DWORD f, SIZE_T b) {
    ++trace.calls; trace.operation=2; trace.heap=h; trace.flags=f; trace.bytes=b;
    return trace.result;
}
LPVOID WINAPI reallocate(HANDLE h, DWORD f, LPVOID p, SIZE_T b) {
    ++trace.calls; trace.operation=3; trace.heap=h; trace.flags=f;
    trace.block=p; trace.bytes=b; return trace.result;
}
BOOL WINAPI release(HANDLE h, DWORD f, LPVOID p) {
    ++trace.calls; trace.operation=4; trace.heap=h; trace.flags=f;
    trace.block=p; return trace.bool_result;
}
SIZE_T WINAPI measure(HANDLE h, DWORD f, LPCVOID p) {
    ++trace.calls; trace.operation=5; trace.heap=h; trace.flags=f;
    trace.block=p; return trace.size_result;
}
DWORD WINAPI error() { ++trace.calls; trace.operation=6; return trace.error; }
const n::Api table{process,allocate,reallocate,release,measure,error};
static_assert(std::is_same_v<decltype(table.heap_alloc),decltype(&::HeapAlloc)>);
static_assert(std::is_same_v<decltype(table.heap_realloc),decltype(&::HeapReAlloc)>);
void rejected_tables() {
    const auto reject=[](n::Api api) {
        const auto before=trace.calls;
        bool rejected=false;
        try { const n::Calls bad(api); } catch (const std::invalid_argument&) {
            rejected=true;
        }
        require(rejected,"null function entry rejected");
        require(trace.calls==before,"validation makes no native calls");
    };
    auto api=table; api.get_process_heap=nullptr; reject(api);
    api=table; api.heap_alloc=nullptr; reject(api);
    api=table; api.heap_realloc=nullptr; reject(api);
    api=table; api.heap_free=nullptr; reject(api);
    api=table; api.heap_size=nullptr; reject(api);
    api=table; api.get_last_error=nullptr; reject(api);
}
void fixtures() {
    trace={};
    auto local=table;
    n::Calls calls(local);
    local={}; // Table values are copied, not borrowed from this variable.
    constexpr n::Heap heap=0xfedcba9876543210ULL;
    auto* const block=reinterpret_cast<void*>(0x1234567887654321ULL);
    auto* const result=reinterpret_cast<void*>(0x8765432112345678ULL);
    constexpr DWORD flags=0xfedcba98U;
    constexpr auto maximum=std::numeric_limits<std::uint64_t>::max();
    trace.heap=reinterpret_cast<HANDLE>(heap);
    require(calls.get_process_heap()==heap,"full heap handle returned");
    require(trace.operation==1 && trace.calls==1,"process heap exactly once");
    for (const auto bytes : std::array<std::uint64_t,4>{0,1,0x100000001ULL,maximum}) {
        trace.result=result;
        const auto before=trace.calls;
        require(calls.alloc(heap,flags,bytes)==result,"allocation pointer exact");
        require(trace.operation==2 && trace.calls==before+1,"alloc exactly once");
        require(trace.heap==reinterpret_cast<HANDLE>(heap),"alloc heap exact");
        require(trace.flags==flags && trace.bytes==bytes,"alloc flags and SIZE_T exact");
        require(calls.realloc(heap,flags,block,bytes)==result,"reallocation pointer exact");
        require(trace.operation==3 && trace.heap==reinterpret_cast<HANDLE>(heap),"realloc heap exact");
        require(trace.flags==flags && trace.bytes==bytes && trace.block==block,"realloc arguments exact");
    }
    trace.result=nullptr;
    require(calls.alloc(0,0,0)==nullptr,"null allocation result untouched");
    require(trace.heap==nullptr && trace.flags==0 && trace.bytes==0,"null handle forwarded in fixture");
    require(calls.realloc(0,0,nullptr,0)==nullptr,"null realloc result untouched");
    require(trace.heap==nullptr && trace.block==nullptr && trace.bytes==0,"null realloc forwarded in fixture");
    for (const BOOL value : std::array<BOOL,4>{0,1,-1,0x40000000}) {
        trace.bool_result=value;
        require(calls.free(heap,flags,block)==(value!=0),"free uses full BOOL != 0");
        require(trace.operation==4 && trace.heap==reinterpret_cast<HANDLE>(heap),"free handle exact");
        require(trace.flags==flags && trace.block==block,"free flags and pointer exact");
    }
    trace.bool_result=FALSE;
    require(!calls.free(0,0,nullptr),"null free failure retained in fixture");
    require(trace.heap==nullptr && trace.flags==0 && trace.block==nullptr,"null free forwarded in fixture");
    for (const auto bytes : std::array<std::uint64_t,4>{0,1,0x100000001ULL,maximum}) {
        trace.size_result=static_cast<SIZE_T>(bytes);
        require(calls.size(heap,flags,block)==bytes,"size result includes failure sentinel");
        require(trace.operation==5 && trace.heap==reinterpret_cast<HANDLE>(heap),"size heap exact");
        require(trace.flags==flags && trace.block==block,"size flags and pointer exact");
    }
    for (const DWORD value : std::array<DWORD,4>{0,5,0x80000000U,0xffffffffU}) {
        trace.error=value;
        require(calls.last_error()==value,"raw last error exact");
        require(trace.operation==6,"last error direct API call");
    }
}
void real_calls() {
    n::Calls calls;
    const auto heap=calls.get_process_heap();
    require(heap!=0 && heap==reinterpret_cast<n::Heap>(::GetProcessHeap()),"real process heap");
    // The process heap is borrowed and never destroyed. Only our allocations
    // are passed to native free/realloc/size, with valid flags and modest sizes.
    auto* block=static_cast<unsigned char*>(calls.alloc(heap,HEAP_ZERO_MEMORY,32));
    require(block!=nullptr,"real allocate32");
    block=::new (static_cast<void*>(block)) unsigned char[32];
    try {
        for (unsigned i=0;i<32;++i) require(block[i]==0,"real zero allocation bytes");
        require(calls.size(heap,0,block)>=32,"real size covers request");
        for (unsigned i=0;i<32;++i) block[i]=static_cast<unsigned char>(i+1);
        auto* enlarged=static_cast<unsigned char*>(calls.realloc(heap,0,block,96));
        require(enlarged!=nullptr,"real reallocate96");
        block=::new (static_cast<void*>(enlarged)) unsigned char[96];
        for (unsigned i=0;i<32;++i) require(block[i]==i+1,"real realloc preserves prefix");
        require(calls.size(heap,0,block)>=96,"real enlarged size");
        auto* shrunk=static_cast<unsigned char*>(calls.realloc(heap,0,block,16));
        require(shrunk!=nullptr,"real shrink16");
        block=::new (static_cast<void*>(shrunk)) unsigned char[16];
        for (unsigned i=0;i<16;++i) require(block[i]==i+1,"real shrink preserves prefix");
        require(calls.size(heap,0,block)>=16,"real shrunk size");
    } catch (...) { calls.free(heap,0,block); throw; }
    require(calls.free(heap,0,block),"real free owned allocation");
    ::SetLastError(0x81234567U);
    require(calls.last_error()==0x81234567U,"real raw last error");
    ::SetLastError(0);
}
}
int main() {
    try {
        rejected_tables(); fixtures(); real_calls();
        std::printf("PASS: %u native heap API checks (typed fixtures + own real allocations)\n",checks);
        return 0;
    } catch (const std::exception& e) {
        std::fprintf(stderr,"FAIL after %u checks: %s\n",checks,e.what());
        return 1;
    }
}
