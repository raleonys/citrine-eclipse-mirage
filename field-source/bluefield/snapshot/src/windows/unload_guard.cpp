// SPDX-License-Identifier: GPL-3.0-only
#include "src/windows/unload_guard.hpp"
#include <windows.h>
#include <tlhelp32.h>
#include <atomic>
#include <cstdint>
#include <algorithm>
#include <cstring>
namespace preloader_l {
namespace {
std::atomic<DWORD> shutdown_caller{0};
bool in_module(std::uint64_t value,std::uint64_t base,std::uint64_t end) noexcept{return value>=base&&value<end;}
bool scan_stack(CONTEXT& context,std::uint64_t base,std::uint64_t end) noexcept{
 const std::uint64_t registers[]={context.Rip,context.Rax,context.Rbx,context.Rcx,context.Rdx,context.Rsi,context.Rdi,context.Rbp,context.R8,context.R9,context.R10,context.R11,context.R12,context.R13,context.R14,context.R15};
 for(auto value:registers)if(in_module(value,base,end))return false;
 MEMORY_BASIC_INFORMATION first{};
 if(!VirtualQuery(reinterpret_cast<const void*>(context.Rsp),&first,sizeof first))return false;
 auto cursor=context.Rsp;const auto allocation=first.AllocationBase;std::uint64_t scanned=0;
 // Conservative return-address/data scan catches pre-lease callback prologues
 // and callbacks temporarily inside a system API. False positives defer unload.
 while(scanned<64ULL*1024*1024){
  MEMORY_BASIC_INFORMATION region{};
  if(!VirtualQuery(reinterpret_cast<const void*>(cursor),&region,sizeof region))return false;
  if(region.AllocationBase!=allocation)return true;
  const auto limit=reinterpret_cast<std::uint64_t>(region.BaseAddress)+region.RegionSize;
  if(limit<=cursor)return false;
  // Refuse a region exceeding the remaining scan budget; never infer it safe.
  if(limit-cursor>64ULL*1024*1024-scanned)return false;
  if(region.State!=MEM_COMMIT||(region.Protect&(PAGE_NOACCESS|PAGE_GUARD)))return false;
  __try {
   for(auto address=cursor;address+sizeof(std::uint64_t)<=limit;address+=sizeof(std::uint64_t)){
    std::uint64_t value;std::memcpy(&value,reinterpret_cast<const void*>(address),sizeof value);
    if(in_module(value,base,end))return false;
   }
  }__except(EXCEPTION_EXECUTE_HANDLER){return false;}
  scanned+=limit-cursor;cursor=limit;
 }
 return false;
}
}
void set_shutdown_caller(std::uint32_t id) noexcept{shutdown_caller.store(id,std::memory_order_release);}
bool external_threads_outside_module() noexcept{
 HMODULE module=nullptr;
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&external_threads_outside_module),&module))return false;
 const auto base=reinterpret_cast<std::uint64_t>(module);
 const auto* dos=reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
 const auto* nt=reinterpret_cast<const IMAGE_NT_HEADERS64*>(base+dos->e_lfanew);
 const auto end=base+nt->OptionalHeader.SizeOfImage;
 HANDLE snapshot=CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD,0);if(snapshot==INVALID_HANDLE_VALUE)return false;
 THREADENTRY32 entry{};entry.dwSize=sizeof entry;bool result=true;
 if(!Thread32First(snapshot,&entry)){CloseHandle(snapshot);return false;}
 const DWORD self=GetCurrentThreadId(),caller=shutdown_caller.load(std::memory_order_acquire),process=GetCurrentProcessId();
 do {
  if(entry.th32OwnerProcessID!=process||entry.th32ThreadID==self||entry.th32ThreadID==caller)continue;
  HANDLE thread=OpenThread(THREAD_SUSPEND_RESUME|THREAD_GET_CONTEXT|THREAD_QUERY_INFORMATION,FALSE,entry.th32ThreadID);
  if(!thread){if(GetLastError()==ERROR_INVALID_PARAMETER)continue;result=false;break;}
  if(SuspendThread(thread)==DWORD(-1)){const auto error=GetLastError();CloseHandle(thread);if(error==ERROR_INVALID_HANDLE||error==ERROR_INVALID_PARAMETER)continue;result=false;break;}
  CONTEXT context{};context.ContextFlags=CONTEXT_FULL;
  bool safe=GetThreadContext(thread,&context)!=FALSE;
  if(safe)safe=scan_stack(context,base,end);
  if(ResumeThread(thread)==DWORD(-1))safe=false;
  CloseHandle(thread);
  if(!safe){result=false;break;}
 }while(Thread32Next(snapshot,&entry));
 if(result&&GetLastError()!=ERROR_NO_MORE_FILES)result=false;
 CloseHandle(snapshot);return result;
}
}
