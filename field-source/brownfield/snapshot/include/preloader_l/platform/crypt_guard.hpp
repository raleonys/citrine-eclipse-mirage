#pragma once
#include <stdint.h>

namespace preloader_l { namespace platform {

// Defensive compatibility guard only. It never converts a failed crypto/trust
// operation into success. The narrow predicate matches the crash path observed
// in the host: main-module caller, CMSG_SIGNER_INFO_PARAM (6), null HCRYPTMSG.
bool should_fail_closed_crypt_msg(const void* message,
                                  uint32_t param_type,
                                  bool caller_is_main) noexcept;

constexpr uint32_t crypt_msg_null_guard_error = 6u; // ERROR_INVALID_HANDLE

} }
