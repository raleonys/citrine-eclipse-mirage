cmake_minimum_required(VERSION 3.20)
project(achievement_storage LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(achievement_storage STATIC storage.cpp ../anadius-achievement-writer-source/writer.cpp
    ../anadius-xml-reader-source/reader.cpp ../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp
    ../anadius-xml-source/memory_printer.cpp ../anadius-path-source/paths.cpp
    ../anadius-file-source/binary_io.cpp ../anadius-file-source/native_io.cpp
    ../anadius-config-startup-source/startup.cpp ../anadius-config-application-source/application.cpp
    ../anadius-config-hash-source/hash_table.cpp ../anadius-parser-source/parser.cpp
    ../anadius-config-source/configuration.cpp ../anadius-startup-source/startup.cpp)
add_executable(achievement_storage_tests storage_tests.cpp)
target_link_libraries(achievement_storage_tests PRIVATE achievement_storage)
enable_testing()
add_test(NAME achievement_storage_checks COMMAND achievement_storage_tests "${CMAKE_CURRENT_BINARY_DIR}/generated-fixtures")
