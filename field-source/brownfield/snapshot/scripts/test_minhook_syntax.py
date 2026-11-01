#!/usr/bin/env python3
from pathlib import Path
import shutil, subprocess, tempfile, sys, textwrap
r=Path(__file__).resolve().parents[1]
cxx=shutil.which('clang++') or shutil.which('g++')
if not cxx:
    print('MINHOOK_WINDOWS_SYNTAX_SURFACE: FAIL - no C++ compiler'); sys.exit(1)
files=[
 'src/platform/minhook_runtime.cpp',
 'src/platform/registry_hooks.cpp',
 'src/platform/registry_wrappers.cpp',
 'src/platform/registry_hook_runtime.cpp',
 'src/platform/crypt_observer.cpp',
 'src/platform/connect_redirect_hook.cpp',
 'src/runtime/bootstrap_thread.cpp',
 'tests/native/native_acceptance_exports.cpp',
]
with tempfile.TemporaryDirectory(prefix='preloader_l_minhook_syntax_') as td_s:
    td=Path(td_s)
    (td/'windows.h').write_text(textwrap.dedent('''\
        #pragma once
        #include <stddef.h>
        #include <stdint.h>
        #define WINAPI
        #define VOID void
        typedef void* LPVOID;
        typedef const wchar_t* LPCWSTR;
        typedef const char* LPCSTR;
        static inline int SwitchToThread(void){return 1;}
        unsigned long long __stdcall GetTickCount64(void);
    '''))
    (td/'MinHook.h').write_text(textwrap.dedent('''\
        #pragma once
        #include "windows.h"
        typedef enum MH_STATUS { MH_UNKNOWN=-1, MH_OK=0, MH_ERROR_ALREADY_INITIALIZED,
          MH_ERROR_NOT_INITIALIZED, MH_ERROR_ALREADY_CREATED, MH_ERROR_NOT_CREATED,
          MH_ERROR_ENABLED, MH_ERROR_DISABLED, MH_ERROR_NOT_EXECUTABLE,
          MH_ERROR_UNSUPPORTED_FUNCTION, MH_ERROR_MEMORY_ALLOC, MH_ERROR_MEMORY_PROTECT,
          MH_ERROR_MODULE_NOT_FOUND, MH_ERROR_FUNCTION_NOT_FOUND } MH_STATUS;
        #ifdef __cplusplus
        extern "C" {
        #endif
        #define MH_ALL_HOOKS ((void*)0)
MH_STATUS MH_Initialize(void); MH_STATUS MH_Uninitialize(void);
        MH_STATUS MH_CreateHook(void*,void*,void**); MH_STATUS MH_RemoveHook(void*);
        MH_STATUS MH_QueueEnableHook(void*); MH_STATUS MH_QueueDisableHook(void*);
        MH_STATUS MH_ApplyQueued(void);
        #ifdef __cplusplus
        }
        #endif
    '''))
    base=[cxx,'-std=c++20','-fsyntax-only','-fms-extensions','-Wno-ignored-attributes',
          '-D_WIN32','-D_WIN64','-D__stdcall=','-D__cdecl=','-D__declspec(x)=',
          '-I',str(td),'-I',str(r/'include'),'-I',str(r/'src'),'-I',str(r/'third_party')]
    for rel in files:
        p=r/rel
        if not p.is_file():
            print('MINHOOK_WINDOWS_SYNTAX_SURFACE: FAIL - missing '+rel); sys.exit(1)
        proc=subprocess.run(base+[str(p)],cwd=r,text=True,stdout=subprocess.PIPE,stderr=subprocess.PIPE)
        if proc.returncode:
            print('MINHOOK_WINDOWS_SYNTAX_SURFACE: FAIL - '+rel)
            if proc.stdout: print(proc.stdout)
            if proc.stderr: print(proc.stderr)
            sys.exit(1)
print(f'MINHOOK_WINDOWS_SYNTAX_SURFACE: PASS ({len(files)}/{len(files)})')
