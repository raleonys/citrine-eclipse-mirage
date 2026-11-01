cmake_minimum_required(VERSION 3.24)
cmake_policy(SET CMP0091 NEW)
project(version_outer_reconstruction LANGUAGES CXX ASM_MASM RC)

if(NOT MSVC OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Requires MSVC and MASM targeting Windows x64.")
endif()
if(CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_CONFIGURATION_TYPES Release CACHE STRING "Supported configuration" FORCE)
elseif(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

add_library(version SHARED loader.cpp wrappers.asm version.rc version.def)
set_target_properties(version PROPERTIES OUTPUT_NAME version PREFIX ""
    CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES MSVC_RUNTIME_LIBRARY "")
target_compile_options(version PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:/O2;/GS-;/GR-;/EHs-c-;/Zl>")
target_link_options(version PRIVATE /ENTRY:ReconstructedDllMain /NODEFAULTLIB
    /MACHINE:X64 /INCREMENTAL:NO)
target_link_libraries(version PRIVATE kernel32)
