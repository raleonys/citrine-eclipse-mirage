cmake_minimum_required(VERSION 3.20)
project(config_pipeline LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(config_pipeline STATIC pipeline.cpp ../anadius-config-order-source/order.cpp
    ../anadius-parser-source/parser.cpp ../anadius-config-source/configuration.cpp
    ../anadius-file-source/binary_io.cpp ../anadius-file-source/native_io.cpp)
add_executable(config_pipeline_tests pipeline_tests.cpp)
target_link_libraries(config_pipeline_tests PRIVATE config_pipeline)
enable_testing()
add_test(NAME config_pipeline_checks COMMAND config_pipeline_tests "${CMAKE_CURRENT_BINARY_DIR}/generated-fixtures")
