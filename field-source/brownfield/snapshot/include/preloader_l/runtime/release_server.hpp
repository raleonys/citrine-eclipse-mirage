#pragma once
#include <stddef.h>
#include <stdint.h>
#include "preloader_l/net/socket_runtime.hpp"
#include "preloader_l/runtime/bootstrap_runner.hpp"
#include "preloader_l/protocol/socket_exchange.hpp"
#ifndef PRELOADER_L_TRACE_DIAGNOSTICS
#define PRELOADER_L_TRACE_DIAGNOSTICS 0
#endif
#if PRELOADER_L_TRACE_DIAGNOSTICS
#include "preloader_l/runtime/event_trace.hpp"
#endif

namespace preloader_l { namespace runtime {
namespace model_abi { struct ReleaseServiceObject548; }

#if PRELOADER_L_TRACE_DIAGNOSTICS
using ReleaseTraceFn = void (*)(TraceEvent event, uint32_t value) noexcept;
#endif
using ReleaseListenerReadyFn = bool (*)(uint16_t port) noexcept;

struct ReleaseServerContext {
    const preloader_l::net::SocketApi* socket_api;
    preloader_l::net::SocketRuntime runtime;
    preloader_l::net::SocketListenerState listener;
    preloader_l::net::SocketEndpointState client;
    preloader_l::protocol::ExchangeState protocol;
    uint16_t compatibility_port;
    uint16_t first_port;
    uint16_t last_port;
#if PRELOADER_L_TRACE_DIAGNOSTICS
    uint32_t session_id;
    ReleaseTraceFn trace;
#endif
    size_t last_written;
    ReleaseListenerReadyFn listener_ready;
    // Clean semantic state which is not part of the curio 0x548 service
    // object. Keeping it here avoids hiding non-model metadata inside the
    // curio XMLDocument/settings bytes.
    uint32_t configuration_loaded;
    uint32_t configuration_present_sections;
    // Clean coordination pointer only. Worker/thread/socket ownership lives in
    // the curio ReleaseServiceObject548 fields, not in this sidecar.
    model_abi::ReleaseServiceObject548* model_service;
    // HF13: separately owned native Asio service generation, never borrowed
    // by the 0x548 model service or the historical WinSock worker owner.
    void* asio_bridge;
};

void reset_release_server(ReleaseServerContext* context,
                          const preloader_l::net::SocketApi* api) noexcept;
BootstrapBackend make_release_server_backend(ReleaseServerContext* context) noexcept;

} }
