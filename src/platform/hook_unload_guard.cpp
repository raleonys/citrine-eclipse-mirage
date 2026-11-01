// SPDX-License-Identifier: GPL-3.0-only
#include "preloader_l/platform/hook_unload_guard.hpp"
#include <windows.h>
#include <tlhelp32.h>
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <cstring>
namespace preloader_l {
#ifdef PRELOADER_L_SHUTDOWN_TRACE
void trace_shutdown_failure(const char* reason) noexcept {
 const auto saved=GetLastError();
 const auto output=GetStdHandle(STD_ERROR_HANDLE);
 DWORD written=0;const char prefix[]="SHUTDOWN_TRACE: ";
 DWORD size=0;while(reason[size]&&size<512)++size;
 WriteFile(output,prefix,sizeof(prefix)-1,&written,nullptr);
 WriteFile(output,reason,size,&written,nullptr);
 WriteFile(output,"\r\n",2,&written,nullptr);
 SetLastError(saved);
}
#endif
namespace {
const char* scan_rejection="thread context or scan API rejected";
bool reject_scan(const char* reason) noexcept{scan_rejection=reason;return false;}
std::atomic<DWORD> shutdown_caller{0};
std::atomic_flag scan_active=ATOMIC_FLAG_INIT;
struct ScanOwners {
 HANDLE thread=nullptr,snapshot=nullptr;
 bool suspended=false;
} retained;
bool release_thread() noexcept{
 if(!retained.thread)return true;
 if(retained.suspended){
  if(ResumeThread(retained.thread)==DWORD(-1)){
   const auto error=GetLastError();DWORD exit_code=STILL_ACTIVE;
   if(error!=ERROR_INVALID_HANDLE&&
      (!GetExitCodeThread(retained.thread,&exit_code)||exit_code==STILL_ACTIVE))return false;
  }
  retained.suspended=false;
 }
 if(!CloseHandle(retained.thread)&&GetLastError()!=ERROR_INVALID_HANDLE)return false;
 retained.thread=nullptr;return true;
}
bool release_scan_owners() noexcept{
 if(!release_thread())return false;
 if(retained.snapshot){
  if(!CloseHandle(retained.snapshot)&&GetLastError()!=ERROR_INVALID_HANDLE)return false;
  retained.snapshot=nullptr;
 }
 return true;
}
bool finish_scan(bool safe) noexcept{
 // Cleanup is required even when the scan already proved unsafe.
 const bool released=release_scan_owners();
 if(!safe)trace_shutdown_failure(scan_rejection);
 if(!released)trace_shutdown_failure("scan handle or suspension release rejected");
 return released&&safe;
}
bool in_module(std::uint64_t value,std::uint64_t base,std::uint64_t end) noexcept{return value>=base&&value<end;}
bool scan_stack(CONTEXT& context,std::uint64_t base,std::uint64_t end) noexcept{
 const std::uint64_t registers[]={context.Rip,context.Rax,context.Rbx,context.Rcx,context.Rdx,context.Rsi,context.Rdi,context.Rbp,context.R8,context.R9,context.R10,context.R11,context.R12,context.R13,context.R14,context.R15};
 for(auto value:registers)if(in_module(value,base,end))return reject_scan("peer register contains image address");
 MEMORY_BASIC_INFORMATION first{};
 if(!VirtualQuery(reinterpret_cast<const void*>(context.Rsp),&first,sizeof first))return reject_scan("initial stack query rejected");
 auto cursor=context.Rsp;const auto allocation=first.AllocationBase;std::uint64_t scanned=0;
 // Conservative return-address/data scan catches pre-lease callback prologues
 // and callbacks temporarily inside a system API. False positives defer unload.
 while(scanned<64ULL*1024*1024){
  MEMORY_BASIC_INFORMATION region{};
  if(!VirtualQuery(reinterpret_cast<const void*>(cursor),&region,sizeof region))return reject_scan("stack region query rejected");
  if(region.AllocationBase!=allocation)return true;
  const auto limit=reinterpret_cast<std::uint64_t>(region.BaseAddress)+region.RegionSize;
  if(limit<=cursor)return reject_scan("stack region has no progress");
  // Refuse a region exceeding the remaining scan budget; never infer it safe.
  if(limit-cursor>64ULL*1024*1024-scanned)return reject_scan("stack scan budget exhausted");
  if(region.State!=MEM_COMMIT||(region.Protect&(PAGE_NOACCESS|PAGE_GUARD)))return reject_scan("stack region unreadable");
  __try {
   for(auto address=cursor;address+sizeof(std::uint64_t)<=limit;address+=sizeof(std::uint64_t)){
    std::uint64_t value;std::memcpy(&value,reinterpret_cast<const void*>(address),sizeof value);
    if(in_module(value,base,end))return reject_scan("peer stack contains image address");
   }
  }__except(EXCEPTION_EXECUTE_HANDLER){return reject_scan("stack read exception");}
  scanned+=limit-cursor;cursor=limit;
 }
 return false;
}
}
void set_shutdown_caller(std::uint32_t id) noexcept{shutdown_caller.store(id,std::memory_order_release);}
bool external_threads_outside_module() noexcept{
 // Never block or acquire another peer set while another scanner owns it.
 if(scan_active.test_and_set(std::memory_order_acquire))return false;
 struct Unlock {~Unlock(){scan_active.clear(std::memory_order_release);}} unlock;
 // Retry the exact handles before enumerating or adding a new suspension.
 if(!release_scan_owners()){trace_shutdown_failure("retained scan cleanup rejected");return false;}
 scan_rejection="thread context or scan API rejected";
 HMODULE module=nullptr;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&external_threads_outside_module),&module))return false;
 const auto base=reinterpret_cast<std::uint64_t>(module);
 const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
 const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
 const auto end=base+nt->OptionalHeader.SizeOfImage;
 HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE)return false;
 retained.snapshot=snapshot;
 THREADENTRY32 entry{};entry.dwSize=sizeof entry;
 if(!Thread32First(snapshot,&entry))return finish_scan(false);
 const DWORD self=GetCurrentThreadId(),caller=shutdown_caller.load(std::memory_order_acquire),process=GetCurrentProcessId();
 do {
  if(entry.th32OwnerProcessID!=process||entry.th32ThreadID==self||entry.th32ThreadID==caller)continue;
  retained.thread=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_QUERY_INFORMATION,FALSE,entry.th32ThreadID);
  if(!retained.thread){if(GetLastError()==ERROR_INVALID_PARAMETER)continue;return finish_scan(false);}
  if(SuspendThread(retained.thread)==DWORD(-1)){
   const auto error=GetLastError();
   if(!release_thread())return false;
   if(error==ERROR_INVALID_HANDLE||error==ERROR_INVALID_PARAMETER)continue;
   return finish_scan(false);
  }
  retained.suspended=true;
  CONTEXT context{};context.ContextFlags=CONTEXT_FULL;
  bool safe=GetThreadContext(retained.thread,&context)!=FALSE;
  if(safe)safe=scan_stack(context,base,end);
  if(!release_thread())return false;
  if(!safe)return finish_scan(false);
 }while(Thread32Next(snapshot,&entry));
 const bool complete=GetLastError()==ERROR_NO_MORE_FILES;
 return finish_scan(complete);
}
}
