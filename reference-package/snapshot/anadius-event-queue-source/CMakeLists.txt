cmake_minimum_required(VERSION 3.20)
project(event_queue_source LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(event_queue_source STATIC queue.cpp adapters.cpp
    ../anadius-achievement-runtime-source/runtime.cpp ../anadius-achievement-runtime-source/utc.cpp
    ../anadius-calendar-source/calendar.cpp ../anadius-achievement-writer-source/writer.cpp
    ../anadius-content-handlers-source/content_handlers.cpp ../anadius-misc-handlers-source/misc_handlers.cpp
    ../anadius-messages-source/messages.cpp ../anadius-xml-reader-source/reader.cpp
    ../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp ../anadius-xml-source/memory_printer.cpp
    ../anadius-path-source/paths.cpp ../anadius-file-source/binary_io.cpp)
add_executable(event_queue_tests queue_tests.cpp)
target_link_libraries(event_queue_tests PRIVATE event_queue_source)
add_executable(event_queue_integration_tests integration_tests.cpp)
target_link_libraries(event_queue_integration_tests PRIVATE event_queue_source)
enable_testing()
add_test(NAME event_queue_checks COMMAND event_queue_tests)
add_test(NAME event_queue_integration_checks COMMAND event_queue_integration_tests)
