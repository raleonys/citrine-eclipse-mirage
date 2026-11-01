#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, tempfile, sys
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('g++') or shutil.which('clang++')
if not cxx:
    print('AUTH8_RUNTIME: FAIL - no C++ compiler'); sys.exit(1)

def build_and_run(source: str, name: str, extra_flags=None):
    extra_flags=extra_flags or []
    with tempfile.TemporaryDirectory(prefix=f'auth8_{name}_') as td:
        td=Path(td); src=td/'test.cpp'; exe=td/'test'; src.write_text(source)
        cmd=[cxx,'-std=c++17','-Wall','-Wextra','-Werror',*extra_flags,
             '-I',str(r/'include'),str(src),str(r/'src/runtime/auth_code_service.cpp'),'-o',str(exe)]
        subprocess.run(cmd,cwd=r,check=True)
        subprocess.run([str(exe)],cwd=r,check=True)

transactional=r'''#include "preloader_l/runtime/auth_code_service.hpp"
#include <cstring>
static bool valid(const preloader_l::runtime::AuthCodeValueView& v){
 if(!v.data||v.size!=8u)return false;
 for(size_t i=0;i<8u;++i){const char c=v.data[i];if(!((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z')))return false;}
 return v.data[8]=='\0';
}
int main(){
 using namespace preloader_l::runtime;
 clear_auth_code_service();
 if(session_auth_code_ready())return 1;
 AuthCodeSessionLease lease{};
 if(!begin_session_auth_code_lease(&lease))return 2;
 const auto staged=session_auth_code_lease_value(&lease); if(!valid(staged))return 3;
 finish_session_auth_code_lease(&lease,false);
 if(session_auth_code_ready())return 4;
 if(!begin_session_auth_code_lease(&lease))return 5;
 const auto committed=session_auth_code_lease_value(&lease); if(!valid(committed))return 6;
 finish_session_auth_code_lease(&lease,true);
 if(!session_auth_code_ready())return 7;
 if(!begin_session_auth_code_lease(&lease))return 8;
 const auto a=session_auth_code_lease_value(&lease); if(!valid(a))return 9;
 char first[9]{}; std::memcpy(first,a.data,8u); first[8]='\0';
 finish_session_auth_code_lease(&lease,false);
 if(!begin_session_auth_code_lease(&lease))return 10;
 const auto b=session_auth_code_lease_value(&lease); if(!valid(b)||std::memcmp(first,b.data,8u)!=0)return 11;
 finish_session_auth_code_lease(&lease,false);
 clear_auth_code_service(); if(session_auth_code_ready())return 12;
 if(!begin_session_auth_code_lease(&lease))return 13;
 const auto c=session_auth_code_lease_value(&lease); if(!valid(c))return 14;
 finish_session_auth_code_lease(&lease,false);
 return 0;
}
'''

win_entropy=r'''#include "preloader_l/runtime/auth_code_service.hpp"
#include <cstdint>
#include <cstring>
using namespace preloader_l::runtime;
static std::uint32_t g_ft_low=0x11111111u, g_ft_high=0x22222222u;
static std::int64_t g_qpc=0x3333333344444444ll;
static unsigned long g_pid=0x5555ul, g_tid=0x6666ul;
extern "C" void GetSystemTimeAsFileTime(void* p){
 struct FT{std::uint32_t low;std::uint32_t high;}; auto* ft=static_cast<FT*>(p); ft->low=g_ft_low; ft->high=g_ft_high;
}
extern "C" int QueryPerformanceCounter(std::int64_t* p){*p=g_qpc;return 1;}
extern "C" unsigned long GetCurrentProcessId(){return g_pid;}
extern "C" unsigned long GetCurrentThreadId(){return g_tid;}
extern "C" long _InterlockedCompareExchange(volatile long* p,long exch,long comp){const long old=*p;if(old==comp)*p=exch;return old;}
static bool valid(AuthCodeValueView v){
 if(!v.data||v.size!=8u)return false;
 for(size_t i=0;i<8u;++i){const char c=v.data[i];if(!((c>='0'&&c<='9')||(c>='A'&&c<='Z')||(c>='a'&&c<='z')))return false;}
 return v.data[8]=='\0';
}
static void copy8(char out[9],AuthCodeValueView v){std::memcpy(out,v.data,8u);out[8]='\0';}
int main(){
 char a[9]{},b[9]{}; AuthCodeSessionLease lease{};
 clear_auth_code_service();
 if(!begin_session_auth_code_lease(&lease))return 1;
 const auto va=session_auth_code_lease_value(&lease); if(!valid(va))return 2; copy8(a,va);
 finish_session_auth_code_lease(&lease,true);
 clear_auth_code_service();
 g_ft_low^=0x01020304u; g_ft_high^=0x10203040u; g_qpc^=0x0102030405060708ll; g_pid+=17ul; g_tid+=29ul;
 if(!begin_session_auth_code_lease(&lease))return 3;
 const auto vb=session_auth_code_lease_value(&lease); if(!valid(vb))return 4; copy8(b,vb);
 finish_session_auth_code_lease(&lease,true);
 if(std::memcmp(a,b,8u)==0)return 5;
 clear_auth_code_service(); return 0;
}
'''

build_and_run(transactional,'transactional')
build_and_run(win_entropy,'win_entropy',['-D_WIN32','-D__declspec(x)=','-D__stdcall='])
print('AUTH8_RUNTIME: PASS (lease-only transactional rollback/commit + session stability; deterministic _WIN32 entropy branch exercised with controlled system seeds; no legacy global-view API)')
