cmake_minimum_required(VERSION 3.20)
project(achievement_runtime_source LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(achievement_runtime_source STATIC utc.cpp runtime.cpp
    ../anadius-calendar-source/calendar.cpp ../anadius-timestamp-source/timestamp.cpp
    ../anadius-achievement-storage-source/storage.cpp ../anadius-achievement-writer-source/writer.cpp
    ../anadius-content-handlers-source/content_handlers.cpp
    ../anadius-xml-reader-source/reader.cpp ../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp
    ../anadius-xml-source/memory_printer.cpp ../anadius-path-source/paths.cpp
    ../anadius-file-source/binary_io.cpp ../anadius-file-source/native_io.cpp)
add_executable(utc_tests utc_tests.cpp)
target_link_libraries(utc_tests PRIVATE achievement_runtime_source)
add_executable(achievement_runtime_tests runtime_tests.cpp)
target_link_libraries(achievement_runtime_tests PRIVATE achievement_runtime_source)
enable_testing()
add_test(NAME utc_checks COMMAND utc_tests)
add_test(NAME achievement_runtime_checks COMMAND achievement_runtime_tests "${CMAKE_CURRENT_BINARY_DIR}/generated-fixtures")
