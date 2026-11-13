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
#include <sstream>
#include "../../tools/citrine_eclipse_mirage_runtime_host/unload_diagnostics.hpp"

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
    std::uint64_t register_value=0,instruction=0;
    DWORD image_protection=PAGE_READWRITE;
    DWORD stack_state=MEM_COMMIT,stack_protection=PAGE_READWRITE;
    unsigned stack_queries=0,fail_stack_query=0;
    volatile LONG stack_faults=0;
    void* stack_override=nullptr;
    SIZE_T stack_region_size=sizeof(stack);
    bool nonadvancing_stack_region=false;
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
    const auto image=reinterpret_cast<std::uintptr_t>(state.image.data());
    const auto requested=reinterpret_cast<std::uintptr_t>(address);
    if(requested>=image&&requested<image+state.image.size()){
        *out={};out->State=MEM_COMMIT;out->Protect=state.image_protection;
        out->BaseAddress=state.image.data();out->AllocationBase=state.image.data();
        out->RegionSize=state.image.size();return sizeof(*out);
    }
    ++state.stack_queries;
    if(state.stack_queries==state.fail_stack_query)return 0;
    void* const storage=state.stack_override?state.stack_override:state.stack.data();
    const auto start=reinterpret_cast<std::uintptr_t>(storage);
    const auto at=reinterpret_cast<std::uintptr_t>(address),end=start+state.stack_region_size;
    *out={};out->State=state.stack_state;out->Protect=state.stack_protection;
    if(at>=start&&at<end){out->BaseAddress=storage;out->AllocationBase=storage;out->RegionSize=state.nonadvancing_stack_region?0:state.stack_region_size;return sizeof(*out);}
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
// Compile-time tripwires: scanner output must never return to the critical path.
#define WriteFile UNLOAD_GUARD_MUST_NOT_CALL_WriteFile
#define GetStdHandle UNLOAD_GUARD_MUST_NOT_CALL_GetStdHandle
#include "../../src/platform/hook_unload_guard.cpp"
#undef WriteFile
#undef GetStdHandle
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
    if(state.reenter)state.reentry_rejected=!citrine_eclipse_mirage::external_threads_outside_module();
    *out={};out->Rsp=reinterpret_cast<std::uint64_t>(state.stack_override?state.stack_override:state.stack.data());
    out->R11=state.register_value;out->Rip=state.instruction;
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
    citrine_eclipse_mirage::set_shutdown_caller(0);
}
bool scan(){return citrine_eclipse_mirage::external_threads_outside_module();}
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
void volatile_register_guard(){
    reset();
    auto& s=fake::state;
    // Only the separately validated TLS endpoint qualifies for a data
    // exemption. Other image references in R11 remain unsafe for unload.
    const auto base=reinterpret_cast<std::uint64_t>(s.image.data());
    s.register_value=base+128;
    require(!scan()&&s.depth==0&&s.suspends==1&&s.resumes==1,
        "foreign R11 image address defers unload despite safe RIP and stack");
    require(s.threads.empty()&&s.snapshots.empty()&&s.closed_owned==0,
        "R11-only rejection releases exact scan suspension and handles");
    s.register_value=base+0x2000;
    require(scan()&&s.depth==0&&s.suspends==2&&s.resumes==2,
        "R11 outside image alone does not strand a finished scan");
    require(s.threads.empty()&&s.snapshots.empty(),
        "successful R11 scan has no leaked native owners");
    s.register_value=base+128;
    citrine_eclipse_mirage::set_shutdown_caller(11);
    const auto prior=s.suspends;
    require(scan()&&s.suspends==prior&&s.depth==0,
        "explicit DLL shutdown caller is excluded without suspending its stack");
    citrine_eclipse_mirage::set_shutdown_caller(0);
    require(!scan()&&s.suspends==prior+1&&s.resumes==s.suspends,
        "clearing caller exclusion restores conservative R11 protection");
}
void scan_behavior(){
    reset();auto& s=fake::state;s.in_module=true;
    require(!scan()&&s.depth==0,"live module instruction pointer defers unload with balanced resume");
    s.in_module=false;s.stack[2]=reinterpret_cast<std::uint64_t>(s.image.data())+64;
    require(!scan()&&s.depth==0,"module return address still defers unload");
    s.stack[2]=0;s.reenter=true;
    require(scan()&&s.reentry_rejected&&s.threads.empty()&&s.snapshots.empty(),"reentrant scan fails closed without acquiring owners");
}
// A false external-quiescence proof must retain unload ownership until a
// complete safe retry. Exercise the concrete stack proof behind that contract.
void stack_region_refusals(){
    auto reject_and_retry=[](const char* reason){
        auto& s=fake::state;const auto foreign=s.foreign_depth;
        require(!scan(),reason);
        require(s.depth==foreign&&s.suspends==1&&s.resumes==1&&s.closed_owned==0&&
                s.threads.empty()&&s.snapshots.empty(),
                "stack proof refusal releases only its own suspension and handles");
        s.fail_stack_query=0;s.stack_state=MEM_COMMIT;s.stack_protection=PAGE_READWRITE;
        s.nonadvancing_stack_region=false;
        require(scan()&&s.depth==foreign&&s.suspends==2&&s.resumes==2&&
                s.threads.empty()&&s.snapshots.empty(),
                "repairing stack observation permits a fresh complete unload proof");
    };
    for(unsigned failure:{1u,2u,3u}){
        reset(2);fake::state.fail_stack_query=failure;
        reject_and_retry("failed initial, current or next-region query refuses unload");
    }
    for(DWORD protection:{DWORD(PAGE_NOACCESS),DWORD(PAGE_READWRITE|PAGE_GUARD)}){
        reset();fake::state.stack_protection=protection;
        reject_and_retry("inaccessible or guarded stack region refuses unload");
    }
    reset();fake::state.stack_state=MEM_RESERVE;
    reject_and_retry("uncommitted stack region refuses unload");
    reset();fake::state.nonadvancing_stack_region=true;
    reject_and_retry("nonadvancing stack region refuses unload");
}
LONG CALLBACK observe_stack_read(PEXCEPTION_POINTERS exception){
    const auto& record=*exception->ExceptionRecord;
    if(record.ExceptionCode==EXCEPTION_ACCESS_VIOLATION&&record.NumberParameters>=2){
        const auto address=record.ExceptionInformation[1];
        const auto start=reinterpret_cast<ULONG_PTR>(fake::state.stack_override);
        if(address>=start&&address-start<4096)InterlockedIncrement(&fake::state.stack_faults);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}
void oversized_stack_region(){
    reset();auto& s=fake::state;
    void* page=VirtualAlloc(nullptr,4096,MEM_RESERVE|MEM_COMMIT,PAGE_NOACCESS);
    require(page!=nullptr,"allocate one protected page as a stack-read tripwire");
    auto observer=AddVectoredExceptionHandler(1,observe_stack_read);
    if(!observer){VirtualFree(page,0,MEM_RELEASE);require(false,"install stack-read observer");}
    s.stack_override=page;s.stack_region_size=64ULL*1024*1024+8;
    // The query shim claims a readable oversized region. Physical protection
    // exposes any attempted read, even if the production SEH handler catches it.
    const bool safe=scan();
    const auto queries=s.stack_queries;const auto faults=s.stack_faults;
    s.stack_override=nullptr;s.stack_region_size=sizeof(s.stack);
    const bool observer_removed=RemoveVectoredExceptionHandler(observer)!=0;
    const bool page_freed=VirtualFree(page,0,MEM_RELEASE)!=FALSE;
    require(observer_removed&&page_freed,"release the observer and protected probe page");
    require(!safe&&queries==2&&faults==0,
            "over-budget region refuses unload before reading any stack bytes");
    require(s.depth==0&&s.suspends==1&&s.resumes==1&&s.closed_owned==0&&
            s.threads.empty()&&s.snapshots.empty(),
            "budget refusal balances its exact native scan owners");
    require(scan()&&s.suspends==2&&s.resumes==2&&s.threads.empty()&&s.snapshots.empty(),
            "bounded stack proof succeeds after oversized observation is repaired");
}
void tls_data_marker(){
    reset();auto& s=fake::state;
    const auto base=reinterpret_cast<std::uint64_t>(s.image.data());
    auto* nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(s.image.data()+128);
    nt->FileHeader.NumberOfSections=1;
    nt->FileHeader.SizeOfOptionalHeader=sizeof(IMAGE_OPTIONAL_HEADER64);
    nt->OptionalHeader.NumberOfRvaAndSizes=IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    auto& directory=nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS];
    directory.VirtualAddress=512;directory.Size=sizeof(IMAGE_TLS_DIRECTORY64);
    auto* tls=reinterpret_cast<IMAGE_TLS_DIRECTORY64*>(s.image.data()+512);
    tls->StartAddressOfRawData=base+800;tls->EndAddressOfRawData=base+900;
    auto* section=IMAGE_FIRST_SECTION(nt);
    section->VirtualAddress=768;section->Misc.VirtualSize=256;
    section->Characteristics=IMAGE_SCN_MEM_READ|IMAGE_SCN_MEM_WRITE;
    s.register_value=base+900;
    require(scan()&&s.depth==0,"non-executable PE TLS end data in R11 is not active image code");
    s.stack[1]=base+900;
    require(scan(),"same validated TLS endpoint on the stack is data, not a return address");
    s.instruction=base+900;
    require(!scan(),"instruction pointer inside the image always blocks unload, even at TLS end");
    s.instruction=0;s.register_value=base+899;
    require(!scan(),"adjacent image data receives no TLS endpoint exemption");
    s.register_value=base+32;
    require(!scan(),"other image addresses in R11 remain unsafe");
    s.register_value=base+900;section->Characteristics|=IMAGE_SCN_MEM_EXECUTE;
    require(!scan(),"executable PE section cannot qualify as a TLS data marker");
    section->Characteristics&=~IMAGE_SCN_MEM_EXECUTE;s.image_protection=PAGE_EXECUTE_READWRITE;
    require(!scan(),"executable page protection blocks the data exemption");
    s.image_protection=PAGE_READWRITE;directory.VirtualAddress=1020;
    require(!scan(),"out-of-image TLS directory fails closed");
    directory.VirtualAddress=512;directory.Size=1;
    require(!scan(),"truncated TLS directory fails closed");
    directory.Size=sizeof(IMAGE_TLS_DIRECTORY64);tls->StartAddressOfRawData=base+901;
    require(!scan(),"reversed TLS data range fails closed");
}
#ifndef NDEBUG
namespace wire=citrine_eclipse_mirage::unload_diagnostics;
namespace diagnostic=citrine_eclipse_mirage::tools::unload_diagnostics;
bool copy_memory(std::uintptr_t address,void* out,std::size_t count) noexcept {
    std::memcpy(out,reinterpret_cast<const void*>(address),count);return true;
}
wire::Snapshot scan_snapshot(){
    wire::Snapshot value{};
    require(wire::read_snapshot(reinterpret_cast<std::uintptr_t>(&citrine_eclipse_mirage::scan_record),diagnostic::read_image,value),
            "quiescent actual scanner record is readable");
    return value;
}
void diagnostic_publication(){
    reset();auto& s=fake::state;
    const auto before=scan_snapshot().sequence;
    s.register_value=reinterpret_cast<std::uintptr_t>(s.image.data())+128;s.error=12345;
    require(!scan()&&s.error==12345,"diagnostic publication preserves failing result and LastError");
    auto value=scan_snapshot();
    require(value.sequence==before+2&&value.reason==static_cast<std::uint64_t>(wire::Reason::image_register)&&
            value.peer==11&&value.slot==10&&value.image_rva==128&&!value.suspended,
            "actual scanner publishes exact peer, R11 index and image-relative offset after cleanup");
    require(s.depth==0&&s.threads.empty()&&s.snapshots.empty(),"diagnostics retain no scan owners");
    std::ostringstream output;output<<std::hex;
    require(diagnostic::report(output,reinterpret_cast<std::uintptr_t>(&value),copy_memory)&&
            output.str().find("reason=image-register peer=11 image_rva=0x80 slot=10")!=std::string::npos&&
            (output.flags()&std::ios::basefield)==std::ios::hex,"host formats a stable record and restores stream flags");
    s.register_value=0;s.resume_fail=true;
    require(!scan(),"failed resume diagnostic setup");value=scan_snapshot();
    require(value.reason==static_cast<std::uint64_t>(wire::Reason::resume)&&value.suspended==1&&s.depth==1,
            "failed resume publishes retained ownership without releasing it");
    output.str("");
    require(!diagnostic::report(output,reinterpret_cast<std::uintptr_t>(&value),copy_memory)&&output.str().empty(),
            "host suppresses output for a retained scanner-owned suspension");
    s.resume_fail=false;s.peer=false;
    require(scan(),"diagnostic case repairs exact pending owners");value=scan_snapshot();
    require(value.reason==0&&!value.suspended,"successful retry replaces stale refusal");
    require(!diagnostic::report(output,reinterpret_cast<std::uintptr_t>(&value),copy_memory)&&output.str().empty(),
            "successful retry produces no refusal log");
    SetLastError(12345);std::uintptr_t found=0;
    require(!diagnostic::locate(nullptr,found)&&GetLastError()==12345,"failed host image read preserves LastError");
}
void diagnostic_reader_bounds(){
    wire::Snapshot value{},out{};const auto address=reinterpret_cast<std::uintptr_t>(&value);
    unsigned reads=0;
    auto reader=[&](std::uintptr_t at,void* target,std::size_t count){++reads;return copy_memory(at,target,count);};
    require(!wire::read_snapshot((std::numeric_limits<std::uintptr_t>::max)(),reader,out)&&reads==0,
            "overflowing record address is rejected without reading");
    value.sequence=1;
    require(!wire::read_snapshot(address,reader,out)&&reads==3,"busy publisher is rejected after exactly three reads");
    value.sequence=2;reads=0;
    auto moving=[&](std::uintptr_t at,void* target,std::size_t count){
        ++reads;if(count==sizeof(wire::Snapshot))value.sequence+=2;return copy_memory(at,target,count);
    };
    require(!wire::read_snapshot(address,moving,out)&&reads==9,"moving publisher is rejected after three bounded attempts");
    for(unsigned invalid=0;invalid<6;++invalid){
        value={};
        switch(invalid){case 0:value.magic1=0;break;case 1:value.version=2;break;case 2:value.bytes=8;break;
            case 3:value.reason=999;break;case 4:value.suspended=2;break;case 5:value.peer=1ULL<<32;break;}
        require(!wire::read_snapshot(address,copy_memory,out),"malformed wire records are rejected");
    }
    value={};value.reason=static_cast<std::uint64_t>(wire::Reason::stack_budget);
    std::ostringstream output;
    auto unreadable=[](std::uintptr_t,void*,std::size_t){return false;};
    require(!diagnostic::report(output,address,unreadable)&&output.str().empty(),"unreadable snapshot emits no partial report");
    output.setstate(std::ios::badbit);output.exceptions(std::ios::goodbit);
    require(!diagnostic::report(output,address,copy_memory),"failed host output remains best effort");
    struct FailingBuffer:std::streambuf{int_type overflow(int_type) override{throw std::runtime_error("output fault");}} buffer;
    std::ostream throwing(&buffer);throwing.exceptions(std::ios::badbit);
    require(!diagnostic::report(throwing,address,copy_memory),"throwing host output cannot escape the diagnostic boundary");
}
void diagnostic_image_bounds(){
    alignas(8) std::array<unsigned char,4096> image{};
    const auto base=reinterpret_cast<std::uintptr_t>(image.data());
    IMAGE_DOS_HEADER dos{};dos.e_magic=IMAGE_DOS_SIGNATURE;dos.e_lfanew=128;
    IMAGE_NT_HEADERS64 nt{};nt.Signature=IMAGE_NT_SIGNATURE;nt.FileHeader.Machine=IMAGE_FILE_MACHINE_AMD64;
    nt.FileHeader.NumberOfSections=1;nt.FileHeader.SizeOfOptionalHeader=sizeof(IMAGE_OPTIONAL_HEADER64);
    nt.OptionalHeader.Magic=IMAGE_NT_OPTIONAL_HDR64_MAGIC;nt.OptionalHeader.SizeOfImage=image.size();
    IMAGE_SECTION_HEADER section{};section.VirtualAddress=1024;section.Misc.VirtualSize=2048;
    section.Characteristics=IMAGE_SCN_MEM_READ|IMAGE_SCN_MEM_WRITE;
    const auto table=128+sizeof(DWORD)+sizeof(IMAGE_FILE_HEADER)+sizeof(IMAGE_OPTIONAL_HEADER64);
    auto headers=[&]{std::memcpy(image.data(),&dos,sizeof dos);std::memcpy(image.data()+128,&nt,sizeof nt);
                    std::memcpy(image.data()+table,&section,sizeof section);};headers();
    wire::Snapshot value{};std::memcpy(image.data()+1024,&value,sizeof value);
    unsigned reads=0;std::uintptr_t found=0;
    auto reader=[&](std::uintptr_t at,void* target,std::size_t count){
        ++reads;if(at<base||at-base>image.size()||count>image.size()-(at-base))return false;
        return copy_memory(at,target,count);
    };
    require(diagnostic::find_record(base,reader,found)&&found==base+1024,"unique writable private image record is found");
    std::memcpy(image.data()+1200,&value,sizeof value);
    require(!diagnostic::find_record(base,reader,found),"ambiguous image records are rejected");
    std::memset(image.data()+1200,0,sizeof value);
    section.Characteristics|=IMAGE_SCN_MEM_EXECUTE;headers();
    require(!diagnostic::find_record(base,reader,found),"executable image section cannot supply a diagnostic record");
    section.Characteristics=IMAGE_SCN_MEM_READ;headers();
    require(!diagnostic::find_record(base,reader,found),"read-only image section cannot supply a diagnostic record");
    section.Characteristics|=IMAGE_SCN_MEM_WRITE;section.Misc.VirtualSize=4096;headers();
    require(!diagnostic::find_record(base,reader,found),"out-of-image section is rejected before copying");
    section.Misc.VirtualSize=2048;nt.FileHeader.SizeOfOptionalHeader=1;headers();
    require(!diagnostic::find_record(base,reader,found),"truncated optional header is rejected");
    nt.FileHeader.SizeOfOptionalHeader=sizeof(IMAGE_OPTIONAL_HEADER64);nt.OptionalHeader.SizeOfImage=16*1024*1024;
    section.Misc.VirtualSize=9*1024*1024;headers();reads=0;
    require(!diagnostic::find_record(base,reader,found)&&reads==3,"oversized section is rejected before any section payload read");
    dos.e_magic=0;headers();reads=0;
    require(!diagnostic::find_record(base,reader,found)&&reads==1,"invalid DOS header terminates image search immediately");
}
#endif

}
int main()try{resume_retry();close_retry();other_failures();volatile_register_guard();scan_behavior();stack_region_refusals();oversized_stack_region();tls_data_marker();
#ifndef NDEBUG
diagnostic_publication();diagnostic_reader_bounds();diagnostic_image_bounds();
#endif
std::cout<<"PASS: "<<checks<<" actual unload guard failure/retry and stack checks\n";}
catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
