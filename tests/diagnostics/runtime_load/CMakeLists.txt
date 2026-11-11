# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)
project(runtime_load_diagnostic LANGUAGES C CXX)
if(NOT WIN32)
  message(FATAL_ERROR "The actual Asio/native-file diagnostic requires Windows")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
get_filename_component(RUNTIME_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)
find_package(Python3 REQUIRED COMPONENTS Interpreter)
enable_testing()
add_executable(runtime_scheduler_contracts probe_tests.cpp)
add_test(NAME runtime_scheduler_contracts COMMAND runtime_scheduler_contracts)
set_tests_properties(runtime_scheduler_contracts PROPERTIES TIMEOUT 30)
execute_process(COMMAND "${Python3_EXECUTABLE}" -B "${CMAKE_CURRENT_SOURCE_DIR}/run.py"
  --prepare "${CMAKE_CURRENT_BINARY_DIR}/generated"
  RESULT_VARIABLE prepared)
if(NOT prepared EQUAL 0)
  message(FATAL_ERROR "Benchmark source identity/preparation failed")
endif()
add_library(runtime_benchmark_parts STATIC
  "${RUNTIME_ROOT}/src/net/server.cpp"
  "${RUNTIME_ROOT}/src/runtime/execution.cpp"
  "${RUNTIME_ROOT}/src/platform/execution_windows.cpp"
  "${RUNTIME_ROOT}/src/xml/reader.cpp"
  "${RUNTIME_ROOT}/src/xml/reader_policy.cpp"
  "${RUNTIME_ROOT}/src/xml/reader_values.cpp"
  "${RUNTIME_ROOT}/src/xml/memory_printer.cpp"
  "${RUNTIME_ROOT}/src/core/binary_io.cpp"
  "${RUNTIME_ROOT}/src/core/atomic_files.cpp"
  "${RUNTIME_ROOT}/src/platform/file_io_windows.cpp"
  "${RUNTIME_ROOT}/src/platform/atomic_files_windows.cpp"
  "${RUNTIME_ROOT}/third_party/pugixml/src/pugixml.cpp")
target_include_directories(runtime_benchmark_parts PUBLIC
  "${RUNTIME_ROOT}/include" "${RUNTIME_ROOT}/third_party/asio/include"
  "${RUNTIME_ROOT}/third_party/pugixml/src" "${CMAKE_CURRENT_SOURCE_DIR}")
target_compile_definitions(runtime_benchmark_parts PUBLIC
  ASIO_STANDALONE ASIO_HAS_THREADS=1 _WIN32_WINNT=0x0601 WINVER=0x0601
  NOMINMAX WIN32_LEAN_AND_MEAN PUGIXML_NO_XPATH)
target_link_libraries(runtime_benchmark_parts PUBLIC ws2_32 mswsock psapi)
foreach(workers IN ITEMS 1 2 4 8 0)
  add_executable(runtime_load_${workers} load.cpp "${CMAKE_CURRENT_BINARY_DIR}/generated/asio_${workers}.cpp")
  target_compile_definitions(runtime_load_${workers} PRIVATE RUNTIME_BENCHMARK_WORKERS=${workers})
  target_link_libraries(runtime_load_${workers} PRIVATE runtime_benchmark_parts)
  set_target_properties(runtime_load_${workers} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
endforeach()

# Reuse the accepted explicit source list and pinned dependency build recipes.
# This standalone diagnostic does not configure or modify the root DLL graph.
function(runtime_application_dependencies)
  set(PROJECT_SOURCE_DIR "${RUNTIME_ROOT}")
  include("${RUNTIME_ROOT}/cmake/VendoredDependencies.cmake")
endfunction()
runtime_application_dependencies()
include("${RUNTIME_ROOT}/cmake/Sources.cmake")
set(application_sources ${CITRINE_ECLIPSE_MIRAGE_APPLICATION_SOURCES}
  ${CITRINE_ECLIPSE_MIRAGE_VDF_SOURCES} src/platform/crypt_query_host.cpp)
list(REMOVE_ITEM application_sources src/net/asio_service.cpp src/runtime/application.cpp)
list(TRANSFORM application_sources PREPEND "${RUNTIME_ROOT}/")
set(avatar_header "${CMAKE_CURRENT_BINARY_DIR}/generated/avatar_data.hpp")
add_custom_command(OUTPUT "${avatar_header}"
  COMMAND "${Python3_EXECUTABLE}" -B "${RUNTIME_ROOT}/scripts/generate_avatar_data.py"
    --root "${RUNTIME_ROOT}" --output "${avatar_header}"
  DEPENDS "${RUNTIME_ROOT}/scripts/generate_avatar_data.py"
    "${RUNTIME_ROOT}/metadata/AVATAR_DATA_CONTRACT.json"
    "${RUNTIME_ROOT}/resources/branding/surge.png"
    "${RUNTIME_ROOT}/resources/branding/default-avatar.png" VERBATIM)
add_library(runtime_application_parts STATIC ${application_sources}
  "${avatar_header}" "${CMAKE_CURRENT_BINARY_DIR}/generated/application.cpp")
target_include_directories(runtime_application_parts PUBLIC "${RUNTIME_ROOT}/include"
  "${RUNTIME_ROOT}/src/runtime" "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}/generated")
target_compile_definitions(runtime_application_parts PUBLIC NOMINMAX)
target_link_libraries(runtime_application_parts PUBLIC citrine_eclipse_mirage_asio
  citrine_eclipse_mirage_pegtl citrine_eclipse_mirage_minhook citrine_eclipse_mirage_pugixml
  crypt32 ws2_32 kernel32 user32 shell32 advapi32 psapi)
if(MINGW AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  set_source_files_properties("${RUNTIME_ROOT}/src/platform/hook_unload_guard.cpp"
    PROPERTIES COMPILE_OPTIONS "-fms-extensions")
endif()
foreach(workers IN ITEMS 1 2 4 8 0)
  add_executable(application_load_${workers} application_load.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/generated/asio_${workers}.cpp")
  target_compile_definitions(application_load_${workers} PRIVATE RUNTIME_BENCHMARK_WORKERS=${workers})
  target_link_libraries(application_load_${workers} PRIVATE runtime_application_parts)
  set_target_properties(application_load_${workers} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>")
endforeach()
add_test(NAME runtime_application_response_contracts COMMAND application_load_2 --verify-controls)
set_tests_properties(runtime_application_response_contracts PROPERTIES TIMEOUT 30)
# Deliberately separate from root production/reference targets and ordinary
# CTest. run.py validates correctness before accepting any timing sample.
