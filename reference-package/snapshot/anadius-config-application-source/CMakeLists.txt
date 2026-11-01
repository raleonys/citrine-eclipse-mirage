cmake_minimum_required(VERSION 3.20)
project(config_application LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(config_application STATIC application.cpp ../anadius-config-hash-source/hash_table.cpp
    ../anadius-parser-source/parser.cpp ../anadius-config-source/configuration.cpp
    ../anadius-file-source/binary_io.cpp ../anadius-file-source/native_io.cpp)
add_executable(config_application_tests application_tests.cpp)
target_link_libraries(config_application_tests PRIVATE config_application)
enable_testing()
add_test(NAME config_application_checks COMMAND config_application_tests "${CMAKE_CURRENT_BINARY_DIR}/generated-fixtures")
