#pragma once
#include <stddef.h>
#include <stdint.h>
#define WINAPI
#define FALSE 0
#define TRUE 1
#define S_OK ((HRESULT)0)
#define E_FAIL ((HRESULT)0x80004005u)
#define CP_UTF8 65001
#define CP_ACP 0
#define MB_ERR_INVALID_CHARS 8
#define WC_NO_BEST_FIT_CHARS 0x400
#define ERROR_PROC_NOT_FOUND 127
#define ERROR_ACCESS_DENIED 5
using BOOL=int;
using DWORD=unsigned long;
using HRESULT=long;
using HANDLE=void*;
using HWND=void*;
using HMODULE=void*;
using FARPROC=void(*)();
using LPVOID=void*;
using LPCSTR=const char*;
using LPSTR=char*;
using LPCWSTR=const wchar_t*;
using LPWSTR=wchar_t*;
struct SECURITY_ATTRIBUTES;
using LPSECURITY_ATTRIBUTES=SECURITY_ATTRIBUTES*;
struct STARTUPINFOA;
struct STARTUPINFOW;
struct PROCESS_INFORMATION;
using LPSTARTUPINFOA=STARTUPINFOA*;
using LPSTARTUPINFOW=STARTUPINFOW*;
using LPPROCESS_INFORMATION=PROCESS_INFORMATION*;
extern "C" {
void SetLastError(DWORD);
HMODULE GetModuleHandleW(LPCWSTR);
HMODULE LoadLibraryW(LPCWSTR);
BOOL FreeLibrary(HMODULE);
FARPROC GetProcAddress(HMODULE,LPCSTR);
int MultiByteToWideChar(unsigned int,DWORD,LPCSTR,int,LPWSTR,int);
int WideCharToMultiByte(unsigned int,DWORD,LPCWSTR,int,LPSTR,int,LPCSTR,BOOL*);
}
