cmake_minimum_required(VERSION 3.20)
project(timestamp_source LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(timestamp_source STATIC timestamp.cpp ../anadius-calendar-source/calendar.cpp
    ../anadius-achievement-storage-source/storage.cpp ../anadius-achievement-writer-source/writer.cpp
    ../anadius-xml-reader-source/reader.cpp ../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp
    ../anadius-xml-source/memory_printer.cpp ../anadius-path-source/paths.cpp
    ../anadius-file-source/binary_io.cpp ../anadius-file-source/native_io.cpp)
add_executable(timestamp_tests timestamp_tests.cpp)
target_link_libraries(timestamp_tests PRIVATE timestamp_source)
enable_testing()
add_test(NAME timestamp_checks COMMAND timestamp_tests "${CMAKE_CURRENT_BINARY_DIR}/generated-fixtures")
