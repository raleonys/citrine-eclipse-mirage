# SPDX-License-Identifier: GPL-3.0-only

# Host-Python infrastructure tests run on Linux as well as native Windows.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
add_test(NAME infrastructure_project_version COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_project_version.py")
set_tests_properties(infrastructure_project_version PROPERTIES TIMEOUT 30)

# Compile a real parser consumer without vendor usage requirements. Reject an
# accidental PEGTL include even on hosts with a system-wide PEGTL installation.
set(_parser_header_guard "${CMAKE_CURRENT_BINARY_DIR}/generated/parser-header-contract")
# Enumerate header names only, never application translation units or vendor code.
file(GLOB_RECURSE _parser_vendor_headers CONFIGURE_DEPENDS LIST_DIRECTORIES FALSE
  RELATIVE "${PROJECT_SOURCE_DIR}/third_party/pegtl/include"
  "${PROJECT_SOURCE_DIR}/third_party/pegtl/include/tao/*")
foreach(_header IN LISTS _parser_vendor_headers)
  get_filename_component(_directory "${_parser_header_guard}/${_header}" DIRECTORY)
  file(MAKE_DIRECTORY "${_directory}")
  file(WRITE "${_parser_header_guard}/${_header}"
    "#error PEGTL implementation headers must not enter the public parser interface\n")
endforeach()
add_library(parser_header_contracts OBJECT
  "${PROJECT_SOURCE_DIR}/tests/unit/parser/header_contract.cpp")
target_include_directories(parser_header_contracts PRIVATE
  "${_parser_header_guard}" "${PROJECT_SOURCE_DIR}/include")

add_test(NAME infrastructure_golden_pe COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_pe_metadata_tools.py")
add_test(NAME infrastructure_named_imports COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_corvexon_mswsock_imports.py")
add_test(NAME infrastructure_msvc_link COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/scripts/check_msvc_shipping_link_command.py" --self-test)
add_test(NAME infrastructure_ownership_packaging COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_infrastructure.py")
add_test(NAME infrastructure_target_requirements COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_requirements.py")
add_test(NAME infrastructure_vendor_contract COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_vendor_contract.py")
set_tests_properties(infrastructure_golden_pe infrastructure_named_imports
  infrastructure_msvc_link infrastructure_ownership_packaging infrastructure_target_requirements
  infrastructure_vendor_contract PROPERTIES TIMEOUT 90)
# Host-side Wine launcher regression (also runs without Wine installed).
add_test(NAME infrastructure_wine_runner
  COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_wine_runner.py")
set_tests_properties(infrastructure_wine_runner PROPERTIES TIMEOUT 30)

add_test(NAME infrastructure_build_partition COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_build_partition.py")
set_tests_properties(infrastructure_build_partition PROPERTIES TIMEOUT 30)
add_test(NAME infrastructure_size_evidence
  COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_size_evidence.py")
set_tests_properties(infrastructure_size_evidence PROPERTIES TIMEOUT 30)

add_test(NAME infrastructure_weekly_release
  COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_weekly_release.py")
set_tests_properties(infrastructure_weekly_release PROPERTIES TIMEOUT 90)

add_test(NAME infrastructure_xml_performance
  COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/tests/infrastructure/test_xml_performance.py")
set_tests_properties(infrastructure_xml_performance PROPERTIES TIMEOUT 30)
