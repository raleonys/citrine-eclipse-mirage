#pragma once
// Native-acceptance-only fault controls for actual Asio-owned resources.
// No shipping behavior or public DLL ABI is modified.
#include <cstdint>
#if defined(_WIN32) && defined(PRELOADER_L_NATIVE_ACCEPTANCE)
namespace preloader_l { namespace runtime {
void configure_asio_acceptance_resource_fault(uint32_t selector) noexcept;
bool consume_asio_acceptance_socket_close_failure() noexcept;
bool consume_asio_acceptance_listener_close_failure() noexcept;
void note_asio_acceptance_socket_close_retained() noexcept;
void note_asio_acceptance_socket_close_retired() noexcept;
void note_asio_acceptance_listener_close_retained() noexcept;
void note_asio_acceptance_listener_close_retired() noexcept;
uint32_t asio_acceptance_resource_fault_metric(uint32_t selector) noexcept;
} }
#endif
