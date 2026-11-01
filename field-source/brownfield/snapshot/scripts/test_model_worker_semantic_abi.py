from pathlib import Path
import sys
r=Path(__file__).resolve().parents[1]
abi=(r/'include/preloader_l/runtime/service_abi.hpp').read_text()
worker=(r/'src/runtime/release_server_worker.cpp').read_text()
acceptor=(r/'src/runtime/release_server_acceptor.cpp').read_text()
lifecycle=(r/'src/runtime/release_server_lifecycle.cpp').read_text()
internal=(r/'src/runtime/release_server_io_internal.hpp').read_text()
public=(r/'include/preloader_l/runtime/release_server_io.hpp').read_text()
src='\n'.join((worker,acceptor,lifecycle,internal))
life=(r/'src/runtime/release_service_lifetime.cpp').read_text()
server=(r/'src/runtime/release_server.cpp').read_text()
protocol='\n'.join((
 (r/'src/protocol/socket_exchange.cpp').read_text(),
 (r/'src/protocol/socket_exchange_transaction.cpp').read_text(),
))
dispatch=(r/'src/core/request_dispatch.cpp').read_text()
checks={
 'jthread24': 'static_assert(sizeof(ModelJthread24) == 0x18' in abi,
 'closure38': 'static_assert(sizeof(ModelWorkerClosure38) == 0x38' in abi,
 'mutex50': 'static_assert(sizeof(ModelRecursiveMutex50) == 0x50' in abi,
 'service_offsets': all(x in abi for x in ['accept_thread) == 0x018','client_records) == 0x030','mutex_state) == 0x048']),
 'no_global_workers': 'g_workers' not in src and 'collect_finished' not in src,
 'dynamic_vector': 'capacity + capacity / 2u' in src and 'client_records.capacity' in src,
 'project_worker_hard_cap': 'release_client_hard_cap = 128u' in public and 'release_client_admission_allowed(size)' in acceptor and 'grown > release_client_hard_cap' in acceptor,
 'worker_socket_owned': 'worker_close(&state)' in src,
 'stop_policy': 'detail::stop_requested(endpoint->stop)' in src and '!detail::load_active(endpoint->service)' in src and 'return !endpoint || bootstrap_stopping()' not in src,
 'mutex_serializes_append': 'lock_service_mutex(service)' in src and 'append_client_thread(' in acceptor and 'service, client, remote_address, remote_address_size);' in acceptor,
 'two_pass_teardown': 'for (auto* it = begin; it && it != end; ++it) request_jthread_stop(it);' in src and 'if (destroy_jthread(it)) continue;' in src,
 'retain_until_quiesce': 'service->client_records.end = reinterpret_cast<uintptr_t>(retained);' in src and 'return retained == begin;' in src,
 'client_pressure_nonterminal': 'semantic_terminal(context, 0x5d02u)' not in acceptor and 'server_trace(context, TraceEvent::failure, 0x5d02u)' in acceptor and 'AppendClientResult::capacity_rejected' in acceptor,
 'accept_owner_service_field': 'service->accept_thread.handle' in src,
 'lifetime_quiesce': 'quiesce_release_worker_policy_windows(adapter)' in life,
 'server_asio_only_shutdown': 'quiesce_release_worker_policy_windows(context)' not in server and 'stop_release_asio_windows(context)' in server,
 'legacy_worker_shutdown_fail_closed': 'legacy_worker_slots_unowned' in lifecycle and 'if (!legacy_worker_slots_unowned(service)) return false;' in lifecycle,
 'native_worker_test_only_gate': '#if defined(PRELOADER_L_NATIVE_ACCEPTANCE)' in lifecycle and 'return false; // a failed Asio start must not start a legacy worker' in lifecycle,
 'active_ipv4_sockaddr': 'static_assert(sizeof(SockaddrIn16) == 0x10' in src and 'accept(listener, &peer, &peer_size)' in src,
 'active_inet_ntop_shape': 'detail::inet_ntop(detail::kAfInet, &peer.address, remote_address, sizeof(remote_address))' in src and 'char remote_address[0x16]{};' in src,
 'peer_no_inet_ntop_failure_branch': '(void)detail::inet_ntop(' in src,
 'peer_written_to_closure': 'assign_model_service_string' in src and '&closure->remote_address' in src,
 'peer_session_plumbing': 'serve_protocol_session_with_peer' in src and 'SessionPeerContext peer_context' in src,
 'peer_dispatch_plumbing': 'DispatchEnvironment environment{peer.address, peer.address_size}' in protocol and 'dispatch_plaintext_request_with_environment' in dispatch,
 'quiesce_wait_checked': 'const uint32_t waited = WaitForSingleObject' in src and 'waited != kWaitObject0' in src and 'semantic_terminal(context, 0x5d07u)' in src,
 'listener_nonblocking': 'configure_release_socket_nonblocking_windows(' in src and 'context->listener.socket' in src and 'ioctlsocket(socket, kFionbio, &enabled)' in src,
 'accepted_client_nonblocking': 'configure_release_socket_nonblocking_windows(client)' in src,
 'accept_bounded_poll': 'wait_release_socket_windows(listener, false)' in src and 'timeout.tv_usec = 100000' in src,
 'native_source_uses_asio_ownership': 'start_release_asio_windows(context, bound_port)' in server and 'drive_release_asio_windows(context)' in server and 'shutdown_release_asio_windows(context)' in server,
 'model_service_abi_invariant': 'static_assert(sizeof(ReleaseServiceObject548) == 0x548' in abi,
 'listener_close_after_join': lifecycle.find('WaitForSingleObject(\n            reinterpret_cast<detail::HANDLE>(service->accept_thread.handle), detail::kInfinite)') < lifecycle.find('close_socket_listener(&context->listener)', lifecycle.find('void quiesce_release_worker_policy_windows(')),
}
fail=[k for k,v in checks.items() if not v]
if fail:
    print('MODEL_WORKER_SEMANTIC_ABI: FAIL'); [print(' -',x) for x in fail]; sys.exit(1)
print(f'MODEL_WORKER_SEMANTIC_ABI: PASS ({len(checks)}/{len(checks)})')
