#include "preloader_l/xml/response_builder.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "preloader_l/core/checked_math.hpp"

namespace preloader_l { namespace xml {
namespace {
constexpr char prefix_a[] = " <LSX>\n  <Response id=\"";
constexpr char prefix_b[] = "\" sender=\"";
constexpr char prefix_c[] = "\">\n    ";
constexpr char suffix[] = "\n  </Response>\n </LSX>\n";

constexpr size_t len_a = sizeof(prefix_a) - 1;
constexpr size_t len_b = sizeof(prefix_b) - 1;
constexpr size_t len_c = sizeof(prefix_c) - 1;
constexpr size_t len_suffix = sizeof(suffix) - 1;

bool valid_span(TextSpan s) noexcept
{
    return s.data != nullptr || s.size == 0;
}

bool add(size_t& total, size_t value) noexcept
{
    size_t next = 0;
    if (!preloader_l::core::checked_add_size(total, value, next)) return false;
    total = next;
    return true;
}

void append(char*& dst, const char* src, size_t n) noexcept
{
    for (size_t i = 0; i < n; ++i) dst[i] = src[i];
    dst += n;
}

ResponseStatus map_codec_status(ValueCodecStatus status) noexcept
{
    switch (status) {
        case ValueCodecStatus::ok: return ResponseStatus::ok;
        case ValueCodecStatus::overflow: return ResponseStatus::overflow;
        case ValueCodecStatus::invalid_argument:
        case ValueCodecStatus::malformed_entity:
        case ValueCodecStatus::invalid_character:
        case ValueCodecStatus::capacity:
        case ValueCodecStatus::allocation_failed:
            return ResponseStatus::invalid_attribute;
    }
    return ResponseStatus::invalid_attribute;
}

typedef ValueCodecStatus (*AttributeCodec)(TextSpan, char*, size_t, size_t*) noexcept;

ResponseStatus build_impl(
    TextSpan id,
    TextSpan sender,
    TextSpan body,
    AttributeCodec codec,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || !codec || !valid_span(body) || (!output && capacity != 0))
        return ResponseStatus::invalid_argument;

    size_t id_size = 0;
    size_t sender_size = 0;
    ValueCodecStatus codec_status = codec(id, nullptr, 0, &id_size);
    if (codec_status != ValueCodecStatus::ok) return map_codec_status(codec_status);
    codec_status = codec(sender, nullptr, 0, &sender_size);
    if (codec_status != ValueCodecStatus::ok) return map_codec_status(codec_status);

    size_t required = 0;
    if (!add(required, len_a) || !add(required, id_size) || !add(required, len_b) ||
        !add(required, sender_size) || !add(required, len_c) || !add(required, body.size) ||
        !add(required, len_suffix)) return ResponseStatus::overflow;

    size_t with_nul = 0;
    if (!preloader_l::core::checked_add_size(required, 1u, with_nul)) return ResponseStatus::overflow;
    *written = required;
    if (!output || capacity < with_nul) return ResponseStatus::capacity;

    char* dst = output;
    append(dst, prefix_a, len_a);
    size_t encoded = 0;
    codec_status = codec(id, dst, capacity - static_cast<size_t>(dst - output), &encoded);
    if (codec_status != ValueCodecStatus::ok || encoded != id_size) return map_codec_status(codec_status);
    dst += encoded;
    append(dst, prefix_b, len_b);
    encoded = 0;
    codec_status = codec(sender, dst, capacity - static_cast<size_t>(dst - output), &encoded);
    if (codec_status != ValueCodecStatus::ok || encoded != sender_size) return map_codec_status(codec_status);
    dst += encoded;
    append(dst, prefix_c, len_c);
    append(dst, body.data, body.size);
    append(dst, suffix, len_suffix);
    *dst = '\0';
    *written = required;
    return ResponseStatus::ok;
}
} // namespace

ResponseStatus build_response_envelope(
    TextSpan id,
    TextSpan sender,
    TextSpan body,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    // `id` and `sender` are semantic values on this API. Escape all five XML
    // attribute specials instead of rejecting otherwise-valid semantic data.
    return build_impl(id, sender, body, &escape_attribute_value, output, capacity, written);
}

ResponseStatus build_response_envelope_from_source_attributes(
    TextSpan id,
    TextSpan sender,
    TextSpan body,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    // RequestView stores non-owning spans into the original XML source. Decode
    // XML entity spelling and re-escape exactly once while serializing the
    // response envelope. This prevents both reject-on-special and double escape.
    return build_impl(id, sender, body, &normalize_source_attribute_value, output, capacity, written);
}

} } // namespace preloader_l::xml
