// SPDX-License-Identifier: GPL-3.0-only
#include "value_codec_internal.hpp"
#include "redfield/core/checked_math.hpp"
#include <cstring>

namespace redfield { namespace xml {
using namespace value_codec_detail;

ValueCodecStatus normalize_source_attribute_value(
    TextSpan source,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || !valid_span(source) || (!output && capacity != 0))
        return ValueCodecStatus::invalid_argument;

    if (source.decoded())
        return escape_attribute_value(source,output,capacity,written);
    size_t required = 0;
    for (size_t p = 0; p < source.size;) {
        if (source.data[p] == '&') {
            const DecodedUnit unit = decode_unit(source.data, source.size, p);
            if (unit.status != ValueCodecStatus::ok) return unit.status;
            const Utf8Unit utf = validate_utf8_unit(unit.bytes, unit.size, 0u);
            if (utf.status != ValueCodecStatus::ok || utf.size != unit.size)
                return ValueCodecStatus::invalid_character;
            size_t add = unit.size;
            if (unit.size == 1u) {
                switch (unit.bytes[0]) {
                    case '&': add = 5u; break;
                    case '<': add = 4u; break;
                    case '>': add = 4u; break;
                    case '"': add = 6u; break;
                    case '\'': add = 6u; break;
                    default: break;
                }
            }
            if (!add_size(&required, add)) return ValueCodecStatus::overflow;
            p += unit.consumed;
        } else {
            const Utf8Unit unit = validate_utf8_unit(source.data, source.size, p);
            if (unit.status != ValueCodecStatus::ok) return unit.status;
            size_t add = unit.size;
            if (unit.size == 1u) {
                switch (source.data[p]) {
                    case '&': add = 5u; break;
                    case '<': add = 4u; break;
                    case '>': add = 4u; break;
                    case '"': add = 6u; break;
                    case '\'': add = 6u; break;
                    default: break;
                }
            }
            if (!add_size(&required, add)) return ValueCodecStatus::overflow;
            p += unit.size;
        }
    }
    *written = required;
    if (!output) return ValueCodecStatus::ok;

    size_t with_nul = 0;
    if (!redfield::core::checked_add_size(required, 1u, with_nul))
        return ValueCodecStatus::overflow;
    if (capacity < with_nul) return ValueCodecStatus::capacity;

    size_t o = 0;
    auto append = [&](const char* z, size_t n) noexcept {
        for (size_t i = 0; i < n; ++i) output[o++] = z[i];
    };
    for (size_t p = 0; p < source.size;) {
        char bytes[4]{0,0,0,0};
        size_t byte_count = 0;
        size_t consumed = 0;
        if (source.data[p] == '&') {
            const DecodedUnit unit = decode_unit(source.data, source.size, p);
            if (unit.status != ValueCodecStatus::ok) return unit.status;
            const Utf8Unit utf = validate_utf8_unit(unit.bytes, unit.size, 0u);
            if (utf.status != ValueCodecStatus::ok || utf.size != unit.size)
                return ValueCodecStatus::invalid_character;
            for (size_t i = 0; i < unit.size; ++i) bytes[i] = unit.bytes[i];
            byte_count = unit.size;
            consumed = unit.consumed;
        } else {
            const Utf8Unit unit = validate_utf8_unit(source.data, source.size, p);
            if (unit.status != ValueCodecStatus::ok) return unit.status;
            for (size_t i = 0; i < unit.size; ++i) bytes[i] = source.data[p + i];
            byte_count = unit.size;
            consumed = unit.size;
        }

        if (byte_count == 1u) {
            switch (bytes[0]) {
                case '&': append("&amp;", 5u); break;
                case '<': append("&lt;", 4u); break;
                case '>': append("&gt;", 4u); break;
                case '"': append("&quot;", 6u); break;
                case '\'': append("&apos;", 6u); break;
                default: output[o++] = bytes[0]; break;
            }
        } else {
            append(bytes, byte_count);
        }
        p += consumed;
    }
    output[o] = '\0';
    return ValueCodecStatus::ok;
}


bool source_value_equals(
    TextSpan source,
    TextSpan semantic,
    ValueCodecStatus* status) noexcept
{
    if (status) *status = ValueCodecStatus::ok;
    if (!valid_span(source) || !valid_span(semantic)) {
        if (status) *status = ValueCodecStatus::invalid_argument;
        return false;
    }
    if (source.decoded()) {
        size_t validated=0;
        const auto rc=decode_source_value(source,nullptr,0,&validated);
        if (rc!=ValueCodecStatus::ok) {
            if (status) *status=rc;
            return false;
        }
        if (source.size!=semantic.size) return false;
        return !source.size || std::memcmp(source.data,semantic.data,source.size)==0;
    }
    size_t out_pos = 0;
    bool equal = true;
    for (size_t p = 0; p < source.size;) {
        const DecodedUnit unit = decode_unit(source.data, source.size, p);
        if (unit.status != ValueCodecStatus::ok) {
            if (status) *status = unit.status;
            return false;
        }
        // Equality and source validity are independent.  Once a mismatch is
        // known, continue decoding the complete source so a malformed trailing
        // entity/UTF-8 unit still wins in the explicit status result.
        if (out_pos > semantic.size || unit.size > semantic.size - out_pos) {
            equal = false;
        } else {
            for (size_t i = 0; i < unit.size; ++i)
                if (unit.bytes[i] != semantic.data[out_pos + i]) equal = false;
        }
        if (unit.size > static_cast<size_t>(-1) - out_pos) {
            if (status) *status = ValueCodecStatus::overflow;
            return false;
        }
        out_pos += unit.size;
        p += unit.consumed;
    }
    return equal && out_pos == semantic.size;
}

bool source_value_equals_literal(
    TextSpan source,
    const char* literal,
    ValueCodecStatus* status) noexcept
{
    if (!literal) {
        if (status) *status = ValueCodecStatus::invalid_argument;
        return false;
    }
    return source_value_equals(source, TextSpan{literal, literal_length(literal)}, status);
}


} }
