# SPDX-License-Identifier: GPL-3.0-only

# Exact owner-provided archive trees only. Upstream CMake and network fetching
# are not executed. Full member hashes and link types are verified offline.
find_package(Python3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/scripts/vendor_sync.py" --root "${PROJECT_SOURCE_DIR}" --verify-installed
  RESULT_VARIABLE _vendor_result OUTPUT_VARIABLE _vendor_output ERROR_VARIABLE _vendor_error)
if(NOT _vendor_result EQUAL 0)
  message(FATAL_ERROR "Vendored source verification failed:\n${_vendor_output}${_vendor_error}")
endif()
add_custom_target(citrine_eclipse_mirage_verify_vendors ALL
  COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/vendor_sync.py"
    --root "${PROJECT_SOURCE_DIR}" --verify-installed
  COMMENT "Verify the four active owner-supplied vendor trees" VERBATIM)

add_library(citrine_eclipse_mirage_asio INTERFACE)
target_include_directories(citrine_eclipse_mirage_asio INTERFACE "${PROJECT_SOURCE_DIR}/third_party/asio/include")
target_compile_definitions(citrine_eclipse_mirage_asio INTERFACE ASIO_STANDALONE)
if(WIN32)
  # LLVM-MinGW does not define the _MT/_REENTRANT macros used by Asio's
  # autodetection, despite providing real Win32 threads. Declare that capability.
  # Match the supplied Infrastructure Windows target and Asio's existing fallback.
  # Declare it before any Windows/Asio header, including in downstream hosts/tests.
  target_compile_definitions(citrine_eclipse_mirage_asio INTERFACE
    ASIO_HAS_THREADS=1 _WIN32_WINNT=0x0601 WINVER=0x0601)
  target_link_libraries(citrine_eclipse_mirage_asio INTERFACE ws2_32 mswsock)
else()
  find_package(Threads REQUIRED)
  target_link_libraries(citrine_eclipse_mirage_asio INTERFACE Threads::Threads)
endif()

add_library(citrine_eclipse_mirage_pegtl INTERFACE)
target_include_directories(citrine_eclipse_mirage_pegtl INTERFACE "${PROJECT_SOURCE_DIR}/third_party/pegtl/include")
target_compile_features(citrine_eclipse_mirage_pegtl INTERFACE cxx_std_17)

# Check the real public API even for BUILD_TESTING=OFF. A dependency target's
# existence is not evidence that its headers compile with the selected STL.
option(CITRINE_ECLIPSE_MIRAGE_REQUIRE_VENDOR_COMPATIBILITY
  "Reject toolchains that cannot compile the required PEGTL public API" ON)
include(CheckCXXSourceCompiles)
set(_saved_required_includes "${CMAKE_REQUIRED_INCLUDES}")
set(CMAKE_REQUIRED_INCLUDES "${PROJECT_SOURCE_DIR}/third_party/pegtl/include")
# The selected 4.x snapshot supports C++17; the probe uses project C++20.
check_cxx_source_compiles("#include <tao/pegtl.hpp>\nint main() { tao::pegtl::view_input in(\"x\"); return tao::pegtl::parse<tao::pegtl::ascii::alpha>(in) ? 0 : 1; }"
  CITRINE_ECLIPSE_MIRAGE_PEGTL_PUBLIC_INPUT_WORKS)
set(CMAKE_REQUIRED_INCLUDES "${_saved_required_includes}")
if(NOT CITRINE_ECLIPSE_MIRAGE_PEGTL_PUBLIC_INPUT_WORKS)
  if(CITRINE_ECLIPSE_MIRAGE_REQUIRE_VENDOR_COMPATIBILITY)
    message(FATAL_ERROR
      "Required PEGTL public API does not compile with the selected toolchain. "
      "Vendor source remains unchanged. Use a compatible toolchain; see docs/VENDORING.md "
      "and CMakeFiles/CMakeConfigureLog.yaml (or CMakeError.log on older CMake). "
      "CITRINE_ECLIPSE_MIRAGE_REQUIRE_VENDOR_COMPATIBILITY=OFF is diagnostic-only and does not establish compatibility.")
  endif()
  message(WARNING "Diagnostic-only build: required PEGTL public API is unavailable; vendor unchanged.")
endif()

add_library(citrine_eclipse_mirage_pugixml STATIC "${PROJECT_SOURCE_DIR}/third_party/pugixml/src/pugixml.cpp")
target_include_directories(citrine_eclipse_mirage_pugixml PUBLIC "${PROJECT_SOURCE_DIR}/third_party/pugixml/src")
target_compile_definitions(citrine_eclipse_mirage_pugixml PUBLIC PUGIXML_NO_XPATH)
add_dependencies(citrine_eclipse_mirage_pugixml citrine_eclipse_mirage_verify_vendors)
add_dependencies(citrine_eclipse_mirage_asio citrine_eclipse_mirage_verify_vendors)
add_dependencies(citrine_eclipse_mirage_pegtl citrine_eclipse_mirage_verify_vendors)

if(WIN32)
  set(_hook_build_copy "${CMAKE_CURRENT_BINARY_DIR}/vendor_build/minhook")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/metadata/HOOK_ENGINE_PATCH.json"
    "${PROJECT_SOURCE_DIR}/scripts/prepare_hook_engine.py"
    "${PROJECT_SOURCE_DIR}/cmake/PatchHookFreeze.cmake")
  execute_process(COMMAND "${Python3_EXECUTABLE}" -B
    "${PROJECT_SOURCE_DIR}/scripts/prepare_hook_engine.py" --root "${PROJECT_SOURCE_DIR}"
    --output "${_hook_build_copy}" --prepare --cmake "${CMAKE_COMMAND}"
    RESULT_VARIABLE _hook_patch_result OUTPUT_VARIABLE _hook_patch_output ERROR_VARIABLE _hook_patch_error)
  if(NOT _hook_patch_result EQUAL 0)
    message(FATAL_ERROR "Hook engine preparation failed:\n${_hook_patch_output}${_hook_patch_error}")
  endif()
  add_custom_target(citrine_eclipse_mirage_verify_hook_engine ALL
    COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/prepare_hook_engine.py"
      --root "${PROJECT_SOURCE_DIR}" --output "${_hook_build_copy}"
    COMMENT "Verify the pinned hook engine build copy" VERBATIM)
  add_library(citrine_eclipse_mirage_minhook STATIC
    "${_hook_build_copy}/src/hook.c"
    "${_hook_build_copy}/src/buffer.c"
    "${_hook_build_copy}/src/trampoline.c"
    "${_hook_build_copy}/src/hde/hde64.c")
  target_include_directories(citrine_eclipse_mirage_minhook PUBLIC "${_hook_build_copy}/include")
  add_dependencies(citrine_eclipse_mirage_minhook citrine_eclipse_mirage_verify_vendors citrine_eclipse_mirage_verify_hook_engine)
endif()
