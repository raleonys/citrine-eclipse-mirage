#include "preloader_l/platform/crypt_guard.hpp"

namespace preloader_l { namespace platform {

bool should_fail_closed_crypt_msg(const void* message,
                                  uint32_t param_type,
                                  bool caller_is_main) noexcept
{
    return message == nullptr && param_type == 6u && caller_is_main;
}

} }
