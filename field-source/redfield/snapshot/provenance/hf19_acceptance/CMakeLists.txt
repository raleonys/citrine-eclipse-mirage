cmake_minimum_required(VERSION 3.20)
project(preloader_l LANGUAGES C CXX)

if(NOT WIN32)
    message(FATAL_ERROR "preloader_l is a Windows x64 DLL project. Configure with MSVC on Windows.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "preloader_l must be built for x64.")
endif()

if(MSVC)
    enable_language(RC)
    # Governs newly added project targets unless an exact target policy overrides it.
    # Release/native use /MT; separate developer Debug targets explicitly use /MTd.
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()


# SAFE-FREEZE: MinHook is fully vendored for deterministic/offline builds.
# No configure-time network, Git checkout, package-manager, or external source path is accepted.
set(_preloader_l_minhook_pin "8af6b4acae5a9388fd742b56fa79ece89d96f823")
set(_preloader_l_minhook_vendored "${CMAKE_CURRENT_SOURCE_DIR}/third_party/minhook/upstream")
set(_preloader_l_minhook_commit_file "${CMAKE_CURRENT_SOURCE_DIR}/third_party/minhook/UPSTREAM_COMMIT.txt")
set(_preloader_l_minhook_required
    "include/MinHook.h"
    "src/buffer.c"
    "src/buffer.h"
    "src/hook.c"
    "src/trampoline.c"
    "src/trampoline.h"
    "src/hde/hde64.c"
    "src/hde/hde64.h"
    "src/hde/pstdint.h"
    "src/hde/table64.h"
    "LICENSE.txt"
)
foreach(_rel IN LISTS _preloader_l_minhook_required)
    if(NOT EXISTS "${_preloader_l_minhook_vendored}/${_rel}")
        message(FATAL_ERROR
            "release-source invariant failed: vendored MinHook file is missing: third_party/minhook/upstream/${_rel}. "
            "This release does not download dependencies at configure time.")
    endif()
endforeach()
if(NOT EXISTS "${_preloader_l_minhook_commit_file}")
    message(FATAL_ERROR "release-source invariant failed: third_party/minhook/UPSTREAM_COMMIT.txt is missing")
endif()
file(READ "${_preloader_l_minhook_commit_file}" _preloader_l_minhook_commit)
string(STRIP "${_preloader_l_minhook_commit}" _preloader_l_minhook_commit)
if(NOT _preloader_l_minhook_commit STREQUAL _preloader_l_minhook_pin)
    message(FATAL_ERROR
        "Vendored MinHook provenance mismatch: expected ${_preloader_l_minhook_pin}, got ${_preloader_l_minhook_commit}")
endif()

# Patch only a build-tree copy. The vendored upstream tree stays pristine and hashable.
set(_preloader_l_minhook_patched "${CMAKE_BINARY_DIR}/preloader_l_minhook_safe_freeze")
file(REMOVE_RECURSE "${_preloader_l_minhook_patched}")
file(MAKE_DIRECTORY "${_preloader_l_minhook_patched}")
file(COPY "${_preloader_l_minhook_vendored}/" DESTINATION "${_preloader_l_minhook_patched}")
execute_process(
    COMMAND "${CMAKE_COMMAND}"
        -DMINHOOK_HOOK_C=${_preloader_l_minhook_patched}/src/hook.c
        -P "${CMAKE_CURRENT_SOURCE_DIR}/scripts/build_orchestration/PatchMinHookSafeFreeze.cmake"
    RESULT_VARIABLE _preloader_l_minhook_patch_result
)
if(NOT _preloader_l_minhook_patch_result EQUAL 0)
    message(FATAL_ERROR "Pinned vendored MinHook safe-freeze patch failed")
endif()

# Compile the exact required MinHook sources directly. Upstream CMake is intentionally
# not executed, avoiding external configure policy/deprecation drift.
add_library(minhook STATIC
    "${_preloader_l_minhook_patched}/src/buffer.c"
    "${_preloader_l_minhook_patched}/src/hook.c"
    "${_preloader_l_minhook_patched}/src/trampoline.c"
    "${_preloader_l_minhook_patched}/src/hde/hde64.c"
)
target_include_directories(minhook PUBLIC "${_preloader_l_minhook_patched}/include")
target_include_directories(minhook PRIVATE
    "${_preloader_l_minhook_patched}/src"
    "${_preloader_l_minhook_patched}/src/hde"
)
set_target_properties(minhook PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED YES
    C_EXTENSIONS NO
)
if(MSVC)
    # Audit2 S0081: normalized shipping and its MinHook dependency must realize
    # the same static CRT in every accepted configuration.
    set_property(TARGET minhook PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()
if(NOT TARGET minhook)
    message(FATAL_ERROR "Vendored MinHook target was not created")
endif()


# HF11: ship the SHA-pinned vendored TinyXML2 v11.0.0 implementation.
# No fetch at configure/build time. Production and debug link the same code.
add_library(preloader_l_tinyxml2 STATIC third_party/tinyxml2/tinyxml2.cpp)
target_include_directories(preloader_l_tinyxml2 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2")
if(MSVC)
    set_property(TARGET preloader_l_tinyxml2 PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# HF12 P1: version-controlled pristine PEGTL source, no network fallback.
set(_hf12_pegtl_pin "3b075432e94f6f906425350b43d03cd9cebfa4f1")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/third_party/pegtl/UPSTREAM_COMMIT.txt" _hf12_pegtl_actual)
string(STRIP "${_hf12_pegtl_actual}" _hf12_pegtl_actual)
if(NOT _hf12_pegtl_actual STREQUAL _hf12_pegtl_pin)
    message(FATAL_ERROR "HF12 PEGTL vendor revision mismatch")
endif()
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/pegtl/upstream/include/tao/pegtl.hpp")
    message(FATAL_ERROR "HF12 PEGTL pristine vendor include missing")
endif()

# HF12 P2: pristine ghc::filesystem path-only integration, no file-I/O API
# substitution. Git pin is checked locally; configure never downloads sources.
set(_hf12_filesystem_pin "3812f8a6f31ed67d32272ac33fe2b85c6b2c947e")
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/third_party/filesystem/UPSTREAM_COMMIT.txt" _hf12_filesystem_actual)
string(STRIP "${_hf12_filesystem_actual}" _hf12_filesystem_actual)
if(NOT _hf12_filesystem_actual STREQUAL _hf12_filesystem_pin)
    message(FATAL_ERROR "HF12 filesystem vendor revision mismatch")
endif()
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/filesystem/upstream/include/ghc/filesystem.hpp"
        OR NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/filesystem/upstream/LICENSE")
    message(FATAL_ERROR "HF12 pristine filesystem vendor or license missing")
endif()

# Local builds are independent of release-gate candidate identities.
# Windows x64, MSVC, vendored dependency pins and production QA remain enforced.

add_library(preloader_l SHARED
    src/dll_entry.cpp
    src/runtime/runtime_state.cpp
    src/runtime/ptd_runtime.cpp
    src/runtime/ptd_fls.cpp
    src/runtime/exception_runtime.cpp
    src/runtime/fatal_policy.cpp
    src/runtime/fatal_termination.cpp
    src/runtime/memory_allocator.cpp
    src/runtime/heap_allocator.cpp
    src/runtime/loader_bridge.cpp
    src/runtime/entry_flow.cpp
    src/runtime/secondary_integration_entry.cpp
    src/runtime/bootstrap_state.cpp
    src/runtime/bootstrap_runner.cpp
    src/runtime/release_server.cpp
    src/runtime/reference_owner.cpp
    src/runtime/service_storage.cpp
    src/runtime/service_projection.cpp
    src/runtime/service_abi.cpp
    src/runtime/service_store.cpp
    src/runtime/service_policy.cpp
    src/runtime/service_values.cpp
    src/runtime/service_settings.cpp
    src/runtime/service_xml.cpp
    src/runtime/release_service_lifetime.cpp
    src/runtime/release_server_lifecycle.cpp
    src/runtime/asio_listener_owner.cpp
    src/runtime/asio_session_owner.cpp
    src/runtime/asio_service_owner.cpp
    src/runtime/asio_release_bridge.cpp
    src/runtime/bootstrap_thread.cpp
    src/runtime/utc_time.cpp
    src/runtime/avatar_path.cpp
    src/runtime/event_queue.cpp
    src/runtime/achievement_watcher.cpp
    src/runtime/auth_code_service.cpp
    src/runtime/service_state.cpp
    src/runtime/service_state_storage.cpp
    src/runtime/service_state_parser.cpp
    src/runtime/service_state_grant.cpp
    src/config/settings_values.cpp
    src/config/settings_provider.cpp
    src/config/runtime_provider.cpp
    src/config/runtime_configuration.cpp
    src/config/runtime_configuration_parser.cpp
    src/config/config2_pegtl.cpp
    src/config/runtime_configuration_storage.cpp
    src/config/runtime_configuration_file.cpp
    src/config/process_configuration.cpp
    src/config/store_values.cpp
    src/config/store_parser.cpp
    src/config/store_materialization.cpp
    src/config/store_runtime.cpp
    src/config/store_service_bridge.cpp
    src/config/store_payload.cpp
    src/config/msvc_byte_vector_abi.cpp
    src/config/byte_vector_binding.cpp
    src/config/store_vectors.cpp
    src/hash/fnv_hash.cpp
    src/codec/hex_codec.cpp
    src/codec/aes128_cipher.cpp
    src/codec/message_codec.cpp
    src/codec/session_crypto.cpp
    src/net/port_mapping.cpp
    src/net/socket_runtime.cpp
    # Legacy socket listener/endpoint compiled only by native fault-injection and Debug probes.
    # Shipping Asio keeps WSA startup/cleanup only; legacy raw sockets are test/debug-only.
    src/net/winsock_lifecycle_native.cpp
    src/xml/lsx_framing.cpp
    src/xml/request_parser.cpp
    src/xml/request_children.cpp
    src/xml/response_builder.cpp
    src/xml/value_codec.cpp
    src/xml/value_codec_utf8.cpp
    src/xml/value_codec_normalize.cpp
    src/xml/value_codec_semantic.cpp
    src/xml/attribute_parser.cpp
    src/router/route_lookup.cpp
    src/router/route_inventory.cpp
    src/handlers/progressive_installation_route.cpp
    src/handlers/permission_route.cpp
    src/handlers/friends_route.cpp
    src/runtime/local_social_state.cpp
    src/runtime/local_social_host.cpp
    src/handlers/integration_routes.cpp
    src/handlers/group_info_route.cpp
    src/handlers/utc_time_route.cpp
    src/handlers/query_presence_route.cpp
    src/handlers/social_mutation_routes.cpp
    src/handlers/social_mutation_socket.cpp
    src/handlers/local_social_list_route.cpp
    src/handlers/config_route.cpp
    src/handlers/setting_value_route.cpp
    src/handlers/settings_snapshot_route.cpp
    src/handlers/handler_registry.cpp
    src/handlers/connection_state_route.cpp
    src/handlers/connection_state_binding.cpp
    src/handlers/runtime_settings_routes.cpp
    src/handlers/selector_pipeline.cpp
    src/handlers/game_info_routes.cpp
    src/handlers/profile_routes.cpp
    src/handlers/auth_routes.cpp
    src/handlers/store_routes.cpp
    src/handlers/achievement_routes.cpp
    src/handlers/achievement_grant_route.cpp
    src/handlers/achievement_query_route.cpp
    src/handlers/image_route.cpp
    src/core/request_dispatch.cpp
    src/protocol/challenge_handshake.cpp
    src/protocol/socket_exchange.cpp
    src/protocol/socket_exchange_wire.cpp
    src/protocol/socket_exchange_frame_step.cpp
    src/protocol/socket_exchange_transaction.cpp
    src/protocol/socket_exchange_transaction_delivery.cpp
        src/protocol/socket_exchange_write_completion.cpp
    src/protocol/socket_exchange_async_transaction.cpp
    src/platform/registry_policy.cpp
    src/platform/registry_policy_config.cpp
    src/platform/registry_policy_state.cpp
    src/platform/registry_policy_values.cpp
    src/platform/registry_bridge.cpp
    src/platform/registry_hook_runtime.cpp
    src/platform/registry_wrappers.cpp
    src/platform/registry_hooks.cpp
    src/platform/minhook_runtime.cpp
    src/platform/core_hooks.cpp
    src/platform/addon_hooks.cpp
    src/platform/addon_selection.cpp
    src/platform/connect_redirect.cpp
    src/platform/connect_redirect_hook.cpp
    src/platform/environment_policy.cpp
    src/platform/process_environment.cpp
    src/platform/pe_import.cpp
    src/platform/crypt_wrapper.cpp
    src/platform/crypt_observer.cpp
    resources/preloader_version.rc

)


option(PRELOADER_L_NATIVE_ACCEPTANCE "Build test-only loader-lock-outside teardown export" OFF)
if(PRELOADER_L_NATIVE_ACCEPTANCE)
    # Legacy worker/acceptor retained for native fault-injection ONLY, never in shipping/debug.
    target_sources(preloader_l PRIVATE
        tests/native/native_acceptance_exports.cpp
        exports/preloader_l_native_acceptance.def
    )
    # Retained as native-acceptance fault-injection models, NEVER as shipping DLL code.
    target_sources(preloader_l PRIVATE
        src/runtime/release_server_worker.cpp
        src/runtime/release_server_acceptor.cpp
        src/net/socket_listener.cpp
        src/net/socket_endpoint.cpp
        src/net/socket_transport.cpp
        src/net/winsock_native.cpp
    )
    target_compile_definitions(preloader_l PRIVATE PRELOADER_L_NATIVE_ACCEPTANCE=1)
else()
    target_sources(preloader_l PRIVATE exports/preloader_l.def)
endif()
target_compile_features(preloader_l PRIVATE cxx_std_20)
target_include_directories(preloader_l PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/pegtl/upstream/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/filesystem/upstream/include")
target_include_directories(preloader_l PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/upstream/include")
target_link_libraries(preloader_l PRIVATE preloader_l_tinyxml2)
# HF12 permanent static CRT authority. Historical 3.1.7 custom-entry source
# remains archived in earlier parent artifacts, not an active build variant.
option(PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY "OBSOLETE and forbidden by HF12 static CRT" OFF)
if(PRELOADER_L_COMPATIBILITY_CUSTOM_ENTRY)
    message(FATAL_ERROR "HF12 static CRT does not support the historical custom-entry comparison; consult preserved lineage")
endif()
target_compile_definitions(preloader_l PRIVATE
    WIN32_LEAN_AND_MEAN NOMINMAX
    PRELOADER_L_RELEASE_BOOTSTRAP=1
    ASIO_STANDALONE=1 ASIO_NO_TYPEID=1
    PRELOADER_L_TRACE_DIAGNOSTICS=0
)
if(MSVC)
    target_compile_options(preloader_l PRIVATE
        $<$<COMPILE_LANGUAGE:CXX>:/W4;/permissive-;/EHsc;/GR-;/GS>
    )
    # HF12 permanent static CRT: normal linker-selected CRT startup, no custom
    # entrypoint, no /NODEFAULTLIB, no source-owned STL/CRT internals.
    set_property(TARGET preloader_l PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_link_libraries(preloader_l PRIVATE kernel32 ws2_32 crypt32 minhook)
    target_link_options(preloader_l PRIVATE
        /INCREMENTAL:NO
        /SUBSYSTEM:WINDOWS,6.00 /OSVERSION:6.0 /DEBUG:NONE
        /LTCG:OFF /NOCOFFGRPINFO /MANIFEST:NO
    )
else()
    message(FATAL_ERROR "Normalized acceptance target is MSVC x64.")
endif()
if(PRELOADER_L_NATIVE_ACCEPTANCE)
    set_target_properties(preloader_l PROPERTIES PREFIX "" OUTPUT_NAME "preloader_l_native_test")
else()
    set_target_properties(preloader_l PROPERTIES PREFIX "" OUTPUT_NAME "preloader.unsigned")
endif()

add_executable(preloader_l_native_acceptance_host
    tools/preloader_l_native_acceptance_host/native_acceptance_main.cpp
    src/codec/aes128_cipher.cpp
    src/codec/hex_codec.cpp
    src/codec/message_codec.cpp
    src/codec/session_crypto.cpp
)
target_compile_features(preloader_l_native_acceptance_host PRIVATE cxx_std_17)
target_include_directories(preloader_l_native_acceptance_host PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_compile_definitions(preloader_l_native_acceptance_host PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
if(MSVC)
    set_property(TARGET preloader_l_native_acceptance_host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_compile_options(preloader_l_native_acceptance_host PRIVATE /W4 /permissive- /GS /GR-)
    target_link_options(preloader_l_native_acceptance_host PRIVATE /SUBSYSTEM:CONSOLE /INCREMENTAL:NO)
    target_link_libraries(preloader_l_native_acceptance_host PRIVATE kernel32 ws2_32 preloader_l_tinyxml2)
endif()
set_target_properties(preloader_l_native_acceptance_host PROPERTIES OUTPUT_NAME "preloader_l_native_acceptance_host")

option(PRELOADER_L_ENABLE_DEBUG_PROTOTYPE "Build the x64dbg-oriented debug prototype DLL" OFF)
if(PRELOADER_L_ENABLE_DEBUG_PROTOTYPE)
    # The developer debug DLL uses /MTd, so it needs a runtime-matched MinHook
    # dependency rather than reusing the normalized shipping /MT target.
    add_library(minhook_debug STATIC
        "${_preloader_l_minhook_patched}/src/buffer.c"
        "${_preloader_l_minhook_patched}/src/hook.c"
        "${_preloader_l_minhook_patched}/src/trampoline.c"
        "${_preloader_l_minhook_patched}/src/hde/hde64.c"
    )
    target_include_directories(minhook_debug PUBLIC "${_preloader_l_minhook_patched}/include")
    target_include_directories(minhook_debug PRIVATE
        "${_preloader_l_minhook_patched}/src"
        "${_preloader_l_minhook_patched}/src/hde"
    )
    set_target_properties(minhook_debug PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED YES
        C_EXTENSIONS NO
    )
    if(MSVC)
        set_property(TARGET minhook_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    endif()

    # HF11: Debug references TinyXML2 DOM symbols just like shipping, but
    # debug builds use /MTd. Compile the SAME pinned vendor implementation
    # into a separate /MTd target; do not cross-link the shipping /MT archive.
    add_library(preloader_l_tinyxml2_debug STATIC third_party/tinyxml2/tinyxml2.cpp)
    target_include_directories(preloader_l_tinyxml2_debug PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2")
    if(MSVC)
        set_property(TARGET preloader_l_tinyxml2_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    endif()

    add_library(preloader_l_debug SHARED
        src/dll_entry.cpp
        src/debug/runtime/loader_signals.cpp
        src/runtime/runtime_state.cpp
        src/runtime/ptd_runtime.cpp
        src/runtime/ptd_fls.cpp
        src/runtime/exception_runtime.cpp
        src/runtime/fatal_policy.cpp
        src/runtime/fatal_termination.cpp
        src/runtime/memory_allocator.cpp
        src/runtime/heap_allocator.cpp
        src/runtime/loader_bridge.cpp
        src/runtime/entry_flow.cpp
        src/debug/runtime/runtime_status.cpp
        src/debug/runtime/consistency_closure.cpp
        src/debug/runtime/secondary_integration.cpp
        src/runtime/secondary_integration_entry.cpp
        src/runtime/bootstrap_state.cpp
        src/runtime/bootstrap_runner.cpp
        src/runtime/release_server.cpp
        src/runtime/reference_owner.cpp
        src/runtime/service_storage.cpp
        src/runtime/service_projection.cpp
    src/runtime/service_abi.cpp
    src/runtime/service_store.cpp
        src/runtime/service_policy.cpp
        src/runtime/service_values.cpp
        src/runtime/service_settings.cpp
        src/runtime/service_xml.cpp
        src/runtime/release_service_lifetime.cpp
    src/runtime/release_server_lifecycle.cpp
    src/runtime/asio_listener_owner.cpp
    src/runtime/asio_session_owner.cpp
    src/runtime/asio_service_owner.cpp
    src/runtime/asio_release_bridge.cpp
        src/runtime/bootstrap_thread.cpp
        src/runtime/utc_time.cpp
        src/runtime/avatar_path.cpp
        src/runtime/event_queue.cpp
        src/runtime/achievement_watcher.cpp
        src/runtime/auth_code_service.cpp
        src/runtime/service_state.cpp
    src/runtime/service_state_storage.cpp
    src/runtime/service_state_parser.cpp
    src/runtime/service_state_grant.cpp
        src/config/settings_values.cpp
        src/config/settings_provider.cpp
        src/config/runtime_provider.cpp
        src/config/runtime_configuration.cpp
        src/config/runtime_configuration_parser.cpp
        src/config/config2_pegtl.cpp
        src/config/runtime_configuration_storage.cpp
        src/config/runtime_configuration_file.cpp
        src/config/process_configuration.cpp
            src/config/store_values.cpp
            src/config/store_parser.cpp
    src/config/store_materialization.cpp
    src/config/store_runtime.cpp
    src/config/store_service_bridge.cpp
        src/config/store_payload.cpp
        src/config/msvc_byte_vector_abi.cpp
        src/config/byte_vector_binding.cpp
        src/config/store_vectors.cpp
        src/hash/fnv_hash.cpp
        src/codec/hex_codec.cpp
        src/codec/aes128_cipher.cpp
        src/codec/message_codec.cpp
        src/codec/session_crypto.cpp
        src/net/port_mapping.cpp
        src/net/socket_transport.cpp
        src/net/socket_runtime.cpp
        src/net/socket_listener.cpp
        src/net/socket_endpoint.cpp
        src/net/winsock_native.cpp
        src/xml/lsx_framing.cpp
        src/xml/request_parser.cpp
    src/xml/request_children.cpp
        src/xml/response_builder.cpp
        src/xml/value_codec.cpp
    src/xml/value_codec_utf8.cpp
    src/xml/value_codec_normalize.cpp
    src/xml/value_codec_semantic.cpp
        src/xml/attribute_parser.cpp
        src/router/route_lookup.cpp
        src/router/route_inventory.cpp
        src/handlers/progressive_installation_route.cpp
        src/handlers/permission_route.cpp
        src/handlers/friends_route.cpp
    src/runtime/local_social_state.cpp
    src/runtime/local_social_host.cpp
        src/handlers/integration_routes.cpp
    src/handlers/group_info_route.cpp
    src/handlers/utc_time_route.cpp
    src/handlers/query_presence_route.cpp
    src/handlers/social_mutation_routes.cpp
    src/handlers/social_mutation_socket.cpp
    src/handlers/local_social_list_route.cpp
        src/handlers/config_route.cpp
        src/handlers/setting_value_route.cpp
        src/handlers/settings_snapshot_route.cpp
        src/handlers/handler_registry.cpp
        src/handlers/connection_state_route.cpp
        src/handlers/connection_state_binding.cpp
        src/handlers/runtime_settings_routes.cpp
        src/handlers/selector_pipeline.cpp
        src/handlers/game_info_routes.cpp
    src/handlers/profile_routes.cpp
    src/handlers/auth_routes.cpp
    src/handlers/store_routes.cpp
    src/handlers/achievement_routes.cpp
    src/handlers/achievement_grant_route.cpp
    src/handlers/achievement_query_route.cpp
    src/handlers/image_route.cpp
        src/core/request_dispatch.cpp
        src/protocol/challenge_handshake.cpp
        src/protocol/socket_exchange.cpp
        src/protocol/socket_exchange_wire.cpp
        src/protocol/socket_exchange_frame_step.cpp
        src/protocol/socket_exchange_transaction.cpp
    src/protocol/socket_exchange_transaction_delivery.cpp
        src/protocol/socket_exchange_write_completion.cpp
    src/protocol/socket_exchange_async_transaction.cpp
        src/platform/registry_policy.cpp
        src/platform/registry_policy_config.cpp
        src/platform/registry_policy_state.cpp
        src/platform/registry_policy_values.cpp
        src/platform/registry_bridge.cpp
        src/platform/registry_hook_runtime.cpp
        src/platform/registry_wrappers.cpp
        src/platform/registry_hooks.cpp
        src/platform/minhook_runtime.cpp
            src/platform/connect_redirect.cpp
                                        src/platform/connect_redirect_hook.cpp
        src/platform/environment_policy.cpp
        src/platform/process_environment.cpp
        src/platform/pe_import.cpp
            src/debug/evidence/crypt_guard.cpp
        src/debug/evidence/crypt_path_restore.cpp
        src/debug/evidence/architecture_provenance.cpp
        src/platform/crypt_wrapper.cpp
        src/debug/platform/optional_host_compat.cpp
        src/debug/platform/optional_host_scan.cpp
        src/platform/crypt_observer.cpp
        src/debug/platform/crypt_observer_diagnostics.cpp
        src/debug/runtime/event_trace.cpp
        src/debug/runtime/trace_format.cpp
        src/debug/runtime/debug_trace.cpp
        src/debug/net/session_state.cpp
        src/debug/net/receive_buffer.cpp
        src/debug/net/frame_ingress.cpp
        src/debug/net/socket_session.cpp
        src/debug/handlers/settings_query.cpp
        src/debug/debug_diagnostics.cpp
        src/debug/self_test.cpp
        src/debug/debug_api.cpp
        src/debug/runtime_api.cpp
        src/debug/session_api.cpp
        src/debug/socket_api.cpp
        src/debug/evidence/identity_state.cpp
        exports/preloader_l_debug.def
    )
    target_compile_features(preloader_l_debug PRIVATE cxx_std_20)
    target_include_directories(preloader_l_debug PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/pegtl/upstream/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/filesystem/upstream/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/third_party/asio/upstream/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/debug"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/debug/platform"
    )
    target_compile_definitions(preloader_l_debug PRIVATE
        WIN32_LEAN_AND_MEAN NOMINMAX ASIO_STANDALONE=1 ASIO_NO_TYPEID=1
        PRELOADER_L_DEBUG_BUILD=1 PRELOADER_L_TRACE_DIAGNOSTICS=1
    )
    if(MSVC)
        set_property(TARGET preloader_l_debug PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
        target_compile_options(preloader_l_debug PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:/W4;/permissive-;/Od;/Zi;/GS;/GR->
        )
        target_link_options(preloader_l_debug PRIVATE /INCREMENTAL:NO /DEBUG:FULL /MAP)
        target_link_libraries(preloader_l_debug PRIVATE kernel32 ws2_32 crypt32 minhook_debug preloader_l_tinyxml2_debug)
    endif()
    set_target_properties(preloader_l_debug PROPERTIES PREFIX "" OUTPUT_NAME "preloader_l_debug")

    foreach(host IN ITEMS debug runtime session socket)
        add_executable(preloader_l_${host}_host tools/preloader_l_${host}_host/${host}_host_main.cpp)
        target_compile_features(preloader_l_${host}_host PRIVATE cxx_std_17)
        target_compile_definitions(preloader_l_${host}_host PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
        if(MSVC)
            # Debug hosts are developer tools, not shipping/freestanding artifacts.
            # Use the normal Debug CRT so /RTC and FH4/GS compiler support resolve
            # through the supported MSVC runtime; normalized shipping now uses the same supported CRT family.
            # Their sources expose normal main(), so CRT startup and /RTC initialization run.
            set_property(TARGET preloader_l_${host}_host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
            target_compile_options(preloader_l_${host}_host PRIVATE /W4 /permissive- /Od /Zi /GS /GR-)
            target_link_options(preloader_l_${host}_host PRIVATE /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /DEBUG:FULL /MAP)
            if(host STREQUAL "socket")
                target_link_libraries(preloader_l_${host}_host PRIVATE kernel32 ws2_32)
            else()
                target_link_libraries(preloader_l_${host}_host PRIVATE kernel32)
            endif()
        endif()
        set_target_properties(preloader_l_${host}_host PROPERTIES OUTPUT_NAME "preloader_l_${host}_host")
    endforeach()
endif()

add_executable(preloader_l_release_host
    tools/preloader_l_release_host/release_host_main.cpp
    src/codec/aes128_cipher.cpp
    src/codec/hex_codec.cpp
    src/codec/message_codec.cpp
    src/codec/session_crypto.cpp
)
target_compile_features(preloader_l_release_host PRIVATE cxx_std_17)
target_compile_definitions(preloader_l_release_host PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(preloader_l_release_host PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
if(MSVC)
    set_property(TARGET preloader_l_release_host PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded")
    target_compile_options(preloader_l_release_host PRIVATE /W4 /permissive- /O2 /GS /GR-)
    target_link_options(preloader_l_release_host PRIVATE /SUBSYSTEM:CONSOLE /INCREMENTAL:NO /DEBUG:NONE)
    target_link_libraries(preloader_l_release_host PRIVATE kernel32 ws2_32)
endif()
set_target_properties(preloader_l_release_host PROPERTIES OUTPUT_NAME "preloader_l_release_host")
