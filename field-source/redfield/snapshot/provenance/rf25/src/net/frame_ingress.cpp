// SPDX-License-Identifier: GPL-3.0-only
#include "redfield/net/frame_ingress.hpp"
#include "redfield/xml/lsx_framing.hpp"
#include <stdint.h>

namespace redfield { namespace net {
namespace {
void clear_frame(redfield::xml::TextSpan* frame) noexcept
{
    if (frame) *frame = redfield::xml::TextSpan{nullptr, 0};
}
IngressStatus scan(IngressState* state, redfield::xml::TextSpan* frame) noexcept
{
    clear_frame(frame);
    if (!state || !frame) return IngressStatus::invalid_argument;
    if (state->receive.size == 0) return IngressStatus::need_more;
    const auto result = redfield::xml::scan_lsx_frame(
        reinterpret_cast<const char*>(state->receive.bytes), state->receive.size);
    if (result.status == redfield::xml::FrameStatus::need_more) return IngressStatus::need_more;
    if (result.status != redfield::xml::FrameStatus::frame_ready) {
        ++state->errors;
        return IngressStatus::invalid_frame;
    }
    frame->data = reinterpret_cast<const char*>(state->receive.bytes);
    frame->size = result.frame_length;
    ++state->frames_ready;
    return IngressStatus::frame_ready;
}
}

void reset_ingress(IngressState* state) noexcept
{
    if (!state) return;
    // Do not materialize an 8 KiB zero-initialized temporary on the stack. The
    // buffered bytes are outside the active range once size is zero, so only
    // metadata needs resetting. This also keeps bounded debug/native builds
    // independent of unnecessary large-stack probing.
    clear_receive_buffer(&state->receive);
    state->bytes_fed = 0;
    state->feed_events = 0;
    state->frames_ready = 0;
    state->frames_consumed = 0;
    state->errors = 0;
}

IngressStatus feed_ingress(IngressState* state, const uint8_t* input,
                           size_t length, redfield::xml::TextSpan* frame) noexcept
{
    clear_frame(frame);
    if (!state || !frame || (!input && length != 0)) return IngressStatus::invalid_argument;
    if (length > UINT64_MAX - state->bytes_fed || state->feed_events == UINT32_MAX) {
        ++state->errors;
        return IngressStatus::capacity;
    }
    const auto appended = append_receive_bytes(&state->receive, input, length);
    if (appended == ReceiveBufferStatus::capacity) {
        ++state->errors;
        return IngressStatus::capacity;
    }
    if (appended != ReceiveBufferStatus::ok) return IngressStatus::invalid_argument;
    state->bytes_fed += static_cast<uint64_t>(length);
    ++state->feed_events;
    return scan(state, frame);
}

IngressStatus peek_ingress_frame(IngressState* state,
                                 redfield::xml::TextSpan* frame) noexcept
{
    return scan(state, frame);
}

IngressStatus consume_ingress_frame(IngressState* state, size_t frame_length) noexcept
{
    if (!state || frame_length == 0 || frame_length > state->receive.size)
        return IngressStatus::invalid_argument;
    const auto scanned = redfield::xml::scan_lsx_frame(
        reinterpret_cast<const char*>(state->receive.bytes), state->receive.size);
    if (scanned.status != redfield::xml::FrameStatus::frame_ready || scanned.frame_length != frame_length) {
        ++state->errors;
        return IngressStatus::invalid_frame;
    }
    if (consume_receive_bytes(&state->receive, frame_length) != ReceiveBufferStatus::ok)
        return IngressStatus::invalid_argument;
    ++state->frames_consumed;
    return IngressStatus::ok;
}

} }
