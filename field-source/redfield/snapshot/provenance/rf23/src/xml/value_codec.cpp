// SPDX-License-Identifier: GPL-3.0-only
#include "value_codec_internal.hpp"
#include "redfield/core/checked_math.hpp"

namespace redfield { namespace xml {
using namespace value_codec_detail;

ValueCodecStatus decode_source_value(
    TextSpan source,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || !valid_span(source) || (!output && capacity != 0))
        return ValueCodecStatus::invalid_argument;

    if (source.decoded()) {
        // TinyXML2 already expanded XML entities; validate UTF-8/XML legality,
        // then pass semantic bytes through without a second entity decode.
        size_t required=0;
        for (size_t pos=0;pos<source.size;) {
            const Utf8Unit unit=validate_utf8_unit(source.data,source.size,pos);
            if (unit.status!=ValueCodecStatus::ok) return unit.status;
            if (!add_size(&required,unit.size)) return ValueCodecStatus::overflow;
            pos+=unit.size;
        }
        *written=required;
        if (!output) return ValueCodecStatus::ok;
        size_t with_nul=0;
        if (!redfield::core::checked_add_size(required,1u,with_nul)) return ValueCodecStatus::overflow;
        if (capacity<with_nul) return ValueCodecStatus::capacity;
        for (size_t i=0;i<required;++i) output[i]=source.data[i];
        output[required]='\0';
        return ValueCodecStatus::ok;
    }
    size_t required = 0;
    for (size_t p = 0; p < source.size;) {
        const DecodedUnit unit = decode_unit(source.data, source.size, p);
        if (unit.status != ValueCodecStatus::ok) return unit.status;
        if (!add_size(&required, unit.size)) return ValueCodecStatus::overflow;
        p += unit.consumed;
    }
    *written = required;

    if (!output) return ValueCodecStatus::ok;
    size_t with_nul = 0;
    if (!redfield::core::checked_add_size(required, 1u, with_nul))
        return ValueCodecStatus::overflow;
    if (capacity < with_nul) return ValueCodecStatus::capacity;

    size_t out_pos = 0;
    for (size_t p = 0; p < source.size;) {
        const DecodedUnit unit = decode_unit(source.data, source.size, p);
        if (unit.status != ValueCodecStatus::ok) return unit.status;
        for (size_t i = 0; i < unit.size; ++i) output[out_pos++] = unit.bytes[i];
        p += unit.consumed;
    }
    output[out_pos] = '\0';
    return ValueCodecStatus::ok;
}

ValueCodecStatus escape_attribute_value(
    TextSpan semantic,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || !valid_span(semantic) || (!output && capacity != 0))
        return ValueCodecStatus::invalid_argument;

    size_t required = 0;
    for (size_t i = 0; i < semantic.size;) {
        const Utf8Unit unit = validate_utf8_unit(semantic.data, semantic.size, i);
        if (unit.status != ValueCodecStatus::ok) return unit.status;
        size_t add = unit.size;
        if (unit.size == 1u) {
            switch (semantic.data[i]) {
                case '&': add = 5u; break;   // &amp;
                case '<': add = 4u; break;   // &lt;
                case '>': add = 4u; break;   // &gt;
                case '"': add = 6u; break;  // &quot;
                case '\'': add = 6u; break; // &apos;
                default: break;
            }
        }
        if (!add_size(&required, add)) return ValueCodecStatus::overflow;
        i += unit.size;
    }
    *written = required;
    if (!output) return ValueCodecStatus::ok;

    size_t with_nul = 0;
    if (!redfield::core::checked_add_size(required, 1u, with_nul))
        return ValueCodecStatus::overflow;
    if (capacity < with_nul) return ValueCodecStatus::capacity;

    size_t p = 0;
    auto append = [&](const char* z, size_t n) noexcept {
        for (size_t j = 0; j < n; ++j) output[p++] = z[j];
    };
    for (size_t i = 0; i < semantic.size;) {
        const Utf8Unit unit = validate_utf8_unit(semantic.data, semantic.size, i);
        if (unit.status != ValueCodecStatus::ok) return unit.status;
        if (unit.size == 1u) {
            switch (semantic.data[i]) {
                case '&': append("&amp;", 5u); break;
                case '<': append("&lt;", 4u); break;
                case '>': append("&gt;", 4u); break;
                case '"': append("&quot;", 6u); break;
                case '\'': append("&apos;", 6u); break;
                default: output[p++] = semantic.data[i]; break;
            }
        } else {
            append(semantic.data + i, unit.size);
        }
        i += unit.size;
    }
    output[p] = '\0';
    return ValueCodecStatus::ok;
}



} }
