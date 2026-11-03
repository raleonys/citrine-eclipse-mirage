# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.20)
project(preloader_l LANGUAGES C CXX)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(PRELOADER_L_PORTABLE_CHECKS_ONLY "Build portable source checks only" OFF)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
include(cmake/VendoredDependencies.cmake)
set(PRELOADER_L_WINE_EXECUTABLE "" CACHE FILEPATH "Optional Wine runner for Windows cross-build tests")
if(CMAKE_CROSSCOMPILING AND PRELOADER_L_WINE_EXECUTABLE)
  set(CMAKE_CROSSCOMPILING_EMULATOR
    "${Python3_EXECUTABLE};${PROJECT_SOURCE_DIR}/scripts/run_wine_test.py;--timeout;180;${PRELOADER_L_WINE_EXECUTABLE}"
    CACHE STRING "Run the Windows test process without Wine service pipe inheritance" FORCE)
endif()
if(PRELOADER_L_PORTABLE_CHECKS_ONLY)
  enable_testing()
  include(cmake/PortableChecks.cmake)
  include(cmake/InfrastructureChecks.cmake)
  include(cmake/VendorChecks.cmake)
  return()
endif()
if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "preloader_l requires Windows x64 or an explicit Windows x64 cross toolchain.")
endif()
enable_language(RC)
set_property(SOURCE resources/surge.rc PROPERTY OBJECT_DEPENDS
  "${PROJECT_SOURCE_DIR}/resources/preloader_version.rc")
include(CTest)
if(MSVC)
  # Compiler-owned standard CRT; consistent /MT Release and /MTd Debug targets.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
option(PRELOADER_L_GOLDEN_LINK_PROFILE "Emit the bounded MSVC Golden link profile" OFF)
if(PRELOADER_L_GOLDEN_LINK_PROFILE AND NOT MSVC)
  message(FATAL_ERROR "Golden shipping link evidence requires native MSVC on Windows.")
endif()

option(PRELOADER_L_BUILD_SOURCE_API_DLL "Build the separate C++ source API development DLL" ${BUILD_TESTING})
include(cmake/Sources.cmake)
include(cmake/BuildPartition.cmake)
if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_source_files_properties(src/platform/hook_unload_guard.cpp PROPERTIES COMPILE_OPTIONS "-fms-extensions")
endif()
set(_avatar_header "${CMAKE_CURRENT_BINARY_DIR}/generated/avatar_data.hpp")
add_custom_command(OUTPUT "${_avatar_header}"
  COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/generate_avatar_data.py"
    --root "${PROJECT_SOURCE_DIR}" --output "${_avatar_header}"
  DEPENDS scripts/generate_avatar_data.py metadata/AVATAR_DATA_CONTRACT.json
    resources/branding/surge.png resources/branding/default-avatar.png
  VERBATIM)
add_library(preloader_l_vdf_objects OBJECT ${PRELOADER_L_VDF_SOURCES})
target_include_directories(preloader_l_vdf_objects PRIVATE include)
target_link_libraries(preloader_l_vdf_objects PRIVATE preloader_l_pegtl)
target_compile_definitions(preloader_l_vdf_objects PRIVATE NOMINMAX)
set_target_properties(preloader_l_vdf_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(preloader_l_objects OBJECT ${PRELOADER_L_APPLICATION_SOURCES}
  src/platform/crypt_query_host.cpp "${_avatar_header}")
target_include_directories(preloader_l_objects PUBLIC include)
target_include_directories(preloader_l_objects PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(preloader_l_objects PUBLIC preloader_l_minhook preloader_l_pugixml
  preloader_l_asio)
target_compile_definitions(preloader_l_objects PUBLIC NOMINMAX)
set_target_properties(preloader_l_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(preloader_l_core STATIC $<TARGET_OBJECTS:preloader_l_objects>
  $<TARGET_OBJECTS:preloader_l_vdf_objects>)
target_include_directories(preloader_l_core PUBLIC include)
target_compile_definitions(preloader_l_core PUBLIC NOMINMAX)
target_link_libraries(preloader_l_core PUBLIC preloader_l_minhook preloader_l_pugixml
  preloader_l_asio
  crypt32 ws2_32 kernel32 user32 shell32)

# The application DLL has an explicit C export table. Source-level C++ APIs
# belong to a separate development DLL and are never exported by this target.
add_library(preloader_l SHARED $<TARGET_OBJECTS:preloader_l_objects>
  $<TARGET_OBJECTS:preloader_l_vdf_objects> src/platform/dll_entry.cpp
  resources/surge.rc exports/preloader_l.def)
target_include_directories(preloader_l PRIVATE include)
target_link_libraries(preloader_l PRIVATE preloader_l_core)
target_include_directories(preloader_l PRIVATE resources)
set_target_properties(preloader_l PROPERTIES
  OUTPUT_NAME "preloader.unsigned" PREFIX "" WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(MSVC)
  # All application profiles have the same exact VERSIONINFO-only resource contract.
  # The default MSVC DLL manifest would add an unselected RT_MANIFEST.
  # Executable manifests are unaffected.
  target_link_options(preloader_l PRIVATE /MANIFEST:NO)
  target_compile_options(preloader_l_objects PRIVATE /GS)
  target_compile_options(preloader_l_vdf_objects PRIVATE /GS)
  if(PRELOADER_L_GOLDEN_LINK_PROFILE)
    target_link_options(preloader_l PRIVATE /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO)
  endif()
endif()
if(MINGW)
  target_link_options(preloader_l PRIVATE -Wl,--exclude-all-symbols)
endif()

# Reference payloads exist only for tests or the explicitly selected source DLL.
if(BUILD_TESTING OR PRELOADER_L_BUILD_SOURCE_API_DLL)
  add_library(preloader_l_reference_objects OBJECT ${PRELOADER_L_REFERENCE_SOURCES})
  target_include_directories(preloader_l_reference_objects PUBLIC include)
  target_link_libraries(preloader_l_reference_objects PRIVATE preloader_l_minhook preloader_l_pugixml preloader_l_asio)
  target_compile_definitions(preloader_l_reference_objects PRIVATE NOMINMAX)
  set_target_properties(preloader_l_reference_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(MSVC)
    target_compile_options(preloader_l_reference_objects PRIVATE /GS)
  endif()
  add_library(preloader_l_reference_models STATIC $<TARGET_OBJECTS:preloader_l_reference_objects>)
  target_link_libraries(preloader_l_reference_models PUBLIC preloader_l_core)
endif()
if(PRELOADER_L_BUILD_SOURCE_API_DLL)
  if(MSVC)
    # C++ objects cross the development DLL boundary, including Debug STL
    # container proxies. Both modules must use the same shared compiler CRT.
    # Never reuse the shipping /MT or /MTd objects in this /MD or /MDd graph.
    set(_source_api_crt "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    get_target_property(_source_hook_sources preloader_l_minhook SOURCES)
    get_target_property(_source_hook_includes preloader_l_minhook INTERFACE_INCLUDE_DIRECTORIES)
    add_library(preloader_l_source_minhook STATIC ${_source_hook_sources})
    target_include_directories(preloader_l_source_minhook PUBLIC ${_source_hook_includes})
    add_dependencies(preloader_l_source_minhook preloader_l_verify_vendors preloader_l_verify_hook_engine)
    add_library(preloader_l_source_pugixml STATIC third_party/pugixml/src/pugixml.cpp)
    target_include_directories(preloader_l_source_pugixml PUBLIC third_party/pugixml/src)
    target_compile_definitions(preloader_l_source_pugixml PUBLIC PUGIXML_NO_XPATH)
    add_dependencies(preloader_l_source_pugixml preloader_l_verify_vendors)
    set_target_properties(preloader_l_source_minhook preloader_l_source_pugixml
      PROPERTIES MSVC_RUNTIME_LIBRARY "${_source_api_crt}")
    add_library(preloader_l_source_api SHARED ${PRELOADER_L_APPLICATION_SOURCES}
      src/platform/crypt_query_host.cpp "${_avatar_header}"
      ${PRELOADER_L_VDF_SOURCES} ${PRELOADER_L_REFERENCE_SOURCES}
      src/platform/dll_entry.cpp resources/surge.rc)
    target_include_directories(preloader_l_source_api PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
    target_compile_definitions(preloader_l_source_api PRIVATE NOMINMAX)
    target_compile_options(preloader_l_source_api PRIVATE /GS)
    target_link_libraries(preloader_l_source_api PRIVATE preloader_l_source_minhook
      preloader_l_source_pugixml preloader_l_asio preloader_l_pegtl
      crypt32 ws2_32 kernel32 user32 shell32)
    set_target_properties(preloader_l_source_api PROPERTIES MSVC_RUNTIME_LIBRARY "${_source_api_crt}")
  else()
    add_library(preloader_l_source_api SHARED $<TARGET_OBJECTS:preloader_l_objects>
      $<TARGET_OBJECTS:preloader_l_vdf_objects> $<TARGET_OBJECTS:preloader_l_reference_objects>
      src/platform/dll_entry.cpp resources/surge.rc)
    target_link_libraries(preloader_l_source_api PRIVATE preloader_l_core)
  endif()
  target_include_directories(preloader_l_source_api PRIVATE include resources)
  set_target_properties(preloader_l_source_api PROPERTIES
    OUTPUT_NAME "preloader.source" PREFIX "" WINDOWS_EXPORT_ALL_SYMBOLS ON)
  if(MSVC)
    target_link_options(preloader_l_source_api PRIVATE /MANIFEST:NO)
  endif()
  if(MINGW)
    target_link_options(preloader_l_source_api PRIVATE -Wl,--export-all-symbols)
  endif()
  add_executable(preloader_l_source_host tools/preloader_l_source_host/main.cpp)
  target_include_directories(preloader_l_source_host PRIVATE include)
  target_link_libraries(preloader_l_source_host PRIVATE preloader_l_source_api preloader_l_asio)
  if(MSVC)
    set_target_properties(preloader_l_source_host PROPERTIES MSVC_RUNTIME_LIBRARY "${_source_api_crt}")
  endif()
endif()

add_executable(preloader_l_runtime_host tools/preloader_l_runtime_host/main.cpp)
target_include_directories(preloader_l_runtime_host PRIVATE include)
target_link_libraries(preloader_l_runtime_host PRIVATE preloader_l_asio)
if(MINGW)
  target_link_options(preloader_l_runtime_host PRIVATE -municode)
endif()
add_dependencies(preloader_l_runtime_host preloader_l)

if(BUILD_TESTING)
  add_executable(application_runtime_contracts tests/native/application_runtime_tests.cpp resources/surge.rc)
  target_include_directories(application_runtime_contracts PRIVATE resources)
  target_link_libraries(application_runtime_contracts PRIVATE preloader_l_core advapi32)
  add_library(online_provider_fixture SHARED tests/support/runtime/online_fixture.cpp)
  set_target_properties(online_provider_fixture PROPERTIES OUTPUT_NAME "preloader_l_online" PREFIX "")
  set_target_properties(online_provider_fixture PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixtures"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixtures")
  add_dependencies(application_runtime_contracts online_provider_fixture)
  add_test(NAME application_runtime_contracts COMMAND application_runtime_contracts
    $<TARGET_FILE:online_provider_fixture> "${PROJECT_SOURCE_DIR}/resources/branding/surge.png"
    "${PROJECT_SOURCE_DIR}/resources/branding/default-avatar.png" -showLSX)
  set_tests_properties(application_runtime_contracts PROPERTIES TIMEOUT 60)
  add_executable(configuration_platform_contracts tests/native/configuration_platform_tests.cpp)
  target_link_libraries(configuration_platform_contracts PRIVATE preloader_l_core advapi32)
  add_test(NAME configuration_platform_contracts COMMAND configuration_platform_contracts)
  set_tests_properties(configuration_platform_contracts PROPERTIES TIMEOUT 60)
  add_executable(environment_dialog_contracts tests/native/environment_dialog_tests.cpp)
  target_include_directories(environment_dialog_contracts PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
  target_link_libraries(environment_dialog_contracts PRIVATE preloader_l_core advapi32)
  add_dependencies(environment_dialog_contracts preloader_l_objects)
  add_test(NAME environment_dialog_contracts COMMAND environment_dialog_contracts)
  set_tests_properties(environment_dialog_contracts PROPERTIES TIMEOUT 60)
  add_executable(startup_publication_contracts tests/native/startup_publication_tests.cpp)
  target_link_libraries(startup_publication_contracts PRIVATE preloader_l_core advapi32)
  add_test(NAME startup_publication_contracts COMMAND startup_publication_contracts)
  set_tests_properties(startup_publication_contracts PROPERTIES TIMEOUT 60)
  include(cmake/InfrastructureChecks.cmake)
  include(cmake/VendorChecks.cmake)
  add_executable(preloader_l_crt_host tools/preloader_l_crt_host/main.cpp)
  add_test(NAME crt_runtime_smoke COMMAND preloader_l_crt_host)
  set_tests_properties(crt_runtime_smoke PROPERTIES TIMEOUT 60)
  include(cmake/Tests.cmake)
  add_executable(vdf_parser_regressions tests/unit/parser/vdf_regression_tests.cpp)
  target_link_libraries(vdf_parser_regressions PRIVATE preloader_l_core)
  add_test(NAME vdf_parser_regressions COMMAND vdf_parser_regressions)
  set_tests_properties(vdf_parser_regressions PROPERTIES TIMEOUT 60)
  if(PRELOADER_L_BUILD_SOURCE_API_DLL)
    add_test(NAME dll_source_smoke COMMAND preloader_l_source_host)
  endif()
  add_test(NAME dll_public_lifecycle COMMAND preloader_l_runtime_host $<TARGET_FILE:preloader_l>)
  set_tests_properties(dll_public_lifecycle PROPERTIES TIMEOUT 60)
  add_test(NAME dll_public_xml_contracts COMMAND preloader_l_runtime_host $<TARGET_FILE:preloader_l> --xml-contracts)
  set_tests_properties(dll_public_xml_contracts PROPERTIES TIMEOUT 120)
  add_executable(dll_public_c_consumer tests/native/public_c_consumer.c)
  target_include_directories(dll_public_c_consumer PRIVATE include)
  target_link_libraries(dll_public_c_consumer PRIVATE preloader_l)
  add_test(NAME dll_public_c_consumer COMMAND dll_public_c_consumer)
  set_tests_properties(dll_public_c_consumer PROPERTIES TIMEOUT 60)
  add_test(NAME dll_public_export_contract COMMAND "${Python3_EXECUTABLE}" -B
    "${PROJECT_SOURCE_DIR}/scripts/check_dll_exports.py" $<TARGET_FILE:preloader_l>)
  add_test(NAME dll_public_export_negatives COMMAND "${Python3_EXECUTABLE}" -B
    "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_public_abi.py" $<TARGET_FILE:preloader_l>)
  set_tests_properties(dll_public_export_contract dll_public_export_negatives PROPERTIES TIMEOUT 30)
  # Keep the upstream behavior fixtures as a comparison baseline. Production
  # uses the verified patched copy; hook_freeze_contracts tests that actual code.
  set(_mh third_party/minhook/src)
  add_library(hook_engine_fixture_objects OBJECT
    ${_mh}/hook.c ${_mh}/buffer.c ${_mh}/trampoline.c ${_mh}/hde/hde64.c)
  if(MSVC)
    target_compile_options(hook_engine_fixture_objects PRIVATE
      "/FI${PROJECT_SOURCE_DIR}/tests/support/hook_engine/fake_win32.h")
  else()
    target_compile_options(hook_engine_fixture_objects PRIVATE -include
      "${PROJECT_SOURCE_DIR}/tests/support/hook_engine/fake_win32.h")
  endif()
  add_executable(hook_engine_fixture_tests tests/native/hook_engine_fixture_tests.cpp
    tests/support/hook_engine/fake_win32.cpp $<TARGET_OBJECTS:hook_engine_fixture_objects>)
  target_include_directories(hook_engine_fixture_tests PRIVATE include "${PROJECT_SOURCE_DIR}")
  target_compile_definitions(hook_engine_fixture_tests PRIVATE NOMINMAX)
  target_link_libraries(hook_engine_fixture_tests PRIVATE preloader_l_minhook)
  add_test(NAME hook_engine_fixtures COMMAND hook_engine_fixture_tests)
endif()

preloader_l_install_partition_gate()
