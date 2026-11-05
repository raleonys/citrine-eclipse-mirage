# Included only when RECOVERED_ENABLE_DLL_LINK_GATE is explicitly enabled.
# Source image integration; a linked artifact still needs runtime validation.
if(NOT MINGW OR NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(FATAL_ERROR "DLL link gate currently requires the recorded LLVM-MinGW Clang toolchain")
endif()
enable_language(ASM)
set(DLL_GATE "${OUT}/dll-link-gate-source")
add_library(anadius64_reconstruction SHARED
  "${DLL_GATE}/entry.cpp" "${DLL_GATE}/export_leaf.S"
  "${DLL_GATE}/anadius64.def"
  "${OUT}/anadius-configuration-factory-source/factory.cpp"
  "${OUT}/anadius-services-factory-source/factory.cpp"
  "${OUT}/anadius-online-factory-source/factory.cpp")
# Direct source selects the production factory for diagnostic linkage only.
# entry.cpp defines both loader helpers and the real native image assembly.
# Building this target does not invoke those factories or execute the DLL.
target_link_libraries(anadius64_reconstruction PRIVATE anadius_core_partial)
set_target_properties(anadius64_reconstruction PROPERTIES
  PREFIX "" OUTPUT_NAME "anadius64.reconstruction"
  WINDOWS_EXPORT_ALL_SYMBOLS OFF)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_link_options(anadius64_reconstruction PRIVATE -static
    "-Wl,--exclude-all-symbols,-Map,${CMAKE_CURRENT_BINARY_DIR}/anadius64.reconstruction.map")
endif()
# Real application roots are referenced by the entry implementation; no
# deliberately undefined loader symbols remain in the source strategy.
