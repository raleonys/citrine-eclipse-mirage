@echo off
rem Run from an existing x64 Native Tools Command Prompt for Visual Studio.
rem Compile and link ONLY. This script does not load or execute the DLL.
setlocal
cd /d "%~dp0"
if not exist build mkdir build
cl /nologo /c /O2 /GS- /GR- /EHs-c- /Zl /std:c++17 /Fobuild\loader.obj loader.cpp
if errorlevel 1 exit /b %errorlevel%
ml64 /nologo /c /Fo build\wrappers.obj wrappers.asm
if errorlevel 1 exit /b %errorlevel%
rc /nologo /fo build\version.res version.rc
if errorlevel 1 exit /b %errorlevel%
link /nologo /DLL /MACHINE:X64 /ENTRY:ReconstructedDllMain /NODEFAULTLIB /INCREMENTAL:NO /DEF:version.def /OUT:build\version.dll /IMPLIB:build\version.lib /MAP:build\version.map build\loader.obj build\wrappers.obj build\version.res kernel32.lib
exit /b %errorlevel%
