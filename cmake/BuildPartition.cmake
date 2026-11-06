# SPDX-License-Identifier: GPL-3.0-only

# Configure/build proof for the production/reference/source-API target boundary.
# Request compiler-generated target/dependency data, including on fresh builds.
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/.cmake/api/v1/query")
file(WRITE "${CMAKE_BINARY_DIR}/.cmake/api/v1/query/codemodel-v2" "")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${PROJECT_SOURCE_DIR}/metadata/BUILD_PARTITION_POLICY.json"
  "${PROJECT_SOURCE_DIR}/scripts/check_build_partition.py")
execute_process(COMMAND "${Python3_EXECUTABLE}" -B
  "${PROJECT_SOURCE_DIR}/scripts/check_build_partition.py" --root "${PROJECT_SOURCE_DIR}" --source-only
  RESULT_VARIABLE _partition_status OUTPUT_VARIABLE _partition_output ERROR_VARIABLE _partition_error)
if(NOT _partition_status EQUAL 0)
  message(FATAL_ERROR "Build partition source contract failed:\n${_partition_output}${_partition_error}")
endif()

function(preloader_l_install_partition_gate)
  set(_map "${CMAKE_CURRENT_BINARY_DIR}/preloader.production.$<CONFIG>.map")
  if(MSVC)
    target_link_options(preloader_l PRIVATE "/MAP:${_map}")
  elseif(MINGW)
    target_link_options(preloader_l PRIVATE "-Wl,-Map,${_map}")
  else()
    message(FATAL_ERROR "Production partition evidence requires a supported Windows linker map")
  endif()
  set_property(TARGET preloader_l APPEND PROPERTY LINK_DEPENDS
    "${PROJECT_SOURCE_DIR}/scripts/check_build_partition.py"
    "${PROJECT_SOURCE_DIR}/metadata/BUILD_PARTITION_POLICY.json"
    "${CMAKE_CURRENT_BINARY_DIR}/partition.$<CONFIG>.targets")
  add_custom_target(preloader_l_verify_partition ALL
    COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/check_build_partition.py"
      --root "${PROJECT_SOURCE_DIR}" --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
      --config "$<CONFIG>" --map "${_map}" --binary "$<TARGET_FILE:preloader_l>"
      --report "${CMAKE_CURRENT_BINARY_DIR}/production-boundary.$<CONFIG>.json"
    DEPENDS preloader_l
    COMMENT "Recheck production boundary on incremental builds" VERBATIM)
  # Emit evaluated source lists and a conservative target dependency graph.
  # A file-API query first written during configure is answered only on a later
  # configure; this generation-time proof is available on the first clean build.
  get_property(_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
  set(_graph "")
  foreach(_target IN LISTS _targets)
    set(_deps)
    foreach(_property IN ITEMS LINK_LIBRARIES INTERFACE_LINK_LIBRARIES MANUALLY_ADDED_DEPENDENCIES SOURCES)
      get_target_property(_values "${_target}" "${_property}")
      if(_values)
        foreach(_value IN LISTS _values)
          set(_tokens)
          if(TARGET "${_value}")
            set(_tokens "${_value}")
          elseif(_value MATCHES "\\$<")
            string(REGEX MATCHALL "[A-Za-z_][A-Za-z0-9_]*" _tokens "${_value}")
          endif()
          foreach(_token IN LISTS _tokens)
            if(TARGET "${_token}" AND NOT _token STREQUAL _target)
              get_target_property(_aliased "${_token}" ALIASED_TARGET)
              if(_aliased)
                list(APPEND _deps "${_aliased}")
              else()
                list(APPEND _deps "${_token}")
              endif()
            endif()
          endforeach()
        endforeach()
      endif()
    endforeach()
    list(REMOVE_DUPLICATES _deps)
    get_target_property(_type "${_target}" TYPE)
    string(APPEND _graph "TARGET\t${_target}\nTYPE\t${_type}\nDEPS\t${_deps}\nSOURCES\t$<TARGET_GENEX_EVAL:${_target},$<TARGET_PROPERTY:${_target},SOURCES>>\nEND\n")
  endforeach()
  file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/partition.$<CONFIG>.targets" CONTENT "${_graph}")
  # Keep the unsigned application output in the same import order as Golden.
  # MSVC may emit a different descriptor sequence from transitive Asio libs.
  if(MSVC)
    set_property(TARGET preloader_l APPEND PROPERTY LINK_DEPENDS
      "${PROJECT_SOURCE_DIR}/scripts/import_descriptor_order.py"
      "${PROJECT_SOURCE_DIR}/metadata/URSYNEX_GOLDEN_FINALIZER.json"
      "${PROJECT_SOURCE_DIR}/metadata/CORVEXON_GOLDEN_TRAIL.json"
      "${PROJECT_SOURCE_DIR}/metadata/DIST_CONTRACT.json")
    add_custom_command(TARGET preloader_l POST_BUILD
      COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/import_descriptor_order.py"
        --root "${PROJECT_SOURCE_DIR}" --dll "$<TARGET_FILE:preloader_l>"
      COMMENT "Normalize unsigned DLL import descriptors before validation" VERBATIM)
  endif()
  add_custom_command(TARGET preloader_l POST_BUILD
    COMMAND "${Python3_EXECUTABLE}" -B "${PROJECT_SOURCE_DIR}/scripts/check_build_partition.py"
      --root "${PROJECT_SOURCE_DIR}" --build-dir "${CMAKE_CURRENT_BINARY_DIR}"
      --config "$<CONFIG>" --map "${_map}" --binary "$<TARGET_FILE:preloader_l>"
      --report "${CMAKE_CURRENT_BINARY_DIR}/production-boundary.$<CONFIG>.json"
    COMMENT "Verify production target closure and linked object boundary" VERBATIM)
  if(BUILD_TESTING)
    file(READ "${PROJECT_SOURCE_DIR}/metadata/BUILD_PARTITION_POLICY.json" _policy)
    # These exhaustive model fixtures encrypt large payload matrices with an
    # unoptimized CRT/codec under Wine. This is a test-process budget only.
    if(CMAKE_CROSSCOMPILING AND PRELOADER_L_WINE_EXECUTABLE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
      string(JSON _slow_count LENGTH "${_policy}" "cross_debug_large_reference_tests")
      string(JSON _slow_timeout GET "${_policy}" "cross_debug_reference_budget_seconds")
      math(EXPR _runner_timeout "${_slow_timeout}+60")
      math(EXPR _slow_last "${_slow_count}-1")
      foreach(_i RANGE 0 ${_slow_last})
        string(JSON _slow GET "${_policy}" "cross_debug_large_reference_tests" ${_i})
        set_tests_properties("${_slow}" PROPERTIES TIMEOUT "${_slow_timeout}")
        set_property(TARGET "${_slow}" PROPERTY CROSSCOMPILING_EMULATOR
          "${Python3_EXECUTABLE};${PROJECT_SOURCE_DIR}/scripts/run_wine_test.py;--timeout;${_runner_timeout};${PRELOADER_L_WINE_EXECUTABLE}")
      endforeach()
    endif()
    foreach(_kind IN ITEMS reference production)
      string(JSON _count LENGTH "${_policy}" "${_kind}_test_targets")
      set(_${_kind}_targets)
      if(_count GREATER 0)
        math(EXPR _last "${_count}-1")
        foreach(_i RANGE 0 ${_last})
          string(JSON _name GET "${_policy}" "${_kind}_test_targets" ${_i})
          list(APPEND _${_kind}_targets "${_name}")
        endforeach()
      endif()
    endforeach()
    get_property(_tests DIRECTORY PROPERTY TESTS)
    foreach(_test IN LISTS _tests)
      if(_test IN_LIST _reference_targets)
        set_tests_properties("${_test}" PROPERTIES LABELS "reference")
      elseif(_test STREQUAL "dll_source_smoke")
        set_tests_properties("${_test}" PROPERTIES LABELS "source-api")
      elseif(_test STREQUAL "hook_engine_fixtures")
        set_tests_properties("${_test}" PROPERTIES LABELS "vendor-comparison")
      elseif(_test IN_LIST _production_targets OR _test MATCHES "^dll_public_")
        set_tests_properties("${_test}" PROPERTIES LABELS "production")
      else()
        set_tests_properties("${_test}" PROPERTIES LABELS "infrastructure")
      endif()
    endforeach()
  endif()
endfunction()
