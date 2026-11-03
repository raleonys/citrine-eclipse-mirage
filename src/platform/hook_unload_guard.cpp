// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/platform/hook_unload_guard.hpp"
#include <windows.h>
#include <tlhelp32.h>
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <cstring>
namespace preloader_l {
namespace {
std::atomic<DWORD> shutdown_caller{0};
std::atomic_flag scan_active=ATOMIC_FLAG_INIT;
struct ScanOwners {
 HANDLE thread=nullptr,snapshot=nullptr;
 bool suspended=false;
} retained;
#ifndef NDEBUG
// Protected by scan_active. Record while inspecting a suspended peer, but
// emit only after this scanner has released every owned suspension.
struct ScanTrace {
 const char* reason=nullptr;
 std::uint64_t base=0,end=0,tls_end=0,value=0;
 DWORD peer=0,error=0;unsigned slot=0;
} scan_trace;
void trace_failure(const char* reason,std::uint64_t value=0,unsigned slot=0,DWORD error=0) noexcept{
 if(!scan_trace.reason){scan_trace.reason=reason;scan_trace.value=value;scan_trace.slot=slot;scan_trace.error=error;}
}
void report_scan_failure() noexcept{
 if(!scan_trace.reason||retained.suspended)return;
 const auto incoming=GetLastError();
 char line[256];std::size_t count=0;
 const auto append=[&](const char* text){while(*text&&count<sizeof line)line[count++]=*text++;};
 const auto number=[&](std::uint64_t value,unsigned radix){
  char digits[24];unsigned used=0;
  do{digits[used++]="0123456789abcdef"[value%radix];value/=radix;}while(value);
  while(used&&count<sizeof line)line[count++]=digits[--used];
 };
 const auto rva=scan_trace.value>=scan_trace.base&&scan_trace.value<scan_trace.end?scan_trace.value-scan_trace.base:0;
 const auto tls=scan_trace.tls_end?scan_trace.tls_end-scan_trace.base:0;
 append("UNLOAD_SCAN_BLOCKED reason=");append(scan_trace.reason);
 append(" peer=");number(scan_trace.peer,10);
 append(" image_rva=0x");number(rva,16);append(" slot=");number(scan_trace.slot,10);
 append(" tls_end_rva=0x");number(tls,16);append(" error=");number(scan_trace.error,10);append("\n");
 if(count<sizeof line){DWORD written=0;WriteFile(GetStdHandle(STD_ERROR_HANDLE),line,static_cast<DWORD>(count),&written,nullptr);}
 SetLastError(incoming);
}
#define SCAN_FAILURE(...) trace_failure(__VA_ARGS__)
#else
#define SCAN_FAILURE(...) ((void)0)
#endif
bool release_thread() noexcept{
 if(!retained.thread)return true;
 if(retained.suspended){
  if(ResumeThread(retained.thread)==DWORD(-1)){
   const auto error=GetLastError();DWORD exit_code=STILL_ACTIVE;
   if(error!=ERROR_INVALID_HANDLE&&
      (!GetExitCodeThread(retained.thread,&exit_code)||exit_code==STILL_ACTIVE)){SCAN_FAILURE("resume",0,0,error);return false;}
  }
  retained.suspended=false;
 }
 if(!CloseHandle(retained.thread)&&GetLastError()!=ERROR_INVALID_HANDLE){SCAN_FAILURE("thread-close",0,0,GetLastError());return false;}
 retained.thread=nullptr;return true;
}
bool release_scan_owners() noexcept{
 if(!release_thread())return false;
 if(retained.snapshot){
  if(!CloseHandle(retained.snapshot)&&GetLastError()!=ERROR_INVALID_HANDLE){SCAN_FAILURE("snapshot-close",0,0,GetLastError());return false;}
  retained.snapshot=nullptr;
 }
 return true;
}
bool finish_scan(bool safe) noexcept{
 // Cleanup is required even when the scan already proved unsafe.
 const bool released=release_scan_owners();return released&&safe;
}
bool in_module(std::uint64_t value,std::uint64_t base,std::uint64_t end) noexcept{return value>=base&&value<end;}
std::uint64_t tls_template_end(const IMAGE_NT_HEADERS64& nt,std::uint64_t base,std::uint64_t end) noexcept{
 // Windows TLS setup may leave the raw template's one-past-end pointer in a
 // volatile register. This PE-declared data marker is not a code continuation.
 // Unknown layouts/protections retain the original conservative rejection.
 if(nt.OptionalHeader.NumberOfRvaAndSizes<=IMAGE_DIRECTORY_ENTRY_TLS)return 0;
 const auto& directory=nt.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
 if(directory.Size<sizeof(IMAGE_TLS_DIRECTORY64)||end-base<sizeof(IMAGE_TLS_DIRECTORY64)||
    directory.VirtualAddress>end-base-sizeof(IMAGE_TLS_DIRECTORY64))return 0;
 IMAGE_TLS_DIRECTORY64 tls{};
 std::memcpy(&tls,reinterpret_cast<const void*>(base+directory.VirtualAddress),sizeof tls);
 const auto marker=tls.EndAddressOfRawData;
 if(!in_module(marker,base,end)||tls.StartAddressOfRawData<base||tls.StartAddressOfRawData>marker)return 0;
 const auto* sections=IMAGE_FIRST_SECTION(&nt);
 const auto section_address=reinterpret_cast<std::uint64_t>(sections);
 if(section_address<base||section_address>end||
    nt.FileHeader.NumberOfSections>(end-section_address)/sizeof(IMAGE_SECTION_HEADER))return 0;
 for(unsigned i=0;i<nt.FileHeader.NumberOfSections;++i){
  const auto& section=sections[i];
  const auto length=std::max(section.Misc.VirtualSize,section.SizeOfRawData);
  if(marker-base<section.VirtualAddress||marker-base-section.VirtualAddress>=length)continue;
  if(section.Characteristics&IMAGE_SCN_MEM_EXECUTE)return 0;
  MEMORY_BASIC_INFORMATION region{};
  if(!VirtualQuery(reinterpret_cast<const void*>(marker),&region,sizeof region)||
     region.AllocationBase!=reinterpret_cast<void*>(base)||region.State!=MEM_COMMIT||!region.Protect||
     (region.Protect&(PAGE_NOACCESS|PAGE_GUARD|PAGE_EXECUTE|PAGE_EXECUTE_READ|PAGE_EXECUTE_READWRITE|PAGE_EXECUTE_WRITECOPY)))return 0;
  return marker;
 }
 return 0;
}
bool scan_stack(CONTEXT& context,std::uint64_t base,std::uint64_t end,std::uint64_t tls_end) noexcept{
 // RIP always denotes execution, including an unexpected jump into TLS data.
 if(in_module(context.Rip,base,end)){SCAN_FAILURE("image-rip",context.Rip);return false;}
 const std::uint64_t registers[]={context.Rax,context.Rbx,context.Rcx,context.Rdx,context.Rsi,context.Rdi,context.Rbp,context.R8,context.R9,context.R10,context.R11,context.R12,context.R13,context.R14,context.R15};
#ifndef NDEBUG
 unsigned slot=0;
#endif
 for(auto value:registers){
  if(in_module(value,base,end)&&value!=tls_end){SCAN_FAILURE("image-register",value,slot);return false;}
#ifndef NDEBUG
  ++slot;
#endif
 }
 MEMORY_BASIC_INFORMATION first{};
 if(!VirtualQuery(reinterpret_cast<const void*>(context.Rsp),&first,sizeof first)){SCAN_FAILURE("stack-query",0,0,GetLastError());return false;}
 auto cursor=context.Rsp;const auto allocation=first.AllocationBase;std::uint64_t scanned=0;
 // Conservative return-address/data scan catches pre-lease callback prologues
 // and callbacks temporarily inside a system API. False positives defer unload.
 while(scanned<64ULL*1024*1024){
  MEMORY_BASIC_INFORMATION region{};
  if(!VirtualQuery(reinterpret_cast<const void*>(cursor),&region,sizeof region)){SCAN_FAILURE("stack-query",0,0,GetLastError());return false;}
  if(region.AllocationBase!=allocation)return true;
  const auto limit=reinterpret_cast<std::uint64_t>(region.BaseAddress)+region.RegionSize;
  if(limit<=cursor){SCAN_FAILURE("stack-region");return false;}
  // Refuse a region exceeding the remaining scan budget; never infer it safe.
  if(limit-cursor>64ULL*1024*1024-scanned){SCAN_FAILURE("stack-budget");return false;}
  if(region.State!=MEM_COMMIT||(region.Protect&(PAGE_NOACCESS|PAGE_GUARD))){SCAN_FAILURE("stack-protection",0,region.Protect);return false;}
  __try {
   for(auto address=cursor;address+sizeof(std::uint64_t)<=limit;address+=sizeof(std::uint64_t)){
    std::uint64_t value;std::memcpy(&value,reinterpret_cast<const void*>(address),sizeof value);
    if(in_module(value,base,end)&&value!=tls_end){SCAN_FAILURE("image-stack",value,static_cast<unsigned>((address-context.Rsp)/sizeof value));return false;}
   }
  }__except(EXCEPTION_EXECUTE_HANDLER){SCAN_FAILURE("stack-access");return false;}
  scanned+=limit-cursor;cursor=limit;
 }
 SCAN_FAILURE("stack-budget");return false;
}
}
void set_shutdown_caller(std::uint32_t id) noexcept{shutdown_caller.store(id,std::memory_order_release);}
bool external_threads_outside_module() noexcept{
 // Never block or acquire another peer set while another scanner owns it.
 if(scan_active.test_and_set(std::memory_order_acquire))return false;
#ifndef NDEBUG
 scan_trace={};
#endif
 struct Unlock {~Unlock(){
#ifndef NDEBUG
  report_scan_failure();
#endif
  scan_active.clear(std::memory_order_release);
 }} unlock;
 // Retry the exact handles before enumerating or adding a new suspension.
 if(!release_scan_owners())return false;
 HMODULE module=nullptr;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&external_threads_outside_module),&module)){SCAN_FAILURE("image-query",0,0,GetLastError());return false;}
 const auto base=reinterpret_cast<std::uint64_t>(module);
 const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
 const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
 const auto end=base+nt->OptionalHeader.SizeOfImage;
 const auto tls_end=tls_template_end(*nt,base,end);
#ifndef NDEBUG
 scan_trace.base=base;scan_trace.end=end;scan_trace.tls_end=tls_end;
#endif
 HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE){SCAN_FAILURE("snapshot-open",0,0,GetLastError());return false;}
 retained.snapshot=snapshot;
 THREADENTRY32 entry{};entry.dwSize=sizeof entry;
 if(!Thread32First(snapshot,&entry)){SCAN_FAILURE("thread-first",0,0,GetLastError());return finish_scan(false);}
 const DWORD self=GetCurrentThreadId(),caller=shutdown_caller.load(std::memory_order_acquire),process=GetCurrentProcessId();
 do {
  if(entry.th32OwnerProcessID!=process||entry.th32ThreadID==self||entry.th32ThreadID==caller)continue;
#ifndef NDEBUG
  scan_trace.peer=entry.th32ThreadID;
#endif
  retained.thread=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_QUERY_INFORMATION,FALSE,entry.th32ThreadID);
  if(!retained.thread){if(GetLastError()==ERROR_INVALID_PARAMETER)continue;SCAN_FAILURE("thread-open",0,0,GetLastError());return finish_scan(false);}
  if(SuspendThread(retained.thread)==DWORD(-1)){
   const auto error=GetLastError();
   if(!release_thread())return false;
   if(error==ERROR_INVALID_HANDLE||error==ERROR_INVALID_PARAMETER)continue;
   SCAN_FAILURE("thread-suspend",0,0,error);
   return finish_scan(false);
  }
  retained.suspended=true;
  CONTEXT context{};context.ContextFlags=CONTEXT_FULL;
  bool safe=GetThreadContext(retained.thread,&context)!=FALSE;
  if(!safe)SCAN_FAILURE("thread-context",0,0,GetLastError());
  if(safe)safe=scan_stack(context,base,end,tls_end);
  if(!release_thread())return false;
  if(!safe)return finish_scan(false);
 }while(Thread32Next(snapshot,&entry));
 const bool complete=GetLastError()==ERROR_NO_MORE_FILES;
 if(!complete)SCAN_FAILURE("thread-next",0,0,GetLastError());
 return finish_scan(complete);
}
#undef SCAN_FAILURE
}
