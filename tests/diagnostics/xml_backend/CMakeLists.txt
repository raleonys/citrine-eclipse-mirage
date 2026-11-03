# SPDX-License-Identifier: GPL-3.0-only
cmake_minimum_required(VERSION 3.20)
project(xml_backend_comparison LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT XML_REFERENCE_ROOT OR NOT PUGIXML_ROOT)
  message(FATAL_ERROR "Provide XML_REFERENCE_ROOT (PR #108 control) and external PUGIXML_ROOT")
endif()
file(TO_CMAKE_PATH "${XML_REFERENCE_ROOT}" XML_REFERENCE_ROOT)
file(TO_CMAKE_PATH "${PUGIXML_ROOT}" PUGIXML_ROOT)
if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
if(WIN32)
  add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
option(XML_COMPARISON_SANITIZERS "Use ASan and UBSan in this standalone diagnostic" OFF)
if(XML_COMPARISON_SANITIZERS)
  if(MSVC)
    message(FATAL_ERROR "The ASan/UBSan diagnostic profile requires GCC or Clang")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()

# Also verify inputs on direct CMake invocation. No implicit network access.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
execute_process(COMMAND "${Python3_EXECUTABLE}" -B "${CMAKE_CURRENT_SOURCE_DIR}/run.py"
  --reference-root "${XML_REFERENCE_ROOT}" --pugixml-root "${PUGIXML_ROOT}" --verify-only
  RESULT_VARIABLE input_status)
if(NOT input_status EQUAL 0)
  message(FATAL_ERROR "XML comparison input identity check failed")
endif()

add_library(comparison_tiny STATIC "${XML_REFERENCE_ROOT}/third_party/tinyxml2/tinyxml2.cpp")
target_include_directories(comparison_tiny PUBLIC "${XML_REFERENCE_ROOT}/third_party/tinyxml2")
add_library(comparison_pugi STATIC "${PUGIXML_ROOT}/src/pugixml.cpp")
target_include_directories(comparison_pugi PUBLIC "${PUGIXML_ROOT}/src")
target_compile_definitions(comparison_pugi PUBLIC PUGIXML_NO_XPATH)
# The candidate and the control share only the pinned first-party value policy
# and protocol/printer sources. No candidate target links the TinyXML2 oracle.
file(READ "${XML_REFERENCE_ROOT}/src/xml/reader.cpp" reference_source)
string(FIND "${reference_source}" "bool xml_character(" policy_begin)
string(FIND "${reference_source}" "class Document;" policy_end)
if(policy_begin LESS 0 OR policy_end LESS policy_begin)
  message(FATAL_ERROR "Pinned reference value-policy extraction markers changed")
endif()
math(EXPR policy_length "${policy_end}-${policy_begin}")
string(SUBSTRING "${reference_source}" ${policy_begin} ${policy_length} value_policy)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/reference_value_policy.hpp"
  "#include <cstdint>\n#include <string>\n#include <string_view>\n#include <utility>\nnamespace xml_comparison::policy {\n${value_policy}\n}\n")
add_library(comparison_common STATIC reference_codec.cpp
  "${XML_REFERENCE_ROOT}/src/xml/memory_printer.cpp"
  "${XML_REFERENCE_ROOT}/src/protocol/messages.cpp")
target_include_directories(comparison_common PUBLIC "${XML_REFERENCE_ROOT}/include" "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(comparison_common PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
add_library(comparison_reference STATIC "${XML_REFERENCE_ROOT}/src/xml/reader.cpp")
target_link_libraries(comparison_reference PUBLIC comparison_common comparison_tiny)
add_library(comparison_candidate STATIC backend.cpp compatibility.cpp)
target_link_libraries(comparison_candidate PUBLIC comparison_common comparison_pugi)
add_library(comparison_storage STATIC
  "${XML_REFERENCE_ROOT}/src/runtime/achievement_storage.cpp"
  "${XML_REFERENCE_ROOT}/src/runtime/achievement_writer.cpp"
  "${XML_REFERENCE_ROOT}/src/core/binary_io.cpp"
  "${XML_REFERENCE_ROOT}/src/core/paths.cpp")
target_include_directories(comparison_storage PUBLIC "${XML_REFERENCE_ROOT}/include")

# Reuse the actual test fixtures without their entrypoints. Exact input hashes
# and the markers below make extraction fail if those fixtures change.
file(READ "${XML_REFERENCE_ROOT}/tests/unit/messages/messages_tests.cpp" messages_fixture)
string(FIND "${messages_fixture}" "\nint main()" entrypoint)
if(entrypoint LESS 0)
  message(FATAL_ERROR "Messages fixture extraction marker changed")
endif()
string(SUBSTRING "${messages_fixture}" 0 ${entrypoint} messages_fixture)
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/messages_fixtures.hpp" "${messages_fixture}\n")
file(READ "${XML_REFERENCE_ROOT}/tests/unit/achievement_storage/storage_fixture.hpp" storage_fixture)
string(REPLACE "namespace {" "namespace eval_storage {" storage_fixture "${storage_fixture}")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/storage_fixtures.hpp" "${storage_fixture}")
add_executable(comparison_suite suite.cpp)
target_include_directories(comparison_suite PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_link_libraries(comparison_suite PRIVATE comparison_candidate comparison_reference comparison_storage)
add_executable(comparison_allocations allocation.cpp)
target_link_libraries(comparison_allocations PRIVATE comparison_candidate comparison_reference)
foreach(test IN ITEMS reader corpus)
  add_executable(reference_${test} "${XML_REFERENCE_ROOT}/tests/unit/xml_reader/${test}_tests.cpp")
  target_link_libraries(reference_${test} PRIVATE comparison_reference)
  file(READ "${XML_REFERENCE_ROOT}/tests/unit/xml_reader/${test}_tests.cpp" candidate_tests)
  string(REPLACE "#include \"preloader_l/xml/reader.hpp\"" "#include \"backend.hpp\"" candidate_tests "${candidate_tests}")
  string(REPLACE "namespace reader=preloader_l::xml_reader_bridge;" "namespace reader=xml_comparison;" candidate_tests "${candidate_tests}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/candidate_${test}.cpp" "${candidate_tests}")
  add_executable(candidate_${test} "${CMAKE_CURRENT_BINARY_DIR}/candidate_${test}.cpp")
  target_link_libraries(candidate_${test} PRIVATE comparison_candidate)
endforeach()

if(WIN32)
  # Run the unchanged complete router/AES/settings/license and native storage
  # fixtures against both backends. Only their reader binding changes.
  add_library(comparison_windows_services STATIC
    "${XML_REFERENCE_ROOT}/src/codec/crypto.cpp"
    "${XML_REFERENCE_ROOT}/src/router/router.cpp"
    "${XML_REFERENCE_ROOT}/src/handlers/handlers.cpp"
    "${XML_REFERENCE_ROOT}/src/handlers/misc_handlers.cpp"
    "${XML_REFERENCE_ROOT}/src/handlers/content_handlers.cpp"
    "${XML_REFERENCE_ROOT}/src/core/license.cpp"
    "${XML_REFERENCE_ROOT}/src/core/atomic_files.cpp"
    "${XML_REFERENCE_ROOT}/src/platform/file_io_windows.cpp"
    "${XML_REFERENCE_ROOT}/src/platform/atomic_files_windows.cpp")
  target_link_libraries(comparison_windows_services PUBLIC comparison_common comparison_storage)
  foreach(test IN ITEMS dispatch storage)
    if(test STREQUAL "dispatch")
      set(test_source "${XML_REFERENCE_ROOT}/tests/unit/dispatch/integration_tests.cpp")
    else()
      set(test_source "${XML_REFERENCE_ROOT}/tests/unit/achievement_storage/storage_tests.cpp")
    endif()
    add_executable(reference_${test} "${test_source}")
    target_link_libraries(reference_${test} PRIVATE comparison_reference comparison_windows_services)
    file(READ "${test_source}" candidate_tests)
    string(REPLACE "namespace reader=preloader_l::xml_reader_bridge;" "namespace reader=xml_comparison;" candidate_tests "${candidate_tests}")
    string(REPLACE "xml_reader_bridge::Backend" "xml_comparison::Backend" candidate_tests "${candidate_tests}")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/candidate_${test}.cpp" "#include \"backend.hpp\"\n${candidate_tests}")
    add_executable(candidate_${test} "${CMAKE_CURRENT_BINARY_DIR}/candidate_${test}.cpp")
    target_link_libraries(candidate_${test} PRIVATE comparison_candidate comparison_windows_services)
  endforeach()
  file(READ "${XML_REFERENCE_ROOT}/tests/unit/achievement_storage/storage_fixture.hpp" candidate_storage_fixture)
  string(REPLACE "xml_reader_bridge::Backend" "xml_comparison::Backend" candidate_storage_fixture "${candidate_storage_fixture}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/storage_fixture.hpp" "#include \"backend.hpp\"\n${candidate_storage_fixture}")
endif()
