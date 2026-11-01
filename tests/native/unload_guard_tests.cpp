// SPDX-License-Identifier: GPL-3.0-only

// Actual production source with fault-injected Windows API boundaries.
#define NOMINMAX
#include <windows.h>
#include <tlhelp32.h>
#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>

namespace fake {
struct State {
    alignas(IMAGE_NT_HEADERS64) std::array<unsigned char,1024> image{};
    std::array<std::uint64_t,4> stack{};
    std::set<HANDLE> threads,snapshots;
    DWORD error=0;
    int next=100,created=0,suspends=0,resumes=0,closed_owned=0,exit_queries=0;
    int depth=0,foreign_depth=0;
    bool peer=true,resume_fail=false,thread_close_fail=false,snapshot_close_fail=false;
    bool suspend_fail=false,context_fail=false,terminated=false,enumeration_fail=false;
    bool reenter=false,reentry_rejected=false,in_module=false;
} state;
HANDLE handle(){return reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(++state.next));}
BOOL WINAPI module(DWORD,LPCWSTR,HMODULE* out){*out=reinterpret_cast<HMODULE>(state.image.data());return TRUE;}
DWORD WINAPI current_thread(){return 10;}
DWORD WINAPI current_process(){return 7;}
DWORD WINAPI last_error(){return state.error;}
HANDLE WINAPI snapshot(DWORD,DWORD){++state.created;auto h=handle();state.snapshots.insert(h);return h;}
BOOL WINAPI first(HANDLE,THREADENTRY32* out){out->th32ThreadID=state.peer?11:10;out->th32OwnerProcessID=7;return TRUE;}
BOOL WINAPI next(HANDLE,THREADENTRY32*){state.error=state.enumeration_fail?ERROR_ACCESS_DENIED:ERROR_NO_MORE_FILES;return FALSE;}
HANDLE WINAPI open(DWORD,BOOL,DWORD){auto h=handle();state.threads.insert(h);return h;}
DWORD WINAPI suspend(HANDLE){++state.suspends;if(state.suspend_fail){state.error=ERROR_ACCESS_DENIED;return DWORD(-1);}return state.depth++;}
DWORD WINAPI resume(HANDLE){
    ++state.resumes;
    if(state.resume_fail||state.terminated){state.error=ERROR_ACCESS_DENIED;return DWORD(-1);}
    if(state.depth<=state.foreign_depth)throw std::runtime_error("resumed beyond the scan-owned suspension");
    return state.depth--;
}
BOOL WINAPI exit_code(HANDLE,LPDWORD code){++state.exit_queries;*code=state.terminated?0:STILL_ACTIVE;return TRUE;}
BOOL WINAPI close(HANDLE h){
    if(state.threads.contains(h)){
        if(state.thread_close_fail){state.error=ERROR_ACCESS_DENIED;return FALSE;}
        if(!state.terminated&&state.depth>state.foreign_depth)++state.closed_owned;
        state.threads.erase(h);return TRUE;
    }
    if(state.snapshots.contains(h)){
        if(state.snapshot_close_fail){state.error=ERROR_ACCESS_DENIED;return FALSE;}
        state.snapshots.erase(h);return TRUE;
    }
    state.error=ERROR_INVALID_HANDLE;return FALSE;
}
BOOL WINAPI context(HANDLE,LPCONTEXT);
SIZE_T WINAPI query(LPCVOID address,PMEMORY_BASIC_INFORMATION out,SIZE_T){
    const auto start=reinterpret_cast<std::uintptr_t>(state.stack.data());
    const auto at=reinterpret_cast<std::uintptr_t>(address),end=start+sizeof(state.stack);
    *out={};out->State=MEM_COMMIT;out->Protect=PAGE_READWRITE;
    if(at>=start&&at<end){out->BaseAddress=state.stack.data();out->AllocationBase=state.stack.data();out->RegionSize=sizeof(state.stack);return sizeof(*out);}
    if(at==end){out->BaseAddress=reinterpret_cast<void*>(end);out->AllocationBase=reinterpret_cast<void*>(end);out->RegionSize=8;return sizeof(*out);}
    return 0;
}
}
#define GetModuleHandleExW fake::module
#define GetCurrentThreadId fake::current_thread
#define GetCurrentProcessId fake::current_process
#define GetLastError fake::last_error
#define CreateToolhelp32Snapshot fake::snapshot
#define Thread32First fake::first
#define Thread32Next fake::next
#define OpenThread fake::open
#define SuspendThread fake::suspend
#define ResumeThread fake::resume
#define GetExitCodeThread fake::exit_code
#define CloseHandle fake::close
#define GetThreadContext fake::context
#define VirtualQuery fake::query
#include "../../src/platform/hook_unload_guard.cpp"
#undef GetModuleHandleExW
#undef GetCurrentThreadId
#undef GetCurrentProcessId
#undef GetLastError
#undef CreateToolhelp32Snapshot
#undef Thread32First
#undef Thread32Next
#undef OpenThread
#undef SuspendThread
#undef ResumeThread
#undef GetExitCodeThread
#undef CloseHandle
#undef GetThreadContext
#undef VirtualQuery

BOOL WINAPI fake::context(HANDLE,LPCONTEXT out){
    if(state.context_fail)return FALSE;
    if(state.reenter)state.reentry_rejected=!preloader_l::external_threads_outside_module();
    *out={};out->Rsp=reinterpret_cast<std::uint64_t>(state.stack.data());
    if(state.in_module)out->Rip=reinterpret_cast<std::uint64_t>(state.image.data())+32;
    return TRUE;
}
namespace {
int checks;
void require(bool value,const char* reason){++checks;if(!value)throw std::runtime_error(reason);}
void reset(int foreign=0){
    require(fake::state.threads.empty()&&fake::state.snapshots.empty(),"preceding case releases every handle");
    fake::state=fake::State{};fake::state.depth=fake::state.foreign_depth=foreign;
    auto* dos=reinterpret_cast<IMAGE_DOS_HEADER*>(fake::state.image.data());dos->e_lfanew=128;
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(fake::state.image.data()+128);nt->OptionalHeader.SizeOfImage=1024;
    preloader_l::set_shutdown_caller(0);
}
bool scan(){return preloader_l::external_threads_outside_module();}
void resume_retry(){
    reset();auto& s=fake::state;s.resume_fail=true;
    require(!scan(),"failed resume rejects unload");
    require(s.threads.size()==1&&s.snapshots.size()==1&&s.depth==1&&s.closed_owned==0,"failed resume retains physical handle and snapshot");
    require(!scan()&&s.created==1&&s.suspends==1,"persistent failure cannot enumerate or suspend again");
    s.resume_fail=false;s.peer=false;
    require(scan()&&s.depth==0&&s.threads.empty()&&s.snapshots.empty(),"retry resumes the original suspension before scanning again");
    require(s.closed_owned==0&&s.suspends==1,"no live suspended owner closed or double-suspended");
    reset();s.resume_fail=true;require(!scan(),"same-peer retry setup");s.resume_fail=false;
    require(scan()&&s.depth==0&&s.suspends==2&&s.resumes==3,"live peer rescanned only after its previous suspension is repaid");
}
void close_retry(){
    reset(2);auto& s=fake::state;s.thread_close_fail=true;
    require(!scan()&&s.depth==2&&s.resumes==1,"successful resume preserves foreign suspension count");
    require(!scan()&&s.resumes==1&&s.suspends==1,"failed handle close retries without another resume");
    s.thread_close_fail=false;s.peer=false;
    require(scan()&&s.depth==2&&s.resumes==1&&s.threads.empty(),"close-only retry preserves prior thread suspension");
    reset();s.peer=false;s.snapshot_close_fail=true;
    require(!scan()&&s.snapshots.size()==1,"failed snapshot close retains owner");
    require(!scan()&&s.created==1,"persistent snapshot close failure prevents replacement snapshot");
    s.snapshot_close_fail=false;require(scan()&&s.snapshots.empty(),"snapshot owner physically released before replacement scan");
}
void other_failures(){
    reset();auto& s=fake::state;s.suspend_fail=true;s.thread_close_fail=true;
    require(!scan()&&s.depth==0&&s.resumes==0&&s.threads.size()==1,"failed suspend plus failed close retains unsuspended handle");
    s.suspend_fail=s.thread_close_fail=false;s.peer=false;
    require(scan()&&s.resumes==0,"unsuspended retained handle is closed without resume");
    reset();s.context_fail=true;
    require(!scan()&&s.depth==0&&s.threads.empty()&&s.snapshots.empty(),"context failure rejects unload and cleans acquired suspension");
    s.context_fail=false;require(scan(),"context failure retry succeeds");
    reset();s.enumeration_fail=true;
    require(!scan()&&s.threads.empty()&&s.snapshots.empty(),"failed enumeration never reports a complete safe scan");
    s.enumeration_fail=false;require(scan(),"enumeration retry succeeds");
    reset();s.resume_fail=true;require(!scan(),"terminated-peer setup retains owner");
    s.terminated=true;s.depth=0;s.peer=false;
    require(scan()&&s.threads.empty()&&s.snapshots.empty()&&s.exit_queries>=2,"proven terminated peer can be released when resume remains unavailable");
}
void scan_behavior(){
    reset();auto& s=fake::state;s.in_module=true;
    require(!scan()&&s.depth==0,"live module instruction pointer defers unload with balanced resume");
    s.in_module=false;s.stack[2]=reinterpret_cast<std::uint64_t>(s.image.data())+64;
    require(!scan()&&s.depth==0,"module return address still defers unload");
    s.stack[2]=0;s.reenter=true;
    require(scan()&&s.reentry_rejected&&s.threads.empty()&&s.snapshots.empty(),"reentrant scan fails closed without acquiring owners");
}
}
int main()try{resume_retry();close_retry();other_failures();scan_behavior();std::cout<<"PASS: "<<checks<<" actual unload guard failure/retry and stack checks\n";}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
