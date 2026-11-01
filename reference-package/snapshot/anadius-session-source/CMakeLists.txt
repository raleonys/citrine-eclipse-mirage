cmake_minimum_required(VERSION 3.20)
project(session_bridge_source LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_library(session_bridge_source STATIC handler.cpp routing.cpp
    ../anadius-server-source/server.cpp ../anadius-execution-source/execution.cpp
    ../anadius-messages-source/messages.cpp ../anadius-crypto-source/crypto.cpp
    ../anadius-dispatch-source/router.cpp ../anadius-handlers-source/handlers.cpp
    ../anadius-event-queue-source/queue.cpp ../anadius-event-queue-source/adapters.cpp
    ../anadius-achievement-runtime-source/runtime.cpp ../anadius-achievement-runtime-source/utc.cpp
    ../anadius-calendar-source/calendar.cpp ../anadius-achievement-writer-source/writer.cpp
    ../anadius-content-handlers-source/content_handlers.cpp ../anadius-misc-handlers-source/misc_handlers.cpp
    ../anadius-xml-reader-source/reader.cpp ../anadius-xml-reader-source/vendor/tinyxml2/tinyxml2.cpp
    ../anadius-xml-source/memory_printer.cpp ../anadius-path-source/paths.cpp ../anadius-file-source/binary_io.cpp)
add_executable(session_handler_tests handler_tests.cpp)
target_link_libraries(session_handler_tests PRIVATE session_bridge_source)
add_executable(session_integration_tests session_tests.cpp)
target_link_libraries(session_integration_tests PRIVATE session_bridge_source)
enable_testing()
add_test(NAME session_handler_checks COMMAND session_handler_tests)
add_test(NAME session_integration_checks COMMAND session_integration_tests)
