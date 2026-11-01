// SPDX-License-Identifier: GPL-3.0-only
#include <windows.h>
#include <tlhelp32.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#ifndef _WIN32
// Syntax adapter only: C++ exceptions do not emulate Windows access violations.
#define __try try
#define __except(value) catch(...)
#endif
#ifndef PRELOADER_L_UNLOAD_SOURCE
#define PRELOADER_L_UNLOAD_SOURCE "src/windows/unload_guard.cpp"
#endif
// Sole body under test is the production implementation, not a copied scanner.
#include PRELOADER_L_UNLOAD_SOURCE
#ifndef _WIN32
#undef __try
#undef __except
#endif
namespace {
int checks=0;
void require(bool yes,const char* reason){++checks;if(!yes)throw std::runtime_error(reason);}
constexpr std::uint64_t budget=64ULL*1024*1024;
}
#ifdef _WIN32
namespace {
struct Memory {
 void* data=VirtualAlloc(nullptr,budget+65536,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
 Memory(){require(data!=nullptr,"native VirtualAlloc");}
 ~Memory(){if(data)VirtualFree(data,0,MEM_RELEASE);}
};
void native_scan(){
 Memory memory;CONTEXT context{};context.Rsp=reinterpret_cast<std::uint64_t>(memory.data);
 const auto module_begin=reinterpret_cast<std::uint64_t>(&native_scan),module_end=module_begin+1;
 // Allocation is bigger than the retained budget: reject without calling it safe.
 require(!preloader_l::scan_stack(context,module_begin,module_end),"native over-budget allocation rejected");
 DWORD old=0;require(VirtualProtect(memory.data,budget,PAGE_READONLY,&old)!=FALSE,"native protect budget prefix");
 require(!preloader_l::scan_stack(context,module_begin,module_end),"native exact-budget prefix remains conservative");
 require(VirtualProtect(memory.data,budget,PAGE_READWRITE,&old)!=FALSE,"restore writable prefix");
 *static_cast<std::uint64_t*>(memory.data)=module_begin;
 require(!preloader_l::scan_stack(context,module_begin,module_end),"native module value cannot report safe");
 context.Rip=module_begin;
 require(!preloader_l::scan_stack(context,module_begin,module_end),"native module instruction pointer rejected");
 // Exercise successful same-allocation scan with an actual small VirtualAlloc.
 SYSTEM_INFO system{};GetSystemInfo(&system);
 auto* small=VirtualAlloc(nullptr,system.dwAllocationGranularity,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);
 require(small!=nullptr,"native small allocation");context={};context.Rsp=reinterpret_cast<std::uint64_t>(small);
 const bool outside=preloader_l::scan_stack(context,module_begin,module_end);
 *static_cast<std::uint64_t*>(small)=module_begin;
 require(!preloader_l::scan_stack(context,module_begin,module_end),"native small allocation module word rejected");
 require(VirtualFree(small,0,MEM_RELEASE)!=FALSE,"native small release");
 require(outside,"native small allocation scans to allocation boundary");
}
}
#else
namespace model {
struct Region {std::uintptr_t begin;std::size_t bytes;std::uintptr_t allocation;DWORD state=MEM_COMMIT,protect=4;};
std::vector<Region> regions;
std::vector<std::string> trace;
std::vector<THREADENTRY32> threads;
std::size_t index=0,queries=0,fail_query=0,read_calls=0;
std::uintptr_t watched_begin=0,watched_end=0;
DWORD error=0,open_error=0,suspend_error=0,resume_error=0,enumeration_end=ERROR_NO_MORE_FILES;
bool module_ok=true,snapshot_ok=true,first_ok=true,context_ok=true;
CONTEXT context{};
alignas(IMAGE_NT_HEADERS64) unsigned char image[256]{};
HANDLE snapshot=reinterpret_cast<HANDLE>(1),thread=reinterpret_cast<HANDLE>(2);
void reset(){
 regions.clear();trace.clear();threads.clear();index=queries=fail_query=read_calls=0;
 watched_begin=watched_end=0;error=open_error=suspend_error=resume_error=0;
 enumeration_end=ERROR_NO_MORE_FILES;module_ok=snapshot_ok=first_ok=context_ok=true;context={};
 std::memset(image,0,sizeof image);auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(image);dos->e_lfanew=64;
 reinterpret_cast<IMAGE_NT_HEADERS64*>(image+64)->OptionalHeader.SizeOfImage=sizeof image;
 preloader_l::set_shutdown_caller(0);
}
void stack(std::vector<std::uint64_t>& memory){
 const auto address=reinterpret_cast<std::uintptr_t>(memory.data());context.Rsp=address;
 regions={{address,memory.size()*8,address},{address+memory.size()*8,8,address+memory.size()*8}};
 watched_begin=address;watched_end=address+memory.size()*8;
}
void one_thread(){threads={{sizeof(THREADENTRY32),11,7}};}
bool scan(std::uint64_t begin=0x100000,std::uint64_t end=0x100100){return preloader_l::scan_stack(context,begin,end);}
bool external(){return preloader_l::external_threads_outside_module();}
bool has(const char* item){for(const auto& x:trace)if(x==item)return true;return false;}
}
extern "C" void* __real_memcpy(void*,const void*,std::size_t);
extern "C" void* __wrap_memcpy(void* target,const void* source,std::size_t size){
 const auto address=reinterpret_cast<std::uintptr_t>(source);
 if(model::watched_begin<=address&&address<model::watched_end)++model::read_calls;
 return __real_memcpy(target,source,size);
}
std::size_t VirtualQuery(const void* pointer,MEMORY_BASIC_INFORMATION* result,std::size_t size){
 ++model::queries;if(model::queries==model::fail_query)return 0;
 if(size!=sizeof(*result))return 0;
 const auto address=reinterpret_cast<std::uintptr_t>(pointer);
 for(const auto& r:model::regions)if(address>=r.begin&&address<r.begin+r.bytes){
  *result={reinterpret_cast<void*>(r.begin),reinterpret_cast<void*>(r.allocation),r.bytes,r.state,r.protect};return size;
 }
 return 0;
}
BOOL GetModuleHandleExW(DWORD flags,LPCWSTR,HMODULE* module){
 model::trace.push_back("module");require(flags==(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT),"module lookup does not acquire a new ref");
 if(!model::module_ok)return FALSE;*module=model::image;return 1;
}
DWORD GetCurrentThreadId(){return 10;}DWORD GetCurrentProcessId(){return 7;}DWORD GetLastError(){return model::error;}
HANDLE CreateToolhelp32Snapshot(DWORD flags,DWORD process){
 model::trace.push_back("snapshot");require(flags==TH32CS_SNAPTHREAD&&process==0,"thread snapshot arguments");return model::snapshot_ok?model::snapshot:INVALID_HANDLE_VALUE;
}
BOOL Thread32First(HANDLE handle,THREADENTRY32* entry){
 model::trace.push_back("first");require(handle==model::snapshot&&entry->dwSize==sizeof(*entry),"first snapshot contract");
 if(!model::first_ok||model::threads.empty())return FALSE;*entry=model::threads[0];model::index=0;return 1;
}
BOOL Thread32Next(HANDLE handle,THREADENTRY32* entry){
 model::trace.push_back("next");require(handle==model::snapshot,"next snapshot owner");
 if(++model::index>=model::threads.size()){model::error=model::enumeration_end;return FALSE;}
 *entry=model::threads[model::index];return 1;
}
HANDLE OpenThread(DWORD flags,BOOL inherit,DWORD id){
 model::trace.push_back("open");require(flags==(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_QUERY_INFORMATION)&&inherit==FALSE&&id==11,"thread open contract");
 model::error=model::open_error;return model::open_error?nullptr:model::thread;
}
DWORD SuspendThread(HANDLE handle){
 model::trace.push_back("suspend");require(handle==model::thread,"suspend owner");model::error=model::suspend_error;return model::suspend_error?DWORD(-1):0;
}
BOOL GetThreadContext(HANDLE handle,CONTEXT* context){
 model::trace.push_back("context");require(handle==model::thread&&context->ContextFlags==CONTEXT_FULL,"context request flags");
 if(!model::context_ok)return FALSE;*context=model::context;return 1;
}
DWORD ResumeThread(HANDLE handle){
 model::trace.push_back("resume");require(handle==model::thread,"resume owner");return model::resume_error?DWORD(-1):1;
}
BOOL CloseHandle(HANDLE handle){model::trace.push_back(handle==model::snapshot?"close_snapshot":"close_thread");return 1;}
namespace {
void scan_cases(){
 using namespace model;
 std::vector<std::uint64_t> memory(4);
 reset();stack(memory);require(scan(),"small allocation outside module");require(read_calls==4,("actual production memcpy read trace: "+std::to_string(read_calls)).c_str());
 reset();stack(memory);memory[2]=0x100040;require(!scan(),"stack module pointer rejected");require(read_calls==3,"scan stops at observed module value");memory[2]=0;
 for(auto member:{&CONTEXT::Rip,&CONTEXT::Rax,&CONTEXT::Rbx,&CONTEXT::Rcx,&CONTEXT::Rdx,&CONTEXT::Rsi,&CONTEXT::Rdi,&CONTEXT::Rbp,&CONTEXT::R8,&CONTEXT::R9,&CONTEXT::R10,&CONTEXT::R11,&CONTEXT::R12,&CONTEXT::R13,&CONTEXT::R14,&CONTEXT::R15}){
  reset();stack(memory);context.*member=0x100080;require(!scan(),"register module pointer rejected");require(queries==0&&read_calls==0,"register rejection precedes stack query");
 }
 reset();stack(memory);context.Rip=0x100100;require(scan(),"module end exclusive");
 for(auto failure:{std::size_t(1),std::size_t(2)}){reset();stack(memory);fail_query=failure;require(!scan(),"VirtualQuery failure refused");require(read_calls==0,"query failure before memory read");}
 for(auto protection:{PAGE_GUARD,PAGE_NOACCESS}){reset();stack(memory);regions[0].protect=protection;require(!scan(),"unreadable/guard page refused");require(read_calls==0,"guard page not scanned");}
 reset();stack(memory);regions[0].state=0;require(!scan(),"uncommitted region refused");require(read_calls==0,"uncommitted region not scanned");
 // Independent actual-read observation: the over-budget region's first word
 // is valid memory and is an in-module value. Parent reads it; fixed body must
 // reject the region before even that first read. No invalid address is read.
 reset();stack(memory);memory[0]=0x100001;regions[0].bytes=budget+8;
 require(!scan(),"oversize region refused");require(read_calls==0,"over-budget region was read before refusal");memory[0]=0;
 // Scan-budget boundaries execute the same source body, including its memcpy.
 std::vector<std::uint64_t> large(budget/8+2,0);const auto address=reinterpret_cast<std::uintptr_t>(large.data());
 reset();stack(large);regions={{address,budget,address},{address+budget,8,address+budget}};
 require(!scan(),"exact-budget exhaustion is conservative, not safe");require(read_calls==budget/8,"exact budget read count");
 reset();stack(large);regions={{address,budget-8,address},{address+budget-8,16,address},{address+budget+8,8,address+budget+8}};
 large[budget/8-1]=0x100001;require(!scan(),"over-budget second region refused");require(read_calls==(budget-8)/8,"second region overshoots remaining bytes");large[budget/8-1]=0;
 reset();stack(large);regions={{address,budget-8,address},{address+budget-8,8,address},{address+budget,8,address+budget}};
 require(!scan(),"last permitted eight bytes consume exact budget");require(read_calls==budget/8,"last permitted region not lost");
}
void enumeration_cases(){
 using namespace model;std::vector<std::uint64_t> memory(4);
 auto setup=[&]{reset();stack(memory);one_thread();};
 setup();require(external(),"normal external thread outside module");
 require(trace==std::vector<std::string>{"module","snapshot","first","open","suspend","context","resume","close_thread","next","close_snapshot"},"suspend/context/resume/close sequence");
 setup();module_ok=false;require(!external()&&!has("snapshot"),"module lookup failure stops");
 setup();snapshot_ok=false;require(!external()&&!has("close_snapshot"),"invalid snapshot is not closed");
 setup();first_ok=false;require(!external()&&has("close_snapshot")&&!has("open"),"first enumeration failure closes snapshot");
 setup();threads={{sizeof(THREADENTRY32),10,7},{sizeof(THREADENTRY32),12,7},{sizeof(THREADENTRY32),13,8}};preloader_l::set_shutdown_caller(12);
 require(external()&&!has("open"),"current worker, shutdown caller and other process excluded");
 for(auto failure:{ERROR_INVALID_PARAMETER,DWORD(5)}){setup();open_error=failure;const auto result=external();require(result==(failure==ERROR_INVALID_PARAMETER),"open failure classification");require(!has("suspend")&&has("close_snapshot"),"open failure cleanup");}
 for(auto failure:{ERROR_INVALID_HANDLE,ERROR_INVALID_PARAMETER,DWORD(5)}){setup();suspend_error=failure;const auto result=external();require(result==(failure!=5),"suspend failure classification");require(has("close_thread")&&!has("resume")&&!has("context"),"failed suspend cleanup");}
 setup();context_ok=false;require(!external()&&has("resume")&&has("close_thread"),"context failure still resumes and closes");
 setup();resume_error=5;require(!external()&&has("close_thread")&&has("close_snapshot"),"resume failure never reports safe");
 setup();enumeration_end=5;require(!external(),"unexpected enumeration termination refused");
 setup();context.Rip=reinterpret_cast<std::uintptr_t>(image);require(!external()&&has("resume")&&has("close_thread"),"unsafe register still resumes and closes");
}
}
#endif
int main(){try{
#ifdef _WIN32
 native_scan();std::cout<<"PASS unload native scanner fixture checks="<<checks<<"; actual VirtualQuery/VirtualAlloc; concurrent thread/DLL unload proof NOT_RUN\n";
#else
 scan_cases();enumeration_cases();std::cout<<"PASS unload production-body model checks="<<checks<<"; actual memcpy observation + test-only SDK declarations/API effects; native SDK ABI/SEH/Windows thread suspension NOT_RUN\n";
#endif
 return 0;
}catch(const std::exception& error){std::cerr<<"FAIL unload scan: "<<error.what()<<'\n';return 1;}}
