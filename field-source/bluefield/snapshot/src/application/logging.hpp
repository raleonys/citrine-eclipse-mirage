// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#ifndef PRELOADER_L_DISK_DIAGNOSTICS
#define PRELOADER_L_DISK_DIAGNOSTICS 0
#endif
namespace preloader_l {
enum class LogEvent {startup,stopped,ready,failure,phase,hook,config_base_present,config_base_absent,config_override_present,config_override_absent,config_unsupported,profile_local,profile_online,profile_request,network_receive,network_send,network_event,network_parse,network_crypto,network_failure,listener_started,listener_stopped,dispatch_failed,accept_rearm_failed,provider,license,avatar,unknown_field,startup_failed,shutdown_failed};
constexpr bool disk_diagnostics_enabled() noexcept{return PRELOADER_L_DISK_DIAGNOSTICS==1;}
const char* log_event_name(LogEvent) noexcept;
}
