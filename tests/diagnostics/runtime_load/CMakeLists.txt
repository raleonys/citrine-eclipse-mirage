# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)
project(runtime_load_diagnostic LANGUAGES CXX)
if(NOT WIN32)
  message(FATAL_ERROR "The actual Asio/native-file diagnostic requires Windows")
endif()
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
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
# Deliberately separate from root production/reference targets and ordinary
# CTest. run.py validates correctness before accepting any timing sample.
