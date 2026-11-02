// SPDX-License-Identifier: GPL-3.0-only

/* Exercise the actual patched engine, injecting only peer-thread OS failures. */
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <MinHook.h>

static int checks, enumeration, resumes, closes, cursor_closes;
static BOOL unavailable, fail_resume, fail_close, fail_heap, fail_context, fail_suspend, fail_cursor, fail_array;
static HANDLE cursor_handle = (HANDLE)(ULONG_PTR)0x4321;
static HANDLE peer_handle = (HANDLE)(ULONG_PTR)0x4322;
static DWORD64 peer_ip;
static int context_writes;
static void require(BOOL ok, const char* message) {
    ++checks;
    if (!ok) { fprintf(stderr, "FAIL: %s\n", message); exit(1); }
}
static LONG NTAPI fake_next(HANDLE process,HANDLE previous,ACCESS_MASK access,ULONG attributes,ULONG flags,PHANDLE next) {
    (void)process;(void)access;(void)attributes;(void)flags;++enumeration;
    if (!previous) {*next=cursor_handle;return 0;}
    *next=NULL;return (LONG)0x8000001AL;
}
static FARPROC WINAPI lookup(HMODULE module,LPCSTR name) {
    if (!strcmp(name,"NtGetNextThread"))return unavailable?NULL:(FARPROC)fake_next;
    return GetProcAddress(module,name);
}
static DWORD WINAPI thread_id(HANDLE thread) {
    return thread==cursor_handle?GetCurrentThreadId()+1:GetThreadId(thread);
}
static BOOL WINAPI duplicate(HANDLE source,HANDLE thread,HANDLE target,LPHANDLE result,DWORD access,BOOL inherit,DWORD flags) {
    if(thread==cursor_handle){*result=peer_handle;return TRUE;}
    return DuplicateHandle(source,thread,target,result,access,inherit,flags);
}
static DWORD WINAPI suspend_peer(HANDLE thread) {
    if(thread==peer_handle){if(fail_suspend){SetLastError(ERROR_ACCESS_DENIED);return (DWORD)-1;}return 0;}
    return SuspendThread(thread);
}
static DWORD WINAPI resume_peer(HANDLE thread) {
    if(thread==peer_handle){++resumes;if(fail_resume){SetLastError(ERROR_ACCESS_DENIED);return (DWORD)-1;}return 1;}
    return ResumeThread(thread);
}
static BOOL WINAPI close_peer(HANDLE thread) {
    if(thread==cursor_handle){++cursor_closes;if(fail_cursor){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return TRUE;}
    if(thread==peer_handle){++closes;if(fail_close){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return TRUE;}
    return CloseHandle(thread);
}
static BOOL WINAPI context_peer(HANDLE thread,LPCONTEXT context) {
    if(thread==peer_handle){if(fail_context){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}context->Rip=peer_ip;return TRUE;}
    return GetThreadContext(thread,context);
}
static BOOL WINAPI update_context_peer(HANDLE thread,const CONTEXT* context) {
    if(thread==peer_handle){peer_ip=context->Rip;++context_writes;return TRUE;}
    return SetThreadContext(thread,context);
}
static BOOL WINAPI exit_code(HANDLE thread,LPDWORD code) {
    if(thread==peer_handle){*code=STILL_ACTIVE;return TRUE;}
    return GetExitCodeThread(thread,code);
}
static BOOL WINAPI release_heap(HANDLE heap,DWORD flags,LPVOID memory) {
    if(fail_heap&&memory){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    return HeapFree(heap,flags,memory);
}
static LPVOID WINAPI allocate_heap(HANDLE heap,DWORD flags,SIZE_T bytes) {
    if(fail_array)return NULL;
    return HeapAlloc(heap,flags,bytes);
}
#define GetProcAddress lookup
#define GetThreadId thread_id
#define DuplicateHandle duplicate
#define SuspendThread suspend_peer
#define ResumeThread resume_peer
#define CloseHandle close_peer
#define GetThreadContext context_peer
#define SetThreadContext update_context_peer
#define GetExitCodeThread exit_code
#define HeapFree release_heap
#define HeapAlloc allocate_heap
/* CMake's include path names the verified build copy, never the original. */
#include "hook.c"

static void reset_faults(void) {
    unavailable=fail_resume=fail_close=fail_heap=fail_context=fail_suspend=fail_cursor=fail_array=FALSE;
}
static void single_hook_ip_relocation(void) {
    unsigned char* target=(unsigned char*)VirtualAlloc(NULL,64,MEM_COMMIT|MEM_RESERVE,PAGE_EXECUTE_READWRITE);
    LPVOID trampoline=NULL;
    int writes;
    require(target!=NULL,"allocate executable hook fixture; never execute its bytes");
    memset(target,0x90,64);target[16]=0xc3;target[32]=0xc3;
    require(MH_CreateHook(target,target+32,&trampoline)==MH_OK&&trampoline!=NULL,"create actual single hook and trampoline");
    peer_ip=(DWORD64)(ULONG_PTR)(target+2);writes=context_writes;
    require(MH_EnableHook(target)==MH_OK,"enable actual single hook");
    require(peer_ip==(DWORD64)(ULONG_PTR)((unsigned char*)trampoline+2)&&context_writes==writes+1,"single enable maps target IP to trampoline");
    require(target[0]==0xe9,"single enable patches target");
    fail_context=TRUE;
    require(MH_DisableHook(target)==MH_ERROR_MEMORY_PROTECT,"unresolved peer context rejects single disable");
    require(target[0]==0xe9&&peer_ip==(DWORD64)(ULONG_PTR)((unsigned char*)trampoline+2),"failed disable preserves live patch and peer IP");
    fail_context=FALSE;writes=context_writes;
    require(MH_DisableHook(target)==MH_OK,"disable actual single hook after context retry");
    require(peer_ip==(DWORD64)(ULONG_PTR)(target+2)&&context_writes==writes+1,"single disable maps trampoline IP back to target");
    require(target[0]==0x90,"single disable restores target bytes");
    require(!HasRetainedThreadOwnership(),"single disable drains peer ownership");
    require(MH_RemoveHook(target)==MH_OK,"remove disabled hook after relocation");
    require(VirtualFree(target,0,MEM_RELEASE),"release hook fixture after removal");
    peer_ip=0;
}
int main(void) {
    FROZEN_THREADS threads;
    HANDLE heap;
    require(MH_Initialize()==MH_OK,"initialize actual engine");heap=g_hHeap;
    require(Freeze(&threads,ALL_HOOKS_POS,ACTION_APPLY_QUEUED)==MH_OK,"Nt enumeration and peer preparation");
    require(enumeration==2&&threads.size==1&&threads.pItems[0].suspended,"duplicated peer owned and suspended");
    fail_resume=TRUE;
    require(Unfreeze(&threads)==MH_ERROR_MEMORY_PROTECT&&HasRetainedThreadOwnership(),"failed resume retains ownership");
    require(closes==0&&g_retainedThreads.pItems[0].suspended,"suspended handle cannot be closed");
    require(MH_Uninitialize()==MH_ERROR_MEMORY_PROTECT&&g_hHeap==heap,"engine heap survives incomplete resume");
    fail_resume=FALSE;fail_close=TRUE;
    require(!RetryRetainedThreadOwnership()&&!g_retainedThreads.pItems[0].suspended,"successful resume retained only for failed close");
    {int count=resumes;require(!RetryRetainedThreadOwnership()&&resumes==count,"retry does not resume twice");}
    fail_close=FALSE;fail_heap=TRUE;
    require(!RetryRetainedThreadOwnership()&&g_retainedThreads.pItems[0].handle==NULL,"array owner survives heap release failure");
    fail_heap=FALSE;
    require(RetryRetainedThreadOwnership()&&!HasRetainedThreadOwnership(),"retry releases final backing allocation");
    fail_cursor=TRUE;
    require(Freeze(&threads,ALL_HOOKS_POS,ACTION_ENABLE)==MH_ERROR_MEMORY_PROTECT&&g_emergencyThread.handle==cursor_handle,"failed enumeration cursor close is retained");
    fail_cursor=FALSE;require(RetryRetainedThreadOwnership()&&!HasRetainedThreadOwnership(),"cursor owner retries cleanly");
    fail_array=fail_close=fail_cursor=TRUE;
    require(Freeze(&threads,ALL_HOOKS_POS,ACTION_ENABLE)==MH_ERROR_MEMORY_PROTECT,"allocation plus two close failures defer cleanup");
    require(g_emergencyThread.handle==peer_handle&&g_emergencyNextThread.handle==cursor_handle,"both unclosed physical handles remain owned after append failure");
    reset_faults();require(RetryRetainedThreadOwnership()&&!HasRetainedThreadOwnership(),"both emergency owners retry cleanly");
    unavailable=TRUE;
    require(Freeze(&threads,ALL_HOOKS_POS,ACTION_ENABLE)==MH_UNKNOWN,"missing Nt API fails closed without Toolhelp fallback");
    unavailable=FALSE;fail_context=TRUE;
    require(Freeze(&threads,ALL_HOOKS_POS,ACTION_ENABLE)==MH_ERROR_MEMORY_PROTECT,"unresolved live thread context rejects patching");
    require(!HasRetainedThreadOwnership(),"failed preparation resumes and closes peers");
    fail_context=FALSE;fail_suspend=TRUE;
    require(Freeze(&threads,ALL_HOOKS_POS,ACTION_ENABLE)==MH_ERROR_MEMORY_PROTECT,"failed live peer suspension rejects patching");
    reset_faults();single_hook_ip_relocation();
    require(MH_Uninitialize()==MH_OK&&g_hHeap==NULL,"engine releases only after successful cleanup");
    printf("PASS: %d actual SAFE-FREEZE failure and retry checks\n",checks);return 0;
}
