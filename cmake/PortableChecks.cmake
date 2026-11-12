# SPDX-License-Identifier: GPL-3.0-only

add_library(portable_config STATIC src/config/configuration.cpp)
target_include_directories(portable_config PUBLIC include)
function(portable_check name)
  add_executable(${name} ${ARGN})
  target_include_directories(${name} PRIVATE include "${PROJECT_SOURCE_DIR}")
  target_link_libraries(${name} PRIVATE portable_config)
  add_test(NAME ${name} COMMAND ${name})
endfunction()
add_library(portable_xml_reader_backend STATIC
  src/xml/reader.cpp src/xml/reader_policy.cpp src/xml/reader_values.cpp src/xml/memory_printer.cpp)
target_include_directories(portable_xml_reader_backend PUBLIC include)
target_link_libraries(portable_xml_reader_backend PUBLIC citrine_eclipse_mirage_pugixml)
portable_check(portable_xml_reader tests/unit/xml_reader/reader_tests.cpp)
target_link_libraries(portable_xml_reader PRIVATE portable_xml_reader_backend)
portable_check(portable_xml_reader_allocations tests/performance/xml_reader_probe.cpp)
target_link_libraries(portable_xml_reader_allocations PRIVATE portable_xml_reader_backend)
add_executable(portable_xml_reader_corpus tests/unit/xml_reader/corpus_tests.cpp)
target_link_libraries(portable_xml_reader_corpus PRIVATE portable_xml_reader_backend)
add_test(NAME portable_xml_reader_corpus COMMAND portable_xml_reader_corpus
  "${PROJECT_SOURCE_DIR}/tests/fixtures/xml_reader/optimization-corpus.txt"
  "${PROJECT_SOURCE_DIR}/tests/fixtures/xml_reader/optimization-baseline.txt")
portable_check(portable_calendar src/runtime/calendar.cpp tests/unit/calendar/calendar_tests.cpp)
find_package(Threads REQUIRED)
portable_check(portable_execution src/runtime/execution.cpp tests/unit/execution/execution_tests.cpp)
portable_check(portable_execution_concurrency src/runtime/execution.cpp tests/unit/execution/concurrency_tests.cpp)
target_link_libraries(portable_execution_concurrency PRIVATE Threads::Threads)
set_tests_properties(portable_execution_concurrency PROPERTIES TIMEOUT 30)
# Recovered source models with synthetic effects, not native CRT/OS threads.
portable_check(portable_bootstrap_model src/runtime/bootstrap.cpp
  tests/unit/bootstrap/bootstrap_tests.cpp)
portable_check(portable_thread_model src/runtime/thread_launch.cpp src/runtime/bootstrap.cpp
  tests/unit/thread/thread_tests.cpp)
set_tests_properties(portable_bootstrap_model portable_thread_model PROPERTIES TIMEOUT 60)
portable_check(portable_network_hook src/platform/hook_connect.cpp src/runtime/execution.cpp
  tests/unit/network_hook/hook_tests.cpp)
target_compile_definitions(portable_network_hook PRIVATE CITRINE_ECLIPSE_MIRAGE_PRODUCTION_NETWORK_TESTS=1)
target_link_libraries(portable_network_hook PRIVATE Threads::Threads)
portable_check(portable_crypt_path_probe src/platform/crypt_query.cpp tests/native/crypt_path_probe_tests.cpp)
target_compile_definitions(portable_crypt_path_probe PRIVATE CITRINE_ECLIPSE_MIRAGE_PORTABLE_CRYPT_PATH_TESTS=1)
set_tests_properties(portable_crypt_path_probe PROPERTIES TIMEOUT 30)
portable_check(portable_parser src/config/parser.cpp tests/unit/parser/parser_tests.cpp)
portable_check(portable_hash src/hash/hash.cpp tests/unit/hash/hash_tests.cpp)
portable_check(portable_file_io src/core/binary_io.cpp tests/unit/file/binary_io_tests.cpp)
target_compile_definitions(portable_file_io PRIVATE CITRINE_ECLIPSE_MIRAGE_PORTABLE_FILE_IO_TESTS=1)
portable_check(portable_config_order src/config/parser.cpp src/config/order.cpp tests/unit/config_order/order_tests.cpp)
portable_check(portable_config_hash src/config/parser.cpp src/config/hash_table.cpp tests/unit/config_hash/hash_table_tests.cpp)
portable_check(portable_config_hash_model_differential src/config/parser.cpp src/config/order.cpp src/config/hash_table.cpp tests/unit/config_hash/order_differential_tests.cpp)
add_executable(portable_config_application tests/unit/config_application/application_tests.cpp
  src/config/application.cpp src/config/parser.cpp src/config/hash_table.cpp
  src/core/binary_io.cpp src/runtime/online.cpp)
target_include_directories(portable_config_application PRIVATE include)
target_link_libraries(portable_config_application PRIVATE portable_config citrine_eclipse_mirage_pegtl)
target_compile_definitions(portable_config_application PRIVATE CITRINE_ECLIPSE_MIRAGE_PORTABLE_CONFIG_APPLICATION_TESTS=1)
add_test(NAME portable_config_application COMMAND portable_config_application
  "${CMAKE_CURRENT_BINARY_DIR}/config-application-fixtures")
set_tests_properties(portable_config_application PROPERTIES TIMEOUT 60)
portable_check(portable_host_hooks src/platform/hook_setup.cpp src/core/paths.cpp tests/unit/host_init/host_init_tests.cpp)
foreach(_parser_target portable_parser portable_config_order portable_config_hash portable_config_hash_model_differential)
  target_link_libraries(${_parser_target} PRIVATE citrine_eclipse_mirage_pegtl)
endforeach()
portable_check(vdf_parser_regressions src/config/parser.cpp tests/unit/parser/vdf_regression_tests.cpp)
target_link_libraries(vdf_parser_regressions PRIVATE citrine_eclipse_mirage_pegtl)
portable_check(portable_configuration_defaults src/runtime/application_defaults.cpp tests/unit/config_defaults/defaults_tests.cpp)
portable_check(portable_configuration_defaults_integration
  tests/unit/config_defaults/defaults_integration_tests.cpp
  src/runtime/application_defaults.cpp src/config/config_startup.cpp src/config/application.cpp
  src/config/parser.cpp src/config/order.cpp src/config/hash_table.cpp src/runtime/startup.cpp
  src/core/binary_io.cpp src/handlers/handlers.cpp src/xml/memory_printer.cpp)
target_link_libraries(portable_configuration_defaults_integration PRIVATE citrine_eclipse_mirage_pegtl)
portable_check(portable_online src/runtime/online.cpp src/core/paths.cpp tests/unit/online/online_tests.cpp)
target_compile_definitions(portable_online PRIVATE CITRINE_ECLIPSE_MIRAGE_PORTABLE_ONLINE_TESTS=1)
portable_check(portable_server src/net/server.cpp src/runtime/execution.cpp tests/unit/server/server_tests.cpp)
portable_check(portable_hook_transaction src/platform/hook_transaction.cpp tests/unit/hooks/transaction_tests.cpp)
portable_check(portable_registry_rollback src/platform/registry.cpp tests/unit/registry/allocation_failure_tests.cpp)

portable_check(portable_atomic_recovery src/core/atomic_files.cpp tests/unit/file/atomic_recovery_tests.cpp)
set_tests_properties(portable_atomic_recovery PROPERTIES TIMEOUT 30)
