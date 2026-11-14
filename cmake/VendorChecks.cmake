# SPDX-License-Identifier: GPL-3.0-only

add_executable(vendor_api_smoke "${PROJECT_SOURCE_DIR}/tests/vendors/vendor_api_smoke.cpp")
target_link_libraries(vendor_api_smoke PRIVATE laureondo_asio
  laureondo_pugixml)
if(WIN32)
  target_link_libraries(vendor_api_smoke PRIVATE mithrilhook)
endif()
add_test(NAME vendor_api_smoke COMMAND vendor_api_smoke)
set_tests_properties(vendor_api_smoke PROPERTIES TIMEOUT 60)
add_executable(vendor_io_contracts "${PROJECT_SOURCE_DIR}/tests/vendors/vendor_io_contracts.cpp")
target_link_libraries(vendor_io_contracts PRIVATE laureondo_asio)
add_test(NAME vendor_io_contracts COMMAND vendor_io_contracts)
set_tests_properties(vendor_io_contracts PROPERTIES TIMEOUT 30)
add_test(NAME vendor_source_integrity COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/scripts/vendor_sync.py" --root "${PROJECT_SOURCE_DIR}" --verify-installed)

# The compatibility probe runs in VendoredDependencies even without tests.
if(LAUREONDO_PEGTL_PUBLIC_INPUT_WORKS)
  add_executable(vendor_pegtl_smoke "${PROJECT_SOURCE_DIR}/tests/vendors/vendor_pegtl_smoke.cpp")
  target_link_libraries(vendor_pegtl_smoke PRIVATE laureondo_pegtl)
  add_test(NAME vendor_pegtl_smoke COMMAND vendor_pegtl_smoke)
else()
  message(STATUS "Supplied PEGTL public-input headers are incompatible with this toolchain; vendor unchanged. See CMakeFiles/CMakeConfigureLog.yaml")
  add_test(NAME vendor_pegtl_toolchain_unavailable COMMAND "${Python3_EXECUTABLE}" -B
    "${PROJECT_SOURCE_DIR}/scripts/vendor_toolchain_status.py" "${CMAKE_BINARY_DIR}/CMakeCache.txt")
  set_tests_properties(vendor_pegtl_toolchain_unavailable PROPERTIES SKIP_RETURN_CODE 77)
endif()
