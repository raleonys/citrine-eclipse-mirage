// SPDX-License-Identifier: GPL-3.0-only
#pragma once
#include <stdint.h>
namespace preloader_l { namespace handlers {
enum class HandlerStatus : uint8_t {
    ok = 0,
    invalid_request,
    wrong_command,
    response_error,
    unsupported,
    no_response_required,
};
} }
