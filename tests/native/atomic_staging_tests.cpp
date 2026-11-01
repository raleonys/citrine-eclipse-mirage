// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace fault {
bool write=false,close=false,remove=false,copy=false;
std::set<HANDLE> handles;
HANDLE WINAPI create(LPCWSTR p,DWORD a,DWORD s,LPSECURITY_ATTRIBUTES b,DWORD d,DWORD f,HANDLE t){
    const auto h=::CreateFileW(p,a,s,b,d,f,t);if(h!=INVALID_HANDLE_VALUE)handles.insert(h);return h;
}
BOOL WINAPI writing(HANDLE h,LPCVOID p,DWORD n,LPDWORD out,LPOVERLAPPED o){
    if(write){SetLastError(ERROR_WRITE_FAULT);return FALSE;}return ::WriteFile(h,p,n,out,o);
}
BOOL WINAPI closing(HANDLE h){
    if(close){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}
    const auto result=::CloseHandle(h);if(result)handles.erase(h);return result;
}
BOOL WINAPI removing(LPCWSTR p){if(remove){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return ::DeleteFileW(p);}
BOOL WINAPI copying(LPCWSTR a,LPCWSTR b,BOOL c){if(copy){SetLastError(ERROR_WRITE_FAULT);return FALSE;}return ::CopyFileW(a,b,c);}
}
// Exercise the real native backend with controlled failures after real handle
// acquisition. No original binary, external process or product API is patched.
#define CreateFileW fault::create
#define WriteFile fault::writing
#define CloseHandle fault::closing
#define DeleteFileW fault::removing
#define CopyFileW fault::copying
#include "../../src/platform/atomic_files_windows.cpp"
#undef CreateFileW
#undef WriteFile
#undef CloseHandle
#undef DeleteFileW
#undef CopyFileW

using namespace preloader_l;
namespace fs=std::filesystem;
unsigned checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}require(failed,"injected operation failed visibly");}
int main()try{
    const auto root=fs::temp_directory_path()/("surge-staging-"+std::to_string(GetCurrentProcessId()));
    fs::create_directory(root);
    struct Directory {fs::path p;~Directory(){std::error_code e;fs::remove_all(p,e);}} dir{root};
    const auto target=root/"original.bin";{std::ofstream out(target);out<<"old";}
    const file::FileWrite value{target.u16string(),"new"};
    file::NativeFactory files;
    struct Restore {~Restore(){fault::write=fault::close=fault::remove=fault::copy=false;}} restore;
    auto members=[&]{return std::distance(fs::directory_iterator(root),fs::directory_iterator{});};
    auto old=[&]{std::ifstream f(target);std::string s;f>>s;return s=="old";};
    for(bool write_failure:{true,false}) {
        fault::write=write_failure;fault::close=true;
        rejects([&]{files.replace_atomic({&value,1});});
        require(fault::handles.size()==1&&members()==2&&old(),"failed close retains one real handle and preserves destination");
        rejects([&]{files.recover_atomic();});
        require(fault::handles.size()==1,"persistent failure cannot discard the exact handle");
        fault::write=fault::close=false;fault::remove=true;
        rejects([&]{files.recover_atomic();});
        require(fault::handles.empty()&&members()==2,"close succeeds once while failed deletion retains its path");
        fault::remove=false;files.recover_atomic();
        require(fault::handles.empty()&&members()==1&&old(),"retry releases failed staging without altering original");
    }
    fault::copy=fault::remove=true;
    rejects([&]{files.replace_atomic({&value,1});});
    require(fault::handles.empty()&&old(),"backup failure keeps original and closes its reserved handle");
    fault::copy=fault::remove=false;files.recover_atomic();
    require(members()==1,"failed backup and earlier staging identities recovered together");
    files.replace_atomic({&value,1});
    require(!old()&&members()==1&&fault::handles.empty(),"normal update works after native recovery");
    std::cout<<"PASS: "<<checks<<" real native staging/backup handle and path recovery checks\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
