#pragma once

#include <stddef.h>
#include <stdint.h>

#include "preloader_l/xml/request_parser.hpp"

namespace preloader_l { namespace xml {

enum class ResponseStatus : uint8_t {
    ok = 0,
    invalid_argument,
    invalid_attribute,
    capacity,
    overflow,
};

// Build the response envelope format observed in the curio constant
// templates. `written` excludes the convenience terminal NUL.
ResponseStatus build_response_envelope(
    TextSpan id,
    TextSpan sender,
    TextSpan body,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

// RequestView attribute spans still carry XML-source entity spelling. This
// variant normalizes source attributes to semantic values and escapes them
// exactly once in the emitted envelope.
ResponseStatus build_response_envelope_from_source_attributes(
    TextSpan id,
    TextSpan sender,
    TextSpan body,
    char* output,
    size_t capacity,
    size_t* written) noexcept;

} } // namespace preloader_l::xml
