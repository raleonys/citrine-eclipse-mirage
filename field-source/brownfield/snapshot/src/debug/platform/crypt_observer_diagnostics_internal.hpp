#pragma once

#include <stdint.h>

#if PRELOADER_L_TRACE_DIAGNOSTICS
namespace preloader_l { namespace platform {

void trace_crypt_query_diagnostics(uint32_t object_type, void* caller) noexcept;
void trace_crypt_query_companion_branch() noexcept;
bool uninstall_crypt_observer_diagnostics() noexcept;

} }
#endif
