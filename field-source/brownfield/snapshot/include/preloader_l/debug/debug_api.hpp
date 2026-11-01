#pragma once
#include <stddef.h>
#include <stdint.h>

#if defined(_WIN32)
#define PRELOADER_L_DEBUG_CALL __cdecl
#else
#define PRELOADER_L_DEBUG_CALL
#endif

extern "C" {


struct preloader_l_debug_runtime_status {
    uint32_t phase;
    uint32_t generation;
    uint64_t module_base;
    uint32_t initialize_count;
    uint32_t shutdown_count;
    uint32_t process_attach_count;
    uint32_t process_detach_count;
    uint32_t surge_call_count;
    uint32_t reserved;
};

constexpr uint32_t PRELOADER_L_DEBUG_RUNTIME_NOT_INITIALIZED = 0x80010001u;
constexpr uint32_t PRELOADER_L_DEBUG_SESSION_NEED_MORE = 0x80020001u;
constexpr uint32_t PRELOADER_L_DEBUG_SESSION_INGRESS_ERROR = 0x80020002u;
constexpr uint32_t PRELOADER_L_DEBUG_SESSION_NOT_ACTIVE = 0x80020003u;

struct preloader_l_debug_session_status {
    uint32_t phase;
    uint32_t session_id;
    uint64_t received_bytes;
    uint64_t sent_bytes;
    uint32_t receive_events;
    uint32_t send_events;
    uint32_t buffered_bytes;
    uint32_t feed_events;
    uint32_t frames_ready;
    uint32_t frames_consumed;
    uint32_t ingress_errors;
    uint32_t reserved;
};

struct preloader_l_debug_socket_status {
    uint32_t runtime_started;
    uint32_t listener_active;
    uint32_t client_active;
    uint32_t session_phase;
    uint32_t session_id;
    uint16_t bound_port;
    uint16_t reserved16;
    int32_t last_socket_error;
    uint32_t dispatch_count;
    uint32_t responses_sent;
};

struct preloader_l_debug_stats {
    uint64_t self_test_runs;
    uint64_t self_test_failures;
    uint64_t dispatch_calls;
    uint64_t dispatch_ok;
    uint64_t dispatch_non_ok;
    uint64_t codec_checks;
    uint64_t framing_checks;
    uint64_t route_checks;
};

// Returns zero when all self-tests pass; otherwise returns the failed bitmask.
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_self_test(uint32_t* passed_mask, uint32_t* failed_mask) noexcept;

// Returns the numeric preloader_l::core::DispatchStatus. The response buffer is
// caller-owned; `written` is zero on all non-successful handler paths.
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_dispatch(const char* request, size_t request_length,
                                          char* response, size_t response_capacity,
                                          size_t* written) noexcept;

void PRELOADER_L_DEBUG_CALL preloader_l_debug_reset_stats() noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_stats(preloader_l_debug_stats* out, size_t out_size) noexcept;

uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_runtime_init() noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_runtime_shutdown() noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_runtime_status(preloader_l_debug_runtime_status* out, size_t out_size) noexcept;

uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_session_reset(uint32_t session_id) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_session_feed(const char* input, size_t length,
                                             char* response, size_t response_capacity,
                                             size_t* written) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_session_status(preloader_l_debug_session_status* out, size_t out_size) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_session_close() noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_start(uint16_t requested_port, uint16_t* bound_port) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_accept(uint32_t session_id) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_pump(char* response, size_t response_capacity, size_t* written) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_get_socket_status(preloader_l_debug_socket_status* out, size_t out_size) noexcept;
uint32_t PRELOADER_L_DEBUG_CALL preloader_l_debug_socket_stop() noexcept;

}
