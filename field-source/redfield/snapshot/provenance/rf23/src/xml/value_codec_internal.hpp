// SPDX-License-Identifier: GPL-3.0-only
#pragma once

#include "redfield/xml/value_codec.hpp"
#include <stddef.h>
#include <stdint.h>

namespace redfield { namespace xml { namespace value_codec_detail {

struct DecodedUnit {
    char bytes[4];
    size_t size;
    size_t consumed;
    ValueCodecStatus status;
};

struct Utf8Unit {
    uint32_t codepoint;
    size_t size;
    ValueCodecStatus status;
};

bool valid_span(TextSpan s) noexcept;
bool add_size(size_t* total, size_t value) noexcept;
Utf8Unit validate_utf8_unit(const char* s, size_t n, size_t pos) noexcept;
DecodedUnit decode_unit(const char* s, size_t n, size_t pos) noexcept;
size_t literal_length(const char* literal) noexcept;

} } }
