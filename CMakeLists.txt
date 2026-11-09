# SPDX-License-Identifier: GPL-3.0-only

cmake_minimum_required(VERSION 3.20)
project(citrine_eclipse_mirage LANGUAGES C CXX)

find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(_project_version_header "${CMAKE_CURRENT_BINARY_DIR}/generated/project_version.h")
execute_process(COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/project_version.py"
  --header "${_project_version_header}"
  RESULT_VARIABLE _project_version_result OUTPUT_VARIABLE CITRINE_ECLIPSE_MIRAGE_PROJECT_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if(NOT _project_version_result EQUAL 0)
  message(FATAL_ERROR "Invalid project resource version")
endif()
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/metadata/BRANDING_AND_IDENTITY.json"
  "${PROJECT_SOURCE_DIR}/scripts/project_version.py")
# All native fixtures using surge.rc need the same generated version header.
include_directories("${CMAKE_CURRENT_BINARY_DIR}/generated")

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
option(CITRINE_ECLIPSE_MIRAGE_PORTABLE_CHECKS_ONLY "Build portable source checks only" OFF)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
include(cmake/VendoredDependencies.cmake)
set(CITRINE_ECLIPSE_MIRAGE_WINE_EXECUTABLE "" CACHE FILEPATH "Optional Wine runner for Windows cross-build tests")
if(CMAKE_CROSSCOMPILING AND CITRINE_ECLIPSE_MIRAGE_WINE_EXECUTABLE)
  set(CMAKE_CROSSCOMPILING_EMULATOR
    "${Python3_EXECUTABLE};${PROJECT_SOURCE_DIR}/scripts/run_wine_test.py;--timeout;180;${CITRINE_ECLIPSE_MIRAGE_WINE_EXECUTABLE}"
    CACHE STRING "Run the Windows test process without Wine service pipe inheritance" FORCE)
endif()
if(CITRINE_ECLIPSE_MIRAGE_PORTABLE_CHECKS_ONLY)
  enable_testing()
  include(cmake/PortableChecks.cmake)
  include(cmake/InfrastructureChecks.cmake)
  include(cmake/VendorChecks.cmake)
  return()
endif()
if(NOT WIN32 OR NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
  message(FATAL_ERROR "citrine_eclipse_mirage requires Windows x64 or an explicit Windows x64 cross toolchain.")
endif()
enable_language(RC)
set_property(SOURCE resources/surge.rc PROPERTY OBJECT_DEPENDS
  "${PROJECT_SOURCE_DIR}/resources/version.rc;${_project_version_header}")
include(CTest)
if(MSVC)
  # Compiler-owned standard CRT; consistent /MT Release and /MTd Debug targets.
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
option(CITRINE_ECLIPSE_MIRAGE_GOLDEN_LINK_PROFILE "Emit the bounded MSVC Golden link profile" OFF)
if(CITRINE_ECLIPSE_MIRAGE_GOLDEN_LINK_PROFILE AND NOT MSVC)
  message(FATAL_ERROR "Golden shipping link evidence requires native MSVC on Windows.")
endif()

option(CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL "Build the separate C++ source API development DLL" ${BUILD_TESTING})
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
add_library(citrine_eclipse_mirage_vdf_objects OBJECT ${CITRINE_ECLIPSE_MIRAGE_VDF_SOURCES})
target_include_directories(citrine_eclipse_mirage_vdf_objects PRIVATE include)
target_link_libraries(citrine_eclipse_mirage_vdf_objects PRIVATE citrine_eclipse_mirage_pegtl)
target_compile_definitions(citrine_eclipse_mirage_vdf_objects PRIVATE NOMINMAX)
set_target_properties(citrine_eclipse_mirage_vdf_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
add_library(citrine_eclipse_mirage_objects OBJECT ${CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES}
  src/platform/crypt_query_host.cpp "${_avatar_header}")
target_include_directories(citrine_eclipse_mirage_objects PUBLIC include)
target_include_directories(citrine_eclipse_mirage_objects PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_link_libraries(citrine_eclipse_mirage_objects PUBLIC citrine_eclipse_mirage_minhook citrine_eclipse_mirage_pugixml
  citrine_eclipse_mirage_asio)
target_compile_definitions(citrine_eclipse_mirage_objects PUBLIC NOMINMAX)
set_target_properties(citrine_eclipse_mirage_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)

add_library(citrine_eclipse_mirage_core STATIC $<TARGET_OBJECTS:citrine_eclipse_mirage_objects>
  $<TARGET_OBJECTS:citrine_eclipse_mirage_vdf_objects>)
target_include_directories(citrine_eclipse_mirage_core PUBLIC include)
target_compile_definitions(citrine_eclipse_mirage_core PUBLIC NOMINMAX)
target_link_libraries(citrine_eclipse_mirage_core PUBLIC citrine_eclipse_mirage_minhook citrine_eclipse_mirage_pugixml
  citrine_eclipse_mirage_asio
  crypt32 ws2_32 kernel32 user32 shell32)

# The application DLL has an explicit C export table. Source-level C++ APIs
# belong to a separate development DLL and are never exported by this target.
add_library(citrine_eclipse_mirage SHARED $<TARGET_OBJECTS:citrine_eclipse_mirage_objects>
  $<TARGET_OBJECTS:citrine_eclipse_mirage_vdf_objects> src/platform/dll_entry.cpp
  resources/surge.rc exports/citrine_eclipse_mirage.def)
target_include_directories(citrine_eclipse_mirage PRIVATE include)
target_link_libraries(citrine_eclipse_mirage PRIVATE citrine_eclipse_mirage_core)
target_include_directories(citrine_eclipse_mirage PRIVATE resources)
set_target_properties(citrine_eclipse_mirage PROPERTIES
  OUTPUT_NAME "citrine_eclipse_mirage.unsigned" PREFIX "" WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(MSVC)
  # All application profiles have the same exact VERSIONINFO-only resource contract.
  # The default MSVC DLL manifest would add an unselected RT_MANIFEST.
  # Executable manifests are unaffected.
  target_link_options(citrine_eclipse_mirage PRIVATE /MANIFEST:NO)
  target_compile_options(citrine_eclipse_mirage_objects PRIVATE /GS)
  target_compile_options(citrine_eclipse_mirage_vdf_objects PRIVATE /GS)
  if(CITRINE_ECLIPSE_MIRAGE_GOLDEN_LINK_PROFILE)
    target_link_options(citrine_eclipse_mirage PRIVATE /DEBUG:NONE /INCREMENTAL:NO /LTCG:OFF /NOCOFFGRPINFO)
  endif()
endif()
if(MINGW)
  target_link_options(citrine_eclipse_mirage PRIVATE -Wl,--exclude-all-symbols)
endif()

# Reference payloads exist only for tests or the explicitly selected source DLL.
if(BUILD_TESTING OR CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL)
  add_library(citrine_eclipse_mirage_reference_objects OBJECT ${CITRINE_ECLIPSE_MIRAGE_REFERENCE_SOURCES})
  target_include_directories(citrine_eclipse_mirage_reference_objects PUBLIC include)
  target_link_libraries(citrine_eclipse_mirage_reference_objects PRIVATE citrine_eclipse_mirage_minhook citrine_eclipse_mirage_pugixml citrine_eclipse_mirage_asio)
  target_compile_definitions(citrine_eclipse_mirage_reference_objects PRIVATE NOMINMAX)
  set_target_properties(citrine_eclipse_mirage_reference_objects PROPERTIES POSITION_INDEPENDENT_CODE ON)
  if(MSVC)
    target_compile_options(citrine_eclipse_mirage_reference_objects PRIVATE /GS)
  endif()
  add_library(citrine_eclipse_mirage_reference_models STATIC $<TARGET_OBJECTS:citrine_eclipse_mirage_reference_objects>)
  target_link_libraries(citrine_eclipse_mirage_reference_models PUBLIC citrine_eclipse_mirage_core)
endif()
if(CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL)
  if(MSVC)
    # C++ objects cross the development DLL boundary, including Debug STL
    # container proxies. Both modules must use the same shared compiler CRT.
    # Never reuse the shipping /MT or /MTd objects in this /MD or /MDd graph.
    set(_source_api_crt "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
    get_target_property(_source_hook_sources citrine_eclipse_mirage_minhook SOURCES)
    get_target_property(_source_hook_includes citrine_eclipse_mirage_minhook INTERFACE_INCLUDE_DIRECTORIES)
    add_library(citrine_eclipse_mirage_source_minhook STATIC ${_source_hook_sources})
    target_include_directories(citrine_eclipse_mirage_source_minhook PUBLIC ${_source_hook_includes})
    add_dependencies(citrine_eclipse_mirage_source_minhook citrine_eclipse_mirage_verify_vendors citrine_eclipse_mirage_verify_hook_engine)
    add_library(citrine_eclipse_mirage_source_pugixml STATIC third_party/pugixml/src/pugixml.cpp)
    target_include_directories(citrine_eclipse_mirage_source_pugixml PUBLIC third_party/pugixml/src)
    target_compile_definitions(citrine_eclipse_mirage_source_pugixml PUBLIC PUGIXML_NO_XPATH)
    add_dependencies(citrine_eclipse_mirage_source_pugixml citrine_eclipse_mirage_verify_vendors)
    set_target_properties(citrine_eclipse_mirage_source_minhook citrine_eclipse_mirage_source_pugixml
      PROPERTIES MSVC_RUNTIME_LIBRARY "${_source_api_crt}")
    add_library(citrine_eclipse_mirage_source_api SHARED ${CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES}
      src/platform/crypt_query_host.cpp "${_avatar_header}"
      ${CITRINE_ECLIPSE_MIRAGE_VDF_SOURCES} ${CITRINE_ECLIPSE_MIRAGE_REFERENCE_SOURCES}
      src/platform/dll_entry.cpp resources/surge.rc)
    target_include_directories(citrine_eclipse_mirage_source_api PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
    target_compile_definitions(citrine_eclipse_mirage_source_api PRIVATE NOMINMAX)
    target_compile_options(citrine_eclipse_mirage_source_api PRIVATE /GS)
    target_link_libraries(citrine_eclipse_mirage_source_api PRIVATE citrine_eclipse_mirage_source_minhook
      citrine_eclipse_mirage_source_pugixml citrine_eclipse_mirage_asio citrine_eclipse_mirage_pegtl
      crypt32 ws2_32 kernel32 user32 shell32)
    set_target_properties(citrine_eclipse_mirage_source_api PROPERTIES MSVC_RUNTIME_LIBRARY "${_source_api_crt}")
  else()
    add_library(citrine_eclipse_mirage_source_api SHARED $<TARGET_OBJECTS:citrine_eclipse_mirage_objects>
      $<TARGET_OBJECTS:citrine_eclipse_mirage_vdf_objects> $<TARGET_OBJECTS:citrine_eclipse_mirage_reference_objects>
      src/platform/dll_entry.cpp resources/surge.rc)
    target_link_libraries(citrine_eclipse_mirage_source_api PRIVATE citrine_eclipse_mirage_core)
  endif()
  target_include_directories(citrine_eclipse_mirage_source_api PRIVATE include resources)
  set_target_properties(citrine_eclipse_mirage_source_api PROPERTIES
    OUTPUT_NAME "citrine_eclipse_mirage.source" PREFIX "" WINDOWS_EXPORT_ALL_SYMBOLS ON)
  if(MSVC)
    target_link_options(citrine_eclipse_mirage_source_api PRIVATE /MANIFEST:NO)
  endif()
  if(MINGW)
    target_link_options(citrine_eclipse_mirage_source_api PRIVATE -Wl,--export-all-symbols)
  endif()
  add_executable(citrine_eclipse_mirage_source_host tools/citrine_eclipse_mirage_source_host/main.cpp)
  target_include_directories(citrine_eclipse_mirage_source_host PRIVATE include)
  target_link_libraries(citrine_eclipse_mirage_source_host PRIVATE citrine_eclipse_mirage_source_api citrine_eclipse_mirage_asio)
  if(MSVC)
    set_target_properties(citrine_eclipse_mirage_source_host PROPERTIES MSVC_RUNTIME_LIBRARY "${_source_api_crt}")
  endif()
endif()

add_executable(citrine_eclipse_mirage_runtime_host tools/citrine_eclipse_mirage_runtime_host/main.cpp)
target_include_directories(citrine_eclipse_mirage_runtime_host PRIVATE include)
target_link_libraries(citrine_eclipse_mirage_runtime_host PRIVATE citrine_eclipse_mirage_asio)
if(MINGW)
  target_link_options(citrine_eclipse_mirage_runtime_host PRIVATE -municode)
endif()
add_dependencies(citrine_eclipse_mirage_runtime_host citrine_eclipse_mirage)

if(BUILD_TESTING)
  add_executable(application_runtime_contracts tests/native/application_runtime_tests.cpp resources/surge.rc)
  target_include_directories(application_runtime_contracts PRIVATE resources)
  target_link_libraries(application_runtime_contracts PRIVATE citrine_eclipse_mirage_core advapi32)
  add_library(online_provider_fixture SHARED tests/support/runtime/online_fixture.cpp)
  target_link_libraries(online_provider_fixture PRIVATE advapi32)
  set_target_properties(online_provider_fixture PROPERTIES OUTPUT_NAME "citrine_eclipse_mirage_online" PREFIX "")
  set_target_properties(online_provider_fixture PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixtures"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/fixtures")
  add_dependencies(application_runtime_contracts online_provider_fixture)
  add_test(NAME application_runtime_contracts COMMAND application_runtime_contracts
    $<TARGET_FILE:online_provider_fixture> "${PROJECT_SOURCE_DIR}/resources/branding/surge.png"
    "${PROJECT_SOURCE_DIR}/resources/branding/default-avatar.png" -showLSX)
  set_tests_properties(application_runtime_contracts PROPERTIES TIMEOUT 60)
  add_executable(configuration_platform_contracts tests/native/configuration_platform_tests.cpp)
  target_include_directories(configuration_platform_contracts PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
  target_link_libraries(configuration_platform_contracts PRIVATE citrine_eclipse_mirage_core advapi32)
  add_dependencies(configuration_platform_contracts citrine_eclipse_mirage_objects)
  add_test(NAME configuration_platform_contracts COMMAND configuration_platform_contracts)
  set_tests_properties(configuration_platform_contracts PROPERTIES TIMEOUT 60)
  add_executable(environment_dialog_contracts tests/native/environment_dialog_tests.cpp)
  target_include_directories(environment_dialog_contracts PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
  target_link_libraries(environment_dialog_contracts PRIVATE citrine_eclipse_mirage_core advapi32)
  add_dependencies(environment_dialog_contracts citrine_eclipse_mirage_objects)
  add_test(NAME environment_dialog_contracts COMMAND environment_dialog_contracts)
  set_tests_properties(environment_dialog_contracts PROPERTIES TIMEOUT 60)
  add_executable(environment_license_key_contracts tests/native/environment_license_key_tests.cpp)
  target_include_directories(environment_license_key_contracts PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
  target_link_libraries(environment_license_key_contracts PRIVATE citrine_eclipse_mirage_core advapi32)
  add_dependencies(environment_license_key_contracts citrine_eclipse_mirage_objects)
  add_test(NAME environment_license_key_contracts COMMAND environment_license_key_contracts)
  set_tests_properties(environment_license_key_contracts PROPERTIES TIMEOUT 60)
  add_executable(environment_path_contracts tests/native/environment_path_tests.cpp)
  target_include_directories(environment_path_contracts PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
  target_link_libraries(environment_path_contracts PRIVATE citrine_eclipse_mirage_core advapi32)
  add_dependencies(environment_path_contracts citrine_eclipse_mirage_objects)
  add_test(NAME environment_path_contracts COMMAND environment_path_contracts)
  set_tests_properties(environment_path_contracts PROPERTIES TIMEOUT 60)
  add_executable(startup_publication_contracts tests/native/startup_publication_tests.cpp)
  target_link_libraries(startup_publication_contracts PRIVATE citrine_eclipse_mirage_core advapi32)
  add_test(NAME startup_publication_contracts COMMAND startup_publication_contracts)
  set_tests_properties(startup_publication_contracts PROPERTIES TIMEOUT 60)
  include(cmake/InfrastructureChecks.cmake)
  include(cmake/VendorChecks.cmake)
  add_executable(citrine_eclipse_mirage_crt_host tools/citrine_eclipse_mirage_crt_host/main.cpp)
  add_test(NAME crt_runtime_smoke COMMAND citrine_eclipse_mirage_crt_host)
  set_tests_properties(crt_runtime_smoke PROPERTIES TIMEOUT 60)
  include(cmake/Tests.cmake)
  add_executable(vdf_parser_regressions tests/unit/parser/vdf_regression_tests.cpp)
  target_link_libraries(vdf_parser_regressions PRIVATE citrine_eclipse_mirage_core)
  add_test(NAME vdf_parser_regressions COMMAND vdf_parser_regressions)
  set_tests_properties(vdf_parser_regressions PROPERTIES TIMEOUT 60)
  if(CITRINE_ECLIPSE_MIRAGE_BUILD_SOURCE_API_DLL)
    add_test(NAME dll_source_smoke COMMAND citrine_eclipse_mirage_source_host)
  endif()
  add_test(NAME dll_public_lifecycle COMMAND citrine_eclipse_mirage_runtime_host $<TARGET_FILE:citrine_eclipse_mirage>)
  set_tests_properties(dll_public_lifecycle PROPERTIES TIMEOUT 60)
  add_test(NAME dll_public_xml_contracts COMMAND citrine_eclipse_mirage_runtime_host $<TARGET_FILE:citrine_eclipse_mirage> --xml-contracts)
  set_tests_properties(dll_public_xml_contracts PROPERTIES TIMEOUT 120)
  add_executable(dll_public_c_consumer tests/native/public_c_consumer.c)
  target_include_directories(dll_public_c_consumer PRIVATE include)
  target_link_libraries(dll_public_c_consumer PRIVATE citrine_eclipse_mirage)
  add_test(NAME dll_public_c_consumer COMMAND dll_public_c_consumer)
  set_tests_properties(dll_public_c_consumer PROPERTIES TIMEOUT 60)
  add_test(NAME dll_public_export_contract COMMAND "${Python3_EXECUTABLE}" -B
    "${PROJECT_SOURCE_DIR}/scripts/check_dll_exports.py" $<TARGET_FILE:citrine_eclipse_mirage>)
  add_test(NAME dll_public_export_negatives COMMAND "${Python3_EXECUTABLE}" -B
    "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_public_abi.py" $<TARGET_FILE:citrine_eclipse_mirage>)
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
  target_link_libraries(hook_engine_fixture_tests PRIVATE citrine_eclipse_mirage_minhook)
  add_test(NAME hook_engine_fixtures COMMAND hook_engine_fixture_tests)
endif()

citrine_eclipse_mirage_install_partition_gate()
