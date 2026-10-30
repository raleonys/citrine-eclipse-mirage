# Host-Python infrastructure tests run on Linux as well as native Windows.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
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
