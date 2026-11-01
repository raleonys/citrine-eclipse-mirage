#pragma once

#include "preloader_l/handlers/handler_status.hpp"
#include "preloader_l/xml/request_parser.hpp"
#include "preloader_l/xml/response_builder.hpp"
#include "preloader_l/xml/value_codec.hpp"
#include "route_error_contracts.hpp"

namespace preloader_l { namespace handlers { namespace detail {

inline size_t cstr_len(const char* text) noexcept
{
    size_t n = 0;
    if (text) while (text[n]) ++n;
    return n;
}

inline size_t i32_decimal(int32_t value, char* output) noexcept
{
    uint32_t magnitude = value < 0
        ? static_cast<uint32_t>(-(static_cast<int64_t>(value)))
        : static_cast<uint32_t>(value);
    char reverse[16]{};
    size_t digits = 0;
    do {
        reverse[digits++] = static_cast<char>('0' + (magnitude % 10u));
        magnitude /= 10u;
    } while (magnitude != 0u);

    size_t pos = 0;
    if (value < 0) output[pos++] = '-';
    for (size_t i = 0; i < digits; ++i) output[pos + i] = reverse[digits - 1u - i];
    pos += digits;
    output[pos] = '\0';
    return pos;
}

inline HandlerStatus render_error_success(
    const preloader_l::xml::RequestView& request,
    preloader_l::protocol::OriginError code,
    const char* description,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    if (written) *written = 0;
    if (!written || (!output && capacity != 0) || !description) return HandlerStatus::invalid_request;
    if (!request.id.data || request.id.size == 0) return HandlerStatus::invalid_request;

    const preloader_l::xml::TextSpan description_span{description, cstr_len(description)};
    size_t escaped_description_size = 0;
    const auto description_probe = preloader_l::xml::escape_attribute_value(
        description_span, nullptr, 0, &escaped_description_size);
    if (description_probe != preloader_l::xml::ValueCodecStatus::ok) return HandlerStatus::response_error;

    // Route-specific descriptions in the active contracts are short. Keep this
    // bounded and fail closed rather than allocating from a protocol helper.
    char escaped_description[256]{};
    if (escaped_description_size >= sizeof(escaped_description)) return HandlerStatus::response_error;
    size_t escaped_description_written = 0;
    const auto description_render = preloader_l::xml::escape_attribute_value(
        description_span, escaped_description, sizeof(escaped_description), &escaped_description_written);
    if (description_render != preloader_l::xml::ValueCodecStatus::ok ||
        escaped_description_written != escaped_description_size) return HandlerStatus::response_error;

    char code_text[16]{};
    const size_t code_size = i32_decimal(preloader_l::protocol::origin_error_value(code), code_text);

    constexpr char prefix[] = "<ErrorSuccess Description=\"";
    constexpr char middle[] = "\" Code=\"";
    constexpr char suffix[] = "\"/>";
    char body[384]{};
    size_t pos = 0;
    const auto append = [&](const char* data, size_t size) noexcept -> bool {
        if (!data || size > sizeof(body) - pos) return false;
        for (size_t i = 0; i < size; ++i) body[pos + i] = data[i];
        pos += size;
        return true;
    };
    if (!append(prefix, sizeof(prefix) - 1u) ||
        !append(escaped_description, escaped_description_size) ||
        !append(middle, sizeof(middle) - 1u) ||
        !append(code_text, code_size) ||
        !append(suffix, sizeof(suffix) - 1u)) return HandlerStatus::response_error;

    const auto sender = preloader_l::xml::response_sender_policy(request);
    const auto status = preloader_l::xml::build_response_envelope_from_source_attributes(
        request.id, sender, {body, pos}, output, capacity, written);
    return status == preloader_l::xml::ResponseStatus::ok ? HandlerStatus::ok : HandlerStatus::response_error;
}

inline HandlerStatus render_error_success(
    const preloader_l::xml::RequestView& request,
    const preloader_l::handlers::route_error_contracts::ErrorSuccessContract& contract,
    char* output,
    size_t capacity,
    size_t* written) noexcept
{
    return render_error_success(request, contract.code, contract.description, output, capacity, written);
}

} } } // namespace preloader_l::handlers::detail
