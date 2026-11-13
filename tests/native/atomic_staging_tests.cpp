// SPDX-License-Identifier: GPL-3.0-only

#include <windows.h>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <new>
#include <set>
#include <stdexcept>

namespace fault {
bool write=false,close=false,remove=false,copy=false;
bool after_copy=false,query_failure=false,attributes_failure=false;
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
BOOL WINAPI copying(LPCWSTR a,LPCWSTR b,BOOL c){
    if(copy){SetLastError(ERROR_WRITE_FAULT);return FALSE;}
    const auto result=::CopyFileW(a,b,c);
    // A controlled exception after acquisition exercises the same pending-owner
    // cleanup as a throwing Path copy in release(), without Debug CRT injection.
    if(result&&after_copy){after_copy=false;throw std::bad_alloc();}
    return result;
}
DWORD WINAPI querying(LPCWSTR path){if(query_failure){SetLastError(ERROR_BUSY);return INVALID_FILE_ATTRIBUTES;}return ::GetFileAttributesW(path);}
BOOL WINAPI setting(LPCWSTR path,DWORD attributes){if(attributes_failure){SetLastError(ERROR_ACCESS_DENIED);return FALSE;}return ::SetFileAttributesW(path,attributes);}
}
// Exercise the real native backend with controlled failures after real handle
// acquisition. No original binary, external process or product API is patched.
#define CreateFileW fault::create
#define WriteFile fault::writing
#define CloseHandle fault::closing
#define DeleteFileW fault::removing
#define CopyFileW fault::copying
#define GetFileAttributesW fault::querying
#define SetFileAttributesW fault::setting
#include "../../src/platform/atomic_files_windows.cpp"
#undef CreateFileW
#undef WriteFile
#undef CloseHandle
#undef DeleteFileW
#undef CopyFileW
#undef GetFileAttributesW
#undef SetFileAttributesW

using namespace citrine_eclipse_mirage;
namespace fs=std::filesystem;
unsigned checks=0;
void require(bool b,const char* text){++checks;if(!b)throw std::runtime_error(text);}
template<class F>void rejects(F f){bool failed=false;try{f();}catch(const std::exception&){failed=true;}require(failed,"injected operation failed visibly");}
void duplicate_native_destinations(const fs::path& root) {
    const auto directory=root/"batch-alias";
    fs::create_directory(directory);
    struct Cleanup {fs::path path;~Cleanup(){std::error_code e;fs::remove_all(path,e);}} cleanup{directory};
    const auto target=directory/"Profile.dat",case_alias=directory/"PROFILE.DAT";
    {std::ofstream output(target,std::ios::binary);output<<"original";}
    file::NativeFactory files;
    const std::array<file::FileWrite,2> duplicate{{{target.u16string(),"first"},
                                                   {case_alias.u16string(),"second"}}};
    bool rejected=false;
    try{files.replace_atomic(duplicate);}
    catch(const std::invalid_argument&){rejected=true;}
    auto unchanged=[&]{std::ifstream input(target,std::ios::binary);return std::string(
        std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>{})=="original";};
    auto members=[&]{return std::distance(fs::directory_iterator(directory),fs::directory_iterator{});};
    require(rejected&&unchanged()&&members()==1&&fault::handles.empty(),
        "case-fold-equivalent Windows batch destinations rejected before staging or replacement");
    auto nul_path=target.u16string();nul_path.push_back(u'\0');nul_path.append(u"hidden");
    const file::FileWrite nul{nul_path,"unwanted"};
    rejected=false;
    try{files.replace_atomic({&nul,1});}
    catch(const std::invalid_argument&){rejected=true;}
    require(rejected&&unchanged()&&members()==1&&fault::handles.empty(),
        "embedded NUL cannot alias an existing Windows destination or create staging");
    const file::FileWrite single{target.u16string(),"accepted"};
    files.replace_atomic({&single,1});
    {std::ifstream input(target,std::ios::binary);
     require(std::string(std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>{})=="accepted",
         "one ordinary destination still replaces successfully after invalid batch rejection");}
    require(members()==1&&fault::handles.empty(),
        "valid retry leaves no staging or backup artifacts");
}
void readonly_backup_recovery(const fs::path& root){
    for(int failure=0;failure<4;++failure){
        const auto directory=root/("readonly-"+std::to_string(failure));fs::create_directory(directory);
        struct Directory {fs::path path;~Directory(){std::error_code e;fs::remove_all(path,e);}} directory_owner{directory};
        const auto target=directory/"owned-original.xml";
        {std::ofstream output(target);output<<"original";}
        require(SetFileAttributesW(target.wstring().c_str(),FILE_ATTRIBUTE_READONLY)!=FALSE,"own a readonly original");
        file::NativeFactory files;
        struct Reset {~Reset(){fault::after_copy=fault::query_failure=fault::attributes_failure=fault::remove=false;}} reset;
        const file::FileWrite value{target.u16string(),"replacement"};
        auto members=[&]{return std::distance(fs::directory_iterator(directory),fs::directory_iterator{});};
        auto original=[&]{std::ifstream input(target);std::string bytes;input>>bytes;return bytes=="original";};
        fault::after_copy=true;fault::query_failure=failure==1;fault::attributes_failure=failure==2;fault::remove=failure==3;
        bool allocation=false;int cleanup_error=0;
        try{files.replace_atomic({&value,1});}
        catch(const std::bad_alloc&){allocation=true;}
        catch(const std::system_error& error){cleanup_error=error.code().value();}
        require(!fault::after_copy&&fault::handles.empty(),"real backup copied before controlled exception; native handles closed");
        require(original()&&(GetFileAttributesW(target.wstring().c_str())&FILE_ATTRIBUTE_READONLY),"cleanup preserves original bytes and readonly metadata");
        if(failure==0)require(allocation&&cleanup_error==0&&members()==1,"readonly backup cleanup preserves original allocation exception and leaves no artifact");
        else{
            require(!allocation&&cleanup_error==static_cast<int>(failure==1?ERROR_BUSY:ERROR_ACCESS_DENIED)&&members()==3,"failed query, attribute update or deletion retains owned backup and earlier staging");
            fault::query_failure=fault::attributes_failure=fault::remove=false;
        }
        files.recover_atomic();files.recover_atomic();
        require(members()==1&&original()&&(GetFileAttributesW(target.wstring().c_str())&FILE_ATTRIBUTE_READONLY),"same owner recovers retained files without changing original metadata");
        require(SetFileAttributesW(target.wstring().c_str(),FILE_ATTRIBUTE_NORMAL)!=FALSE,"repair only the owned test destination for replacement retry");
        files.replace_atomic({&value,1});
        require(!original()&&members()==1&&fault::handles.empty(),"same factory can replace again after readonly backup recovery");
    }
}
int main()try{
    const auto root=fs::temp_directory_path()/("surge-staging-"+std::to_string(GetCurrentProcessId()));
    fs::create_directory(root);
    struct Directory {fs::path p;~Directory(){std::error_code e;fs::remove_all(p,e);}} dir{root};
    const auto target=root/"original.bin";{std::ofstream out(target);out<<"old";}
    const file::FileWrite value{target.u16string(),"new"};
    file::NativeFactory files;
    struct Restore {~Restore(){fault::write=fault::close=fault::remove=fault::copy=fault::after_copy=fault::query_failure=fault::attributes_failure=false;}} restore;
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
    readonly_backup_recovery(root);
    duplicate_native_destinations(root);
    std::cout<<"PASS: "<<checks<<" real native staging/backup handle, path aliases and recovery checks\n";
}catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
