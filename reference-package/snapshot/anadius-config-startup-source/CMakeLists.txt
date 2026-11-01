cmake_minimum_required(VERSION 3.20)
project(config_startup LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(config_startup STATIC startup.cpp ../anadius-config-application-source/application.cpp
    ../anadius-config-hash-source/hash_table.cpp ../anadius-parser-source/parser.cpp
    ../anadius-config-source/configuration.cpp ../anadius-file-source/binary_io.cpp
    ../anadius-file-source/native_io.cpp ../anadius-source/bootstrap.cpp ../anadius-startup-source/startup.cpp)
add_executable(config_startup_tests startup_tests.cpp)
target_link_libraries(config_startup_tests PRIVATE config_startup)
enable_testing()
add_test(NAME config_startup_checks COMMAND config_startup_tests "${CMAKE_CURRENT_BINARY_DIR}/generated-fixtures")
